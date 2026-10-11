"""tools/sweep_variants.py: template parsing, rendering and enumeration (no VC6 needed)."""
import argparse
import unittest

from tools.sweep_variants import (TemplateError, _permutation, assignments, describe, dimensions,
                                  parse_target, parse_template, render, symbol_for_va)


def variants(text):
    nodes = parse_template(text)
    dims = dimensions(nodes)
    return [render(nodes, a) for a in assignments(dims, None, 0)]


class TemplateTests(unittest.TestCase):
    def test_plain_text_round_trips(self):
        text = 'int f() {\n    return 1;\n}\n'
        self.assertEqual(variants(text), [text])

    def test_perm_lists_every_order_base_first(self):
        text = '//@perm P\na;\n//@|\nb;\n//@|\nc;\n//@end\n'
        out = variants(text)
        self.assertEqual(len(out), 6)
        self.assertEqual(out[0], 'a;\nb;\nc;\n')
        self.assertEqual(len(set(out)), 6)
        self.assertIn('c;\nb;\na;\n', out)

    def test_alt_with_empty_item_is_optional(self):
        text = 'x;\n//@alt A\ny;\n//@|\n//@end\nz;\n'
        self.assertEqual(variants(text), ['x;\ny;\nz;\n', 'x;\nz;\n'])

    def test_opt_is_body_then_nothing(self):
        text = '//@opt O\nint dead = 0;\n//@end\nreturn;\n'
        self.assertEqual(variants(text), ['int dead = 0;\nreturn;\n', 'return;\n'])

    def test_opt_rejects_separator(self):
        with self.assertRaises(TemplateError):
            parse_template('//@opt O\na;\n//@|\nb;\n//@end\n')

    def test_linked_names_choose_together(self):
        text = ('//@alt L\n//@|\nint* p = &x;\n//@end\n'
                'use({@L:x@|*p@});\n')
        self.assertEqual(variants(text), ['use(x);\n', 'int* p = &x;\nuse(*p);\n'])

    def test_linked_choices_must_agree_in_shape(self):
        nodes = parse_template('//@alt L\na;\n//@|\nb;\n//@end\nf({@L:1@|2@|3@});\n')
        with self.assertRaises(TemplateError):
            dimensions(nodes)

    def test_anonymous_inline_choices_are_independent(self):
        out = variants('f({@a@|(a)@}, {@b@|(b)@});\n')
        self.assertEqual(out, ['f(a, b);\n', 'f(a, (b));\n', 'f((a), b);\n', 'f((a), (b));\n'])

    def test_nested_inline_choices(self):
        out = variants('r = {@E:m - {@P:a@|p@}@|{@P:a@|p@} + m@};\n')
        self.assertEqual(out, ['r = m - a;\n', 'r = m - p;\n', 'r = a + m;\n', 'r = p + m;\n'])

    def test_nested_inline_anonymous_and_empty_items(self):
        out = variants('f({@x{@1@|2@}@|@});\n')
        self.assertEqual(sorted(set(out)), ['f();\n', 'f(x1);\n', 'f(x2);\n'])

    def test_unterminated_inline_choice_raises(self):
        with self.assertRaises(TemplateError):
            parse_template('f({@a@|b);\n')

    def test_nested_choices(self):
        text = '//@perm P\n//@alt A\na1;\n//@|\na2;\n//@end\n//@|\nb;\n//@end\n'
        out = variants(text)
        self.assertEqual(sorted(out), sorted(['a1;\nb;\n', 'b;\na1;\n', 'a2;\nb;\n', 'b;\na2;\n']))

    def test_marker_needs_own_token(self):
        # `//@permanent` and ordinary comments are text, not markers.
        text = '//@permanent note\n// @perm not a marker\n'
        self.assertEqual(variants(text), [text])

    def test_unbalanced_markers_raise(self):
        for bad in ('//@end\n', '//@|\n', '//@perm P\na;\n'):
            with self.assertRaises(TemplateError):
                parse_template(bad)

    def test_describe_shows_order_and_pick(self):
        nodes = parse_template('//@perm P\na;\n//@|\nb;\n//@end\n{@A:x@|y@}\n')
        dims = dimensions(nodes)
        self.assertEqual(describe(dims, {'P': 1, 'A': 1}), 'P=10 A=1')


class EnumerationTests(unittest.TestCase):
    def test_permutation_is_lexicographic(self):
        self.assertEqual([_permutation(3, i) for i in range(6)],
                         [(0, 1, 2), (0, 2, 1), (1, 0, 2), (1, 2, 0), (2, 0, 1), (2, 1, 0)])

    def test_sampling_keeps_base_and_is_seeded(self):
        dims = {'P': ('perm', 5), 'A': ('alt', 3)}     # 360 combinations
        first = list(assignments(dims, 20, 7))
        self.assertEqual(len(first), 20)
        self.assertEqual(first[0], {'P': 0, 'A': 0})
        self.assertEqual(first, list(assignments(dims, 20, 7)))
        self.assertEqual(len({tuple(a.values()) for a in first}), 20)
        for a in first:
            self.assertLess(a['P'], 120)
            self.assertLess(a['A'], 3)

    def test_limit_above_total_enumerates_all(self):
        dims = {'A': ('alt', 2), 'B': ('alt', 3)}
        self.assertEqual(len(list(assignments(dims, 100, 0))), 6)


class DiffTests(unittest.TestCase):
    def test_instruction_diff_marks_changes_and_elides_equal_runs(self):
        from tools.sweep_variants import instruction_diff
        retail = bytes.fromhex('8b442404' '83c001' + '40' * 6 + 'c3')
        cand = bytes.fromhex('8b442404' '83c002' + '40' * 6 + 'c3')
        lines = instruction_diff(retail, cand)
        changed = [line for line in lines if ' | ' in line]
        self.assertEqual(len(changed), 1)
        self.assertIn('add eax, 1', changed[0])
        self.assertIn('add eax, 2', changed[0])
        self.assertTrue(any(line.startswith('...') for line in lines))
        self.assertIn('ret', lines[-1])
        self.assertEqual(instruction_diff(retail, retail, context=10)[0].count('mov eax'), 2)


class TargetTests(unittest.TestCase):
    def test_parse_target_forms(self):
        t = parse_target('0x4705d0')
        self.assertEqual((t.va, t.size, t.symbol), (0x4705d0, None, None))
        t = parse_target('0x431df0:85=CollisionModelShape_Free')
        self.assertEqual((t.va, t.size, t.symbol), (0x431df0, 85, 'CollisionModelShape_Free'))
        t = parse_target('0x472e30:0x52')
        self.assertEqual(t.size, 0x52)
        with self.assertRaises(argparse.ArgumentTypeError):
            parse_target('foo')

    def test_symbol_for_va_skips_eh_helpers(self):
        bindings = {'??0UIAnim@@QAE@PAX0@Z': 0x472e30,
                    '??0UIAnim@@QAE@PAX0@Z$ehhandler': 0x472e30,
                    '__FILE__gameui': 0x472e30,
                    '?Other@@YAXXZ': 0x400000}
        self.assertEqual(symbol_for_va(bindings, 0x472e30), '??0UIAnim@@QAE@PAX0@Z')
        with self.assertRaises(SystemExit):
            symbol_for_va(bindings, 0x123456)


if __name__ == '__main__':
    unittest.main()
