"""The real meter pointer is initialized data referring to the existing object."""
from __future__ import annotations

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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import sha256_file

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
POINTER = '?g_HiResMeter@@3PAVGameHiResMeterView@@A'
INSTANCE = '?g_HiResMeterInstance@@3VHiResMeter@@A'


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class HiResMeterStorageTests(unittest.TestCase):
    def test_native_pointer_relocation_storage_and_zero_offset_view(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        meter = next(c for c in parse_rtti(retail) if c.name == 'HiResMeter')
        self.assertTrue(any(r.object_offset == 0 and 0x554170 in r.vtables
                            for r in meter.vtable_records))
        base = next(b for b in meter.bases if b.name == 'GameObject')
        self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        bindings = json.loads((ROOT / 'src/reconstructed/HiResMeterStorage.bindings.json').read_text())
        self.assertEqual(bindings, {POINTER: '0x0056c470', INSTANCE: '0x0067b468'})
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x56c470, 4))[0], 0x67b468)
        with tempfile.TemporaryDirectory(prefix='mcm2-hires-storage-') as directory:
            objects = {}
            for name, source in {
                'storage': 'src/reconstructed/HiResMeterStorage.cpp',
                'layout': 'tests/fixtures/hires_meter_storage.cpp',
                'instance': 'src/reconstructed/GUIManager.cpp',
            }.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)
            obj = objects['storage']
            definitions = {s.name: s for s in obj.symbols
                           if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), {POINTER})
            self.assertFalse(any(s.raw_size and s.characteristics & 0x20 for s in obj.sections))
            pointer = definitions[POINTER]
            section = obj.sections[pointer.section_number - 1]
            self.assertEqual((pointer.value, section.raw_size), (0, 4))
            self.assertTrue(section.characteristics & 0x40)  # initialized data
            self.assertTrue(section.characteristics & 0x80000000)  # writable
            original_section = retail.section_for_rva(0x56c470 - retail.image_base)
            self.assertTrue(original_section.characteristics & 0x80000000)
            rels = [r for r in obj.relocations if r.section_number == pointer.section_number]
            self.assertEqual(len(rels), 1)
            rel = rels[0]
            self.assertEqual((rel.virtual_address, rel.type), (0, 6))  # IMAGE_REL_I386_DIR32
            target = obj.symbol_by_index[rel.symbol_index]
            self.assertEqual((target.name, target.section_number), (INSTANCE, 0))
            addend = struct.unpack_from('<I', obj.data, section.raw_ptr)[0]
            self.assertEqual(addend, 0)
            self.assertEqual(struct.pack('<I', int(bindings[target.name], 16) + addend),
                             retail.bytes_at_va(int(bindings[POINTER], 16), 4))
            instance = next(s for s in objects['instance'].symbols if s.name == INSTANCE)
            self.assertGreater(instance.section_number, 0)
            for name in ('?MeterObjectBase@@YAPAVGameObject@@PAVHiResMeter@@@Z',
                         '?MeterViewBase@@YAPAVGameObject@@PAVGameHiResMeterView@@@Z'):
                layout = objects['layout']
                symbol = next(s for s in layout.symbols if s.name == name)
                sec = layout.sections[symbol.section_number - 1]
                start = sec.raw_ptr + symbol.value
                self.assertEqual(symbol.function_size, 5)
                self.assertEqual(layout.data[start:start + 5], bytes.fromhex('8b442404c3'))


if __name__ == '__main__':
    unittest.main()
