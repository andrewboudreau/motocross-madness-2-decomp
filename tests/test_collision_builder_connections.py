"""Complete collision shape callers use the supplied matrix and BVH providers."""
from collections import Counter
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
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
TREE = 'src/reconstructed/BoundingBoxTreeBuild.cpp'
MATRIX = 'src/reconstructed/MatrixUtil.cpp'
MODEL = 'src/reconstructed/D3DIMSoulTree.cpp'
SCENE = 'src/krusty2/soultree/soultree.cpp'
PARTIAL = 'samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp'
PROVIDERS = (
    (MATRIX, '?IdentityMatrix@@YA?AUMatrix4@@XZ', 0x4a1410, 71),
    (TREE, '?BuildModelTriangleTree@@YAXPAUBoxTreeNode@@PAUTreeMatrix4@@PAVTreeModelSource@@PAPAUTreeVec3@@PAVTreeSceneNode@@H@Z', 0x42c8c0, 928),
    (TREE, '?BuildTriangleMeshTree@@YAXPAUBoxTreeNode@@PAUTreeMatrix4@@PAUTreeVec3@@PBHHHH@Z', 0x42cc60, 831),
    (TREE, '?BuildModelPointTree@@YAXPAUBoxTreeNode@@PAUTreeMatrix4@@PAVTreeModelSource@@PAVTreeSceneNode@@@Z', 0x42d390, 1608),
    (TREE, '?BuildModelBoxTree@@YAXPAUBoxTreeNode@@PBUTreeMatrix4@@PAUTreeVec3@@H@Z', 0x42dc90, 501),
    (MODEL, '?TransformVertexGroups@D3DIMSoultreeObject@@QAEXH@Z', 0x440d40, 486),
    (SCENE, '?GetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z', 0x4fca80, 494),
)
CALLERS = (
    ('SetModelShape@CollisionObject', 0x4324b0, 615, (0, 1, 3)),
    ('SetHullShape@CollisionObject', 0x432720, 214, (0, 1, 3)),
    ('SetTriangleMeshHullShape@CollisionObject', 0x4328b0, 231, (0, 2)),
    ('SetMeshShape@CollisionObject', 0x432ab0, 122, (4,)),
    ('ReadShape@CollisionObject', 0x439e10, 178, (0,)),
    ('ReadModelShape@CollisionObject', 0x43a050, 387, (0,)),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class CollisionBuilderConnectionsTests(unittest.TestCase):
    def test_complete_shape_callers_and_tree_provider_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # ModelSource observes the primary scene pointer. The model's separate
        # GameObject base is +0xc and must not be introduced at this boundary.
        locator, = struct.unpack('<I', retail.bytes_at_va(0x5513ec - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0], b'.?AVD3DIMSoultreeObject@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVSoultreeObject@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (12, -1, 0))
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-builders-') as directory:
            objects = {}
            jobs = [(source, []) for source in (COLLISION, TREE, MATRIX, MODEL, SCENE, PARTIAL)]
            fixture = 'tests/fixtures/collision_builder_layout.cpp'
            jobs += [(fixture, []), (fixture, ['--extra', '/DPHYSICS_VIEW'])]
            for index, (source, extra) in enumerate(jobs):
                path = Path(directory) / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *extra], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            for source, symbol, va, size in PROVIDERS:
                obj = objects[source]
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            obj = objects[COLLISION]
            all_calls = Counter()
            for symbol, va, size, provider_indices in CALLERS:
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / COLLISION).with_suffix('.bindings.json'))['strict_exact'], symbol)
                _, _, relocations = obj.symbol_extent(entry)
                calls = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocations if r.type == 0x14)
                code = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in code), size)
                retail_calls = Counter(i.operands[0].imm for i in code if i.mnemonic == 'call'
                                       and i.operands[0].type == capstone.x86.X86_OP_IMM)
                for index in provider_indices:
                    _, target, address, _ = PROVIDERS[index]
                    self.assertEqual(calls[target], 1)
                    self.assertEqual(retail_calls[address], 1)
                    all_calls[target] += 1
            self.assertEqual(sum(all_calls.values()), 11)
            imports = {s.name for s in obj.symbols if s.section_number == 0}
            retired = ('?GetIdentityMatrix@@', '?BuildPointTree@@', '?BuildModelTriangleTree@@YAXPAX',
                       '?BuildTriangleMeshTree@@YAXPAX', '?BuildModelBoxTree@@YAXPAX')
            self.assertFalse(any(name.startswith(retired) for name in imports))
            tree = objects[TREE]
            _, _, relocations = tree.symbol_extent(tree.find_symbol(PROVIDERS[3][1]))
            calls = Counter(tree.symbol_by_index[r.symbol_index].name for r in relocations if r.type == 0x14)
            self.assertEqual(calls[PROVIDERS[5][1]], 1)
            self.assertEqual(calls[PROVIDERS[6][1]], 2)
            code = list(decoder.disasm(retail.bytes_at_va(0x42d390, 1608), 0x42d390))
            retail_calls = Counter(i.operands[0].imm for i in code if i.mnemonic == 'call'
                                   and i.operands[0].type == capstone.x86.X86_OP_IMM)
            self.assertEqual(retail_calls[0x440d40], 1)
            self.assertEqual(retail_calls[0x4fca80], 2)
            partial = objects[PARTIAL]
            entry = partial.find_symbol('?GatherModelTriangles@@')
            self.assertEqual((entry.function_size, entry.extent_source), (1627, 'coff_function_aux'))
            result = compare(retail, partial, entry.name, 0x42c260, 1627,
                             (ROOT / PARTIAL).with_suffix('.bindings.json'))
            self.assertFalse(result['strict_exact'])
            self.assertEqual(result['strict_match']['matching_positions'], 1622)


if __name__ == '__main__':
    unittest.main()
