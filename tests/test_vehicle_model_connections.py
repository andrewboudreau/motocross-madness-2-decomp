"""Check complete vehicle-camera/audio callers against actual scene providers."""
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
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    '?GetAxes@SoultreeObject@@QAEXPAUVec3@@0@Z': (0x4fc4f0, 71),
    '?GetPosition@SoultreeObject@@QAEPAUVec3@@PAU2@@Z': (0x4fc970, 33),
    '?GetPositionIn@SoultreeObject@@QAEXPAV1@PAUVec3@@@Z': (0x4fc9a0, 134),
    '?FindByName@SoultreeObject@@QAEPAV1@PBD@Z': (0x4fdae0, 84),
    '?ClearFlag14c@SoultreeObject@@QAEXXZ': (0x4fdb50, 11),
    '?ClearScreenRects@D3DIMSoultreeObject@@QAEXXZ': (0x4444e0, 55),
}
FOLLOWING = ('?CreateFollowing@SoultreeSoundEmitter@@QAEPAV1@PAXPBD1'
             'PAVSoultreeObject@@UUnknownSound3DParameters@@KMMHH@Z')
CALLERS = ('VehicleCamera', 'BikeCamera', 'KrustyBikeCamera', 'AuralScape',
           'RaceSound', 'TrackOverlay')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and private retail image')
class VehicleModelConnections(unittest.TestCase):
    def test_full_callers_layout_and_existing_provider_closure(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name in ('SoultreeObject', 'D3DIMSoultreeObject'):
            cls = classes[name]
            self.assertEqual({r.object_offset for r in cls.vtable_records}, {0, 12})
            for base, offset in (('SoultreeObject', 0), ('QuadTreeObject', 0),
                                 ('GameObject', 12)):
                relation = next(b for b in cls.bases if b.name == base)
                self.assertEqual((relation.mdisp, relation.pdisp), (offset, -1))
        self.assertEqual(classes['D3DIMSoultreeObject'].direct_bases, ['SoultreeObject'])

        sources = {name: 'src/reconstructed/' + name + '.cpp'
                   for name in CALLERS + ('D3DIMSoulTree', 'SceneManager', 'ProCircuitProcs')}
        sources.update({'soultree': 'src/krusty2/soultree/soultree.cpp',
                        'scene_sample': 'samples/track/SceneManagerNearMisses.cpp',
                        'layout': 'tests/fixtures/vehicle_model_layout.cpp'})
        with tempfile.TemporaryDirectory(prefix='mcm2-vehicle-models-') as temp:
            objects = {}
            for name, source in sources.items():
                output = Path(temp) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(output), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(output)

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            def definitions(obj):
                return {s.name for s in obj.symbols
                        if s.section_number > 0 and s.storage_class == 2}

            all_imports = set()
            for name in CALLERS:
                names = imports(objects[name])
                self.assertFalse(any('UnknownVehiclePart' in symbol for symbol in names))
                self.assertTrue(set(PROVIDERS) & names, name)
                all_imports.update(names)
            self.assertTrue(set(PROVIDERS) <= all_imports)
            self.assertIn(FOLLOWING, definitions(objects['AuralScape']))
            # These caller bodies remain partial samples; compile and check their
            # real relocation dependency without calling them strict matches.
            self.assertIn(FOLLOWING, imports(objects['scene_sample']))

            for symbol, (va, size) in PROVIDERS.items():
                name = 'D3DIMSoulTree' if va == 0x4444e0 else 'soultree'
                bindings = (ROOT / 'src/reconstructed/D3DIMSoulTree.bindings.json'
                            if name == 'D3DIMSoulTree' else
                            ROOT / 'src/krusty2/soultree/soultree.bindings.json')
                self.assertIn(symbol, definitions(objects[name]))
                result = compare(retail, objects[name], symbol, va, size, bindings)
                self.assertTrue(result['strict_exact'], result)

            by_source = {source: name for name, source in sources.items()}
            matched = []
            for case in CASES:
                if case['source'] not in by_source:
                    continue
                result = compare(retail, objects[by_source[case['source']]], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'],
                                 ROOT / case['bindings'])
                self.assertTrue(result['strict_exact'], result)
                matched.append(case)
            self.assertGreaterEqual(len(matched), 405)
            self.assertTrue(any(case['symbol'] == FOLLOWING for case in matched))
            self.assertTrue(any(int(case['target_va'], 0) == 0x4d59a0 for case in matched))


if __name__ == '__main__':
    unittest.main()
