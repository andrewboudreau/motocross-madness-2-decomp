from __future__ import annotations

import unittest

from mcm2tool.crt_atlas import (
    CodeRegion,
    find_masked_hits,
    group_by_address,
    longest_unmasked_run,
    masked_equal,
    summarize,
)


class CrtAtlasTests(unittest.TestCase):
    def test_longest_unmasked_run(self):
        self.assertEqual(
            longest_unmasked_run(bytearray([0, 0, 1, 0, 0, 0, 1])),
            (3, 3),
        )
        self.assertEqual(
            longest_unmasked_run(bytearray([0, 0, 0])),
            (0, 3),
        )
        self.assertEqual(
            longest_unmasked_run(bytearray([1, 1])),
            (0, 0),
        )

    def test_masked_search_reports_ambiguity(self):
        raw = b"ABCDEFGHIJ"
        mask = bytearray(len(raw))
        mask[4:6] = b"\x01\x01"
        regions = [
            CodeRegion(
                ".text",
                0x401000,
                b"xxABCDzzGHIJyyABCD11GHIJ",
            )
        ]
        hits = find_masked_hits(regions, raw, mask, 4)
        self.assertEqual(
            hits,
            [
                (".text", 0x401002),
                (".text", 0x40100E),
            ],
        )
        self.assertTrue(masked_equal(raw, b"ABCDxxGHIJ", mask))
        self.assertFalse(masked_equal(raw, b"ABCDxxGHXJ", mask))

    def test_group_and_summary_are_address_based(self):
        rows = [
            {
                "archive_member": "a",
                "symbol": "alias",
                "size": 10,
                "relocations": 0,
                "relocation_bytes": 0,
                "compared_bytes": 10,
                "anchor_offset": 0,
                "anchor_bytes": 10,
                "section": ".text",
                "target_va": 0x1000,
            },
            {
                "archive_member": "b",
                "symbol": "better",
                "size": 12,
                "relocations": 1,
                "relocation_bytes": 4,
                "compared_bytes": 8,
                "anchor_offset": 0,
                "anchor_bytes": 8,
                "section": ".text",
                "target_va": 0x1000,
            },
            {
                "archive_member": "c",
                "symbol": "next",
                "size": 5,
                "relocations": 0,
                "relocation_bytes": 0,
                "compared_bytes": 5,
                "anchor_offset": 0,
                "anchor_bytes": 5,
                "section": ".text",
                "target_va": 0x100A,
            },
        ]
        grouped = group_by_address(rows)
        self.assertEqual(len(grouped), 2)
        self.assertEqual(grouped[0]["symbol"], "alias")
        self.assertEqual(grouped[0]["aliases"], ["better"])
        summary = summarize(grouped)
        self.assertEqual(summary["unique_addresses"], 2)
        self.assertEqual(summary["matched_body_bytes"], 15)
        self.assertEqual(summary["span_start"], "0x00001000")
        self.assertEqual(summary["span_end"], "0x0000100f")
        self.assertEqual(summary["overlapping_selected_ranges"], 0)


if __name__ == "__main__":
    unittest.main()
