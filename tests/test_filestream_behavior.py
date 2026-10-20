"""Private differential tests for the reviewed nonexact stream methods."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root


ROOT = Path(__file__).resolve().parents[1]
TARGETS = {0x460DB0: 186, 0x461340: 702, 0x461640: 663,
           0x4618E0: 152, 0x461B90: 282}
DEPENDENCIES = {0x534EAC, 0x534C3D, 0x5365D6, 0x534F65, 0x5350B7,
                0x5359D2, 0x45C6A0, 0x45C7B0, 0x461600, 0x461D20,
                0x461980, 0x461A60}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class FileStreamBehaviorTests(unittest.TestCase):
    def test_buffered_encoded_and_archive_paths_against_private_retail(self):
        import capstone

        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         expected['mcm2_sha256'])
        pe = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        patches = []
        seen = set()
        for start, size in TARGETS.items():
            for instruction in decoder.disasm(pe.bytes_at_va(start, size), start):
                if instruction.mnemonic != 'call':
                    continue
                self.assertEqual(instruction.bytes[0], 0xE8)
                target = instruction.operands[0].imm
                self.assertIn(target, DEPENDENCIES)
                patches.append(struct.pack('<II', instruction.address, target))
                seen.add(target)
        self.assertEqual(seen, DEPENDENCIES)
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-filestream-') as directory:
            output = Path(directory)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            objects = []
            for source in ('samples/io/FileStream.cpp',
                           'tests/fixtures/filestream_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                command = [
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(obj),
                    '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ]
                if source.startswith('samples/'):
                    # Replace only external dependency names in this test
                    # object. The candidate's control flow remains unchanged.
                    for original, mock in (
                        ('fseek', 'ProbeSeek'), ('fclose', 'ProbeClose'),
                        ('ftell', 'ProbeTell'), ('fread', 'ProbeRead'),
                        ('fwrite', 'ProbeWrite'), ('_splitpath', 'ProbeSplit'),
                    ):
                        command.append(f'--extra=/D{original}={mock}')
                result = subprocess.run(command, cwd=ROOT, capture_output=True,
                                        text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
            executable = output / 'stream_probe.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}', *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('FileStream: 12390 scenarios match results, state, bytes and ordered calls',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
