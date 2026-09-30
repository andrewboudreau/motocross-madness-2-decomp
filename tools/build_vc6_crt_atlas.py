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

from mcm2tool.crt_atlas import group_by_address, scan_library, summarize
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
            "The atlas labels only individually matched function bodies. "
            "It does **not** label every gap in the surrounding address span "
            "as CRT. Gaps can contain alignment, import thunks, short or "
            "ambiguous runtime functions, or unrelated code and remain "
            "unclassified until separately proven."
        ),
        "",
        (
            "This evidence is useful for provenance and work-queue filtering; "
            "it is not a decompilation progress metric and does not recover "
            "the original per-translation-unit compile flags."
        ),
        "",
    ]
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
        "claims": {
            "whole_span_is_crt": False,
            "remaining_gaps_classified": False,
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
