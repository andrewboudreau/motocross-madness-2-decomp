"""Existing camera, matrix, bike and BVH calls join exact shared providers."""
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import match_object
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
DOT = '?Vec3DotCall@@YAMPBUVec3@@0@Z'
SCALE = '?Vec3ScaleCall@@YAPAUVec3@@PAU1@PBU1@M@Z'
SUBTRACT = '?Vec3SubtractCall@@YAPAUVec3@@PAU1@PBU1@1@Z'
SOURCES = (
    'src/reconstructed/Camera.cpp', 'src/reconstructed/FollowCamera.cpp',
    'src/reconstructed/MatrixUtil.cpp', 'src/reconstructed/Wrecker.cpp',
    'src/krusty2/vehicle/Bike.cpp', 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp',
)
# Complete relocation-resolved candidate hashes captured before the connections.
# These protect existing partial functions; they are not retail-match claims.
PARTIAL = [
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot38@Bike', 4215184, 2874, 2865, 1758, '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot7@Bike', 4219072, 1678, 1678, 1677, 'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot89@Bike', 4221664, 2396, 2120, 115, '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot102@Bike', 4224064, 485, 474, 188, 'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot61@Bike', 4243456, 1015, 946, 162, '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'),
    ('src/krusty2/vehicle/Bike.cpp', 'Method_0x0040a520@Bike', 4236576, 2272, 2272, 2112, 'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'),
    ('src/krusty2/vehicle/Bike.cpp', 'LoadBike@Bike', 4225472, 6745, 6745, 6428, '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'),
    ('samples/race/WreckerNearMisses.cpp', '?PushProbesOutOfBox@Wrecker@@', 5450208, 1312, 1312, 1200, 'ee990cbeffbb403f2238ce88038c256717237b874c57444ad28b5ed7ff53e2cb'),
    ('samples/race/WreckerNearMisses.cpp', '?SprayContactParticles@Wrecker@@', 5445440, 1624, 1624, 1606, '439fd80876fa804dde2e6522a6deaae3048455a89c2d1074ae21263310e408d6'),
]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class MathCallConnectionsTests(unittest.TestCase):
    def test_actual_callers_complete_providers_and_unchanged_partial_functions(self):
        retail_path = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(retail_path.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(retail_path)
        sources = (*SOURCES, 'samples/race/WreckerNearMisses.cpp',
                   'src/reconstructed/VectorCalls.cpp', 'tests/fixtures/camera_wrecker_math_layout.cpp')
        with tempfile.TemporaryDirectory(prefix='mcm2-math-call-connections-') as directory:
            objects = {}
            for source in sources:
                path = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)

            provider = objects['src/reconstructed/VectorCalls.cpp']
            for symbol, va, size in ((DOT, 0x40ae30, 29), (SCALE, 0x5015b0, 69),
                                     (SUBTRACT, 0x421d00, 69)):
                entry = provider.find_symbol(symbol)
                self.assertGreater(entry.section_number, 0)
                self.assertEqual(entry.function_size, size)
                self.assertEqual(entry.extent_source, 'coff_function_aux')
                body, extent, relocations = provider.symbol_extent(entry)
                self.assertEqual(extent, size)
                self.assertEqual(body, retail.bytes_at_va(va, size))
                self.assertEqual(relocations, [])
            expected_imports = {
                SOURCES[0]: {DOT}, SOURCES[1]: {DOT, SCALE},
                SOURCES[2]: {DOT, SCALE}, SOURCES[3]: {SCALE},
                SOURCES[4]: {DOT, SCALE, SUBTRACT}, SOURCES[5]: {DOT},
            }
            retired = {'?Vec3Dot@@YAMPBUVector3@@0@Z',
                       '?Vec3ScaleCall@@YA?AUVector3@@ABU1@M@Z',
                       '?Vec3Scale@@YA?AUVector3@@ABU1@M@Z',
                       '?BikeVec3Dot@@YAMABUVec3@@0@Z',
                       '?Vec3Dot@@YAMABUVec3@@0@Z',
                       '?BikeVec3Sub@@YA?AUVec3@@ABU1@0@Z',
                       '?BikeVec3Scale@@YA?AUVec3@@ABU1@M@Z'}
            for source, wanted in expected_imports.items():
                obj = objects[source]
                imports = {symbol.name for symbol in obj.symbols
                           if symbol.storage_class == 2 and symbol.section_number == 0}
                self.assertTrue(wanted <= imports, (source, wanted - imports))
                self.assertFalse(retired & imports, (source, retired & imports))
                calls = {obj.symbol_by_index[r.symbol_index].name
                         for r in obj.relocations if r.type == 0x14}
                self.assertTrue(wanted <= calls)

            # All nine new connections belong to the complete steering-torque
            # partial below. Its existing resolved fingerprint also preserves
            # the result-buffer addresses, cleanup and post-call vector copies.
            bike = objects[SOURCES[4]]
            _, _, relocs = bike.symbol_extent(bike.find_symbol('Method_0x0040a520@Bike'))
            calls = Counter(bike.symbol_by_index[r.symbol_index].name for r in relocs
                            if r.type == 0x14 and bike.symbol_by_index[r.symbol_index].name in (SCALE, SUBTRACT))
            self.assertEqual(calls, Counter({SUBTRACT: 5, SCALE: 4}))
            import capstone
            decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            retail_calls = Counter(i.op_str for i in decoder.disasm(retail.bytes_at_va(0x40a520, 2272), 0x40a520)
                                   if i.mnemonic == 'call' and i.op_str in ('0x421d00', '0x5015b0'))
            self.assertEqual(retail_calls, Counter({'0x421d00': 5, '0x5015b0': 4}))

            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'],
                      ROOT / c['bindings']) for c in CASES if c['source'] in SOURCES]
            for manifest in (ROOT / 'src/krusty2').rglob('targets.json'):
                for target in json.loads(manifest.read_text()):
                    if 'source' not in target:
                        continue
                    source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                    if source in SOURCES:
                        cases.append((source, target['candidate_symbol_contains'],
                                      int(target['target_va'], 0), target['target_size'],
                                      manifest.parent / target['bindings']))
            partial_keys = {(source, va) for source, symbol, va, size, candidate_size, positions, digest in PARTIAL}
            strict = 0
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                if (source, va) in partial_keys:
                    continue
                result = compare(retail, objects[source], symbol, va, size, bindings)
                self.assertTrue(result['strict_exact'], (source, symbol, result))
                strict += 1
            self.assertGreaterEqual(strict, 209)

            for source, symbol, va, size, candidate_size, positions, digest in PARTIAL:
                bindings_path = (ROOT / source).with_suffix('.bindings.json')
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, bindings_path)
                self.assertEqual(result['candidate_size'], candidate_size)
                self.assertEqual(result['strict_match']['matching_positions'], positions)
                bindings = {key: int(value, 0) for key, value in json.loads(bindings_path.read_text()).items()}
                # Capture the complete resolved object body, using the same
                # independently bound relocation machinery as the byte gate.
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(obj, symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, (source, symbol))


if __name__ == '__main__':
    unittest.main()
