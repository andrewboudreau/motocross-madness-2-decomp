"""Default and pure entries retain their individual vehicle-vtable identities."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
TABLES = {
    'Vehicle': ((0x558a84, 0), (0x558a50, 540), (0x5589e0, 1472)),
    'Bike': ((0x550900, 0), (0x5508cc, 540), (0x55085c, 1848)),
    'KrustyBike': ((0x554548, 0), (0x554514, 540), (0x5544a4, 5644)),
}
ENTRIES = {
    'Vehicle': {87: ('?UnknownVirtualSlot87@Vehicle@@UAEXXZ', 0x464e90),
                94: ('?UnknownVirtualSlot94@Vehicle@@UAEXXZ', 0x464e90)},
    'Bike': {87: ('?UnknownVirtualSlot87@Vehicle@@UAEXXZ', 0x464e90),
             94: ('?UnknownVirtualSlot94@Vehicle@@UAEXXZ', 0x464e90),
             101: ('__purecall', 0x534cfe)},
    'KrustyBike': {87: ('?UnknownVirtualSlot87@Vehicle@@UAEXXZ', 0x464e90),
                   94: ('?UnknownVirtualSlot94@KrustyBike@@UAEXXZ', 0x4963f0),
                   101: ('?UnknownVirtualSlot101@KrustyBike@@UAEXXZ', 0x496d10)},
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class VehicleDefaultSlotsTests(unittest.TestCase):
    def test_complete_defaults_and_actual_primary_secondary_and_pure_entries(self):
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        for name, tables in TABLES.items():
            for table, offset in tables:
                locator = word(table - 4)
                self.assertEqual(word(locator + 4), offset)
                type_name = ('.?AV' + name + '@@\0').encode()
                self.assertEqual(retail.bytes_at_va(word(locator + 12) + 8, len(type_name)), type_name)
            for slot, (_, va) in ENTRIES[name].items():
                self.assertEqual(word(tables[0][0] + 4 * slot), va)
        self.assertEqual(retail.bytes_at_va(0x464e90, 1), b'\xc3')
        # Distinct from RET: the purecall body raises the CRT runtime error.
        self.assertEqual(retail.bytes_at_va(0x534cfe, 2), b'\x6a\x19')
        with tempfile.TemporaryDirectory(prefix='mcm2-vehicle-defaults-') as directory:
            output = Path(directory)
            sources = ['src/krusty2/vehicle/' + n + '.cpp' for n in TABLES]
            sources += ['src/reconstructed/VehicleDefaults.cpp', 'tests/fixtures/vehicle_defaults.cpp']
            paths, objects = {}, {}
            for index, source in enumerate(sources):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[Path(source).stem], objects[Path(source).stem] = obj, CoffObject(obj)
            for name, tables in TABLES.items():
                obj = objects[name]
                actual_offsets = set()
                for entry in obj.symbols:
                    if not entry.name.startswith('??_7' + name + '@@6B') or entry.section_number <= 0:
                        continue
                    relocs = {r.virtual_address: obj.symbol_by_index[r.symbol_index] for r in obj.relocations
                              if r.section_number == entry.section_number}
                    locator = relocs[entry.value - 4]
                    section = obj.sections[locator.section_number - 1]
                    offset = struct.unpack_from('<I', obj.data, section.raw_ptr + locator.value + 4)[0]
                    actual_offsets.add(offset)
                    if offset == 0:
                        for slot, (symbol, _) in ENTRIES[name].items():
                            self.assertEqual(relocs[entry.value + 4 * slot].name, symbol, (name, slot))
                self.assertEqual(actual_offsets, {offset for _, offset in tables})
                self.assertFalse(any('UnknownVirtualSlot101@Bike@@' in s.name for s in obj.symbols))
            for slot in (87, 94):
                self.assertTrue(compare(retail, objects['VehicleDefaults'],
                    '?UnknownVirtualSlot%d@Vehicle@@UAEXXZ' % slot, 0x464e90, 1,
                    ROOT / 'src/reconstructed/VehicleDefaults.bindings.json')['strict_exact'])
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'defaults.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths['VehicleDefaults']), str(paths['vehicle_defaults']), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Vehicle defaults: 512 complete-storage calls passed')
            print('Two strict defaults, nine primary/secondary tables, eight slot identities; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
