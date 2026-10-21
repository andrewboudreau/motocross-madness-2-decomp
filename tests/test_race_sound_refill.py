"""Complete race refill, strict Sound boundaries, buffer oracles and partial faults."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as build_compile_source

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/race/RaceSoundRefill.cpp'
AGGREGATE = 'samples/race/RaceSoundNearMisses.cpp'
CANONICAL = 'src/reconstructed/RaceSound.cpp'
SYMBOL = '?RefillStream@RaceSound@@QAEXXZ'
HELPERS = (
    ('GetBufferPosition', '?GetBufferPosition@Sound@@QAEHPAK0@Z', 0x4bcd40, 57, 0x10),
    ('LockBuffer', '?LockBuffer@Sound@@QAEHKKPAPAXPAK01K@Z', 0x4bd020, 85, 0x2c),
    ('UnlockBuffer', '?UnlockBuffer@Sound@@QAEHPAXK0K@Z', 0x4bd080, 56, 0x4c),
)
CALLS = {0x4e4d34: 0x4bcd40, 0x4e4d8a: 0x4bd020, 0x4e4eae: 0x534c1b,
         0x4e4f19: 0x534c1b, 0x4e4f81: 0x534c1b, 0x4e50aa: 0x4bd080,
         0x4e511d: 0x4bd020, 0x4e52b7: 0x534c1b, 0x4e5322: 0x534c1b,
         0x4e538a: 0x534c1b, 0x4e54e6: 0x4bd080}


def signature(obj, query):
    symbol = obj.find_symbol(query)
    data, size, relocations = obj.symbol_extent(symbol)
    return data, size, [(r.virtual_address-symbol.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


def definition(text, name):
    match = re.search(r'^int Sound::'+name+r'\([^{}]*\)\s*{', text, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (text[end] == '{')-(text[end] == '}')
        end += 1
    return text[match.start():end]


def complete_object(obj):
    """Every nondebug byte/flag/destination; only a known checkout prefix varies."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefix = str(ROOT).replace('\\', '/')+'/'
    def filename(data):
        text, nul, tail = data.partition(b'\0')
        value = text.decode('latin1').replace('\\', '/')
        if nul and value.lower().startswith(prefix.lower()) and value[len(prefix):] == CANONICAL:
            return ('source:'+CANONICAL).encode()+nul+tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return symbol.name
    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((name(s), s.value) for s in obj.symbols
                       if s.section_number == section.index and (s.storage_class == 2 or s.type == 32)))
        if labels:
            keys[section.index] = ('named', labels)
        else:
            count = anonymous.get(section.name, 0)
            anonymous[section.name] = count+1
            keys[section.index] = ('anonymous', section.name, count)
    assert len(set(keys.values())) == len(keys)
    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocations))
    return sorted(rows, key=lambda r: repr(r[0]))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and hash-pinned retail')
