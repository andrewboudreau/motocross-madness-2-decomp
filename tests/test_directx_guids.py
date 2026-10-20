"""Verify DirectX and network identifier providers against private retail data."""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import sha256_file


ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class DirectXGuidTests(unittest.TestCase):
    def test_sdk_compatible_symbols_sizes_readonly_storage_and_values(self):
        self.assert_guid_provider('DirectXGuids')

    def test_network_symbols_sizes_readonly_storage_and_values(self):
        self.assert_guid_provider('NetworkGuids')

    def assert_guid_provider(self, stem):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        bindings = json.loads((ROOT / f'src/reconstructed/{stem}.bindings.json').read_text())
        retail = PEImage(EXE)
        with tempfile.TemporaryDirectory(prefix='mcm2-guids-') as tmp:
            path = Path(tmp) / f'{stem}.obj'
            result = subprocess.run([
                sys.executable, str(ROOT / 'tools/compile.py'),
                str(ROOT / f'src/reconstructed/{stem}.cpp'), '-o', str(path),
                '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            obj = CoffObject(path)
            self.assertEqual(obj.machine, 0x14c)
            definitions = {s.name: s for s in obj.symbols
                           if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), set(bindings))
            self.assertFalse(any(s.raw_size and s.characteristics & 0x20 for s in obj.sections))
            for name, address in bindings.items():
                with self.subTest(symbol=name):
                    symbol = definitions[name]
                    section = obj.section(symbol.section_number)
                    following = [s.value for s in definitions.values()
                                 if s.section_number == symbol.section_number
                                 and s.value > symbol.value]
                    self.assertEqual(min(following, default=section.raw_size) - symbol.value, 16)
                    start = section.raw_ptr + symbol.value
                    va = int(address, 16)
                    self.assertEqual(obj.data[start:start + 16], retail.bytes_at_va(va, 16))
                    self.assertFalse(section.characteristics & 0x80000000)
                    original = retail.section_for_rva(va - retail.image_base)
                    self.assertFalse(original.characteristics & 0x80000000)


if __name__ == '__main__':
    unittest.main()
