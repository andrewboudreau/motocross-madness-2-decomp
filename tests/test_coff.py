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


if __name__ == '__main__':
    unittest.main()
