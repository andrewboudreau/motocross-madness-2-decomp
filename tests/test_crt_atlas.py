from __future__ import annotations

import unittest

from mcm2tool.crt_atlas import (
    DIR32,
    REL32,
    CodeRegion,
    LibrarySection,
    attribute_span,
    find_masked_hits,
    group_by_address,
    longest_unmasked_run,
    masked_equal,
    relocation_target,
    summarize,
    summarize_span,
)


def section(member, symbol, raw, relocations=(), alignment=1):
    """Synthetic COFF code section of library ``L`` defining ``symbol``."""
    mask = bytearray(len(raw))
    for offset, _, _, _ in relocations:
        mask[offset : offset + 4] = b"\x01" * 4
    return LibrarySection(
        library="L",
        member=member,
        index=1,
        name=".text",
        raw=bytes(raw),
        mask=mask,
        relocations=list(relocations),
        symbols=[(symbol, 0, 2)],
        alignment=alignment,
    )


def rel32(field_va, target):
    return (target - field_va - 4).to_bytes(4, "little", signed=True)


BASE = 0x1000
# 0x1000 A  call _b                   atlas row
# 0x1010 C  same bytes as B           nothing names it
# 0x1024 E  ret                       one byte, fills its gap exactly
# 0x1025 B  same bytes as C           A's call decodes to it
# 0x1039 int3 x7                      up to F's 16-byte alignment
# 0x1040 F                            atlas row
# 0x1050 D  call _b, decodes to 0x1010: contradicts B, stays unexplained
A_TAIL = bytes(range(0x40, 0x4B))
BC = bytes(range(0x60, 0x74))
F_BYTES = bytes(range(0xA0, 0xB0))
D_TAIL = bytes(range(0x80, 0x8B))
IMAGE = (
    b"\xe8" + rel32(0x1001, 0x1025) + A_TAIL
    + BC
    + b"\xc3"
    + BC
    + b"\xcc" * 7
    + F_BYTES
    + b"\xe8" + rel32(0x1051, 0x1010) + D_TAIL
)
SECTIONS = [
    section("a.obj", "_a", b"\xe8\0\0\0\0" + A_TAIL, [(1, REL32, "_b", 0)]),
    section("b.obj", "_b", BC),
    section("c.obj", "_c", BC),
    section("d.obj", "_d", b"\xe8\0\0\0\0" + D_TAIL, [(1, REL32, "_b", 0)]),
    section("a.obj", "_e", b"\xc3"),
    section("f.obj", "_f", F_BYTES, alignment=16),
]
SEEDS = [
    {"archive_member": "a.obj", "symbol": "_a", "target_va": 0x1000},
    {"archive_member": "f.obj", "symbol": "_f", "target_va": 0x1040},
]


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


class SpanAttributionTests(unittest.TestCase):
    def setUp(self):
        self.assertEqual(len(IMAGE), 0x60)
        self.result = attribute_span(
            IMAGE, BASE, 0x1000, 0x1060, SECTIONS, SEEDS, seed_library="L"
        )
        self.by_start = {
            row["start"]: row for row in self.result["placements"]
        }

    def test_relocation_target(self):
        self.assertEqual(relocation_target(REL32, 0x1001, 0x20, 0), 0x1025)
        self.assertEqual(relocation_target(REL32, 0x1051, -0x45, 0), 0x1010)
        self.assertEqual(
            relocation_target(DIR32, 0x2000, 0x401008, 8), 0x401000
        )
        self.assertIsNone(relocation_target(0x0A, 0x2000, 0, 0))

    def test_atlas_rows_place_their_sections(self):
        for start, symbol in ((0x1000, "_a"), (0x1040, "_f")):
            row = self.by_start[start]
            self.assertEqual(
                (row["symbol"], row["kind"], row["tier"]),
                (symbol, "atlas_section", 1),
            )

    def test_relocation_names_one_of_two_identical_sections(self):
        named = self.by_start[0x1025]
        self.assertEqual(
            (named["symbol"], named["kind"], named["tier"]),
            ("_b", "relocation_named", 1),
        )
        filled = self.by_start[0x1010]
        self.assertEqual(
            (filled["symbol"], filled["kind"], filled["tier"]),
            ("_c", "gap_fill", 2),
        )

    def test_weak_section_only_fills_an_exact_gap(self):
        row = self.by_start[0x1024]
        self.assertEqual(
            (row["symbol"], row["kind"], row["tier"]),
            ("_e", "member_adjacent", 3),
        )
        # the same one-byte section is never placed inside a larger gap
        image = IMAGE[:0x24] + b"\xc3\x90" + IMAGE[0x26:]
        result = attribute_span(
            image, BASE, 0x1000, 0x1060, SECTIONS, SEEDS, seed_library="L"
        )
        self.assertNotIn(
            "_e", [row["symbol"] for row in result["placements"]]
        )

    def test_contradicting_relocation_rejects_a_match(self):
        self.assertNotIn(0x1050, self.by_start)
        self.assertEqual(
            self.result["unexplained"],
            [{"start": 0x1050, "end": 0x1060, "size": 16}],
        )
        self.assertEqual(self.result["symbol_addresses"]["_b"], [0x1025])

    def test_int3_up_to_the_next_alignment_is_padding(self):
        self.assertEqual(
            [(row["start"], row["end"], row["tier"])
             for row in self.result["padding"]],
            [(0x1039, 0x1040, 2)],
        )
        # not aligned: the run before B (alignment 1) is no padding
        self.assertNotIn(0x1024, [row["start"] for row in self.result["padding"]])

    def test_summary_counts_only_bytes_inside_the_gaps(self):
        summary = summarize_span(self.result, [(0x1010, 0x1040)])
        self.assertEqual(
            {kind: row["bytes"] for kind, row in summary["by_kind"].items()},
            {
                "atlas_section": 0,
                "relocation_named": 20,
                "gap_fill": 20,
                "member_adjacent (tier 3)": 1,
                "alignment_padding": 7,
            },
        )
        self.assertEqual(
            summary["by_member"],
            {"L:a.obj": 1, "L:b.obj": 20, "L:c.obj": 20},
        )
        self.assertEqual(summary["unexplained_bytes"], 16)


if __name__ == "__main__":
    unittest.main()
