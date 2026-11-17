"""Original-unit tooling: ownership inference, report diffs and definition extraction."""
import json
import tempfile
import unittest
from pathlib import Path

from tools import accuracy_diff, tu_ownership
from tools.unit_source import function_definition, standalone_source


def row(va, symbol, obj, size=16, score=1.0, **extra):
    return dict({'retail_va': '0x%08x' % va, 'retail_bytes': size, 'symbol': symbol, 'object': obj,
                 'score': score, 'unexpected_calls': [], 'missing_calls': []}, **extra)


class OwnershipTests(unittest.TestCase):
    def setUp(self):
        self.rows = [
            row(0x1000, '?A@@YAXXZ', 'a.obj'),       # anchor a.cpp
            row(0x1010, '?B@@YAXXZ', 'other.obj'),   # between two a.cpp anchors
            row(0x1020, '??_GA@@UAEPAXI@Z', 'a.obj'),  # pick-any: never an anchor
            row(0x1030, '?C@@YAXXZ', 'a.obj'),       # anchor a.cpp
            row(0x1040, '?D@@YAXXZ', 'a.obj'),       # run from C
            row(0x1050, '?E@@YAXXZ', 'b.obj'),       # unattributed
            row(0x1060, '?F@@YAXXZ', 'b.obj'),       # anchor via a header path only
        ]
        self.xrefs = [{'path': 'D:\\aardvark\\VC\\krusty2\\a.cpp', 'text_xrefs': ['0x1004', '0x1034', '0x1028']},
                      {'path': 'D:\\aardvark\\VC\\krusty2\\a.h', 'text_xrefs': ['0x1064']}]

    def attribute(self):
        with tempfile.TemporaryDirectory() as d:
            report, xrefs = Path(d) / 'r.json', Path(d) / 'x.json'
            report.write_text(json.dumps(self.rows))
            xrefs.write_text(json.dumps(self.xrefs))
            functions = tu_ownership.load_functions(report)
            return functions, tu_ownership.attribute(functions, tu_ownership.load_anchors(xrefs))

    def test_evidence_levels(self):
        functions, evidence = self.attribute()
        got = {f['symbol']: e for f, e in zip(functions, evidence)}
        self.assertEqual(got['?A@@YAXXZ'], ('anchor', 'a.cpp'))
        self.assertEqual(got['?B@@YAXXZ'], ('between', 'a.cpp'))
        self.assertEqual(got['??_GA@@UAEPAXI@Z'], ('anchor', 'a.cpp'))
        self.assertEqual(got['?C@@YAXXZ'], ('anchor', 'a.cpp'))
        self.assertEqual(got['?D@@YAXXZ'], ('run', 'a.cpp'))
        self.assertIsNone(got['?E@@YAXXZ'])
        self.assertIsNone(got['?F@@YAXXZ'])

    def test_class_members_in_a_gap_join_the_unit_holding_their_class(self):
        self.rows = [
            row(0x1000, '?A@T@@QAEXXZ', 't.obj'),      # anchor t.cpp
            row(0x1010, '??0T@@QAE@XZ', 't.obj'),      # between
            row(0x1020, '?B@T@@QAEXXZ', 't.obj'),      # anchor t.cpp
            row(0x1030, '?C@T@@QAEXXZ', 'g.obj'),      # gap, class T is in t.cpp only
            row(0x1040, '?Free@@YAXXZ', 'g.obj'),      # gap, no class: unattributed
            row(0x1050, '?A@U@@QAEXXZ', 'u.obj'),      # anchor u.cpp
        ]
        self.xrefs = [{'path': 'x\\t.cpp', 'text_xrefs': ['0x1004', '0x1024']},
                      {'path': 'x\\u.cpp', 'text_xrefs': ['0x1054']}]
        functions, evidence = self.attribute()
        got = {f['symbol']: e for f, e in zip(functions, evidence)}
        self.assertEqual(got['??0T@@QAE@XZ'], ('between', 't.cpp'))
        self.assertEqual(got['?C@T@@QAEXXZ'], ('class', 't.cpp'))
        self.assertIsNone(got['?Free@@YAXXZ'])
        self.assertEqual(tu_ownership.class_of('??0T@@QAE@XZ'), 'T')
        self.assertIsNone(tu_ownership.class_of('??H@YA?AUV@@ABU0@0@Z'))

    def test_report_lists_foreign_definitions(self):
        functions, _ = self.attribute()
        with tempfile.TemporaryDirectory() as d:
            xrefs = Path(d) / 'x.json'
            xrefs.write_text(json.dumps(self.xrefs))
            units, objects = tu_ownership.analyse(functions, tu_ownership.load_anchors(xrefs))
        text = tu_ownership.report_text(units, objects)
        self.assertIn('| `?B@@YAXXZ` | 0x00001010 | between | `other.obj` | `a.cpp` |', text)
        self.assertNotIn('??_GA@@UAEPAXI@Z', text.split('## Functions defined outside')[1])
        self.assertTrue(tu_ownership.is_pick_any('??$max@H@std@@YAABHABH0@Z'))
        self.assertFalse(tu_ownership.is_pick_any('?Tick@RaceSound@@UAEHM@Z'))


