import struct
import unittest

from mcm2tool.easy_match import global_bindings
from mcm2tool.resolved_match import apply_relocations, compare_bytes, RelocationError
from tools.vc6_gate import acceptance


class GlobalProbeTests(unittest.TestCase):
    def setUp(self):
        self.va = 0x401000
        self.address = 0x550000
        self.target = {'target_va': hex(self.va), 'kind': 'return_float_global',
                       'details': {'address': self.address, 'pop_bytes': 0}}
        self.retail = b'\xd9\x05' + struct.pack('<I', self.address) + b'\xc3'
        self.symbol = '?g_00401000@@3MA'
        self.relocs = [{'offset': 2, 'type': 6, 'symbol': self.symbol}]

    def test_full_address_bytes_are_compared(self):
        bindings = global_bindings(self.retail, self.target)
        code, audit = apply_relocations(b'\xd9\x05\0\0\0\0\xc3', self.relocs, bindings, self.va)
        self.assertTrue(compare_bytes(self.retail, code)['strict_exact'])
        self.assertEqual(audit[0]['bound_va'], '0x00550000')

    def test_wrong_address_fails_even_when_opcode_matches(self):
        code, _ = apply_relocations(b'\xd9\x05\0\0\0\0\xc3', self.relocs,
                                    {self.symbol: self.address + 4}, self.va)
        self.assertFalse(compare_bytes(self.retail, code)['strict_exact'])

    def test_stale_manifest_address_is_rejected(self):
        self.target['details']['address'] += 4
        with self.assertRaises(RelocationError):
            global_bindings(self.retail, self.target)

    def test_wrong_opcode_is_rejected(self):
        with self.assertRaises(RelocationError):
            global_bindings(b'\x90' + self.retail[1:], self.target)

    def test_trailing_bytes_are_not_discarded(self):
        with self.assertRaises(RelocationError):
            global_bindings(self.retail + b'\x90', self.target)

    def test_callee_pop_is_checked(self):
        retail = self.retail[:-1] + b'\xc2\x04\0'
        with self.assertRaises(RelocationError):
            global_bindings(retail, self.target)
        self.target['details']['pop_bytes'] = 4
        self.assertEqual(global_bindings(retail, self.target)[self.symbol], self.address)

    def test_arbitrary_external_symbol_is_not_bound(self):
        self.relocs[0]['symbol'] = '?another@@3MA'
        with self.assertRaises(RelocationError):
            apply_relocations(b'\xd9\x05\0\0\0\0\xc3', self.relocs,
                              global_bindings(self.retail, self.target), self.va)

    def test_non_global_probe_gets_no_bindings(self):
        self.target['kind'] = 'get_i32'
        self.assertEqual(global_bindings(self.retail, self.target), {})


class GateAcceptanceTests(unittest.TestCase):
    def setUp(self):
        self.probe = {'sp3_core_match': True, 'compiler_probe_returncode': 0}
        self.smoke = [{'exact_after_relocation_mask': True, 'relocations_masked': 0}]
        self.easy = [{'strict_exact': True}]

    def test_strict_corpora_and_executed_sp3_pass(self):
        self.assertTrue(acceptance(self.probe, self.smoke, self.easy)['all_exact'])

    def test_missing_or_failed_execution_cannot_pass(self):
        for rc in (None, 1, 0xc0000135):
            self.probe['compiler_probe_returncode'] = rc
            self.assertFalse(acceptance(self.probe, self.smoke, self.easy)['all_exact'])

    def test_wrong_compiler_cannot_pass(self):
        self.probe['sp3_core_match'] = False
        self.assertFalse(acceptance(self.probe, self.smoke, self.easy)['all_exact'])

    def test_masked_only_generated_result_cannot_pass(self):
        self.assertFalse(acceptance(self.probe, self.smoke,
                                    [{'exact_after_relocation_mask': True}])['all_exact'])

    def test_unresolved_manual_relocation_cannot_pass(self):
        self.smoke[0]['relocations_masked'] = 4
        self.assertFalse(acceptance(self.probe, self.smoke, self.easy)['all_exact'])

    def test_empty_or_failed_corpus_cannot_pass(self):
        for rows in ([], {'error': 'compile failed'}, None):
            self.assertFalse(acceptance(self.probe, rows, self.easy)['all_exact'])
            self.assertFalse(acceptance(self.probe, self.smoke, rows)['all_exact'])


if __name__ == '__main__':
    unittest.main()
