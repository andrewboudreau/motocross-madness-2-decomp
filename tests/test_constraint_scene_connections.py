"""Constraint scene calls preserve whole callers and use actual node providers."""
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
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_garage_shadow_connections import function_text

ROOT = Path(__file__).resolve().parents[1]
CALLER = 'src/krusty2/constraint/ConstraintMethodCollisionModel.cpp'
SAMPLE = 'samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
MATRIX = 'src/krusty2/collision/CollisionObject.cpp'
BIKE = 'src/krusty2/vehicle/Bike.cpp'
WRECKER = 'src/reconstructed/Wrecker.cpp'
BODIES = (
    ('void SoultreeObject::UpdateWorldMatrix(', '?UpdateWorldMatrix@SoultreeObject@@QAEXXZ', 0x4fb4f0, 858),
    ('void SoultreeObject::InvalidateWorldMatrix(', '?InvalidateWorldMatrix@SoultreeObject@@QAEXXZ', 0x4fb850, 45),
    ('void SoultreeObject::InvalidateSiblingChain(', '?InvalidateSiblingChain@SoultreeObject@@QAEXXZ', 0x4fb880, 50),
    ('void SoultreeObject::SetPosition(const Vec3&', '?SetPosition@SoultreeObject@@QAEXABUVec3@@@Z', 0x4fc660, 38),
    ('void SoultreeObject::SetPositionInFrame(', '?SetPositionInFrame@SoultreeObject@@QAEXPAV1@PBUVec3@@@Z', 0x4fc740, 265),
    ('void SoultreeObject::GetPositionIn(', '?GetPositionIn@SoultreeObject@@QAEXPAV1@PAUVec3@@@Z', 0x4fc9a0, 134),
    ('Vec3 SoultreeObject::LocalToWorldPoint(', '?LocalToWorldPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z', 0x4fd660, 168),
    ('Vec3 SoultreeObject::WorldToLocalPoint(', '?WorldToLocalPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z', 0x4fd7f0, 285),
    ('void SoultreeObject::MarkSubtreeDirty(', '?MarkSubtreeDirty@SoultreeObject@@QAEXXZ', 0x4fdab0, 38))
INLINE = ('static inline void MatrixProduct(', 'static inline void TransposeRotation(',
          'static inline void InvertRigid(', 'static inline Vec3 TransformPoint(')
