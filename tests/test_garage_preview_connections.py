"""Garage character and plate-painter calls join complete existing providers."""
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
SOURCES = tuple('src/reconstructed/' + name + '.cpp' for name in (
    'KrustyUI', 'DlgProcs', 'ProCircuitProcs', 'SelectGamePicProcs', 'ProCircuit', 'BikeNumberPainter')) + (
    'samples/race/BikeRaceNearMisses.cpp', 'samples/ui/KrustyUINearMisses.cpp',
    'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp', 'src/krusty2/motion/Motnctrl.cpp')
CHARACTER = '??0D3DIMSoultreeCharacter@@QAE@H@Z'
ADVANCE = '?AdvanceMotion@Character@@QAEHMHH@Z'
PAINTER = '??0UnknownBikeNumberPainter@@QAE@PAVTextureMapManager@@@Z'
DESTROY = '??1UnknownBikeNumberPainter@@QAE@XZ'
PAINT = '?PaintNumber@UnknownBikeNumberPainter@@QAEXPAVD3DIMSoultreeObject@@H@Z'
PROVIDERS = ((CHARACTER, 0x4455b0, 195, SOURCES[8]), (ADVANCE, 0x4a6bb0, 1293, SOURCES[9]),
             (PAINTER, 0x417500, 97, SOURCES[5]), (DESTROY, 0x417570, 7, SOURCES[5]),
             (PAINT, 0x417670, 1157, SOURCES[5]))
# Complete pre-change resolved bodies; partial preservation is not source acceptance.
PARTIAL = [['samples/race/BikeRaceNearMisses.cpp',
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
 # Shared canonical racer declarations exchange XOR EDI/MOV EBX at +538..545.
 # Same state/flags; no interior entry; all other 13,428 bytes and targets
 # are unchanged. Nonexact score 13396 -> 13390; BIKE_RACE_STATE_CONNECTIONS.md.
 ['samples/race/BikeRaceNearMisses.cpp',
  '?CreateRacers@BikeRace@@',
  4299120,
  13428,
  13428,
  13390,
  'dfc0ee81775e2529319f4644466baacd436755506630b8d402b6621e29cea48c'],
 ['samples/ui/KrustyUINearMisses.cpp',
  '?Create@KrustyUI@@',
  4819104,
  1104,
  1102,
  872,
  'a0c7fe2a846eb24d4bf7dfdab6fcbcff2e503af4e3ab5456d11aab1c01f7bb99']]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class GaragePreviewConnectionsTests(unittest.TestCase):
    def test_complete_providers_callers_layout_and_painter_lifetime(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        # The primary Character receiver and virtual GameObject base are
        # distinct. Actual construction allocates 0x240, not the partial view.
        col, = struct.unpack('<I', retail.bytes_at_va(0x551530 - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVD3DIMSoultreeCharacter@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCharacter@@'], (0, -1, 0))
        self.assertNotEqual(bases[b'.?AVGameObject@@'][1], -1)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(retail.bytes_at_va(0x498cf0, 3114), 0x498cf0))
        self.assertEqual(sum(i.size for i in code), 3114)
        calls = [i.operands[0].imm for i in code if i.mnemonic == 'call'
                 and i.operands[0].type == capstone.x86.X86_OP_IMM]
        for va, count in ((0x4455b0, 3), (0x4a6bb0, 3), (0x417500, 1), (0x417670, 1), (0x417570, 1)):
            self.assertEqual(calls.count(va), count)
        self.assertEqual(sum(i.mnemonic == 'push' and i.operands[0].type == capstone.x86.X86_OP_IMM
                             and i.operands[0].imm == 0x240 for i in code), 3)
        with tempfile.TemporaryDirectory(prefix='mcm2-garage-preview-') as directory:
            output = Path(directory)
            fixture = 'tests/fixtures/garage_preview_lifetime.cpp'
            objects, paths = {}, {}
            for source in (*SOURCES, fixture):
                path = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if '/krusty2/' in source:
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            for symbol, va, size, source in PROVIDERS:
                obj = objects[source]
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                       (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
            for source in (SOURCES[0], SOURCES[1], SOURCES[2], SOURCES[3], SOURCES[6]):
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.section_number == 0}
                required = {PAINTER, DESTROY, PAINT}
                if source == SOURCES[0]:
                    required |= {CHARACTER, ADVANCE}
                self.assertTrue(required <= imports, (source, required - imports))
                self.assertFalse(any('UnknownKrustyUIPlatePainter' in s or 'AdvanceMotion@UnknownKrustyUICharacter' in s
                                     or '??0UnknownKrustyUICharacter' in s or s == '?PaintNumber@UnknownBikeNumberPainter@@QAEXPAXH@Z'
                                     for s in imports), source)
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
            cases += [(source, symbol, va, size, (ROOT / source).with_suffix('.bindings.json'))
                      for source, symbol, va, size in partial]
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
            self.assertEqual((strict_count, partial_count), (317, 6))
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'garage_lifetime.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[SOURCES[5]]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Garage painter: 192 resource, normal cleanup and exception lifetime cases passed', result.stdout)
            print('317 strict checks, six complete partial fingerprints; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
