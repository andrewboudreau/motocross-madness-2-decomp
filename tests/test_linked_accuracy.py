import json
import tempfile
import unittest
from pathlib import Path

from tools.frame_layout import disassemble
from tools.linked_accuracy import drop_tables, forward_target, parse_functions, similarity, target_symbols


MAP = """\
  Address         Publics by Value              Rva+Base     Lib:Object

 0001:00000000       ??0AgeManager@@QAE@XZ      00401000 f   AgeManager.obj
 0001:00000020       ??_EFoo@@UAEPAXI@Z         00401020 f i Foo.obj
 0002:00000000       ?g_data@@3HA               00500000     Data.obj
 0001:00000010       _memmove                   00401010 f   LIBCMT:memmove.obj
"""


class LinkedAccuracyTest(unittest.TestCase):
    def test_parse_functions_keeps_code_symbols_in_address_order(self):
        self.assertEqual(parse_functions(MAP), [
            (0x401000, '??0AgeManager@@QAE@XZ', 'AgeManager.obj'),
            (0x401010, '_memmove', 'LIBCMT:memmove.obj'),
            (0x401020, '??_EFoo@@UAEPAXI@Z', 'Foo.obj'),
        ])

    def test_target_symbols_bind_only_unique_unbound_matches(self):
        funcs = [(0x401000, '?Insert@QuadTree@@QAEXPAX@Z', 'A.obj'),
                 (0x401010, '?Draw@Foo@@QAEXXZ', 'B.obj'),
                 (0x401020, '?Draw@Bar@@QAEXXZ', 'B.obj'),
                 (0x401030, '?Bound@Baz@@QAEXXZ', 'C.obj')]
        bindings = {'?Bound@Baz@@QAEXXZ': {0x460000}}
        with tempfile.TemporaryDirectory() as tmp:
            d = Path(tmp) / 'samples' / 'x'
            d.mkdir(parents=True)
            (d / 'targets.json').write_text(json.dumps([
                {'target_va': '0x4a0000', 'candidate_symbol_contains': 'Insert@QuadTree'},
                {'target_va': '0x4a0100', 'candidate_symbol_contains': '?Draw@'},
                {'target_va': '0x4a0200', 'candidate_symbol_contains': 'Bound@Baz'},
                {'target_va': '0x4a0300'},
            ]), encoding='utf-8')
            got = target_symbols(funcs, bindings, Path(tmp))
        self.assertEqual(got, {'?Insert@QuadTree@@QAEXPAX@Z': 0x4a0000})

    def test_forwarder_is_followed_but_real_body_is_not(self):
        # mov eax,[esp+8]; push eax; mov ecx,[esp+8]; push ecx; call 0x2000; add esp,8; ret
        forwarder = disassemble(bytes.fromhex('8b44240850' '8b4c240851' 'e8f10f0000' '83c408' 'c3'), 0x1000)
        self.assertEqual(forward_target(forwarder), 0x2000)
        jmp = disassemble(bytes.fromhex('e9fb0f0000'), 0x1000)       # jmp 0x2000
        self.assertEqual(forward_target(jmp), 0x2000)
        # a compare and branch make it an implementation, not a forwarder
        real = disassemble(bytes.fromhex('85c07405' 'e8f70f0000' 'c3'), 0x1000)
        self.assertIsNone(forward_target(real))
        # a constructor: base call plus a vptr store through esi is not a forwarder
        ctor = disassemble(bytes.fromhex('56' '8bf1' 'e8f80f0000' 'c70600305000' '8bc6' '5e' 'c3'), 0x1000)
        self.assertIsNone(forward_target(ctor))
        # a by-value argument copied through `mov ecx, esp` still forwards
        byval = disassemble(bytes.fromhex('83ec0c' '8bcc' '8b442410' '8901' 'e8f00f0000' 'c3'), 0x1000)
        self.assertEqual(forward_target(byval), 0x2000)

    def test_switch_tables_after_the_code_are_not_decoded(self):
        # xor ecx,ecx; mov cl,[eax+0x1014]; jmp [ecx*4+0x1010]; ret; then the jump table at 0x1010
        # and a byte index table, whose absolute entries differ in a relinked body.
        code = bytes.fromhex('33c9' '8a8814100000' 'ff248d10100000' 'c3') + \
            bytes.fromhex('0f100000') + bytes.fromhex('00000000')
        insns = drop_tables(disassemble(code, 0x1000))
        self.assertEqual([x.mnemonic for x in insns], ['xor', 'mov', 'jmp', 'ret'])
        # a global array displacement outside the body is not a table
        load = disassemble(bytes.fromhex('8b048d00305000' 'c3'), 0x1000)
        self.assertEqual(drop_tables(load), load)

    def test_similarity_masks_frame_registers_and_ebp_frames(self):
        esp_frame = disassemble(bytes.fromhex('8b442404' 'c3'), 0)          # mov eax,[esp+4]; ret
        ebp_frame = disassemble(bytes.fromhex('8b4d08' 'c3'), 0)            # mov ecx,[ebp+8]; ret
        self.assertEqual(similarity(esp_frame, ebp_frame, strict=False), 1.0)
        self.assertLess(similarity(esp_frame, ebp_frame, strict=True), 1.0)


if __name__ == '__main__':
    unittest.main()
