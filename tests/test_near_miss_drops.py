"""near_miss_drops compares per-row strict bytes; synthetic gate outputs only, no VC6."""
import json
import tempfile
import unittest
from pathlib import Path

from tools.near_miss_drops import drops, main, strict_bytes


def physics_row(va, matched):
    return {"target_va": va, "strict_match": {"matching_positions": matched}}


def calibration_case(va, matched):
    return {"target_va": va, "symbol": "?F@@YAXXZ", "result": physics_row(va, matched)}


class NearMissDropsTest(unittest.TestCase):
    def write(self, name, rows):
        path = Path(self.dir.name) / name
        path.write_text(json.dumps(rows))
        return str(path)

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.dir.cleanup()

    def test_reads_both_formats_and_keeps_best_case(self):
        path = self.write("cal.json", [calibration_case("0x00401000", 10), calibration_case("0x00401000", 12),
                                       physics_row("0x00402000", 7)])
        self.assertEqual(strict_bytes(path), {"0x00401000": 12, "0x00402000": 7})

    def test_reports_lost_bytes_and_missing_rows_only(self):
        base = {"0x1": 10, "0x2": 20, "0x3": 30}
        new = {"0x1": 11, "0x2": 19}
        self.assertEqual(drops(base, new), [("0x2", 20, 19), ("0x3", 30, None)])

    def test_exit_status(self):
        base = self.write("base.json", [physics_row("0x1", 5)])
        same = self.write("same.json", [physics_row("0x1", 6)])
        worse = self.write("worse.json", [physics_row("0x1", 4)])
        self.assertEqual(main(["x", base, same]), 0)
        self.assertEqual(main(["x", base, worse]), 1)


if __name__ == "__main__":
    unittest.main()
