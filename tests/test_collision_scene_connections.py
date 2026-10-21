"""Collision ownership and scene traversal use existing complete providers."""
from collections import Counter
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
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
CALLER = 'src/krusty2/collision/CollisionObject.cpp'
PROVIDERS = (
    ('src/krusty2/bvh/BoundingBoxTreeQuery.cpp', '?FreeBoxTree@@YAXPAUQueryTreeNode@@@Z', 0x42a160, 52),
    ('src/reconstructed/GameObject.cpp', '?AppendClassName@GameObject@@QAEXPAV1@@Z', 0x469ce0, 201),
    ('src/krusty2/soultree/soultree.cpp', '?CountNodes@SoultreeObject@@QAEHXZ', 0x4fda30, 46),
    ('src/krusty2/soultree/soultree.cpp', '?CollectDescendants@SoultreeObject@@QAEXPAHPAPAV1@@Z', 0x4fda60, 76),
    ('src/reconstructed/D3DIMSoulTree.cpp', '?NodeMovesVertices@D3DIMSoultreeObject@@QAEHPAVSoultreeObject@@H@Z', 0x445060, 91),
)
CALLS = (
    ('CollisionHullShape_Free', 0x431da0, 71, {0: 2}),
    ('CollisionMeshShape_Free', 0x431e50, 32, {0: 1}),
    ('??0CollisionObject', 0x431e70, 346, {1: 1}),
    ('SetModelShape@CollisionObject', 0x4324b0, 615, {2: 1, 3: 1, 4: 2}),
    ('ReadModelShape@CollisionObject', 0x43a050, 387, {2: 1, 3: 1}),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class CollisionSceneConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_and_secondary_base(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        locator, = struct.unpack('<I', retail.bytes_at_va(0x551148 - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
        self.assertEqual(offset, 12)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVCollisionObject@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVGraphicsTest@@'], (12, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (12, -1, 0))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-scene-') as directory:
            objects = {}
            fixture = 'tests/fixtures/collision_scene_layout.cpp'
            jobs = [(source, []) for source in dict.fromkeys([CALLER, *(r[0] for r in PROVIDERS), fixture])]
            jobs.append((fixture, ['--extra', '/DQUERY_VIEW']))
            for index, (source, extra) in enumerate(jobs):
                path = Path(directory) / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *extra], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if not extra:
                    objects[source] = CoffObject(path)
            obj = objects[fixture]
            self.assertEqual(obj.symbol_extent(obj.find_symbol('_CollisionGameObjectOffset')),
                             (bytes.fromhex('b80c000000c3'), 6, []))
            for source, symbol, va, size in PROVIDERS:
                obj = objects[source]
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            obj = objects[CALLER]
            for symbol, va, size, wanted in CALLS:
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / CALLER).with_suffix('.bindings.json'))['strict_exact'], symbol)
                _, _, relocs = obj.symbol_extent(entry)
                calls = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs if r.type == 0x14)
                code = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in code), size)
                direct = Counter(i.operands[0].imm for i in code if i.mnemonic == 'call'
                                 and i.operands[0].type == capstone.x86.X86_OP_IMM)
                for index, count in wanted.items():
                    _, target, address, _ = PROVIDERS[index]
                    self.assertEqual(calls[target], count)
                    self.assertEqual(direct[address], count)
            retired = ('?FreeBoxTree@@YAXPAX', '?AppendClassName@GraphicsTest',
                       '?CountNodes@CollisionSourceNode', '?CollectNodes@CollisionSourceNode',
                       '?Accepts@CollisionSourceNode')
            self.assertFalse(any(s.name.startswith(retired) for s in obj.symbols if s.section_number == 0))


if __name__ == '__main__':
    unittest.main()
