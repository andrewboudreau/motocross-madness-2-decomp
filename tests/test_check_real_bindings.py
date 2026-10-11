"""A `__real@` binding must point at retail bytes that hold the value its key names."""
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('check_real_bindings', ROOT / 'tools/check_real_bindings.py')
crb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(crb)


class FakePE:
    def __init__(self, memory):
        self.memory = memory

    def bytes_at_va(self, va, size):
        return self.memory[va][:size]


class CheckRealBindingsTests(unittest.TestCase):
    def run_check(self, bindings, memory):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'x.bindings.json'
            path.write_text(json.dumps(bindings), encoding='utf-8')
            return crb.check([path], FakePE(memory))

    def test_matching_float_and_double_pass(self):
        bindings = {'__real@4@4005c800000000000000': '0x00551000',   # 100.0f
                    '__real@8@3fff8000000000000000': '0x00551008',   # 1.0
                    '?Other@@YAXXZ': '0x00401000'}
        memory = {0x551000: struct.pack('<f', 100.0), 0x551008: struct.pack('<d', 1.0)}
        self.assertEqual(self.run_check(bindings, memory), [])

    def test_wrong_constant_is_reported(self):
        bindings = {'__real@4@4005c800000000000000': '0x00551420'}  # 100.0f key, 255.0f there
        memory = {0x551420: struct.pack('<f', 255.0)}
        problems = self.run_check(bindings, memory)
        self.assertEqual(len(problems), 1)
        self.assertIn('255.0', problems[0])
        self.assertIn('100.0', problems[0])


if __name__ == '__main__':
    unittest.main()
