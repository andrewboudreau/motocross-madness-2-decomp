from __future__ import annotations
import unittest
from tools.vc6_profile_matrix import summarize_calibration, summarize_rows


class ProfileMatrixTests(unittest.TestCase):
    def test_strict_failure_overrides_masked_success(self):
        result = {'exact_after_relocation_mask': True, 'strict_exact': False,
                  'match_percent': 100}
        self.assertEqual(summarize_rows([result])['exact'], 0)
        self.assertEqual(summarize_calibration([{'result': result}])['exact'], 0)

    def test_summarize_rows(self):
        summary = summarize_rows([
            {'exact_after_relocation_mask': True, 'match_percent': 100},
            {'exact_after_relocation_mask': False, 'match_percent': 75.5},
        ])
        self.assertEqual(summary['total'], 2)
        self.assertEqual(summary['exact'], 1)
        self.assertEqual(summary['mean_match_percent'], 87.75)

    def test_summarize_calibration_counts_compile_failure(self):
        summary = summarize_calibration([
            {'result': {'exact_after_relocation_mask': True, 'match_percent': 100}},
            {'result': {'exact_after_relocation_mask': False, 'match_percent': 50}},
            {'compile_error': 'failed'},
        ])
        self.assertEqual(summary['total'], 3)
        self.assertEqual(summary['compiled'], 2)
        self.assertEqual(summary['compile_failures'], 1)
        self.assertEqual(summary['exact'], 1)
        self.assertEqual(summary['mean_match_percent'], 75.0)


if __name__ == '__main__':
    unittest.main()
