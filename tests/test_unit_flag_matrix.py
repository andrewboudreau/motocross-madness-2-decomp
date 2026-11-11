"""unit_flag_matrix collects every scored target of a unit; no VC6 or retail image needed."""
import unittest

from tools.unit_flag_matrix import ROOT, known_targets, summary


class UnitFlagMatrixTest(unittest.TestCase):
    def test_targets_cover_calibration_and_physics_entries(self):
        targets = known_targets()
        bitstring = [t for t in targets if t['source'] == (ROOT / 'src/reconstructed/GR_BitString.cpp').resolve()]
        self.assertIn(0x00423e90, {t['va'] for t in bitstring})
        physics = [t for t in targets if 'krusty2' in t['source'].parts]
        self.assertTrue(physics)
        # physics trees compile with src/krusty2 on the include path, as run_physics_samples does
        self.assertTrue(all(str(ROOT / 'src/krusty2') in t['includes'] for t in physics))

    def test_summary_rules_out_a_unit_that_loses_an_exact_function(self):
        rows = [
            {'source': 'a.cpp', 'va': '0x1', 'symbol': 'f', 'flags': 'base', 'exact': True, 'insn': 0, 'matching': 4},
            {'source': 'a.cpp', 'va': '0x2', 'symbol': 'g', 'flags': 'base', 'exact': False, 'insn': 9, 'matching': 1},
            {'source': 'a.cpp', 'va': '0x1', 'symbol': 'f', 'flags': '/Oa', 'exact': False, 'insn': 2, 'matching': 3},
            {'source': 'a.cpp', 'va': '0x2', 'symbol': 'g', 'flags': '/Oa', 'exact': False, 'insn': 3, 'matching': 2},
        ]
        from io import StringIO
        from contextlib import redirect_stdout
        out = StringIO()
        with redirect_stdout(out):
            summary(rows, ['/Oa'])
        text = out.getvalue()
        self.assertIn('1 lose one (ruled out), 0 keep all; gains 0', text)
        self.assertIn('0x2 9->3', text)


if __name__ == '__main__':
    unittest.main()
