"""Garage shadow calls preserve complete bodies and use existing providers."""
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
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/KrustyUI.cpp', 'samples/ui/KrustyUINearMisses.cpp',
           'src/krusty2/shadow/ProjectedShadow.cpp', 'src/krusty2/shadow/D3DIMSoultreeShadow.cpp',
           'src/reconstructed/GameObject.cpp')
ROUTES = (
    ('??0ProjectedShadow@@QAE@H@Z', 0x4da570, 331, SOURCES[2]),
    ('??0D3DIMSoultreeShadow@@QAE@H@Z', 0x446840, 67, SOURCES[3]),
    ('?Attach@D3DIMSoultreeShadow@@', 0x4468b0, 58, SOURCES[3]),
    ('?AddCaster@ProjectedShadow@@', 0x4dab00, 135, SOURCES[2]),
    ('?SetShadowStrength@ProjectedShadow@@', 0x4dace0, 336, SOURCES[2]),
    ('?SetShadowGrey@ProjectedShadow@@', 0x4dae30, 31, SOURCES[2]),
    ('?SetLight@ProjectedShadow@@', 0x4dae50, 365, SOURCES[2]))
NATIVE = (
    ('void ProjectedShadow::AddCaster(', '?AddCaster@ProjectedShadow@@', 0x4dab00, 135, SOURCES[2]),
    ('void ProjectedShadow::AddReceiver(', '?AddReceiver@ProjectedShadow@@', 0x4daba0, 167, SOURCES[2]),
    ('void ProjectedShadow::ResizeVertexBuffer(', '?ResizeVertexBuffer@ProjectedShadow@@', 0x4dac50, 138, SOURCES[2]),
    ('void ProjectedShadow::SetShadowStrength(', '?SetShadowStrength@ProjectedShadow@@', 0x4dace0, 336, SOURCES[2]),
    ('void ProjectedShadow::SetShadowGrey(', '?SetShadowGrey@ProjectedShadow@@', 0x4dae30, 31, SOURCES[2]),
    ('int ProjectedShadow::SetLight(', '?SetLight@ProjectedShadow@@', 0x4dae50, 365, SOURCES[2]),
    ('D3DIMSoultreeShadow* D3DIMSoultreeShadow::Attach(', '?Attach@D3DIMSoultreeShadow@@', 0x4468b0, 58, SOURCES[3]),
    ('GameObject* GameObject::SetRenderContext(', '?SetRenderContext@GameObject@@', 0x4692f0, 12, SOURCES[4]))


