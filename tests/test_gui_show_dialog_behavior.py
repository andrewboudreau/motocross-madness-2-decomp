"""Strict native production proof and private finite ShowDialog validation."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.pe import PEImage
from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root

ROOT = Path(__file__).resolve().parents[1]
START, SIZE = 0x485a70, 341
SYMBOL = '?ShowDialog@GUIManager@@QAEPAVUIDialog@@PAV2@HHH0HHH@Z'
DEPENDENCIES = {0x485df0, 0x4860f0, 0x470070, 0x4860a0, 0x4865e0, 0x485bd0}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class GuiShowDialogBehaviorTests(unittest.TestCase):
    def test_branch_arguments_state_and_order_against_private_retail(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         expected['mcm2_sha256'])
        pe = PEImage(reference)
        body = pe.bytes_at_va(START, SIZE)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # Establish the retail extent from reachable control flow, independently
        # of compiler output. The historical 345-byte range ended inside padding.
        pending, reached, returns = [START], {}, set()
        while pending:
            address = pending.pop()
            if address in reached:
                continue
            self.assertTrue(START <= address < START + SIZE)
            instruction = next(decoder.disasm(body[address - START:], address, count=1))
            reached[address] = instruction
            if instruction.group(capstone.CS_GRP_RET):
                self.assertEqual(bytes(instruction.bytes), bytes.fromhex('c22000'))
                returns.add(address)
                continue
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                pending.append(instruction.operands[0].imm)
                if instruction.mnemonic == 'jmp':
                    continue
            pending.append(address + instruction.size)
        self.assertEqual(returns, {0x485b96, 0x485bc2})
        self.assertEqual(len(reached), 132)
        covered = [byte for address, instruction in reached.items()
                   for byte in range(address, address + instruction.size)]
        self.assertEqual(sorted(covered), list(range(START, 0x485bc5)))
        self.assertEqual(pe.bytes_at_va(0x485bc5, 11), b'\x90' * 11)
        # AttachDialog is a separately called entry, beyond the complete NOP gap.
        self.assertEqual(pe.bytes_at_va(0x485bd0, 3), bytes.fromhex('53568b'))
        patches, seen, virtual = [], set(), 0
        instructions = list(decoder.disasm(body, START))
        self.assertEqual(sum(i.size for i in instructions), SIZE)
        for instruction in instructions:
            if instruction.mnemonic == 'call':
                if instruction.bytes[0] == 0xe8:
                    target = instruction.operands[0].imm
                    self.assertIn(target, DEPENDENCIES)
                    patches.append(struct.pack('<II', instruction.address - START, target))
                    seen.add(target)
                else:
                    # Keep the real virtual dispatch at slot 27 unmodified.
                    self.assertEqual(bytes(instruction.bytes), bytes.fromhex('ff526c'))
                    virtual += 1
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertTrue(START <= instruction.operands[0].imm < START + SIZE)
            for operand in instruction.operands:
                if operand.type == capstone.x86.X86_OP_MEM:
                    # This function has no retail absolute data dependencies.
                    self.assertTrue(operand.mem.base or operand.mem.index)
        self.assertEqual(seen, DEPENDENCIES)
        self.assertEqual((len(patches), virtual), (8, 1))
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-show-dialog-') as directory:
            output = Path(directory)
            code = output / 'reference.bin'
            code.write_bytes(body)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            # Compile the actual production TU independently. The fixture needs
            # only its exact source function, since production also defines the
            # dependencies that this finite test intentionally scripts.
            canonical = ROOT / 'src/reconstructed/GUIManager.cpp'
            source_text = canonical.read_text()
            begin = source_text.index('// 0x00485a70\n')
            end = source_text.index('// 0x00485bd0\n', begin)
            function = source_text[begin:end]
            self.assertEqual(function.count('GUIManager::ShowDialog('), 1)
            extracted = output / 'show_dialog.cpp'
            extracted.write_text(''.join(
                f'#include "{(ROOT / "src/reconstructed" / header).as_posix()}"\n'
                for header in ('GUIManagerCore.h', 'RenderTarget.h')) + function)
            values = json.loads((ROOT / 'src/reconstructed/GUIManager.bindings.json').read_text())
            bindings = {name: int(value, 0) if isinstance(value, str) else value
                        for name, value in values.items()}
            objects = []
            for source in (canonical, extracted,
                           ROOT / 'tests/fixtures/gui_show_dialog_compare.cpp'):
                obj = output / (source.stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(source), '-o', str(obj), '--vc6-root',
                    str(compiler_root), '--profile', 'vc6_o2_mt',
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if source in (canonical, extracted):
                    native = CoffObject(obj)
                    symbol = native.find_symbol(SYMBOL)
                    self.assertEqual(symbol.extent_source, 'coff_function_aux')
                    emitted, size, relocations = native.symbol_extent(symbol)
                    self.assertEqual(size, SIZE)
                    self.assertEqual(len(relocations), 8)
                    proof = match_object(native, SYMBOL, START, body, bindings)
                    self.assertTrue(proof['strict_exact'], proof)
                    self.assertEqual(proof['ignored_bytes'], 0)
                if source != canonical:
                    objects.append(str(obj))
            executable = output / 'show_dialog_probe.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *objects, 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(code), str(patch_file)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('ShowDialog: 34560 scenarios match return, complete state and ordered calls',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
