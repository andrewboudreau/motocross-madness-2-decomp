"""Complete bike text callers, emitted providers, and bounded native ownership."""
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
from test_vehicle_gearbox_connections import cases as gearbox_cases, fingerprint

ROOT = Path(__file__).resolve().parents[1]
CALLERS = ('src/krusty2/vehicle/KrustyBike.cpp', 'samples/physics/krustybike/KrustyBikeNearMisses.cpp')
PROVIDERS = (
    ('TrackOverlay', 'UnknownMessage::UnknownMessage(const char* text, float duration)', '??0UnknownMessage@@QAE@PBDM@Z', 0x51b200, 68),
    ('TrackOverlay', 'UnknownMessage::UnknownMessage(const UnknownMessage& other)', '??0UnknownMessage@@QAE@ABV0@@Z', 0x51b250, 74),
    ('TrackOverlay', 'void TextQueueOverlay::ReplaceHeadMessage(UnknownMessage* message)', '?ReplaceHeadMessage@TextQueueOverlay@@', 0x51b540, 146),
    ('EventManager', 'TextQueueOverlay* EventManager::FindTextQueue()', '?FindTextQueue@EventManager@@', 0x45d340, 78),
    ('TrackGame', 'int TrackGame::LoadResourceString(int id, char* buffer, int size)', '?LoadResourceString@TrackGame@@', 0x521970, 189))


def extract(source, signature):
    text = source.read_text()
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    # Preserve the real production source location for __FILE__, which is an
    # observed allocator diagnostic argument, not a substituted method body.
    return '#line %d %s\n%s\n' % (text[:start].count('\n') + 1, json.dumps(str(source)), text[start:end])


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class BikeTextConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_and_native_text_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        classes = {c.name: c for c in parse_rtti(retail)}
        for derived, base in (('TextQueueOverlay', 'GameObject'), ('EventManager', 'GameObject'), ('TrackGame', 'Game')):
            relation = next(b for b in classes[derived].bases if b.name == base)
            self.assertEqual((relation.mdisp, relation.pdisp, relation.vdisp), (0, -1, 0))
        creation = {i.address: (i.mnemonic, i.op_str) for i in decoder.disasm(
            retail.bytes_at_va(0x4e0ade, 0x58), 0x4e0ade)}
        for address, instruction in {0x4e0af8: ('call', '0x51b2a0'),
            0x4e0b14: ('mov', 'dword ptr [esi + 0x6c], eax'), 0x4e0b28: ('call', '0x51b320'),
            0x4e0b2f: ('mov', 'dword ptr [esi + 0x6c], eax')}.items():
            self.assertEqual(creation[address], instruction)
        partials = {tuple(row[:4]): row[4:] for row in json.loads(
            (ROOT / 'tests/fixtures/vehicle_gearbox_partials.json').read_text())['partials'] if row[0] in CALLERS}
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-text-') as directory:
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
            provider_sources = tuple(dict.fromkeys(f'src/reconstructed/{p[0]}.cpp' for p in PROVIDERS))
            fixtures = ('tests/fixtures/bike_text_layout.cpp', 'tests/fixtures/bike_text_boundaries.cpp')
            for source in (*CALLERS, *provider_sources, *fixtures):
                compile_source(ROOT / source, source)
            for stem in dict.fromkeys(p[0] for p in PROVIDERS):
                source = ROOT / f'src/reconstructed/{stem}.cpp'
                target = output / (stem + '.cpp')
                prefix = '#include <stdio.h>\n#include <string.h>\n' + ''.join(
                    '#include "%s"\n' % (ROOT / 'src/reconstructed' / h).as_posix()
                    for h in ('TrackGame.h', 'EventManager.h', 'RaceView.h', 'DebugAlloc.h'))
                if stem == 'TrackGame':
                    prefix += 'extern "C" __declspec(dllimport) int __stdcall LoadStringA(void*,unsigned int,char*,int);\n'
                target.write_text(prefix + ''.join(extract(source, p[1]) for p in PROVIDERS if p[0] == stem))
                compile_source(target, stem)
            addresses = {}
            for stem, _, symbol, va, size in PROVIDERS:
                for key in (stem, f'src/reconstructed/{stem}.cpp'):
                    obj = objects[key]
                    entry = obj.find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    result = compare(retail, obj, symbol, va, size, ROOT / f'src/reconstructed/{stem}.bindings.json')
                    self.assertTrue(result['strict_exact'], (key, symbol, result))
                addresses[entry.name] = va
            rows = [r for r in gearbox_cases() if r[0] in CALLERS]
            rows += [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
                     for c in CASES if c['source'] in provider_sources]
            strict = preserved = 0
            calls = {}
            for source, symbol, va, size, binding in dict.fromkeys(rows):
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key in partials:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match'].get('matching_positions')), (extent, positions), key)
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
                        calls[(source, entry.name, name)] = count
            self.assertEqual(preserved, len(partials))
            for source in CALLERS:
                self.assertEqual(sum(n for (s, _, _), n in calls.items() if s == source), 18)
                self.assertFalse(any(any(old in s.name for old in ('GetStringText@KbGame', 'FindTextQueue@KbSession',
                    'ReplaceHeadMessage@KbMsgSink', '??0KbMessage@@')) for s in objects[source].symbols))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'boundaries.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[key]) for key in ('TrackOverlay', 'EventManager', 'TrackGame', *fixtures)),
                'kernel32.lib', 'user32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Bike text: 4704 native message, queue, owner and resource cases passed')
            print('%d strict bodies, %d complete partial fingerprints, 18 production calls; %s' %
                  (strict, preserved, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
