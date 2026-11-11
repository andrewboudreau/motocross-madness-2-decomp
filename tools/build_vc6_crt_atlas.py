#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.crt_atlas import (
    attribute_span,
    group_by_address,
    library_sections,
    scan_library,
    summarize,
    summarize_span,
)
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, infer_vc98_root

EXPECTED_EXE_SHA = (
    "31fde4cc686a5ee89ef9095b90235325"
    "b195596867ecacefe511263e1509b874"
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def render(report: dict) -> str:
    multithread = report["libcmt"]["summary"]
    single_thread = report["libc_control"]["summary"]
    variants = report["variant_comparison"]
    lines = [
        "# VC6 CRT atlas",
        "",
        f"Input SHA-256: `{report['input_sha256']}`",
        "",
        (
            "This is a conservative static source-of-truth atlas built "
            "from the pinned VC6 libraries. A row is accepted only when "
            "at least 20 non-relocation bytes match, the longest contiguous "
            "non-relocation anchor is at least 12 bytes, and the masked "
            "body has exactly one hit in MCM2 executable code."
        ),
        "",
        "## Multithread LIBCMT",
        "",
        (
            "- Unique matched function addresses: "
            f"**{multithread['unique_addresses']}**"
        ),
        (
            "- Union of matched function bodies: "
            f"**{multithread['matched_body_bytes']:,} bytes**"
        ),
        (
            "- Directly compared non-relocation bytes: "
            f"**{multithread['directly_compared_bytes']:,} bytes**"
        ),
        (
            f"- Observed span: `{multithread['span_start']}` .. "
            f"`{multithread['span_end']}` "
            f"({multithread['span_bytes']:,} bytes)"
        ),
        (
            "- Matched-body coverage of that span: "
            f"**{multithread['matched_body_coverage_percent']:.4f}%**"
        ),
        "",
        "## Single-thread control",
        "",
        (
            "The same conservative scan against `LIBC.LIB` finds "
            f"{single_thread['unique_addresses']} addresses. "
            f"{variants['same_address_count']} of the LIBCMT addresses "
            "also have a single-thread match at the same retail address; "
            f"**{variants['libcmt_only_address_count']} addresses do not**."
        ),
        "",
        (
            "This control does not mean all common rows were linked from "
            "LIBC.LIB. It shows that a large common CRT core is shared while "
            "a material set of retail routines requires the multithread "
            "variant. The fully relocation-resolved allocator proof remains "
            "the strongest specific LIBCMT identification."
        ),
        "",
        "## Scope",
        "",
        (
            "The atlas rows label only individually matched function bodies. "
            "They do **not** label every gap in the surrounding address span "
            "as CRT. The span attribution below places whole library "
            "sections in those gaps, each with its evidence tier; a byte it "
            "cannot explain stays unexplained."
        ),
        "",
        (
            "This evidence is useful for provenance and work-queue filtering; "
            "it is not a decompilation progress metric and does not recover "
            "the original per-translation-unit compile flags."
        ),
        "",
    ]
    span = report.get("span_attribution")
    if span:
        gaps = span["summary_in_atlas_gaps"]
        lines += [
            "## Span attribution",
            "",
            (
                "Whole COFF code sections of "
                f"{', '.join(span['libraries'])} placed over the span, "
                "relocation-masked. Tier 1: the section holds an atlas row, "
                "or a relocation in already placed library code decodes to "
                "its symbol. Tier 2: the section matches inside a gap without "
                "a naming reference, or int3 fill reaches the next section's "
                "COFF alignment. Tier 3: a short section accepted only because "
                "it fills a gap exactly."
            ),
            "",
            f"Bytes inside the {gaps['gap_bytes']:,} gap bytes between atlas "
            "rows:",
            "",
            "| Kind | Tier | Ranges | Bytes |",
            "|---|---:|---:|---:|",
        ]
        for kind, row in sorted(
            gaps["by_kind"].items(), key=lambda item: item[1]["tier"]
        ):
            lines.append(
                f"| `{kind}` | {row['tier']} | {row['ranges']} | "
                f"{row['bytes']:,} |"
            )
        lines += [
            "",
            (
                f"Unexplained bytes: **{gaps['unexplained_bytes']}**, ranges: "
                f"{gaps['unexplained_ranges']}"
            ),
        ]
        for row in span["unexplained"]:
            lines.append(f"- `0x{row['start']:08x}` .. `0x{row['end']:08x}`")
        lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Build a conservative address atlas from the pinned VC6 CRT "
            "libraries."
        )
    )
    parser.add_argument("--vc6-root", default=os.environ.get("VC6_ROOT"))
    parser.add_argument(
        "--exe",
        type=Path,
        default=Path(os.environ.get("MCM2_EXE", "work/game/mcm2.exe")),
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("work/vc6-crt-atlas"),
    )
    parser.add_argument("--minimum-anchor", type=int, default=12)
    parser.add_argument("--minimum-compared", type=int, default=20)
    parser.add_argument(
        "--span-libs",
        nargs="*",
        default=["LIBCMT.LIB"],
        help=(
            "libraries whose whole code sections may fill the gaps of the "
            "LIBCMT span, in priority order"
        ),
    )
    args = parser.parse_args()

    if not args.vc6_root:
        raise SystemExit("set VC6_ROOT or pass --vc6-root")

    exe = args.exe.expanduser().resolve()
    vc6_root = Path(args.vc6_root).expanduser().resolve()
    if sha256_file(exe) != EXPECTED_EXE_SHA:
        raise SystemExit("unexpected MCM2 executable hash")

    vc98 = infer_vc98_root(vc6_root)
    lib_dir = find_child_ci(vc98, "Lib")
    if not lib_dir:
        raise SystemExit("VC98/Lib not found")
    libcmt = find_child_ci(lib_dir, "LIBCMT.LIB")
    libc = find_child_ci(lib_dir, "LIBC.LIB")
    if not libcmt or not libc:
        raise SystemExit("LIBCMT.LIB and LIBC.LIB are required")

    pinned = json.loads(
        (ROOT / "config/private_bundle_expected.json").read_text()
    )
    pinned_files = pinned.get("core_files", {})
    for relative, path in (
        ("VC98/LIB/LIBCMT.LIB", libcmt),
        ("VC98/LIB/LIBC.LIB", libc),
    ):
        expected = pinned_files.get(relative)
        if expected and sha256_file(path) != expected:
            raise SystemExit(f"pinned hash mismatch: {relative}")

    pe = PEImage(exe)
    multithread = group_by_address(
        scan_library(
            libcmt,
            pe,
            args.minimum_anchor,
            args.minimum_compared,
        )
    )
    single_thread = group_by_address(
        scan_library(
            libc,
            pe,
            args.minimum_anchor,
            args.minimum_compared,
        )
    )
    span_result = None
    if multithread:
        text = next(s for s in pe.sections if s.name == ".text")
        text_base = pe.image_base + text.virtual_address
        text_data = pe.data[
            text.raw_offset : text.raw_offset + text.virtual_size
        ]
        summary = summarize(multithread)
        span = (int(summary["span_start"], 16), int(summary["span_end"], 16))
        candidates = []
        for name in args.span_libs:
            path = find_child_ci(lib_dir, name)
            if path:
                candidates += library_sections(path, name)
        span_result = attribute_span(
            text_data,
            text_base,
            span[0],
            span[1],
            candidates,
            multithread,
        )
        atlas_bytes = bytearray(span[1] - span[0])
        for row in multithread:
            lo = max(row["target_va"], span[0]) - span[0]
            hi = min(row["target_va"] + row["size"], span[1]) - span[0]
            atlas_bytes[lo:hi] = b"\x01" * (hi - lo)
        gaps = []
        index = 0
        while index < len(atlas_bytes):
            if atlas_bytes[index]:
                index += 1
                continue
            stop = index
            while stop < len(atlas_bytes) and not atlas_bytes[stop]:
                stop += 1
            gaps.append((span[0] + index, span[0] + stop))
            index = stop
        span_result["libraries"] = list(args.span_libs)
        span_result["summary_in_atlas_gaps"] = summarize_span(
            span_result, gaps
        )
        span_result["summary_in_atlas_gaps"]["gap_bytes"] = sum(
            hi - lo for lo, hi in gaps
        )

    multithread_addresses = {row["target_va"] for row in multithread}
    single_thread_addresses = {row["target_va"] for row in single_thread}
    common_addresses = multithread_addresses & single_thread_addresses

    report = {
        "schema_version": 1,
        "input_sha256": EXPECTED_EXE_SHA,
        "minimum_anchor_bytes": args.minimum_anchor,
        "minimum_compared_bytes": args.minimum_compared,
        "libcmt": {
            "sha256": sha256_file(libcmt),
            "summary": summarize(multithread),
            "matches": multithread,
        },
        "libc_control": {
            "sha256": sha256_file(libc),
            "summary": summarize(single_thread),
            "matches": single_thread,
        },
        "variant_comparison": {
            "same_address_count": len(common_addresses),
            "libcmt_only_address_count": (
                len(multithread_addresses) - len(common_addresses)
            ),
        },
        "span_attribution": span_result,
        "claims": {
            "whole_span_is_crt": False,
            "remaining_gaps_classified": bool(
                span_result and not span_result["unexplained"]
            ),
            "compiler_flags_recovered": False,
        },
    }

    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "atlas.json").write_text(
        json.dumps(report, indent=2) + "\n"
    )
    (args.out / "REPORT.md").write_text(render(report))
    print(render(report))


if __name__ == "__main__":
    main()
