"""Strict matching resolves labels inside the candidate function's own extent."""
from pathlib import Path
import struct
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import RelocationError, match_object

TARGET_VA = 0x401000


def label_fixture(label_offset=5, *, label_section=1, function_size=8):
    """func: mov eax, offset $L1 ; ret ; nop nop (DIR32 relocation to $L1)."""
    text = bytes.fromhex('b800000000c3') + b'\x90' * 2 + b'\x90' * 8
    symbols = bytearray()
    symbols += struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, 1)
    symbols += struct.pack('<IIIIH', 0, function_size, 0, 0, 0)
    symbols += struct.pack('<8sIhHBB', b'$L1', label_offset, label_section, 0, 6, 0)
    label_index = 2
    relocs = struct.pack('<IIH', 1, label_index, 0x0006)
    text_ptr = 20 + 40
    reloc_ptr = text_ptr + len(text)
    sym_ptr = reloc_ptr + len(relocs)
    header = struct.pack('<HHIIIHH', 0x14c, 1, 0, sym_ptr, len(symbols) // 18, 0, 0)
    section = struct.pack('<8sIIIIIIHHI', b'.text', 0, 0, len(text), text_ptr, reloc_ptr, 0, 1, 0, 0x60501020)
    return header + section + text + relocs + symbols + struct.pack('<I', 4)


class InternalLabelTests(unittest.TestCase):
    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'labels.obj'
        path.write_bytes(label_fixture(**kwargs))
        return CoffObject(path)

    def retail(self, address):
        return b'\xb8' + struct.pack('<I', address) + b'\xc3\x90\x90'

    def test_label_resolves_to_function_va_plus_offset(self):
        result = match_object(self.load(), 'func', TARGET_VA, self.retail(TARGET_VA + 5), {})
        self.assertTrue(result['strict_exact'])
        self.assertTrue(result['relocations_applied'][0]['internal_label'])

    def test_wrong_retail_address_fails(self):
        result = match_object(self.load(), 'func', TARGET_VA, self.retail(TARGET_VA + 6), {})
        self.assertFalse(result['strict_exact'])

    def test_label_outside_extent_still_needs_binding(self):
        obj = self.load(label_offset=12)
        with self.assertRaises(RelocationError):
            match_object(obj, 'func', TARGET_VA, self.retail(TARGET_VA + 12), {})

    def test_contradicting_explicit_binding_is_rejected(self):
        with self.assertRaises(RelocationError):
            match_object(self.load(), 'func', TARGET_VA, self.retail(TARGET_VA + 5), {'$L1': TARGET_VA + 9})

    def test_consistent_explicit_binding_is_accepted(self):
        result = match_object(self.load(), 'func', TARGET_VA, self.retail(TARGET_VA + 5), {'$L1': TARGET_VA + 5})
        self.assertTrue(result['strict_exact'])


if __name__ == '__main__':
    unittest.main()
