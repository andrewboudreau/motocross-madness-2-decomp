"""Private differential test for the reviewed nonexact 8888/1555 halvers."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root


ROOT = Path(__file__).resolve().parents[1]
TARGETS = {0x4CDF10: 636, 0x4CE420: 456}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class PixtransHalveBehaviorTests(unittest.TestCase):
    def test_halvers_against_private_retail(self):
        import capstone

        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         expected['mcm2_sha256'])
        pe = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        for start, size in TARGETS.items():
            # Position independent: no calls, no absolute memory operands.
            for instruction in decoder.disasm(pe.bytes_at_va(start, size), start):
                self.assertNotEqual(instruction.mnemonic, 'call')
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-pixtrans-halve-') as directory:
            output = Path(directory)
            blobs = []
            for start, size in TARGETS.items():
                blob = output / f'{start:08x}.bin'
                blob.write_bytes(pe.bytes_at_va(start, size))
                blobs.append(f'{start:x}:{blob}')
            objects = []
            for source in ('samples/render/PixtransNearMisses.cpp',
                           'tests/fixtures/pixtrans_halve_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(obj),
                    '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            executable = output / 'halve_probe.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', '/base:0x10000000', '/fixed:no', '/force:unresolved',
                f'/out:{executable}', f'/libpath:{libraries}', *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), *blobs],
                                    capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Pixtrans halve 4cdf10: rebased 6 absolute references', result.stdout)
            self.assertIn('Pixtrans halve 4ce420: rebased 0 absolute references', result.stdout)
            self.assertIn('Pixtrans halve: 20000 scenarios per function match', result.stdout)


if __name__ == '__main__':
    unittest.main()
