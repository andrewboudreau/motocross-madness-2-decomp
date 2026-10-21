# Streamed audio loader thread

`samples/audio/AudioLoaderThread.cpp` contains the complete existing
`UnknownPCAudioObject::UnknownThreadProc(void*)` candidate, extracted without
body changes from `PCAudioNearMisses.cpp`. The aggregate includes the standalone
file. This method is the callback passed by the canonical manager's
`StartLoaderThread` to `_beginthreadex`; it is distinct from the notification
helper's refill thread. The class name and original translation-unit ownership
remain provisional.

The hash-pinned retail function starts at `0x004bdc00`. Complete decoding reaches
`ret 4` at `0x004bdde0`, ending at `0x004bdde3`: **483 bytes**, followed by 13
alignment NOPs before the manager constructor at `0x004bddf0`. Every conditional
and unconditional branch stays within this body. Its six direct calls reach
the five game helpers below and `_endthreadex`; indirect calls use only the
three synchronization imports. There is no local EH registration or cleanup
funclet.

| Existing canonical dependency | Retail address |
|---|---:|
| `UnknownPCAudioObject::MakeRoomForSound` | `0x004be130` |
| `Sound::CreatePendingBuffer` | `0x004bd4b0` |
| `Sound::FillBufferFromFile` | `0x004bd0c0` |
| `Sound::Play` | `0x004bbcd0` |
| `UnknownPCAudioObject::RecordLoadedSound` | `0x004be0a0` |

All five definitions are present in `src/reconstructed/PCAudio.cpp`. Remaining
dependencies are the existing `WaitForMultipleObjects`, `EnterCriticalSection`,
`LeaveCriticalSection` imports and the VC6 CRT `_endthreadex` provider. The
standalone object defines just this method, with no static initializer or new
game dependency.

## Functional build review

Authentic VC6 SP3 emits **478 bytes**, retaining 91 matching positions against
the full 483-byte retail extent. This remains explicitly reviewed-nonexact;
it is not a strict reconstruction or evidence of original TU ownership.
Both aggregate bodies retain their pre-extraction complete bytes and relocation
sites, types, targets and addends: 158 bytes for `SetPrimaryFormat`, 478 for the
loader. The private extraction report is
`work/game-build/audio-loader-extraction-proof.json`, reproducible with
`work/game-build/check_audio_loader_extraction.py` against `3fc0a9b8`.
The persistent native test also requires standalone and aggregate loader
objects to agree on those complete bytes and relocations.

The loader snapshots its two event handles, waits indefinitely, drains the
queue on result zero, and exits on result one or `WAIT_FAILED`. Other wait
results repeat the wait. Removal replaces the removed element with the last
queue element; it does not preserve order. A sound without the streamed flag
or with `bufferReady` already set is skipped. The buffer-byte count is captured
under the sound lock before the manager makes room, then reused when recording
the loaded sound.

A successful `CreatePendingBuffer` leads to `FillBufferFromFile`. Its result is
ignored: the loader clears bit 7 at `Sound+0x1f4`, sets the ready bit, moves both
pending buffer pointers into their active fields, and clears both pending
fields. It calls `Play` unless the stopped bit is set, ignores that result too,
then records the sound. These potentially surprising behaviors are retained.
There is no invented lock cleanup when a dependency throws; the locks held at
that boundary remain held.

`tests/test_audio_loader_thread_behavior.py` builds the actual standalone
candidate and a separate native fixture with VC6 SP3. **37,387 scenarios**
compare the full manager, five full sound objects, queue and loaded-list
backing storage, ordered dependency calls, callback-entry state, held-lock
state, return/exit observations, and exception/fault information. They cover:

- Every five-sound streamed/ready/create-failure mask combination, stopped
  states, negative and empty counts, duplicate entries, and repeated batches.
- Zero, one, signed-boundary and maximum unsigned buffer-byte values; cached
  handles and bytes under callback mutations; ignored fill/play results.
- Timeout, abandoned and unexpected wait results; immediate stop and wait
  failure; both nonreturning thread-exit observation and the emitted return
  tail if a diagnostic exit hook returns.
- Mutation and C++ propagation at every reachable callback boundary in the
  selected complete-drain traces; enqueue/removal during unlocked
  create/fill/play boundaries; precise null context, queue-data and entry
  access violations, including the lock left held by the queue-data fault.

Independent outcome assertions require a three-sound success path to create,
fill, transfer, play and record each sound once, and a failed-create path to
remove each sound without filling, playing or recording it. Additional checks
require balanced locks on normal termination and reuse of the captured byte
count.

The fixture maps retail code only for private validation. Python checks every
retail direct-call site/target and complete branch extent. Runtime checks the
original direct-call opcode/displacement before redirecting the six calls.
The production candidate object remains unchanged: validation redirects only
its synchronization import operands to private pointer cells and its single
CRT exit call. Shared CRT import cells are not replaced. The complete source
body and inline queue operations execute on both sides.

The five game helpers and synchronization APIs are scripted dependencies.
Nonreturning `_endthreadex` is observed with a private exception; real CRT
thread teardown is not exercised. Deterministic callback interleavings do not
establish Windows scheduling, races, the memory model, real DirectSound buffer
creation, or audible playback. This finite behavioral evidence supports the
explicit candidate admission, not end-to-end audio or game readiness.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_audio_loader_thread_behavior.py -v
```
