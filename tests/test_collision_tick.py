"""Collision tick retains its independent secondary-base identity and full body."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = 'src/reconstructed/CollisionObjectTick.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
CONSTRAINT = 'src/krusty2/constraint/ConstraintMethodCollisionModel.cpp'
GAME = 'src/reconstructed/GameObject.cpp'
TICK = '?Tick@CollisionObject@@UAEHM@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native authentic VC6 and private retail image')
class CollisionTickTests(unittest.TestCase):
    def test_secondary_table_complete_provider_caller_and_native_boundary(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, table in (('CollisionObject', 0x551148), ('ConstraintMethodCollisionModel', 0x5511f0)):
            base = next(b for b in classes[name].bases if b.name == 'GameObject')
            self.assertEqual((base.mdisp, base.pdisp), (12, -1))
            locator = word(table - 4)
            self.assertEqual(word(locator + 4), 12)
            typename = ('.?AV' + name + '@@\0').encode()
            self.assertEqual(retail.bytes_at_va(word(locator + 12) + 8, len(typename)), typename)
        self.assertEqual(word(0x551148 + 10 * 4), 0x499ae0)
        self.assertEqual(word(0x5511f0 + 10 * 4), 0x43ba40)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        body = list(decoder.disasm(retail.bytes_at_va(0x499ae0, 13), 0x499ae0))
        self.assertEqual([(i.mnemonic, i.op_str) for i in body], [
            ('mov', 'eax, dword ptr [esp + 4]'), ('push', 'eax'),
            ('call', '0x4693d0'), ('ret', '4')])
        self.assertEqual(retail.bytes_at_va(0x499aed, 3), b'\x90' * 3)
        caller = list(decoder.disasm(retail.bytes_at_va(0x43ba40, 37), 0x43ba40))
        self.assertEqual(sum(i.size for i in caller), 37)
        self.assertEqual([(i.address, i.op_str) for i in caller if i.mnemonic == 'call'],
                         [(0x43ba55, 'dword ptr [eax + 0x2c]'), (0x43ba5b, '0x499ae0')])
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-tick-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            fixture = 'tests/fixtures/collision_tick.cpp'
            for index, source in enumerate((PROVIDER, COLLISION, CONSTRAINT, GAME, fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source], objects[source] = obj, CoffObject(obj)
            for source, symbol, va, size in (
                (PROVIDER, TICK, 0x499ae0, 13),
                (GAME, '?Tick@GameObject@@UAEHM@Z', 0x4693d0, 94),
                (CONSTRAINT, '?Tick@ConstraintMethodCollisionModel@@UAEHM@Z', 0x43ba40, 37)):
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            obj = objects[COLLISION]
            table = obj.find_symbol('??_7CollisionObject@@6BGraphicsTest@@@')
            relocations = {r.virtual_address: obj.symbol_by_index[r.symbol_index].name
                           for r in obj.relocations if r.section_number == table.section_number}
            self.assertEqual(relocations[table.value + 40], TICK)
            # Code itself is entered on GameObject+0, i.e. complete CollisionObject+12.
            obj = objects[PROVIDER]
            external = {s.name for s in obj.symbols if s.section_number == 0 and s.storage_class == 2}
            self.assertEqual(external, {'?Tick@GameObject@@UAEHM@Z', '__fltused'})
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'tick.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths[PROVIDER]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Collision tick: 4096 secondary-base, raw-argument, return and exception cases passed')
            print('Three complete strict bodies; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
