# First category-guided reconstruction pilots

This change turns the EcoSystem and Terrain category/source/class leads into
reviewed normal-control-flow evidence and executable C++98 behavior models.
It extends the category-context branch without replacing either existing mapper.

## What is actually established

Input `mcm2.exe` SHA-256:
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

| Target | Reviewed extent (exclusive end) | Stronger identification |
|---|---|---|
| `0x0045aad0` | `0x0045add3` — 771 bytes / 214 instructions | EcoSystem primary vtable `0x00552508`, slot 12 |
| `0x005079f0` | `0x00507ba9` — 441 bytes / 137 instructions | Terrain destructor core, reached by its scalar deleting-destructor wrapper |

The tool verifies the whole input, each reviewed function range, the select and
restore helper ranges, the RTTI/table chain, the literal category strings, and
relevant call destinations. It re-decodes the input instead of consuming the
old context-map heuristics as authoritative function extents.

## Terrain: cleanup, not construction

The category clue at `0x005079f0` originally only established a Terrain-related
routine with a Terrain vptr write. That write alone could be construction or
destruction. The chain now establishes the latter:

```text
Terrain RTTI -> primary vtable 0x0055825c
             -> slot 0: deleting wrapper 0x005059b0
             -> destructor core 0x005079f0
             -> base cleanup 0x00468d60

GameObject RTTI -> slot-0 wrapper 0x00468d40
               -> the same base cleanup core 0x00468d60
```

Terrain RTTI lists both GameObject and GroundFogableObject as direct bases. This
pass does not invent the second base's destructor or flatten the inheritance.
It identifies the observed GameObject cleanup call.

The normal path selects `Terrain` at `0x00507a29`, saves the previous index,
cleans up the members below, restores at `0x00507b84`, and calls the GameObject
base cleanup at `0x00507b93`. Its sole normal return is `0x00507ba8`.

### Member responsibilities and order

| Offset | Observed use in cleanup |
|---|---|
| `+0xcb8` / `+0xcbc` | Pointer table / signed count; slot-2 member calls followed by freeing the table |
| `+0xc3c`, `+0x30` | Individually guarded slot-2 calls with receiver in ECX |
| `+0x540`, `+0x544` | Signed count and inline pointer sequence; null-checked slot-2 calls |
| `+0x44` | Prepare with flag 1, then reload/recheck the member and invoke slot 0 with flag 1 |
| `+0x34` | Slot-2 call with receiver on the stack, followed by explicitly zeroing the field |
| `+0xc10` / `+0xc14` | Count / table of raw blocks; individually guarded tracked-delete calls, then table free |
| `+0xc84`, `+0xc88` | Cached pointer: call body at `0x00401020`, then tracked deletion of the same pointer |

The table frees carry a literal `Terrain.cpp` source path and line values
`0x4b3` (1203) and `0x4d0` (1232). These are debug/allocation-call arguments in
the binary, not present-day line numbers in this repository.

Several details must survive reconstruction: the first table loop does not
check each element for null; counts and table pointers are reloaded around
callbacks; preparation can change `+0x44`, so the next call reloads it; only
`+0x34` is explicitly zeroed by this body. The cached standalone pointer is
retained across its destructor-like helper. Unknown capacities and pointee
classes remain unknown.

The stack-receiver slot-2 call is COM/IUnknown-Release-like, unlike the ECX
member calls. The exact interface is not established, so the model keeps these
as separate operations instead of naming it a specific DirectX interface.

## EcoSystem: three different category-lifetime exits

| Return VA | Direct normal-path observation |
|---|---|
| `0x0045aae8` | Global `0x0056a12c` is zero: return 1 before category selection |
| `0x0045ab16` | After selection and the first counter sample, member `+0x34` is zero: return 1 without a local restore call |
| `0x0045add2` | Main body completes, elapsed counter is stored, previous category is locally restored, then return 1 |

The previous index is saved at `0x0045aaf9` and reloaded into EDX at
`0x0045adb1` before the restore at `0x0045adc3`. A manual caller-frame review
places both accesses at entry-SP minus 16 despite their different ESP-relative
offsets. Terrain's corresponding saved value is at entry-SP minus 20. The tool
records those instructions but does not claim automatic stack/alias proof.

