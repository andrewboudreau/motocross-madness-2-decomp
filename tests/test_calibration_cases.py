import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


class CalibrationCaseListTests(unittest.TestCase):
    def test_no_case_dict_repeats_a_key(self):
        # A missing `},{` between two cases merges them into one dict literal,
        # and Python silently keeps only the later keys, dropping a case.
        tree = ast.parse((ROOT / 'tools/run_calibration.py').read_text())
        for node in ast.walk(tree):
            if isinstance(node, ast.Dict):
                keys = [k.value for k in node.keys if isinstance(k, ast.Constant)]
                self.assertEqual(len(keys), len(set(keys)),
                                 f'duplicate keys in the dict at line {node.lineno}')


if __name__ == '__main__':
    unittest.main()
