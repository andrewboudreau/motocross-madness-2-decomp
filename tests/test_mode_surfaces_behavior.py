"""Complete display-mode surface candidate, including signed budget arithmetic."""
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
SYMBOL = '?CreateModeSurfaces@UnknownDisplay@@QAEHH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class ModeSurfacesBehaviorTests(unittest.TestCase):
    def test_full_mode_budget_branches_and_callback_state(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x4ca520, 116), 0x4ca520))
        self.assertEqual(sum(i.size for i in instructions), 116)
        self.assertEqual(bytes(instructions[-1].bytes), bytes.fromhex('c20400'))
        boundaries = {i.address for i in instructions}
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertIn(i.operands[0].imm, boundaries)
        calls = [(i.address, i.operands[0].imm) for i in instructions if i.mnemonic == 'call']
        self.assertEqual(calls, [(0x4ca56b, 0x4c9f30), (0x4ca582, 0x4ca130)])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-mode-surfaces-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/render/ModeSurfaces.cpp', 'tests/fixtures/mode_surfaces_compare.cpp'):
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
                    self.assertEqual(entry.function_size, 124)
                    report = compare(retail, candidate, SYMBOL, 0x4ca520, 116,
                                     ROOT / 'samples/render/ModeSurfaces.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 26)
            executable = output / 'modes.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('ModeSurfaces: 200448 arithmetic, boundary and callback scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
