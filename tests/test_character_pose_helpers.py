"""Direct native, alias-sensitive tests of the complete standalone pose helpers."""
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

ROOT = Path(__file__).resolve().parents[1]
HELPERS = (
    ('?RotatePose@@YAXPAUCharacterPose@@0UVec3@@M@Z', 0x4a7dc0, 523),
    ('?PoseRotation@@YAXPAUCharacterPose@@0PAUVec3@@PAM@Z', 0x4a7fd0, 1121),
    ('?InterpolatePose@@YAXPAUCharacterPose@@00M@Z', 0x4a8470, 1205),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class CharacterPoseHelperTests(unittest.TestCase):
    def test_complete_helpers_with_aliases_and_finite_inputs(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        ranges = [(va, size) for _, va, size in HELPERS]
        allowed = {0x5087b0, 0x515600, 0x42de90, 0x42a1a0, 0x404e60,
                   0x40ae30, 0x460c00, 0x5015b0, 0x4a8440, 0x535240, 0x421cb0}
        seen = set()
        patches = []
        for start, size in ranges:
            instructions = list(decoder.disasm(retail.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(instructions[-1].mnemonic, 'ret')
            for instruction in instructions:
                if instruction.mnemonic == 'call':
                    self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                    target = instruction.operands[0].imm
                    self.assertIn(target, allowed)
                    seen.add(target)
                    if target == 0x535240:
                        patches.append(struct.pack('<II', instruction.address, target))
        self.assertEqual(seen, allowed)
        self.assertEqual(len(patches), 3)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-character-poses-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/physics/motion/Motnctrl.cpp', 'tests/fixtures/character_pose_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    for symbol, _, size in HELPERS:
                        entry = candidate.find_symbol(symbol)
                        self.assertEqual(entry.extent_source, 'coff_function_aux')
                        self.assertEqual(entry.function_size, size)
            executable, mapping = output / 'poses.exe', output / 'poses.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', *['/include:' + symbol for symbol, _, _ in HELPERS],
                f'/out:{executable}', f'/map:{mapping}', f'/libpath:{libraries}', *objects, 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            addresses = []
            for symbol, _, _ in HELPERS:
                match = re.search(re.escape(symbol) + r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin-1'))
                self.assertIsNotNone(match)
                addresses.append(match.group(1))
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(reference), str(patch_file), *addresses],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('RotatePose 43008, PoseRotation 9216, InterpolatePose 107520 complete-state cases match', result.stdout)


if __name__ == '__main__':
    unittest.main()