class AccuracyDiffTests(unittest.TestCase):
    def test_moves_are_not_regressions_but_score_drops_are(self):
        before = {0x10: row(0x10, '?A', 'old.obj', score=0.9), 0x20: row(0x20, '?B', 'old.obj'),
                  0x30: row(0x30, '?C', 'old.obj')}
        after = {0x10: row(0x10, '?A', 'unit.obj', score=0.9), 0x20: row(0x20, '?B2', 'unit.obj')}
        result = accuracy_diff.compare(before, after)
        self.assertEqual([len(result[k]) for k in ('lost', 'worse', 'moved', 'renamed')], [1, 0, 2, 1])
        self.assertEqual(accuracy_diff.regressions(result), 1)
        after[0x30] = row(0x30, '?C', 'unit.obj', score=0.5)
        self.assertEqual(len(accuracy_diff.compare(before, after)['worse']), 1)
        after[0x30] = row(0x30, '?C', 'unit.obj', unexpected_calls=['?X'])
        result = accuracy_diff.compare(before, after)
        self.assertEqual((len(result['calls']), accuracy_diff.regressions(result)), (1, 1))


class UnitSourceTests(unittest.TestCase):
    UNIT = ('#include "a.h"\n#include <string.h>\n\n'
            '// 0x1000\nint Thing::Get() const {\n    return "}"[0] == \'{\'; // }\n}\n\n'
            '/* { */\nvoid Thing::Put(int v) {\n    if (v) { value = v; }\n}\n')

    def test_extracts_whole_definition_ignoring_braces_in_literals_and_comments(self):
        self.assertEqual(function_definition(self.UNIT, 'Thing::Get'),
                         'int Thing::Get() const {\n    return "}"[0] == \'{\'; // }\n}')
        self.assertEqual(function_definition(self.UNIT, 'Thing::Put'),
                         'void Thing::Put(int v) {\n    if (v) { value = v; }\n}')
        with self.assertRaises(ValueError):
            function_definition(self.UNIT, 'Thing::Missing')

    def test_standalone_keeps_includes_absolute_when_present(self):
        with tempfile.TemporaryDirectory() as d:
            unit = Path(d) / 'unit.cpp'
            unit.write_text(self.UNIT)
            (Path(d) / 'a.h').write_text('struct Thing { int value; int Get() const; void Put(int); };\n')
            text = standalone_source(unit, ['Thing::Put'], prelude='#define X 1')
        lines = text.splitlines()
        self.assertTrue(lines[0].startswith('#include "') and lines[0].endswith('/a.h"'))
        self.assertEqual(lines[1:3], ['#include <string.h>', '#define X 1'])
        self.assertNotIn('Thing::Get', text)


if __name__ == '__main__':
    unittest.main()
