"""Private whole-body crash-reporter differential execution, including SEH."""
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
SYMBOL = '?RecordExceptionInfo@@YAHPAU_EXCEPTION_POINTERS@@PBD@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail EXE')
class ExceptionReportTests(unittest.TestCase):
    def test_complete_reporter_with_faulting_code_and_stack_reads(self):
        import capstone

        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x45ff80, 1290), 0x45ff80))
        self.assertEqual(sum(i.size for i in instructions), 1290)
        self.assertEqual((instructions[-1].address, instructions[-1].mnemonic), (0x460489, 'ret'))
        self.assertEqual(image.bytes_at_va(0x46048a, 6), b'\x90' * 6)
        patches, seen, imports = [], set(), set()
        for i in instructions:
            if i.mnemonic != 'call':
                continue
            operand = i.operands[0]
            if operand.type == capstone.x86.X86_OP_IMM:
                target = operand.imm
                self.assertIn(target, {0x460490, 0x4604e0, 0x4607a0, 0x4608c0, 0x460ab0, 0x534a60})
                seen.add(target)
                patches.append(struct.pack('<II', i.address, target))
            else:
                imports.add(i.op_str)
        self.assertEqual(seen, {0x460490, 0x4604e0, 0x4607a0, 0x4608c0, 0x460ab0, 0x534a60})
        self.assertEqual(imports, {'edi', 'dword ptr [0x5501e4]', 'dword ptr [0x5501ec]',
            'dword ptr [0x5501f4]', 'dword ptr [0x5501f8]', 'dword ptr [0x5501fc]',
            'dword ptr [0x550200]', 'dword ptr [0x5502dc]'})
        self.assertEqual(image.bytes_at_va(0x4602ef, 6), b'\x64\xa1\x04\0\0\0')
        self.assertEqual(struct.unpack('<6I', image.bytes_at_va(0x5526b0, 24)),
                         (0xffffffff, 0x46029e, 0x4602a4, 0xffffffff, 0x460443, 0x460449))
        root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-exception-report-') as directory:
            output = Path(directory)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            guard_offsets = []
            for name, source in [('candidate', 'samples/game/RecordExceptionInfo.cpp'),
                                 ('fixture', 'tests/fixtures/exception_report_compare.cpp')]:
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(root), '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if name == 'candidate':
                    compiled = CoffObject(obj)
                    entry = compiled.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertGreater(entry.function_size, 1000)
                    for relocation in compiled.relocations:
                        target = compiled.symbol_by_index[relocation.symbol_index]
                        if target.name == '_exceptionReportStarted' and relocation.section_number == entry.section_number:
                            self.assertEqual(relocation.type, 6)
                            self.assertEqual(target.storage_class, 3)
                            guard_offsets.append(relocation.virtual_address - entry.value)
                    self.assertEqual(len(guard_offsets), 2)
            executable, map_file = output / 'report.exe', output / 'report.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/include:{SYMBOL}',
                f'/out:{executable}', f'/map:{map_file}', f'/libpath:{libraries}',
                str(output / 'candidate.obj'), str(output / 'fixture.obj'), 'kernel32.lib', 'user32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            match = re.search(re.escape(SYMBOL) + r'\s+([0-9a-fA-F]{8})\s', map_file.read_text(encoding='latin-1'))
            self.assertIsNotNone(match)
            linked = PEImage(executable)
            function_va = int(match.group(1), 16)
            guards = {struct.unpack('<I', linked.bytes_at_va(function_va + offset, 4))[0]
                      for offset in guard_offsets}
            self.assertEqual(len(guards), 1)
            guard = guards.pop()
            section = linked.section_for_rva(guard - linked.image_base)
            self.assertTrue(section.characteristics & 0x80000000)
            self.assertLessEqual(guard + 4 - linked.image_base, section.rva_end)
            result = subprocess.run([str(executable), str(reference), str(patch_file), match.group(1), f'{guard:x}'],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('13838 scenarios match ordered output, failures and one-shot state', result.stdout)


if __name__ == '__main__':
    unittest.main()
