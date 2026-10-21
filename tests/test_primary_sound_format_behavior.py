"""Complete primary audio-format candidate and initialized PCM payload."""
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
SYMBOL = '?SetPrimaryFormat@PCSoundInterface@@QAEHHHH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class PrimarySoundFormatBehaviorTests(unittest.TestCase):
    def test_complete_pcm_state_calls_and_faults(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x4be910, 158), 0x4be910))
        self.assertEqual(sum(i.size for i in instructions), 158)
        self.assertEqual(bytes(instructions[-1].bytes), bytes.fromhex('c20c00'))
        boundaries = {i.address for i in instructions}
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertIn(i.operands[0].imm, boundaries)
        calls = [i for i in instructions if i.mnemonic=='call']
        self.assertEqual(len(calls),1)
        self.assertEqual(calls[0].operands[0].type,capstone.x86.X86_OP_MEM)
        self.assertEqual(calls[0].operands[0].mem.disp,0x38)
        self.assertEqual(retail.bytes_at_va(0x4be9ae,2),b'\x90'*2)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-primary-format-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/audio/PrimarySoundFormat.cpp', 'tests/fixtures/primary_sound_format_compare.cpp'):
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
                                      if s.storage_class == 2 and s.section_number > 0}, {SYMBOL})
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 158)
                    report = compare(retail, candidate, SYMBOL, 0x4be910, 158,
                                     ROOT / 'samples/audio/PrimarySoundFormat.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 108)
            executable = output / 'format.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('PrimarySoundFormat: 172032 PCM, return, callback and fault scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
