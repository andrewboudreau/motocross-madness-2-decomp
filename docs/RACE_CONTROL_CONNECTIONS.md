# Race control calls

Five provisional `UnknownKrustyBikeView` method references now call existing
`BikeRace` implementations through explicit primary-pointer casts. The
provisional field view remains in place; no class layout, inheritance,
virtual declaration, method body, or provider ownership changes.

| Canonical provider | Retail VA | Complete strict bytes | Callers |
|---|---:|---:|---|
| `HandlePlayerLeft(int)` | `0x00420590` | 189 | EventManager, QuarryStuntEvent |
| `ToggleReplayPause(int)` | `0x0041f550` | 60 | InGameProcs |
| `RestartRace(int, char*, char*)` | `0x00420650` | 1,194 | InGameProcs |
| `SaveReplay(char*, char*)` | `0x00420b00` | 207 | EventManager |
| `ResetStartCountdown()` | `0x00421050` | 158 | DlgProcs |

All five providers already reside in `src/reconstructed/BikeRace.cpp` and
match their complete retail extents with relocations resolved. Their emitted
COFF function extents agree with the retail extents. This slice adds no
implementations, forwarding wrappers, linker aliases or candidate admissions.
Consumers use the existing complete declaration in `BikeRaceCore.h`.

## Receiver and argument evidence

The retail event loader calls the BikeRace constructor `0x00417c30` at
`0x004df9ff` and stores its returned pointer at event+`0x34` at `0x004dfa08`.
It loads that field into ECX at `0x004dfabb`, calls BikeRace's setup at
`0x004dfabf`, and stores its returned pointer back at `0x004dfac6`.
EventManager's complete 78-byte `FindRaceView` at `0x0045d2f0` chooses the
first present mode event among TrackGame+`0x558/0x55c/0x560/0x564/0x568` and
returns event+`0x34` unchanged. A present event with a null race pointer
returns null; it does not continue searching.

BikeRace's COL `0x0055ac68` and hierarchy `0x0055ac58` identify its own
BikeRace/GraphicsTest/GameObject/BaseObject chain at displacement zero.
That inheritance record does not establish the complete identity of the
provisional race declaration. The constructor/setup stores, getter, actual
direct callers and shared field offsets supply the narrower receiver proof.
Native VC6 checks the canonical size `0x400`, provisional size `0x3fc`,
local racer offset `0x38`, and replay-paused offset `0x3f8`. The different
sizes remain explicit; complete type equivalence is not claimed.

Fifteen retail direct call sites are checked. The methods use the same ECX
receiver and ordinary 32-bit stack arguments. In particular, ExitDlg's only
`RestartRace` call at `0x004888b5` pushes `0`, `0`, `1` and moves the original
pointer from EAX to ECX. The old three-int declaration was a provisional
description of those bits. The canonical signature retains the mode integer
and gives the two literal nulls their actual pointer types. The provider
passes those pointers to `SaveReplay`; that method treats them as a path
and optional description. EventManager's save call already supplies two
character buffers. No nonzero integer is reinterpreted as a string pointer.

## Verification and remaining scope

`tests/test_race_control_connections.py` compiles the actual five production
translation units without an extra physics include directory and checks all
206 registered comparisons. All remain strict. It also checks the consumers'
actual COFF calls/imports against the five unique existing provider symbols.

The native fixture executes the unchanged `FindRaceView` and
`ToggleReplayPause` bodies, extracted only to avoid linking unrelated missing
game functions. Both execution objects independently match the full retail
bytes, as do their original production objects. Its 993 cases exercise all
event-presence combinations, first-event precedence, null race pointers,
pause/toggle values, and full race/game storage. In particular, an already
paused race with a nonzero pause-only argument returns without synchronizing
the global pause field; the test retains that behavior. No game constructors
or uninitialized virtual methods execute.

`DlgProcsNearMisses.cpp` includes the changed canonical DlgProcs file. Before
this work it failed compilation because one `ShowDialog` parent argument
still used `(int)this` despite the canonical `UIDialog*` parameter. Only that
cast is repaired to `(UIDialog*)this`. MainDlg directly derives from UIDialog;
retail saves the receiver in EBP at `0x0044b226` and pushes that unchanged
pointer as the parent at `0x0044b360`. The four sample bodies' complete
resolved hashes remain identical to a private baseline with this mandatory
type correction applied **before** the race-control changes. This is not
byte-preservation evidence for the unmodified, uncompilable old sample.
The sample remains nonexact and excluded from the source-built game.

These connections resolve declaration mismatches; they do not establish
working race restarts, replay files, network play or game startup. The game
still requires other missing providers before a complete link and runtime
validation.

The isolated authentic source build `build-wjtz79k7` compiles all 201
translation units. Compared with `build-ev5772um`, unresolved symbols fall
from 509 to 504: exactly the five provisional methods above disappear,
with no added unresolved names. The link still fails.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_race_control_connections.py -v
```
