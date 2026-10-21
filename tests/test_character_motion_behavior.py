"""Native execution of the complete standalone playback/pose candidate unit."""
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
SYMBOL = '?CharacterVirtualSlot7@Character@@UAEHMHH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class CharacterMotionBehaviorTests(unittest.TestCase):
    def test_complete_playback_and_pose_helpers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        ranges = {0x4a70c0: 3327, 0x4a7dc0: 523, 0x4a7fd0: 1121,
                  0x4a8470: 1205, 0x4a5750: 230}
        allowed = {0x4a7fd0, 0x421d00, 0x4a8930, 0x5015b0, 0x4a7dc0,
                   0x4a8470, 0x404e60, 0x421cb0, 0x515600, 0x4a5750,
                   0x5087b0, 0x4a8b60, 0x42de90, 0x42a1a0, 0x40ae30,
                   0x4a8440, 0x460c00, 0x535240, 0x534a88}
        patches, seen, indirect = [], set(), set()
        for start, size in ranges.items():
            instructions = list(decoder.disasm(retail.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(instructions[-1].mnemonic, 'ret')
            for i in instructions:
                if i.mnemonic != 'call':
                    continue
                if i.operands[0].type == capstone.x86.X86_OP_IMM:
                    target = i.operands[0].imm
                    self.assertIn(target, allowed)
                    seen.add(target)
                    if target in {0x535240, 0x534a88}:
                        patches.append(struct.pack('<II', i.address, target))
                else:
                    indirect.add(i.op_str)
        self.assertEqual(seen, allowed)
        self.assertEqual(indirect, {'dword ptr [eax + 0x10]', 'dword ptr [eax + 0x14]',
                                    'dword ptr [edx + 0x10]', 'dword ptr [edx + 0x14]'})
        self.assertEqual(len(patches), 4)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-character-motion-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/physics/motion/Motnctrl.cpp', 'src/reconstructed/VectorCalls.cpp', 'tests/fixtures/character_motion_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 3333)
                    for symbol, va, size in [('ClampFloat', 0x4a8440, 44),
                                             ('??0CharacterPose', 0x4a8930, 3),
                                             ('InterpolatePose', 0x4a8470, 1205)]:
                        report = compare(retail, candidate, symbol, va, size,
                                         ROOT / 'samples/physics/motion/Motnctrl.bindings.json')
                        self.assertTrue(report['strict_exact'], report)
            executable, mapping = output / 'motion.exe', output / 'motion.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/include:{SYMBOL}', f'/out:{executable}', f'/map:{mapping}',
                f'/libpath:{libraries}', *objects, 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            match = re.search(re.escape(SYMBOL) + r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin-1'))
            self.assertIsNotNone(match)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(reference), str(patch_file), match.group(1)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('835584 full state and applied pose scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
