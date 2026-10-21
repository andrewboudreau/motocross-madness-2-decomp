"""Private native differential test of the standalone LoadVue candidate object."""
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.eh_funcinfo import candidate, retail, shape

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?LoadVue@MotionManager@@QAEHPAVCharacter@@PBDPAUMotion@@@Z'
RANGES = {0x4a5e40: 1090, 0x54c450: 26, 0x54c46a: 10}
DEPENDENCIES = {0x4a3010, 0x4a3060, 0x4a30c0, 0x460d10, 0x460d60,
                0x460f50, 0x461aa0, 0x461340, 0x4a6b50, 0x4a2e20,
                0x4a2ec0, 0x534aaf, 0x535d3d, 0x536397, 0x536070,
                0x536340, 0x53471a}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class MotionVueBehaviorTests(unittest.TestCase):
    def test_actual_loader_object_parse_state_ownership_and_unwind(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        patches, seen = [], set()
        for start, size in RANGES.items():
            instructions = list(decoder.disasm(image.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            if start == 0x4a5e40:
                self.assertEqual(instructions[-1].mnemonic, 'ret')
                self.assertEqual(instructions[-1].address + instructions[-1].size, start + size)
            for instruction in instructions:
                if instruction.mnemonic not in ('call', 'jmp'):
                    continue
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                target = instruction.operands[0].imm
                if start <= target < start + size:
                    continue
                self.assertIn(target, DEPENDENCIES)
                self.assertIn(instruction.bytes[0], (0xe8, 0xe9))
                patches.append(struct.pack('<III', instruction.address, target, instruction.bytes[0]))
                seen.add(target)
        self.assertEqual(seen, DEPENDENCIES)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-motion-vue-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/physics/motion/Motnctrl.cpp',
                           'tests/fixtures/motion_vue_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(obj), '--vc6-root', str(compiler),
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if source.startswith('samples/'):
                    compiled = CoffObject(obj)
                    symbol = compiled.find_symbol(SYMBOL)
                    self.assertEqual(symbol.extent_source, 'coff_function_aux')
                    self.assertEqual(symbol.function_size, 1088)
                    self.assertEqual(shape(candidate(obj, SYMBOL)), shape(retail(0x4a5e40, image)))
                objects.append(str(obj))
            executable = output / 'vue.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/opt:ref',
                '/base:0x100000', '/fixed', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('LoadVue: 33280 scenarios match complete state, ownership and calls', result.stdout)


if __name__ == '__main__':
    unittest.main()
