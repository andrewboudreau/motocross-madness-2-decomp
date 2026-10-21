"""Existing Character motion providers retain primary receivers and caller bytes."""
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
SOURCES = ('src/krusty2/vehicle/Bike.cpp', 'src/reconstructed/BikeRace.cpp',
           'src/reconstructed/KrustyUI.cpp', 'samples/race/BikeRaceNearMisses.cpp',
           'src/krusty2/motion/Motnctrl.cpp')
FIND = '?FindMotion@Character@@QAEPAUMotion@@PBDH@Z'
SET = '?SetMotionByName@Character@@QAEXPBD@Z'
PROVIDERS = (
    ('Motion* MotionManager::Find(', '?Find@MotionManager@@QAEPAUMotion@@PBDH@Z', 0x4a6290, 63),
    ('Motion* Character::FindMotion(', FIND, 0x4a6b30, 24),
    ('void Character::SetMotionByName(', SET, 0x4a8b10, 33),
    ('void Character::SetMotion(', '?SetMotion@Character@@QAEXPAUMotion@@@Z', 0x4a8b40, 27),
)
# Full resolved candidates captured before this connection. These hashes guard
# preservation, including the five complete BikeRace sample methods, not fidelity.
PARTIAL = [['src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot38@Bike',
  4215184,
  2874,
  2865,
  1758,
  '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'],
 ['src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot7@Bike',
  4219072,
  1678,
  1678,
  1677,
  'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'],
 ['src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot89@Bike',
  4221664,
  2396,
  2120,
  115,
  '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'],
 ['src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot102@Bike',
  4224064,
  485,
  474,
  188,
  'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'],
 ['src/krusty2/vehicle/Bike.cpp',
  'UnknownVirtualSlot61@Bike',
  4243456,
  1015,
  946,
  162,
  '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'],
 ['src/krusty2/vehicle/Bike.cpp',
  'Method_0x0040a520@Bike',
  4236576,
  2272,
  2272,
  2112,
  'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'],
 ['src/krusty2/vehicle/Bike.cpp',
  'LoadBike@Bike',
  4225472,
  6745,
  6745,
  6428,
  '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'],
 ['samples/race/BikeRaceNearMisses.cpp',
  '?AimCamera@BikeRace@@',
  4320032,
  1708,
  1710,
  1697,
  '24a50f1c79e1a1c71d73838815b5d777e21c8ccfce2489e72403efbc04ebe25f'],
 ['samples/race/BikeRaceNearMisses.cpp',
  '?NextCameraTarget@BikeRace@@',
  4321744,
  881,
  881,
  266,
  '15af6d17ef636a607c78fe95b84b55d35f268b9443f9a5b012b0eb31e3d4da16'],
 ['samples/race/BikeRaceNearMisses.cpp',
  '?WriteRelativePositions@BikeRace@@',
  4329712,
  2940,
  2964,
  1017,
  '13875d63da3b90ea267fdcf5a4e9bea85014d296499812d6550403f13e6555bc'],
 ['samples/race/BikeRaceNearMisses.cpp',
  '?Create@BikeRace@@',
  4292304,
  6745,
  6760,
  2821,
  '59d5d6fed62ff5b907d3df81802c39f84c8feeede5d33f5cde930089545f8c90'],
 ['samples/race/BikeRaceNearMisses.cpp',
  '?CreateRacers@BikeRace@@',
  4299120,
  13428,
  13428,
  13396,
  'a6159f61caafc2347d11f9ef24dfc6956a1da5b180cd6749c63609a4716f8482']]


def function_text(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class CharacterMotionConnectionsTests(unittest.TestCase):
    def test_provider_closure_full_callers_and_native_motion_state(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        # Decode primary D3DIMSoultreeCharacter RTTI, including Character's
        # nonvirtual zero-offset base. GameObject is a distinct virtual base.
        locator, = struct.unpack('<I', retail.bytes_at_va(0x551530 - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVD3DIMSoultreeCharacter@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCharacter@@'], (0, -1, 0))
        self.assertNotEqual(bases[b'.?AVGameObject@@'][1], -1)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        for address, size, constructors in ((0x4079c0, 6745, 1), (0x498cf0, 3114, 3),
                                             (0x417ed0, 6745, 1)):
            code = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            calls = [i.operands[0].imm for i in code if i.mnemonic == 'call'
                     and i.operands[0].type == capstone.x86.X86_OP_IMM]
            self.assertEqual(calls.count(0x4455b0), constructors)
        with tempfile.TemporaryDirectory(prefix='mcm2-character-motion-') as directory:
            output = Path(directory)
            extracted = output / 'motion_routes.cpp'
            text = (ROOT / SOURCES[4]).read_text()
            extracted.write_text('#include <string.h>\n#include "motion/Motnctrl.h"\n'
                'extern MotionManager* g_pMotionManager;\n' + '\n\n'.join(
                    function_text(text, signature) for signature, _, _, _ in PROVIDERS))
            fixture = 'tests/fixtures/character_motion_routes.cpp'
            objects, paths = {}, {}
            for source in (*SOURCES, str(extracted), fixture):
                path = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source in (SOURCES[0], SOURCES[4], str(extracted), fixture):
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            bindings_path = ROOT / Path(SOURCES[4]).with_suffix('.bindings.json')
            for _, symbol, va, size in PROVIDERS:
                for source in (SOURCES[4], str(extracted)):
                    obj = objects[source]
                    self.assertEqual((obj.find_symbol(symbol).function_size, obj.find_symbol(symbol).extent_source),
                                     (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, obj, symbol, va, size, bindings_path)['strict_exact'], symbol)
            for source, symbol, retired in (
                (SOURCES[0], FIND, '?FindMotion@BikeA5C4@@QAEHPBDH@Z'),
                (SOURCES[1], SET, '?SetMotionByName@UnknownBikeRaceCharacter@@QAEXPBD@Z'),
                (SOURCES[2], SET, '?SetMotionByName@UnknownKrustyUICharacter@@QAEXPBD@Z'),
                (SOURCES[3], SET, '?SetMotionByName@UnknownBikeRaceCharacter@@QAEXPBD@Z')):
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.section_number == 0}
                self.assertIn(symbol, imports)
                self.assertNotIn(retired, imports)
                self.assertIn(symbol, {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 20})
            body, size, relocs = objects[fixture].symbol_extent(objects[fixture].find_symbol('CharacterBase'))
            self.assertEqual((body, size, relocs), (bytes.fromhex('8b442404c3'), 5, []))
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
            for source, symbol, va, size in partial:
                if source == SOURCES[3]:
                    cases.append((source, symbol, va, size, (ROOT / source).with_suffix('.bindings.json')))
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
            self.assertEqual(partial_count, 12)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'motion_routes.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Character motion: 3840 lookup, state and missing-name cases passed', result.stdout)
            print('%d strict checks and %d complete partial fingerprints; %s' %
                  (strict_count, partial_count, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
