"""Verify Vehicle method ownership, strict bytes, and actual vtable linkage."""
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
class VehicleMethodProviderTests(unittest.TestCase):
    def test_primary_slot_identity_exact_bodies_and_source_connections(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x558a80, 4))[0], 0x55f9b0)
        self.assertEqual(struct.unpack('<5I', retail.bytes_at_va(0x55f9b0, 20)),
                         (0, 0, 0, 0x566c98, 0x55f9a0))
        name = b'.?AVVehicle@@\0'
        self.assertEqual(retail.bytes_at_va(0x566ca0, len(name)), name)
        slots = {23, 24, 25, 34, 35, 36, 45, 51, 52, 68, 69, 78, 79, 92}
        sources = {
            'methods': 'src/reconstructed/VehicleMethods.cpp',
            'owner': 'src/krusty2/vehicle/Vehicle.cpp',
            'bike': 'src/krusty2/vehicle/Bike.cpp',
            'reset': 'src/reconstructed/VehicleReset.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-vehicle-methods-') as directory:
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
            table = owner.find_symbol('??_7Vehicle@@6BSoultreePhysicsBaseObject@@@')
            table_refs = {r.virtual_address - table.value:
                          owner.symbol_by_index[r.symbol_index].name
                          for r in owner.relocations if r.section_number == table.section_number}
            definitions = {s.name for key in ('methods', 'reset') for s in objects[key].symbols
                           if s.storage_class == 2 and s.section_number > 0}
            path = ROOT / 'samples/physics/vehicle/targets.json'
            checked = set()
            for target in json.loads(path.read_text()):
                source = (path.parent / target['source']).resolve()
                if source not in {ROOT / sources['methods'], ROOT / sources['reset']}:
                    continue
                slot = int(re.search(r'Slot(\d+)@', target['candidate_symbol_contains'])[1])
                promoted = slot in slots
                self.assertEqual(source, ROOT / sources['reset' if slot == 36 else 'methods'])
                obj = objects['reset' if slot == 36 else 'methods']
                result = compare(retail, obj, target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 path.parent / target['bindings'])
                with self.subTest(slot=slot):
                    self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x558a84 + slot * 4, 4))[0],
                                     int(target['target_va'], 0))
                    self.assertTrue(result['strict_exact'], result)
                    self.assertEqual(result['candidate_size'], target['target_size'])
                    if promoted:
                        self.assertEqual(table_refs[slot * 4], result['symbol'])
                        self.assertIn(table_refs[slot * 4], definitions)
                    checked.add(slot)
            self.assertEqual(checked, slots)
            self.assertIn('?UnknownVirtualSlot36@Vehicle@@UAEXXZ', definitions)

            methods = objects['methods']
            self.assertEqual({s.name for s in methods.symbols if s.section_number == 0}, {
                '__fltused', '?GetAxesIn@SoultreeObject@@QAEXPAV1@PAUVec3@@1@Z',
                '?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z',
            })
            # These two providers already exist. The separate reset object
            # must not introduce globals or view aliases into this unit.
            constants = [s for s in methods.symbols if s.name.startswith('_kVec3')]
            self.assertEqual(len(constants), 4)
            self.assertTrue(all(s.storage_class == 3 for s in constants))
            self.assertEqual([(s.raw_size, s.reloc_count) for s in methods.sections
                              if s.name == '.CRT$XCU'], [(16, 4)])

            # Bike's zero-vector reads use the storage its own strict initializer
            # supplies, not an undefined external name for the same retail VA.
            bike = objects['bike']
            zero = bike.find_symbol('_kVec3Zero')
            self.assertEqual(zero.storage_class, 3)
            self.assertGreater(zero.section_number, 0)
            self.assertFalse(any('g_BikeVec3_005778a8' in s.name for s in bike.symbols))
            initializer = compare(retail, bike, '_$E1', 0x407890, 60,
                                  ROOT / 'src/krusty2/vehicle/Bike.bindings.json')
            self.assertTrue(initializer['strict_exact'], initializer)
            self.assertGreater(sum(bike.symbol_by_index[r.symbol_index].name == '_kVec3Zero'
                                   for r in bike.relocations), 10)


if __name__ == '__main__':
    unittest.main()
