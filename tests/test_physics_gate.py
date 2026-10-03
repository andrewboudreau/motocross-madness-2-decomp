import json
from pathlib import Path
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from test_internal_labels import label_fixture, TARGET_VA
from tools.run_physics_samples import compare_target, required_failures


class PhysicsGateTests(unittest.TestCase):
    def test_masked_success_cannot_pass_strict_gate(self):
        rows = [{'expected': {'expect': 'exact'}, 'exact_after_relocation_mask': True,
                 'strict_exact': False}]
        self.assertEqual(required_failures(rows), [])
        self.assertEqual(required_failures(rows, strict=True), rows)

    def test_compile_failure_fails_both_modes_and_partial_stays_diagnostic(self):
        rows = [{'expected': {}, 'error': 'compile failed'},
                {'expected': {'expect': 'partial'}, 'strict_exact': False}]
        for strict in (False, True):
            self.assertEqual(required_failures(rows, strict), rows[:1])

    def test_reviewed_bindings_are_used_and_wrong_destinations_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            obj_path = root / 'external.obj'
            obj_path.write_bytes(label_fixture(label_offset=12))
            obj = CoffObject(obj_path)
            retail = b'\xb8' + (TARGET_VA + 12).to_bytes(4, 'little') + b'\xc3\x90\x90'

            class Retail:
                def bytes_at_va(self, va, size):
                    return retail

            target = {'candidate_symbol_contains': 'func', 'target_va': hex(TARGET_VA),
                      'target_size': len(retail)}
            result = compare_target(Retail(), obj, target, root / 'targets.json')
            self.assertTrue(result['exact_after_relocation_mask'])
            self.assertFalse(result['strict_exact'])
            target['bindings'] = 'bindings.json'
            for address, expected in ((TARGET_VA + 12, True), (TARGET_VA + 13, False)):
                (root / 'bindings.json').write_text(json.dumps({'$L1': address}))
                result = compare_target(Retail(), obj, target, root / 'targets.json')
                self.assertEqual(result['strict_exact'], expected)
