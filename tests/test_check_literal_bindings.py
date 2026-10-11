"""A `__real@` or `??_C@` binding must point at retail bytes that hold the literal its key names."""
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('check_literal_bindings', ROOT / 'tools/check_literal_bindings.py')
crb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(crb)


class FakePE:
    def __init__(self, memory):
        self.memory = memory

    def bytes_at_va(self, va, size):
        return self.memory[va][:size]


class CheckLiteralBindingsTests(unittest.TestCase):
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

    def test_decode_string(self):
        self.assertEqual(crb.decode_string('??_C@_03ABCD@a?5b?$AA@'), (4, b'a b\x00'))
        self.assertEqual(crb.decode_string('??_C@_0CB@FPLI@?6Cannot?5find@'), (33, b'\nCannot find'))
        self.assertIsNone(crb.decode_string('?Other@@YAXXZ'))

    def test_string_length_and_prefix(self):
        memory = {0x560000: b'abc\x00zzzz', 0x560010: b'abcd\x00zzz', 0x560020: b'xbc\x00zzzz'}
        good = {'??_C@_03ABCD@abc?$AA@': '0x00560000'}
        self.assertEqual(self.run_check(good, memory), [])
        too_long = {'??_C@_03ABCD@abc?$AA@': '0x00560010'}  # a longer string shares the prefix
        self.assertEqual(len(self.run_check(too_long, memory)), 1)
        wrong = {'??_C@_03ABCD@abc?$AA@': '0x00560020'}
        self.assertEqual(len(self.run_check(wrong, memory)), 1)

    def test_file_path_literal_is_skipped(self):
        bindings = {'??_C@_0CE@ABCD@Z?3?2home?2user?2x?4cpp?$AA@': '0x0056d3ac'}
        memory = {0x56d3ac: b'D:\\aardvark\\VC\\x.cpp\x00'}
        self.assertEqual(self.run_check(bindings, memory), [])


if __name__ == '__main__':
    unittest.main()
