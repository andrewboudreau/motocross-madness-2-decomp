"""Retail storage, mutability, lifecycle writes and canonical caller closure."""
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
# Name, VA, initial value, independently decoded write and operation.
VALUES = (
    ('g_terrainDebugPage', 0x574714, -1, 0x506a2a, 'mov'),
    ('g_terrainCacheBytes', 0x689ff4, 0, 0x507672, 'mov'),
    ('g_gridDrawMemoryPeak', 0x68a2fc, 0, 0x47f51f, 'mov'),
    ('g_gridDrawMemory', 0x68a300, 0, 0x47ecec, 'mov'),
    ('g_gridDrawFrameCount', 0x68a304, 0, 0x47ef97, 'inc'),
    ('g_terrainAgeTime0', 0x68a308, 0, 0x50770f, 'mov'),
    ('g_terrainAgeTime1', 0x68a30c, 0, 0x507644, 'mov'),
    ('g_terrainCacheCount', 0x68a310, 0, 0x5078ee, 'mov'),
    ('g_terrainPrepareGeometryTicks', 0x68a318, 0, 0x5078f9, 'mov'),
    ('g_terrainRender3DTicks', 0x68a31c, 0, 0x5069f1, 'mov'),
    ('g_terrainTotalTicks', 0x68a320, 0, 0x506ac9, 'mov'),
    ('g_terrainDualTextureProbed', 0x68a328, 0, 0x5064e6, 'mov'),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class TerrainDrawStateTests(unittest.TestCase):
    def test_full_storage_lifecycle_and_actual_callers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        bindings = json.loads((ROOT / 'src/reconstructed/TerrainDrawState.bindings.json').read_text())
        expected = {'?' + n + '@@3HA': va for n, va, value, write, op in VALUES}
        self.assertEqual({k: int(v, 0) for k, v in bindings.items()}, expected)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-state-') as directory:
            objects = {}
            for name, source in (
                ('state', 'src/reconstructed/TerrainDrawState.cpp'),
                ('terrain', 'src/krusty2/broadphase/Terrain.cpp'),
                ('grid', 'src/reconstructed/Griddraw.cpp'),
            ):
                obj = Path(directory) / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)
            obj = objects['state']
            definitions = {s.name: s for s in obj.symbols if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), set(expected))
            self.assertFalse([s for s in obj.symbols if s.storage_class == 2 and s.section_number == 0])
            self.assertFalse([r for r in obj.relocations
                              if not obj.section(r.section_number).name.startswith('.debug')])
            self.assertFalse([s for s in obj.sections if s.name.startswith('.CRT') or s.characteristics & 0x20])
            self.assertEqual(sum(s.raw_size for s in obj.sections if s.characteristics & 0x80000000), 48)
            for name, va, initial, write, op in VALUES:
                symbol = '?' + name + '@@3HA'
                entry = definitions[symbol]
                section = obj.section(entry.section_number)
                following = [s.value for s in definitions.values() if s.section_number == entry.section_number and s.value > entry.value]
                self.assertEqual(min(following, default=section.raw_size) - entry.value, 4)
                zero = va >= 0x670000
                self.assertTrue(section.characteristics & 0x80000000)
                self.assertTrue(section.characteristics & 0x40000000)
                self.assertEqual(bool(section.characteristics & 0x80), zero)
                data = bytes(4) if zero else obj.data[section.raw_ptr + entry.value:section.raw_ptr + entry.value + 4]
                self.assertEqual(data, struct.pack('<i', initial))
                original = retail.section_for_rva(va - retail.image_base)
                self.assertEqual(original.name, '.data')
                self.assertEqual(original.characteristics, 0xc0000040)
                offset = va - retail.image_base - original.virtual_address
                self.assertLessEqual(offset + 4, original.virtual_size)
                if zero:
                    self.assertGreaterEqual(offset, original.raw_size)
                else:
                    self.assertLessEqual(offset + 4, original.raw_size)
                    self.assertEqual(retail.bytes_at_va(va, 4), data)
                instruction = next(decoder.disasm(retail.bytes_at_va(write, 15), write))
                self.assertEqual(instruction.mnemonic, op)
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_MEM)
                self.assertEqual(instruction.operands[0].mem.disp, va)
                self.assertEqual(instruction.operands[0].size, 4)
            # Actual consumer relocations prove these are used providers, and
            # both TUs share the same memory-counter symbols.
            imports = {}
            for key in ('terrain', 'grid'):
                caller = objects[key]
                imports[key] = {caller.symbol_by_index[r.symbol_index].name for r in caller.relocations
                                if caller.symbol_by_index[r.symbol_index].section_number == 0}
            self.assertTrue(set(expected) <= imports['terrain'] | imports['grid'])
            for shared in ('?g_gridDrawMemory@@3HA', '?g_gridDrawMemoryPeak@@3HA'):
                self.assertIn(shared, imports['terrain'])
                self.assertIn(shared, imports['grid'])
            from tools.run_calibration import CASES
            cases = [(objects['grid'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] == 'src/reconstructed/Griddraw.cpp']
            manifest = ROOT / 'src/krusty2/broadphase/targets.json'
            targets = [t for t in json.loads(manifest.read_text())
                       if t['source'] == 'Terrain.cpp' and t.get('expect', 'exact') == 'exact']
            cases.extend((objects['terrain'], t['candidate_symbol_contains'], int(t['target_va'], 0),
                          t['target_size'], manifest.parent / t['bindings']) for t in targets)
            self.assertEqual(len(cases), 79)
            for caller, symbol, va, size, bindings in cases:
                proof = compare(retail, caller, symbol, va, size, bindings)
                self.assertTrue(proof['strict_exact'], (symbol, proof))



if __name__ == '__main__':
    unittest.main()
