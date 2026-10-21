"""Vehicle reset: primary slots, complete providers, constants and native state."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/VehicleReset.cpp'
SYMBOL = '?UnknownVirtualSlot36@Vehicle@@UAEXXZ'
TREE = 'src/krusty2/soultree/soultree.cpp'
STEERING = 'src/krusty2/motion/SteeringControl.cpp'
PROVIDERS = (
    (TREE, '?SetPosition@SoultreeObject@@QAEXABUVec3@@@Z', 0x4fc660, 38),
    (TREE, '?SetAxesPtr@SoultreeObject@@QAEXPBUVec3@@0HH@Z', 0x4fbd70, 733),
    (STEERING, '?SetAngle@SteeringControl@@QAEXMPAVSoultreeObject@@@Z', 0x504e20, 150),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail image')
class VehicleResetTests(unittest.TestCase):
    def test_complete_body_constants_slots_and_native_callback_state(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(image), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        code = list(decoder.disasm(retail.bytes_at_va(0x40c710, 368), 0x40c710))
        self.assertEqual(sum(i.size for i in code), 368)
        self.assertEqual([(i.address, i.op_str) for i in code if i.mnemonic == 'call'], [
            (0x40c7cd, 'dword ptr [edx + 0x8c]'), (0x40c7d7, 'dword ptr [eax + 0x88]'),
            (0x40c84e, '0x4fc660'), (0x40c860, '0x4fbd70'), (0x40c873, '0x504e20')])
        self.assertEqual((code[-1].mnemonic, code[-1].op_str), ('ret', ''))
        compiler = Path(os.environ['VC6_ROOT'])
        fixture = 'tests/fixtures/vehicle_reset.cpp'
        callers = {name: 'src/krusty2/vehicle/' + name + '.cpp' for name in ('Vehicle', 'Bike', 'KrustyBike')}
        with tempfile.TemporaryDirectory(prefix='mcm2-vehicle-reset-') as directory:
            output = Path(directory)
            objects, paths = {}, {}
            for index, source in enumerate((SOURCE, TREE, STEERING, fixture, *callers.values())):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            reset = objects[SOURCE]
            for source, symbol, va, size in ((SOURCE, SYMBOL, 0x40c710, 368), *PROVIDERS):
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                self.assertTrue(compare(retail, obj, symbol, va, size,
                    (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            self.assertEqual({s.name for s in reset.symbols if s.section_number == 0},
                             {'__fltused', *(row[1] for row in PROVIDERS)})
            self.assertEqual([(s.raw_size, s.reloc_count) for s in reset.sections if s.name == '.CRT$XCU'], [(16, 4)])
            for name, initializer, va in (('_kVec3Zero', '_$E1', 0x407890), ('_kVec3YAxis', '_$E7', 0x407930)):
                constant = reset.find_symbol(name)
                self.assertEqual(constant.storage_class, 3)
                self.assertGreater(constant.section_number, 0)
                self.assertTrue(compare(retail, reset, initializer, va, 60,
                    ROOT / 'src/reconstructed/VehicleReset.bindings.json')['strict_exact'])
            body, _, relocs = reset.symbol_extent(reset.find_symbol(SYMBOL))
            for _, symbol, _, _ in PROVIDERS:
                self.assertEqual(sum(reset.symbol_by_index[r.symbol_index].name == symbol for r in relocs), 1)
            for name, table_va in (('Vehicle', 0x558a84), ('Bike', 0x550900), ('KrustyBike', 0x554548)):
                word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
                locator = word(table_va - 4)
                self.assertEqual(word(locator + 4), 0)
                type_name = ('.?AV' + name + '@@\0').encode()
                self.assertEqual(retail.bytes_at_va(word(locator + 12) + 8, len(type_name)), type_name)
                self.assertEqual(word(table_va + 36 * 4), 0x40c710)
                obj = objects[callers[name]]
                table = obj.find_symbol('??_7' + name + '@@6BSoultreePhysicsBaseObject@@@')
                routes = {r.virtual_address - table.value: obj.symbol_by_index[r.symbol_index].name
                          for r in obj.relocations if r.section_number == table.section_number}
                self.assertEqual(routes[36 * 4], SYMBOL)
            executable = output / 'reset.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:' + str(executable),
                '/libpath:' + str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(paths[SOURCE]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Vehicle reset: 6144 complete-storage, callback and exception cases passed')
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
