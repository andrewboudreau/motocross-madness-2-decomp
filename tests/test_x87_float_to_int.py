"""Check the bring-up fistp stand-in against a real x87 FISTP under VC6."""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'),
                     'requires native Windows and VC6_ROOT')
class X87FloatToIntTests(unittest.TestCase):
    def test_helper_matches_fistp_in_every_rounding_mode(self):
        compiler_root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-x87-float-to-int-') as directory:
            output = Path(directory)
            objects = []
            # The oracle uses /QIfist, so its cast is a FISTP in the current mode.
            for source, extra in (('tests/fixtures/x87_float_to_int_oracle.cpp', ['/QIfist']),
                                  ('tests/fixtures/x87_float_to_int_check.cpp', [])):
                obj = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'),
                           str(ROOT / source), '-o', str(obj),
                           '--vc6-root', str(compiler_root),
                           '--include', str(ROOT / 'src/bringup'),
                           *[item for flag in extra for item in ('--extra', flag)]]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                        encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            executable = output / 'x87_float_to_int.exe'
            result = subprocess.run(
                [str(linker), '/nologo', '/subsystem:console',
                 f'/out:{executable}', f'/libpath:{libraries}',
                 *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True,
                text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn(' 0 mismatches', result.stdout)


if __name__ == '__main__':
    unittest.main()
