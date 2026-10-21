"""Scene matrices and segment hits use the existing canonical query state."""
from collections import Counter
import hashlib
import os
from pathlib import Path
import re
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
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
CROSS = 'src/reconstructed/CrossProduct.cpp'
VECTOR = 'src/reconstructed/VectorCalls.cpp'
STATE = '?g_CollisionBoxResult@@3PAUQueryHit@@A'
MATRIX = '?GetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z'
SEGMENT = '?SegmentTreeQueryWithVertices@@YAHPBUVec3@@PAUQueryTreeNode@@PBUMatrix4@@PAU1@@Z'
HELPERS = (
    ('Vec3Call::Vec3Call', '??0Vec3Call@@QAE@MMM@Z', 0x404e60, 25),
    ('SegmentBoxOverlap', '?SegmentBoxOverlap@@', 0x4253b0, 924),
    ('PointInTriangle', '?PointInTriangle@@', 0x4278d0, 830),
    ('SegmentTreeQueryWithVertices', SEGMENT, 0x429e60, 33),
    ('SegmentTreeQuery', '?SegmentTreeQuery@@', 0x429e90, 712),
)


def definition(text, name):
    match = re.search(r'^[^\n;{}]*\b' + re.escape(name) + r'\([^;{}]*\)\s*\{', text, re.M)
    assert match, name
    depth, end = 1, match.end()
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class CollisionQueryStateConnectionsTests(unittest.TestCase):
    def test_shared_pointer_and_actual_segment_hit_producer(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        address = 0x579058
        section = retail.section_for_rva(address - retail.image_base)
        self.assertTrue(section.characteristics & 0x80000000)
        offset = address - retail.image_base - section.virtual_address
        self.assertLessEqual(offset + 4, section.virtual_size)
        backed = min(4, max(0, section.raw_size - offset))
        self.assertEqual((retail.bytes_at_va(address, backed) if backed else b'') + bytes(4-backed), bytes(4))
        with tempfile.TemporaryDirectory(prefix='mcm2-query-state-') as directory:
            output = Path(directory)
            query_source = output / 'query_producer.cpp'
            text = (ROOT / QUERY).read_text()
            # These are the same source definitions, including the actual shared
            # pointer type. They are test-only extracted objects, not production
            # storage replacements or copied retail instructions.
            query_source.write_text('#include "bvh/BoundingBoxTreeQuery.h"\n' +
                'static Vec3* s_vertices;\nQueryHit* g_CollisionBoxResult;\n' +
                '\n'.join(definition(text, row[0]) for row in HELPERS) + '\n')
            setter_source = output / 'collision_setter.cpp'
            setter_source.write_text('#include "collision/CollisionShapeTests.h"\n' +
                definition((ROOT / COLLISION).read_text(), 'SetCollisionBoxResult') + '\n')
            fixture = 'tests/fixtures/collision_query_state.cpp'
            objects, paths = {}, {}
            for index, source in enumerate((COLLISION, QUERY, NODE, CROSS, VECTOR,
                                             str(query_source), str(setter_source), fixture)):
                path = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            state = objects[QUERY].find_symbol(STATE)
            storage = next(s for s in objects[QUERY].sections if s.index == state.section_number)
            self.assertEqual(storage.name, '.bss')
            self.assertEqual(storage.raw_ptr, 0)
            self.assertTrue(storage.characteristics & 0x80000000)
            self.assertLessEqual(state.value + 4, storage.raw_size)
            self.assertEqual(sum(s.name == STATE and s.section_number > 0 for s in objects[QUERY].symbols), 1)
            imported = Counter(objects[COLLISION].symbol_by_index[r.symbol_index].name
                               for r in objects[COLLISION].relocations if r.type == 0x14)
            self.assertEqual(imported[MATRIX], 4)
            self.assertEqual(imported[SEGMENT], 2)
            self.assertFalse(any('GetMatrixIn@CollisionSceneNode' in s.name or
                                 s.name == '?g_CollisionBoxResult@@3PAUCollisionBoxResult@@A'
                                 for s in objects[COLLISION].symbols))
            # Both scene calls remain complete at their existing extents; the
            # SetTransform and segment-query callers are also covered by the
            # separate whole-partial fingerprints in test_collision_query_connections.
            checks = ((NODE, MATRIX, 0x4fca80, 494),
                      (COLLISION, 'SyncTransformFromNode', 0x435f10, 160),
                      (COLLISION, 'SetCollisionBoxResult', 0x439e00, 10))
            for source, symbol, va, size in checks:
                entry = objects[source].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            for _, symbol, va, size in HELPERS:
                for source in (QUERY, str(query_source)):
                    entry = objects[source].find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, objects[source], symbol, va, size,
                        (ROOT / QUERY).with_suffix('.bindings.json'))['strict_exact'], (source, symbol))
            self.assertTrue(compare(retail, objects[str(setter_source)], 'SetCollisionBoxResult', 0x439e00, 10,
                                   (ROOT / COLLISION).with_suffix('.bindings.json'))['strict_exact'])
            _, _, relocs = objects[str(setter_source)].symbol_extent(objects[str(setter_source)].find_symbol('SetCollisionBoxResult'))
            self.assertEqual([objects[str(setter_source)].symbol_by_index[r.symbol_index].name for r in relocs], [STATE])
            decoded = list(decoder.disasm(retail.bytes_at_va(0x439e00, 10), 0x439e00))
            writes = [i.operands[0] for i in decoded if i.mnemonic == 'mov' and
                      i.operands[0].type == capstone.x86.X86_OP_MEM and i.operands[0].mem.disp == address]
            self.assertEqual(len(writes), 1)
            self.assertEqual(writes[0].size, 4)
            self.assertTrue(compare(retail, objects[CROSS], '?CrossProductCall@@', 0x515600, 91,
                                   ROOT / 'src/reconstructed/VectorCalls.bindings.json')['strict_exact'])
            self.assertTrue(compare(retail, objects[VECTOR], '?Vec3DotCall@@', 0x40ae30, 29,
                                   ROOT / 'src/reconstructed/VectorCalls.bindings.json')['strict_exact'])
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'query_state.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *(str(paths[s]) for s in (str(query_source), str(setter_source), CROSS, VECTOR, fixture)),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Collision query state: 56448 native producer/consumer cases passed', result.stdout)
            print('Three connections, six dual-strict producer/helpers, actual math providers; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
