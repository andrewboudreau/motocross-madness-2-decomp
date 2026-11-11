from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import tempfile

from .coff import RELOC_WIDTH_I386, CoffError, CoffObject, relocation_mask
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


# --------------------------------------------------------------------------
# Span attribution: whole library sections placed inside the atlas span.
#
# The atlas above admits single function bodies that are long and unique in
# the executable. VC6 LIBCMT is compiled with function-level linking, so most
# members are many COMDAT sections, and short or byte-identical sections
# (the ``_fread``/``_fwrite`` wrappers, ``memcpy``/``memmove``) never become
# unique rows. ``attribute_span`` places whole COFF code sections and records
# the evidence tier of each placement:
#
#   1 ``atlas_section``     the section holding an atlas row matches whole,
#                           relocation-masked, at the address the row implies
#   1 ``relocation_named``  a relocation in already placed library code names
#                           the section's symbol, the retail rel32/dir32 field
#                           decodes to that address, and the section matches
#                           there relocation-masked
#   2 ``member_adjacent``   the section matches inside a remaining gap and its
#                           member is already linked; no reference names it
#   2 ``gap_fill``          the same without a linked member
#   3 (either of the two)   too few compared bytes on their own; accepted only
#                           because the section fills a gap exactly
#   2 ``alignment_padding`` int3 fill before a placed section whose start is
#                           aligned to that section's COFF alignment
#
# A placement is rejected when one of its own relocations names a symbol whose
# address is already known and the retail field decodes to another address.

REL32 = 0x14
DIR32 = 0x06
TIER = {
    "atlas_section": 1,
    "relocation_named": 1,
    "member_adjacent": 2,
    "gap_fill": 2,
    "alignment_padding": 2,
}


@dataclass
class LibrarySection:
    library: str
    member: str
    index: int
    name: str
    raw: bytes
    mask: bytearray
    # (offset, type, key, addend); key from ``symbol_key`` or None for
    # section symbols, whose target address says nothing about a name.
    relocations: list
    # (name, offset, storage_class) of the symbols defined in this section
    symbols: list
    alignment: int = 1

    @property
    def label(self) -> str:
        names = [name for name, _, storage in self.symbols if storage == 2]
        names = names or [name for name, _, _ in self.symbols]
        return names[0] if names else f"{self.name}#{self.index}"

    @property
    def compared_bytes(self) -> int:
        return len(self.raw) - sum(self.mask)


def symbol_key(library: str, member: str, name: str, storage_class: int):
    """External symbols by name; statics only within their own member."""
    return name if storage_class == 2 else (library, member, name)


def _named(symbol) -> bool:
    return (
        symbol.storage_class in (2, 3, 6, 105)
        and not symbol.name.startswith((".", "$"))
    )


def sections_from_object(
    obj: CoffObject,
    library: str,
    member: str,
) -> list[LibrarySection]:
    out = []
    for sec in obj.sections:
        if not sec.characteristics & 0x20 or sec.raw_size == 0:
            continue
        raw = obj.data[sec.raw_ptr : sec.raw_ptr + sec.raw_size]
        mask = bytearray(len(raw))
        relocations = []
        for rel in obj.relocations:
            if rel.section_number != sec.index:
                continue
            width = RELOC_WIDTH_I386.get(rel.type, 4)
            stop = min(len(raw), rel.virtual_address + width)
            for index in range(rel.virtual_address, stop):
                mask[index] = 1
            target = obj.symbol_by_index.get(rel.symbol_index)
            key = None
            if target is not None and _named(target):
                key = symbol_key(
                    library, member, target.name, target.storage_class
                )
            addend = 0
            if rel.virtual_address + 4 <= len(raw):
                addend = int.from_bytes(
                    raw[rel.virtual_address : rel.virtual_address + 4],
                    "little",
                    signed=True,
                )
            relocations.append((rel.virtual_address, rel.type, key, addend))
        symbols = [
            (s.name, s.value, s.storage_class)
            for s in obj.symbols
            if s.section_number == sec.index and _named(s)
        ]
        align_code = (sec.characteristics >> 20) & 0xF
        out.append(
            LibrarySection(
                library=library,
                member=member,
                index=sec.index,
                name=sec.name,
                raw=raw,
                mask=mask,
                relocations=relocations,
                symbols=symbols,
                alignment=(1 << (align_code - 1)) if align_code else 16,
            )
        )
    return out


