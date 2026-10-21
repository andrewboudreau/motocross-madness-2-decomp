"""Race receiver connections, complete bodies, and native received-state providers."""
from collections import Counter
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
from test_bike_text_connections import extract
from test_vehicle_gearbox_connections import cases as physics_cases, fingerprint as physics_fingerprint
from test_race_event_connections import fingerprint as race_fingerprint
from test_character_motion_connections import PARTIAL as previous_partials

ROOT = Path(__file__).resolve().parents[1]
CALLER = 'src/reconstructed/BikeRace.cpp'
SAMPLE = 'samples/race/BikeRaceNearMisses.cpp'
PROVIDER = 'src/krusty2/vehicle/KrustyBike.cpp'
BODIES = (
    ('int KrustyBike::Fn_00495C00()', '?Fn_00495C00@KrustyBike@@QAEHXZ', 0x495c00, 151),
    ('void KrustyBike::Fn_004933E0(', '?Fn_004933E0@KrustyBike@@QAEXPBUKbNetDelta@@PAUKbNetState@@@Z', 0x4933e0, 634),
    ('void KrustyBike::Fn_00496E20(', '?Fn_00496E20@KrustyBike@@QAEXPAUKbRecorder@@@Z', 0x496e20, 13))


def cases():
    rows = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
            for c in CASES if c['source'] in (CALLER, SAMPLE)]
    rows += [(SAMPLE, symbol, va, size, SAMPLE.replace('.cpp', '.bindings.json'))
             for source, symbol, va, size, _ in rows if source == CALLER]
    rows += [r for r in physics_cases() if r[0] == PROVIDER]
    rows += [(s, n, v, z, s.replace(".cpp", ".bindings.json"))
             for s, n, v, z, *_ in previous_partials if s in (CALLER, SAMPLE)]
    return list(dict.fromkeys(rows))


def fingerprint(obj, symbol):
    return race_fingerprint(obj, symbol) if "@BikeRace@@" in symbol else physics_fingerprint(obj, symbol)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail image')
