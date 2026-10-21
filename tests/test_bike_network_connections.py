"""KrustyBike's actual transport, recorder and race-owner call boundaries."""
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
from test_vehicle_gearbox_connections import cases as gearbox_cases, fingerprint

ROOT = Path(__file__).resolve().parents[1]
CALLERS = ('src/krusty2/vehicle/KrustyBike.cpp',
           'samples/physics/krustybike/KrustyBikeNearMisses.cpp')
PROVIDERS = (
    ('Net', 'NetworkInterface::Send', '?Send@NetworkInterface@@', 0x4ac830, 145, 20),
    ('Recorder', 'VCRInterface::QueueRecord', '?QueueRecord@VCRInterface@@', 0x4e8720, 240, 16),
    ('EventManager', 'EventManager::FindRaceMode', '?FindRaceMode@EventManager@@', 0x45d2b0, 53, 0))


def function_text(source, name):
    text = source.read_text()
    match = re.search(r'^[^\n;{}]*\b' + re.escape(name) + r'\([^;{}]*\)\s*\{', text, re.M)
    if not match:
        raise AssertionError(name)
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end] + '\n'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class BikeNetworkConnectionsTests(unittest.TestCase):
    def test_callers_complete_providers_and_native_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        classes = {c.name: c for c in parse_rtti(retail)}
        for derived, base in (('KrustyVCR', 'VCRInterface'), ('VCRInterface', 'GameObject'),
                              ('EventManager', 'GameObject'), ('NetworkInterface', 'NetworkInterface')):
            relation = next(b for b in classes[derived].bases if b.name == base)
            self.assertEqual((relation.mdisp, relation.pdisp, relation.vdisp), (0, -1, 0))
        partials = {tuple(row[:4]): row[4:] for row in json.loads(
            (ROOT / 'tests/fixtures/vehicle_gearbox_partials.json').read_text())['partials']
            if row[0] in CALLERS}
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-network-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            def compile_source(source, key):
                obj = output / ('unit%d.obj' % len(paths))
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[key], objects[key] = obj, CoffObject(obj)
            for source in (*CALLERS, *(f'src/reconstructed/{p[0]}.cpp' for p in PROVIDERS),
                           'src/reconstructed/VCR.cpp', 'tests/fixtures/bike_network_layout.cpp',
                           'tests/fixtures/bike_network_boundaries.cpp'):
                compile_source(ROOT / source, source)
            addresses = {}
            for stem, name, symbol, va, size, popped in PROVIDERS:
                source = ROOT / f'src/reconstructed/{stem}.cpp'
                headers = [stem + '.h']
                if stem == 'Recorder':
                    headers += ['VCR.h']
                if stem == 'EventManager':
                    headers += ['TrackGame.h']
                extracted = output / (stem + '.cpp')
                extracted.write_text('#define WIN32_LEAN_AND_MEAN\n#include <string.h>\n' + ''.join(
                    '#include "%s"\n' % (ROOT / 'src/reconstructed' / header).as_posix()
                    for header in headers) + function_text(source, name))
                compile_source(extracted, stem)
                for key in (stem, f'src/reconstructed/{stem}.cpp'):
                    obj = objects[key]
                    entry = obj.find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, obj, symbol, va, size,
                        source.with_suffix('.bindings.json'))['strict_exact'], key)
                    addresses[entry.name] = va
                instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in instructions), size)
                returns = [i for i in instructions if i.mnemonic == 'ret']
                self.assertTrue(returns)
                self.assertTrue(all((i.operands[0].imm if i.operands else 0) == popped for i in returns))
            rows = [row for row in gearbox_cases() if row[0] in CALLERS]
            rows += [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
                     for c in CASES if c['source'] in
                     {f'src/reconstructed/{p[0]}.cpp' for p in PROVIDERS} | {'src/reconstructed/VCR.cpp'}]
            strict = preserved = 0
            calls = Counter()
            for source, symbol, va, size, binding in dict.fromkeys(rows):
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key in partials:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match'].get('matching_positions')),
                                     (extent, positions), key)
                    self.assertEqual(fingerprint(obj, symbol), digest, key)
                    preserved += 1
                else:
                    self.assertTrue(result['strict_exact'], key)
                    strict += 1
                if source in CALLERS:
                    entry = obj.find_symbol(symbol)
                    _, _, relocs = obj.symbol_extent(entry)
                    connected = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs
                        if r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name in addresses)
                    original = Counter(i.operands[0].imm for i in decoder.disasm(retail.bytes_at_va(va, size), va)
                        if i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM)
                    for name, count in connected.items():
                        self.assertEqual(original[addresses[name]], count, key)
                    # A symbol can occur more than once in the target manifests.
                    for name, count in connected.items():
                        calls[(source, entry.name, name)] = count
            self.assertEqual(preserved, len(partials))
            self.assertEqual(sum(n for (s, _, _), n in calls.items() if s == CALLERS[0]), 8)
            self.assertEqual(sum(n for (s, _, _), n in calls.items() if s == CALLERS[1]), 12)
            for source in CALLERS:
                self.assertFalse(any(any(old in s.name for old in
                    ('Send@KbGameCfg', 'QueueRecord@KbRecorder', 'FindRaceMode@KbSession'))
                    for s in objects[source].symbols))
            # Actual setter forwards the recorder's primary pointer unchanged.
            setter = list(decoder.disasm(retail.bytes_at_va(0x496e20, 16), 0x496e20))
            self.assertEqual([(i.mnemonic, i.op_str) for i in setter[:3]],
                             [('mov', 'eax, dword ptr [esp + 4]'),
                              ('mov', 'dword ptr [ecx + 0x13fc], eax'), ('ret', '4')])
            # The collision-record caller deliberately queues one byte even
            # though its preceding transport call passes a twelve-byte packet.
            packet_call = list(decoder.disasm(retail.bytes_at_va(0x49264b, 18), 0x49264b))
            self.assertEqual([(i.mnemonic, i.op_str) for i in packet_call],
                [('lea', 'edx, [esp + 8]'), ('push', '1'), ('push', 'edx'), ('push', 'eax'),
                 ('push', '0x87'), ('call', '0x4e8720')])
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'boundaries.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[key]) for key in ('Net', 'Recorder', 'EventManager', 'src/reconstructed/VCR.cpp',
                    'tests/fixtures/bike_network_layout.cpp', 'tests/fixtures/bike_network_boundaries.cpp')),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Bike network: 4896 native send, recorder and race-owner cases passed')
            print('%d strict bodies, %d complete partial fingerprints, eight production calls; %s' %
                  (strict, preserved, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
