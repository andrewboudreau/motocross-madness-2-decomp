"""Complete constructor definitions preserve original call sites and float words."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
VIEWS = (
    ('CollisionVec3', 'src/reconstructed/CollisionVectorConstructor.cpp', 'src/krusty2/collision/CollisionTypes.h'),
    ('BikeVec3Call', 'src/krusty2/vehicle/Bike.cpp', 'src/krusty2/vehicle/Bike.cpp'),
    ('CameraVec3Call', 'src/reconstructed/Camera.cpp', 'src/reconstructed/Camera.cpp'),
    ('CollisionVec3Call', 'src/krusty2/collision/CollisionObject.cpp', 'src/krusty2/collision/CollisionObject.cpp'),
    ('SoultreeOolVec3', 'src/krusty2/soultree/soultree.cpp', 'src/krusty2/soultree/soultree.cpp'),
    ('Vec3Call', 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp', 'src/krusty2/bvh/BoundingBoxTreeQuery.h'),
    ('VehVec3Call', 'src/krusty2/vehicle/Vehicle.cpp', 'src/krusty2/vehicle/Vehicle.cpp'),
)
SOURCES = tuple(v[1] for v in VIEWS) + (
    'src/krusty2/collision/CollisionPoint.cpp',
    'samples/physics/collision/CollisionObjectNearMisses.cpp',
    'samples/physics/collision/CollisionPointNearMisses.cpp',
)
# Complete resolved fingerprints captured before adding any constructor bodies.
# These preserve the existing partial candidates without asserting retail identity.
PARTIAL = [('src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot38@Bike',
  4215184,
  2874,
  2865,
  1758,
  '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'),
 ('src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot7@Bike',
  4219072,
  1678,
  1678,
  1677,
  'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'),
 ('src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot89@Bike',
  4221664,
  2396,
  2120,
  115,
  '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'),
 ('src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot102@Bike',
  4224064,
  485,
  474,
  188,
  'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'),
 ('src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot61@Bike',
  4243456,
  1015,
  946,
  162,
  '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'),
 ('src/krusty2/vehicle/Bike.cpp',
  'Method_0x0040a520@Bike',
  4236576,
  2272,
  2272,
  2112,
  'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'),
 ('src/krusty2/vehicle/Bike.cpp',
  'LoadBike@Bike',
  4225472,
  6745,
  6745,
  6428,
  '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'CollisionRelativeFrame',
  4399712,
  458,
  458,
  441,
  '13b2593df0c30c371064af085046e1a443542dd1c1e0ffd8a65d4d3206e34670'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'GetWorldBounds',
  4415744,
  804,
  804,
  798,
  '9f3fd72f8e32438b9b66530b77f750a8287d6b2858a74581fdfb6a1b684406f6'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'HullVsHull@',
  4417312,
  962,
  962,
  936,
  '8b51731599379651c878568733a0746c86020e828174530a0c1547e1b622141e'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'HullVsHullSwept',
  4418288,
  852,
  852,
  809,
  'a409357a807e8232305aabfdb5f8afe5242dedfe61733bbac40f1d925f3800fe'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'HullVsModel@',
  4419152,
  2205,
  2205,
  2109,
  'c4b9ed6ec90da1e5657a84fab522747d38934707f27e262cbbacf1aafce24444'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'HullVsModelSwept',
  4421360,
  707,
  707,
  695,
  '2b3660dde5a5e2a4f5ba064b1b13c626b5b0cb9a98e9be861a12b45a95ce2240'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'ModelVsHull@',
  4422688,
  716,
  716,
  712,
  'd993b6c5c7d30e6f1e265114c539161fe72efec204003716f25930e036976033'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'ModelVsHullSwept',
  4423408,
  906,
  906,
  857,
  '2407ee32709171cbc61ade59c097784d46282c6eec3cad7cbe019bf9fb0be4f9'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'ModelVsModel@',
  4424320,
  710,
  710,
  705,
  '7f56a12b37d0099e2ea419dfaa50e4d6fc0deb9750bb9a57f18678c57e531c10'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'ModelVsModelSwept',
  4425040,
  783,
  783,
  734,
  '211d6cb7d1607d1b2adc4d8a4b809251304278d8e4d68a2e315b583e6727ba8c'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'UpdateModelBounds@CollisionObject',
  4411056,
  1885,
  1885,
  1666,
  '0eca7fc79b13ba16d007be417af6f90b092fd9a6219b832e228dfbb49a8a783c'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'SetTransform@CollisionObject',
  4413488,
  1676,
  1684,
  890,
  '0918e17658ebe61094b489bfee74ae7e1d0e8c00c2c7af7231add76b5bbef9ce'),
 ('src/krusty2/collision/CollisionObject.cpp',
  'SegmentTouchesObject',
  4429856,
  1493,
  1493,
  1309,
  '8db8fbd9cb7fb347ea0fb8dfb7f08d40d200beb1d8feedfefea7cb8396770588'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  '??0Vehicle@@QAE@H@Z',
  5396384,
  745,
  745,
  693,
  'f0f794c41bd245706ce1d164fc4c37edaff241a81f77a197096ed588adf734cf'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  'UnknownVirtualSlot4@Vehicle',
  5400016,
  555,
  555,
  381,
  'a8a322f5185e324b05699f46ebb2f214a832404aa7d1ed4d74a0bb79c1259a9e'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  'UnknownVirtualSlot74@Vehicle',
  5407152,
  539,
  539,
  528,
  '1008f678cbeb2454ffc2aa3e637bd6cb5682d900434ac3fee195ed30dfbf6a1f'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  'UnknownVirtualSlot56@Vehicle',
  5408416,
  784,
  784,
  754,
  '56af218a3986e030c9c59c6a494e23c8ce895a3a2137118b0f8f5e8b812536c7'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  'UnknownVirtualSlot49@Vehicle',
  5417280,
  3254,
  3254,
  3229,
  '00b155aafdae0021e18ba64055e46609f5ad556b2eb37f6db07844362940ec7d'),
 ('src/krusty2/vehicle/Vehicle.cpp',
  '?Method_00527A20@Vehicle@@QAEXPAMPAUVec3@@1@Z',
  5405216,
  870,
  866,
  350,
  '11188ef71d21a8c0aee7aaf1d652dca3c45374514a0a1ca3e3e83476b9ce83ac'),
 ('samples/physics/collision/CollisionPointNearMisses.cpp',
  'SoultreeMergeContacts',
  4434480,
  845,
  845,
  820,
  'b53bd4eb847c2470008cdc15f71c20f908d8ce8a8b3f181ac1c4a8376975428b'),
 ('samples/physics/collision/CollisionPointNearMisses.cpp',
  'SoultreeRefreshContacts',
  4435328,
  615,
  615,
  604,
  'c1eebf9abd629e2092a54ff39359ae46985dfdb896f6d660bba1e17e974271f1')]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class VectorConstructorTests(unittest.TestCase):
    def test_full_providers_callers_and_native_float_words(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        body = retail.bytes_at_va(0x404e60, 25)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoded = list(decoder.disasm(body, 0x404e60))
        self.assertEqual(sum(i.size for i in decoded), 25)
        self.assertEqual([i.mnemonic for i in decoded], ['mov'] * 7 + ['ret'])
        self.assertEqual(decoded[-1].op_str, '0xc')
        self.assertEqual(retail.bytes_at_va(0x404e79, 7), b'\x90' * 7)
        with tempfile.TemporaryDirectory(prefix='mcm2-vector-constructors-') as directory:
            output = Path(directory)
            declarations = ['#include "' + (ROOT / path).as_posix() + '"\n' for path in (
                'src/krusty2/math/Vec3.h', 'src/reconstructed/MatrixUtil.h',
                'src/krusty2/collision/CollisionTypes.h')]
            implementations = ['#include "vector_constructor_views.h"\n']
            for name, source, declaration_source in VIEWS[1:]:
                text = (ROOT / declaration_source).read_text()
                start = text.index('struct ' + name + ' :')
                end = text.index('\n};', start) + 3
                declarations.append(text[start:end] + '\n')
                text = (ROOT / source).read_text()
                start = text.index(name + '::' + name + '(float x_, float y_, float z_)')
                end = text.index('\n}', start) + 2
                implementations.append(text[start:end] + '\n')
            (output / 'vector_constructor_views.h').write_text(''.join(declarations))
            extracted = output / 'constructors.cpp'
            extracted.write_text(''.join(implementations))
            fixture = 'tests/fixtures/vector_constructor_bits.cpp'
            objects, paths = {}, {}
            for source in (*SOURCES, str(extracted), fixture):
                path = output / (Path(source).stem + '.obj')
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                        '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source.startswith(('src/krusty2/', 'samples/physics/')):
                    args += ['--include', str(ROOT / 'src/krusty2')]
                if source in (str(extracted), fixture):
                    args += ['--include', str(output)]
                result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            for name, source, _ in VIEWS:
                symbol = '??0' + name + '@@QAE@MMM@Z'
                # The complete production body and, for local declarations,
                # independently extracted execution body both match all bytes.
                sources = (source,) if name == 'CollisionVec3' else (source, str(extracted))
                for candidate in sources:
                    obj = objects[candidate]
                    entry = obj.find_symbol(symbol)
                    actual, extent, relocations = obj.symbol_extent(entry)
                    self.assertEqual((entry.function_size, entry.extent_source), (25, 'coff_function_aux'))
                    self.assertEqual((actual, extent, relocations), (body, 25, []))
                definitions = [candidate for candidate in SOURCES if candidate.startswith('src/')
                               and any(s.name == symbol and s.section_number > 0 and s.storage_class == 2
                                       for s in objects[candidate].symbols)]
                self.assertEqual(definitions, [source])
                if name != 'CollisionVec3':
                    self.assertIn(symbol, {objects[source].symbol_by_index[r.symbol_index].name
                                           for r in objects[source].relocations if r.type == 20})
            shared = objects[VIEWS[0][1]]
            self.assertEqual({s.name for s in shared.symbols if s.storage_class == 2 and s.section_number > 0},
                             {'??0CollisionVec3@@QAE@MMM@Z'})
            self.assertEqual({s.name for s in shared.symbols if s.storage_class == 2 and s.section_number == 0},
                             {'__fltused'})  # compiler marker, no code relocation or game dependency
            for source in ('src/krusty2/collision/CollisionObject.cpp', 'src/krusty2/collision/CollisionPoint.cpp'):
                self.assertIn('??0CollisionVec3@@QAE@MMM@Z',
                              {s.name for s in objects[source].symbols if s.section_number == 0})
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for base in ('src/krusty2', 'samples/physics'):
                for manifest in (ROOT / base).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if 'source' not in target or not target.get('bindings'):
                            continue
                        source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES:
                            cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                          target['target_size'], manifest.parent / target['bindings']))
            partial = {tuple(row[:4]): row[4:] for row in PARTIAL}
            strict_count = partial_count = 0
            for source, symbol, va, size, bindings_path in dict.fromkeys(cases):
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, bindings_path)
                key = (source, symbol, va, size)
                if key not in partial:
                    self.assertTrue(result['strict_exact'], key)
                    strict_count += 1
                else:
                    extent, positions, digest = partial[key]
                    self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']),
                                     (extent, positions), key)
                    bindings = {k: int(v, 0) for k, v in json.loads(bindings_path.read_text()).items()}
                    with patch('mcm2tool.resolved_match.compare_bytes',
                               lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                        resolved = match_object(obj, symbol, va, b'', bindings)
                    self.assertEqual(resolved['digest'], digest, key)
                    partial_count += 1
            self.assertEqual((strict_count, partial_count), (355, 28))
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'constructors.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[VIEWS[0][1]]), str(paths[str(extracted)]),
                str(paths[fixture]), 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Vector constructors: 72240 raw-bit, placement and overlap cases passed', result.stdout)
            print('355 strict checks and 28 complete partial fingerprints; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
