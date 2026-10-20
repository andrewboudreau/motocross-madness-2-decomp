"""Check that scene collision calls import the supplied production methods."""
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
class CollisionConnectionTests(unittest.TestCase):
    def test_arcade_and_picker_import_existing_collision_bodies(self):
        sources = {
            'collision': 'src/krusty2/collision/CollisionObject.cpp',
            'arcade': 'src/reconstructed/ArcadeObject.cpp',
            'picker': 'src/reconstructed/ObjectPicker.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-') as directory:
            objects = {}
            for name, source in sources.items():
                obj = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(obj),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)

            configure = '?Configure@CollisionObject@@QAEXPAXHHH@Z'
            expected = {
                configure,
                '?LoadShape@CollisionObject@@QAEXPAXPBD@Z',
                '?SetHullShape@CollisionObject@@QAEXPAXHHHH@Z',
                '?SetOwner@CollisionObject@@QAEXPAXH@Z',
            }
            supplied = {s.name for s in objects['collision'].symbols
                        if s.storage_class == 2 and s.section_number > 0}
            self.assertTrue(expected <= supplied)

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name
                        for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            self.assertTrue(expected <= imports(objects['arcade']))
            self.assertIn(configure, imports(objects['picker']))
            self.assertNotIn('?Configure@CollisionObject@@QAEXHHHH@Z', supplied)


if __name__ == '__main__':
    unittest.main()
