"""tools/static_initializers.py on a synthetic image (no exe needed; unicorn for the run)."""
import struct
import unittest
from dataclasses import dataclass

from tools.static_initializers import cxx_table, initterm_tables, run_initializers

try:
    import unicorn  # noqa: F401
    HAVE_UNICORN = True
except ImportError:
    HAVE_UNICORN = False

BASE = 0x400000
TEXT_RVA, DATA_RVA = 0x1000, 0x2000
TEXT_RAW, DATA_RAW, HEADERS = 0x400, 0x600, 0x400
INITTERM = BASE + 0x1100
TABLE = BASE + 0x2000           # four entries
SHORT_TABLE = BASE + 0x2020     # one entry: a C initializer table
VALUE, COUNTER, PARTIAL, IAT = BASE + 0x2100, BASE + 0x2104, BASE + 0x2108, BASE + 0x2180


@dataclass
class Section:
    virtual_address: int
    virtual_size: int
    raw_offset: int
    raw_size: int
    characteristics: int


class FakePE:
    def __init__(self, text: dict[int, bytes], data: dict[int, bytes]):
        raw = bytearray(DATA_RAW + 0x200)
        for blocks, rva, at in ((text, TEXT_RVA, TEXT_RAW), (data, DATA_RVA, DATA_RAW)):
            for va, blob in blocks.items():
                off = at + va - BASE - rva
                raw[off:off + len(blob)] = blob
        self.data = bytes(raw)
        self.image_base, self.size_of_image, self.size_of_headers = BASE, 0x3000, HEADERS
        self.sections = [Section(TEXT_RVA, 0x200, TEXT_RAW, 0x200, 0x60000020),
                         Section(DATA_RVA, 0x200, DATA_RAW, 0x200, 0xc0000040)]


def call(at: int, target: int) -> bytes:
    return b'\xe8' + struct.pack('<i', target - (at + 5))


def push(v: int) -> bytes:
    return b'\x68' + struct.pack('<I', v)


def image() -> FakePE:
    text = {
        # __cinit: _initterm(__xi_a, __xi_z) and _initterm(__xc_a, __xc_z)
        BASE + 0x1000: push(SHORT_TABLE + 4) + push(SHORT_TABLE) + call(BASE + 0x100a, INITTERM)
        + push(TABLE + 16) + push(TABLE) + call(BASE + 0x1019, INITTERM),
        BASE + 0x1040: b'\xc7\x05' + struct.pack('<I', VALUE) + struct.pack('<f', 1.0) + b'\xc3',
        BASE + 0x1050: b'\xff\x05' + struct.pack('<I', COUNTER) + b'\xc3',     # inc [counter]
        BASE + 0x1060: b'\xff\x05' + struct.pack('<I', COUNTER) + b'\xc3',
        # writes, then calls an import nobody answers
        BASE + 0x1070: b'\xc7\x05' + struct.pack('<II', PARTIAL, 7)
        + b'\xff\x15' + struct.pack('<I', IAT) + b'\xc3',
        INITTERM: b'\xc3',
    }
    data = {TABLE: struct.pack('<4I', BASE + 0x1040, BASE + 0x1050, BASE + 0x1060, BASE + 0x1070),
            SHORT_TABLE: struct.pack('<I', 0)}
    return FakePE(text, data)


class TableTests(unittest.TestCase):
    def test_the_cxx_table_has_the_most_code_pointers(self):
        pe = image()
        self.assertEqual(sorted(initterm_tables(pe, INITTERM)), [(TABLE, TABLE + 16),
                                                                  (SHORT_TABLE, SHORT_TABLE + 4)])
        self.assertEqual(cxx_table(pe, INITTERM), (TABLE, TABLE + 16))


@unittest.skipUnless(HAVE_UNICORN, 'unicorn not installed')
class RunTests(unittest.TestCase):
    def test_values_shared_state_and_failed_entries(self):
        pe = image()
        res = run_initializers(pe, (TABLE, TABLE + 16), {'__imp__GetVersionExA': IAT})
        self.assertEqual(res.ok, [BASE + 0x1040, BASE + 0x1050, BASE + 0x1060])
        self.assertEqual(bytes(res.values[VALUE + k] for k in range(4)), struct.pack('<f', 1.0))
        # two entries bump one counter: running state, not either entry's value
        self.assertEqual(res.values[COUNTER], 2)
        self.assertIn(COUNTER, res.shared)
        self.assertNotIn(VALUE, res.shared)
        self.assertEqual(res.failed, {BASE + 0x1070: 'calls __imp__GetVersionExA'})
        self.assertIn(PARTIAL, res.uncertain)
        self.assertNotIn(PARTIAL, res.values)


if __name__ == '__main__':
    unittest.main()
