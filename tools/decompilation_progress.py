#!/usr/bin/env python3
"""Generate the public, input-free decompilation progress summary."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "config" / "decompilation_progress.json"
OUTPUT = ROOT / "docs" / "DECOMPILATION_PROGRESS.md"


def repository_counts(root: Path) -> dict[str, int]:
    source_roots = [root / "src" / "reconstructed", root / "src" / "krusty2"]
    cpp_files = sorted(p for source_root in source_roots for p in source_root.rglob("*.cpp"))
    header_files = sorted(p for source_root in source_roots for p in source_root.rglob("*.h"))
    source_lines = sum(
        len(path.read_text(encoding="utf-8").splitlines())
        for path in cpp_files + header_files
    )
    return {
        "cpp_files": len(cpp_files),
        "header_files": len(header_files),
        "source_lines": source_lines,
    }


def render(config: dict[str, object], counts: dict[str, int]) -> str:
    exact = int(config["vc6_exact_targets"])
    selected = int(config["vc6_selected_targets"])
    if selected <= 0 or not 0 <= exact <= selected:
        raise ValueError("expected 0 <= exact <= selected and selected > 0")
    exact_percent = exact * 100 / selected
    text_mib = int(config["retail_text_bytes"]) / (1024 * 1024)
    return f"""# Decompilation progress

> Last reviewed: **{config['as_of']}** · Retail executable: `{config['retail_sha256']}`

There is not yet a defensible whole-game percentage. The best reproducible
headline is that **{exact} of {selected} selected function targets ({exact_percent:.1f}%)**
have strict, byte-for-byte matches in the reviewed calibration suite, compiled
with VC6 SP3. The suite contains **{config['vc6_case_count']} cases**; repeated
retail address/extent pairs count once.
That percentage measures the active target set, **not {exact_percent:.1f}% of MCM2**:
targets are chosen because they are useful or tractable, and the executable's
complete function inventory has not been established.

## Current indicators

| Indicator | Current value | What it means |
|---|---:|---|
| Strict VC6 exact targets | **{exact} / {selected} ({exact_percent:.1f}%)** | Unique retail address/extent pairs in the reviewed calibration run |
| Canonical reconstructed implementation files | **{counts['cpp_files']}** | `.cpp` files under `src/reconstructed/` and `src/krusty2/`; may include incomplete candidates |
| Canonical reconstructed headers | **{counts['header_files']}** | Layout and interface declarations, including support-only headers |
| Canonical C++ source lines | **{counts['source_lines']:,}** | Physical lines in the canonical `.cpp` and `.h` files; not a completion percentage |
| Retail source-path strings | **{config['retail_source_paths']}** | Navigation evidence; reconstructed files are not one-to-one with original TUs |
| Recovered RTTI types | **{config['rtti_types']}** | Confirmed type descriptors, not necessarily reconstructed classes |
| Retail `.text` virtual size | **{config['retail_text_bytes']:,} bytes ({text_mib:.2f} MiB)** | Broad code-section denominator; includes library code, thunks and padding |

## Why this is not one percentage

Counting source lines compares newly written readable C++ with optimized machine
code, so it cannot measure completion. Counting classes also overstates progress
when only a few slots are reconstructed. Conversely, dividing matched bytes by
all of `.text` understates game-code progress because the section includes the
CRT and third-party/library code. Until function boundaries and ownership cover
the entire image, the table deliberately keeps these measures separate.

“Exact” means all bytes in a defensible function extent match after independently
supported relocations are resolved. This snapshot covers `tools/run_calibration.py`
under `{config['vc6_profile']}`. The separate manual suite, generated probes and
physics diagnostics are not added to this count. Relocation-masked matches, near
matches, clang-only checks and skeletons do not qualify.
See [the match contract](VC6_MATCHING.md#match-contract) for details.

## Updating this page

After a reviewed strict calibration run, update
`config/decompilation_progress.json`, then run:

```bash
make progress
make progress-check
```

The repository-check GitHub Action runs `make progress-check`, so changes to
canonical reconstructed sources or the snapshot cannot silently leave this page
stale. The action uses no proprietary executable or compiler; the reviewed VC6
numbers remain an explicit checked-in snapshot.

Reproduce the calibration with the [private-input setup](TOOLCHAIN.md):

```bash
python3 tools/run_calibration.py --compiler vc6 --profile {config['vc6_profile']} --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

Inspect every result: the runner reports nonmatches as data and its exit status
alone does not prove strict matching. Reviewed code revision:
`{config['reviewed_commit']}`. Source inventory counts reflect the current tree
and do not imply every body in those files matches.

### Caveats recorded with the snapshot

{config['notes']}
"""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if the document is stale")
    args = parser.parse_args()
    config = json.loads(CONFIG.read_text(encoding="utf-8"))
    content = render(config, repository_counts(ROOT))
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text(encoding="utf-8") != content:
            print(f"{OUTPUT.relative_to(ROOT)} is stale; run: make progress")
            return 1
        print(f"{OUTPUT.relative_to(ROOT)} is up to date")
        return 0
    OUTPUT.write_text(content, encoding="utf-8")
    print(f"wrote {OUTPUT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
