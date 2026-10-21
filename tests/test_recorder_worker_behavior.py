"""Complete recorder worker CFG and standalone native boundary comparison."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?UnknownRecorderThread@@YGIPAX@Z'
HELPERS = {
    '?HasSlotToWrite@UnknownVcr@@QAEHXZ': 0x524350,
    '?TakeSlotToWrite@UnknownVcr@@QAEXPAPAXPAH1@Z': 0x524300,
    '?ReleaseSlot@UnknownVcr@@QAEXH@Z': 0x524540,
    '?HasRoomToRead@UnknownVcr@@QAEHXZ': 0x524560,
    '?ClaimReadSlot@UnknownVcr@@QAEHIPAHPAPAX@Z': 0x524390,
    '?CommitReadSlot@UnknownVcr@@QAEXHH@Z': 0x524440,
    '?MarkForwardEnd@UnknownVcr@@QAEXXZ': 0x524940,
    '?MarkReverseEnd@UnknownVcr@@QAEXXZ': 0x524970,
    '?StepPositionBack@UnknownVcr@@QAEHPAH@Z': 0x5246c0,
    '?PeekPosition@UnknownVcr@@QAEHPAH@Z': 0x5247c0,
    '?SetFirstPosition@UnknownVcr@@QAEXH@Z': 0x5248f0,
    '?GetFirstPosition@UnknownVcr@@QAEHXZ': 0x524900,
    '?ResetSlots@UnknownVcr@@QAEXXZ': 0x524910,
    '?Open@UnknownVcrFile@@QAEHPAD0@Z': 0x524dd0,
    '?Close@UnknownVcrFile@@QAEXH@Z': 0x525000,
    '?Read@UnknownVcrFile@@QAEHPAXHHH@Z': 0x525070,
    '?Write@UnknownVcrFile@@QAEHPBXHHH@Z': 0x5251f0,
    '?Seek@UnknownVcrFile@@QAEHHHH@Z': 0x5253d0,
    '?Tell@UnknownVcrFile@@QAEHH@Z': 0x525440,
    '?IsEndOfFile@UnknownVcrFile@@QAEHH@Z': 0x5254a0,
}
IMPORTS = {'__imp__WaitForMultipleObjects@16': 0, '__imp__EnterCriticalSection@4': 1,
           '__imp__LeaveCriticalSection@4': 2, '__imp__SetEvent@4': 3, '__imp__ResetEvent@4': 4}
IATS = {0x5500d8, 0x55025c, 0x550284, 0x5500e8, 0x5500e0}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class RecorderWorkerBehaviorTests(unittest.TestCase):
    def test_complete_worker_events_data_and_termination(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4e6f80, 2101), 0x4e6f80))
        self.assertEqual(sum(i.size for i in instructions), 2101)
        self.assertEqual((instructions[-1].address, bytes(instructions[-1].bytes)), (0x4e77b2, b'\xc2\x04\0'))
        self.assertEqual(image.bytes_at_va(0x4e77b5, 3), b'\x8d\x49\0')
        self.assertEqual(image.bytes_at_va(0x4e77e4, 12), b'\x90'*12)
        table = struct.unpack('<11I', image.bytes_at_va(0x4e77b8, 44))
        boundaries = {i.address for i in instructions}
        self.assertTrue(set(table) <= boundaries)
        decoded = {i.address: i for i in instructions}
        register_calls = {0x4e7588, 0x4e758e, 0x4e75f5, 0x4e75fb, 0x4e765a, 0x4e7660}
        # Each pair of register calls follows this exact ResetEvent IAT load,
        # with no intervening write to EDI; all other indirect calls use IATs.
        for start, end in ((0x4e7581, 0x4e7590), (0x4e75ee, 0x4e75fd), (0x4e7653, 0x4e7662)):
            self.assertEqual(decoded[start].op_str, 'edi, dword ptr [0x5500e0]')
            for i in instructions:
                if start < i.address < end and i.mnemonic != 'call':
                    self.assertNotIn(capstone.x86.X86_REG_EDI, i.regs_access()[1])
        patches = []
        targets = set()
        indirect_count = 0
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                operand = i.operands[0]
                if operand.type == capstone.x86.X86_OP_IMM:
                    self.assertIn(operand.imm, boundaries)
                else:
                    self.assertEqual((i.address, operand.mem.disp), (0x4e704e, 0x4e77b8))
            if i.mnemonic != 'call':
                continue
            operand = i.operands[0]
            if operand.type == capstone.x86.X86_OP_IMM:
                self.assertIn(operand.imm, {*HELPERS.values(), 0x537c9f})
                self.assertEqual(i.bytes[0], 0xe8)
                targets.add(operand.imm)
                patches.append(struct.pack('<III', 0, i.address, operand.imm))
            else:
                indirect_count += 1
                if operand.type == capstone.x86.X86_OP_REG:
                    self.assertIn(i.address, register_calls)
                    self.assertEqual(i.op_str, 'edi')
                else:
                    self.assertEqual(operand.type, capstone.x86.X86_OP_MEM)
                    self.assertEqual((operand.mem.base, operand.mem.index), (0, 0))
                    self.assertIn(operand.mem.disp, IATS)
        self.assertEqual(len(patches), 48)
        self.assertEqual(indirect_count, 57)
        self.assertEqual(targets, {*HELPERS.values(), 0x537c9f})
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        binding_path = ROOT / 'samples/race/RecorderWorker.bindings.json'
        bindings = json.loads(binding_path.read_text())
        with tempfile.TemporaryDirectory(prefix='mcm2-recorder-worker-') as directory:
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

            candidate_path, candidate = compile_source('samples/race/RecorderWorker.cpp')
            _, aggregate = compile_source('samples/race/RecorderNearMisses.cpp')
            symbol = candidate.find_symbol(SYMBOL)
            self.assertEqual(symbol.function_size, 2148)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number > 0 and s.storage_class == 2 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.section_number == 0 and s.storage_class == 2}, {*HELPERS, *IMPORTS, '__endthreadex'})
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

            # Including the unchanged body preserves the old aggregate context;
            # the standalone TU has a different, independently checked code shape.
            aggregate_body, aggregate_size, aggregate_relocs = normalized(aggregate)
            self.assertEqual(aggregate_size, 2124)
            self.assertEqual(hashlib.sha256(aggregate_body).hexdigest(),
                             '30028ed354faec404948fa17993859d2ff64bdee9592ea9fba9567ee9db09612')
            self.assertEqual(hashlib.sha256(json.dumps(aggregate_relocs).encode()).hexdigest(),
                             '29e5858064ab107f6b366fccd8d7ae899d28dcf5fdf185d7d3cf06913dd38535')
            report = compare(image, candidate, SYMBOL, 0x4e6f80, 2148, binding_path)
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 2100)
            for s in candidate.symbols:
                if not s.name.startswith('??_C@'):
                    continue
                section = candidate.section(s.section_number)
                self.assertTrue(section.characteristics & 0x80000000)
                address = int(bindings[s.name], 0)
                retail_section = next(x for x in image.sections if
                    image.image_base+x.virtual_address <= address < image.image_base+x.virtual_address+x.virtual_size)
                self.assertTrue(retail_section.characteristics & 0x80000000)
                start = section.raw_ptr + s.value
                data = candidate.data[start:start+section.raw_size-s.value].split(b'\0', 1)[0] + b'\0'
                self.assertEqual(data, image.bytes_at_va(int(bindings[s.name], 0), len(data)))
            for r in relocations:
                name = candidate.symbol_by_index[r.symbol_index].name
                offset = r.virtual_address-symbol.value
                if name in IMPORTS:
                    self.assertEqual(r.type, 6)
                    self.assertEqual(body[offset:offset+4], b'\0'*4)
                    patches.append(struct.pack('<III', 1, offset, IMPORTS[name]))
                elif name == '__endthreadex':
                    self.assertEqual((r.type, body[offset-1]), (20, 0xe8))
                    patches.append(struct.pack('<III', 2, offset, 0))
            self.assertEqual(len(patches), 103)
            for source in ('src/reconstructed/VCR.cpp', 'src/reconstructed/VCRfile.cpp'):
                _, provider = compile_source(source)
                provider_bindings = json.loads((ROOT / source).with_suffix('.bindings.json').read_text())
                for name, address in HELPERS.items():
                    if ('@UnknownVcr@@' in name) == source.endswith('/VCR.cpp'):
                        self.assertGreater(provider.find_symbol(name).function_size, 0)
                        self.assertEqual(int(provider_bindings[name], 0), address)
                        proof = compare(image, provider, name, address, provider.find_symbol(name).function_size,
                                        (ROOT / source).with_suffix('.bindings.json'))
                        self.assertTrue(proof['strict_exact'], name)
            _, caller = compile_source('src/reconstructed/Recorder.cpp')
            start = '?Start@VCRInterface@@QAEHHHHHHIPAUUnknownRecorderOwner@@@Z'
            self.assertTrue(compare(image, caller, start, 0x4e7a90, 716,
                                    ROOT / 'src/reconstructed/Recorder.bindings.json')['strict_exact'])
            self.assertEqual(image.bytes_at_va(0x4e7c55, 5), b'\x68' + struct.pack('<I', 0x4e6f80))
            self.assertEqual(image.bytes_at_va(0x4e7c5e, 5), b'\xe8' + struct.pack('<i', 0x537b98-0x4e7c63))
            fixture_path, _ = compile_source('tests/fixtures/recorder_worker_compare.cpp')
            patch_path = output / 'calls.bin'
            patch_path.write_bytes(b''.join(patches))
            executable = output / 'recorder.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_path)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('RecorderWorker: 24595 complete event, data, lock, failure and termination scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
