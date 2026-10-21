"""Complete audio loader body, private dependency instrumentation and state proof."""
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
SYMBOL = '?UnknownThreadProc@UnknownPCAudioObject@@SGIPAX@Z'
HELPERS = {
    '?MakeRoomForSound@UnknownPCAudioObject@@QAEXH@Z': 0x4be130,
    '?CreatePendingBuffer@Sound@@QAEHXZ': 0x4bd4b0,
    '?FillBufferFromFile@Sound@@QAEHPAPAUUnknownSoundBuffer@@@Z': 0x4bd0c0,
    '?Play@Sound@@QAEHXZ': 0x4bbcd0,
    '?RecordLoadedSound@UnknownPCAudioObject@@QAEXPAVSound@@H@Z': 0x4be0a0,
}
IMPORTS = {'__imp__WaitForMultipleObjects@16': 0,
           '__imp__EnterCriticalSection@4': 1, '__imp__LeaveCriticalSection@4': 2}


def signature(coff, symbol):
    entry = coff.find_symbol(symbol)
    body, size, relocations = coff.symbol_extent(entry)
    return body, size, [(r.virtual_address - entry.value, r.type,
                        coff.symbol_by_index[r.symbol_index].name) for r in relocations]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class AudioLoaderThreadBehaviorTests(unittest.TestCase):
    def test_complete_loader_queue_ownership_locks_and_faults(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4bdc00, 483), 0x4bdc00))
        self.assertEqual(sum(i.size for i in instructions), 483)
        self.assertEqual(bytes(instructions[-1].bytes), bytes.fromhex('c20400'))
        self.assertEqual(image.bytes_at_va(0x4bdde3, 13), b'\x90' * 13)
        boundaries = {i.address for i in instructions}
        patches, calls, indirect = [], {}, set()
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm, boundaries)
            if i.mnemonic == 'call':
                if i.operands[0].type == capstone.x86.X86_OP_IMM:
                    self.assertEqual(i.bytes[0], 0xe8)
                    calls[i.address] = i.operands[0].imm
                    patches.append(struct.pack('<III', 0, i.address, i.operands[0].imm))
                else:
                    self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_MEM)
                    self.assertEqual(i.operands[0].mem.base, 0)
                    indirect.add(i.operands[0].mem.disp)
        self.assertEqual(calls, {0x4bdd01: 0x4be130, 0x4bdd18: 0x4bd4b0,
                                0x4bdd27: 0x4bd0c0, 0x4bdd71: 0x4bbcd0,
                                0x4bdd9b: 0x4be0a0, 0x4bddcf: 0x537c9f})
        self.assertEqual(indirect, {0x5500d8, 0x55025c, 0x550284})
        # Complete body has no local EH registration or cleanup funclet. A
        # callback exception propagates with the locks held at that boundary.
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-audio-loader-') as directory:
            output = Path(directory)

            def compile_source(source):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source('samples/audio/AudioLoaderThread.cpp')
            _, aggregate = compile_source('samples/audio/PCAudioNearMisses.cpp')
            self.assertEqual(signature(candidate, SYMBOL), signature(aggregate, SYMBOL))
            entry = candidate.find_symbol(SYMBOL)
            self.assertEqual(entry.extent_source, 'coff_function_aux')
            self.assertEqual(entry.function_size, 478)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number > 0 and s.storage_class == 2}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.section_number == 0 and s.storage_class == 2},
                             set(HELPERS) | set(IMPORTS) | {'__endthreadex'})
            report = compare(image, candidate, SYMBOL, 0x4bdc00, 483,
                             ROOT / 'samples/audio/AudioLoaderThread.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 91)
            body, size, relocations = candidate.symbol_extent(entry)
            for r in relocations:
                name = candidate.symbol_by_index[r.symbol_index].name
                offset = r.virtual_address - entry.value
                if name in IMPORTS:
                    self.assertEqual(r.type, 6)  # i386 DIR32 import-cell operand
                    self.assertEqual(body[offset:offset+4], b'\0' * 4)
                    patches.append(struct.pack('<III', 1, offset, IMPORTS[name]))
                elif name == '__endthreadex':
                    self.assertEqual(r.type, 20)
                    self.assertEqual(body[offset-1], 0xe8)
                    patches.append(struct.pack('<III', 2, offset, 0))
                else:
                    self.assertIn(name, HELPERS)
            self.assertEqual(len(patches), 21)
            # Every game boundary exists in the actual canonical TU. The
            # differential fixture substitutes their behavior, not the loader.
            _, providers = compile_source('src/reconstructed/PCAudio.cpp')
            bindings = json.loads((ROOT / 'src/reconstructed/PCAudio.bindings.json').read_text())
            for symbol, address in HELPERS.items():
                self.assertGreater(providers.find_symbol(symbol).function_size, 0)
                self.assertEqual(int(bindings[symbol], 0), address)
            fixture, _ = compile_source('tests/fixtures/audio_loader_thread_compare.cpp')
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'loader.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(candidate_path), str(fixture), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)],
                                    capture_output=True, text=True, timeout=90)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('AudioLoaderThread: 37387 complete-body state, queue, lock and callback scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
