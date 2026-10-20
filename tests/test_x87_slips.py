import unittest

from tools import x87_slips as X


class OperandKindTests(unittest.TestCase):
    def test_kinds(self):
        self.assertEqual(X.operand_kind('dword ptr [esp + 0x10]'), 'frame')
        self.assertEqual(X.operand_kind('dword ptr [ebp - 8]'), 'frame')
        self.assertEqual(X.operand_kind('dword ptr [ecx + 4]'), 'indirect')
        self.assertEqual(X.operand_kind('dword ptr [esp + eax*4]'), 'indirect')
        self.assertEqual(X.operand_kind('dword ptr [0x5a0010]'), 'global')
        self.assertEqual(X.operand_kind('qword ptr ds:[0x5a0010]'), 'global')
        self.assertIsNone(X.operand_kind('st(1)'))


class FormsTests(unittest.TestCase):
    def test_dup_load_and_memmem(self):
        lines = [
            'fld st(0)', 'fmul dword ptr [ecx]',
            'fld dword ptr [esp + 8]', 'fmul st(1)',
            'fld dword ptr [ecx]', 'fadd dword ptr [ecx + 4]',
            'fstp st(0)',
        ]
        got = [(f, op, mem) for f, op, mem, _ in X.forms(lines)]
        self.assertEqual(got, [
            ('dup', 'fmul', 'dword ptr [ecx]'),
            ('load', 'fmul', 'dword ptr [esp + 8]'),
            ('memmem', 'fadd', 'dword ptr [ecx] | dword ptr [ecx + 4]'),
        ])

    def test_non_arith_pairs_ignored(self):
        self.assertEqual(X.forms(['fld st(0)', 'fstp dword ptr [ecx]']), [])


class ClassifyTests(unittest.TestCase):
    def test_dup_against_load(self):
        ours = ['fld st(0)', 'fmul dword ptr [esp + 8]']
        retail = ['fld dword ptr [esp + 8]', 'fmul st(1)']
        slips = X.classify(ours, retail)
        self.assertEqual(len(slips), 1)
        self.assertEqual(slips[0]['kind'], 'ours dup, retail load')
        self.assertEqual(slips[0]['mem'], ['dword ptr [esp + 8]'])

    def test_swapped_memory_operands(self):
        ours = ['fld dword ptr [ecx + 8]', 'fmul dword ptr [edx]']
        retail = ['fld dword ptr [edx]', 'fmul dword ptr [ecx + 8]']
        slips = X.classify(ours, retail)
        self.assertEqual(slips[0]['kind'], 'memory operands swapped')
        self.assertEqual(slips[0]['retail_first'], 'dword ptr [edx]')

    def test_different_words_are_not_a_slip(self):
        ours = ['fld st(0)', 'fmul dword ptr [esp + 8]']
        retail = ['fld dword ptr [esp + 0xc]', 'fmul st(1)']
        self.assertEqual(X.classify(ours, retail), [])


class HintTests(unittest.TestCase):
    def slip(self, kind, mem):
        return dict(kind=kind, op='fmul', mem=mem, at=0)

    def test_rule6_store(self):
        s = self.slip('ours dup, retail load', ['dword ptr [esp + 8]'])
        self.assertTrue(X.hint(s, ['frame'], [[0x10]]).startswith('rule 6'))
        self.assertTrue(X.hint(s, ['frame'], [[]]).startswith('rules 3-5'))

    def test_load_to_dup(self):
        s = self.slip('ours load, retail dup', ['dword ptr [esp + 8]'])
        self.assertTrue(X.hint(s, ['frame'], [[]]).startswith('rule 6, 4 or 5'))

    def test_frame_pairs(self):
        s = self.slip('memory operands swapped', ['dword ptr [esp + 8]', 'dword ptr [esp + 0xc]'])
        self.assertIn('never-stored', X.hint(s, ['frame', 'frame'], [[], []]))
        self.assertIn('canonical', X.hint(s, ['frame', 'frame'], [[1], [2]]))

    def test_same_base_offsets(self):
        s = self.slip('memory operands swapped', ['dword ptr [ecx + 8]', 'dword ptr [ecx + 4]'])
        self.assertIn('different pointers', X.hint(s, ['indirect', 'indirect'], [[], []]))
        s = self.slip('memory operands swapped', ['dword ptr [ecx + 4]', 'dword ptr [edx + 8]'])
        self.assertIn('different bases', X.hint(s, ['indirect', 'indirect'], [[], []]))


class SourceLineTests(unittest.TestCase):
    def test_latest_entry_at_or_before(self):
        entries = [(0, 10), (0x20, 12), (0x10, 11)]
        self.assertEqual(X.source_line(entries, 0x18), 11)
        self.assertEqual(X.source_line(entries, 0x20), 12)
        self.assertIsNone(X.source_line([(4, 3)], 0))


if __name__ == '__main__':
    unittest.main()
