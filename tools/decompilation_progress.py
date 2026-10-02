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
    source_root = root / "src" / "reconstructed"
    cpp_files = sorted(source_root.glob("*.cpp"))
    header_files = sorted(source_root.glob("*.h"))
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
    exact_percent = exact * 100 / selected
    unit_percent = counts["cpp_files"] * 100 / int(config["retail_source_paths"])
    text_mib = int(config["retail_text_bytes"]) / (1024 * 1024)
    return f"""# Decompilation progress

> Last reviewed: **{config['as_of']}** · Retail executable: `{config['retail_sha256']}`

There is not yet a defensible whole-game percentage. The best reproducible
headline is that **{exact} of {selected} selected function targets ({exact_percent:.1f}%)**
have strict, byte-for-byte matches from readable C++ compiled with VC6 SP3.
That percentage measures the active target set, **not {exact_percent:.1f}% of MCM2**:
targets are chosen because they are useful or tractable, and the executable's
complete function inventory has not been established.

## Current indicators

| Indicator | Current value | What it means |
|---|---:|---|
| Strict VC6 exact targets | **{exact} / {selected} ({exact_percent:.1f}%)** | Unique selected functions reproduced byte-for-byte in the latest reviewed matrix |
| Canonical reconstructed implementation files | **{counts['cpp_files']}** | `.cpp` files promoted to `src/reconstructed/`; a file may still contain incomplete classes |
| Canonical reconstructed headers | **{counts['header_files']}** | Layout and interface declarations, including support-only headers |
| Canonical C++ source lines | **{counts['source_lines']:,}** | Physical lines in the canonical `.cpp` and `.h` files; not a completion percentage |
| Reconstructed files / retail source-path strings | **{counts['cpp_files']} / {config['retail_source_paths']} ({unit_percent:.1f}%)** | A rough navigation proxy only; paths do not prove TU ownership or completeness |
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
supported relocations are resolved. Near matches, clang-only checks, generated
accessor probes, skeletons, and semantic-only reconstructions do not qualify.
See [the match contract](VC6_MATCHING.md#match-contract) for details.

## Updating this page

After a reviewed VC6 profile-matrix run, update
`config/decompilation_progress.json`, then run:

```bash
make progress
make progress-check
```

The repository-check GitHub Action runs `make progress-check`, so changes to
canonical reconstructed sources or the snapshot cannot silently leave this page
stale. The action uses no proprietary executable or compiler; the reviewed VC6
numbers remain an explicit checked-in snapshot.

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
