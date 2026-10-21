"""Native proof of the explicitly nonexact QPC profiling backend."""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'),
                     'requires native Windows and authentic VC6_ROOT')
class ProfilingCounterTests(unittest.TestCase):
    def test_real_backend_import_signed_return_and_native_qpc_brackets(self):
        compiler_root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-profiling-counter-') as directory:
            output = Path(directory)
            objects = []
            for name, source in (
                ('backend', 'samples/game/ProfilingCounter.cpp'),
                ('fixture', 'tests/fixtures/profiling_counter.cpp'),
            ):
                path = output / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', str(compiler_root),
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'samples/game'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(path))
            backend = CoffObject(objects[0])
            self.assertEqual(backend.machine, 0x14c)
            symbol = next(s for s in backend.symbols if s.name == '?ReadProfilingCounter@@YA_JXZ')
            section = backend.sections[symbol.section_number - 1]
            start = section.raw_ptr + symbol.value
            self.assertIsNotNone(symbol.function_size)
            instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
                backend.data[start:start + symbol.function_size], symbol.value))
            self.assertEqual(sum(i.size for i in instructions), symbol.function_size)
            operations = [(i.mnemonic, i.op_str) for i in instructions]
            # The signed __int64 result returns both raw DWORDs in EDX:EAX;
            # no floating conversion or low-word truncation is admitted here.
            self.assertEqual(operations[-4:], [
                ('mov', 'eax, dword ptr [esp]'),
                ('mov', 'edx, dword ptr [esp + 4]'),
                ('add', 'esp, 8'), ('ret', ''),
            ])
            self.assertFalse(any(i.mnemonic == 'rdtsc' for i in instructions))
            relocations = [r for r in backend.relocations
                           if r.section_number == symbol.section_number
                           and symbol.value <= r.virtual_address < symbol.value + symbol.function_size]
            self.assertEqual(len(relocations), 1)
            target = backend.symbol_by_index[relocations[0].symbol_index]
            self.assertEqual((target.name, target.section_number, relocations[0].type),
                             ('__imp__QueryPerformanceCounter@4', 0, 6))
            calls = [i for i in instructions if i.mnemonic == 'call']
            self.assertEqual(len(calls), 1)
            self.assertLessEqual(calls[0].address, relocations[0].virtual_address)
            self.assertLess(relocations[0].virtual_address, calls[0].address + calls[0].size)
            executable = output / 'profiling-counter.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('4096 real QPC brackets passed;', result.stdout)


if __name__ == '__main__':
    unittest.main()
