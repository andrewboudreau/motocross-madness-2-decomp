"""Full caller/provider byte proofs for three existing terrain operations."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/Grid1.cpp', 'src/reconstructed/QuarryStuntEvent.cpp',
           'src/reconstructed/TrackOverlay.cpp', 'src/krusty2/broadphase/Terrain.cpp',
           'src/reconstructed/TerrainSupport.cpp')
PROVIDERS = (
    ('?RetireOwnedObject@Terrain@@QAEXPAVBaseObject@@@Z', 0x5057d0, 83),
    ('?SelectQuality@Terrain@@QAEXH@Z', 0x507960, 129),
    ('?CastSegment@Terrain@@QAEHPBUTerrainVec3@@0PAU2@HHH@Z', 0x506e90, 1654),
)
# Preserve the existing incomplete functions too. Their COFF bodies plus named
# relocation records are hashed because QueryGround still has unbound helpers.
# This is deliberately not a successful retail comparison of either function.
PARTIAL = (
    ('?Load@Terrain@@', 2117, '0e0914cc5a15a2f8b5210e4d1a2b2354dc559821e8c031e072a4004b348c98f3'),
    ('QueryGround', 3321, '07ea9bdeb16235841aaf1d125c77ee580b9b1667ec900de488c74df2f57207b3'),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainConnectionsTests(unittest.TestCase):
    def test_existing_complete_providers_and_unchanged_callers(self):
        image_path = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(image_path)
        col = struct.unpack('<5I', retail.bytes_at_va(0x55f338, 20))
        self.assertEqual(col[1], 0)
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(col[4], 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            name = retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0]
            bases[name] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVTerrain@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGroundFogableObject@@'], (0x2c, -1, 0))

        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-connections-') as directory:
            objects = {}
            for source in (*SOURCES, 'tests/fixtures/terrain_connection_layout.cpp'):
                path = Path(directory) / (Path(source).stem + '.obj')
                command = [
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt',
                ]
                # Reconstruction calibration supplies no physics search path.
                # Require its callers and the layout fixture to work that way.
                if source.startswith('src/krusty2/'):
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            provider = objects[SOURCES[3]]
            bindings = ROOT / 'src/krusty2/broadphase/Terrain.bindings.json'
            for caller, (symbol, va, size) in zip(SOURCES[:3], PROVIDERS):
                entry = provider.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertEqual(provider.symbol_extent(entry)[1], size)
                self.assertTrue(compare(retail, provider, symbol, va, size, bindings)['strict_exact'])
                obj = objects[caller]
                self.assertIn(symbol, {obj.symbol_by_index[r.symbol_index].name
                                       for r in obj.relocations if r.type == 0x14})
                retired = ('?RetireOwnedObject@Grid1Terrain@@QAEXPAVBaseObject@@@Z',
                           '?SelectQuality@UnknownTerrain@@QAEXH@Z',
                           '?CastSegment@UnknownTerrain@@QAEHPBUVector3@@0PAU2@HHH@Z')
                self.assertFalse(set(retired) & {s.name for s in obj.symbols})
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            manifest = ROOT / 'src/krusty2/broadphase/targets.json'
            for target in json.loads(manifest.read_text()):
                if 'source' not in target:
                    continue
                source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                if source in SOURCES and target.get('expect') != 'partial':
                    cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                  target['target_size'], manifest.parent / target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, bindings)['strict_exact'],
                                (source, symbol))
            self.assertGreaterEqual(len(dict.fromkeys(cases)), 178)
            for symbol, expected_size, digest in PARTIAL:
                entry = provider.find_symbol(symbol)
                body, size, relocations = provider.symbol_extent(entry)
                self.assertEqual(size, expected_size)
                records = [(r.virtual_address - entry.value, r.type,
                            provider.symbol_by_index[r.symbol_index].name) for r in relocations]
                self.assertEqual(hashlib.sha256(body + json.dumps(records).encode()).hexdigest(), digest)


if __name__ == '__main__':
    unittest.main()
