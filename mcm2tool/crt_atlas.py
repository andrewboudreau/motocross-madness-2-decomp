from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import tempfile

from .coff import CoffError, CoffObject, relocation_mask
from .coff_archive import read_archive
from .pe import PEImage


@dataclass(frozen=True)
class CodeRegion:
    name: str
    va: int
    data: bytes


def executable_regions(pe: PEImage) -> list[CodeRegion]:
    regions = []
    for section in pe.sections:
        if not (
            section.characteristics & 0x20
            or section.characteristics & 0x20000000
        ):
            continue
        data = pe.data[
            section.raw_offset : section.raw_offset + section.raw_size
        ]
        regions.append(
            CodeRegion(
                section.name,
                pe.image_base + section.virtual_address,
                data,
            )
        )
    return regions


def longest_unmasked_run(mask: bytes | bytearray) -> tuple[int, int]:
    best_start = best_len = 0
    start = 0
    for index in range(len(mask) + 1):
        blocked = index == len(mask) or bool(mask[index])
        if not blocked:
            continue
        if index - start > best_len:
            best_start, best_len = start, index - start
        start = index + 1
    return best_start, best_len


def masked_equal(
    candidate: bytes,
    target: bytes,
    mask: bytes | bytearray,
) -> bool:
    return (
        len(candidate) == len(target) == len(mask)
        and all(
            mask[index] or candidate[index] == target[index]
            for index in range(len(mask))
        )
    )


def find_masked_hits(
    regions: list[CodeRegion],
    raw: bytes,
    mask: bytes | bytearray,
    minimum_anchor: int,
) -> list[tuple[str, int]]:
    anchor_start, anchor_len = longest_unmasked_run(mask)
    if anchor_len < minimum_anchor:
        return []
    anchor = raw[anchor_start : anchor_start + anchor_len]
    hits = []
    for region in regions:
        position = region.data.find(anchor)
        while position >= 0:
            candidate_start = position - anchor_start
            if (
                candidate_start >= 0
                and candidate_start + len(raw) <= len(region.data)
            ):
                target = region.data[
                    candidate_start : candidate_start + len(raw)
                ]
                if masked_equal(raw, target, mask):
                    hits.append(
                        (
                            region.name,
                            region.va + candidate_start,
                        )
                    )
            position = region.data.find(anchor, position + 1)
    return hits


def scan_library(
    library: Path,
    pe: PEImage,
    minimum_anchor: int = 12,
    minimum_compared: int = 20,
) -> list[dict]:
    regions = executable_regions(pe)
    matches = []
    with tempfile.TemporaryDirectory(prefix="mcm2-crt-atlas-") as temp_dir:
        temporary_object = Path(temp_dir) / "member.obj"
        for member in read_archive(library):
            temporary_object.write_bytes(member.data)
            try:
                obj = CoffObject(temporary_object)
            except (CoffError, ValueError, IndexError):
                continue
            if obj.machine != 0x14C:
                continue
            for symbol in obj.symbols:
                if (
                    symbol.section_number <= 0
                    or symbol.storage_class not in (2, 3, 105)
                    or symbol.name.startswith(".")
                ):
                    continue
                try:
                    section = obj.section(symbol.section_number)
                    if not (
                        section.characteristics & 0x20
                        or section.characteristics & 0x20000000
                    ):
                        continue
                    raw, _, relocations = obj.symbol_extent(symbol)
                except (CoffError, ValueError, IndexError):
                    continue
                if not raw:
                    continue
                mask = relocation_mask(
                    obj,
                    symbol,
                    len(raw),
                    relocations,
                )
                compared = len(raw) - sum(mask)
                anchor_start, anchor_len = longest_unmasked_run(mask)
                if (
                    compared < minimum_compared
                    or anchor_len < minimum_anchor
                ):
                    continue
                hits = find_masked_hits(
                    regions,
                    raw,
                    mask,
                    minimum_anchor,
                )
                if len(hits) != 1:
                    continue
                matches.append(
                    {
                        "archive_member": member.name,
                        "symbol": symbol.name,
                        "size": len(raw),
                        "relocations": len(relocations),
                        "relocation_bytes": sum(mask),
                        "compared_bytes": compared,
                        "anchor_offset": anchor_start,
                        "anchor_bytes": anchor_len,
                        "section": hits[0][0],
                        "target_va": hits[0][1],
                    }
                )
    return matches


def group_by_address(matches: list[dict]) -> list[dict]:
    grouped = {}
    for row in matches:
        grouped.setdefault(row["target_va"], []).append(row)

    results = []
    for _, rows in sorted(grouped.items()):
        primary = max(
            rows,
            key=lambda row: (
                row["compared_bytes"],
                row["size"],
                row["anchor_bytes"],
                -len(row["symbol"]),
            ),
        )
        aliases = sorted(
            {
                row["symbol"]
                for row in rows
                if row["symbol"] != primary["symbol"]
            }
        )
        results.append(
            {
                **primary,
                "aliases": aliases,
                "matching_archive_entries": len(rows),
            }
        )
    return results


def summarize(rows: list[dict]) -> dict:
    if not rows:
        return {
            "unique_addresses": 0,
            "matched_body_bytes": 0,
            "directly_compared_bytes": 0,
            "span_start": None,
            "span_end": None,
            "span_bytes": 0,
            "matched_body_coverage_percent": 0.0,
            "overlapping_selected_ranges": 0,
        }

    intervals = sorted(
        (
            row["target_va"],
            row["target_va"] + row["size"],
        )
        for row in rows
    )
    union = []
    overlaps = 0
    for start, end in intervals:
        if union and start < union[-1][1]:
            overlaps += 1
        if not union or start > union[-1][1]:
            union.append([start, end])
        else:
            union[-1][1] = max(union[-1][1], end)

    body_bytes = sum(end - start for start, end in union)
    span_start = intervals[0][0]
    span_end = max(end for _, end in intervals)
    span_bytes = span_end - span_start
    return {
        "unique_addresses": len(rows),
        "matched_body_bytes": body_bytes,
        "directly_compared_bytes": sum(
            row["compared_bytes"] for row in rows
        ),
        "span_start": f"0x{span_start:08x}",
        "span_end": f"0x{span_end:08x}",
        "span_bytes": span_bytes,
        "matched_body_coverage_percent": (
            round(100 * body_bytes / span_bytes, 4)
            if span_bytes
            else 100.0
        ),
        "overlapping_selected_ranges": overlaps,
    }
