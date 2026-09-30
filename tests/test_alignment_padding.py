import unittest

from mcm2tool.coff import alignment_padding

RET = bytes.fromhex('8b410440894104c3')  # BaseObject::AddRef, 8 bytes


class AlignmentPaddingTests(unittest.TestCase):
    def test_vc6_nop_fill_to_boundary_is_padding(self):
        self.assertEqual(alignment_padding(RET + b'\x90' * 8, 8, 0), 8)
        self.assertEqual(alignment_padding(RET + b'\x90' * 8, 8, 0x20), 8)

    def test_no_extra_bytes(self):
        self.assertEqual(alignment_padding(RET, 8, 0), 0)
        self.assertEqual(alignment_padding(RET, 12, 0), 0)

    def test_non_nop_tail_is_not_padding(self):
        self.assertEqual(alignment_padding(RET + b'\x90' * 7 + b'\xc3', 8, 0), 0)
        self.assertEqual(alignment_padding(RET + b'\xcc' * 8, 8, 0), 0)

    def test_tail_must_end_on_alignment_boundary(self):
        self.assertEqual(alignment_padding(RET + b'\x90' * 7, 8, 0), 0)
        self.assertEqual(alignment_padding(RET + b'\x90' * 8, 8, 4), 0)

    def test_padding_never_spans_a_full_alignment_unit(self):
        self.assertEqual(alignment_padding(RET + b'\x90' * 24, 8, 0), 0)


if __name__ == '__main__':
    unittest.main()
