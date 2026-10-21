"""Distinct UI vtable roles, exact complete providers, and native field effects."""
import hashlib
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
METHODS = {
    '?KillFocus@UIControl@@UAEXH@Z': (0x464e80, 3),
    '?SyncBoundValue@UIControl@@UAEXH@Z': (0x464e80, 3),
    '?HandleTimer@UIControl@@UAEXH@Z': (0x464e80, 3),
    '?IsInteractive@UIControl@@UAEHXZ': (0x467ae0, 6),
    '?IsInteractive@UIDropDownList@@UAEHXZ': (0x467ae0, 6),
    '?IsInteractive@UIStaticText@@UAEHXZ': (0x478fe0, 3),
    '?SetState@UIStaticText@@UAEXH@Z': (0x479220, 20),
}
TABLES = {
    'UIControl': (0x552ba4, {30: '?IsInteractive@UIControl@@UAEHXZ',
        33: '?KillFocus@UIControl@@UAEXH@Z', 59: '?SyncBoundValue@UIControl@@UAEXH@Z',
        60: '?HandleTimer@UIControl@@UAEXH@Z'}),
    'UIStaticText': (0x553420, {29: '?SetState@UIStaticText@@UAEXH@Z',
        30: '?IsInteractive@UIStaticText@@UAEHXZ', 33: '?KillFocus@UIControl@@UAEXH@Z',
        59: '?SyncBoundValue@UIControl@@UAEXH@Z', 60: '?HandleTimer@UIControl@@UAEXH@Z'}),
    'UIDropDownList': (0x553a58, {29: '?SetState@UIStaticText@@UAEXH@Z',
        30: '?IsInteractive@UIDropDownList@@UAEHXZ', 60: '?HandleTimer@UIControl@@UAEXH@Z'}),
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class UIControlDefaultsTests(unittest.TestCase):
    def test_complete_providers_primary_slots_and_native_state_changes(self):
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, (table, slots) in TABLES.items():
            locator = word(table - 4)
            self.assertEqual(word(locator + 4), 0)
            type_name = ('.?AV' + name + '@@\0').encode()
            self.assertEqual(retail.bytes_at_va(word(locator + 12) + 8, len(type_name)), type_name)
            base_names = [name] + (['UIStaticText'] if name == 'UIDropDownList' else [])
            if name != 'UIControl':
                base_names.append('UIControl')
            base_names += ['GameObject', 'BaseObject']
            self.assertEqual([(b.name, b.mdisp, b.pdisp, b.vdisp) for b in classes[name].bases],
                             [(base, 0, -1, 0) for base in base_names])
            for slot, symbol in slots.items():
                self.assertEqual(word(table + 4 * slot), METHODS[symbol][0])
        # Complete independent instruction effects, including callee cleanup.
        self.assertEqual(retail.bytes_at_va(0x464e80, 3), b'\xc2\x04\0')
        self.assertEqual(retail.bytes_at_va(0x467ae0, 6), b'\xb8\x01\0\0\0\xc3')
        self.assertEqual(retail.bytes_at_va(0x478fe0, 3), b'\x33\xc0\xc3')
        self.assertEqual(retail.bytes_at_va(0x479220, 20),
                         bytes.fromhex('8b442404c781c001000001000000894160c20400'))
        with tempfile.TemporaryDirectory(prefix='mcm2-ui-defaults-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            for source in ('src/reconstructed/UIControlDefaults.cpp', 'src/reconstructed/GameUi.cpp',
                           'tests/fixtures/ui_control_defaults.cpp'):
                path = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/reconstructed')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[Path(source).stem], objects[Path(source).stem] = path, CoffObject(path)
            provider = objects['UIControlDefaults']
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2
                              and s.section_number > 0 and s.type == 0x20}, set(METHODS))
            for symbol, (address, extent) in METHODS.items():
                self.assertEqual(provider.symbol_extent(provider.find_symbol(symbol))[1], extent)
                self.assertTrue(compare(retail, provider, symbol, address, extent,
                    ROOT / 'src/reconstructed/UIControlDefaults.bindings.json')['strict_exact'])
            obj = objects['GameUi']
            for name, (_, slots) in TABLES.items():
                table = obj.find_symbol('??_7' + name + '@@6B@')
                relocs = {r.virtual_address: obj.symbol_by_index[r.symbol_index] for r in obj.relocations
                          if r.section_number == table.section_number}
                locator = relocs[table.value - 4]
                section = obj.sections[locator.section_number - 1]
                self.assertEqual(struct.unpack_from('<I', obj.data, section.raw_ptr + locator.value + 4)[0], 0)
                for slot, symbol in slots.items():
                    self.assertEqual(relocs[table.value + 4 * slot].name, symbol, (name, slot))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'defaults.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths['UIControlDefaults']), str(paths['ui_control_defaults']), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'UI defaults: 12544 complete-storage calls passed')
            print('Seven strict bodies and twelve primary slots; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
