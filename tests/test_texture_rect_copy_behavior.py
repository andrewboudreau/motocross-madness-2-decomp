"""Complete texture rectangle copying, including the real low-address fault."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?CopyRectTo@PCTextureMap@@QAEHKKPAUUnknownSurfaceInterface@@PAUUnknownRect@@HPAE@Z'
INDIRECT = {0x4c7b87: 'dword ptr [ecx + 0x1c]', 0x4c7bd5: 'dword ptr [ecx + 0x64]',
            0x4c7c07: 'dword ptr [ecx + 0x64]', 0x4c7dfb: 'dword ptr [ecx + 0x80]',
            0x4c7e13: 'dword ptr [ecx + 0x80]'}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class TextureRectCopyBehaviorTests(unittest.TestCase):
    def test_complete_copy_pixels_descriptors_and_fault_contract(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4c7b40, 750), 0x4c7b40))
        self.assertEqual(sum(i.size for i in instructions), 750)
        self.assertEqual([(i.address, bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'],
                         [(0x4c7b9f, b'\xc2\x18\0'), (0x4c7e2b, b'\xc2\x18\0')])
        self.assertEqual(image.bytes_at_va(0x4c7e2e, 2), b'\x90\x90')
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        boundaries = {i.address for i in instructions}
        indirect = {}
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(instruction.operands[0].imm, boundaries)
            if instruction.mnemonic == 'call':
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_MEM)
                indirect[instruction.address] = instruction.op_str
        self.assertEqual(indirect, INDIRECT)
        self.assertEqual(image.bytes_at_va(0x4c7d8f, 8), b'\x33\xf6\x33\xd2\x66\x8b\x30\x66')
        self.assertEqual(image.bytes_at_va(0x4c7d9f, 2), b'\x8a\x1e')
        caller = list(decoder.disasm(image.bytes_at_va(0x404700, 719), 0x404700))
        self.assertEqual(sum(i.size for i in caller), 719)
        self.assertEqual([(i.address, i.operands[0].imm) for i in caller if i.mnemonic == 'call'
            and i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == 0x4c7b40],
            [(site, 0x4c7b40) for site in (0x40474d, 0x40476e, 0x4048a9, 0x4048c7, 0x404918, 0x404974)])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-rect-copy-') as directory:
            output = Path(directory)

            def compile_source(source, name, fixture=False):
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'), *(['--extra', '/Od'] if fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source(ROOT / 'samples/render/TextureRectCopy.cpp', 'candidate')
            fixture_path, _ = compile_source(ROOT / 'tests/fixtures/texture_rect_copy_compare.cpp', 'fixture', True)
            self.assertEqual(candidate.find_symbol(SYMBOL).function_size, 775)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0}, set())
            self.assertFalse(any(s.name.startswith(('.CRT', '.text$x', '.xdata')) for s in candidate.sections))
            body, size, relocations = candidate.symbol_extent(candidate.find_symbol(SYMBOL))
            self.assertFalse(relocations)
            report = compare(image, candidate, SYMBOL, 0x4c7b40, 750, ROOT / 'samples/render/TextureRectCopy.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 364)
            _, aggregate = compile_source(ROOT / 'samples/render/PCTextureMapNearMisses.cpp', 'aggregate')
            self.assertEqual(aggregate.symbol_extent(aggregate.find_symbol(SYMBOL))[:2], (body, size))
            self.assertFalse(aggregate.symbol_extent(aggregate.find_symbol(SYMBOL))[2])
            _, background = compile_source(ROOT / 'src/reconstructed/BackgroundImage.cpp', 'background')
            caller_symbol = '?DrawImageRect@BackgroundImage@@QAEHPAVPCTextureMap@@HHPAUCameraRect@@HHHPAHPAE@Z'
            self.assertEqual(background.symbol_extent(background.find_symbol(caller_symbol))[1], 719)
            self.assertTrue(compare(image, background, caller_symbol, 0x404700, 719,
                ROOT / 'src/reconstructed/BackgroundImage.bindings.json')['strict_exact'])
            layout = output / 'layout.cpp'
            layout.write_text('#include "PCTextureMap.h"\n#include "RenderInterfaces.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UnknownSurfaceDesc)==124 && offsetof(UnknownSurfaceDesc,pitch)==0x10 && '
                'offsetof(UnknownSurfaceDesc,surface)==0x24 && offsetof(UnknownSurfaceDesc,pixelFormat)==0x48 && '
                'offsetof(UnknownPixelFormat,bitCount)==0xc && offsetof(PCTextureMap,systemSurface)==0x70) ? 1 : -1];\n')
            compile_source(layout, 'layout')
            executable = output / 'copy.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('TextureRectCopy: 72899 complete pixel, clipping, COM, alias, low-address and fault scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
