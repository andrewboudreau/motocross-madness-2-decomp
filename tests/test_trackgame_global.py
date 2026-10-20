"""Validate shared TrackGame pointer access across independently compiled views."""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root


ROOT = Path(__file__).resolve().parents[1]
GLOBAL = '?g_TrackGame@@3PAVTrackGame@@A'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'),
                     'requires native Windows and VC6_ROOT')
class TrackGameGlobalTests(unittest.TestCase):
    def test_views_observe_shared_writes_and_pointer_reassignment(self):
        compiler_root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-trackgame-') as directory:
            output = Path(directory)
            objects = []
            for stem in ('particle', 'eco', 'smoke'):
                source = ROOT / f'tests/fixtures/trackgame_global_{stem}.cpp'
                obj = output / f'{stem}.obj'
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(source),
                    '-o', str(obj), '--vc6-root', str(compiler_root),
                    '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                coff = CoffObject(obj)
                shared = [s for s in coff.symbols if s.name == GLOBAL]
                self.assertEqual(len(shared), 1)
                self.assertEqual(shared[0].storage_class, 2)
                if stem != 'smoke':
                    self.assertEqual(shared[0].section_number, 0)
                    self.assertEqual(shared[0].value, 0)
                objects.append(str(obj))
            executable = output / 'trackgame.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('share writes and observe pointer reassignment', result.stdout)


if __name__ == '__main__':
    unittest.main()