def library_sections(path: Path, library: str | None = None) -> list:
    library = library or Path(path).name
    out = []
    with tempfile.TemporaryDirectory(prefix="mcm2-crt-span-") as temp_dir:
        temporary_object = Path(temp_dir) / "member.obj"
        for member in read_archive(path):
            if len(member.data) < 20 or member.data[:2] != b"\x4c\x01":
                continue  # import descriptors and short import members
            temporary_object.write_bytes(member.data)
            try:
                obj = CoffObject(temporary_object)
            except (CoffError, ValueError, IndexError):
                continue
            out += sections_from_object(obj, library, member.name)
    return out


def relocation_target(
    rtype: int,
    field_va: int,
    retail_field: int,
    addend: int,
) -> int | None:
    """Symbol address a linked i386 relocation field encodes."""
    if rtype == REL32:
        return (field_va + 4 + retail_field - addend) & 0xFFFFFFFF
    if rtype == DIR32:
        return (retail_field - addend) & 0xFFFFFFFF
    return None


class _SpanState:
    def __init__(self, data, base, start, end, sections, occupied):
        self.data, self.base, self.start, self.end = data, base, start, end
        self.used = bytearray(end - start)
        for lo, hi in occupied:
            for va in range(max(lo, start), min(hi, end)):
                self.used[va - start] = 1
        self.placements: list[dict] = []
        self.placed: dict[int, tuple[LibrarySection, int]] = {}
        self.linked_members: set = set()
        self.addresses: dict = {}
        self.by_symbol: dict = {}
        for section in sections:
            for name, offset, storage in section.symbols:
                key = symbol_key(section.library, section.member, name, storage)
                self.by_symbol.setdefault(key, []).append((section, offset))

    def field(self, va: int) -> int | None:
        offset = va - self.base
        if offset < 0 or offset + 4 > len(self.data):
            return None
        return int.from_bytes(
            self.data[offset : offset + 4], "little", signed=True
        )

    def fits(self, section: LibrarySection, va: int) -> bool:
        stop = va + len(section.raw)
        if id(section) in self.placed or va < self.start or stop > self.end:
            return False
        if any(self.used[va - self.start : stop - self.start]):
            return False
        offset = va - self.base
        return masked_equal(
            section.raw,
            self.data[offset : offset + len(section.raw)],
            section.mask,
        )

    def targets(self, section: LibrarySection, va: int):
        for offset, rtype, key, addend in section.relocations:
            field = self.field(va + offset)
            if key is None or field is None:
                continue
            target = relocation_target(rtype, va + offset, field, addend)
            if target is not None:
                yield key, target

    def checked(self, section: LibrarySection, va: int) -> tuple[int, int]:
        resolved = conflicting = 0
        for key, target in self.targets(section, va):
            known = self.addresses.get(key)
            if known is None:
                continue
            if known == {target}:
                resolved += 1
            else:
                conflicting += 1
        return resolved, conflicting

    def place(self, section, va, kind, evidence, alternatives=()):
        resolved, _ = self.checked(section, va)
        for index in range(va, va + len(section.raw)):
            self.used[index - self.start] = 1
        self.placed[id(section)] = (section, va)
        self.linked_members.add((section.library, section.member))
        self.placements.append(
            {
                "start": va,
                "end": va + len(section.raw),
                "library": section.library,
                "member": section.member,
                "section": section.index,
                "symbol": section.label,
                "symbols": [name for name, _, _ in section.symbols],
                "alignment": section.alignment,
                "kind": kind,
                "tier": TIER[kind],
                "evidence": evidence,
                "compared_bytes": section.compared_bytes,
                "relocations": len(section.relocations),
                "relocations_resolved": resolved,
                "alternatives": sorted(set(alternatives)),
            }
        )
        for name, offset, storage in section.symbols:
            key = symbol_key(section.library, section.member, name, storage)
            self.addresses.setdefault(key, set()).add(va + offset)
        for key, target in self.targets(section, va):
            self.addresses.setdefault(key, set()).add(target)

    def free_runs(self) -> list[tuple[int, int]]:
        runs, index = [], 0
        while index < len(self.used):
            if self.used[index]:
                index += 1
                continue
            stop = index
            while stop < len(self.used) and not self.used[stop]:
                stop += 1
            runs.append((self.start + index, self.start + stop))
            index = stop
        return runs


