# Current matching status

Target `mcm2.exe` SHA-256:
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
Results use VC6 SP3 natively on Windows; the 2026-09-30 calibration and full
gate were repeated under Linux/Wine with the same results. A complete linked
game remains a separate, unverified gate.

## Compiler profiles

| Profiles | Strict generated | Manual | Calibration |
|---|---:|---:|---:|
| `vc6_o2_mt` (default) | 39/39 | 19/19 | 239/239 |
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

## Next targets

All 17 calibration targets match the default profile, including
[FollowCamera](FOLLOW_CAMERA.md) slots 68 (`0x00466d50`, x87 distance/clamp,
every relocation resolved), 69 (`0x00466a80`, source shape) and 71
(`0x00466e50`, 192-byte extent including its jump table). Every FollowCamera
slot 63–72 now has an exact candidate. Fifteen Camera/PCCamera bodies, including their destructors and the PCCamera
constructor,
(`src/reconstructed/Camera.cpp`, `PCCamera.cpp`) also match strictly with every
call bound;
see [FollowCamera](FOLLOW_CAMERA.md#camera-and-pccamera).

The legacy function manifest and queue consume clang reports, not the VC6 profile
matrix. Use actual VC6 reports for current matching status; queue validation
labels do not yet reflect these results.

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
