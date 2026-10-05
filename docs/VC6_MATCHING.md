# Current matching status

Target `mcm2.exe` SHA-256:
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
Results use VC6 SP3 natively on Windows. The full gate and profile matrix were
repeated on 2026-10-02 under Linux/wibo; the default-profile calibration
was rerun on 2026-10-05 (631 cases). A complete linked game remains a
separate, unverified gate.

The VC6 gate currently checks byte-exact functions from 49 handwritten C++
candidate files: 42 of the 43 files in `src/reconstructed/` (all but
`TerrainSupport.cpp`) and seven focused probes in `samples/`. This is a count of source files represented by at least one
checked function, not a claim that complete object files or a linked game match.

## Compiler profiles

| Profiles | Strict generated | Manual | Calibration |
|---|---:|---:|---:|
| `vc6_o2_mt` (default) | 39/39 | 19/19 | 631/631 |
| `vc6_o2_ml` | 39/39 | 19/19 | 52/52 (first 52 cases) |
| `vc6_o2_mt_g6` | 39/39 | 19/19 | 27/61 (first 61 cases) |
| `vc6_o2_ml_g6` | 39/39 | 19/19 | 25/52 (first 52 cases) |
| `vc6_o1_ml`, `vc6_o1_mt` | 31/39 | 13/19 | 8/17 (camera cases not rerun) |

Passing manual samples mask no bytes. Generated probes resolve both global-load
addresses. Summaries prefer strict results when available.

The default is `vc6_o2_mt`: `/O2` without `/G6` is the only tested family that
matches every calibration target, and no target prefers `/G6`. The 34 `/G6`
misses (BaseObject Release, UIControl 61/62, FollowCamera 68/69/72, Camera
13/18/30–32, `~Camera`, PCCamera 13 and 21 [GameObject](GAMEOBJECT.md) functions) differ only in
instruction selection and scheduling; explicit `/G5` behaves like VC6's default. `/ML` and
`/MT` emit identical code for every tested target; `/MT` follows the
[runtime identity](VC6_CRT_ATLAS.md). This is the best-supported working
hypothesis, not proof of original per-file flags; test others with `--profile`.
The physics samples (`tools/run_physics_samples.py`) independently show the
same split: 139/194 targets match without `/G6` versus 76/194 with it, and none
match only under `/G6`.

## BaseObject

Canonical source is `src/reconstructed/BaseObject.cpp` and `.h`. All six bodies
match under `vc6_o2_ml`, with zero ignored bytes:

| Body | Retail VA | Bytes | Relocations |
|---|---|---:|---|
| Constructor | `0x00405120` | 16 | Vptr to `0x005507c0` |
| Scalar deleting destructor | `0x00405130` | 30 | Calls `0x00405150`, `0x004a30c0` |
| Destructor core | `0x00405150` | 7 | Vptr to `0x005507c0` |
| AddRef | `0x00405160` | 8 | None |
| Release | `0x00405170` | 32 | None; profile without `/G6` |
| GetRefCount | `0x00401940` | 4 | None |

RTTI confirms the class and primary table at object offset zero. The 32-bit field
at +4 begins at 1. A constructor-body assignment preserves vptr-before-field
store order. Release saves the decremented result across virtual deletion and
returns zero for an already-zero count. Semantic names, signedness and original
translation-unit ownership remain provisional.

`src/reconstructed/BaseObject.bindings.json` records destinations supported by
RTTI, decoded vptr writes and deleting-wrapper evidence. `0x004a30c0` includes
[application allocation accounting](ALLOCATION.md). Wrong bindings fail comparison.

## Validated slices

