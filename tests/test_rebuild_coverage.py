import importlib.util
import json
import struct
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('rebuild_coverage', ROOT / 'tools' / 'rebuild_coverage.py')
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


def fake_decode(chunk, va):
    """Tiny decoder: c3 ret, cc int3, 90 nop, b8 imm32 (5 bytes), anything else 1 byte."""
    op = chunk[0]
    if op == 0xC3:
        return 1, 'ret'
    if op == 0xCC:
        return 1, 'int3'
    if op == 0x90:
        return 1, 'nop'
    if op == 0xB8:
        return 5, 'mov'
    return 1, 'other'


class CoverageMapTests(unittest.TestCase):
    def test_higher_priority_category_wins_regardless_of_order(self):
        cov = MODULE.CoverageMap(0x1000, 0x1010)
        cov.paint(0x1000, 0x1010, 'near_miss')
        cov.paint(0x1004, 0x1008, 'exact_src')
        cov.paint(0x1000, 0x1008, 'padding')  # lower priority: ignored where painted
        self.assertEqual(cov.runs(), [(0x1000, 0x1004, 'near_miss'), (0x1004, 0x1008, 'exact_src'),
                                      (0x1008, 0x1010, 'near_miss')])
        totals = cov.totals()
        self.assertEqual(totals['exact_src'], 4)
        self.assertEqual(totals['near_miss'], 12)
        self.assertEqual(sum(totals.values()), 16)

    def test_paint_is_clipped_to_the_section(self):
        cov = MODULE.CoverageMap(0x1000, 0x1004)
        cov.paint(0x0ff0, 0x2000, 'library_crt_atlas')
        self.assertEqual(cov.runs(), [(0x1000, 0x1004, 'library_crt_atlas')])


class PaddingTests(unittest.TestCase):
    def test_padding_needs_a_terminator_and_an_aligned_end(self):
        base = 0x1000
        # 0x1000: mov eax, 0xcccccc90 ; ret ; int3 x10 -> 0x1010 ; ret ; nop ; then data
        data = bytes([0xB8, 0x90, 0xCC, 0xCC, 0xCC, 0xC3]) + b'\xcc' * 10 + b'\xc3\x90\x41\x42'
        pads = MODULE.padding_runs(data, base, base, base + len(data), fake_decode)
        # the cc/90 bytes inside the mov immediate are not padding; the int3 run is,
        # and the trailing nop is not (it does not end on a 16-byte boundary or at end)
        self.assertEqual(pads, [(0x1006, 0x1010)])

    def test_jump_table_padding_after_out_of_step_decoding(self):
        base = 0x1000
        table = struct.pack('<II', 0x1002, 0x1004)  # dword case targets inside the function
        data = b'\x41' * 4 + table + b'\xcc' * 4
        self.assertEqual(MODULE.jump_table_padding(data, base, base, base + len(data)), [(0x100c, 0x1010)])
        not_table = b'\x41' * 8 + b'\xcc' * 8
        self.assertEqual(MODULE.jump_table_padding(not_table, base, base, base + 16), [])

    def test_split_chunks(self):
        self.assertEqual(MODULE.split_chunks(0, 100, [(10, 16), (40, 48)]), [(0, 10), (16, 40), (48, 100)])
        self.assertEqual(MODULE.split_chunks(0, 16, [(0, 16)]), [])


class ParserTests(unittest.TestCase):
    def test_near_miss_rows(self):
        text = ('| TU | VA | score | class | what differs |\n|---|---|---|---|---|\n'
                '| camera/Camera | `0x0042eb10` | 659/690 | c | operand order |\n'
                '| audio/PCAudio | `0x004bb890` | 35/272 (masked) | b | frame |\n')
        rows = MODULE.parse_near_miss_index(text)
        self.assertEqual([(r['va'], r['class'], r['score_compared']) for r in rows],
                         [(0x42eb10, 'c', 690), (0x4bb890, 'b', 272)])

    def test_class_definitions_ignore_forward_declarations(self):
        text = ('class Forward;\nclass UIControl : public UIBase {\n};\n'
                'struct Vec3\n{\n float x;\n};\ntemplate <class T> class ContainerList {\n};\n')
        self.assertEqual(MODULE.class_definitions(text), {'UIControl', 'Vec3', 'ContainerList'})

    def test_alphabetical_order_is_case_insensitive(self):
        self.assertEqual(MODULE.order_inversions(['AgeManager.cpp', 'bikerace.cpp', 'BlockAllocator.cpp']), 0)
        self.assertEqual(MODULE.order_inversions(['b.cpp', 'A.cpp']), 1)

    def test_nearest_sources_matches_nearest_source_tool_ordering(self):
        rows = MODULE.nearest_sources(0x100, [(0x180, 'b.cpp'), (0x0f0, 'a.cpp'), (0x400, 'c.cpp')], 2)
        self.assertEqual([(r['path'], r['delta']) for r in rows], [('a.cpp', -0x10), ('b.cpp', 0x80)])

    def test_calibration_cases_are_read_without_importing_the_runner(self):
        cases = MODULE.load_calibration_cases()
        self.assertGreater(len(cases), 100)
        self.assertTrue(all('target_va' in c and 'target_size' in c for c in cases))

    def test_targets_resolve_sources_and_default_expectation(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / 'samples' / 'x' / 'targets.json'
            manifest.parent.mkdir(parents=True)
            manifest.write_text(json.dumps([
                {'source': '../y/A.cpp', 'candidate_symbol_contains': 'A', 'target_va': '0x401000',
                 'target_size': 4, 'expect': 'partial'},
                {'candidate_symbol_contains': 'B', 'target_va': '0x401010', 'target_size': 1,
                 'status': 'exact-byte-match'},
            ]))
            (root / 'src').mkdir()
            rows = MODULE.load_targets(root)
        self.assertEqual([(r['source'], r['expect'], r['va']) for r in rows],
                         [('samples/y/A.cpp', 'partial', 0x401000), ('samples/x', 'exact', 0x401010)])
        self.assertEqual(MODULE.source_tree('samples/y/A.cpp'), 'samples')
        self.assertEqual(MODULE.source_tree('src/krusty2/core/A.cpp'), 'src')


if __name__ == '__main__':
    unittest.main()
