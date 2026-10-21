"""Complete camera/Wrecker callers connect to existing native math providers."""
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
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
CROSS = '?CrossProductCall@@YA?AUVec3@@ABU1@0@Z'
NORMALIZE = '?Vec3Normalize@@YA?AUVec3@@ABU1@@Z'
CTOR = '??0Vec3@@QAE@MMM@Z'


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class CameraWreckerMathConnections(unittest.TestCase):
    def test_complete_callers_layout_and_provider_closure(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        sources = {name: 'src/reconstructed/' + name + '.cpp'
                   for name in ('FollowCamera', 'Wrecker', 'CrossProduct', 'Math3D')}
        sources['layout'] = 'tests/fixtures/camera_wrecker_math_layout.cpp'
        sources['WreckerSample'] = 'samples/race/WreckerNearMisses.cpp'
        with tempfile.TemporaryDirectory(prefix='mcm2-camera-wrecker-math-') as directory:
            objects = {}
            for name, source in sources.items():
                obj = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)

            empty_bindings = Path(directory) / 'empty.json'
            empty_bindings.write_text('{}')
            for name, symbol, va, size in (
                ('CrossProduct', CROSS, 0x515600, 91),
                ('Math3D', NORMALIZE, 0x5087b0, 150),
                ('Wrecker', CTOR, 0x404e60, 25),
            ):
                obj = objects[name]
                entry = obj.find_symbol(symbol)
                self.assertGreater(entry.section_number, 0)
                self.assertEqual(entry.extent_source, 'coff_function_aux')
                self.assertEqual(entry.function_size, size)
                bindings = ROOT / Path(sources[name]).with_suffix('.bindings.json')
                proof = compare(retail, obj, symbol, va, size,
                                bindings if bindings.exists() else empty_bindings)
                self.assertTrue(proof['strict_exact'], proof)

            def calls(name, symbol):
                obj = objects[name]
                caller = obj.find_symbol(symbol)
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if r.section_number == caller.section_number and r.type == 0x14
                        and caller.value <= r.virtual_address < caller.value + caller.function_size}

            self.assertIn(CROSS, calls('FollowCamera',
                '?UnknownVirtualSlot46@FollowCamera@@UAEXPAUVector3@@0@Z'))
            self.assertTrue({CROSS, NORMALIZE, CTOR} <= calls('Wrecker', '?Tick@Wrecker@@UAEHM@Z'))
            former = {'?CrossProduct@@YA?AUVector3@@ABU1@0@Z',
                      '?WreckerCrossCall@@YA?AUVector3@@ABU1@0@Z',
                      '?WreckerNormalizeCall@@YA?AUVector3@@ABU1@@Z',
                      '??0WreckerVec3Call@@QAE@MMM@Z'}
            for name in ('FollowCamera', 'Wrecker'):
                self.assertFalse(former & {s.name for s in objects[name].symbols})

            counts = {'FollowCamera': 0, 'Wrecker': 0}
            for case in CASES:
                name = Path(case['source']).stem
                if name not in counts or case['source'] != sources[name]:
                    continue
                proof = compare(retail, objects[name], case['symbol'],
                                int(case['target_va'], 0), case['target_size'], ROOT / case['bindings'])
                self.assertTrue(proof['strict_exact'], proof)
                counts[name] += 1
            self.assertGreaterEqual(counts['FollowCamera'], 38)
            self.assertGreaterEqual(counts['Wrecker'], 29)

            # The sample includes Wrecker.cpp, but still needs its own call-only
            # constructor view. Preserve both partial comparisons honestly.
            for symbol, va, size, positions in (
                ('?PushProbesOutOfBox@Wrecker@@', 0x5329e0, 1312, 1200),
                ('?SprayContactParticles@Wrecker@@', 0x531740, 1624, 1606),
            ):
                proof = compare(retail, objects['WreckerSample'], symbol, va, size,
                                ROOT / 'samples/race/WreckerNearMisses.bindings.json')
                self.assertEqual(proof['candidate_size'], size)
                self.assertFalse(proof['strict_exact'])
                self.assertEqual(proof['strict_match']['matching_positions'], positions)


if __name__ == '__main__':
    unittest.main()
