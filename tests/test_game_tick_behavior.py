"""Private execution of complete Tick/calibration bodies with scripted clocks."""
from __future__ import annotations

import hashlib
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
SYMBOLS = ('?Tick@Game@@UAEHXZ', '?SetRenderContext@HiResMeter@@UAEPAVGameObject@@PAX@Z')
RANGES = {0x4685d0: 683, 0x488440: 196}
DIRECT = {0x468880, 0x4bfa80, 0x43cf00, 0x4acde0, 0x488510,
          0x469c80, 0x4e8cc0, 0x4692f0, 0x536b60}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT')
                     and os.environ.get('MCM2_EXE'),
                     'requires native Windows, VC6_ROOT and private MCM2_EXE')
class GameTickBehaviorTests(unittest.TestCase):
    def test_full_tick_and_calibration_against_retail(self):
        import capstone

        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        seen, counters, indirect = set(), set(), set()
        patches = []
        for start, size in RANGES.items():
            instructions = list(decoder.disasm(image.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            for i in instructions:
                if i.mnemonic == 'rdtsc':
                    counters.add(i.address)
                if i.mnemonic not in ('call', 'jmp'):
                    continue
                operand = i.operands[0]
                if operand.type == capstone.x86.X86_OP_IMM:
                    target = operand.imm
                    if not start <= target < start + size:
                        self.assertIn(target, DIRECT)
                        seen.add(target)
                        patches.append(struct.pack("<II", i.address, target))
                else:
                    self.assertEqual(i.mnemonic, 'call')
                    indirect.add(i.op_str)
        self.assertEqual(seen, DIRECT)
        self.assertEqual(counters, {0x4686a0, 0x468712, 0x488477, 0x4884c6})
        self.assertEqual(indirect, {'dword ptr [edx + 0x28]', 'dword ptr [edx + 0x20]',
                                    'dword ptr [edx + 0x24]', 'dword ptr [0x5501b0]',
                                    'dword ptr [0x5501b4]'})
        compiler_root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-game-tick-') as directory:
            output = Path(directory)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            tick_bodies = []
            for name, source in [('candidate', 'samples/game/GameTick.cpp'),
                                 ('fixture', 'tests/fixtures/game_tick_compare.cpp')]:
                obj = output / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                compiled = CoffObject(obj)
                for symbol in SYMBOLS:
                    entry = compiled.find_symbol(symbol)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertGreater(compiled.symbol_extent(entry)[1], 190)
                # Inclusion exists only to expose the private timer cache to
                # scenario reset. Prove it did not change Tick's emitted code.
                tick = compiled.find_symbol(SYMBOLS[0])
                section = compiled.sections[tick.section_number - 1]
                start = section.raw_ptr + tick.value
                body = bytearray(compiled.data[start:start + tick.function_size])
                targets = []
                for relocation in compiled.relocations:
                    offset = relocation.virtual_address - tick.value
                    if relocation.section_number == tick.section_number and 0 <= offset < len(body):
                        targets.append((offset, relocation.type,
                                        compiled.symbol_by_index[relocation.symbol_index].name))
                        body[offset:offset + 4] = b'\0' * 4
                tick_bodies.append((bytes(body), targets))
                if name == 'candidate':
                    methods = {s.name for s in compiled.symbols if s.section_number > 0
                               and s.type == 0x20 and s.storage_class == 2}
                    self.assertEqual(methods, set(SYMBOLS))
            self.assertEqual(tick_bodies[0], tick_bodies[1],
                             'fixture inclusion changed production Tick code or call destinations')
            executable, map_file = output / 'tick.exe', output / 'tick.map'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/map:{map_file}', f'/libpath:{libraries}',
                *(f'/include:{symbol}' for symbol in SYMBOLS),
                str(output / 'fixture.obj'), 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            mapping = map_file.read_text(encoding='latin-1')
            addresses = []
            for symbol in SYMBOLS:
                match = re.search(re.escape(symbol) + r'\s+([0-9a-fA-F]{8})\s', mapping)
                self.assertIsNotNone(match, mapping)
                addresses.append(match.group(1))
            result = subprocess.run([str(executable), str(reference), str(patch_file), *addresses],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('39376 scenarios match full state and ordered calls', result.stdout)


if __name__ == '__main__':
    unittest.main()
