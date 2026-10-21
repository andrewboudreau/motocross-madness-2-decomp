"""The loading dialog owns the progress control's actual UIDialog receiver."""
import json
import os
from pathlib import Path
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
PROVIDER = '?FindControl@UIDialog@@QAEPAVUIControl@@PBDH@Z'
RETIRED = '?FindControl@UnknownGameUiPage@@QAEPAVUIControl@@PBDH@Z'
SOURCES = ('EventManager', 'KrustyUI', 'DlgProcs', 'GameUi')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class ProgressDialogConnectionTests(unittest.TestCase):
    def test_loading_lifecycle_pointer_and_complete_callers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        pins = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(reference), pins['mcm2_sha256'])
        retail = PEImage(reference)
        classes = {c.name: c for c in parse_rtti(retail)}
        loading = classes['LoadingDlg']
        self.assertEqual(loading.direct_bases, ['UIDialog'])
        self.assertEqual({r.object_offset for r in loading.vtable_records}, {0})
        base = next(b for b in loading.bases if b.name == 'UIDialog')
        self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x451020, 582), 0x451020))
        stores = [i for i in instructions if i.mnemonic == 'mov' and i.operands[0].type == capstone.x86.X86_OP_MEM
                  and i.operands[0].mem.disp == 0x490]
        self.assertEqual([i.address for i in stores], [0x45113f, 0x4511f2])
        self.assertEqual([i.reg_name(i.operands[1].reg) for i in stores], ['ebp', 'edi'])

        with tempfile.TemporaryDirectory(prefix='mcm2-progress-dialog-') as directory:
            objects = {}
            sources = ['src/reconstructed/' + name + '.cpp' for name in SOURCES]
            sources.append('tests/fixtures/progress_dialog_layout.cpp')
            for index, source in enumerate(sources):
                obj = Path(directory) / (str(index) + '.obj')
                run = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                objects[source] = CoffObject(obj)
            for source, obj in objects.items():
                imports = {s.name for s in obj.symbols if s.section_number == 0}
                self.assertNotIn(RETIRED, imports)
                if source.endswith(('EventManager.cpp', 'KrustyUI.cpp')):
                    self.assertIn(PROVIDER, imports)
                    calls = {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 0x14}
                    self.assertIn(PROVIDER, calls)
            layout = objects[sources[-1]]
            entry = layout.find_symbol('?LoadingDialogBase@@YAPAVUIDialog@@PAVLoadingDlg@@@Z')
            body, size, relocations = layout.symbol_extent(entry)
            self.assertEqual(body, bytes.fromhex('8b442404c3'))  # unchanged argument pointer, ret
            self.assertEqual(size, 5)
            self.assertEqual(relocations, [])
            checked = []
            for case in CASES:
                if case['source'] not in objects:
                    continue
                result = compare(retail, objects[case['source']], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'], ROOT / case['bindings'])
                self.assertTrue(result['strict_exact'], (case['symbol'], result))
                checked.append(case)
            self.assertGreaterEqual(len(checked), 506)
            self.assertTrue(any(c['symbol'] == PROVIDER and c['target_size'] == 200 for c in checked))


if __name__ == '__main__':
    unittest.main()