Do not silently add a scope guard which restores on all exits. It would change
the observed second path. Conversely, this static finding is not by itself
proof of a gameplay bug, a feasible state in real play, or the active global
category after unknown callees execute.

### Timer helper

The helper at `0x004bfa80` is now traced through its three named imports:

```text
timeBeginPeriod(1)
value = timeGetTime()
timeEndPeriod(1)
return value
```

It is called twice on EcoSystem's full path; the difference is stored at
`0x0059aee8`. The early selected return takes only the first sample. The C++98
candidate in `TimerProbe.cpp` requires no redistributed Windows SDK headers.

With local clang-cl 17 targeting i686 MSVC ABI, it produces 29 bytes, equal in
length to retail, but matches only **23/29 positions (79.3103%)** after applying
all **three DIR32 relocations** to verified local import slots. **No bytes are
ignored. This is not an exact match.** Differences are register-move encoding
and instruction scheduling. VC6 is still needed for historical calibration.

## What the code delivers, and what it does not

`PilotModels.h` contains:

- `TerrainNormalCleanup`: the reviewed normal cleanup sequence with explicit
  View/Operations boundaries for still-unknown field types and callees.
- `EcoSystemScopeShell`: the entry gates, sampling and restore behavior; the
  iteration/math region `0x0045ab17..0x0045ada8` is explicitly an opaque callback.

These are executable behavioral models, **not** complete original class
implementations. Terrain's vptr setup, compiler-generated exception machinery,
and real base-destructor execution are not reproduced by the native model.
The main EcoSystem routine has not been fully decompiled. No new whole-function
byte match or original translation-unit assignment is claimed.

`category_lifetimes.py` walks the reviewed normal CFG and retains a witness path
for each return/history pair. Its finite state records local select/restore
calls, not global runtime category state. It does not solve branch feasibility,
callee effects, exception unwinding, or restoration argument equivalence. A
stop/limit/unknown branch is reported, not silently treated as completion.

## Reproduce

```bash
make category-pilots           # Requires extracted owned EXE and GNU objdump
make category-pilots-probe     # Also compiles TimerProbe.cpp using clang-cl
make category-pilots-test      # No game, SDK or VC6 required
```

Direct invocation from any working directory:

```bash
python3 /path/to/repo/tools/review_category_pilots.py \
    --exe /private/game/mcm2.exe --compile-probe
```

Outputs are `work/category_pilots/pilots.json` and `REPORT.md`. `work/` is already
ignored by Git/Docker. The JSON includes input/config/tool hashes, reviewed
instructions, source references, RTTI/wrapper checks, exit witnesses and strict
timer-probe results. The config is specific to this one reviewed build.

## Validation performed

- **26 new tests pass**, including a native C++98 executable running **19 model
  scenarios**. Those 19 are inside one unittest, not 19 extra unit tests.
- **95 existing tests pass**: 28 category-anchor, 34 category-context and 33
  allocation tests. Total across the four suites: **121 passing tests**.
- Two independent live pilot runs (including timer compilation) produce
  byte-identical JSON and Markdown. All reviewed hashes and RTTI checks pass.
- Tested PE, RTTI, MSVC-artifact, allocation, COFF, resolved-matcher and context
  modules match their Git blob hashes on PR #2 at `24c107e`.
- Original game, installer and DLL code was not executed. The native executable
  runs only our models and stubbed callbacks.

No historical VC6 run, complete-bootstrap rerun, exception-equivalence test or
new exact decomp percentage is implied. The new workflow runs the four
input-free suites; hosted workflow status must be checked separately.

## Next reconstruction gate

Use the cleanup responsibilities to identify member types and their acquisition
sites in Terrain's constructor/loading routines. For EcoSystem, reconstruct the
opaque iteration/math body while preserving the gate and lifetime observations.
Keep the normal model as a regression oracle for intended behavior, not a
substitute for original-binary equivalence. Match the helper and real class
functions under privately supplied VC6 when available.

## Primary ABI references

- https://learn.microsoft.com/en-us/cpp/cpp/thiscall
- https://learn.microsoft.com/en-us/cpp/cpp/stdcall
- https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown

These explain ABI/interface patterns, not MCM2-specific symbol identities.