CONNECTED = (BODIES[4], BODIES[5], BODIES[6])


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class ConstraintSceneConnectionsTests(unittest.TestCase):
    def test_complete_callers_receivers_and_actual_scene_routes(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, offset in (('SoultreeObject', 0), ('GameObject', 12)):
            base = next(b for b in classes['D3DIMSoultreeObject'].bases if b.name == name)
            self.assertEqual((base.mdisp, base.pdisp), (offset, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def decode(va, size):
            return list(decoder.disasm(retail.bytes_at_va(va, size), va))
        instructions = {i.address: (i.mnemonic, i.op_str) for va, size in (
            (0x4cbdf0, 17), (0x43c7f0, 130), (0x532900, 210), (0x5301c0, 995), (0x43ba70, 829))
                        for i in decode(va, size)}
        for va, operation in (
            (0x4cbdf8, ('mov', 'dword ptr [ecx + 0x234], eax')),
            (0x5302c4, ('mov', 'edx, dword ptr [edx + 0x1a0]')),
            (0x5302cc, ('push', 'edx')),
            (0x5302cd, ('call', 'dword ptr [eax + 0x70]')),
            (0x532900, ('mov', 'eax, dword ptr [esp + 0x10]')),
            (0x532909, ('push', 'eax')),
            (0x532924, ('call', '0x43c7f0')),
            (0x43c81a, ('mov', 'ecx, dword ptr [esp + 0x24]')),
            (0x43c82a, ('mov', 'dword ptr [edx + eax + 0xc], ecx')),
            (0x43bada, ('mov', 'ecx, dword ptr [edx + 0x234]')),
            (0x43bb8d, ('mov', 'ecx, dword ptr [eax + 0xc]'))):
            self.assertEqual(instructions[va], operation)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x556e0c + 28*4, 4))[0], 0x4cbdf0)
        bone_calls = Counter(i.operands[0].imm for i in decode(0x409420, 1516)
                             if i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM)
        self.assertEqual((bone_calls[0x4fdae0], bone_calls[0x532900]), (15, 15))
        with tempfile.TemporaryDirectory(prefix='mcm2-constraint-scene-') as directory:
            output = Path(directory)
            text = (ROOT / TREE).read_text()
            signatures = sorted((*INLINE, *(r[0] for r in BODIES)), key=text.index)
            node = output / 'node.cpp'
            node.write_text('#include <string.h>\n#include "math/Math3D.h"\n'
                'void CollisionMatrixMultiply(Matrix4*,const Matrix4*,const Matrix4*);\n' +
                '\n'.join(function_text(text, signature) for signature in signatures) + '\n')
            matrix = output / 'matrix.cpp'
            matrix.write_text('#include "math/Math3D.h"\n' + function_text((ROOT / MATRIX).read_text(),
                              'void CollisionMatrixMultiply('))
            fixture = 'tests/fixtures/constraint_scene_routes.cpp'
            objects, paths = {}, {}
            for index, source in enumerate((CALLER, SAMPLE, TREE, MATRIX, BIKE, WRECKER, str(node), str(matrix), fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            for _, symbol, va, size in BODIES:
                for source in (TREE, str(node)):
                    self.assertEqual(objects[source].symbol_extent(objects[source].find_symbol(symbol))[1], size)
                    self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                           (ROOT / TREE).with_suffix('.bindings.json'))['strict_exact'], (source, symbol))
            for source in (MATRIX, str(matrix)):
                self.assertTrue(compare(retail, objects[source], '?CollisionMatrixMultiply@@', 0x436500, 532,
                                       (ROOT / MATRIX).with_suffix('.bindings.json'))['strict_exact'])
            strict = 0
            manifest = ROOT / 'src/krusty2/constraint/targets.json'
            for target in json.loads(manifest.read_text()):
                for source in (CALLER, SAMPLE):
                    binding = (ROOT / source).with_suffix('.bindings.json')
                    self.assertTrue(compare(retail, objects[source], target['candidate_symbol_contains'],
                        int(target['target_va'], 0), target['target_size'], binding)['strict_exact'], (source, target))
                    strict += 1
            for source, symbol, va, size in ((BIKE, '?UnknownVirtualSlot97@Bike@@UAEXXZ', 0x409420, 1516),
                (WRECKER, '?CreateBodies@Wrecker@@QAEXPAVD3DIMSoultreeCharacter@@PBD@Z', 0x5301c0, 995),
                (WRECKER, '?AddProbe@Wrecker@@QAEXUVector3@@H@Z', 0x532900, 210)):
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                    (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                strict += 1
            original = Counter(i.operands[0].imm for i in decode(0x43ba70, 829)
                               if i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM)
            for source in (CALLER, SAMPLE):
                obj = objects[source]
                _, _, relocs = obj.symbol_extent(obj.find_symbol('UnknownVirtualSlot11@ConstraintMethodCollisionModel'))
                actual = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs if r.type == 0x14)
                for _, symbol, va, _ in CONNECTED:
                    self.assertEqual(actual[symbol], 1 if va == 0x4fd660 else 2)
                    self.assertEqual(actual[symbol], original[va])
            imports = {s.name for s in objects[CALLER].symbols if s.section_number == 0}
            self.assertFalse(any('@ConNode@@' in name for name in imports))
            binding = (ROOT / SAMPLE).with_suffix('.bindings.json')
            symbol = '?ApplyContactImpulse@ConstraintMethodCollisionModel@@QAEXPAVConBody@@MUConVec3@@11@Z'
            result = compare(retail, objects[SAMPLE], symbol, 0x43bdb0, 2619, binding)
            self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (2619, 2515))
            with patch('mcm2tool.resolved_match.compare_bytes',
                       lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                resolved = match_object(objects[SAMPLE], symbol, 0x43bdb0, b'',
                    {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
            self.assertEqual(resolved['digest'], 'e327806177915bc7daeae9317786dbc3412127b9be00c3474b12a7c0e2629b06')
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'scene.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[s]) for s in (str(node), str(matrix), fixture)), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Constraint scene routes: 32768 actual getter, placement, point, cache and alias cases passed')
            print('%d strict caller/producer bodies, 20 dual-strict helper checks, one complete partial; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
