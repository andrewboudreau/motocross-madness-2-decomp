"""Existing terrain/BVH sampling calls use complete supplied implementations."""
import hashlib
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
TERRAIN = 'src/krusty2/broadphase/Terrain.cpp'
TREE = 'src/reconstructed/BoundingBoxTreeBuild.cpp'
GRID = 'src/reconstructed/GridNode.cpp'
MATRIX = 'src/reconstructed/MatrixUtil.cpp'
VECTOR = 'src/reconstructed/VectorCalls.cpp'
SCALE = 'src/reconstructed/VectorScaleAssign.cpp'
BIKE = 'src/krusty2/vehicle/Bike.cpp'
SAMPLER = '?GetCellCorners@GridNode@@QAEHHHPAMPAUGridVec3@@PAE@Z'
TRIANGLE = '?TriangleNormal@@YAXPBUVector3@@00PAU1@PAM@Z'
DOT = '?Vec3DotCall@@YAMPBUVec3@@0@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainSamplingConnectionsTests(unittest.TestCase):
    def test_full_providers_callers_and_actual_grid_sampling(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # The constructed 0x5c-byte field object retains GridNode at offset zero.
        col, = struct.unpack('<I', retail.bytes_at_va(0x553e54 - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 64).split(b'\0')[0],
                         b'.?AVDrawableGridNodeSharedTextures@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 64).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVGridNode@@'], (0, -1, 0))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-sampling-') as directory:
            output = Path(directory)
            fixture = 'tests/fixtures/grid_corner_compare.cpp'
            objects, paths = {}, {}
            for source in (TERRAIN, TREE, GRID, MATRIX, VECTOR, SCALE, BIKE, fixture):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *(['--extra', '/Od'] if source == fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            for source, symbol, address, size in ((GRID, SAMPLER, 0x483910, 1067),
                                                (MATRIX, TRIANGLE, 0x4a11e0, 281),
                                                (VECTOR, DOT, 0x40ae30, 29),
                                                (SCALE, '??XBikeVec3ScaleAssign@@', 0x40ae00, 33),
                                                (SCALE, '??XTerrainVec3@@', 0x40ae00, 33)):
                entry = objects[source].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[source], symbol, address, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
            self.assertFalse(any(section.name.startswith('.CRT') for section in objects[SCALE].sections))
            for source, symbol, address, size, targets in (
                (TERRAIN, '?QueryGround@Terrain@@', 0x507c10, 2963, {SAMPLER: (0x483910, 1), TRIANGLE: (0x4a11e0, 4)}),
                (TREE, '?BuildModelTriangleTree@@', 0x42c8c0, 928, {TRIANGLE: (0x4a11e0, 1)}),
                (TREE, '?BuildTriangleMeshTree@@', 0x42cc60, 831, {TRIANGLE: (0x4a11e0, 1)}),
                (TERRAIN, '?CastSegment@Terrain@@', 0x506e90, 1654,
                 {'??XTerrainVec3@@QAEAAU0@M@Z': (0x40ae00, 1)}),
                (BIKE, '?Method_0x0040a520@Bike@@', 0x40a520, 2272,
                 {'??XBikeVec3ScaleAssign@@QAEAAU0@M@Z': (0x40ae00, 3)}),
            ):
                obj = objects[source]
                entry = obj.find_symbol(symbol)
                _, _, relocations = obj.symbol_extent(entry)
                instructions = list(decoder.disasm(retail.bytes_at_va(address, size), address))
                self.assertEqual(sum(i.size for i in instructions), size)
                for target, (destination, count) in targets.items():
                    self.assertEqual(sum(r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name == target
                                         for r in relocations), count)
                    self.assertEqual(sum(i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                         and i.operands[0].imm == destination for i in instructions), count)
            imports = {s.name for s in objects[TERRAIN].symbols if s.section_number == 0}
            self.assertTrue({DOT, TRIANGLE, SAMPLER} <= imports)
            self.assertFalse(any('TerrainDot@@' in s or 'TerrainTriangleNormal@@' in s or 'GetCellCorners@TerrainShutdownObject' in s for s in imports))
            self.assertNotIn('?TriangleNormal@@YAXPBUTreeVec3@@00PAU1@PAM@Z',
                             {s.name for s in objects[TREE].symbols if s.section_number == 0})
            cases = [c for c in CASES if c['source'] in (TREE, GRID, MATRIX, VECTOR, SCALE)]
            for case in cases:
                self.assertTrue(compare(retail, objects[case['source']], case['symbol'], int(case['target_va'], 0),
                                        case['target_size'], ROOT / case['bindings'])['strict_exact'], case['name'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'grid.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}', str(paths[GRID]), str(paths[SCALE]), str(paths[fixture]),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('native leaf, descent, clamp, output and fault cases match', result.stdout)
            print(f'{len(cases)} strict provider/BVH cases; {result.stdout.strip()}')


if __name__ == '__main__':
    unittest.main()
