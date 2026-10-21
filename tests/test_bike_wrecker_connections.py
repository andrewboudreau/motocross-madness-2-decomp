"""Native proof that Bike's real callers link to the existing Wrecker bodies."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    '??0Wrecker@@': (0x52ff90, 261),
    '?SetupWrecker@Wrecker@@': (0x530190, 36),
    '?AddContactAnimation@Wrecker@@': (0x5305b0, 60),
    '?AddRandomAnimation@Wrecker@@': (0x5305f0, 60),
    '?AddIndexedAnimation@Wrecker@@': (0x530630, 65),
    '?AddIgnoredObject@Wrecker@@': (0x530680, 82),
    '?StartAnimatedWreck@Wrecker@@': (0x532220, 230),
    '?StartTumble@Wrecker@@': (0x532310, 372),
    '?EndWreck@Wrecker@@': (0x5327c0, 193),
    '?SetPushBox@Wrecker@@': (0x5328b0, 73),
    '?AddProbe@Wrecker@@': (0x532900, 210),
}


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class BikeWreckerConnectionTests(unittest.TestCase):
    def test_callers_complete_bodies_layout_and_providers(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        classes = {c.name: c for c in parse_rtti(retail)}
        wrecker = classes['Wrecker']
        self.assertTrue(any(r.object_offset == 0 and 0x558e24 in r.vtables
                            for r in wrecker.vtable_records))
        for name in ('GraphicsTest', 'GameObject', 'BaseObject'):
            base = next(b for b in wrecker.bases if b.name == name)
            self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        # Bike's front/rear wheel pointers are complete Tire objects. Their
        # CollisionObject base has no adjustment at the ignore-list boundary.
        base = next(b for b in classes['Tire'].bases if b.name == 'CollisionObject')
        self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        sources = {
            'Bike': 'src/krusty2/vehicle/Bike.cpp',
            'KrustyBike': 'src/krusty2/vehicle/KrustyBike.cpp',
            'Wrecker': 'src/reconstructed/Wrecker.cpp',
            'layout': 'tests/fixtures/bike_wrecker_connections.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-wrecker-') as directory:
            objects = {}
            for name, source in sources.items():
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
            callers = set()
            for name in ('Bike', 'KrustyBike'):
                obj = objects[name]
                callers.update(obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                               if obj.symbol_by_index[r.symbol_index].section_number == 0)
                self.assertFalse(any('BikeA604@@' in s.name or 'KbA604@@' in s.name
                                     for s in obj.symbols))
            for prefix, (address, size) in PROVIDERS.items():
                provider = objects['Wrecker']
                entries = [s for s in provider.symbols if s.name.startswith(prefix)
                           and s.storage_class == 2 and s.section_number > 0]
                self.assertEqual(len(entries), 1, prefix)
                symbol = entries[0].name
                self.assertIn(symbol, callers)
                proof = compare(retail, provider, symbol, address, size,
                                ROOT / 'src/reconstructed/Wrecker.bindings.json')
                self.assertTrue(proof['strict_exact'], proof)
            for name, symbol, address, size in (
                ('Bike', '?UnknownVirtualSlot97@Bike@@UAEXXZ', 0x409420, 1516),
                ('Bike', '?UnknownVirtualSlot99@Bike@@UAEHMMHM@Z', 0x409b30, 1374),
                ('KrustyBike', '?UnknownVirtualSlot97@KrustyBike@@UAEXXZ', 0x4912d0, 624),
            ):
                proof = compare(retail, objects[name], symbol, address, size,
                                ROOT / f'src/krusty2/vehicle/{name}.bindings.json')
                self.assertTrue(proof['strict_exact'], proof)
            obj = objects['layout']
            symbol = next(s for s in obj.symbols
                          if s.name == '?BikeWreckerBase@@YAPAVGameObject@@PAVWrecker@@@Z')
            section = obj.sections[symbol.section_number - 1]
            start = section.raw_ptr + symbol.value
            self.assertEqual(symbol.function_size, 5)
            self.assertEqual(obj.data[start:start + 5], bytes.fromhex('8b442404c3'))


if __name__ == '__main__':
    unittest.main()
