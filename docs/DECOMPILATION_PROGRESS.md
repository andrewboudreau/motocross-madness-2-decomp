# Decompilation progress

> Last reviewed: **2026-10-02** · Retail executable: `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`

There is not yet a defensible whole-game percentage. The best reproducible
headline is that **483 of 483 selected function targets (100.0%)**
have strict, byte-for-byte matches in the reviewed calibration suite, compiled
with VC6 SP3. The suite contains **487 cases**; repeated
retail address/extent pairs count once.
That percentage measures the active target set, **not 100.0% of MCM2**:
targets are chosen because they are useful or tractable, and the executable's
complete function inventory has not been established.

## Current indicators

| Indicator | Current value | What it means |
|---|---:|---|
| Strict VC6 exact targets | **483 / 483 (100.0%)** | Unique retail address/extent pairs in the reviewed calibration run |
| Canonical reconstructed implementation files | **45** | `.cpp` files under `src/reconstructed/` and `src/krusty2/`; may include incomplete candidates |
| Canonical reconstructed headers | **66** | Layout and interface declarations, including support-only headers |
| Canonical C++ source lines | **17,145** | Physical lines in the canonical `.cpp` and `.h` files; not a completion percentage |
| Retail source-path strings | **111** | Navigation evidence; reconstructed files are not one-to-one with original TUs |
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
supported relocations are resolved. This snapshot covers `tools/run_calibration.py`
under `vc6_o2_mt`. The separate manual suite, generated probes and
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
python3 tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

Inspect every result: the runner reports nonmatches as data and its exit status
alone does not prove strict matching. Reviewed code revision:
`365016d7f96549bafec0973ebcdf596f4a8a68eb`. Source inventory counts reflect the current tree
and do not imply every body in those files matches.

### Caveats recorded with the snapshot

483 unique retail address/extent pairs passed strict comparison in 487 calibration cases using authentic VC6 SP3 on Windows. Every relocation was resolved, or the function had no relocation bytes. This excludes the physics runner, whose historical exact label masks relocations, and is not a census of retail functions. RTTI/source-path and .text figures were regenerated from the hash-pinned executable.
