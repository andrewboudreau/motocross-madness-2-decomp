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


def codeview_fixture(size=4, *, aux_size=None, split=True, signature=2,
                     paired=True, addend=0, conflict=False, truncated=False,
                     wrong_symbol=False, next_symbol=None):
    """Synthetic public-format metadata; contains no compiler or retail bytes."""
    text = bytes.fromhex('8b4104c3') + b'\x90' * 12
    symbols = bytearray(struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, int(aux_size is not None)))
    if aux_size is not None:
        symbols += struct.pack('<IIIIH', 0, aux_size, 0, 0, 0)
    ref = len(symbols)//18
    symbols += struct.pack('<8sIhHBB', b'func', 0, 0, 0x20, 2, 0)
    other = len(symbols)//18
    symbols += struct.pack('<8sIhHBB', b'other', 0, 0, 0x20, 2, 0)
    if next_symbol is not None:
        symbols += struct.pack('<8sIhHBB', b'next', next_symbol, 1, 0x20, 2, 0)
    def proc(length):
        # Deliberately different display name: identity must use relocations.
        record = bytearray(40 + len(b'not_func'))
        struct.pack_into('<HH', record, 0, len(record)-2, 0x100b)
        struct.pack_into('<I', record, 16, length)
        struct.pack_into('<I', record, 32, addend)
        record[39] = len(b'not_func')
        record[40:] = b'not_func'
        return record
    debug = proc(size)
    positions = [0]
    if conflict:
        positions.append(len(debug))
        debug += proc(size+1)
    if truncated: debug.pop()
    prefix = struct.pack('<I', signature)
    if not split:
        debug = prefix + debug
        positions = [p+4 for p in positions]
    rels = b''.join(struct.pack('<IIH', p+32, ref, 0xb) +
                    (struct.pack('<IIH', p+36, other if wrong_symbol else ref, 0xa) if paired else b'')
                    for p in positions)
    sections = [(b'.text', text, b'', 0x60501020)]
    if split: sections.append((b'.debug$S', prefix, b'', 0x42100040))
    sections.append((b'.debug$S', debug, rels, 0x42100040))
    headers, payload = bytearray(), bytearray()
    pos = 20 + 40*len(sections)
    for name, raw, relocations, flags in sections:
        headers += struct.pack('<8sIIIIIIHHI', name, 0, 0, len(raw), pos,
                               pos+len(raw) if relocations else 0, 0, len(relocations)//10, 0, flags)
        payload += raw + relocations
        pos += len(raw) + len(relocations)
    return (struct.pack('<HHIIIHH', 0x14c, len(sections), 0, pos, len(symbols)//18, 0, 0)
            + headers + payload + symbols + struct.pack('<I', 4))


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


class CodeViewExtentTests(unittest.TestCase):
    def load(self, **kwargs):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        path = Path(td.name) / 'codeview.obj'
        path.write_bytes(codeview_fixture(**kwargs))
        return CoffObject(path)

    def test_relocation_bound_length_in_both_section_forms(self):
        for split in (True, False):
            obj = self.load(split=split)
            sym = obj.find_symbol('func')
            self.assertIsNone(sym.function_size)
            self.assertEqual(sym.codeview_size, 4)
            result = match_object(obj, 'func', 0x401000, bytes.fromhex('8b4104c3'), {})
            self.assertTrue(result['strict_exact'])
            self.assertEqual(result['extent_source'], 'codeview_proc')

    def test_agreement_with_auxiliary_record(self):
        obj = self.load(aux_size=4)
        self.assertEqual(obj.find_symbol('func').extent_source, 'coff_function_aux')

    def test_conflicting_lengths_rejected(self):
        for kwargs in ({'aux_size': 5}, {'conflict': True}):
            with self.assertRaises(CoffError): self.load(**kwargs)

    def test_requires_paired_relocations_to_same_symbol_and_zero_addend(self):
        for kwargs in ({'paired': False}, {'wrong_symbol': True}, {'addend': 1}):
            obj = self.load(**kwargs)
            self.assertIsNone(obj.find_symbol('func').codeview_size)
            self.assertEqual(obj.symbol_extent(obj.find_symbol('func'))[1], 16)

    def test_unknown_debug_generation_does_not_supply_size(self):
        obj = self.load(signature=4)
        self.assertIsNone(obj.find_symbol('func').codeview_size)

    def test_truncated_record_rejected(self):
        with self.assertRaises(CoffError): self.load(truncated=True)

    def test_invalid_extents_rejected(self):
        for kwargs in ({'size': 0}, {'size': 17}, {'size': 9, 'next_symbol': 8}):
            obj = self.load(**kwargs)
            with self.assertRaises(CoffError): obj.symbol_extent(obj.find_symbol('func'))

    def test_target_size_cannot_shorten_codeview_extent(self):
        obj = self.load(size=5)
        result = match_object(obj, 'func', 0x401000, bytes.fromhex('8b4104c3'), {})
        self.assertFalse(result['strict_exact'])
        self.assertEqual(result['candidate_size'], 5)


if __name__ == '__main__':
    unittest.main()
