import struct
import unittest

from tools import eh_funcinfo as E


class FakePE:
    """Sparse little-endian memory image keyed by VA."""

    def __init__(self):
        self.mem = {}

    def put(self, va, data):
        for i, b in enumerate(data):
            self.mem[va + i] = b

    def bytes_at_va(self, va, n):
        return bytes(self.mem.get(va + i, 0) for i in range(n))


def build(prologue, max_state=2, tries=()):
    pe = FakePE()
    func, handler, fi, um, tb = 0x401000, 0x402000, 0x403000, 0x403100, 0x403200
    pe.put(func, prologue + struct.pack('<I', handler))
    pe.put(handler, b'\xb8' + struct.pack('<I', fi) + b'\xe9\0\0\0\0')
    pe.put(fi, struct.pack('<IIIIIII', E.MAGIC, max_state, um, len(tries), tb, 0, 0))
    pe.put(um, b''.join(struct.pack('<iI', -1, 0x404000 + k) for k in range(max_state)))
    pe.put(tb, b''.join(struct.pack('<iiiiI', *t, 0) for t in tries))
    return pe, func


class EhFuncInfoTests(unittest.TestCase):
    def test_prologue_with_and_without_frame_pointer(self):
        self.assertEqual(E.handler_operand_offset(b'\x6a\xff\x68\0\0\0\0'), 3)
        self.assertEqual(E.handler_operand_offset(b'\x55\x8b\xec\x6a\xff\x68\0\0\0\0'), 6)
        self.assertIsNone(E.handler_operand_offset(b'\x83\xec\x10\x53'))

    def test_retail_parses_states_and_try_blocks(self):
        pe, va = build(b'\x55\x8b\xec\x6a\xff\x68', max_state=4, tries=[(1, 2, 3, 1)])
        info = E.retail(va, pe)
        self.assertEqual(info['max_state'], 4)
        self.assertEqual(info['try_blocks'], 1)
        self.assertEqual(info['try'][0], dict(low=1, high=2, catch_high=3, catches=1))
        self.assertEqual([t for t, _ in info['unwind']], [-1] * 4)

    def test_shape_ignores_funclet_addresses_but_not_counts(self):
        pe, va = build(b'\x6a\xff\x68', max_state=2)
        a = E.retail(va, pe)
        b = dict(a, unwind=[(-1, '$L1'), (-1, '$L2')])
        self.assertEqual(E.shape(a), E.shape(b))
        c = dict(a, max_state=3, unwind=a['unwind'] + [(1, '$L3')])
        self.assertNotEqual(E.shape(a), E.shape(c))

    def test_seh_handler_is_not_a_cxx_funcinfo(self):
        pe, va = build(b'\x6a\xff\x68')
        pe.put(0x402000, b'\x55\x8b\xec')  # __except_handler3-style target
        self.assertIsNone(E.retail(va, pe))


if __name__ == '__main__':
    unittest.main()
