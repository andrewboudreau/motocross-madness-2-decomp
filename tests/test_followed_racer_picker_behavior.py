"""Whole followed-racer selection, real iterator/FP conversion, and packet contract."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?PickFollowedRacer@TrackGameViewOwner@@QAEXXZ'
NEXT = '?NextBike@BikeRace@@QAEPAVUnknownBikeRaceRacer@@PAH@Z'
DIRECT = {0x4a9d54: 0x4204e0, 0x4a9d70: 0x4204e0, 0x4a9da5: 0x534c1b,
          0x4a9dbc: 0x534a88, 0x4a9e23: 0x4ac830, 0x4a9e69: 0x4e8720}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class FollowedRacerPickerBehaviorTests(unittest.TestCase):
    def test_complete_picker_defined_payload_and_callback_contract(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4a9d20, 340), 0x4a9d20))
        self.assertEqual(sum(i.size for i in instructions), 340)
        self.assertEqual([(i.address, bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'], [(0x4a9e73, b'\xc3')])
        self.assertEqual(image.bytes_at_va(0x4a9e74, 12), b'\x90' * 12)
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        boundaries = {i.address for i in instructions}
        direct, patches = {}, []
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(instruction.operands[0].imm, boundaries)
            if instruction.mnemonic == 'call':
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertEqual(instruction.bytes[0], 0xe8)
                direct[instruction.address] = instruction.operands[0].imm
                patches.append(struct.pack('<III', instruction.address, instruction.operands[0].imm, 0xe8))
        self.assertEqual(direct, DIRECT)
        # These are the only writes into the twelve-byte outgoing packet:
        # byte +4 and DWORD +8. Its other seven bytes remain unspecified.
        self.assertEqual(image.bytes_at_va(0x4a9e07, 5), b'\xc6\x44\x24\x24\0')
        self.assertEqual(image.bytes_at_va(0x4a9e0c, 4), b'\x89\x54\x24\x28')
        self.assertEqual(image.bytes_at_va(0x5505f0, 4), struct.pack('<f', 1 / 32768))
        for va, size, site in ((0x420590, 189, 0x42062a), (0x4526b0, 632, 0x4527ef)):
            # HandleMenuCancel's complete extent also contains its switch tables.
            code_size = 0x4528a3 - va if va == 0x4526b0 else size
            caller = list(decoder.disasm(image.bytes_at_va(va, code_size), va))
            self.assertEqual(sum(i.size for i in caller), code_size)
            self.assertEqual([(i.address, i.operands[0].imm) for i in caller if i.mnemonic == 'call'
                and i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == 0x4a9d20], [(site, 0x4a9d20)])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-follow-picker-') as directory:
            output = Path(directory)

            def compile_source(source, name, fixture=False):
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'), *(['--extra', '/Od'] if fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source(ROOT / 'samples/race/FollowedRacerPicker.cpp', 'candidate')
            fixture_path, _ = compile_source(ROOT / 'tests/fixtures/followed_racer_picker_compare.cpp', 'fixture', True)
            self.assertEqual(candidate.find_symbol(SYMBOL).function_size, 342)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL, '??_H@YGXPAXIHP6EX0@Z@Z'})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0},
                {NEXT, '?Send@NetworkInterface@@QAEHHPAXHHH@Z', '?QueueRecord@VCRInterface@@QAEHHHPBXI@Z',
                 '?g_TrackGame@@3PAVTrackGame@@A', '_rand', '__ftol', '__fltused'})
            self.assertFalse(any(s.name.startswith(('.CRT', '.text$x', '.xdata')) for s in candidate.sections))
            report = compare(image, candidate, SYMBOL, 0x4a9d20, 340, ROOT / 'samples/race/FollowedRacerPicker.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 67)
            _, aggregate = compile_source(ROOT / 'samples/race/FollowRacerNearMisses.cpp', 'aggregate')

            def fingerprint(obj):
                symbol = obj.find_symbol(SYMBOL)
                body, size, relocations = obj.symbol_extent(symbol)
                return body, size, [(r.virtual_address - symbol.value, r.type, obj.symbol_by_index[r.symbol_index].name) for r in relocations]

            self.assertEqual(fingerprint(candidate), fingerprint(aggregate))
            # Execute the unchanged canonical iterator on an empty derived view
            # solely to allow the observing callback to keep its own symbol.
            source = (ROOT / 'src/reconstructed/BikeRace.cpp').read_text()
            start = source.index('UnknownBikeRaceRacer* BikeRace::NextBike(int* iterator) {')
            end = source.index('\n}\n', start) + 3
            helper_file = output / 'iterator.cpp'
            helper_file.write_text('#include "BikeRaceCore.h"\n#include "TrackGame.h"\n'
                'class ActualBikeRace : public BikeRace {public:UnknownBikeRaceRacer* NextBike(int*);};\n' +
                source[start:end].replace('BikeRace::NextBike', 'ActualBikeRace::NextBike'))
            helper_path, helper = compile_source(helper_file, 'iterator')
            helper_symbol = '?NextBike@ActualBikeRace@@QAEPAVUnknownBikeRaceRacer@@PAH@Z'
            self.assertEqual(helper.symbol_extent(helper.find_symbol(helper_symbol))[1], 162)
            self.assertTrue(compare(image, helper, helper_symbol, 0x4204e0, 162,
                ROOT / 'src/reconstructed/BikeRace.bindings.json')['strict_exact'])
            for owner, name, va, size in [
                ('BikeRace', NEXT, 0x4204e0, 162),
                ('BikeRace', '?HandlePlayerLeft@BikeRace@@QAEXH@Z', 0x420590, 189),
                ('DlgProcs', 'HandleMenuCancel', 0x4526b0, 632),
                ('Net', '?Send@NetworkInterface@@QAEHHPAXHHH@Z', 0x4ac830, 145),
                ('Recorder', '?QueueRecord@VCRInterface@@QAEHHHPBXI@Z', 0x4e8720, 240),
            ]:
                _, provider = compile_source(ROOT / f'src/reconstructed/{owner}.cpp', owner)
                self.assertEqual(provider.symbol_extent(provider.find_symbol(name))[1], size)
                self.assertTrue(compare(image, provider, name, va, size,
                    ROOT / f'src/reconstructed/{owner}.bindings.json')['strict_exact'])
            layout = output / 'layout.cpp'
            layout.write_text('#include "BikeRace.h"\n#include "TrackGame.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UnknownBikeRaceVcrFollow)==12 && '
                'offsetof(UnknownBikeRaceVcrFollow,field_0x04)==4 && offsetof(UnknownBikeRaceVcrFollow,followedRacerId)==8 && '
                'offsetof(TrackGameViewOwner,raceView)==0x34 && offsetof(TrackGameViewOwner,followedRacer)==0xa8 && '
                'offsetof(UnknownEventRacer,outOfRace)==0x4a0 && offsetof(UnknownEventRacer,networkPlayerId)==0x11bc && '
                'offsetof(BikeRace,racerSlots)==0x3c && offsetof(BikeRace,replayVcr)==0x1a0 && '
                'offsetof(UnknownKrustyBikeView,racerSlots)==0x3c && offsetof(UnknownKrustyBikeView,replayVcr)==0x1a0) ? 1 : -1];\n')
            compile_source(layout, 'layout')
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'picker.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path), str(helper_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('FollowedRacerPicker: 4270 complete selection, iterator, packet, callback, FP and fault scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
