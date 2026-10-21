"""Canonical race iterator connections preserve full callers and pointer ABI."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?NextBike@BikeRace@@QAEPAVUnknownBikeRaceRacer@@PAH@Z'
SOURCES = (
    'src/reconstructed/EventManager.cpp', 'src/reconstructed/FollowRacer.cpp',
    'src/reconstructed/RaceSound.cpp', 'src/reconstructed/RaceStatus.cpp',
    'src/reconstructed/TrackOverlay.cpp', 'src/reconstructed/Krusty3DObjects.cpp',
    'src/krusty2/vehicle/KrustyBike.cpp', 'src/reconstructed/BikeRace.cpp',
)
# Complete resolved candidate hashes captured before connecting the calls.
# These preserve partial functions; they do not assert retail byte identity.
PARTIAL = [
    ('samples/race/FollowRacerNearMisses.cpp', '?PickFollowedRacer@TrackGameViewOwner@@QAEXXZ', 4889888, 340, 346, 69, 'a9d4a6f4f3770e957a5efa00ca26dcaffe88bbd5dd8cf8011f1f50d942f0f303'),
    ('samples/race/FollowRacerNearMisses.cpp', '?SetFollowedRacer@TrackGameViewOwner@@QAEXPAUUnknownEventRacer@@0H@Z', 4890240, 272, 272, 259, '6f7a0fdf97b1e244baeb6a3f3af7cf634acda08a0ea738fd1e3de094ccc7032d'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot70@KrustyBike', 4790688, 229, 230, 151, '3124025ed125d088a9cf9d706154f76eb28d5e8661543863782a91d540c2e19f'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot11@KrustyBike', 4791568, 1296, 1288, 526, '2260143a9d880dfabfff868e69178f34173f372aa53195cc6d885ed5f14bc576'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'Fn_00492AD0@KrustyBike', 4795088, 2315, 2312, 192, '1941ddbc72fdfd7b01b5ad4ea65fcace024e8e828a4019450c91e9f4d1fb6a95'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'Fn_00493660@KrustyBike', 4798048, 8545, 8558, 2305, 'ffc638804e115abac59baeed51fe496e2c2167f5f006b1c337aaee4f7e249815'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', '??0KrustyBike@@QAE@H@Z', 4782688, 532, 532, 472, 'f333644c75f0d39f6e5af3c3d8c898b85d85f9467e953b8d0e6f4c5771d00276'),
    ('samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'UnknownVirtualSlot102@KrustyBike', 4779680, 2920, 2952, 608, '1bbe35c0f4249dd69e1255c94f129b84d7d0f79ae2e94173ce021cf0a3a4b87c'),
]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class NextBikeConnectionsTests(unittest.TestCase):
    def test_real_callers_provider_layout_and_iterator(self):
        retail_path = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(retail_path.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(retail_path)
        # Retail primary COL and every base PMD prove this boundary has no
        # adjustment. This says nothing about returned racers' virtual bases.
        col = struct.unpack('<5I', retail.bytes_at_va(0x55ac68, 20))
        self.assertEqual(col, (0, 0, 0, 0x567ab0, 0x55ac58))
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(col[4], 16))
        self.assertEqual(count, 4)
        names = []
        for pointer in struct.unpack('<4I', retail.bytes_at_va(array, 16)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            self.assertEqual((mdisp, pdisp, vdisp), (0, -1, 0))
            names.append(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0])
        self.assertEqual(names, [b'.?AVBikeRace@@', b'.?AVGraphicsTest@@', b'.?AVGameObject@@', b'.?AVBaseObject@@'])
        # A returned KrustyBike primary pointer must not be adjusted to either
        # its secondary character subobject or its virtual GameObject base.
        for locator, offset in ((0x55d078, 0), (0x55d008, 0x21c), (0x55cff0, 0x160c)):
            fields = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
            self.assertEqual((fields[1], fields[3]), (offset, 0x56d0a8))

        with tempfile.TemporaryDirectory(prefix='mcm2-nextbike-') as directory:
            output = Path(directory)
            # Extract the unchanged function, then independently prove both
            # this execution object and the full production TU against retail.
            text = (ROOT / SOURCES[-1]).read_text()
            start = text.index('UnknownBikeRaceRacer* BikeRace::NextBike(int* iterator) {')
            end = text.index('\n}\n', start) + 3
            extracted = output / 'iterator.cpp'
            extracted.write_text('#include "BikeRaceCore.h"\n#include "TrackGame.h"\n' + text[start:end])
            # Separate headers intentionally keep the partial views separate;
            # matching selected offsets does not assert complete type identity.
            layouts = []
            for name, header, assertions in (
                ('event', 'RaceView.h', 'offsetof(UnknownKrustyBikeView, localRacer) == 0x38 && offsetof(UnknownEventRacer, position) == 0xc && offsetof(UnknownEventRacer, bikeView) == 0x740'),
                ('visual', 'Krusty3DObjects.h', 'offsetof(UnknownVisualCueView, ownRacer) == 0x38 && offsetof(UnknownVisualCueRacer, position) == 0xc'),
                ('stats', 'TrackOverlay.h', 'offsetof(UnknownStatsSource, ownRacer) == 0x38 && offsetof(UnknownStatsSource, replayTime) == 0x1b8'),
                ('bike', 'vehicle/KrustyBike.h', 'offsetof(KbRace, localRacer) == 0x38 && offsetof(KrustyBike, race) == 0x740 && sizeof(KrustyBike) == 0x1638'),
                ('canonical', 'BikeRace.h', 'offsetof(UnknownBikeRaceRacer, bikeView) == 0x740 && offsetof(BikeRace, replayTime) == 0x1b8'),
            ):
                layout = output / (name + '_layout.cpp')
                layout.write_text(f'#include "{header}"\n#include <stddef.h>\ntypedef char Layout[({assertions}) ? 1 : -1];\n')
                layouts.append(str(layout))
            sources = (*SOURCES, *dict.fromkeys(row[0] for row in PARTIAL),
                       'tests/fixtures/nextbike_iterator.cpp', str(extracted), *layouts)
            objects = {}
            paths = {}
            for source in sources:
                path = output / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
                paths[source] = path
            bindings_path = ROOT / 'src/reconstructed/BikeRace.bindings.json'
            for source in (SOURCES[-1], str(extracted)):
                obj = objects[source]
                entry = obj.find_symbol(SYMBOL)
                self.assertEqual((entry.function_size, entry.extent_source), (162, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, SYMBOL, 0x4204e0, 162, bindings_path)['strict_exact'])
                _, extent, relocations = obj.symbol_extent(entry)
                self.assertEqual(extent, 162)
                self.assertEqual({obj.symbol_by_index[r.symbol_index].name for r in relocations},
                                 {'?g_TrackGame@@3PAVTrackGame@@A'})

            retired = {
                '?NextBike@KbRace@@QAEPAVKrustyBike@@PAH@Z',
                '?NextBike@UnknownKrustyBikeView@@QAEPAUUnknownEventRacer@@PAH@Z',
                '?NextBike@UnknownStatsSource@@QAEPAUUnknownEventRacer@@PAH@Z',
                '?NextBike@UnknownVisualCueView@@QAEPAUUnknownVisualCueRacer@@PAH@Z',
            }
            for source in SOURCES[:-1]:
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.storage_class == 2 and s.section_number == 0}
                self.assertIn(SYMBOL, imports, source)
                self.assertFalse(retired & imports, source)
                self.assertIn(SYMBOL, {obj.symbol_by_index[r.symbol_index].name
                                      for r in obj.relocations if r.type == 0x14})
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for manifest in (ROOT / 'src/krusty2').rglob('targets.json'):
                for target in json.loads(manifest.read_text()):
                    if 'source' not in target:
                        continue
                    source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                    if source in SOURCES:
                        cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                      target['target_size'], manifest.parent / target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                result = compare(retail, objects[source], symbol, va, size, bindings)
                self.assertTrue(result['strict_exact'], (source, symbol, result))
            self.assertGreaterEqual(len(dict.fromkeys(cases)), 338)
            for source, symbol, va, size, candidate_size, positions, digest in PARTIAL:
                bindings_path = (ROOT / source).with_suffix('.bindings.json')
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, bindings_path)
                self.assertEqual(result['candidate_size'], candidate_size)
                self.assertEqual(result['strict_match']['matching_positions'], positions)
                bindings = {key: int(value, 0) for key, value in json.loads(bindings_path.read_text()).items()}
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(obj, symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, (source, symbol))

            compiler_root = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
            executable = output / 'iterator.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}', str(paths[str(extracted)]),
                str(paths['tests/fixtures/nextbike_iterator.cpp']), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('14400 iterator cases passed', result.stdout)


if __name__ == '__main__':
    unittest.main()
