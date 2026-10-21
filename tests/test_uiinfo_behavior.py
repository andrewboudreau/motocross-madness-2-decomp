"""Whole-TU defaults and file-search comparison with native VC6 and retail."""
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
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
METHODS = {
    '?ResetDefaults@TrackGameMode@@QAEXXZ': (0x522440, 427, 427, 383),
    '?FindDataFile@TrackGameMode@@QAEHPBDPAD@Z': (0x5238f0, 355, 357, 190),
}
DEPENDENCIES = {0x522720, 0x522780, 0x5227d0, 0x522e20, 0x522800,
                0x522840, 0x522cb0, 0x522cd0, 0x5476f0, 0x534aaf,
                0x534ceb, 0x523bf0, 0x523800, 0x534c3d}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class UiInfoBehaviorTests(unittest.TestCase):
    def test_defaults_and_search_with_scripted_and_native_file_dependencies(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        patches, seen = [], set()
        for start, size, _, _ in METHODS.values():
            body = image.bytes_at_va(start, size)
            reached, pending = {}, [start]
            while pending:
                at = pending.pop()
                if at in reached:
                    continue
                self.assertTrue(start <= at < start + size)
                instruction = next(decoder.disasm(body[at-start:], at, count=1))
                reached[at] = instruction
                if instruction.group(capstone.CS_GRP_RET):
                    continue
                if instruction.group(capstone.CS_GRP_JUMP):
                    self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                    pending.append(instruction.operands[0].imm)
                    if instruction.mnemonic == 'jmp':
                        continue
                pending.append(at + instruction.size)
            self.assertEqual(sorted(byte for at, instruction in reached.items()
                                    for byte in range(at, at + instruction.size)),
                             list(range(start, start + size)))
            for instruction in reached.values():
                if instruction.mnemonic != 'call':
                    continue
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                target = instruction.operands[0].imm
                self.assertIn(target, DEPENDENCIES)
                patches.append(struct.pack('<II', instruction.address, target))
                seen.add(target)
        self.assertEqual(seen, DEPENDENCIES)
        self.assertEqual(image.bytes_at_va(0x523a50, 3), bytes.fromhex('c20800'))
        self.assertEqual(image.bytes_at_va(0x523a53, 13), b'\x90' * 13)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-uiinfo-') as directory:
            output = Path(directory)
            objects = []
            sources = [('samples/game/UiInfoNearMisses.cpp', 'candidate', []),
                       ('tests/fixtures/uiinfo_compare.cpp', 'scripted', []),
                       ('tests/fixtures/uiinfo_compare.cpp', 'native_files', ['/DUIINFO_REAL_FILES'])]
            for source, stem, flags in sources:
                obj = output / (stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    *[arg for flag in flags for arg in ('--extra', flag)]],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if stem == 'candidate':
                    compiled = CoffObject(obj)
                    methods = {s.name for s in compiled.symbols
                               if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}
                    self.assertEqual(methods, set(METHODS))
                    for symbol, (va, size, compiled_size, matching) in METHODS.items():
                        entry = compiled.find_symbol(symbol)
                        self.assertEqual(entry.extent_source, 'coff_function_aux')
                        self.assertEqual(entry.function_size, compiled_size)
                        report = compare(image, compiled, symbol, va, size,
                                         ROOT / 'samples/game/UiInfoNearMisses.bindings.json')
                        self.assertEqual(report['strict_match']['matching_positions'], matching)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            for index, stem in ((1, 'scripted'), (2, 'native_files')):
                executable = output / (stem + '.exe')
                result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                    '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                    objects[0], objects[index], 'kernel32.lib'], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin-1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                result = subprocess.run([str(executable), str(reference), str(patch_file), str(output)],
                                        cwd=output, capture_output=True, text=True, timeout=60)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                expected = ('UiInfo: 21332 scripted comparisons (1152 defaults) match complete state and ordered calls'
                            if stem == 'scripted' else 'UiInfo: 192 native CRT file-search comparisons match')
                self.assertIn(expected, result.stdout)


if __name__ == '__main__':
    unittest.main()
