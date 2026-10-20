"""Check the startup binding-id method and its independently supplied storage."""
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
from tools.match import compare


ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class ControlInterfaceProviderTests(unittest.TestCase):
    def test_next_id_uses_exact_member_code_and_writable_zero_filled_counter(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        counter_va = 0x57985C
        counter_rva = counter_va - retail.image_base
        original = retail.section_for_rva(counter_rva)
        offset = counter_rva - original.virtual_address
        # The retail word belongs to the loader-zeroed tail, not absent input
        # silently substituted with zeros. The complete word fits that section.
        self.assertGreaterEqual(offset, original.raw_size)
        self.assertLessEqual(offset + 4, original.virtual_size)
        self.assertTrue(original.characteristics & 0x80000000)
        with tempfile.TemporaryDirectory(prefix='mcm2-binding-id-') as directory:
            obj_path = Path(directory) / 'ControlInterface.obj'
            result = subprocess.run([
                sys.executable, str(ROOT / 'tools/compile.py'),
                str(ROOT / 'src/reconstructed/ControlInterface.cpp'),
                '-o', str(obj_path), '--vc6-root', os.environ['VC6_ROOT'],
                '--profile', 'vc6_o2_mt',
                '--include', str(ROOT / 'src/krusty2'),
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            obj = CoffObject(obj_path)
            self.assertEqual(obj.machine, 0x14C)
            counter = obj.find_symbol('?g_LastBindingId@@3HA')
            self.assertEqual(counter.storage_class, 2)
            self.assertGreater(counter.section_number, 0)
            section = obj.section(counter.section_number)
            self.assertTrue(section.characteristics & 0x80)
            self.assertTrue(section.characteristics & 0x80000000)
            following = [s.value for s in obj.symbols
                         if s.section_number == counter.section_number
                         and s.value > counter.value]
            self.assertEqual(min(following, default=section.raw_size) - counter.value, 4)
            checked = compare(
                retail, obj, '?NextBindingId@ControlInterface@@QAEHXZ', 0x43CE90, 12,
                ROOT / 'src/reconstructed/ControlInterface.bindings.json')
            self.assertTrue(checked['strict_exact'], checked)
            self.assertEqual(checked['strict_match']['ignored_bytes'], 0)


if __name__ == '__main__':
    unittest.main()
