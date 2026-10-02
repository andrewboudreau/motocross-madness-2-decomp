"""Strict matching of /GX frame prologues and __FILE__ literals."""
from pathlib import Path
import struct
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import RelocationError, match_object

TARGET_VA = 0x401000
HANDLER_VA = 0x54ad68
FILE_VA = 0x56b6c8


def frame_fixture(literal=b'D:\\aardvark\\VC\\krusty2\\gameobj.cpp\0', prologue=b'\x6a\xff\x68',
                  scheduled=False):
    """func: push -1; push $L1; mov eax, fs:[__except_list]; push ??_C@str; ret.

    With `scheduled`, the fs:[0] load comes first (handler push at +9)."""
    text = prologue + b'\0' * 4 + b'\x64\xa1' + b'\0' * 4 + b'\x68' + b'\0' * 4 + b'\xc3'
    if scheduled:
        text = b'\x64\xa1' + b'\0' * 4 + b'\x6a\xff\x68' + b'\0' * 4 + b'\x68' + b'\0' * 4 + b'\xc3'
    textx = b'\xb8\0\0\0\0\xc3'
    sections = [(b'.text', text, 0x60501020), (b'.text$x', textx, 0x60501020),
                (b'.data', literal, 0xc0300040)]
    symbols = bytearray()
    symbols += struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, 1)
    symbols += struct.pack('<IIIIH', 0, len(text), 0, 0, 0)
    symbols += struct.pack('<8sIhHBB', b'$L1', 0, 2, 0, 6, 0)
    strings = bytearray(b'\0\0\0\0')
    def long_name(name):
        offset = len(strings)
        strings.extend(name + b'\0')
        return struct.pack('<II', 0, offset)
    symbols += struct.pack('<8sIhHBB', long_name(b'__except_list'), 0, 0, 0, 2, 0)
    symbols += struct.pack('<8sIhHBB', long_name(b'??_C@_0CD@ABC@D?3?2aardvark@'), 0, 3, 0, 2, 0)
    relocs = struct.pack('<IIH', 3, 2, 6) + struct.pack('<IIH', 9, 3, 6) + struct.pack('<IIH', 14, 4, 6)
    if scheduled:
        relocs = struct.pack('<IIH', 2, 3, 6) + struct.pack('<IIH', 9, 2, 6) + struct.pack('<IIH', 14, 4, 6)
    header_size = 20 + 40 * len(sections)
    ptr = header_size
    headers, blobs = bytearray(), bytearray()
    for index, (name, data, flags) in enumerate(sections):
        reloc_ptr, nrelocs = 0, 0
        blob = data
        if index == 0:
            reloc_ptr, nrelocs = ptr + len(data), 3
            blob = data + relocs
        headers += struct.pack('<8sIIIIIIHHI', name, 0, 0, len(data), ptr, reloc_ptr, 0, nrelocs, 0, flags)
        blobs += blob
        ptr += len(blob)
    sym_ptr = ptr
    strings[0:4] = struct.pack('<I', len(strings))
    header = struct.pack('<HHIIIHH', 0x14c, len(sections), 0, sym_ptr, len(symbols) // 18, 0, 0)
    return header + headers + blobs + symbols + strings


def retail(handler=HANDLER_VA, file_va=FILE_VA):
    return (b'\x6a\xff\x68' + struct.pack('<I', handler) + b'\x64\xa1' + b'\0' * 4
            + b'\x68' + struct.pack('<I', file_va) + b'\xc3')


class EhFrameRelocationTests(unittest.TestCase):
    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'frame.obj'
        path.write_bytes(frame_fixture(**kwargs))
        return CoffObject(path)

    bindings = {'func$ehhandler': HANDLER_VA, '__FILE__': FILE_VA}

    def test_handler_except_list_and_file_resolve(self):
        result = match_object(self.load(), 'func', TARGET_VA, retail(), self.bindings)
        self.assertTrue(result['strict_exact'])
        self.assertEqual([row['symbol'] for row in result['relocations_applied']],
                         ['func$ehhandler', '__except_list', '__FILE__'])

    def test_scheduled_prologue_handler_resolves(self):
        retail_bytes = (b'\x64\xa1' + b'\0' * 4 + b'\x6a\xff\x68' + struct.pack('<I', HANDLER_VA)
                        + b'\x68' + struct.pack('<I', FILE_VA) + b'\xc3')
        result = match_object(self.load(scheduled=True), 'func', TARGET_VA, retail_bytes, self.bindings)
        self.assertTrue(result['strict_exact'])
        self.assertEqual([row['symbol'] for row in result['relocations_applied']],
                         ['__except_list', 'func$ehhandler', '__FILE__'])

    def test_wrong_handler_address_fails(self):
        result = match_object(self.load(), 'func', TARGET_VA, retail(handler=HANDLER_VA + 0x20), self.bindings)
        self.assertFalse(result['strict_exact'])

    def test_handler_label_needs_frame_prologue(self):
        with self.assertRaises(RelocationError):
            match_object(self.load(prologue=b'\x90\x90\x68'), 'func', TARGET_VA, retail(), self.bindings)

    def test_file_literal_prefers_basename_key(self):
        bindings = {'func$ehhandler': HANDLER_VA, '__FILE__': FILE_VA + 0x40,
                    '__FILE__:gameobj.cpp': FILE_VA}
        result = match_object(self.load(), 'func', TARGET_VA, retail(), bindings)
        self.assertTrue(result['strict_exact'])
        self.assertEqual(result['relocations_applied'][2]['symbol'], '__FILE__:gameobj.cpp')

    def test_non_path_literal_keeps_its_own_name(self):
        with self.assertRaises(RelocationError):
            match_object(self.load(literal=b'class \0'), 'func', TARGET_VA, retail(), self.bindings)


if __name__ == '__main__':
    unittest.main()