def _key_name(key) -> str:
    return key if isinstance(key, str) else key[2]


def _place_named(state: _SpanState) -> bool:
    """Step 2: place sections whose symbol a placed relocation decodes to."""
    progress = False
    for key, targets in sorted(
        state.addresses.items(), key=lambda item: str(item[0])
    ):
        if len(targets) != 1:
            continue
        (target,) = targets
        if not state.start <= target < state.end:
            continue
        candidates = []
        for section, offset in state.by_symbol.get(key, []):
            va = target - offset
            if state.fits(section, va) and not state.checked(section, va)[1]:
                candidates.append((section, va))
        if not candidates:
            continue
        linked = [
            candidate
            for candidate in candidates
            if (candidate[0].library, candidate[0].member)
            in state.linked_members
        ]
        section, va = (linked or candidates)[0]
        state.place(
            section,
            va,
            "relocation_named",
            f"relocation to {_key_name(key)} decodes to this address",
            [
                f"{other.library}:{other.member}"
                for other, _ in candidates
                if other is not section
            ],
        )
        progress = True
    return progress


def _fill_gaps(state: _SpanState, sections: list) -> bool:
    """Step 3: place one section that matches whole inside a gap.

    A candidate must not contradict a known address through its relocations
    and needs some evidence of its own: a relocation that resolves to a known
    address, 16 compared bytes, or 4 compared bytes without relocations. A
    candidate without that evidence is accepted only when it fills a gap
    exactly, at tier 3. Across all gaps the candidate with most evidence
    (compared bytes plus 4 per resolved relocation) wins; candidates of the
    same rank at the same address are kept as alternatives; a member that
    already has a placed section breaks ties (``member_adjacent``, otherwise
    ``gap_fill``). Returns after one placement so that relocation naming runs
    first again.
    """
    groups: dict = {}
    for section in sections:
        if id(section) in state.placed or not section.compared_bytes:
            continue
        index = section.mask.index(0)
        groups.setdefault(index, {}).setdefault(
            section.raw[index], []
        ).append(section)
    best = None
    for lo, hi in state.free_runs():
        for va in range(lo, hi):
            for index, by_byte in groups.items():
                offset = va - state.base + index
                if offset >= len(state.data):
                    continue
                for section in by_byte.get(state.data[offset], ()):
                    if va + len(section.raw) > hi or not state.fits(section, va):
                        continue
                    resolved, conflicting = state.checked(section, va)
                    if conflicting:
                        continue
                    compared = section.compared_bytes
                    strong = (
                        resolved > 0
                        or compared >= 16
                        or (not section.relocations and compared >= 4)
                    )
                    exact = va == lo and va + len(section.raw) == hi
                    if not strong and not exact:
                        continue
                    linked = (
                        section.library,
                        section.member,
                    ) in state.linked_members
                    rank = (strong, compared + 4 * resolved, linked, -va)
                    if best is None or rank > best[0]:
                        best = (rank, va, section, [])
                    elif rank == best[0]:
                        best[3].append(section)
    if best is None:
        return False
    (strong, *_), va, section, others = best
    if (section.library, section.member) in state.linked_members:
        kind = "member_adjacent"
        evidence = "section matches in a gap; its member is linked"
    else:
        kind = "gap_fill"
        evidence = "section matches in a gap; no reference names it"
    if not strong:
        evidence += "; weak bytes, accepted because it fills the gap exactly"
    state.place(
        section,
        va,
        kind,
        evidence,
        [f"{s.library}:{s.member}:{s.label}" for s in others],
    )
    if not strong:
        state.placements[-1]["tier"] = 3
    return True


