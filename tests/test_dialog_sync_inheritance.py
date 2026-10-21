"""Retail primary slots and actual option-dialog vtables use UIDialog's body."""
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import sha256_file
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = '?SyncValues@UIDialog@@UAEXH@Z'
TABLES = {
    'OptionsDlg': 0x551b18,
    'GlobalSettingsDlg': 0x555604,
    'ConfirmRestoreDlg': 0x5559a0,
    'OptAdvancedGraphicsDlg': 0x555580,
    'OptMessagesDlg': 0x55570c,
}
SOURCES = ('OptionProcs', 'DlgProcs', 'KrustyUI', 'ProCircuitProcs', 'SelectGamePicProcs', 'GameUi')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class DialogSyncInheritanceTests(unittest.TestCase):
    def test_primary_slots_compiled_vtables_and_complete_callers(self):
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(reference), expected['mcm2_sha256'])
        retail = PEImage(reference)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, table in TABLES.items():
            cls = classes[name]
            self.assertEqual(cls.direct_bases, ['UIDialog'])
            relation = next(b for b in cls.bases if b.name == 'UIDialog')
            self.assertEqual((relation.mdisp, relation.pdisp), (0, -1))
            self.assertEqual([(r.object_offset, r.cd_offset, r.vtables) for r in cls.vtable_records],
                             [(0, 0, [table])])
            self.assertEqual(struct.unpack('<I', retail.bytes_at_va(table + 31 * 4, 4))[0], 0x470050)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x552a9c + 31 * 4, 4))[0], 0x470050)

        with tempfile.TemporaryDirectory(prefix='mcm2-dialog-sync-') as directory:
            objects = {}
            sources = ['src/reconstructed/' + name + '.cpp' for name in SOURCES]
            sources.append('tests/fixtures/dialog_sync_inheritance.cpp')
            for index, source in enumerate(sources):
                obj = Path(directory) / (str(index) + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(obj)

            retired = {'?SyncValues@' + name + '@@UAEXH@Z' for name in TABLES}
            seen = set()
            for source, obj in objects.items():
                imports = {s.name for s in obj.symbols if s.section_number == 0}
                self.assertFalse(imports & retired, source)
                for name in TABLES:
                    tables = [s for s in obj.symbols if s.name == '??_7' + name + '@@6B@'
                              and s.section_number > 0]
                    for table in tables:
                        relocations = [r for r in obj.relocations if r.section_number == table.section_number
                                       and r.virtual_address == table.value + 31 * 4]
                        self.assertEqual(len(relocations), 1, (source, name))
                        relocation = relocations[0]
                        self.assertEqual(relocation.type, 6)  # IMAGE_REL_I386_DIR32
                        self.assertEqual(obj.symbol_by_index[relocation.symbol_index].name, PROVIDER)
                        seen.add(name)
            self.assertEqual(seen, set(TABLES))

            checked = []
            for case in CASES:
                if case['source'] not in objects:
                    continue
                report = compare(retail, objects[case['source']], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'], ROOT / case['bindings'])
                self.assertTrue(report['strict_exact'], (case['source'], case['symbol'], report))
                checked.append(case)
            self.assertGreaterEqual(len(checked), 593)
            self.assertTrue(any(c['symbol'] == PROVIDER and c['target_size'] == 24 for c in checked))


if __name__ == '__main__':
    unittest.main()
