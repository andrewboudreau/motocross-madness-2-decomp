"""Verify the compiled renderer globals against the private retail image."""
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
class GraphicsStateTests(unittest.TestCase):
    def test_compiled_initial_values_sizes_and_mutability_match_retail(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        bindings = json.loads((ROOT / 'src/reconstructed/GraphicsState.bindings.json').read_text())
        sizes = {
            'g_DefaultNearPlane': 4, 'g_DefaultFarPlane': 4, 'g_BladeDeviceGuid': 16,
            'g_GammaRamp': 3 * 256 * 2, 'g_DebugCountTextureMemory': 4,
            'g_SharedMipTextureSurfaces': 10 * 4, 'g_SharedTextureSurfaces': 10 * 4,
            'g_Displays': 4 * 4, 'g_DisplayCount': 4,
            'g_NonBladeLodSettings': 10 * 16, 'g_BladeLodSettings': 10 * 16,
            'g_ActiveLodSettings': 4,
        }
        retail = PEImage(EXE)
        with tempfile.TemporaryDirectory(prefix='mcm2-graphics-') as tmp:
            path = Path(tmp) / 'GraphicsState.obj'
            result = subprocess.run([
                sys.executable, str(ROOT / 'tools/compile.py'),
                str(ROOT / 'src/reconstructed/GraphicsState.cpp'), '-o', str(path),
                '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            obj = CoffObject(path)
            self.assertEqual(obj.machine, 0x14c)
            definitions = {s.name: s for s in obj.symbols
                           if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), set(bindings))
            # No dynamic initializer is needed for these constants and caches.
            self.assertFalse(any(s.raw_size and s.characteristics & 0x20 for s in obj.sections))
            for name, address in bindings.items():
                with self.subTest(symbol=name):
                    symbol = definitions[name]
                    section = obj.sections[symbol.section_number - 1]
                    size = sizes[name[1:name.index('@@')]]
                    following = [s.value for s in definitions.values()
                                 if s.section_number == symbol.section_number
                                 and s.value > symbol.value]
                    end = min(following, default=section.raw_size)
                    self.assertEqual(end - symbol.value, size)
                    if section.characteristics & 0x80:
                        compiled = bytes(size)
                    else:
                        start = section.raw_ptr + symbol.value
                        compiled = obj.data[start:start + size]
                    va = int(address, 16)
                    retail_section = retail.section_for_rva(va - retail.image_base)
                    offset = va - retail.image_base - retail_section.virtual_address
                    self.assertLessEqual(offset + size, retail_section.virtual_size)
                    backed = min(size, max(0, retail_section.raw_size - offset))
                    original = (retail.bytes_at_va(va, backed) if backed else b'')
                    original += bytes(size - backed)
                    self.assertEqual(compiled, original)
                    self.assertEqual(bool(section.characteristics & 0x80000000),
                                     bool(retail_section.characteristics & 0x80000000))


if __name__ == '__main__':
    unittest.main()