def attribute_span(
    data: bytes,
    base: int,
    start: int,
    end: int,
    sections: list[LibrarySection],
    seeds: list[dict],
    occupied=(),
    seed_library: str = "LIBCMT.LIB",
) -> dict:
    """Place whole library code sections over ``[start, end)``.

    ``data`` is the code image at ``base``; ``sections`` are candidate
    sections in library priority order; ``seeds`` are atlas rows
    (``archive_member``, ``symbol``, ``aliases``, ``target_va``) from
    ``seed_library``; ``occupied`` ranges belong to something else (import
    thunks). Returns placements, alignment padding, the decoded addresses of
    symbols in the span and the bytes still unexplained.
    """
    state = _SpanState(data, base, start, end, sections, occupied)

    # 1. the sections that hold atlas rows
    for row in seeds:
        for name in [row["symbol"], *row.get("aliases", [])]:
            keys = (name, (seed_library, row["archive_member"], name))
            for key in keys:
                for section, offset in state.by_symbol.get(key, []):
                    if (
                        section.library != seed_library
                        or section.member != row["archive_member"]
                    ):
                        continue
                    va = row["target_va"] - offset
                    if state.fits(section, va):
                        state.place(
                            section,
                            va,
                            "atlas_section",
                            "whole section matches at the atlas row",
                        )

    # 2-4, strongest first: whenever a step places something, start again
    # from step 2 so that new relocations can name further sections.
    while _place_named(state) or _fill_gaps(state, sections):
        pass

    # 5. int3 fill up to the alignment of the following placed section

    following = {
        placement["start"]: placement for placement in state.placements
    }
    padding = []
    for lo, hi in state.free_runs():
        offset = lo - base
        if any(byte != 0xCC for byte in data[offset : offset + hi - lo]):
            continue
        after = following.get(hi)
        if after and hi % after["alignment"] == 0 and hi - lo < after["alignment"]:
            padding.append(
                {
                    "start": lo,
                    "end": hi,
                    "kind": "alignment_padding",
                    "tier": TIER["alignment_padding"],
                    "evidence": (
                        f"int3 fill to the {after['alignment']}-byte "
                        f"alignment of {after['member']}"
                    ),
                }
            )
            for index in range(lo, hi):
                state.used[index - start] = 1

    placements = sorted(state.placements, key=lambda row: row["start"])
    return {
        "span": [start, end],
        "placements": placements,
        "padding": padding,
        "unexplained": [
            {"start": lo, "end": hi, "size": hi - lo}
            for lo, hi in state.free_runs()
        ],
        "symbol_addresses": {
            (key if isinstance(key, str) else f"{key[1]}:{key[2]}"): sorted(
                values
            )
            for key, values in state.addresses.items()
            if any(start <= value < end for value in values)
        },
    }


def summarize_span(result: dict, gaps: list[tuple[int, int]] = ()) -> dict:
    """Totals by kind; with ``gaps``, only the bytes inside those ranges."""

    def inside(row) -> int:
        if not gaps:
            return row["end"] - row["start"]
        return sum(
            max(0, min(row["end"], hi) - max(row["start"], lo))
            for lo, hi in gaps
        )

    by_kind: dict = {}
    by_member: dict = {}
    for row in result["placements"] + result["padding"]:
        size = inside(row)
        kind = row["kind"]
        if row["tier"] != TIER[kind]:
            kind = f"{kind} (tier {row['tier']})"
        entry = by_kind.setdefault(
            kind, {"tier": row["tier"], "ranges": 0, "bytes": 0}
        )
        if size:
            entry["ranges"] += 1
            entry["bytes"] += size
        if size and "member" in row:
            name = f"{row['library']}:{row['member'].split(chr(92))[-1]}"
            by_member[name] = by_member.get(name, 0) + size
    return {
        "span_bytes": result["span"][1] - result["span"][0],
        "by_kind": by_kind,
        "by_member": dict(sorted(by_member.items())),
        "unexplained_bytes": sum(r["size"] for r in result["unexplained"]),
        "unexplained_ranges": len(result["unexplained"]),
    }
