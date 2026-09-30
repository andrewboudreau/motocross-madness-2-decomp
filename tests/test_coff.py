"""Function extents must come from object evidence, not target size or padding."""
from pathlib import Path
import struct
import tempfile
import unittest

from mcm2tool.coff import CoffError, CoffObject
from mcm2tool.resolved_match import match_object


def fixture(size=4, *, symbol_type=0x20, storage=2, aux=True, next_symbol=None):
    # A function ending in RET followed by alignment NOPs, in a 16-byte section.
    text = bytes.fromhex('8b4104c3') + b'\x90' * 12
    symbols = bytearray(struct.pack('<8sIhHBB', b'func', 0, 1, symbol_type, storage, int(aux)))
    if aux:
        symbols += struct.pack('<IIIIH', 0, size, 0, 0, 0)
    if next_symbol is not None:
        symbols += struct.pack('<8sIhHBB', b'next', next_symbol, 1, 0x20, 2, 0)
    header = struct.pack('<HHIIIHH', 0x14c, 1, 0, 76, len(symbols)//18, 0, 0)
    section = struct.pack('<8sIIIIIIHHI', b'.text', 0, 0, 16, 60, 0, 0, 0, 0, 0x60501020)
    return header + section + text + symbols + struct.pack('<I', 4)


class FunctionExtentTests(unittest.TestCase):
    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'fixture.obj'
        path.write_bytes(fixture(**kwargs))
        return CoffObject(path)

    def test_function_aux_size_excludes_alignment(self):
        obj = self.load()
        raw, size, _ = obj.symbol_extent(obj.find_symbol('func'))
        self.assertEqual(size, 4)
        self.assertEqual(raw, bytes.fromhex('8b4104c3'))

    def test_without_metadata_preserves_padding(self):
        obj = self.load(aux=False)
        self.assertEqual(obj.symbol_extent(obj.find_symbol('func'))[1], 16)

    def test_strict_match_does_not_guess_boundary_from_target(self):
        for kwargs in ({'aux': False}, {'size': 16}):
            obj = self.load(**kwargs)
            result = match_object(obj, 'func', 0x401000, bytes.fromhex('8b4104c3'), {})
            self.assertFalse(result['strict_exact'])
            self.assertEqual(result['candidate_size'], 16)

    def test_does_not_strip_nop_in_declared_extent(self):
        obj = self.load(size=5)
        self.assertEqual(obj.symbol_extent(obj.find_symbol('func'))[0][-1], 0x90)

    def test_data_aux_record_is_not_function_size(self):
        obj = self.load(symbol_type=0)
        self.assertIsNone(obj.find_symbol('func').function_size)

    def test_static_aux_record_is_not_function_size(self):
        obj = self.load(storage=3)
        self.assertIsNone(obj.find_symbol('func').function_size)

    def test_rejects_size_outside_section(self):
        obj = self.load(size=17)
        with self.assertRaises(CoffError):
            obj.symbol_extent(obj.find_symbol('func'))

    def test_rejects_size_overlapping_next_symbol(self):
        obj = self.load(size=9, next_symbol=8)
        with self.assertRaises(CoffError):
            obj.symbol_extent(obj.find_symbol('func'))

    def test_rejects_zero_size(self):
        obj = self.load(size=0)
        with self.assertRaises(CoffError):
            obj.symbol_extent(obj.find_symbol('func'))

    def test_fallback_stops_at_next_symbol(self):
        obj = self.load(aux=False, next_symbol=8)
        self.assertEqual(obj.symbol_extent(obj.find_symbol('func'))[1], 8)



def codeview_fixture(cv_len=4, *, target='defined', rectyp=0x100b, second_len=None, duplicate_def=False):
    """A function without an aux size, described by a CodeView proc record.

    target: 'defined' relocates the record to the definition, 'undefined' to a
    separate undefined external entry of the same name (as VC6 does for ??_G).
    """
    text = bytes.fromhex('8b4104c3') + b'\x90' * 12
    off_field = 28 if rectyp in (0x100a, 0x100b) else 26

    def proc(length):
        body = struct.pack('<IIII', 0, 0, 0, length) + b'\0' * (off_field - 16) + b'\0' * 7 + b'\x04func'
        return struct.pack('<HH', len(body) + 2, rectyp) + body

    records = [proc(cv_len)] + ([proc(second_len)] if second_len is not None else [])
    debug = b''.join(records)
    rec_offsets, pos = [], 0
    for r in records:
        rec_offsets.append(pos); pos += len(r)

    symbols = bytearray()
    symbols += struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, 0)          # 0: definition
    if duplicate_def:
        symbols += struct.pack('<8sIhHBB', b'func', 8, 1, 0x20, 2, 0)      # 1: second definition
    undefined_index = len(symbols) // 18
    symbols += struct.pack('<8sIhHBB', b'func', 0, 0, 0x20, 2, 0)          # undefined external
    target_index = 0 if target == 'defined' else undefined_index

    relocs = b''.join(struct.pack('<IIH', o + 4 + off_field, target_index, 0x000b) for o in rec_offsets)
    text_ptr = 20 + 2 * 40
    debug_ptr = text_ptr + len(text)
    reloc_ptr = debug_ptr + len(debug)
    sym_ptr = reloc_ptr + len(relocs)
    header = struct.pack('<HHIIIHH', 0x14c, 2, 0, sym_ptr, len(symbols) // 18, 0, 0)
    sec_text = struct.pack('<8sIIIIIIHHI', b'.text', 0, 0, len(text), text_ptr, 0, 0, 0, 0, 0x60501020)
    sec_debug = struct.pack('<8sIIIIIIHHI', b'.debug$S', 0, 0, len(debug), debug_ptr, reloc_ptr, 0,
                            len(records), 0, 0x42100048)
    return header + sec_text + sec_debug + text + debug + relocs + symbols + struct.pack('<I', 4)


class CodeViewExtentTests(unittest.TestCase):
    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'cv.obj'
        path.write_bytes(codeview_fixture(**kwargs))
        return CoffObject(path)

    def defined(self, obj):
        return next(s for s in obj.symbols if s.name == 'func' and s.section_number > 0)

    def test_codeview_length_bounds_function_without_aux_size(self):
        obj = self.load()
        raw, size, _ = obj.symbol_extent(self.defined(obj))
        self.assertEqual((size, raw), (4, bytes.fromhex('8b4104c3')))

    def test_relocation_to_undefined_entry_resolves_to_definition(self):
        obj = self.load(target='undefined')
        self.assertEqual(obj.symbol_extent(self.defined(obj))[1], 4)

    def test_cv4_record_form(self):
        obj = self.load(rectyp=0x0205)
        self.assertEqual(obj.symbol_extent(self.defined(obj))[1], 4)

    def test_conflicting_lengths_are_ignored(self):
        obj = self.load(second_len=5)
        self.assertIsNone(self.defined(obj).function_size)
        self.assertEqual(obj.symbol_extent(self.defined(obj))[1], 16)

    def test_ambiguous_definition_is_not_sized(self):
        obj = self.load(target='undefined', duplicate_def=True)
        self.assertTrue(all(s.function_size is None for s in obj.symbols))

    def test_codeview_length_outside_section_is_rejected(self):
        obj = self.load(cv_len=17)
        with self.assertRaises(CoffError):
            obj.symbol_extent(self.defined(obj))


if __name__ == '__main__':
    unittest.main()
