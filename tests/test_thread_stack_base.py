"""Native test of the nonexact OS stack-bound backend, including fibers."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'), 'requires native VC6')
class ThreadStackBaseTests(unittest.TestCase):
    def test_real_os_stack_bounds(self):
        root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-stack-base-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/game/ThreadStackBase.cpp', 'tests/fixtures/thread_stack_base.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'samples/game')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            backend = CoffObject(objects[0])
            names = {backend.symbol_by_index[r.symbol_index].name for r in backend.relocations}
            self.assertTrue({'__imp__GetModuleHandleA@4', '__imp__GetProcAddress@8',
                             '__imp__RaiseException@16'} <= names)
            symbol = backend.find_symbol('?ReadCurrentThreadStackBase@@YAPAKXZ')
            self.assertEqual(symbol.extent_source, 'coff_function_aux')
            self.assertGreater(symbol.function_size, 0)
            executable = output / 'stack-base.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}', *objects, 'kernel32.lib'],
                capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('1536 actual stack bounds match independent OS limits', result.stdout)
            self.assertIn('Both unavailable-backend paths raise the guarded stack-dump exception', result.stdout)


if __name__ == '__main__':
    unittest.main()
