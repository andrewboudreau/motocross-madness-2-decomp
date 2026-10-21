"""Complete replay tick CFG and standalone native behavior against private retail."""
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
SYMBOL = '?Tick@VCRInterface@@UAEHM@Z'
HELPERS = {
    '?TakeRecord@VCRInterface@@QAEHPAH00PAXPAIH@Z': 0x4e8810,
    '?StopReading@VCRInterface@@QAEXXZ': 0x4e8990,
    '?RestartReverseRead@VCRInterface@@QAEXXZ': 0x4e89c0,
    '?Seek@VCRInterface@@QAEXI@Z': 0x4e8a20,
    '?Rewind@VCRInterface@@QAEXXZ': 0x4e8a70,
    '?AppendPosition@UnknownVcr@@QAEXHH@Z': 0x524590,
    '?SeekPosition@UnknownVcr@@QAEXH@Z': 0x524870,
}
IMPORTS = {'__imp__EnterCriticalSection@4': 0, '__imp__LeaveCriticalSection@4': 1}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class RecorderTickBehaviorTests(unittest.TestCase):
    def test_complete_playback_state_callbacks_and_locks(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4e7d60, 2364), 0x4e7d60))
        self.assertEqual(sum(i.size for i in instructions), 2364)
        self.assertEqual((instructions[-1].address, bytes(instructions[-1].bytes)), (0x4e8699, b'\xc2\x04\0'))
        self.assertEqual(image.bytes_at_va(0x4e86cc, 4), b'\x90' * 4)
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x55778c + 10 * 4, 4))[0], 0x4e7d60)
        table = struct.unpack('<12I', image.bytes_at_va(0x4e869c, 48))
        boundaries = {i.address for i in instructions}
        self.assertTrue(set(table) <= boundaries)
        patches = []
        targets = set()
        register_calls = {
            0x4e7d73: 'ebp', 0x4e7db8: 'ebp', 0x4e7dd0: 'ebp', 0x4e7dff: 'ebp',
            0x4e7d9b: 'ebx', 0x4e7dc5: 'ebx', 0x4e7de7: 'ebx', 0x4e7e08: 'ebx',
        }
        # These exact loads dominate the register-call paths; the first later
        # assignments are after the last call through each register. Earlier
        # pop instructions occur only in complete return paths.
        decoded = {i.address: i for i in instructions}
        self.assertEqual(decoded[0x4e7d65].op_str, 'ebp, dword ptr [0x55025c]')
        self.assertEqual(decoded[0x4e7d95].op_str, 'ebx, dword ptr [0x550284]')
        self.assertEqual(decoded[0x4e7d6c].op_str, 'esi, ecx')
        for register, start, end in (('ebp', 0x4e7d6b, 0x4e7e01), ('ebx', 0x4e7d9b, 0x4e7e12)):
            writes = [i for i in instructions if start <= i.address < end and i.mnemonic not in ('pop', 'call')
                      and i.operands and i.operands[0].type == capstone.x86.X86_OP_REG
                      and i.reg_name(i.operands[0].reg) == register and i.operands[0].access & capstone.CS_AC_WRITE]
            self.assertEqual(writes, [])
        indirect_count = 0
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                operand = instruction.operands[0]
                if operand.type == capstone.x86.X86_OP_IMM:
                    self.assertIn(operand.imm, boundaries)
                else:
                    self.assertEqual(instruction.address, 0x4e8190)
                    self.assertEqual(operand.mem.disp, 0x4e869c)
            if instruction.mnemonic == 'call' and instruction.operands[0].type == capstone.x86.X86_OP_IMM:
                target = instruction.operands[0].imm
                self.assertIn(target, {*HELPERS.values(), 0x534a88})
                self.assertEqual(instruction.bytes[0], 0xe8)
                targets.add(target)
                patches.append(struct.pack('<III', 0, instruction.address, target))
            elif instruction.mnemonic == 'call':
                indirect_count += 1
                operand = instruction.operands[0]
                if operand.type == capstone.x86.X86_OP_REG:
                    self.assertEqual(instruction.op_str, register_calls[instruction.address])
                else:
                    self.assertEqual(operand.type, capstone.x86.X86_OP_MEM)
                    self.assertEqual(operand.mem.index, 0)
                    if operand.mem.base:
                        self.assertEqual(instruction.reg_name(operand.mem.base), 'esi')
                        self.assertEqual(operand.mem.disp, 0x80)
                    else:
                        self.assertIn(operand.mem.disp, (0x55025c, 0x550284))
        self.assertEqual(indirect_count, 28)
        self.assertEqual(targets, {*HELPERS.values(), 0x534a88})
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        binding_path = ROOT / 'samples/race/RecorderTick.bindings.json'
        with tempfile.TemporaryDirectory(prefix='mcm2-recorder-tick-') as directory:
            output = Path(directory)

            def compile_source(source):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    *(['--extra', '/Od'] if source.startswith('tests/') else [])], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source('samples/race/RecorderTick.cpp')
            _, aggregate = compile_source('samples/race/RecorderNearMisses.cpp')
            symbol = candidate.find_symbol(SYMBOL)
            self.assertEqual(symbol.function_size, 2448)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number > 0 and s.storage_class == 2 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.section_number == 0 and s.storage_class == 2}, {*HELPERS, *IMPORTS, '__fltused', '__ftol'})
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            body, _, relocations = candidate.symbol_extent(symbol)

            def normalized(coff):
                entry = coff.find_symbol(SYMBOL)
                data, size, rs = coff.symbol_extent(entry)
                rows = []
                for r in rs:
                    target = coff.symbol_by_index[r.symbol_index]
                    name = target.name
                    if target.name.startswith(('$L', '$T')) and target.section_number == entry.section_number:
                        name = f'local:{target.value-entry.value}'
                    rows.append((r.virtual_address-entry.value, r.type, name))
                return data, size, rows

            self.assertEqual(normalized(candidate), normalized(aggregate))
            report = compare(image, candidate, SYMBOL, 0x4e7d60, 2412, binding_path)
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 276)
            constant = candidate.find_symbol('__real@4@4008fa00000000000000')
            section = candidate.section(constant.section_number)
            self.assertFalse(section.characteristics & 0x80000000)
            start = section.raw_ptr + constant.value
            self.assertEqual(candidate.data[start:start+4], image.bytes_at_va(0x550f44, 4))
            for r in relocations:
                name = candidate.symbol_by_index[r.symbol_index].name
                if name in IMPORTS:
                    offset = r.virtual_address-symbol.value
                    self.assertEqual(r.type, 6)
                    self.assertEqual(body[offset:offset+4], b'\0'*4)
                    patches.append(struct.pack('<III', 1, offset, IMPORTS[name]))
            for source in ('src/reconstructed/Recorder.cpp', 'src/reconstructed/VCR.cpp'):
                _, provider = compile_source(source)
                bindings = json.loads((ROOT / source).with_suffix('.bindings.json').read_text())
                for name, address in HELPERS.items():
                    if ('@UnknownVcr@@' in name) == source.endswith('/VCR.cpp'):
                        self.assertGreater(provider.find_symbol(name).function_size, 0)
                        self.assertEqual(int(bindings[name], 0), address)
            fixture_path, _ = compile_source('tests/fixtures/recorder_tick_compare.cpp')
            patch_path = output / 'calls.bin'
            patch_path.write_bytes(b''.join(patches))
            executable = output / 'recorder.exe'
            map_path = output / 'recorder.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/map:{map_path}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            entry_line = next(line for line in map_path.read_text().splitlines() if SYMBOL in line)
            address = entry_line.split()[2]
            result = subprocess.run([str(executable), str(reference), str(patch_path), address], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('RecorderTick: 147749 complete playback, callback, lock and floating-point scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
