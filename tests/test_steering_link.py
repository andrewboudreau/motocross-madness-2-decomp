"""Verify production vehicle callers reference the real steering implementations."""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.environ.get('VC6_ROOT'), 'requires VC6_ROOT')
class SteeringLinkTests(unittest.TestCase):
    def test_vehicle_imports_have_canonical_steering_providers(self):
        with tempfile.TemporaryDirectory(prefix='mcm2-steering-') as directory:
            objects = {}
            for source in ('motion/SteeringControl.cpp',
                           'vehicle/Vehicle.cpp', 'vehicle/Bike.cpp'):
                obj = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run(
                    [sys.executable, str(ROOT / 'tools/compile.py'),
                     str(ROOT / 'src/krusty2' / source), '-o', str(obj),
                     '--vc6-root', os.environ['VC6_ROOT'],
                     '--include', str(ROOT / 'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[Path(source).stem] = CoffObject(obj)

            expected = {
                '??0SteeringControl@@QAE@MMPBUVec3@@0@Z',
                '??1SteeringControl@@QAE@XZ',
                '?SetAxisFromDirection@SteeringControl@@QAEHPBUVec3@@PAVSoultreeObject@@@Z',
                '?SetAngle@SteeringControl@@QAEXMPAVSoultreeObject@@@Z',
                '?AddAngle@SteeringControl@@QAEXMPAVSoultreeObject@@@Z',
            }
            providers = {s.name for s in objects['SteeringControl'].symbols
                         if s.storage_class == 2 and s.section_number > 0}
            self.assertTrue(expected <= providers)

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name
                        for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            vehicle_imports = imports(objects['Vehicle'])
            bike_imports = imports(objects['Bike'])
            self.assertTrue(expected <= vehicle_imports)
            self.assertIn('?AddAngle@SteeringControl@@QAEXMPAVSoultreeObject@@@Z',
                          bike_imports)
            self.assertIn('?SetAngle@SteeringControl@@QAEXMPAVSoultreeObject@@@Z',
                          bike_imports)
            for obj in objects.values():
                self.assertFalse(any('VehicleSteerState@@' in s.name
                                     for s in obj.symbols))
            # The unresolved partial reconstruction must not become an
            # accidental source provider through inclusion of its sample.
            partial = '?SetAxisFromPoints@SteeringControl@@QAEHPAVSoultreeObject@@00@Z'
            self.assertIn(partial, bike_imports)
            self.assertNotIn(partial, providers)


if __name__ == '__main__':
    unittest.main()
