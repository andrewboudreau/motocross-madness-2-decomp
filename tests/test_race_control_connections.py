"""Existing race-control providers, complete callers, and primary-pointer routes."""
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
SOURCES = (
    'src/reconstructed/EventManager.cpp', 'src/reconstructed/InGameProcs.cpp',
    'src/reconstructed/DlgProcs.cpp', 'src/reconstructed/QuarryStuntEvent.cpp',
    'src/reconstructed/BikeRace.cpp', 'samples/ui/DlgProcsNearMisses.cpp',
)
PROVIDERS = {
    '?HandlePlayerLeft@BikeRace@@QAEXH@Z': (0x420590, 189),
    '?ToggleReplayPause@BikeRace@@QAEXH@Z': (0x41f550, 60),
    '?RestartRace@BikeRace@@QAEXHPAD0@Z': (0x420650, 1194),
    '?SaveReplay@BikeRace@@QAEXPAD0@Z': (0x420b00, 207),
    '?ResetStartCountdown@BikeRace@@QAEXXZ': (0x421050, 158),
}
# The old aggregate failed compilation: its ShowDialog parent argument still
# cast `this` to int. These complete hashes were captured in a private baseline
# with ONLY that required UIDialog* correction, before changing race calls.
# They are not a claim that the unmodified old aggregate compiled.
PARTIAL = (
    ('?HandleEvent@MainDlg@@UAEXPAUUnknownDialogEvent@@@Z', 0x44b200, 5232, 5292, 962,
     '4f2334f0bd2edbfafdf8407310fa2ce13d0bb565421560f8fd89870f0ad5f509'),
    ('?Tick@SPBikeRiderDlg@@UAEHM@Z', 0x44fae0, 1235, 1235, 1181,
     '029e1fc3e324db9c22cefe24cd69e7331f5b04d4f90ea443f4fb607d6d10edc7'),
    ('?HandleEvent@SPRaceInfoDlg@@UAEXPAUUnknownDialogEvent@@@Z', 0x4507a0, 1652, 1652, 589,
     'febd987a86806c1f4273fcf7ecee9ac54bd1b30f8702a3b5052c685f7ba21745'),
    ('?HandleEvent@MPRaceInfoDlg@@UAEXPAUUnknownDialogEvent@@@Z', 0x4513a0, 2004, 2004, 595,
     'f3657ad6a9b0e4f2b3eb5cc2e1e6142e27325af8b7f4f7f342c906df043dcfe6'),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class RaceControlConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_lifecycle_and_routes(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # The real loader constructs BikeRace, stores the primary pointer at
        # event+34, calls its setup and retains the returned primary pointer.
        decoded = {i.address: i for i in decoder.disasm(retail.bytes_at_va(0x4df9fb, 0xd1), 0x4df9fb)}
        self.assertEqual(decoded[0x4df9ff].operands[0].imm, 0x417c30)
        self.assertEqual(decoded[0x4dfa08].op_str, 'dword ptr [ebp + 0x34], eax')
        self.assertEqual(decoded[0x4dfabb].op_str, 'ecx, dword ptr [ebp + 0x34]')
        self.assertEqual(decoded[0x4dfabf].operands[0].imm, 0x417ed0)
        self.assertEqual(decoded[0x4dfac6].op_str, 'dword ptr [ebp + 0x34], eax')
        # BikeRace's own inheritance, not an assertion that the provisional
        # UnknownKrustyBikeView declaration describes its complete class.
        col = struct.unpack('<5I', retail.bytes_at_va(0x55ac68, 20))
        self.assertEqual(col, (0, 0, 0, 0x567ab0, 0x55ac58))
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(col[4], 16))
        self.assertEqual(count, 4)
        for pointer in struct.unpack('<4I', retail.bytes_at_va(array, 16)):
            _, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            self.assertEqual((mdisp, pdisp, vdisp), (0, -1, 0))
        # ExitDlg's provisional three-int declaration actually receives an
        # int mode and two literal null pointer arguments, with unchanged ECX.
        self.assertEqual(retail.bytes_at_va(0x4888ad, 8), bytes.fromhex('6a006a006a018bc8'))
        # The unrelated stale sample cast passes the original MainDlg receiver
        # as ShowDialog's parent. Retail saves ECX in EBP and pushes it without
        # adjustment; MainDlg publicly derives directly from UIDialog.
        self.assertEqual(retail.bytes_at_va(0x44b226, 2), bytes.fromhex('8be9'))
        self.assertEqual(retail.bytes_at_va(0x44b360, 1), b'\x55')
        sites = {0x45f8fb: 0x420590, 0x45d026: 0x420b00, 0x4888b5: 0x420650,
                 0x488e94: 0x41f550, 0x488e9d: 0x41f550, 0x488eef: 0x41f550,
                 0x488f5b: 0x41f550, 0x488fc8: 0x41f550, 0x489031: 0x41f550,
                 0x489094: 0x41f550, 0x4890e0: 0x41f550, 0x48912c: 0x41f550,
                 0x4510e1: 0x421050, 0x4e1e3c: 0x420590, 0x4e1ed1: 0x420590}
        for address, target in sites.items():
            call = next(decoder.disasm(retail.bytes_at_va(address, 5), address))
            self.assertEqual((call.mnemonic, call.operands[0].imm), ('call', target))

        with tempfile.TemporaryDirectory(prefix='mcm2-race-controls-') as directory:
            output = Path(directory)
            extracted = output / 'routes.cpp'
            pieces = ['#include "' + (ROOT / 'src/reconstructed' / h).as_posix() + '"\n'
                      for h in ('BikeRaceCore.h', 'EventManager.h', 'TrackGame.h')]
            for source, declaration in (
                (SOURCES[0], 'UnknownKrustyBikeView* EventManager::FindRaceView() {'),
                (SOURCES[4], 'void BikeRace::ToggleReplayPause(int pauseOnly) {'),
            ):
                text = (ROOT / source).read_text()
                start = text.index(declaration)
                end = text.index('\n}\n', start) + 3
                pieces.append(text[start:end])
            extracted.write_text(''.join(pieces))
            fixture = 'tests/fixtures/race_control_routes.cpp'
            objects, paths = {}, {}
            for source in (*SOURCES, str(extracted), fixture):
                path = output / (Path(source).stem + '.obj')
                # No extra physics include path: protect the production
                # calibration's actual standalone include configuration.
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            provider = objects[SOURCES[4]]
            for symbol, (va, size) in PROVIDERS.items():
                entry = provider.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, provider, symbol, va, size,
                                       ROOT / 'src/reconstructed/BikeRace.bindings.json')['strict_exact'])
            for symbol, va, size, bindings in (
                ('?FindRaceView@EventManager@@QAEPAUUnknownKrustyBikeView@@XZ', 0x45d2f0, 78,
                 'src/reconstructed/EventManager.bindings.json'),
                ('?ToggleReplayPause@BikeRace@@QAEXH@Z', 0x41f550, 60,
                 'src/reconstructed/BikeRace.bindings.json'),
            ):
                obj = objects[str(extracted)]
                self.assertEqual(obj.find_symbol(symbol).function_size, size)
                self.assertTrue(compare(retail, obj, symbol, va, size, ROOT / bindings)['strict_exact'])
            required = ({'HandlePlayerLeft', 'SaveReplay'}, {'ToggleReplayPause', 'RestartRace'},
                        {'ResetStartCountdown'}, {'HandlePlayerLeft'})
            for source, methods in zip(SOURCES, required):
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.storage_class == 2 and s.section_number == 0}
                calls = {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 20}
                for symbol in PROVIDERS:
                    method = symbol.split('@')[0][1:]
                    if method in methods:
                        self.assertIn(symbol, imports)
                        self.assertIn(symbol, calls)
                    self.assertFalse(any(name.startswith('?' + method + '@UnknownKrustyBikeView@@')
                                         for name in imports), source)
            cases = [c for c in CASES if c['source'] in SOURCES]
            for case in cases:
                self.assertTrue(compare(retail, objects[case['source']], case['symbol'],
                    int(case['target_va'], 0), case['target_size'], ROOT / case['bindings'])['strict_exact'], case)
            self.assertEqual(len(cases), 206)
            bindings_path = ROOT / 'samples/ui/DlgProcsNearMisses.bindings.json'
            bindings = {key: int(value, 0) for key, value in json.loads(bindings_path.read_text()).items()}
            obj = objects[SOURCES[5]]
            for symbol, va, size, extent, positions, digest in PARTIAL:
                result = compare(retail, obj, symbol, va, size, bindings_path)
                self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']),
                                 (extent, positions))
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(obj, symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, symbol)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'routes.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Race control: 993 exact-provider route and pause cases passed', result.stdout)
            print('206 strict comparisons and four complete partial fingerprints preserved; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
