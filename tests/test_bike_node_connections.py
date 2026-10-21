"""Bike model views join existing scene-node providers without changing bodies."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import match_object
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/krusty2/vehicle/Bike.cpp', 'src/krusty2/vehicle/KrustyBike.cpp',
           'samples/physics/krustybike/KrustyBikeNearMisses.cpp',
           'src/krusty2/soultree/soultree.cpp', 'samples/physics/helpers/SoultreeMatrix.cpp')
FIND = '?FindByName@SoultreeObject@@QAEPAV1@PBD@Z'
BOUNDS = '?GetSubtreeBounds@SoultreeObject@@QAEXPAUVec3@@0@Z'
MATRIX = '?SetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z'
# Complete relocation-resolved hashes captured before connecting these callers.
# They preserve existing nonexact bodies, not establish retail byte identity.
PARTIAL = [
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot38@Bike', 4215184, 2874, 2865, 1758, '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot7@Bike', 4219072, 1678, 1678, 1677, 'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot89@Bike', 4221664, 2396, 2120, 115, '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot102@Bike', 4224064, 485, 474, 188, 'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'),
    ('src/krusty2/vehicle/Bike.cpp', 'UnknownVirtualSlot61@Bike', 4243456, 1015, 946, 162, '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'),
    ('src/krusty2/vehicle/Bike.cpp', 'Method_0x0040a520@Bike', 4236576, 2272, 2272, 2112, 'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'),
    ('src/krusty2/vehicle/Bike.cpp', 'LoadBike@Bike', 4225472, 6745, 6745, 6428, '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'),
    ('samples/physics/helpers/SoultreeMatrix.cpp', 'SetAxesIn', 5226576, 1171, 1171, 1137, '38091e34c86106b38f4d932e661d248cb46eae64419bc394554958ccaa7d31aa'),
    ('samples/physics/helpers/SoultreeMatrix.cpp', 'SetMatrixIn', 5224640, 1096, 1134, 262, '21bd27d95aad6a35851bd016a070df65b6d2ad9cbd4acf8205d6fba7d06e08b3'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot70@KrustyBike', 4790688, 229, 230, 151, '3124025ed125d088a9cf9d706154f76eb28d5e8661543863782a91d540c2e19f'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot11@KrustyBike', 4791568, 1296, 1288, 526, '2260143a9d880dfabfff868e69178f34173f372aa53195cc6d885ed5f14bc576'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'Fn_00492AD0@KrustyBike', 4795088, 2315, 2312, 192, '1941ddbc72fdfd7b01b5ad4ea65fcace024e8e828a4019450c91e9f4d1fb6a95'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'Fn_00493660@KrustyBike', 4798048, 8545, 8558, 2305, 'ffc638804e115abac59baeed51fe496e2c2167f5f006b1c337aaee4f7e249815'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', '??0KrustyBike@@QAE@H@Z', 4782688, 532, 532, 472, 'f333644c75f0d39f6e5af3c3d8c898b85d85f9467e953b8d0e6f4c5771d00276'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot102@KrustyBike', 4779680, 2920, 2952, 608, '1bbe35c0f4249dd69e1255c94f129b84d7d0f79ae2e94173ce021cf0a3a4b87c'),
]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class BikeNodeConnectionsTests(unittest.TestCase):
    def test_actual_callers_providers_and_all_partial_bodies(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        # The real rendered node's primary SoultreeObject pointer is unchanged;
        # its GameObject pointer is distinct and must not cross this boundary.
        for locator, offset in ((0x55b2b8, 0), (0x55b248, 0xc)):
            _, actual, _, descriptor, _ = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
            self.assertEqual(actual, offset)
            self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVD3DIMSoultreeObject@@')
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x4079c0, 6745), 0x4079c0))
        self.assertEqual(sum(i.size for i in instructions), 6745)
        decoded = {i.address: i for i in instructions}
        self.assertEqual(decoded[0x407c6c].mnemonic, 'call')
        self.assertEqual(decoded[0x407c6c].operands[0].imm, 0x4455b0)
        self.assertEqual(decoded[0x407c84].op_str, 'dword ptr [ebp + 0x5c4], eax')

        with tempfile.TemporaryDirectory(prefix='mcm2-bike-nodes-') as directory:
            objects = {}
            fixture = 'tests/fixtures/bike_node_layout.cpp'
            for source in (*SOURCES, fixture):
                path = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            provider = objects[SOURCES[3]]
            for symbol, va, extent in ((FIND, 0x4fdae0, 84), (BOUNDS, 0x4fe850, 74)):
                entry = provider.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (extent, 'coff_function_aux'))
                self.assertTrue(compare(retail, provider, symbol, va, extent,
                                       ROOT / 'src/krusty2/soultree/soultree.bindings.json')['strict_exact'])
            matrix = objects[SOURCES[4]]
            self.assertGreater(matrix.find_symbol(MATRIX).section_number, 0)
            self.assertEqual(matrix.find_symbol(MATRIX).function_size, 1134)
            # Matrix body remains reviewed nonexact; test_soultree_transform_behavior
            # independently executes it against retail in 86,016 scenarios.
            transpose = matrix.find_symbol('TransposeRotation3x3')
            body, extent, relocations = matrix.symbol_extent(transpose)
            self.assertEqual((body, extent, relocations), (retail.bytes_at_va(0x4fefb0, 41), 41, []))
            retired = {'?FindByName@BikeA1A0@@QAEHPBD@Z',
                       '?GetSubtreeBounds@BikeA1A0@@QAEXPAUVec3@@0@Z',
                       '?SetMatrixIn@BikeA1A0@@QAEXHPAUMatrix4@@@Z'}
            for source, required in ((SOURCES[0], {FIND, BOUNDS, MATRIX}),
                                     (SOURCES[1], {MATRIX}), (SOURCES[2], {MATRIX})):
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.storage_class == 2 and s.section_number == 0}
                self.assertTrue(required <= imports, (source, required - imports))
                self.assertFalse(retired & imports, source)
                calls = {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 0x14}
                self.assertTrue(required <= calls, source)
            probe = objects[fixture]
            entry = probe.find_symbol('?RiderModel@@YAPAVSoultreeObject@@PAUBikeA5C4@@@Z')
            body, extent, relocations = probe.symbol_extent(entry)
            self.assertEqual(body, bytes.fromhex('8b4424048b80a0010000c3'))
            self.assertEqual(relocations, [])
            cases = []
            for base in ('src/krusty2', 'samples/physics'):
                for manifest in (ROOT / base).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if 'source' not in target:
                            continue
                        source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES and target.get('expect') != 'partial' and target.get('bindings'):
                            cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                          target['target_size'], manifest.parent / target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, bindings)['strict_exact'],
                                (source, symbol))
            self.assertGreaterEqual(len(dict.fromkeys(cases)), 189)
            for source, symbol, va, size, candidate_size, positions, digest in PARTIAL:
                bindings_path = (ROOT / source).with_suffix('.bindings.json')
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, bindings_path)
                self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']),
                                 (candidate_size, positions))
                bindings = {key: int(value, 0) for key, value in json.loads(bindings_path.read_text()).items()}
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(obj, symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, (source, symbol))


if __name__ == '__main__':
    unittest.main()
