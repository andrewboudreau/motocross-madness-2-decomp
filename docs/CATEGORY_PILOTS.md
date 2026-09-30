# Terrain cleanup and category lifetimes

Reviewed normal-control-flow evidence and C++98 candidates. The tool re-decodes
retail bytes; host models do not establish binary equivalence.

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

## Terrain destructor

The RTTI/wrapper chain identifies `0x005079f0` as the destructor core:

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

The helper at `0x004bfa80` calls three named imports:

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
and instruction scheduling. This helper still needs authentic VC6 calibration.

## Reconstruction scope

`PilotModels.h` holds the Terrain normal-cleanup model. It excludes vptr setup,
exception machinery and actual base destruction. Member acquisition sites/types
remain unresolved. The full EcoSystem algorithm and its lifetime scenarios are
in [ECOSYSTEM.md](ECOSYSTEM.md); it is not yet an exact VC6 match.

`category_lifetimes.py` retains witness paths for normal select/restore exits.
It does not solve branch feasibility, callee effects, exceptions or stack-alias
proof. Unknown branches and limits remain explicit.

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

## Next work

Identify Terrain member types and acquisition sites in constructor/loading code.
Match its real cleanup function and the timer helper under VC6. Keep the normal
model as a behavior regression check, not proof of original-binary equivalence.
