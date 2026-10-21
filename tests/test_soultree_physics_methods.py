"""Verify promoted methods against their actual retail and source vtable slots."""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import struct
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
class SoultreePhysicsMethodTests(unittest.TestCase):
    def test_exact_methods_and_primary_vtable_ownership(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557d8c, 4))[0], 0x55f0d0)
        # Primary object offset 0; the GameObject virtual-base table is separate.
        self.assertEqual(struct.unpack('<5I', retail.bytes_at_va(0x55f0d0, 20)),
                         (0, 0, 0, 0x566c48, 0x55f0c0))
        name = b'.?AVSoultreePhysicsBaseObject@@\0'
        self.assertEqual(retail.bytes_at_va(0x566c50, len(name)), name)
        slots = {5, 10, 12, 19, 22, 23, 24, 26, 32, 34, 35}
        sources = {
            'methods': 'src/reconstructed/SoultreePhysicsMethods.cpp',
            'owner': 'src/krusty2/soultree/SoulTreePhysics.cpp',
            'remaining': 'samples/physics/soultree_base/SoultreePhysicsInlines.cpp',
            'node': 'src/krusty2/soultree/soultree.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-soultree-methods-') as directory:
            objects = {}
            for key, source in sources.items():
                path = Path(directory) / (key + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key] = CoffObject(path)

            owner = objects['owner']
            table = owner.find_symbol('??_7SoultreePhysicsBaseObject@@6B0@@')
            table_refs = {r.virtual_address - table.value:
                          owner.symbol_by_index[r.symbol_index].name
                          for r in owner.relocations if r.section_number == table.section_number}
            definitions = {s.name for s in objects['methods'].symbols
                           if s.storage_class == 2 and s.section_number > 0}
            path = ROOT / 'samples/physics/soultree_base/targets.json'
            checked = set()
            for target in json.loads(path.read_text()):
                slot = int(re.search(r'Slot(\d+)@', target['candidate_symbol_contains'])[1])
                promoted = slot in slots
                source = (path.parent / target['source']).resolve()
                self.assertEqual(source, ROOT / sources['methods' if promoted else 'remaining'])
                obj = objects['methods' if promoted else 'remaining']
                result = compare(retail, obj, target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 path.parent / target['bindings'])
                with self.subTest(slot=slot):
                    self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557d90 + slot * 4, 4))[0],
                                     int(target['target_va'], 0))
                    self.assertEqual(result['strict_exact'], slot != 6, result)
                    if slot == 6:
                        # The complete nonexact candidate now retains FastSqrt's
                        # result; the previous 432-byte body used squared length.
                        # Native behavior evidence is in test_soultree_drag_behavior.
                        self.assertEqual(result['candidate_size'], 440)
                        self.assertEqual(result['strict_match']['matching_positions'], 126)
                    else:
                        self.assertEqual(result['candidate_size'], target['target_size'])
                    if promoted:
                        self.assertEqual(table_refs[slot * 4], result['symbol'])
                        self.assertIn(table_refs[slot * 4], definitions)
                        checked.add(slot)
            self.assertEqual(checked, slots)
            self.assertFalse(any(re.search(r'Slot(?:6|36)@', name) for name in definitions))

            methods = objects['methods']
            imports = {methods.symbol_by_index[r.symbol_index].name for r in methods.relocations
                       if methods.symbol_by_index[r.symbol_index].section_number == 0}
            node_calls = {
                '?GetAxesIn@SoultreeObject@@QAEXPAV1@PAUVec3@@1@Z',
                '?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z',
            }
            self.assertEqual(imports, node_calls)
            self.assertEqual({s.name for s in methods.symbols if s.section_number == 0},
                             node_calls | {'__fltused'})
            node_definitions = {s.name for s in objects['node'].symbols
                                if s.storage_class == 2 and s.section_number > 0}
            self.assertIn('?GetAxesIn@SoultreeObject@@QAEXPAV1@PAUVec3@@1@Z', node_definitions)
            # SetAxesIn is still sample-only; this existing unresolved dependency
            # is shared by other canonical physics and motion callers.
            self.assertNotIn('?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z', node_definitions)


if __name__ == '__main__':
    unittest.main()
