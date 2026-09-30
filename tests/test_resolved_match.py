from __future__ import annotations
from types import SimpleNamespace
import struct
import unittest

from mcm2tool.resolved_match import RelocationError, match_object


class FakeObject:
    machine = 0x14c

    def __init__(self, raw: bytes, record, *, relocation_type=0x0006):
        self.raw = raw
        self.function = SimpleNamespace(
            name='func',
            value=0x20,
            section_number=1,
            extent_source='coff_function_aux',
        )
        self.section_record = SimpleNamespace(
            index=1, raw_size=0x100, raw_ptr=0
        )
        self.relocation = SimpleNamespace(
            section_number=1,
            virtual_address=self.function.value + 1,
            symbol_index=7,
            type=relocation_type,
        )
        self.symbol_by_index = {7: record}
        self.data = raw + b'\0' * (0x100 - len(raw))

    def find_symbol(self, query):
        if query != 'func':
            raise ValueError(query)
        return self.function

    def symbol_extent(self, symbol):
        return self.raw, len(self.raw), [self.relocation]

    def section(self, number):
        if number != 1:
            raise ValueError(number)
        return self.section_record


def code(opcode: int, addend: int = 0) -> bytes:
    return bytes([opcode]) + struct.pack('<I', addend) + b'\xc3' + b'\x90' * 10


class InternalRelocationTests(unittest.TestCase):
    def test_defined_local_dir32_is_resolved_inside_function(self):
        record = SimpleNamespace(name='case0', value=0x28, section_number=1)
        obj = FakeObject(code(0xa1), record)
        retail = code(0xa1, 0x00401008)
        result = match_object(obj, 'func', 0x00401000, retail)
        self.assertTrue(result['strict_exact'])
        self.assertEqual(result['ignored_bytes'], 0)
        self.assertEqual(
            result['relocations_applied'][0]['binding_source'],
            'internal_same_function',
        )
        self.assertEqual(
            result['relocations_applied'][0]['bound_va'],
            '0x00401008',
        )

    def test_section_symbol_addend_resolves_jump_table_style_reference(self):
        # Section base is 0x400fe0 because func lives at section offset 0x20.
        # A section-symbol addend of 0x28 therefore maps to target+8.
        record = SimpleNamespace(name='.text', value=0, section_number=1)
        obj = FakeObject(code(0xa1, 0x28), record)
        retail = code(0xa1, 0x00401008)
        result = match_object(obj, 'func', 0x00401000, retail)
        self.assertTrue(result['strict_exact'])
        self.assertEqual(
            result['relocations_applied'][0]['bound_va'],
            '0x00400fe0',
        )

    def test_internal_rel32_is_resolved(self):
        record = SimpleNamespace(name='case0', value=0x28, section_number=1)
        obj = FakeObject(code(0xe8), record, relocation_type=0x0014)
        retail = code(0xe8, 3)
        result = match_object(obj, 'func', 0x00401000, retail)
        self.assertTrue(result['strict_exact'])
        self.assertEqual(result['relocations_applied'][0]['written_u32'], 3)

    def test_same_section_reference_outside_function_fails_closed(self):
        record = SimpleNamespace(name='other', value=0x50, section_number=1)
        obj = FakeObject(code(0xa1), record)
        with self.assertRaisesRegex(RelocationError, 'unresolved symbol'):
            match_object(obj, 'func', 0x00401000, code(0xa1))

    def test_external_reference_still_requires_binding(self):
        record = SimpleNamespace(name='external', value=0, section_number=0)
        obj = FakeObject(code(0xa1), record)
        with self.assertRaisesRegex(RelocationError, 'unresolved symbol'):
            match_object(obj, 'func', 0x00401000, code(0xa1))

        retail = code(0xa1, 0x00402000)
        result = match_object(
            obj, 'func', 0x00401000, retail, {'external': 0x00402000}
        )
        self.assertTrue(result['strict_exact'])
        self.assertEqual(
            result['relocations_applied'][0]['binding_source'], 'explicit'
        )

    def test_explicit_binding_overrides_same_section_auto_mapping(self):
        record = SimpleNamespace(name='case0', value=0x28, section_number=1)
        obj = FakeObject(code(0xa1), record)
        retail = code(0xa1, 0x00401008)
        result = match_object(
            obj, 'func', 0x00401000, retail, {'case0': 0x00401009}
        )
        self.assertFalse(result['strict_exact'])
        self.assertEqual(
            result['relocations_applied'][0]['binding_source'], 'explicit'
        )


if __name__ == '__main__':
    unittest.main()