def function_text(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class GarageShadowConnectionsTests(unittest.TestCase):
    def test_complete_providers_callers_and_native_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        # Both actual objects have their GameObject primary base at zero.
        for vtable, name, required in ((0x5575c8, b'.?AVProjectedShadow@@', (b'.?AVGameObject@@',)),
                (0x55156c, b'.?AVD3DIMSoultreeShadow@@', (b'.?AVShadowReceiver@@', b'.?AVGameObject@@'))):
            col, = struct.unpack('<I', retail.bytes_at_va(vtable - 4, 4))
            _, offset, _, td, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
            self.assertEqual(offset, 0)
            self.assertEqual(retail.bytes_at_va(td + 8, 40).split(b'\0')[0], name)
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            bases = {}
            for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
                td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                bases[retail.bytes_at_va(td + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
            for base in required:
                self.assertEqual(bases[base], (0, -1, 0))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(retail.bytes_at_va(0x498cf0, 3114), 0x498cf0))
        self.assertEqual(sum(i.size for i in code), 3114)
        calls = [i.operands[0].imm for i in code if i.mnemonic == 'call'
                 and i.operands[0].type == capstone.x86.X86_OP_IMM]
        for _, address, _, _ in ROUTES:
            self.assertEqual(calls.count(address), 2 if address == 0x4dab00 else 1)
        for allocation in (0x134, 0x48):
            self.assertIn(allocation, [i.operands[0].imm for i in code if i.mnemonic == 'push'
                                      and i.operands[0].type == capstone.x86.X86_OP_IMM])
        with tempfile.TemporaryDirectory(prefix='mcm2-garage-shadow-') as directory:
            output = Path(directory)
            extracted = output / 'shadow_routes.cpp'
            texts = {source: (ROOT / source).read_text() for source in SOURCES}
            text = texts[SOURCES[2]]
            vector = text[text.index('struct ShadowConstVec3'):text.index('};', text.index('struct ShadowConstVec3')) + 2]
            extracted.write_text('#include "shadow/D3DIMSoultreeShadow.h"\n' + vector + '\n' +
                function_text(text, 'static inline ShadowVec3 ShadowNormalize(') + '\n' + '\n\n'.join(
                    function_text(texts[source], signature) for signature, _, _, _, source in NATIVE))
            fixture = 'tests/fixtures/garage_shadow_routes.cpp'
            objects, paths = {}, {}
            for source in (*SOURCES, str(extracted), fixture):
                path = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source in (*SOURCES[2:4], str(extracted), fixture):
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            for symbol, address, size, source in ROUTES:
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                self.assertTrue(compare(retail, obj, symbol, address, size,
                                       (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
            imports = {s.name for s in objects[SOURCES[0]].symbols if s.section_number == 0}
            for symbol, _, _, source in ROUTES:
                self.assertIn(objects[source].find_symbol(symbol).name, imports)
            self.assertEqual([s for s in imports if 'UnknownKrustyUIShadow' in s],
                ['?Init@UnknownKrustyUIShadow@@QAEPAV1@PAXPAVTextureMapManager@@@Z'])
            # Test extraction has no substituted function statements. Only its
            # compiler-generated __FILE__ literal relocation has a new name.
            bindings = {}
            for source in SOURCES[2:]:
                bindings.update(json.loads((ROOT / source).with_suffix('.bindings.json').read_text()))
            bindings['__FILE__'] = '0x00571fd8'
            extracted_object = objects[str(extracted)]
            for symbol in extracted_object.symbols:
                if symbol.section_number > 0 and symbol.storage_class == 2:
                    sec = extracted_object.section(symbol.section_number)
                    raw = extracted_object.data[sec.raw_ptr + symbol.value:sec.raw_ptr + sec.raw_size]
                    if raw.split(b'\0')[0].endswith(b'shadow_routes.cpp'):
                        bindings[symbol.name] = '0x00571fd8'
            extracted_bindings = output / 'shadow_routes.bindings.json'
            extracted_bindings.write_text(json.dumps(bindings))
            for _, symbol, address, size, source in NATIVE:
                for obj, binding in ((objects[source], (ROOT / source).with_suffix('.bindings.json')),
                                     (extracted_object, extracted_bindings)):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                    self.assertTrue(compare(retail, obj, symbol, address, size, binding)['strict_exact'], symbol)
            for symbol in ('_ShadowPrimary', '_ReceiverPrimary'):
                body, size, relocs = objects[fixture].symbol_extent(objects[fixture].find_symbol(symbol))
                self.assertEqual((body, size, relocs), (bytes.fromhex('8b442404c3'), 5, []))
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for target in json.loads((ROOT / 'src/krusty2/shadow/targets.json').read_text()):
                source = 'src/krusty2/shadow/' + target['source']
                if source in SOURCES:
                    cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                  target['target_size'], ROOT / 'src/krusty2/shadow' / target['bindings']))
            strict = 0
            for source, symbol, address, size, binding in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, address, size, binding)['strict_exact'],
                                (source, symbol, address))
                strict += 1
            # This admitted nonexact caller is unchanged by the boundary edits.
            binding = (ROOT / SOURCES[1]).with_suffix('.bindings.json')
            result = compare(retail, objects[SOURCES[1]], '?Create@KrustyUI@@', 0x4988a0, 1104, binding)
            self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (1102, 872))
            with patch('mcm2tool.resolved_match.compare_bytes',
                       lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                resolved = match_object(objects[SOURCES[1]], '?Create@KrustyUI@@', 0x4988a0, b'',
                    {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
            self.assertEqual(resolved['digest'], 'a0c7fe2a846eb24d4bf7dfdab6fcbcff2e503af4e3ab5456d11aab1c01f7bb99')
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'shadow_routes.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Garage shadow: 3776 receiver, caster, color and light boundary cases passed', result.stdout)
            print('%d strict caller/provider cases, 16 dual-strict native body checks, one complete partial; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
