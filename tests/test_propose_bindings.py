"""Binding proposals never fit a reconstructed function to an address other than its own."""
from collections import defaultdict
import importlib.util
from pathlib import Path
import struct
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('propose_bindings', ROOT / 'tools/propose_bindings.py')
pb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pb)

SPHERE, CAPSULE = '?HullVsSphere@@YAHXZ', '?HullVsCapsule@@YAHXZ'


def evidence(targets):
    """An Evidence with only the report-derived tables populated."""
    ev = pb.Evidence.__new__(pb.Evidence)
    ev.reviewed = defaultdict(dict)
    ev.reviewed_va = {}
    ev.targets = defaultdict(list)
    ev.by_va = defaultdict(list)
    for sym, va in targets:
        ev.targets[sym].append((va, True, 'src/x.cpp'))
        ev.by_va[va].append((sym, True, 'src/x.cpp'))
    return ev


class ProposeBindingsTests(unittest.TestCase):
    obj = SimpleNamespace(symbols=[])

    def test_own_target_is_matched(self):
        ev = evidence([(SPHERE, 0x4379c0), (CAPSULE, 0x437aa0)])
        self.assertEqual(ev.classify(SPHERE, 0x4379c0, self.obj, set())[0], 'matched')

    def test_target_symbol_implied_elsewhere_is_a_conflict(self):
        # Swapped switch cases: the call site of the capsule test implies the sphere's address.
        ev = evidence([(SPHERE, 0x4379c0), (CAPSULE, 0x437aa0)])
        kind, why = ev.classify(CAPSULE, 0x4379c0, self.obj, set())
        self.assertEqual(kind, 'conflict')
        self.assertIn('0x437aa0', why)

    def test_stand_in_alias_of_a_matched_body_is_accepted(self):
        ev = evidence([('?SetPosition@Emitter@@QAEXXZ', 0x4b8d90)])
        kind, why = ev.classify('?Fn_4b8d90@Stand@@QAEXXZ', 0x4b8d90, self.obj, set())
        self.assertEqual(kind, 'matched')
        self.assertIn('SetPosition', why)

    def test_recorder_leaves_self_call_and_labels_to_the_matcher(self):
        rec = pb._Recorder('?Walk@@YAXPAX@Z')
        self.assertNotIn('?Walk@@YAXPAX@Z', rec)
        self.assertNotIn('$L123', rec)
        self.assertIn('?Other@@YAXXZ', rec)

    def test_decode_real(self):
        self.assertEqual(pb.decode_real('__real@4@3fff8000000000000000'), struct.pack('<f', 1.0))
        self.assertEqual(pb.decode_real('__real@4@3ffdcccccccccccccccd'), struct.pack('<f', 0.4))
        self.assertEqual(pb.decode_real('__real@4@00000000000000000000'), struct.pack('<f', 0.0))
        self.assertIsNone(pb.decode_real('?g_x@@3HA'))


if __name__ == '__main__':
    unittest.main()
