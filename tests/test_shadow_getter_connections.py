"""Shadow matrix/bounds calls use complete scene providers without changing partials."""
import hashlib
import json
import os
from pathlib import Path
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
SHADOW = 'src/krusty2/shadow/D3DIMSoultreeShadow.cpp'
SAMPLE = 'samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
BODIES = (
    ('void SoultreeObject::UpdateWorldMatrix(', '?UpdateWorldMatrix@SoultreeObject@@QAEXXZ', 0x4fb4f0, 858),
    ('void SoultreeObject::GetMatrixIn(', '?GetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z', 0x4fca80, 494),
    ('Vec3 SoultreeObject::LocalToWorldDirection(', '?LocalToWorldDirection@SoultreeObject@@QAE?AUVec3@@ABU2@@Z', 0x4fd5c0, 150),
    ('Vec3 SoultreeObject::LocalToWorldPoint(', '?LocalToWorldPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z', 0x4fd660, 168),
    ('void SoultreeObject::UpdateSubtreeBounds(', '?UpdateSubtreeBounds@SoultreeObject@@QAEXXZ', 0x4fe0f0, 483),
    ('SoultreeOolVec3::SoultreeOolVec3(', '??0SoultreeOolVec3@@QAE@MMM@Z', 0x404e60, 25),
    ('void SoultreeObject::AccumulateBounds(', '?AccumulateBounds@SoultreeObject@@QAEXPAHPAUVec3@@1PBUMatrix4@@@Z', 0x4fe2e0, 1390),
    ('void SoultreeObject::GetSubtreeBounds(', '?GetSubtreeBounds@SoultreeObject@@QAEXPAUVec3@@0@Z', 0x4fe850, 74))
INLINE = ('static inline void SetIdentity(', 'static inline void MatrixProduct(',
          'static inline void TransposeRotation(', 'static inline void RotateByRowsTo(',
          'static inline float AbsValue(', 'static inline float MinValue(', 'static inline float MaxValue(',
          'static inline void FramePointTo(')
