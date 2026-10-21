"""Native validation of the complete standalone partial-upload probe."""
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
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?ProbePartialTextureUploads@UnknownDisplay@@QAEXPAVRenderTarget@@@Z'
RANGES = {0x4cab00: 1129, 0x54cdd0: 23, 0x54cde7: 23, 0x54cdfe: 23, 0x54ce15: 23, 0x54ce2c: 10}
DEPENDENCIES = {0x5125c0, 0x4ca900, 0x4a3010, 0x4c4ee0, 0x4c4f80,
                0x4c5f00, 0x512dd0, 0x4caf70, 0x4a3060, 0x53471a}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class PartialTextureUploadsTests(unittest.TestCase):
    def test_complete_probe_state_ownership_calls_exceptions_and_faults(self):
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
            if start == 0x4cab00:
                boundaries = {i.address for i in instructions}
                self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'ret'],
                                 [(0x4caeae, '4')])
                self.assertEqual((instructions[-1].address, instructions[-1].size, instructions[-1].mnemonic),
                                 (0x4caf64, 5, 'jmp'))
                for instruction in instructions:
                    if instruction.mnemonic.startswith('j'):
                        self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                        self.assertIn(instruction.operands[0].imm, boundaries)
            for instruction in instructions:
                if instruction.mnemonic not in ('call', 'jmp'):
                    continue
                if instruction.operands[0].type != capstone.x86.X86_OP_IMM:
                    self.assertEqual(instruction.mnemonic, 'call')
                    continue
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
        with tempfile.TemporaryDirectory(prefix='mcm2-partial-upload-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/render/PartialTextureUploads.cpp',
                           'tests/fixtures/partial_texture_uploads_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    *(['--extra', '/Od'] if source.startswith('tests/') else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    compiled = CoffObject(obj)
                    methods = {s.name for s in compiled.symbols
                               if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}
                    self.assertEqual(methods, {SYMBOL})
                    entry = compiled.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 1145)
                    eh = shape(retail(0x4cab00, image))
                    self.assertEqual(eh['max_state'], 4)
                    self.assertEqual(shape(candidate(obj, SYMBOL)), eh)
                    report = compare(image, compiled, SYMBOL, 0x4cab00, 1129,
                                     ROOT / 'samples/render/PartialTextureUploads.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 140)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'partial.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('PartialTextureUploads: 26600 scenarios match state, ownership, calls, exceptions and faults',
                          result.stdout)
            self.assertIn('23 C++ exceptions, 6 access violations', result.stdout)


if __name__ == '__main__':
    unittest.main()
