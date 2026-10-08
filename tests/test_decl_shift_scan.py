"""tools/decl_shift_scan.py: the prefix, unit and summary parts (no VC6 needed)."""
import unittest

from tools.decl_shift_scan import (UNITS_PER_PERIOD, is_function_name, prefix_declarations,
                                   resolve_symbol, summarize, units, windows)


class FakeSymbol:
    def __init__(self, name, section=1, storage=2):
        self.name, self.section_number, self.storage_class = name, section, storage


class FakeObject:
    def __init__(self, names):
        self.symbols = [FakeSymbol(n) for n in names]


class DeclShiftTests(unittest.TestCase):
    def test_prefix_kinds_and_units(self):
        self.assertEqual(prefix_declarations(0), '')
        self.assertEqual(prefix_declarations(2), 'typedef int DeclShiftProbe0;\ntypedef int DeclShiftProbe1;\n')
        self.assertEqual(prefix_declarations(1, 'fn'), 'int DeclShiftProbe0(int);\n')
        with self.assertRaises(ValueError):
            prefix_declarations(1, 'struct')
        # a typedef is half a unit, a one-parameter prototype a whole one; 64 typedefs span a period
        self.assertEqual(units(64, 'td'), UNITS_PER_PERIOD)
        self.assertEqual(units(32, 'fn'), UNITS_PER_PERIOD)

    def test_windows(self):
        self.assertEqual(windows([]), [])
        self.assertEqual(windows([True, True]), [(0, 1)])
        self.assertEqual(windows([False, True, True, False, True]), [(1, 2), (4, 4)])

    def test_summary_verdicts(self):
        always = {k: (10, 10, True) for k in range(4)}
        self.assertEqual(summarize(always)['verdict'], 'exact at every k')
        self.assertFalse(summarize(always)['sensitive'])
        never = {k: (9, 10, False) for k in range(4)}
        s = summarize(never)
        self.assertEqual(s['verdict'], 'never exact')
        self.assertEqual(s['k0'], [9, 10])
        self.assertEqual(s['exact_windows'], [])
        # the TrackGame slot-1 shape: exact except in one window of prefix sizes
        tie = {k: ((10, 10, True) if k not in (3, 4, 5, 6) else (8, 10, False)) for k in range(10)}
        s = summarize(tie)
        self.assertTrue(s['sensitive'])
        self.assertEqual(s['distinct_scores'], [8, 10])
        self.assertEqual(s['exact_windows'], [(0, 2), (7, 9)])
        self.assertEqual(s['verdict'], 'exact only at some k (scheduling tie-break)')
        # a sparse k set has no window list
        self.assertIsNone(summarize({0: (1, 1, True), 5: (1, 1, True)})['exact_windows'])

    def test_function_name_filter(self):
        self.assertTrue(is_function_name('?UnknownVirtualSlot1@TrackGame@@UAEHXZ'))
        self.assertTrue(is_function_name('_main'))
        self.assertFalse(is_function_name('??_C@_0CB@ENLE@IntervalBetweenShortNetPacketsMS@'))
        self.assertFalse(is_function_name('??_7TrackGame@@6B@'))
        self.assertFalse(is_function_name('?g_DebugSocket@@3PAVDebugSocket@@A'))
        self.assertFalse(is_function_name('$L1'))
        self.assertFalse(is_function_name('?f@@YAXXZ$ehhandler'))

    def test_resolve_symbol_prefers_exact_name(self):
        obj = FakeObject(['?Foo@A@@QAEXXZ', '?FooBar@A@@QAEXXZ', '$L1'])
        self.assertEqual(resolve_symbol(obj, '?FooBar@A@@QAEXXZ'), '?FooBar@A@@QAEXXZ')
        self.assertEqual(resolve_symbol(obj, 'Foo@A'), '?Foo@A@@QAEXXZ')
        self.assertIsNone(resolve_symbol(obj, 'Baz'))


if __name__ == '__main__':
    unittest.main()
