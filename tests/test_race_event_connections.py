"""Race clocks and sound-option messages use existing event providers."""
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
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

ROOT = Path(__file__).resolve().parents[1]
SOURCES = tuple('src/reconstructed/' + n + '.cpp' for n in
                ('BikeRace', 'DlgProcs', 'RaceSound', 'QuarryStuntEvent', 'EventManager', 'TrackGame')) + (
                'samples/race/BikeRaceNearMisses.cpp', 'samples/ui/DlgProcsNearMisses.cpp',
                'samples/race/RaceSoundNearMisses.cpp')
PROVIDERS = (('?ResetClock@BaseQuarryEvent@@QAEXXZ', 0x4e1f00, 9),
             ('?ShowOnOffMessage@BaseQuarryEvent@@QAEXHH@Z', 0x4e0c30, 175))


def cases():
    rows = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
            for c in CASES if c['source'] in SOURCES]
    for original, aggregate in zip(SOURCES[:3], SOURCES[6:]):
        rows += [(aggregate, symbol, va, size, aggregate.replace('.cpp', '.bindings.json'))
                 for source, symbol, va, size, _ in list(rows) if source == original]
    return list(dict.fromkeys(rows))


def fingerprint(obj, symbol):
    entry = obj.find_symbol(symbol)
    body, size, relocs = obj.symbol_extent(entry)
    def identity(target):
        if target.name.startswith('??_C@') and target.section_number > 0:
            section = obj.sections[target.section_number - 1]
            value = obj.data[section.raw_ptr + target.value:section.raw_ptr + section.raw_size].split(b'\0')[0]
            filename = value.decode('latin1').replace('\\', '/')
            for source in SOURCES:
                if filename.endswith('/' + source):
                    return 'source-filename:' + source
        return target.name
    targets = [(r.virtual_address - entry.value, r.type, identity(obj.symbol_by_index[r.symbol_index]),
                obj.symbol_by_index[r.symbol_index].value) for r in relocs]
    return hashlib.sha256(body + json.dumps(targets).encode()).hexdigest()


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class RaceEventConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_prefix_layout_and_clock_storage(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, parent in (('BaseQuarryEvent', 'GameObject'), ('NationalRace', 'BaseQuarryEvent')):
            base = next(b for b in classes[name].bases if b.name == parent)
            self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        evidence = json.loads((ROOT / 'tests/fixtures/race_event_partials.json').read_text())
        partial_keys = {(r[0], r[1]) for r in evidence['partials']}
        with tempfile.TemporaryDirectory(prefix='mcm2-race-event-') as directory:
            output = Path(directory)
            # Extract the actual provider body, then require the extracted
            # emission and original production object to match the complete VA.
            text = (ROOT / SOURCES[3]).read_text()
            start = text.index('void BaseQuarryEvent::ResetClock()')
            end = text.index('\n}', start) + 2
            reset = output / 'reset.cpp'
            reset.write_text('#include "BaseQuarryEvent.h"\n' + text[start:end] + '\n')
            fixture = 'tests/fixtures/race_event_boundaries.cpp'
            objects, paths = {}, {}
            for index, source in enumerate((*SOURCES, str(reset), fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), '--include', str(ROOT / 'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            addresses = {}
            for symbol, va, size in PROVIDERS:
                obj = objects[SOURCES[3]]
                self.assertEqual((obj.find_symbol(symbol).function_size, obj.find_symbol(symbol).extent_source),
                                 (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        ROOT / 'src/reconstructed/QuarryStuntEvent.bindings.json')['strict_exact'])
                instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in instructions), size)
                addresses[symbol] = va
            self.assertTrue(compare(retail, objects[str(reset)], PROVIDERS[0][0], 0x4e1f00, 9,
                                    ROOT / 'src/reconstructed/QuarryStuntEvent.bindings.json')['strict_exact'])
            clock = list(decoder.disasm(retail.bytes_at_va(0x4e1f00, 9), 0x4e1f00))
            stores = [i.operands[0] for i in clock if i.mnemonic == 'mov' and i.operands[0].type == capstone.x86.X86_OP_MEM]
            self.assertEqual([(i.mem.base, i.mem.disp, i.size) for i in stores],
                             [(capstone.x86.X86_REG_ECX, 0x70, 4), (capstone.x86.X86_REG_ECX, 0x74, 4)])
            strict = 0
            calls = Counter()
            for source, symbol, va, size, binding in cases():
                obj = objects[source]
                if (source, obj.find_symbol(symbol).name) not in partial_keys:
                    self.assertTrue(compare(retail, obj, symbol, va, size, ROOT / binding)['strict_exact'], (source, symbol))
                    strict += 1
                if source not in SOURCES[:3]:
                    continue
                entry = obj.find_symbol(symbol)
                _, _, relocs = obj.symbol_extent(entry)
                connected = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs
                                    if r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name in addresses)
                if connected:
                    instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                    original = Counter(i.operands[0].imm for i in instructions if i.mnemonic == 'call'
                                       and i.operands[0].type == capstone.x86.X86_OP_IMM)
                    for name, count in connected.items():
                        self.assertEqual(original[addresses[name]], count, (source, symbol, name))
                    calls.update(connected)
            self.assertEqual(calls, Counter({PROVIDERS[0][0]: 4, PROVIDERS[1][0]: 2}))
            for source, symbol, extent, digest in evidence['partials']:
                self.assertEqual(objects[source].find_symbol(symbol).function_size, extent)
                self.assertEqual(fingerprint(objects[source], symbol), digest, (source, symbol))
            for source in SOURCES:
                self.assertFalse(any(('ResetClock@TrackGameViewOwner' in s.name or
                                     'ShowOnOffMessage@TrackGameViewOwner' in s.name) for s in objects[source].symbols))
            self.assertEqual(objects[fixture].symbol_extent(objects[fixture].find_symbol('_EventPrimaryBase')),
                             (bytes.fromhex('33c0c3'), 3, []))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'clock.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths[str(reset)]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Race event clock: 256 full-storage cases passed')
            print('%d strict bodies, %d full partial fingerprints, six production calls; %s' %
                  (strict, len(evidence['partials']), result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
