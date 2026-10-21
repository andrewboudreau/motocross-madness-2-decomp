"""Bike's real focus-point caller uses naturally emitted canonical vector COMDATs."""
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
VECTORS = (
    ('??0Vec3@@QAE@MMM@Z', 0x404e60, 25),
    ('??H@YA?AUVec3@@ABU0@0@Z', 0x421cb0, 69),
    ('??G@YA?AUVec3@@ABU0@0@Z', 0x421d00, 69),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class BikeMathConnectionTests(unittest.TestCase):
    def test_complete_caller_and_canonical_comdat_providers(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        sources = ('src/krusty2/vehicle/Bike.cpp', 'src/krusty2/collision/CollisionObject.cpp')
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-math-') as directory:
            objects = {}
            for source in sources:
                output = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(output), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(output)

            # Both actual TUs emit the same complete inline COMDAT implementations.
            # Nothing supplies a second body under the former Bike-specific names.
            for source, obj in objects.items():
                for symbol, address, size in VECTORS:
                    entry = obj.find_symbol(symbol)
                    self.assertEqual(entry.function_size, size)
                    self.assertTrue(obj.sections[entry.section_number - 1].characteristics & 0x1000)
                    result = compare(retail, obj, symbol, address, size,
                                     ROOT / Path(source).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'], result)

            obj = objects[sources[0]]
            self.assertFalse(any('BikeOolVec3' in s.name or 'BikeVec3Add@@' in s.name
                                 for s in obj.symbols))
            caller = obj.find_symbol('?UnknownVirtualSlot46@Bike@@UAEPAUVec3@@PAU2@M@Z')
            calls = [obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                     if r.section_number == caller.section_number and r.type == 0x14
                     and caller.value <= r.virtual_address < caller.value + caller.function_size]
            for symbol, _, _ in VECTORS:
                self.assertIn(symbol, calls)
            matched = 0
            for target in json.loads((ROOT / 'src/krusty2/vehicle/targets.json').read_text()):
                if target.get('source') != 'Bike.cpp' or target.get('expect', 'exact') != 'exact':
                    continue
                result = compare(retail, obj, target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 ROOT / 'src/krusty2/vehicle' / target['bindings'])
                self.assertTrue(result['strict_exact'], result)
                matched += 1
            self.assertGreaterEqual(matched, 51)


if __name__ == '__main__':
    unittest.main()
