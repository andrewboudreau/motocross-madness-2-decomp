# Decompilation progress

> Last reviewed: **2026-09-30** · Retail executable: `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`

There is not yet a defensible whole-game percentage. The best reproducible
headline is that **388 of 484 selected function targets (80.2%)**
have strict, byte-for-byte matches from readable C++ compiled with VC6 SP3.
That percentage measures the active target set, **not 80.2% of MCM2**:
targets are chosen because they are useful or tractable, and the executable's
complete function inventory has not been established.

## Current indicators

| Indicator | Current value | What it means |
|---|---:|---|
| Strict VC6 exact targets | **388 / 484 (80.2%)** | Unique selected functions reproduced byte-for-byte in the latest reviewed matrix |
| Canonical reconstructed implementation files | **27** | `.cpp` files promoted to `src/reconstructed/`; a file may still contain incomplete classes |
| Canonical reconstructed headers | **40** | Layout and interface declarations, including support-only headers |
| Canonical C++ source lines | **7,419** | Physical lines in the canonical `.cpp` and `.h` files; not a completion percentage |
| Reconstructed files / retail source-path strings | **27 / 111 (24.3%)** | A rough navigation proxy only; paths do not prove TU ownership or completeness |
| Recovered RTTI types | **252** | Confirmed type descriptors, not necessarily reconstructed classes |
| Retail `.text` virtual size | **1,368,518 bytes (1.31 MiB)** | Broad code-section denominator; includes library code, thunks and padding |

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

The VC6 figures are unique, selected function targets from the latest reviewed profile-matrix run. They are not a census of every retail function. The .text size includes third-party libraries, CRT code, thunks, padding, and other non-game code.
