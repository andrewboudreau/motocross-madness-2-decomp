"""Verify compiled renderer and input globals against the private retail image."""
from __future__ import annotations

import json
import os
import struct
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
    def test_font_rectangle_indices_storage_and_actual_loader_reads(self):
        self.assert_data_provider('Rectangle2DState', {
            'g_UnknownGlobal57972c': 4, 'g_UnknownGlobal568468': 4,
            'g_UnknownGlobal56846c': 4, 'g_UnknownGlobal568470': 4,
        })
        retail = PEImage(EXE)
        # Complete absolute-DWORD scan: each cell has one encoded load in the
        # font loader, with no encoded direct write or address-taking reference.
        # This does not exclude arbitrary indirect writes or prove TU ownership.
        for va, site, opcode in ((0x57972c, 0x467547, b'\x8b\x0d'),
                                 (0x568468, 0x467540, b'\x8b\x15'),
                                 (0x56846c, 0x467532, b'\xa1'),
                                 (0x568470, 0x46752c, b'\x8b\x0d')):
            needle = struct.pack('<I', va)
            offsets, start = [], 0
            while (offset := retail.data.find(needle, start)) >= 0:
                offsets.append(retail.offset_to_va(offset))
                start = offset + 1
            self.assertEqual(offsets, [site + len(opcode)])
            self.assertEqual(retail.bytes_at_va(site, len(opcode) + 4), opcode + needle)
        # The four loaded DWORDs index the stack's four floats, pushed right to
        # left into the real Rectangle2D constructor. Retain every intervening
        # instruction, including the changing stack displacement after pushes.
        self.assertEqual(retail.bytes_at_va(0x467537, 4), bytes.fromhex('8b548c20'))
        self.assertEqual(retail.bytes_at_va(0x46753b, 5), bytes.fromhex('8b4c842052'))
        self.assertEqual(retail.bytes_at_va(0x46754d, 14),
                         bytes.fromhex('8b4494288b548c2850528d4c2440'))
        call = retail.bytes_at_va(0x46755b, 5)
        self.assertEqual(call[0], 0xe8)
        self.assertEqual(0x467560 + struct.unpack('<i', call[1:])[0], 0x4e8ad0)

    def test_bike_class_table_size_values_and_writable_storage(self):
        self.assert_data_provider('BikeClassData', {'g_BikeClassEngineSizes': 20})

    def test_compiled_initial_values_sizes_and_mutability_match_retail(self):
        sizes = {
            'g_DefaultNearPlane': 4, 'g_DefaultFarPlane': 4, 'g_BladeDeviceGuid': 16,
            'g_GammaRamp': 3 * 256 * 2, 'g_DebugCountTextureMemory': 4,
            'g_SharedMipTextureSurfaces': 10 * 4, 'g_SharedTextureSurfaces': 10 * 4,
            'g_Displays': 4 * 4, 'g_DisplayCount': 4,
            'g_NonBladeLodSettings': 10 * 16, 'g_BladeLodSettings': 10 * 16,
            'g_ActiveLodSettings': 4,
        }
        self.assert_data_provider('GraphicsState', sizes)

    def test_texture_dump_flag_loader_zero_storage_and_direct_reference(self):
        self.assert_data_provider('TextureSurfaceState', {'g_DebugDumpTextureLevels': 4})
        retail = PEImage(EXE)
        va = 0x689968
        section = retail.section_for_rva(va - retail.image_base)
        self.assertTrue(section.characteristics & 0x80000000)
        self.assertGreaterEqual(va - retail.image_base - section.virtual_address,
                                section.raw_size)
        self.assertLessEqual(va + 4 - retail.image_base - section.virtual_address,
                             section.virtual_size)
        # Scan all file-backed sections for the explicit address. This proves
        # the located direct reference, not absence of possible indirect writes.
        sites = []
        needle = struct.pack('<I', va)
        for candidate in retail.sections:
            data = retail.data[candidate.raw_offset:candidate.raw_offset + candidate.raw_size]
            start = 0
            while (offset := data.find(needle, start)) >= 0:
                sites.append(retail.image_base + candidate.virtual_address + offset)
                start = offset + 1
        self.assertEqual(sites, [0x4c7097])
        self.assertEqual(retail.bytes_at_va(0x4c7096, 5), b'\xa1' + needle)

    def test_keyboard_table_detection_sentinel_and_platform_storage_match_retail(self):
        self.assert_data_provider('DeviceSetupState', {
            'g_Pc98ScanCodes': 256 * 4, 'g_KeyboardSubtype': 4, 'g_IsWindowsNT': 4,
        })

    def assert_data_provider(self, stem, sizes):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        bindings = json.loads((ROOT / f'src/reconstructed/{stem}.bindings.json').read_text())
        retail = PEImage(EXE)
        with tempfile.TemporaryDirectory(prefix='mcm2-graphics-') as tmp:
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
            if stem == 'Rectangle2DState':
                self.assertFalse([r for r in obj.relocations
                                  if not obj.section(r.section_number).name.startswith('.debug')])
                self.assertFalse(any(s.storage_class == 2 and s.section_number == 0
                                     for s in obj.symbols))
                self.assertFalse(any(s.name.startswith('.CRT') for s in obj.sections))
                self.assertEqual(sorted((s.name, s.raw_size) for s in obj.sections
                                        if s.raw_size and s.name in ('.data', '.bss')),
                                 [('.bss', 4), ('.data', 12)])
            if stem == 'TextureSurfaceState':
                self.assertFalse([r for r in obj.relocations
                                  if not obj.section(r.section_number).name.startswith('.debug')])
                self.assertFalse(any(s.storage_class == 2 and s.section_number == 0
                                     for s in obj.symbols))
                self.assertFalse(any(s.name.startswith('.CRT') for s in obj.sections))
                nondebug = [s for s in obj.sections
                            if s.raw_size and not s.name.startswith('.debug')
                            and s.name != '.drectve']
                self.assertEqual([(s.name, s.raw_size, s.raw_ptr) for s in nondebug],
                                 [('.bss', 4, 0)])
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
