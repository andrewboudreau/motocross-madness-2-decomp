"""Strict matching of __try/__except frame prologues (scope-table key)."""
from pathlib import Path
import struct
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import RelocationError, match_object

TARGET_VA = 0x460580
SCOPE_VA = 0x5526c8
HANDLER3_VA = 0x536444


def seh_fixture(prologue=b'\x55\x8b\xec\x6a\xff\x68', scope_section=b'.rdata', scope_class=3):
    """func: push ebp; mov ebp, esp; push -1; push $T1; push __except_handler3; ret."""
    text = prologue + b'\0' * 4 + b'\x68' + b'\0' * 4 + b'\xc3'
    scope = struct.pack('<iII', -1, 0, 0)
    sections = [(b'.text', text, 0x60501020), (scope_section, scope, 0x40300040)]
    symbols = bytearray()
    symbols += struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, 1)
    symbols += struct.pack('<IIIIH', 0, len(text), 0, 0, 0)
    symbols += struct.pack('<8sIhHBB', b'$T42110', 0, 2, 0, scope_class, 0)
    strings = bytearray(b'\0\0\0\0')
    offset = len(strings)
    strings.extend(b'__except_handler3\0')
    symbols += struct.pack('<8sIhHBB', struct.pack('<II', 0, offset), 0, 0, 0, 2, 0)
    at = len(prologue)
    relocs = struct.pack('<IIH', at, 2, 6) + struct.pack('<IIH', at + 5, 3, 6)
    header_size = 20 + 40 * len(sections)
    ptr = header_size
    headers, blobs = bytearray(), bytearray()
    for index, (name, data, flags) in enumerate(sections):
        reloc_ptr, nrelocs = 0, 0
        blob = data
        if index == 0:
            reloc_ptr, nrelocs = ptr + len(data), 2
            blob = data + relocs
        headers += struct.pack('<8sIIIIIIHHI', name, 0, 0, len(data), ptr, reloc_ptr, 0, nrelocs, 0, flags)
        blobs += blob
        ptr += len(blob)
    sym_ptr = ptr
    strings[0:4] = struct.pack('<I', len(strings))
    header = struct.pack('<HHIIIHH', 0x14c, len(sections), 0, sym_ptr, len(symbols) // 18, 0, 0)
    return header + headers + blobs + symbols + strings


def retail(prologue=b'\x55\x8b\xec\x6a\xff\x68', scope=SCOPE_VA):
    return prologue + struct.pack('<I', scope) + b'\x68' + struct.pack('<I', HANDLER3_VA) + b'\xc3'


class SehScopeTableTests(unittest.TestCase):
    bindings = {'func$scopetable': SCOPE_VA, '__except_handler3': HANDLER3_VA}

    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'seh.obj'
        path.write_bytes(seh_fixture(**kwargs))
        return CoffObject(path)

    def test_scope_table_binds_under_stable_key(self):
        result = match_object(self.load(), 'func', TARGET_VA, retail(), self.bindings)
        self.assertTrue(result['strict_exact'])
        self.assertEqual([row['symbol'] for row in result['relocations_applied']],
                         ['func$scopetable', '__except_handler3'])

    def test_wrong_scope_table_address_fails(self):
        result = match_object(self.load(), 'func', TARGET_VA, retail(scope=SCOPE_VA + 0x18), self.bindings)
        self.assertFalse(result['strict_exact'])

    def test_temporary_needs_seh_prologue(self):
        prologue = b'\x90\x90\x90\x6a\xff\x68'
        with self.assertRaises(RelocationError):
            match_object(self.load(prologue=prologue), 'func', TARGET_VA, retail(prologue), self.bindings)

    def test_temporary_outside_rdata_keeps_its_name(self):
        with self.assertRaises(RelocationError):
            match_object(self.load(scope_section=b'.data'), 'func', TARGET_VA, retail(), self.bindings)


if __name__ == '__main__':
    unittest.main()
