"""Verify shared collision layouts and production caller/provider identities."""
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
class CollisionLinkTests(unittest.TestCase):
    def test_shared_layouts_and_real_collision_providers(self):
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-') as directory:
            objects = {}
            for source in ('src/krusty2/collision/CollisionObject.cpp',
                           'src/reconstructed/CarProcedural.cpp',
                           'src/reconstructed/Wrecker.cpp',
                           'src/reconstructed/ObjectPicker.cpp',
                           'tests/fixtures/collision_layout.cpp'):
                obj = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run(
                    [sys.executable, str(ROOT / 'tools/compile.py'),
                     str(ROOT / source), '-o', str(obj),
                     '--vc6-root', os.environ['VC6_ROOT'],
                     '--profile', 'vc6_o2_mt',
                     '--include', str(ROOT / 'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[Path(source).stem] = CoffObject(obj)
            providers = {s.name for s in objects['CollisionObject'].symbols
                         if s.storage_class == 2 and s.section_number > 0}

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name
                        for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            car = {
                '??0CollisionObject@@QAE@H@Z',
                '?Configure@CollisionObject@@QAEXPAXHHH@Z',
                '?LoadShape@CollisionObject@@QAEXPAXPBD@Z',
                '?SetHullShape@CollisionObject@@QAEXPAXHHHH@Z',
                '?SnapPlacement@CollisionObject@@QAEXXZ',
                '?AddIgnoredOwner@CollisionObject@@QAEXPAX@Z',
            }
            wrecker = {
                '?SnapPlacement@CollisionObject@@QAEXXZ',
                '?AddIgnoredOwner@CollisionObject@@QAEXPAX@Z',
            }
            picker = {
                '?Configure@CollisionObject@@QAEXPAXHHH@Z',
                '?SetMeshShape@CollisionObject@@QAEXHPAX@Z',
                '?UpdatePlacement@CollisionObject@@QAEXXZ',
                '?QueryCollisions@CollisionObject@@QAEHXZ',
            }
            for caller, names in (('CarProcedural', car), ('Wrecker', wrecker),
                                  ('ObjectPicker', picker)):
                self.assertTrue(names <= imports(objects[caller]))
                self.assertTrue(names <= providers)
                self.assertFalse(any('CarProceduralCollision@@' in s.name or
                                     'UnknownWreckerCollisionObject@@' in s.name
                                     for s in objects[caller].symbols))


if __name__ == '__main__':
    unittest.main()