class RaceSoundRefillTests(unittest.TestCase):
    def test_full_refill_real_sound_methods_and_native_buffers(self):
        import capstone
        from tools.run_calibration import CASES
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def decode(va, size):
            result = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in result), size)
            return result
        instructions = decode(0x4e4d10, 2032)
        self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'ret'],
                         [(0x4e50c3, ''), (0x4e54ff, '')])
        boundaries = {i.address for i in instructions}
        calls = {}
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm, boundaries)
            if i.mnemonic == 'call':
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertEqual(i.bytes[0], 0xe8)
                calls[i.address] = i.operands[0].imm
        self.assertEqual(calls, CALLS)
        self.assertFalse(any('fs:' in i.op_str or i.mnemonic.startswith('f') for i in instructions))
        self.assertEqual(retail.bytes_at_va(0x4e5500, 2), b'\x6a\xff')
        # Independent identity: primary RTTI and strict full caller chain.
        col, = struct.unpack('<I', retail.bytes_at_va(0x557710-4, 4))
        self.assertEqual(col, 0x55eb78)
        _, offset, _, td, _ = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(td+8, 32).split(b'\0')[0], b'.?AVRaceSound@@')
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557710+10*4, 4))[0], 0x4e3730)
        self.assertEqual([(i.address, i.op_str) for i in decode(0x4e3730, 632)
                          if i.mnemonic == 'call' and i.op_str == '0x4e39b0'], [(0x4e3808, '0x4e39b0')])
        self.assertEqual([(i.address, i.op_str) for i in decode(0x4e39b0, 2348)
                          if i.mnemonic == 'call' and i.op_str == '0x4e4d10'], [(0x4e4013, '0x4e4d10')])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-race-refill-') as directory:
            output = Path(directory)
            def compile_source(source, name):
                obj = output/(name+'.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT/'src/krusty2')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return obj, CoffObject(obj)
            candidate_path, candidate = compile_source(ROOT/SOURCE, 'candidate')
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = build_compile_source(ROOT/SOURCE, output/'game-command', compiler,
                [ROOT/p for p in spec['includes']], spec['profile'], {**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertTrue(built['compiled'], Path(built['log']).read_text())
            self.assertEqual(complete_object(candidate), complete_object(CoffObject(Path(built['object']))))
            _, aggregate = compile_source(ROOT/AGGREGATE, 'aggregate')
            _, canonical = compile_source(ROOT/CANONICAL, 'canonical')
            self.assertEqual(signature(candidate, SYMBOL), signature(aggregate, SYMBOL))
            entry = candidate.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size, entry.extent_source), (2038, 'coff_function_aux'))
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0},
                             {h[1] for h in HELPERS} | {'_rand'})
            report = compare(retail, candidate, SYMBOL, 0x4e4d10, 2032, ROOT/'samples/race/RaceSoundRefill.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 806)
            rows = complete_object(aggregate)
            self.assertEqual((len(rows), sum(len(row[3]) for row in rows)), (106, 579))
            self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(), 'f4f0a753f4bd506b302d2340c73c345ac4a4f54a35b940679bdf3018728a743a')
            cases = [c for c in CASES if c['source'] == CANONICAL]
            self.assertEqual(len(cases), 26)
            for case in cases:
                for obj in (canonical, aggregate):
                    result = compare(retail, obj, case['symbol'], int(case['target_va'], 0), case['target_size'], ROOT/case['bindings'])
                    self.assertTrue(result['strict_exact'], result)
            # Extract actual source definitions only into a private validation TU.
            # Each is independently strict against retail and the complete canonical object.
            audio_source = (ROOT/'src/reconstructed/PCAudio.cpp').read_text()
            helper_source = output/'helpers.cpp'
            helper_source.write_text('#include "'+(ROOT/'src/reconstructed/PCAudio.h').as_posix()+'"\n'
                '#include "'+(ROOT/'src/reconstructed/TrackGame.h').as_posix()+'"\n'
                '#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)\n'+
                '\n'.join(definition(audio_source, h[0]) for h in HELPERS)+'\n')
            helper_path, helpers = compile_source(helper_source, 'helpers')
            _, audio = compile_source(ROOT/'src/reconstructed/PCAudio.cpp', 'audio')
            for _, symbol, va, size, slot in HELPERS:
                for obj in (helpers, audio):
                    result = compare(retail, obj, symbol, va, size, ROOT/'src/reconstructed/PCAudio.bindings.json')
                    self.assertTrue(result['strict_exact'], result)
                self.assertEqual(signature(helpers, symbol), signature(audio, symbol))
                call = [i for i in decode(va, size) if i.mnemonic == 'call']
                self.assertEqual(len(call), 1)
                self.assertEqual(call[0].operands[0].type, capstone.x86.X86_OP_MEM)
                self.assertEqual(call[0].operands[0].mem.disp, slot)
            patches = [struct.pack('<III', 0, site, target) for site, target in CALLS.items()]
            data, _, relocations = candidate.symbol_extent(entry)
            for r in relocations:
                if candidate.symbol_by_index[r.symbol_index].name == '_rand':
                    offset = r.virtual_address-entry.value
                    self.assertEqual(r.type, 20)
                    self.assertEqual(data[offset-1], 0xe8)
                    self.assertEqual(data[offset:offset+4], b'\0'*4)
                    patches.append(struct.pack('<III', 1, offset, 0x534c1b))
            self.assertEqual(len(patches), 17)
            patch_file = output/'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            fixture, _ = compile_source(ROOT/'tests/fixtures/race_sound_refill_compare.cpp', 'fixture')
            executable = output/'refill.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(helper_path), str(fixture), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image), str(patch_file)], capture_output=True, text=True, timeout=90)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertIn('RaceSoundRefill: 4782 complete-state, sample-copy, COM and fault scenarios match', result.stdout)
            print('RaceSoundRefill: 58 strict checks; '+result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
