# Replay playback tick

`samples/race/RecorderTick.cpp` contains the unchanged complete
`VCRInterface::Tick(float)` candidate. `RecorderNearMisses.cpp` includes that
source so there is one maintained body. The class identity comes from RTTI;
the replay field and helper names remain provisional. This extraction does
not establish an original translation-unit boundary.

## Retail extent and source preservation

VCRInterface's primary table at `0x0055778c`, object offset zero, has slot 10
at `0x004e7d60`. The complete body has **2,364 code bytes**, ending with
`ret 4` at `0x004e8699`, followed by a **48-byte table of 12 destinations**.
Every table entry and direct branch targets a decoded instruction within the
body. The complete **2,412-byte** extent ends at `0x004e86cc`; four NOPs
precede the next function at `0x004e86d0`.

The persistent test checks all direct calls and all 28 indirect call sites.
Indirect calls use Enter/LeaveCriticalSection at IAT addresses `0x0055025c`
and `0x00550284`, either directly or through registers with verified loads,
and the cdecl record callback at receiver offset `0x80`. The body has no
local EH registration or cleanup funclet. Its seven game helper dependencies
already have canonical definitions in `Recorder.cpp` and `VCR.cpp`.

Authentic VC6 SP3 `/O2` emits **2,448 bytes**, with **276 matching positions**
against the full retail extent. This is explicitly nonexact. The standalone
and aggregate candidates have identical complete function bytes and
relocations, with generated labels resolved to their actual local offsets.
The standalone defines only Tick, has no CRT initializer, and introduces no
new missing dependency. Its sole floating constant, 1000, is checked against
the retail readonly value at `0x00550f44`.

A private extraction check against `8d7abb93` preserves all **13 emitted
aggregate functions** and all **45 nondebug sections**, including EH, CRT,
constants, jump tables and storage. Generated labels are compared by their
section/offset destinations; no filename storage needed normalization in
this aggregate. The ignored reproduction script and report are
`work/game-build/check_recorder_tick_extraction.py` and
`work/game-build/recorder-tick-extraction-proof.json`.

## Functional build review

Tick waits for the ready flag under the recorder lock and operates only in
playback mode. It handles forward records, pending records deferred by the
callback, reverse restart and reverse traversal. Callback answers select
continued playback, deferral, rewinding, reverse reading or seeking. Frame
records supply seconds and a record count; the body converts seconds to
milliseconds through the VC6 CRT floating-to-integer helper. It appends
retained record positions and end markers in the observed order. The supplied
frame-time argument is unused; all normal exits return one.

`tests/test_recorder_tick_behavior.py` runs **147,749 native scenarios**
against the hash-pinned complete retail body. The actual standalone candidate
and the aggregate compile with `/O2`; the independent fixture uses `/Od`.
Only external dependency calls and import operands are redirected in the
private executable. The candidate object is not rewritten. The actual linked
VC6 CRT `__ftol` runs for both paths; its arithmetic is not modeled by a test
substitute. No retail execution or fallback enters the production build.

The fixture compares all 0xd8 recorder bytes, two distinct 0x400-byte record
buffers and two distinct 0x101c-byte ring views at each dependency boundary
and after execution. It also compares callback arguments, buffer/ring
identity, record-request flags and output-pointer roles, ordered operations,
held-lock state, return values, exception/fault details and x87 status.
Coverage includes:

- Forward, deferred and reverse states, ready/mode gates, record statuses
  zero through four, callback answers -1 through 12, and negative, zero and
  positive record counts.
- Exceptions at every observed boundary in representative complete traces,
  and mutations to replay state, the active buffer/ring, frame data and the
  callback pointer. Null callback and buffer cases compare actual faults.
- Signed zero, subnormal and normal values, conversion thresholds, very large
  finite values, all four x87 rounding modes and 8,192 deterministic varied
  cases. Floating conversion results and status are compared bit for bit.
- Independent expected outcomes for not-ready playback, reverse restart,
  callback deferral and rewind, and a successful two-record frame. The last
  case requires three record takes, three callbacks and four position writes,
  including the final -1 marker and the expected 1234-millisecond value.

Normal returns must leave no held lock. Exceptions preserve the lock count
observed at the throwing boundary; the body does not invent cleanup. Invalid
buffer/callback faults occur outside the lock. Record scripts supply at most
five successful takes and then report status four, so malformed negative
counts still terminate. These are finite dependency contracts, not a proof
for arbitrary callback programs.

This evidence supports **reviewed-nonexact** admission. Lock APIs and the seven
game helpers are scripted, even though their canonical providers exist. The
test therefore does not exercise actual thread scheduling, worker events,
ring allocation, replay-file I/O, or end-to-end recording/playback. NaN
payload behavior is not established. The missing worker operation and other
link gaps still require their own implementations and evidence.

The isolated real build `build-jfyngil3` compiles **205/205 translation units**.
Compared with `build-pclfp8jr`, unresolved names decrease from **484 to 483**:
only `VCRInterface::Tick(float)` disappears, with no new missing names. The
game still does not link; startup and live replay playback remain unverified.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_recorder_tick_behavior.py -v
```
