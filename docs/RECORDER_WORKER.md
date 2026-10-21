# Replay recorder worker

`samples/race/RecorderWorker.cpp` maintains the complete readable worker
candidate; `RecorderNearMisses.cpp` includes it. The body is unchanged from
the earlier aggregate. Its semantic name and this source grouping remain
provisional; extraction does not establish an original translation unit.

## Entry, extent and dependency evidence

The canonical `VCRInterface::Start` at `0x004e7a90` passes `0x004e6f80`
directly to `_beginthreadex`: the pointer push is at `0x004e7c55`, followed
by the CRT call at `0x004e7c5e`. The function receives the recorder's
parameter block at offset `0x90`. Start selects the create, reopen or
playback event after starting the worker.

The complete retail extent is **2,148 bytes**: **2,101 code bytes** through
`ret 4` at `0x004e77b2`, three alignment bytes, and an **11-entry, 44-byte
jump table** at `0x004e77b8`. Twelve NOP bytes separate its end at
`0x004e77e4` from the next function. The test decodes from the true entry,
checks every direct branch and table destination, and checks all **48 direct
calls** and **57 indirect calls**. The indirect calls use only the five
synchronization imports; six ResetEvent calls use EDI after checked loads
from its IAT slot. There is no local EH registration or cleanup funclet.

All **20 game helper dependencies** already have canonical definitions in
`VCR.cpp` and `VCRfile.cpp`. The focused test compiles those providers and
checks their complete strict bytes, plus the complete 716-byte Start caller.
This proves provider closure separately from the scripted behavioral tests.

VC6 SP3 `/O2` emits **2,148 bytes**, with **2,100 matching positions** after
resolving every relocation. It remains nonexact. The standalone declaration
context changes register/stack allocation and shared-tail scheduling relative
to the older aggregate, which remains **2,124 bytes / 652 matching positions**.
Neither body is trimmed. The new object defines only the worker, has no CRT
initializer, and imports only the twenty existing helpers, five Win32 APIs
and `_endthreadex`. Its thirteen string literals retain their complete
retail bytes including terminators, and writable data storage in both COFF
and PE; no readonly-storage claim is made for these VC6 literals.

An independent extraction check against `67edacf0` preserves all **13 emitted
aggregate functions** and all **45 nondebug sections**, including bytes,
relocation types/destinations/addends, jump tables, EH data and storage.
Only compiler-generated local symbol names are represented by their actual
section/offset destinations; no filename bytes required normalization.
The ignored reproduction script/report are
`work/game-build/check_recorder_worker_extraction.py` and
`work/game-build/recorder-worker-extraction-proof.json`. The persistent test
also pins the complete aggregate worker bytes and relocation fingerprint.

## Functional build review

The worker waits on eleven events to record buffered data, create/reopen a
file, read forward or backward, seek, rewind, stop reading, close the file,
or terminate. It caches the file receiver, header pointer/size, filename and
eleven wait handles at entry, while reloading the recorder's ring pointer
and event fields where the instructions do. It transfers record payloads,
records positions and end markers, and updates acknowledgement/state flags
in the observed lock order. Failed waits and fatal file operations take the
same shutdown path. Shutdown clears `isStopped`, closes only a nonnegative
handle, and calls `_endthreadex(0)`; no invented unwind cleanup is added.

`tests/test_recorder_worker_behavior.py` runs **24,595 native scenarios**
against the hash-pinned complete retail body and the actual separately
compiled `/O2` candidate. The independent fixture is compiled `/Od`.
The test executable redirects only audited external call/import operands;
the compiled candidate object is not rewritten. No reference execution,
retail dispatch or test fixture enters the production build.

The fixture compares every byte of the 0xd8 recorder, two distinct
0x101c-byte ring views, two 0x248-byte file views, four 0x400-byte buffers,
two 128-byte headers and two 512-byte filename stores. It snapshots that
storage at every dependency boundary and after execution, along with
ordered calls, receiver and pointer roles, actual written payload bytes,
wait handles/results, counts/positions, lock ownership, return values and
exception/access-violation details.

Coverage includes:

- Every event, repeated open/create events, WAIT_FAILED, abandoned and
  unexpected wait results; recording, forward/reverse reads, seek/rewind,
  stop-reading, close and normal kill sequences.
- All combinations of open, write, size-read, data-read, header-read and
  read-slot failure bits, EOF/busy gates and available/full ring responses.
  Record sizes include 0, 1, 1023, 1024, 1025 and UINT_MAX. Handles include
  signed extremes, -2, -1, zero and positive values.
- Exceptions at every boundary in representative complete traces and four
  mutation families changing state flags, ring/file receivers, thread
  parameters and event fields. Cached and reloaded values are distinguished.
- Null parameter, owner, filename and ring faults, plus null-buffer faults
  after scripted slot failure. Ordinary exception/fault paths preserve held
  locks rather than inventing cleanup. Thread termination is observed both
  through a private nonreturning control exception and through its otherwise
  unreachable return tail.
- Independent expected outcomes for successful recording/playback, create
  and header-write failures, forward EOF, reverse sentinel insertion,
  WAIT_FAILED and normal kill. These assertions check concrete writes,
  markers, close counts, acknowledgements, payload and termination state.

These are finite, scripted dependency contracts. File reads of zero bytes
return zero, consistent with the canonical file helper; tests do not grant
a failed slot claim an initialized output index. Payloads fit the provided
storage, filenames are at most 511 bytes, and the event script eventually
signals termination. Twenty ring/file methods, five synchronization APIs
and thread exit are scripted. The evidence does **not** exercise real
thread scheduling, event signalling, disk I/O, ring allocation or complete
record/playback integration. Test exceptions do not claim a faithful CRT
thread teardown. These limits remain separate from the strict provider
proofs and from functional source-build admission.

The reviewed candidate is explicitly admitted as **reviewed-nonexact** in
`config/game_build.json`. The isolated real build `build-pl4kpvmo` compiles
**209/209 translation units**. Relative to `build-l99te5t5`, unresolved names
decrease **462 to 461**: only `UnknownRecorderThread` disappears and no new
missing name appears. The game still does not link; startup, live menus,
racing and integrated replay behavior remain unverified.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_recorder_worker_behavior.py -v
```
