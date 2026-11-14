# Decompilation progress

> Last reviewed: **2026-11-13** · Retail executable: `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`

There is not yet a defensible whole-game percentage. The best reproducible
headline is that **3019 of 3021 selected function targets (99.9%)**
have strict, byte-for-byte matches in the reviewed calibration suite, compiled
with VC6 SP3. The suite contains **3072 cases**; repeated
retail address/extent pairs count once.
That percentage measures the active target set, **not 99.9% of MCM2**:
targets are chosen because they are useful or tractable, and the executable's
complete function inventory has not been established.

## Current indicators

| Indicator | Current value | What it means |
|---|---:|---|
| Strict VC6 exact targets | **3019 / 3021 (99.9%)** | Unique retail address/extent pairs in the reviewed calibration run |
| Canonical reconstructed implementation files | **226** | `.cpp` files under `src/reconstructed/` and `src/krusty2/`; may include incomplete candidates |
| Canonical reconstructed headers | **230** | Layout and interface declarations, including support-only headers |
| Canonical C++ source lines | **113,238** | Physical lines in the canonical `.cpp` and `.h` files; not a completion percentage |
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

The source-inventory rows are computed from the current tree, so any commit that
adds, removes or edits a canonical `.cpp`/`.h` file must also commit the output of
`make progress`; that needs no compiler or calibration run. `make static-check`
and the repository-check GitHub Action both fail with a diff when this page is
stale, so changes to canonical reconstructed sources or the snapshot cannot
silently leave it out of date. The check uses no proprietary executable or
compiler; the reviewed VC6 numbers remain an explicit checked-in snapshot.

Reproduce the calibration with the [private-input setup](TOOLCHAIN.md):

```bash
python3 tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

Inspect every result: the runner reports nonmatches as data and its exit status
alone does not prove strict matching. Reviewed code revision:
`calibration rerun after waves 25-29 and Fable round 12 (SetAxesIn, PathSteeringDirection), main 38d2a74e, 2026-11-14`. Source inventory counts reflect the current tree
and do not imply every body in those files matches.

### Caveats recorded with the snapshot

3019 of 3021 unique retail address/extent pairs passed strict comparison in 3072 calibration cases using authentic VC6 SP3 (Windows, native CL.EXE). There are 3065 strict rows and 3056 distinct source/address/extent cases; multiple source definitions may share a retail address. Every strict comparison resolves all relocations or has no relocation bytes. The 2 nonexact pairs are the FollowRacer near misses (0x4a9d20, 0x4a9e80). BaseObject::Release and the seven UI probe addresses now have strict rows; the seven bindings-free CompilerShapeProbe rows remain deliberate regression probes. The separately reviewed physics runner has 1091 strict exact results among 1152 comparisons; those results are not included in this calibration percentage. Neither suite is a census of retail functions or evidence that the source-built game links or runs. RTTI/source-path and .text figures retain their hash-pinned executable provenance.