PARTIALS = (
    ('?UnknownVirtualSlot29@D3DIMSoultreeShadow@@UAEHXZ', 0x446c30, 771, 771, 730,
     '4cfddc481899ea9c02093fb09b3035df78ca2734ee7453dcab19a90dabaf5cc9'),
    ('?UnknownVirtualSlot30@D3DIMSoultreeShadow@@UAEXXZ', 0x446f40, 1524, 1485, 180,
     '8297865bb1d75b1b539f4da4ba0c3087d581b2e32eb1b67ee6f5575d4b111e22'))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class ShadowGetterConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_and_native_scene_closure(self):
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
        def instructions(va, size):
            return {i.address: (i.mnemonic, i.op_str) for i in decoder.disasm(retail.bytes_at_va(va, size), va)}
        garage = instructions(0x498cf0, 3114)
        self.assertEqual(garage[0x499504], ('mov', 'ecx, dword ptr [edx + 0x1a0]'))
        self.assertEqual(garage[0x49950d], ('push', 'ecx'))
        self.assertEqual(garage[0x499511], ('call', '0x4468b0'))
        self.assertEqual(instructions(0x4468b0, 58)[0x4468db], ('mov', 'dword ptr [esi + 0x2c], ecx'))
        for va, size, first, second in ((0x4468f0, 767, 0x44691f, 0x446937),
                                       (0x446c30, 771, 0x446c63, 0x446c7b)):
            calls = instructions(va, size)
            self.assertEqual(calls[first], ('call', '0x4fca80'))
            self.assertEqual(calls[second], ('call', '0x4fe850'))
        with tempfile.TemporaryDirectory(prefix='mcm2-shadow-getters-') as directory:
            output = Path(directory)
            text = (ROOT / TREE).read_text()
            node = output / 'node.cpp'
            start = text.index('struct SoultreeOolVec3 :')
            end = text.index('\n};', start)+3
            node.write_text('#include <string.h>\n#include "math/Math3D.h"\n'
                'void CollisionMatrixMultiply(Matrix4*,const Matrix4*,const Matrix4*);\n' +
                '\n'.join(function_text(text, signature) for signature in INLINE) + '\n' +
                text[start:end] + '\n' + '\n'.join(function_text(text, signature) for signature, _, _, _ in BODIES))
            matrix = output / 'matrix.cpp'
            matrix.write_text('#include "math/Math3D.h"\n' + function_text((ROOT / COLLISION).read_text(),
                              'void CollisionMatrixMultiply('))
            layout = output / 'layout.cpp'
            caller_text = (ROOT / SHADOW).read_text()
            layout.write_text('#include <stddef.h>\n#include "math/Vec3.h"\n'
                '#include "../reconstructed/Matrix4.h"\n#include "core/SoultreeObject.h"\n'
                'struct ShadowVec3 { float x,y,z; };\n' +
                function_text(caller_text, 'struct D3DIMShadowMatrix4 {') + ';\n' +
                function_text(caller_text, 'class D3DIMShadowCasterView {') + ';\n' +
                'typedef char MatrixSize[(sizeof(D3DIMShadowMatrix4)==sizeof(Matrix4) && sizeof(Matrix4)==64)?1:-1];\n'
                'typedef char Translation[(offsetof(D3DIMShadowMatrix4,_41)==offsetof(Matrix4,_41) && offsetof(Matrix4,_41)==48)?1:-1];\n'
                'typedef char LastComponent[(offsetof(D3DIMShadowMatrix4,_44)==offsetof(Matrix4,_44) && offsetof(Matrix4,_44)==60)?1:-1];\n'
                'typedef char SkipField[(offsetof(D3DIMShadowCasterView,field_0x14c)==offsetof(SoultreeObject,field_0x14c))?1:-1];\n')
            fixture = 'tests/fixtures/shadow_getter_routes.cpp'
            sources = (SHADOW, SAMPLE, TREE, COLLISION)
            paths, objects = {}, {}
            for index, source in enumerate((*sources, str(node), str(matrix), str(layout), fixture)):
                path = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source], objects[source] = path, CoffObject(path)
            for _, symbol, va, size in BODIES:
                for source in (TREE, str(node)):
                    obj = objects[source]
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size, (source, symbol))
                    self.assertTrue(compare(retail, obj, symbol, va, size,
                                           (ROOT / TREE).with_suffix('.bindings.json'))['strict_exact'], (source, symbol))
            for source in (COLLISION, str(matrix)):
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol('?CollisionMatrixMultiply@@'))[1], 532)
                self.assertTrue(compare(retail, obj, '?CollisionMatrixMultiply@@', 0x436500, 532,
                                       (ROOT / COLLISION).with_suffix('.bindings.json'))['strict_exact'])
            for source in (SHADOW, SAMPLE):
                imports = {s.name for s in objects[source].symbols if s.section_number == 0}
                self.assertIn(BODIES[1][1], imports)
                self.assertIn(BODIES[7][1], imports)
                self.assertFalse(any('GetMatrixIn@D3DIMShadowCasterView' in s or 'GetBounds@D3DIMShadowCasterView' in s for s in imports))
            strict = 0
            for area, source in (('shadow', SHADOW), ('soultree', TREE)):
                manifest = ROOT / 'src/krusty2' / area / 'targets.json'
                for target in json.loads(manifest.read_text()):
                    if (manifest.parent / target['source']).resolve() != ROOT / source:
                        continue
                    self.assertTrue(compare(retail, objects[source], target['candidate_symbol_contains'],
                        int(target['target_va'], 0), target['target_size'], manifest.parent / target['bindings'])['strict_exact'], target)
                    strict += 1
            binding = (ROOT / SAMPLE).with_suffix('.bindings.json')
            for symbol, va, size, emitted, matching, digest in PARTIALS:
                result = compare(retail, objects[SAMPLE], symbol, va, size, binding)
                self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (emitted, matching))
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(objects[SAMPLE], symbol, va, b'',
                        {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
                self.assertEqual(resolved['digest'], digest)
            compiler = Path(os.environ['VC6_ROOT'])
            exe = output / 'shadow_getters.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo', '/subsystem:console',
                f'/out:{exe}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[s]) for s in (str(node), str(matrix), fixture)), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Shadow scene getters: 6144 real matrix, bounds, cache and alias cases passed', result.stdout)
            print('%d strict cases, 18 dual-strict getter/helper checks, two full partial fingerprints; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
