"""Complete channel comparison, including the cold zero-height branch."""
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
SYMBOL = '?MeanTextureChannelDifference@@YAMPAVTextureMap@@0@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class TextureChannelDifferenceBehaviorTests(unittest.TestCase):
    def test_complete_cfg_pixels_pitches_and_lock_callbacks(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        start, size = 0x4cb330, 629
        body = retail.bytes_at_va(start, size)
        reached, pending = {}, [start]
        while pending:
            at = pending.pop()
            if at in reached:
                continue
            self.assertTrue(start <= at < start + size)
            i = next(decoder.disasm(body[at-start:], at, count=1))
            reached[at] = i
            if i.group(capstone.CS_GRP_RET):
                continue
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                pending.append(i.operands[0].imm)
                if i.mnemonic == 'jmp':
                    continue
            pending.append(at+i.size)
        self.assertEqual(sorted(b for at,i in reached.items() for b in range(at,at+i.size)),
                         list(range(start,start+size)))
        self.assertIn(0x4cb5a3,reached)
        self.assertEqual(retail.bytes_at_va(start+size,11), b'\x90'*11)
        calls = [i for i in reached.values() if i.mnemonic == 'call']
        self.assertEqual(len(calls),5)
        self.assertTrue(all(i.operands[0].type == capstone.x86.X86_OP_MEM for i in calls))
        self.assertCountEqual([i.operands[0].mem.disp for i in calls],[0x34,0x34,0x38,0x38,0x38])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-channel-difference-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/render/TextureChannelDifference.cpp', 'tests/fixtures/texture_channel_difference_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    self.assertEqual({s.name for s in candidate.symbols
                                      if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 620)
                    report = compare(retail, candidate, SYMBOL, 0x4cb330, 629,
                                     ROOT / 'samples/render/TextureChannelDifference.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 35)
            executable = output / 'channels.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('TextureChannelDifference: 144944 pixel, pitch, failure and callback scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