class RaceStateConnectionsTests(unittest.TestCase):
    def test_complete_callers_actual_providers_and_native_state(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # Distinguish the returned racer primary pointer from its character and
        # virtual GameObject subobjects. This is not full provisional-view identity.
        for locator, offset in ((0x55d078, 0), (0x55d008, 0x21c), (0x55cff0, 0x160c)):
            fields = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
            self.assertEqual((fields[1], fields[3]), (offset, 0x56d0a8))
        creation = list(decoder.disasm(retail.bytes_at_va(0x419970, 13428), 0x419970))
        decoded = {i.address: (i.mnemonic, i.op_str) for i in creation}
        constructor_sites = (0x419e09, 0x41a275, 0x41a78d, 0x41acb8, 0x41b1a8,
                             0x41b496, 0x41b862, 0x41bfcd, 0x41c7af)
        self.assertEqual(tuple(i.address for i in creation if i.mnemonic == 'call'
                              and i.op_str == '0x48fa60'), constructor_sites)
        for address in constructor_sites:
            n = next(n for n, i in enumerate(creation) if i.address == address)
            previous = creation[n-22:n]
            self.assertTrue(any(i.mnemonic == 'push' and i.op_str == '0x1638' for i in previous))
            self.assertTrue(any(i.mnemonic == 'call' and i.op_str == '0x4a3010' for i in previous))
            self.assertTrue(any(i.mnemonic == 'mov' and i.op_str == 'ecx, eax' for i in previous[-4:]))
        # Constructor returns its original primary ECX; no secondary-base adjustor.
        ctor = {i.address: (i.mnemonic, i.op_str) for i in decoder.disasm(
            retail.bytes_at_va(0x48fa60, 532), 0x48fa60)}
        self.assertEqual(ctor[0x48fa80], ('mov', 'esi, ecx'))
        self.assertEqual(ctor[0x48fc5f], ('mov', 'eax, esi'))
        self.assertEqual(ctor[0x48fc71], ('ret', '8'))
        # Every creation branch stores that return directly, or through unchanged
        # EDI, into the race's local pointer (+38) or arrays (+3c/+40).
        stores = {
            0x419e12: 'dword ptr [ebp + 0x38], eax',
            0x41a27e: 'edx, dword ptr [ebp + 0x40]',
            0x41a28e: 'dword ptr [edx + ebx*4 - 4], eax',
            0x41a79b: 'ecx, dword ptr [ebp + 0x3c]',
            0x41a7a3: 'dword ptr [ecx + esi*4], eax',
            0x41acc1: 'edx, dword ptr [ebp + 0x3c]',
            0x41acca: 'dword ptr [edx + ebx*4], eax',
            0x41b1ad: 'edi, eax', 0x41b1b3: 'ecx, dword ptr [ebp + 0x3c]',
            0x41b1bc: 'dword ptr [ecx + ebx*4], edi',
            0x41b4b3: 'dword ptr [ebp + 0x38], eax',
            0x41b86b: 'edx, dword ptr [ebp + 0x40]',
            0x41b873: 'dword ptr [edx], eax',
            0x41bfd6: 'ecx, dword ptr [ebp + 0x40]',
            0x41bfe1: 'dword ptr [ecx + ebx*4], eax',
            0x41c7b4: 'edi, eax', 0x41c7ba: 'ecx, dword ptr [ebp + 0x3c]',
            0x41c7c3: 'dword ptr [ecx + ebx*4], edi'}
        for address, operands in stores.items():
            self.assertEqual(decoded[address], ('mov', operands))
        for address, target in ((4319793, 0x495c00), (4329249, 0x496e20),
            (4329465, 0x496e20), (4326186, 0x4933e0), (4328013, 0x496e20),
            (4335476, 0x4933e0), (4335747, 0x4933e0)):
            call = next(decoder.disasm(retail.bytes_at_va(address, 5), address))
            self.assertEqual((call.mnemonic, call.operands[0].imm), ('call', target))
        partials = {tuple(row[:4]): row[4:] for row in json.loads(
            (ROOT / 'tests/fixtures/race_state_partials.json').read_text())['partials']}
        with tempfile.TemporaryDirectory(prefix='mcm2-race-state-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            def compile_source(source, key, physics=False):
                obj = output / ('unit%d.obj' % len(paths))
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                        '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if key == "fixture":
                    args += ["--extra", "/Op"]  # Oracle must honor its explicit float round trips.
                if physics:
                    args += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[key], objects[key] = obj, CoffObject(obj)
            for source in (CALLER, SAMPLE, PROVIDER):
                compile_source(ROOT / source, source, source == PROVIDER)
            extracted = output / 'providers.cpp'
            extracted.write_text('#include "vehicle/KrustyBike.h"\n' + ''.join(
                extract(ROOT / PROVIDER, signature) for signature, _, _, _ in BODIES))
            compile_source(extracted, 'native', True)
            compile_source(ROOT / 'tests/fixtures/race_state_providers.cpp', 'fixture', True)
            layout = output / 'layout.cpp'
            layout.write_text(''.join('#include "%s"\n' % (ROOT / name).as_posix() for name in (
                'src/reconstructed/BikeRace.h', 'src/krusty2/vehicle/KrustyBikeCore.h')) + '''
#include <stddef.h>
#define CHECK(n,x) typedef char n[(x)?1:-1]
CHECK(primary_size,sizeof(UnknownBikeRaceRacer)==sizeof(KrustyBike));
CHECK(remote,offsetof(UnknownBikeRaceRacer,isRemote)==offsetof(KrustyBike,isRemote));
CHECK(states,offsetof(UnknownBikeRaceRacer,receivedStates)==offsetof(KrustyBike,states));
CHECK(velocity,offsetof(UnknownBikeRaceRacer,deltaBaseVelocity)==offsetof(KrustyBike,deltaBaseVelocity));
CHECK(time,offsetof(UnknownBikeRaceRacer,deltaBaseTime)==offsetof(KrustyBike,deltaBaseTime));
CHECK(packet,sizeof(UnknownBikeRaceNetMessage13)==0x17 && offsetof(UnknownBikeRaceNetMessage13,racerIndex)==0x16);
CHECK(record,sizeof(UnknownBikeRaceRacerPart)==0x60 && offsetof(UnknownBikeRaceRacerPart,state)==0 && offsetof(UnknownBikeRaceRacerPart,messageFlags)==0x58 && offsetof(UnknownBikeRaceRacerPart,fresh)==0x5c);
''')
            compile_source(layout, 'layout')
            for _, symbol, va, size in BODIES:
                for key in (PROVIDER, 'native'):
                    entry = objects[key].find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, objects[key], symbol, va, size,
                        ROOT / PROVIDER.replace('.cpp', '.bindings.json'))['strict_exact'], (key, symbol))
            self.assertEqual({s.name for s in objects["native"].symbols
                if s.section_number == 0 and s.storage_class == 2 and s.value == 0},
                {"__ftol", "__fltused"})
            strict = preserved = 0
            totals = {CALLER: Counter(), SAMPLE: Counter()}
            addresses = {symbol: va for _, symbol, va, _ in BODIES}
            for source, symbol, va, size, binding in cases():
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key in partials:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match'].get('matching_positions'),
                                      fingerprint(obj, symbol)), (extent, positions, digest), key)
                    preserved += 1
                else:
                    self.assertTrue(result['strict_exact'], (key, result))
                    strict += 1
            # Count each compiled call once, including bodies reused by aggregate.
            for source in totals:
                obj = objects[source]
                for r in obj.relocations:
                    name = obj.symbol_by_index[r.symbol_index].name
                    if r.type == 0x14 and name in addresses:
                        totals[source][name] += 1
                self.assertEqual(totals[source], Counter({BODIES[0][1]: 1, BODIES[1][1]: 3, BODIES[2][1]: 3}))
            self.assertEqual(preserved, len(partials))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'state.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths['native']), str(paths['fixture']), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Race state: 50496 native guarded provider cases passed')
            print('%d strict bodies, %d full partial fingerprints; %s' % (strict, preserved, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
