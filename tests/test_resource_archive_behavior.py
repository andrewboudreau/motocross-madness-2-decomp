"""Private VC6 differential validation of the standalone AddArchive candidate."""
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

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.eh_funcinfo import candidate, retail, shape

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?AddArchive@UnknownResourceManager@@QAEPAVUnknownTextureStream@@PBDH@Z'
RANGES = {0x4E9030: 804, 0x54DAA0: 23, 0x54DAB7: 23, 0x54DACE: 10}
DEPENDENCIES = {0x4A3010, 0x4A3060, 0x4A30C0, 0x460D10, 0x460D60,
                0x4E8D70, 0x460E70, 0x460F50, 0x461640, 0x4A2EC0,
                0x4E8F40, 0x536070, 0x53471A}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class ResourceArchiveBehaviorTests(unittest.TestCase):
    def test_archive_loader_state_ownership_failures_and_unwind_against_retail(self):
        import capstone

        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        patches = []
        seen = set()
        for start, size in RANGES.items():
            instructions = list(decoder.disasm(image.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            for instruction in instructions:
                if instruction.mnemonic not in ('call', 'jmp'):
                    continue
                if instruction.operands[0].type != capstone.x86.X86_OP_IMM:
                    # The item Release virtual call uses the fixture's real C++
                    # test vtable; no retail vtable or callback is redirected.
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
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-resource-archive-') as directory:
            output = Path(directory)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            objects = []
            for source in ('samples/render/ResourceManagerNearMisses.cpp',
                           'tests/fixtures/resource_archive_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if source.startswith('samples/'):
                    compiled = CoffObject(obj)
                    self.assertEqual(compiled.machine, 0x14c)
                    methods = {s.name for s in compiled.symbols
                               if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}
                    self.assertEqual(methods, {SYMBOL})
                    self.assertFalse(any('g_GlobalResourceManager' in s.name or 'g_ResourceManager' in s.name
                                         for s in compiled.symbols))
                    self.assertEqual(shape(candidate(obj, SYMBOL)), shape(retail(0x4e9030, image)))
                    symbol = compiled.find_symbol(SYMBOL)
                    self.assertEqual(symbol.extent_source, 'coff_function_aux')
                    self.assertEqual(compiled.symbol_extent(symbol)[1], 798)
                objects.append(str(obj))
            executable = output / 'archive_probe.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('AddArchive: 17292 scenarios match state, allocation ownership and ordered calls',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
