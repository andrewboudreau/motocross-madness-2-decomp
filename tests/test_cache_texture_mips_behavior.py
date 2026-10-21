"""Complete cache mip-copy candidate with real coordinate/FP helpers and bounded COM chains."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare
ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?CopyMipLevels@CacheTexture@@QAEHPAUUnknownTextureRegion@@H@Z'
DIRECT = {0x5102e9: 0x510610, 0x510315: 0x534a88, 0x51032a: 0x534a88,
          0x510340: 0x534a88, 0x51035c: 0x534a88}
INDIRECT = {0x5103c8: 'dword ptr [ecx + 0x30]', 0x5103ff: 'dword ptr [ecx + 0x14]',
            0x510444: 'dword ptr [eax + 0x30]', 0x510456: 'dword ptr [eax + 0x30]',
            0x5104d3: 'dword ptr [edx + 0x24]'}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class CacheTextureMipsBehaviorTests(unittest.TestCase):
    def test_complete_mip_chain_rectangle_state_and_failure_contract(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x5102d0, 558), 0x5102d0))
        self.assertEqual(sum(i.size for i in instructions), 558)
        self.assertEqual([(i.address, bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'],
                         [(a, b'\xc2\x08\0') for a in (0x5104b1, 0x5104e2, 0x5104fb)])
        self.assertEqual(image.bytes_at_va(0x5104fe, 2), b'\x90\x90')
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        boundaries = {i.address for i in instructions}
        direct, indirect, patches = {}, {}, []
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(instruction.operands[0].imm, boundaries)
            if instruction.mnemonic != 'call':
                continue
            operand = instruction.operands[0]
            if operand.type == capstone.x86.X86_OP_IMM:
                self.assertEqual(instruction.bytes[0], 0xe8)
                direct[instruction.address] = operand.imm
                patches.append(struct.pack('<III', instruction.address, operand.imm, instruction.bytes[0]))
            else:
                indirect[instruction.address] = instruction.op_str
        self.assertEqual(direct, DIRECT)
        self.assertEqual(indirect, INDIRECT)
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x5583d8 + 9 * 4, 4))[0], 0x4c7640)
        col = struct.unpack('<I', image.bytes_at_va(0x5583d4, 4))[0]
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(col + 4, 4))[0], 0)
        self.assertEqual(image.bytes_at_va(0x550748, 4), struct.pack('<f', 1.0))
        caller = list(decoder.disasm(image.bytes_at_va(0x50fdb0, 492), 0x50fdb0))
        self.assertEqual(sum(i.size for i in caller), 492)
        self.assertEqual([(i.address, i.operands[0].imm) for i in caller if i.mnemonic == 'call'
                          and i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == 0x5102d0],
                         [(0x50ff72, 0x5102d0)])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-cache-mips-') as directory:
            output = Path(directory)

            def compile_source(source, name, fixture=False):
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'), *(['--extra', '/Od'] if fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source(ROOT / 'samples/render/CacheTextureNearMisses.cpp', 'candidate')
            fixture_path, _ = compile_source(ROOT / 'tests/fixtures/cache_texture_mips_compare.cpp', 'fixture', True)
            self.assertEqual(candidate.find_symbol(SYMBOL).function_size, 569)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0},
                {'?UpdatePageCoords@ManagedTexture@@QAEXH@Z', '?g_TrackGame@@3PAVTrackGame@@A', '__ftol', '__fltused'})
            self.assertFalse(any(s.name.startswith(('.CRT', '.text$x', '.xdata')) for s in candidate.sections))
            report = compare(image, candidate, SYMBOL, 0x5102d0, 558, ROOT / 'samples/render/CacheTextureNearMisses.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 337)
            # The complete canonical helper body is compiled on an empty
            # derived view solely to give the observing callback a distinct
            # symbol. Its full emitted code/relocations must still be strict.
            canonical = (ROOT / 'src/reconstructed/ManagedTexture.cpp').read_text()
            start = canonical.index('void ManagedTexture::UpdatePageCoords(')
            end, depth = canonical.index('{', start) + 1, 1
            while depth:
                depth += (canonical[end] == '{') - (canonical[end] == '}')
                end += 1
            helper_file = output / 'coordinate_helper.cpp'
            helper_file.write_text('#include "ManagedTexture.h"\n'
                'class ActualManagedTexture : public ManagedTexture {public:void UpdatePageCoords(int);};\n' +
                canonical[start:end].replace('ManagedTexture::', 'ActualManagedTexture::') + '\n')
            helper_path, helper = compile_source(helper_file, 'helper')
            helper_symbol = '?UpdatePageCoords@ActualManagedTexture@@QAEXH@Z'
            helper_binding = output / 'helper.bindings.json'
            binding = json.loads((ROOT / 'src/reconstructed/ManagedTexture.bindings.json').read_text())
            binding[helper_symbol] = '0x00510610'
            helper_binding.write_text(json.dumps(binding))
            self.assertEqual(helper.symbol_extent(helper.find_symbol(helper_symbol))[1], 85)
            self.assertTrue(compare(image, helper, helper_symbol, 0x510610, 85, helper_binding)['strict_exact'])
            _, managed = compile_source(ROOT / 'src/reconstructed/ManagedTexture.cpp', 'managed')
            _, pc = compile_source(ROOT / 'src/reconstructed/PCTextureMap.cpp', 'pc')
            _, cache = compile_source(ROOT / 'src/reconstructed/CacheTexture.cpp', 'cache')
            for provider, name, va, size, owner in [
                (managed, '?UpdatePageCoords@ManagedTexture@@QAEXH@Z', 0x510610, 85, 'ManagedTexture'),
                (pc, '?Upload@PCTextureMap@@UAEHPAUUnknownRect@@H@Z', 0x4c7640, 385, 'PCTextureMap'),
                (cache, '?PlaceTextures@CacheTexture@@QAEHPAV?$ContainerList@PAVManagedTexture@@@@0@Z',
                 0x50fdb0, 492, 'CacheTexture'),
            ]:
                self.assertEqual(provider.symbol_extent(provider.find_symbol(name))[1], size)
                self.assertTrue(compare(image, provider, name, va, size,
                    ROOT / f'src/reconstructed/{owner}.bindings.json')['strict_exact'])
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'mips.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path),
                str(helper_path), 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('CacheTextureMips: 5674 complete mip-chain, rectangle, ownership-boundary, FP and failure scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
