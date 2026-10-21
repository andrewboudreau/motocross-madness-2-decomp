"""Three existing provider joins preserve whole callers and execute real helper closure."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_collision_point_consumers import definition
from test_rider_motion_start_connections import view
def signature(obj, query):
    entry = obj.find_symbol(query)
    body, size, relocs = obj.symbol_extent(entry)
    targets = []
    for relocation in relocs:
        target = obj.symbol_by_index[relocation.symbol_index]
        name = target.name
        if target.section_number > 0 and target.storage_class in (3, 6) and name.startswith(('$L', '$T')):
            # BoxOverlap switch-table labels remain inside its complete text extent.
            assert target.section_number == entry.section_number
            assert entry.value <= target.value < entry.value + size
            name = ('local', entry.name, target.value - entry.value)
        targets.append((relocation.virtual_address - entry.value, relocation.type, name))
    return body, size, targets


ROOT = Path(__file__).resolve().parents[1]
BIKE = 'src/krusty2/vehicle/KrustyBike.cpp'
SAMPLE = 'samples/physics/krustybike/KrustyBikeNearMisses.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
OVERLAY = 'src/reconstructed/DebugOverlay.cpp'
GROUPS = {
    'collision': (COLLISION, '#include "collision/CollisionShapeTests.h"\n',
                  [('CollisionObject::TestMeshBounds', '?TestMeshBounds@CollisionObject', 0x4392c0, 311)]),
    'overlay': (OVERLAY, '#include "DebugOverlay.h"\n#include <stdarg.h>\n#include <stdio.h>\n#include <string.h>\n',
                [('DebugOverlay::SetRowText', '?SetRowText@DebugOverlay', 0x447e90, 173),
                 ('DebugOverlay::Print', '?Print@DebugOverlay', 0x447f40, 94),
                 ('DebugOverlay::Title', '?Title@DebugOverlay', 0x447fa0, 94)]),
    'query': (QUERY, '#include "bvh/BoundingBoxTreeQuery.h"\nstatic Matrix4 s_relativeAB;\nstatic Vec3 s_axes[15];\n',
              [('BoxOverlap', '?BoxOverlap@@', 0x424ab0, 2300),
               ('TreeBoxOverlap', '?TreeBoxOverlap@@', 0x428c20, 395),
               ('MatrixMultiply', '?MatrixMultiply@@', 0x42a1a0, 684)]),
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class KrustyBikeCollisionOverlayTests(unittest.TestCase):
    def test_complete_objects_receivers_and_native_provider_closure(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        word = lambda va: struct.unpack('<I', pe.bytes_at_va(va, 4))[0]
        for table, col, name in ((0x55166c, 0x55b460, 'DebugOverlay'),
                                 (0x5511b8, 0x55aff0, 'CollisionObject'),
                                 (0x558938, 0x55f8e0, 'TrackGame')):
            self.assertEqual(word(table - 4), col)
            self.assertEqual(word(col + 4), 0)
            self.assertEqual(pe.bytes_at_va(word(col + 12) + 8, 64).split(b'\0')[0],
                             ('.?AV' + name + '@@').encode())
        classes = {c.name: c for c in parse_rtti(pe)}
        for owner, base, displacement in [('DebugOverlay', 'GameObject', 0),
                                          ('CollisionObject', 'GameObject', 12),
                                          ('TrackGame', 'Game', 0)]:
            relation = next(b for b in classes[owner].bases if b.name == base)
            self.assertEqual((relation.mdisp, relation.pdisp), (displacement, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoded = {i.address: (i.mnemonic, i.op_str)
                   for va, size in [(0x48fc80, 5384), (0x48da50, 416), (0x4977a0, 1153), (0x467b70, 775)]
                   for i in decoder.disasm(pe.bytes_at_va(va, size), va)}
        for va, expected in {
            0x490717: ('call', '0x431e70'), 0x4907de: ('call', '0x431e70'),
            0x490766: ('mov', 'dword ptr [ebp + 0x1414], eax'),
            0x4907f8: ('mov', 'dword ptr [ebp + 0x1418], eax'),
            0x49087b: ('mov', 'ecx, dword ptr [ebp + 0x1414]'),
            0x490881: ('mov', 'dword ptr [ebp + 0x128], ecx'),
            0x467dbc: ('call', '0x447920'), 0x467ddd: ('call', '0x447de0'),
            0x467de4: ('mov', 'dword ptr [esi + 0x38], eax'),
            0x497ad6: ('mov', 'edx, dword ptr [ecx + 0x38]'),
            0x497ad9: ('push', 'edx'),
        }.items():
            self.assertEqual(decoded[va], expected)
        sites = {0x48dab6: 0x4392c0, 0x48daf3: 0x4392c0, 0x497a0f: 0x4392c0,
                 0x497ada: 0x447fa0, 0x497af5: 0x447f40, 0x497b47: 0x447f40,
                 0x497be3: 0x447f40, 0x497c0b: 0x447f40}
        for va, target in sites.items():
            self.assertEqual(decoded[va], ('call', hex(target)))
        pins = json.loads((ROOT/'tests/fixtures/krustybike_collision_overlay_objects.json').read_text())
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-boundaries-') as directory:
            out = Path(directory)
            def compile_source(source, name):
                path = out/(name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return path, CoffObject(path)
            objects = {}
            for index, source in enumerate([BIKE, SAMPLE, COLLISION, OVERLAY, QUERY, 'src/reconstructed/Game.cpp']):
                _, obj = compile_source(ROOT/source, 'full%d' % index)
                objects[source] = obj
                if source in pins['objects']:
                    rows, filenames = view(obj, (ROOT,))
                    actual = dict(sections=len(rows), relocations=sum(len(r[3]) for r in rows),
                                  sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())
                    self.assertEqual(actual, pins['objects'][source])
                    self.assertTrue(all(suffix in pins['objects'] for _, suffix in filenames))
            targets = json.loads((ROOT/'src/krusty2/vehicle/targets.json').read_text())
            for t in targets:
                if t['source'] != 'KrustyBike.cpp' or t['expect'] != 'exact': continue
                report = compare(pe, objects[BIKE], t['candidate_symbol_contains'], int(t['target_va'], 0),
                                 t['target_size'], ROOT/'src/krusty2/vehicle'/t['bindings'])
                self.assertTrue(report['strict_exact'], report); strict += 1
            for source, symbol, va, size in [
                (COLLISION, '??0CollisionObject', 0x431e70, 346),
                (OVERLAY, '??0DebugOverlay', 0x447920, 77),
                (OVERLAY, '?Setup@DebugOverlay', 0x447de0, 174),
                ('src/reconstructed/Game.cpp', '?CreateObjects@Game', 0x467b70, 775),
            ]:
                report = compare(pe, objects[source], symbol, va, size,
                                 (ROOT/source).with_suffix('.bindings.json'))
                self.assertTrue(report['strict_exact'], report); strict += 1
            joined = set(pins['renames'].values())
            for symbol, va, expected in [('?UnknownVirtualSlot28@KrustyBike', 0x48da50, 2),
                                         ('?Tick@KrustyBike@@UAEHM@Z', 0x4977a0, 6)]:
                actual = [(va + offset - 1, name) for offset, kind, name in signature(objects[BIKE], symbol)[2]
                          if name in joined]
                self.assertEqual(len(actual), expected)
                for address, name in actual:
                    self.assertIn(address, sites)
                    self.assertEqual(int(json.loads((ROOT/BIKE).with_suffix('.bindings.json').read_text())[name], 0),
                                     sites[address])
            links = []
            for name, (source, header, cases) in GROUPS.items():
                text = (ROOT/source).read_text()
                extracted = out/(name + '.cpp')
                body = header + '\n'.join(definition(text, fn) for fn, _, _, _ in cases) + '\n'
                if name == 'query':
                    body += 'extern "C" void* QueryFrame(){return &s_relativeAB;}\nextern "C" void* QueryAxes(){return s_axes;}\n'
                extracted.write_text(body)
                path, obj = compile_source(extracted, name)
                links.append(path)
                for _, symbol, va, size in cases:
                    for tested in (objects[source], obj):
                        report = compare(pe, tested, symbol, va, size, (ROOT/source).with_suffix('.bindings.json'))
                        self.assertTrue(report['strict_exact'], report); strict += 1
                    self.assertEqual(signature(obj, symbol), signature(objects[source], symbol))
                if name == 'query':
                    # These are real header-emitted helpers, not substituted test implementations.
                    for helper in ('?CrossProduct@@', '?DotProduct@@'):
                        self.assertEqual(signature(obj, helper), signature(objects[QUERY], helper))
                    for tested in (obj, objects[QUERY]):
                        report = compare(pe, tested, '?CrossProduct@@', 0x515600, 91,
                                         (ROOT/QUERY).with_suffix('.bindings.json'))
                        self.assertTrue(report['strict_exact'], report); strict += 1
                    # DotProduct's existing z+y+x order differs from retail y+x+z.
                    # The native collision matrix intentionally uses bounded binary fractions;
                    # this connection does not admit a general floating-point helper correction.
            fixture, _ = compile_source(ROOT/'tests/fixtures/krustybike_collision_overlay.cpp', 'fixture')
            exe = out/'boundaries.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', '/out:' + str(exe), '/libpath:' + str(libraries),
                *map(str, links), str(fixture), 'kernel32.lib'], capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(exe), str(image)], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('22225 collision/overlay scenarios match', result.stdout)
            print('KrustyBike boundaries:', strict, 'strict checks; two whole objects;', result.stdout.strip())


if __name__ == '__main__': unittest.main()
