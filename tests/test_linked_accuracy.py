import unittest

from tools.frame_layout import disassemble
from tools.linked_accuracy import forward_target, parse_functions, similarity


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

    def test_forwarder_is_followed_but_real_body_is_not(self):
        # mov eax,[esp+8]; push eax; mov ecx,[esp+8]; push ecx; call 0x2000; add esp,8; ret
        forwarder = disassemble(bytes.fromhex('8b44240850' '8b4c240851' 'e8f10f0000' '83c408' 'c3'), 0x1000)
        self.assertEqual(forward_target(forwarder), 0x2000)
        jmp = disassemble(bytes.fromhex('e9fb0f0000'), 0x1000)       # jmp 0x2000
        self.assertEqual(forward_target(jmp), 0x2000)
        # a compare and branch make it an implementation, not a forwarder
        real = disassemble(bytes.fromhex('85c07405' 'e8f70f0000' 'c3'), 0x1000)
        self.assertIsNone(forward_target(real))

    def test_similarity_masks_frame_registers_and_ebp_frames(self):
        esp_frame = disassemble(bytes.fromhex('8b442404' 'c3'), 0)          # mov eax,[esp+4]; ret
        ebp_frame = disassemble(bytes.fromhex('8b4d08' 'c3'), 0)            # mov ecx,[ebp+8]; ret
        self.assertEqual(similarity(esp_frame, ebp_frame, strict=False), 1.0)
        self.assertLess(similarity(esp_frame, ebp_frame, strict=True), 1.0)


if __name__ == '__main__':
    unittest.main()
