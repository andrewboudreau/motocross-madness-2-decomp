"""Garage scene setter calls retain complete bytes and canonical primary receivers."""
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
from tools.run_calibration import CASES
from test_garage_shadow_connections import function_text

ROOT = Path(__file__).resolve().parents[1]
GARAGE = 'src/reconstructed/KrustyUI.cpp'
CREATE = 'samples/ui/KrustyUINearMisses.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
RENDER = 'src/reconstructed/D3DIMSoulTree.cpp'
CHARACTER = 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp'
MATH = ('src/reconstructed/VectorCalls.cpp', 'src/reconstructed/CrossProduct.cpp', 'src/reconstructed/FastMath.cpp')
BODIES = (
    ('void SoultreeObject::InvalidateWorldMatrix(', '?InvalidateWorldMatrix@SoultreeObject@@QAEXXZ', 0x4fb850, 45),
    ('void SoultreeObject::InvalidateSiblingChain(', '?InvalidateSiblingChain@SoultreeObject@@QAEXXZ', 0x4fb880, 50),
    ('void SoultreeObject::SetAxes(', '?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z', 0x4fbd10, 82),
    ('void SoultreeObject::SetAxesPtr(', '?SetAxesPtr@SoultreeObject@@QAEXPBUVec3@@0HH@Z', 0x4fbd70, 733),
    ('void SoultreeObject::SetPosition(const Vec3&', '?SetPosition@SoultreeObject@@QAEXABUVec3@@@Z', 0x4fc660, 38),
    ('void SoultreeObject::MarkSubtreeDirty(', '?MarkSubtreeDirty@SoultreeObject@@QAEXXZ', 0x4fdab0, 38))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class GarageSetterConnectionsTests(unittest.TestCase):
    def test_complete_callers_primary_layout_and_real_setter_closure(self):
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
        instructions = list(decoder.disasm(retail.bytes_at_va(0x498cf0, 3114), 0x498cf0))
        for target, sites in ((0x4fc660, (0x499414, 0x4995dc, 0x499774)),
                              (0x4fbd10, (0x49943b, 0x499603, 0x499795))):
            self.assertEqual(tuple(i.address for i in instructions if i.mnemonic == 'call'
                                   and i.op_str == hex(target)), sites)
        for site in (0x49943b, 0x499603, 0x499795):
            at = next(n for n, i in enumerate(instructions) if i.address == site)
            window = instructions[at-11:at]
            self.assertEqual([i.op_str for i in window if i.mnemonic == 'push'][-8:],
                             ['1', '0', '0', '0x3f800000', '0', '0xbf800000', '0', '0'])
            self.assertTrue(any(i.mnemonic == 'mov' and i.op_str.endswith('+ 0x1a0]') for i in window))
        # The model producer stores the constructor's primary pointer at +1a0;
        # only the separate GameObject attachment adjusts it by +0xc.
        producer = {i.address: (i.mnemonic, i.op_str) for i in decoder.disasm(
            retail.bytes_at_va(0x445a70, 1021), 0x445a70)}
        self.assertEqual(producer[0x445b29], ('call', '0x43f160'))
        self.assertEqual(producer[0x445b45], ('mov', 'dword ptr [ebp + 0x1a0], eax'))
        self.assertEqual(producer[0x445b6a], ('mov', 'eax, dword ptr [ebp + 0x1a0]'))
        self.assertEqual(producer[0x445b74], ('add', 'eax, 0xc'))
        with tempfile.TemporaryDirectory(prefix='mcm2-garage-setters-') as directory:
            output = Path(directory)
            text = (ROOT / TREE).read_text()
            extracted = output / 'setters.cpp'
            extracted.write_text('#include "math/Math3D.h"\n' +
                function_text(text, 'static inline Vec3 Normalized(') + '\n' + '\n\n'.join(
                    function_text(text, signature) for signature, _, _, _ in BODIES))
            layout = output / 'layout.cpp'
            # Actual provisional producer view from the caller; no invented test layout.
            caller = (ROOT / GARAGE).read_text()
            char_start = caller.index('class UnknownKrustyUICharacter :')
            char_end = caller.index('\n};', char_start)+3
            layout.write_text('#include <stddef.h>\n#include "D3DIMSoulTree.h"\n'
                'struct UnknownKrustyUISoultree;\n' + caller[char_start:char_end] + '\n'
                'typedef char ModelOffset[(offsetof(UnknownKrustyUICharacter,modelNode)==0x1a0)?1:-1];\n'
                'typedef char LocalOffset[(offsetof(SoultreeObject,localMatrix)==0xb8)?1:-1];\n'
                'typedef char ValidOffset[(offsetof(SoultreeObject,worldValid)==0x138)?1:-1];\n'
                'typedef char DirtyOffset[(offsetof(SoultreeObject,subtreeDirty)==0x18c)?1:-1];\n'
                'extern "C" void* GarageModelPrimary(void* p) { return static_cast<SoultreeObject*>(reinterpret_cast<D3DIMSoultreeObject*>(p)); }\n'
                'extern "C" void* GarageGameObject(void* p) { return static_cast<GameObject*>(reinterpret_cast<D3DIMSoultreeObject*>(p)); }\n')
            fixture = 'tests/fixtures/garage_setter_routes.cpp'
            sources = (GARAGE, CREATE, TREE, RENDER, CHARACTER, *MATH)
            objects, paths = {}, {}
            for index, source in enumerate((*sources, str(extracted), str(layout), fixture)):
                obj = output / ('unit%d.obj' % index)
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(obj),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                # Reconstructed callers intentionally compile without a project include
                # override, matching run_calibration rather than hiding header problems.
                if source in (TREE, CHARACTER, *MATH, str(extracted), fixture):
                    command += ['--include', str(ROOT / 'src/krusty2')]
                if source == str(layout):
                    command += ['--include', str(ROOT / 'src/reconstructed')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                        encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            for _, symbol, va, size in BODIES:
                for source in (TREE, str(extracted)):
                    obj = objects[source]
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                    self.assertTrue(compare(retail, obj, symbol, va, size,
                                           (ROOT / TREE).with_suffix('.bindings.json'))['strict_exact'], (source, symbol))
            imports = {s.name for s in objects[GARAGE].symbols if s.section_number == 0}
            self.assertIn(BODIES[2][1], imports)
            self.assertIn(BODIES[4][1], imports)
            self.assertFalse(any('UnknownKrustyUISoultree@@' in name for name in imports))
            self.assertIn('?Init@UnknownKrustyUIShadow@@QAEPAV1@PAXPAVTextureMapManager@@@Z', imports)
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in sources]
            for base in ('src/krusty2', 'samples/physics'):
                for manifest in (ROOT / base).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if not target.get('source') or not target.get('bindings'):
                            continue
                        source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in sources:
                            cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                          target['target_size'], manifest.parent / target['bindings']))
            strict = 0
            for source, symbol, va, size, binding in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, binding)['strict_exact'],
                                (source, symbol, va))
                strict += 1
            binding = (ROOT / CREATE).with_suffix('.bindings.json')
            result = compare(retail, objects[CREATE], '?Create@KrustyUI@@', 0x4988a0, 1104, binding)
            self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (1102, 872))
            with patch('mcm2tool.resolved_match.compare_bytes',
                       lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                resolved = match_object(objects[CREATE], '?Create@KrustyUI@@', 0x4988a0, b'',
                    {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
            self.assertEqual(resolved['digest'], 'a0c7fe2a846eb24d4bf7dfdab6fcbcff2e503af4e3ab5456d11aab1c01f7bb99')
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'garage_setters.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *(str(paths[s]) for s in (str(extracted), str(layout), fixture, *MATH)),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Garage setters: 8192 actual setter, alias, axes and cache cases passed', result.stdout)
            print('%d full strict cases, 12 dual-strict setter/helper checks, one preserved partial; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
