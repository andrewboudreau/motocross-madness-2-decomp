"""Execute full nonexact transforms against retail, including in-place aliases."""
import hashlib
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
FUNCTIONS = (
    ('?WorldToLocalDirection@SoultreeObject@@QAE?AUVec3@@ABU2@@Z', 0x4fd710, 209),
    ('?SetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z', 0x4fb8c0, 1096),
    ('?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z', 0x4fc050, 1171),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class SoultreeTransformBehaviorTests(unittest.TestCase):
    def test_full_standalone_transform_objects(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        script = {0x4fb4f0, 0x4fb850, 0x4fca30}
        math = {0x4fefb0, 0x436500, 0x42a4b0, 0x515600, 0x5087b0, 0x42a1a0}
        patches, seen = [], set()
        for symbol, start, size in FUNCTIONS:
            instructions = list(decoder.disasm(retail.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(instructions[-1].mnemonic, 'ret')
            self.assertEqual(instructions[-1].address + instructions[-1].size, start + size)
            for i in instructions:
                if i.mnemonic not in ('call', 'jmp'):
                    continue
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                target = i.operands[0].imm
                if start <= target < start + size:
                    continue
                self.assertIn(target, script | math)
                seen.add(target)
                if target in script:
                    self.assertEqual(i.mnemonic, 'call')
                    patches.append(struct.pack('<II', i.address, target))
        self.assertEqual(seen, script | math)
        self.assertEqual(len(patches), 9)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-soultree-transform-') as directory:
            output = Path(directory)
            objects = []
            candidates = {}
            for source in ('samples/physics/helpers/SoultreeTransform.cpp',
                           'samples/physics/helpers/SoultreeMatrix.cpp',
                           'tests/fixtures/soultree_transform_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if not source.startswith('tests/'):
                    candidates[Path(source).stem] = CoffObject(obj)
            for symbol, va, size in FUNCTIONS:
                obj = candidates['SoultreeTransform' if va == 0x4fd710 else 'SoultreeMatrix']
                entry = obj.find_symbol(symbol)
                self.assertEqual(entry.extent_source, 'coff_function_aux')
                self.assertEqual(entry.function_size, 1134 if va == 0x4fb8c0 else size)
            transpose = compare(retail, candidates['SoultreeMatrix'], 'TransposeRotation3x3',
                                0x4fefb0, 41, ROOT / 'samples/physics/helpers/SoultreeMatrix.bindings.json')
            self.assertTrue(transpose['strict_exact'], transpose)
            executable, mapping = output / 'transform.exe', output / 'transform.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', f'/out:{executable}', f'/map:{mapping}', f'/libpath:{libraries}',
                *(f'/include:{s}' for s, _, _ in FUNCTIONS), *objects, 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            addresses = []
            for symbol, _, _ in FUNCTIONS:
                match = re.search(re.escape(symbol) + r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin-1'))
                self.assertIsNotNone(match)
                addresses.append(match.group(1))
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(reference), str(patch_file), *addresses],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('86016 complete-state and alias scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
