"""Unit layout sweep: diagnosis of misplaced functions on a synthetic object.

The object (tests/test_unit_layout.py) holds fA and fC (extern, nodup) and
the pick-any fB; only the bindings change between cases.
"""
from __future__ import annotations

from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'tests'))

from mcm2tool.coff import CoffObject  # noqa: E402
from test_unit_layout import BINDINGS, build_object, reader, retail_image  # noqa: E402
import unit_layout_sweep as sweep  # noqa: E402


class SweepDiagnosisTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        path = Path(cls.tmp.name) / 'unit.obj'
        build_object(path)
        cls.obj = CoffObject(path)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def diagnose(self, owners=(), **bindings):
        layout, report = sweep.lay_out(self.obj, dict(BINDINGS, **bindings), {}, reader(retail_image()))
        findings = sweep.diagnose(layout, report, self.obj, list(owners), 'src/unit.cpp')
        return report, findings, sweep.unit_verdict(report, findings)

    def kinds(self, findings):
        return sorted(f['kind'] for f in findings)

    def test_identical_unit(self):
        report, findings, verdict = self.diagnose(fC=0x401010)
        self.assertEqual(verdict, 'identical')
        self.assertEqual(self.kinds(findings), ['pick_any_elsewhere'])
        self.assertEqual(findings[0]['functions'], ['fB'])

    def test_unbound_function(self):
        _, findings, verdict = self.diagnose()
        self.assertEqual(verdict, 'identical_with_unbound')
        unregistered = [f for f in findings if f['kind'] == 'unregistered']
        self.assertEqual([(f['functions'], f['static']) for f in unregistered], [(['fC'], False)])

    def test_order(self):
        # fB bound inside the span is laid out in its section-table place.
        _, findings, verdict = self.diagnose(fB=0x401020, fC=0x401010)
        self.assertEqual(verdict, 'differs')
        order = [f for f in findings if f['kind'] == 'order']
        self.assertEqual(len(order), 1)
        self.assertIn(order[0]['functions'], (['fB'], ['fC']))

    def test_gap_names_retail_owner(self):
        owners = [(0x401010, 0x10, 'src/other.cpp', 'fX'), (0x401010, 0x10, 'src/unit.cpp', 'fA')]
        _, findings, _ = self.diagnose(owners, fC=0x401020)
        gap = [f for f in findings if f['kind'] == 'gap_missing']
        self.assertEqual(len(gap), 1)
        self.assertEqual((gap[0]['after'], gap[0]['before'], gap[0]['bytes']), ('fA', 'fC', 0x10))
        self.assertEqual([o['symbol'] for o in gap[0]['retail_owners']], ['fX'])
        self.assertEqual(gap[0]['other_src'], ['src/other.cpp'])

    def test_far_binding_is_kept_elsewhere(self):
        _, findings, _ = self.diagnose(fC=0x480000)
        self.assertEqual(self.kinds(findings), ['icf_elsewhere', 'pick_any_elsewhere'])

    def test_extra_bytes(self):
        _, findings, _ = self.diagnose(fC=0x401008)
        self.assertIn('overlap', self.kinds(findings))

    def test_in_unit_icf_fold(self):
        report, findings, _ = self.diagnose(fC=0x401000)
        folded = [r for r in report['sections'] if r['status'] == 'folded']
        self.assertEqual([r['section'] for r in folded], [5])
        icf = [f for f in findings if f['kind'] == 'icf_duplicate']
        self.assertEqual([f['functions'] for f in icf], [['fC']])
        self.assertNotIn('order', self.kinds(findings))

    def test_opt_ref_removed_is_not_blocking(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'unit.obj'
            build_object(path, unreferenced=True)
            obj = CoffObject(path)
            layout, report = sweep.lay_out(obj, dict(BINDINGS, fC=0x401010), {}, reader(retail_image()),
                                           b'\xcc' * 64)
            findings = sweep.diagnose(layout, report, obj, [], 'src/unit.cpp')
        self.assertEqual(self.kinds(findings), ['opt_ref_removed', 'pick_any_elsewhere'])
        self.assertEqual(sweep.unit_verdict(report, findings), 'identical')

    def test_heaviest_increasing_subsequence(self):
        self.assertEqual(sweep.increasing_subsequence([3, 1, 2]), {1, 2})
        self.assertEqual(sweep.increasing_subsequence([5, 1, 2], [3, 1, 1]), {0})
        self.assertEqual(sweep.increasing_subsequence([]), set())

    def test_markdown_lists_findings(self):
        entries = [{'source': 'src/a.cpp', 'status': 'laid_out', 'verdict': 'identical', 'registered': 1,
                    'strict': 1, 'findings': [], 'groups': {'.text': {'start': '0x00401000', 'end': '0x00401020'}}},
                   {'source': 'src/b.cpp', 'status': 'skipped', 'reason': 'no registered functions',
                    'registered': 0, 'strict': 0}]
        text = sweep.render_markdown(entries)
        self.assertIn('src/a.cpp', text)
        self.assertIn('src/b.cpp', text)


if __name__ == '__main__':
    unittest.main()
