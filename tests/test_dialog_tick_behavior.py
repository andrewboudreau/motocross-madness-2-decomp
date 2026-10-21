"""Private whole-object UIDialog Tick comparison, including its cold CFG tail."""
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

ROOT = Path(__file__).resolve().parents[1]
START, SIZE = 0x46ef00, 543
SYMBOL = '?Tick@UIDialog@@UAEHM@Z'
DEPENDENCIES = {0x534a88, 0x487fb0, 0x488310, 0x4727c0, 0x472810,
                0x470000, 0x46f120, 0x4693d0}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class DialogTickBehaviorTests(unittest.TestCase):
    def test_complete_cfg_object_state_and_mutating_callbacks(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        body = retail.bytes_at_va(START, SIZE)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        pending, reached, returns = [START], {}, set()
        while pending:
            address = pending.pop()
            if address in reached:
                continue
            self.assertTrue(START <= address < START + SIZE)
            instruction = next(decoder.disasm(body[address - START:], address, count=1))
            reached[address] = instruction
            if instruction.group(capstone.CS_GRP_RET):
                self.assertEqual(bytes(instruction.bytes), bytes.fromhex('c20400'))
                returns.add(address)
                continue
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                pending.append(instruction.operands[0].imm)
                if instruction.mnemonic == 'jmp':
                    continue
            pending.append(address + instruction.size)
        self.assertEqual(returns, {0x46f0d0})
        covered = [byte for address, instruction in reached.items()
                   for byte in range(address, address + instruction.size)]
        self.assertEqual(sorted(covered), list(range(START, START + SIZE)))
        self.assertEqual(retail.bytes_at_va(START + SIZE, 1), b'\x90')
        self.assertEqual(retail.bytes_at_va(0x46f120, 3), bytes.fromhex('83ec08'))
        self.assertIn(0x46f11d, reached)  # final cold branch returns to the device loop

        patches, seen, indirect = [], [], []
        for instruction in reached.values():
            if instruction.mnemonic != 'call':
                continue
            if instruction.operands[0].type == capstone.x86.X86_OP_IMM:
                self.assertEqual(instruction.bytes[0], 0xe8)
                target = instruction.operands[0].imm
                self.assertIn(target, DEPENDENCIES)
                seen.append(target)
                patches.append(struct.pack('<II', instruction.address, target))
            else:
                indirect.append(bytes(instruction.bytes))
        self.assertEqual(set(seen), DEPENDENCIES)
        self.assertEqual(len(seen), 9)
        self.assertEqual(seen.count(0x488310), 2)
        self.assertCountEqual(indirect, [bytes.fromhex('ff5274'), bytes.fromhex('ff5274'),
                                        bytes.fromhex('ff92f0000000')])

        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-dialog-tick-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/ui/UIDialogTick.cpp', 'tests/fixtures/dialog_tick_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 555)
                    bindings = json.loads((ROOT / 'samples/ui/UIDialogTick.bindings.json').read_text())
                    imports = {candidate.symbol_by_index[r.symbol_index].name for r in candidate.relocations
                               if r.section_number == entry.section_number
                               and r.type == 0x14 and entry.value <= r.virtual_address < entry.value + entry.function_size}
                    self.assertEqual({int(bindings[name], 0) for name in imports}, DEPENDENCIES)
            executable, mapping = output / 'dialog.exe', output / 'dialog.map'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/include:{SYMBOL}', f'/out:{executable}', f'/map:{mapping}',
                f'/libpath:{libraries}', *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            entry = re.search(re.escape(SYMBOL) + r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin-1'))
            self.assertIsNotNone(entry)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(reference), str(patch_file), entry.group(1)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('UIDialogTick: 73728 full-state and callback scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