The seven emitted fixed-size `BlockAllocator` bodies are now exact under the
default profile, including the five-byte destructor tail jump and the two
fully resolved `BlockAllocator.cpp` debug-allocation calls. See
[allocation evidence](ALLOCATION.md#fixed-size-block-allocator).

All 17 calibration targets match the default profile, including
[FollowCamera](FOLLOW_CAMERA.md) slots 68 (`0x00466d50`, x87 distance/clamp,
every relocation resolved), 69 (`0x00466a80`, source shape) and 71
(`0x00466e50`, 192-byte extent including its jump table). Every FollowCamera
slot 63–72 now has an exact candidate. Fifteen Camera/PCCamera bodies, including their destructors and the PCCamera
constructor,
(`src/reconstructed/Camera.cpp`, `PCCamera.cpp`) also match strictly with every
call bound;
see [FollowCamera](FOLLOW_CAMERA.md#camera-and-pccamera).

Every function of `Lzw.cpp` (11 bodies, `0x004a01d0..0x004a05da`) matches
strictly; see [LZW](LZW.md).

`Parameterblocks.cpp` matches strictly in 26 of its 27 functions; see
[PARAMETERBLOCKS](PARAMETERBLOCKS.md).

`Parser.cpp` matches strictly in 13 of its 14 functions; see [PARSER](PARSER.md).

Track.cpp (tokenizer, node/segment walks, lap-time formatting): see [Track](TRACK.md).

TrackRecord.cpp (the high-score table): see [TrackRecord](TRACKRECORD.md).

The legacy function manifest and queue consume clang reports, not the VC6 profile
matrix. Use actual VC6 reports for current matching status; queue validation
labels do not yet reflect these results.

## Next matching campaign

The next set should improve both the number of exact bodies and the reliability
of the progress view. The targets below are a shortlist, not assignments: open
pull requests may already contain newer candidates than the checked-out branch.
Use the following order only after the coordination check; do not start with the
largest unreconstructed UI or event routines merely because their VAs are
already known.

### 0. Exclude work already in flight

Fetch the target branch and inspect every open pull request before selecting a
VA. Do not rely only on PR titles: review changed paths and search each diff for
the candidate VA, class, slot and symbol. Also check overlapping shared headers,
bindings and calibration-list edits, since two PRs can conflict even when they
match different functions.

```bash
git fetch --all --prune
gh pr list --state open --limit 100 \
  --json number,title,headRefName,baseRefName,url
gh pr diff <number> --name-only
gh pr diff <number> | rg -i '0x004c16f0|PCGame|UnknownVirtualSlot'
```

Record the reviewed PR numbers and claimed VAs in the work notes. If the GitHub
remote or credentials are unavailable, do not interpret an empty local branch
list as evidence that no work is in flight; restore access or coordinate with a
maintainer before taking a target. Rebase the shortlist on the fetched target
branch, then choose the highest-ranked unclaimed slice. Repeat this check before
starting another slice, not merely once per long-running branch.

**Exit criterion:** the selected target and the shared files it needs do not
overlap any open PR, or the PR authors have explicitly agreed how the work will
be divided.

### 1. Make VC6 results visible in the inventory

Extend the manifest/queue pipeline to ingest strict results from the canonical
VC6 calibration run. The current `make status` result of zero validated targets
contradicts the 480/480 matrix because it only reads the legacy clang reports.
The importer must key records by target VA, extent, candidate symbol and source,
retain relocation/binding status, and avoid counting aliases or repeated profile
runs as new retail functions. Add regression fixtures before changing the status
totals.

**Exit criterion:** regenerating the manifest, dossiers and queue reports the
same strict targets as the default-profile calibration, while the existing
clang-only data remains distinguishable rather than being promoted to VC6
evidence.

### 2. Close the smallest, best-constrained unclaimed near misses

Use one candidate per commit and run the full calibration after any shared-header
change, because seemingly harmless declarations have already changed VC6 register
allocation in other translation units.

Subject to the open-PR check, prefer:

1. `PCGame` profile loader `0x004c16f0` (767/771 bytes): the behavior, extent and
   calls are already reconstructed; only the copy loop's SIB base/index choice
   differs.
2. `ControlInterface` update `0x0043cf00`: the devices, event layout and dispatch
   are established, and the remaining discrepancy is confined to modifier null
   handling.
3. Event progress callback `0x0045cb20` (63/67 bytes): isolate the two-register
   swap without moving provisional GUI types into shared headers.

For each body, first confirm its VA/extent and direct bindings against the current
retail image, keep experiments in the existing `samples/` near-miss file, and
promote only a readable strict match into `src/reconstructed/`.

**Exit criterion:** every promoted body has equal candidate/retail extents, zero
ignored bytes, and all external relocations resolved under `vc6_o2_mt`; nearby
calibration cases still pass.

### 3. Take bounded helpers before large orchestrators

After the quick wins, reconstruct EcoSystem's two constrained record helpers at
`0x00456890` and `0x00456a10`. Their call contracts and downstream uses are known,
so they can replace provisional state interpretations with evidence without
claiming that the allocation category establishes source ownership. In parallel
conceptually—but as separate commits—trace Terrain construction/acquisition sites
before attempting its cleanup, so member types and lifetime order are supported
independently.

Then revisit the `ViewMatrix` helper at `0x004a14f0`: its math and complete extent
are known, but store scheduling and floating-point term order remain open. Keep it
in `samples/` until the full 744 bytes match; do not inline already-established
retail helper calls to force a local resemblance.

**Exit criterion:** each helper is either promoted with a strict VC6 result or
left as a documented near miss with the exact differing instruction ranges and
an evidence-preserving next experiment.

### 4. Expand one subsystem at a time

Prefer `PCTextureMap` slots 9 and 6 next because their frames and high-level flow
are already understood. Defer its 2031-byte setup (slot 4), EventManager's
4247-byte routine, and KrustyUI's 2432/3114-byte routines until their callees,
member layouts and smaller surrounding methods are represented. This keeps a
failed large match from conflating ABI, register allocation, control flow and
unknown type errors.

At the end of every slice, regenerate analysis artifacts, run the static/native
tests and the authoritative gate, then update this section with exact counts and
remaining uncertainty. Generated `analysis/` and `work/` reports stay untracked.

## Match contract

Candidate lengths come from COFF function auxiliary `TotalSize` or legacy VC6
CodeView procedure lengths. CodeView association requires paired i386
SECREL/SECTION relocations to one exact function symbol with zero address addends.
Conflicting, zero, truncated or out-of-bounds metadata fails. Unsupported metadata
retains the symbol/section extent. Target sizes and NOPs never determine lengths.

`/Z7` supplies measurement metadata, including the deleting wrapper's length;
paired compilations preserve code sections and relocations. It is not a claimed
original flag. Format references: [PE/COFF](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#auxiliary-format-1-function-definitions)
and [CodeView](https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h).

Same-section relocations are inferred only when their final symbol-plus-addend
destination is inside the independently measured function extent. This includes
section-symbol references into jump tables; an internal label plus an escaping
addend still needs an explicit binding. Inference is per relocation, so a local
reference cannot authorize another reference outside the function.

`tools/match.py --bindings` applies DIR32/REL32 relocations and compares every byte.
Bindings require independent address evidence. Without bindings the tool masks
relocation fields; that is strict only when no bytes were masked. Unsupported or
unresolved relocations fail strict matching.

## Reproduce

After [setup](TOOLCHAIN.md), in PowerShell:

```powershell
python tools/compile.py src/reconstructed/BaseObject.cpp -o work/base.obj --vc6-root $env:VC6_ROOT --profile vc6_o2_ml
python tools/match.py --exe $env:MCM2_EXE --target-va 0x00405130 --target-size 30 --obj work/base.obj --symbol '??_GBaseObject' --bindings src/reconstructed/BaseObject.bindings.json --json
python tools/match.py --exe $env:MCM2_EXE --target-va 0x00405170 --target-size 32 --obj work/base.obj --symbol 'Release@BaseObject' --bindings src/reconstructed/BaseObject.bindings.json --json
python tools/vc6_profile_matrix.py --exe $env:MCM2_EXE --vc6-root $env:VC6_ROOT
```

`make vc6-gate` enforces executed SP3 identity and manual/generated matches;
calibration failures remain diagnostic. `make vc6-profile-matrix` compares all
profiles. Detailed reports belong in ignored `analysis/` and `work/`.
