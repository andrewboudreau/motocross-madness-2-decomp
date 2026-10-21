"""Check canonical math bodies, initializer scope, and real caller linkage."""
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
PROVIDERS = ('AxisSettle', 'CrossProduct', 'Math3D', 'OrientationAngles')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class PhysicsMathProviderTests(unittest.TestCase):
    def test_exact_bodies_local_initializers_and_real_callers(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        sources = {name: f'src/reconstructed/{name}.cpp' for name in PROVIDERS}
        sources.update({
            'Vehicle': 'src/krusty2/vehicle/Vehicle.cpp',
            'KrustyBike': 'src/krusty2/vehicle/KrustyBike.cpp',
            'FastMath': 'src/reconstructed/FastMath.cpp',
        })
        with tempfile.TemporaryDirectory(prefix='mcm2-physics-math-') as directory:
            objects = {}
            for name, source in sources.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)

            empty_bindings = Path(directory) / 'empty.json'
            empty_bindings.write_text('{}')
            checked = set()
            for manifest in ('helpers', 'common'):
                path = ROOT / f'samples/physics/{manifest}/targets.json'
                for target in json.loads(path.read_text()):
                    source = (path.parent / target['source']).resolve()
                    if source.parent != ROOT / 'src/reconstructed' or source.stem not in PROVIDERS:
                        continue
                    bindings = (path.parent / target['bindings']
                                if target.get('bindings') else empty_bindings)
                    result = compare(retail, objects[source.stem],
                                     target['candidate_symbol_contains'],
                                     int(target['target_va'], 0), target['target_size'], bindings)
                    with self.subTest(target=target['target_va']):
                        self.assertTrue(result['strict_exact'], result)
                        self.assertEqual(result['candidate_size'], target['target_size'])
                    checked.add(int(target['target_va'], 0))
            self.assertEqual(checked, {
                0x4a13e0, 0x4a1d10, 0x4a1f90, 0x4b5a60, 0x4cb6e0, 0x5087b0, 0x515600,
                0x4cbb10, 0x4cbb20, 0x4cbb60, 0x4cbb70,
                0x4cbbb0, 0x4cbbc0, 0x4cbc00, 0x4cbc10,
            })

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            calls = {
                '?OrientationAnglesFromVectors@@YAHUVec3@@0PAM111111@Z',
                '?UnknownAxisSettle_4cb6e0@@YAXPAM0PBMME@Z',
                '?CrossProductCall@@YA?AUVec3@@ABU1@0@Z',
                '?Vec3Normalize@@YA?AUVec3@@ABU1@@Z',
            }
            self.assertTrue(calls <= imports(objects['Vehicle']) | imports(objects['KrustyBike']))
            supplied = {s.name for name in PROVIDERS for s in objects[name].symbols
                        if s.storage_class == 2 and s.section_number > 0}
            self.assertTrue(calls <= supplied)
            self.assertIn('?FastInvSqrt@@YAMM@Z', imports(objects['Math3D']))
            self.assertIn('?FastInvSqrt@@YAMM@Z', {
                s.name for s in objects['FastMath'].symbols
                if s.storage_class == 2 and s.section_number > 0})

            for name in PROVIDERS:
                obj = objects[name]
                # These header-generated copies must remain local to each object.
                # Only AxisSettle's initializer addresses are attributed to retail.
                constants = [s for s in obj.symbols if s.name.startswith('_kVec3')]
                self.assertEqual({s.name for s in constants}, {
                    '_kVec3Zero', '_kVec3XAxis', '_kVec3YAxis', '_kVec3ZAxis'})
                for symbol in constants:
                    self.assertEqual(symbol.storage_class, 3)
                    self.assertEqual(obj.sections[symbol.section_number - 1].name, '.bss')
                initializers = [s for s in obj.sections if s.name == '.CRT$XCU']
                self.assertEqual([(s.raw_size, s.reloc_count) for s in initializers], [(16, 4)])
                for section in initializers:
                    for relocation in obj.relocations:
                        if relocation.section_number == section.index:
                            symbol = obj.symbol_by_index[relocation.symbol_index]
                            self.assertGreater(symbol.section_number, 0)
                            self.assertEqual(symbol.storage_class, 3)
                self.assertTrue(imports(obj) <= {
                    '__fltused', '__CIacos', '?FastInvSqrt@@YAMM@Z'})


if __name__ == '__main__':
    unittest.main()
