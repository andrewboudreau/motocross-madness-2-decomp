"""Verify DirectX and network identifier providers against private retail data."""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import sha256_file
from tools.match import compare


ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
CONSUMERS = (
    ('Net', '?ConnectUsingLobby@NetworkInterface@@QAEJXZ', 0x4ab960, 989,
     '_IID_IDirectPlayLobby3A', 0x556870, ((0x4ab9ee, 0),)),
    ('Net', '?CreateAddress@NetworkInterface@@QAEHU_GUID@@PAD1PAXPAPAXPAK@Z', 0x4abf10, 948,
     '_DPAID_INetPort', 0x556960, ((0x4ac044, 0), (0x4ac049, 4), (0x4ac04f, 8), (0x4ac059, 12))),
    ('PCAudio', '?CreateBuffer@Sound@@QAEHPAPAUUnknownSoundBuffer@@KKHHHHHKH@Z', 0x4bd540, 342,
     '_DS3DALG_NO_VIRTUALIZATION', 0x556cf0,
     ((0x4bd5ec, 0), (0x4bd5f2, 4), (0x4bd5fc, 8), (0x4bd606, 12))),
    ('PCAudio', '?ProbeEAXSupport@PCSoundInterface@@QAEXXZ', 0x4be9e0, 301,
     '_DSPROPSETID_EAX_ListenerProperties', 0x556d20, ((0x4bead6, 0),)),
    ('PCAudio', '?SetEAXEnvironment@PCSoundInterface@@QAEHK@Z', 0x4bed00, 61,
     '_DSPROPSETID_EAX_ListenerProperties', 0x556d20, ((0x4bed2b, 0),)),
    ('PCAudio', '?SetEAXListenerParameters@PCSoundInterface@@QAEHPAX@Z', 0x4bed40, 61,
     '_DSPROPSETID_EAX_ListenerProperties', 0x556d20, ((0x4bed6b, 0),)),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class DirectXGuidTests(unittest.TestCase):
    def test_sdk_compatible_symbols_sizes_readonly_storage_and_values(self):
        self.assert_guid_provider('DirectXGuids')

    def test_network_symbols_sizes_readonly_storage_and_values(self):
        self.assert_guid_provider('NetworkGuids')

    def test_lobby_and_audio_guid_consumers_keep_complete_retail_bodies(self):
        import capstone
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        provider_bindings = json.loads((ROOT / 'src/reconstructed/DirectXGuids.bindings.json').read_text())
        with tempfile.TemporaryDirectory(prefix='mcm2-guid-callers-') as tmp:
            objects = {}
            for stem in ('Net', 'PCAudio'):
                path = Path(tmp) / (stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / f'src/reconstructed/{stem}.cpp'), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[stem] = CoffObject(path)
            for stem, symbol, address, size, guid, guid_address, uses in CONSUMERS:
                with self.subTest(caller=symbol, guid=guid):
                    obj = objects[stem]
                    entry = obj.find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, obj, symbol, address, size,
                        ROOT / f'src/reconstructed/{stem}.bindings.json')['strict_exact'])
                    self.assertEqual(int(provider_bindings[guid], 0), guid_address)
                    _, _, relocations = obj.symbol_extent(entry)
                    references = [r for r in relocations if obj.symbol_by_index[r.symbol_index].name == guid]
                    self.assertEqual(len(references), len(uses))
                    self.assertTrue(all(r.type == 6 and obj.symbol_by_index[r.symbol_index].section_number == 0
                                        for r in references))
                    instructions = {i.address: i for i in decoder.disasm(retail.bytes_at_va(address, size), address)}
                    self.assertEqual(sum(i.size for i in instructions.values()), size)
                    for site, offset in uses:
                        instruction = instructions[site]
                        if len(uses) == 1:
                            self.assertEqual(instruction.mnemonic, 'push')
                            self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                            self.assertEqual(instruction.operands[0].imm, guid_address)
                        else:
                            self.assertEqual(instruction.mnemonic, 'mov')
                            operand = instruction.operands[1]
                            self.assertEqual(operand.type, capstone.x86.X86_OP_MEM)
                            self.assertEqual((operand.size, operand.mem.base, operand.mem.index, operand.mem.disp),
                                             (4, 0, 0, guid_address + offset))

    def assert_guid_provider(self, stem):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        bindings = json.loads((ROOT / f'src/reconstructed/{stem}.bindings.json').read_text())
        retail = PEImage(EXE)
        with tempfile.TemporaryDirectory(prefix='mcm2-guids-') as tmp:
            path = Path(tmp) / f'{stem}.obj'
            result = subprocess.run([
                sys.executable, str(ROOT / 'tools/compile.py'),
                str(ROOT / f'src/reconstructed/{stem}.cpp'), '-o', str(path),
                '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            obj = CoffObject(path)
            self.assertEqual(obj.machine, 0x14c)
            definitions = {s.name: s for s in obj.symbols
                           if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), set(bindings))
            self.assertFalse(any(s.raw_size and s.characteristics & 0x20 for s in obj.sections))
            self.assertFalse(any(s.storage_class == 2 and s.section_number == 0 for s in obj.symbols))
            self.assertFalse(any(not obj.section(r.section_number).name.startswith('.debug')
                                 for r in obj.relocations))
            for name, address in bindings.items():
                with self.subTest(symbol=name):
                    symbol = definitions[name]
                    section = obj.section(symbol.section_number)
                    following = [s.value for s in definitions.values()
                                 if s.section_number == symbol.section_number
                                 and s.value > symbol.value]
                    self.assertEqual(min(following, default=section.raw_size) - symbol.value, 16)
                    start = section.raw_ptr + symbol.value
                    va = int(address, 16)
                    self.assertEqual(obj.data[start:start + 16], retail.bytes_at_va(va, 16))
                    self.assertFalse(section.characteristics & 0x80000000)
                    original = retail.section_for_rva(va - retail.image_base)
                    self.assertFalse(original.characteristics & 0x80000000)


if __name__ == '__main__':
    unittest.main()
