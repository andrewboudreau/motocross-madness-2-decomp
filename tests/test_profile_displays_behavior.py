"""Private native comparison of the complete display-profiling candidate."""
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
SYMBOL = '?ProfileDisplays@PCGame@@QAEHXZ'
RANGES = {0x4c0d10: 1790, 0x54cbf0: 26, 0x54cc0a: 10}
DEPENDENCIES = {0x4c16f0, 0x534aaf, 0x4c9d20, 0x4c9c90, 0x4c9b50,
                0x4a3010, 0x4c4ee0, 0x4c4f80, 0x4c0760, 0x4ca5a0,
                0x5119c0, 0x4cab00, 0x4a3060, 0x53471a}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class ProfileDisplaysBehaviorTests(unittest.TestCase):
    def test_whole_profiling_state_calls_resources_and_unwind(self):
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
            self.assertEqual(instructions[-1].mnemonic, 'jmp' if start == 0x54cc0a else 'ret')
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
        # This prologue loads FS:[0] before push -1; the general EH parser's
        # prefix recognizer does not cover it. Check its one-state map directly.
        magic, states, unwind, tries = struct.unpack('<IIII', image.bytes_at_va(0x562870, 16))
        self.assertEqual((magic, states, tries), (0x19930520, 1, 0))
        self.assertEqual(struct.unpack('<iI', image.bytes_at_va(unwind, 8)), (-1, 0x54cbf0))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-profile-displays-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/game/PCGameNearMisses.cpp', 'tests/fixtures/profile_displays_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    # VC6 /O2 ICEs compiling the fixture's seven-argument
                    # scripted display callback. Production stays /O2.
                    *(['--extra', '/Od'] if source.startswith('tests/') else [])], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    methods = {s.name for s in candidate.symbols
                               if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}
                    self.assertEqual(methods, {SYMBOL})
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 1790)
                    def relocated(section, offset):
                        matches = [r for r in candidate.relocations
                                   if r.section_number == section and r.virtual_address == offset]
                        self.assertEqual(len(matches), 1)
                        return candidate.symbol_by_index[matches[0].symbol_index]

                    def data(symbol, size):
                        section = candidate.section(symbol.section_number)
                        start = section.raw_ptr + symbol.value
                        return candidate.data[start:start + size]

                    handler = relocated(entry.section_number, entry.value + 9)
                    info = relocated(handler.section_number, handler.value + 1)
                    fields = struct.unpack('<IIII', data(info, 16))
                    self.assertEqual((fields[0], fields[1], fields[3]), (magic, states, tries))
                    unwind_map = relocated(info.section_number, info.value + 8)
                    self.assertEqual(struct.unpack('<i', data(unwind_map, 4))[0], -1)
                    action = relocated(unwind_map.section_number, unwind_map.value + 4)
                    cleanup = relocated(action.section_number, action.value + 18)
                    self.assertEqual(cleanup.name, '??3@YAXPAXPBDH@Z')
                    report = compare(image, candidate, SYMBOL, 0x4c0d10, 1790,
                                     ROOT / 'samples/game/PCGameNearMisses.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 1776)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'profile.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('ProfileDisplays: 8486 scenarios match state, ordered calls, resources and unwind',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
