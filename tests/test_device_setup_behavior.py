"""Private native comparison of startup behavior, separate from byte matching."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class DeviceSetupBehaviorTests(unittest.TestCase):
    def test_all_startup_branches_against_private_retail(self):
        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         expected['mcm2_sha256'])
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-device-setup-') as directory:
            output = Path(directory)
            objects = []
            for source in ('tests/fixtures/device_setup_compare.cpp',
                           'src/reconstructed/DirectXGuids.cpp'):
                obj = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'),
                           str(ROOT / source), '-o', str(obj),
                           '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt']
                result = subprocess.run(command, cwd=ROOT, capture_output=True,
                                        text=True, encoding='latin-1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            # Avoid Windows' legacy installer detection for executable names
            # containing "setup"; this test performs no installation.
            executable = output / 'dx_probe.exe'
            result = subprocess.run(
                [str(linker), '/nologo', '/subsystem:console',
                 f'/out:{executable}', f'/libpath:{libraries}',
                 *objects, 'kernel32.lib', 'dxguid.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)],
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('720 scenarios match outputs and ordered API/COM calls',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
