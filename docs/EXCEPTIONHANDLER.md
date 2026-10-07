# ExceptionHandler (crash reporter)

Canonical reconstruction: `src/reconstructed/ExceptionHandler.*`. The unit
has no RTTI and no `__FILE__` literal.

## Extent and identity

The unit spans `0x0045ff80..0x00460acb`. It sits between EventManager.cpp
(`0x0045e89a`) and FollowCam.cpp (`0x0046315a`). FastMath's initialisers
start at `0x00460ad0`.

The literals and the code follow Bruce Dawson's published
ExceptionHandler.cpp (Game Developer, 1999) function by function:
- `"errorlog.txt"`;
- `"Error creating exception report"`;
- the `"a Control-C"`..`"a Microsoft C++ Exception"` table;
- `" - file date is "`.

The file name and the function names come from that listing. That makes them
external context (tier 3), not retail evidence.

The entry wrapper `WinMain` (`0x004a0c19`, see
[MAIN](MAIN.md)) passes `"main thread"` to the filter.

## Status (7 of 8 exact)

| VA | Size | Function |
|---|---:|---|
| `0x00460490` | 75 | `hprintf` |
| `0x004604e0` | 160 | `RecordModuleList` |
| `0x00460580` | 384 | `ShowModuleInfo` |
| `0x00460700` | 147 | `PrintTime` |
| `0x004607a0` | 278 | `RecordSystemInformation` |
| `0x004608c0` | 487 | `GetExceptionDescription` |
| `0x00460ab0` | 27 | `GetFilePart` |

Notes:
- `ShowModuleInfo` has a `__try/__except` with scope table `0x005526c8`.
  The strict matcher binds a scope-table push under the stable key
  `<function symbol>$scopetable`; `tests/test_seh_scopetable_relocations.py`
  covers it.
- `GetExceptionDescription` builds its 24-entry code/name table on the stack.
  Its loop index is compared unsigned against a `sizeof` quotient (`jb`).

Not reconstructed: `RecordExceptionInfo` (`0x0045ff80`, the `__except`
filter). Its stack dump reads the stack top with inline assembly
(`mov eax, fs:[4]`), which the project rules exclude.

FastMath's `InitFastMath` (`0x00460ad0`, 15 bytes) and its square root table
builder (`0x00460ae0`, 97 bytes) close the same gap. They match in
`samples/physics/helpers/FastMath.cpp`.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/ExceptionHandler.cpp -o work/ExceptionHandler.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
