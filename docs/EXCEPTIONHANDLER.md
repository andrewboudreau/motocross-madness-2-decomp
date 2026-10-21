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

`RecordExceptionInfo` remains nonexact; its complete functional candidate and
stack-bound adaptation are described below. The seven canonical helpers retain
their strict matches.

FastMath's `InitFastMath` (`0x00460ad0`, 15 bytes) and its square root table
builder (`0x00460ae0`, 97 bytes) close the same gap. They match in
`src/reconstructed/FastMath.cpp`.

## Functional build review

`samples/game/RecordExceptionInfo.cpp` supplies the complete filter at
`0x0045ff80..0x0046048a` (1,290 retail bytes, followed by six NOPs).
VC6 SP3 emits 1,277 bytes. It is explicitly admitted as a reviewed nonexact
candidate in `config/game_build.json`, not promoted to canonical source.
The one-shot guard is set before opening `errorlog.txt`; every ordinary exit
returns `EXCEPTION_CONTINUE_SEARCH`. The filter appends the exception, machine
information, registers, 16 instruction bytes, up to 2,048 stack words and the
module list. Each instruction-byte read has its own SEH guard; the stack dump
has a separate guarded region. Failed reads retain the retail diagnostics.

Retail reads `FS:[4]` at `0x004602ef`. The paired ordinary-C++ candidate
`ThreadStackBase.cpp` obtains the actual current TEB through a dynamically
resolved `ntdll.dll!NtCurrentTeb` and reads `NT_TIB::StackBase`. The current host's
x86 export is independently confirmed in its PE table and native execution.
[Microsoft documents NtCurrentTeb's return value](https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-ntcurrentteb),
but this is not evidence that every historical Windows version exports it.
Missing module/export lookup raises `0xc0000002` into the reporter's existing
stack-dump error handler; it does not invent a stack bound. This dependency and
the emitted code differences are explicit functional adaptations. VC6 does
not provide the modern `__readfsdword` intrinsic; no assembly or machine-code
provider is introduced.

`tests/test_exception_report_behavior.py` links and executes the actual
standalone candidate object against the hash-pinned retail body in 13,838
native differential scenarios. It checks ordered output and dependency
arguments, both SEH regions with genuinely inaccessible memory, stack limits
and flush boundaries, access-violation parameters, path variants, failed opens
and the one-shot guard. Scripted dependencies isolate the filter's behavior;
they do not establish game startup. The private fixture instruments retail's
stack-base read and redirects its SEH runtime dependency to the native VC6 CRT.
The candidate's production instructions are not patched.

`tests/test_thread_stack_base.py` separately executes the standalone backend
1,536 times on the main thread, three worker threads and two fibers, comparing
with [GetCurrentThreadStackLimits](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentthreadstacklimits).
That newer API is a test oracle only. Both unavailable-backend branches also
exercise the real raised exception and SEH handling. These are current-host
checks, not a claim of compatibility with all historical Windows systems.

`tests/test_exception_report_io.py` links the standalone reporter, backend and
canonical helpers without mocks. Two real access violations reach an outer
SEH handler after the filter returns `EXCEPTION_CONTINUE_SEARCH`. The first
appends a real log beside the executable (with a different working directory),
including the captured native registers, instruction bytes, stack words,
system information and module list. The second leaves the entire file
unchanged. This verifies the report path on the current host; the game itself
still does not link or reach startup.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/ExceptionHandler.cpp -o work/ExceptionHandler.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
python -m unittest discover -s tests -p 'test_exception_report*.py' -v
python -m unittest discover -s tests -p test_thread_stack_base.py -v
```
