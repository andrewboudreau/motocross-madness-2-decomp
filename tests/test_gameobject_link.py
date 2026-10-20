"""Exercise shared physics/game declarations against their real implementations."""
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
class GameObjectLinkTests(unittest.TestCase):
    def test_shared_hierarchy_links_and_dispatches(self):
        compiler_root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-gameobject-') as directory:
            output = Path(directory)
            objects = []
            for source in ('src/reconstructed/BaseObject.cpp',
                           'src/reconstructed/GameObject.cpp',
                           'src/reconstructed/MemTag.cpp',
                           'tests/fixtures/gameobject_smoke.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run(
                    [sys.executable, str(ROOT / 'tools/compile.py'),
                     str(ROOT / source), '-o', str(obj),
                     '--vc6-root', str(compiler_root),
                     '--include', str(ROOT / 'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            executable = output / 'gameobject.exe'
            result = subprocess.run(
                [str(linker), '/nologo', '/subsystem:console',
                 f'/out:{executable}', f'/libpath:{libraries}',
                 *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True,
                text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('virtual dispatch', result.stdout)


if __name__ == '__main__':
    unittest.main()
