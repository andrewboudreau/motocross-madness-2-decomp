"""Missing visibility scalars: PE initialization, complete users and COFF storage."""
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
from mcm2tool.toolchain import sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
CELLS = {
    '?g_visibilityQueryTicks@@3HA': (0x68ab88, [0x52ef99, 0x52efc2, 0x52f02d]),
    '?g_visibilityDebugDraw@@3HA': (0x68ab9c, [0x52e3b1, 0x52eee0, 0x52eeed]),
    '?g_visibilityFreeze@@3HA': (0x68aba0, [0x52ef11, 0x52ef1e, 0x52ef5c]),
}
USERS = [
    ('?UnknownVirtualSlot14@VisibilityQuadTree@@', 0x52e3b0, 148),
    ('?OnControlPress@VisibilityQuadTree@@', 0x52eec0, 135),
    ('?PrepareGeometry@VisibilityQuadTree@@', 0x52ef50, 252),
]


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class VisibilityControlStateTests(unittest.TestCase):
    def test_zero_tail_scalars_all_references_and_complete_consumers(self):
        import capstone
        exe = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(exe),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(exe)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        instructions = {}
        for _, va, size in USERS:
            code = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in code), size)
            instructions.update({i.address: (i.mnemonic, i.op_str) for i in code})
        for name, (va, expected) in CELLS.items():
            section = image.section_for_rva(va - image.image_base)
            offset = va - image.image_base - section.virtual_address
            self.assertEqual(section.name, '.data')
            self.assertTrue(section.characteristics & 0x80000000)
            self.assertGreaterEqual(offset, section.raw_size)
            self.assertLessEqual(offset + 4, section.virtual_size)
            # Exhaustive explicit DWORD-address scan; indirect accesses are not excluded.
            sites, start = [], 0
            while (found := image.data.find(struct.pack('<I', va), start)) >= 0:
                sites.append(image.offset_to_va(found))
                start = found + 1
            self.assertEqual(sites, expected, name)
        for va, mnemonic, operands in [
            (0x52e3b0, 'mov', 'eax, dword ptr [0x68ab9c]'),
            (0x52eede, 'mov', 'ecx, dword ptr [0x68ab9c]'),
            (0x52eee4, 'mov', 'eax, 1'),
            (0x52eee9, 'sub', 'eax, ecx'),
            (0x52eeec, 'mov', 'dword ptr [0x68ab9c], eax'),
            (0x52ef10, 'mov', 'eax, dword ptr [0x68aba0]'),
            (0x52ef15, 'mov', 'ecx, 1'),
            (0x52ef1a, 'sub', 'ecx, eax'),
            (0x52ef1c, 'mov', 'dword ptr [0x68aba0], ecx'),
            (0x52ef22, 'je', '0x52ef36'),
            (0x52ef27, 'mov', 'ecx, 0x88'),
            (0x52ef2c, 'mov', 'edi, 0x68a968'),
            (0x52ef34, 'rep movsd', 'dword ptr es:[edi], dword ptr [esi]'),
            (0x52ef5b, 'mov', 'eax, dword ptr [0x68aba0]'),
            (0x52ef64, 'mov', 'eax, 0x68a968'),
            (0x52ef80, 'lea', 'ecx, [esi - 0x874]'),
            (0x52ef86, 'call', '0x52d560'),
            (0x52ef96, 'sub', 'eax, edi'),
            (0x52ef98, 'mov', 'dword ptr [0x68ab88], eax'),
            (0x52efc1, 'mov', 'eax, dword ptr [0x68ab88]'),
            (0x52f02c, 'mov', 'eax, dword ptr [0x68ab88]'),
        ]:
            self.assertEqual(instructions[va], (mnemonic, operands))
        bindings = ROOT / 'src/reconstructed/VisibilityControlState.bindings.json'
        self.assertEqual({k: int(v, 0) for k, v in json.loads(bindings.read_text()).items()},
                         {k: v[0] for k, v in CELLS.items()})
        with tempfile.TemporaryDirectory(prefix='mcm2-visibility-state-') as directory:
            objects = []
            for stem, source in [
                ('state', 'src/reconstructed/VisibilityControlState.cpp'),
                ('consumer', 'src/krusty2/visibility/VisibilityQuadTree.cpp'),
            ]:
                path = Path(directory) / (stem + '.obj')
                run = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                objects.append(CoffObject(path))
            provider, consumer = objects
            self.assertEqual(provider.machine, 0x14c)
            definitions = [s for s in provider.symbols if s.storage_class == 2]
            self.assertEqual({s.name for s in definitions}, set(CELLS))
            self.assertEqual({s.value for s in definitions}, {0, 4, 8})
            self.assertEqual(len({s.section_number for s in definitions}), 1)
            storage = provider.section(definitions[0].section_number)
            self.assertEqual((storage.name, storage.raw_size, storage.raw_ptr), ('.bss', 12, 0))
            self.assertTrue(storage.characteristics & 0x80000000)
            self.assertTrue(storage.characteristics & 0x80)
            self.assertEqual([(s.name, s.raw_size) for s in provider.sections
                              if not s.name.startswith('.debug')], [('.drectve', 40), ('.bss', 12)])
            self.assertFalse([r for r in provider.relocations
                              if not provider.section(r.section_number).name.startswith('.debug')])
            actual = {name: [] for name in CELLS}
            for query, va, size in USERS:
                function = consumer.find_symbol(query)
                _, extent, relocations = consumer.symbol_extent(function)
                self.assertEqual(extent, size)
                for r in relocations:
                    name = consumer.symbol_by_index[r.symbol_index].name
                    if name in CELLS:
                        self.assertEqual(r.type, 6)
                        actual[name].append(va + r.virtual_address - function.value)
                result = compare(image, consumer, function.name, va, size,
                                 ROOT / 'src/krusty2/visibility/VisibilityQuadTree.bindings.json')
                self.assertTrue(result['strict_exact'], result)
            self.assertEqual(actual, {name: sites for name, (_, sites) in CELLS.items()})
            print('Visibility control storage: three zero-tail cells, nine references, three complete strict consumers')


if __name__ == '__main__':
    unittest.main()
