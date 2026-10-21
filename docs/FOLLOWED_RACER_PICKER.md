# Followed-racer selection

`TrackGameViewOwner::PickFollowedRacer` selects the host's followed racer and
sends the selection to the network and optional replay recorder. Its complete
candidate is in [FollowedRacerPicker.cpp](../samples/race/FollowedRacerPicker.cpp),
included at the original position by `FollowRacerNearMisses.cpp`. Original TU
ownership and the semantic class/method names remain provisional.

## Retail extent and callers

The hash-pinned retail body occupies **0x004a9d20 through 0x004a9e73**, inclusive:
**340 bytes**. All decoded branches stay within this extent; the sole `ret` is
at 0x004a9e73, followed by twelve alignment NOPs before 0x004a9e80. There is no
EH setup or indirect call in this function. The test independently checks all
six direct call sites:

| Site | Target | Boundary |
| --- | --- | --- |
| 0x004a9d54, 0x004a9d70 | 0x004204e0 | `BikeRace::NextBike(int*)` |
| 0x004a9da5 | 0x00534c1b | CRT `rand` |
| 0x004a9dbc | 0x00534a88 | CRT floating-to-integer conversion |
| 0x004a9e23 | 0x004ac830 | `NetworkInterface::Send` |
| 0x004a9e69 | 0x004e8720 | `VCRInterface::QueueRecord` |

The two existing production callers are `BikeRace::HandlePlayerLeft`
(0x00420590, 189 bytes; call at 0x0042062a) and `HandleMenuCancel`
(0x004526b0, 632 bytes including its switch tables; call at 0x004527ef).
The former replaces a departed followed racer; the menu exit path marks the
local racer out of the race before selecting a replacement. Both complete
emitted callers remain strict matches in the focused test.

## Iterator connection and dependencies

Only the picker's two iterator invocations change from the provisional
`UnknownKrustyBikeView` method name to the existing canonical `BikeRace`
provider. Explicit receiver and result casts preserve the primary pointer
values. This uses the same boundary already established in canonical
`FollowRacer.cpp`; it does not establish complete equivalence between the
partial race/racer types. The [iterator evidence](NEXTBIKE_CONNECTIONS.md)
includes lifecycle, raw RTTI, layouts, full callers and returned-pointer
checks, including KrustyBike's separate secondary and virtual bases.

The focused test compiles the actual canonical TUs and requires full strict
matches for the 162-byte iterator, 145-byte `Send`, and 240-byte `QueueRecord`.
The new object imports these three methods, the existing `g_TrackGame`, and
normal CRT `rand`, `_ftol` and `_fltused` symbols. It also emits the existing
42-byte compiler vector-constructor helper from the included declarations;
there is no new game implementation hidden in that helper, no initializer or
EH data, and no new storage provider. The standalone does not include the
canonical `FollowRacer.cpp` implementation.

## Source correction and aggregate preservation

The original picker candidate was **346 bytes**, with **69** matching resolved
positions. It reloaded `owner->raceView` after `rand()`. Retail instead caches
that pointer and its racer count before `rand()`, then reads the slot array
through the cached pointer afterward. A scripted mutation at that boundary
demonstrated different selected racers. The corrected readable source uses a
local `selectionView`, preserving that read order. It is **342 bytes**, with
**67** matching resolved positions; this is a behavior correction, not a byte
match improvement. Ordinary CRT `rand()` is not claimed to mutate a race.

The recorded relocation-resolved picker fingerprints explicitly change from
`a9d4a6f4f3770e957a5efa00ca26dcaffe88bbd5dd8cf8011f1f50d942f0f303` to
`458488750e4bac7e802c4cfedbd485c6a2abb98a503c88e044ab31a9f57a78fc`.
`test_nextbike_connections.py` documents this exception and preserves all
seven other partial fingerprints and its existing strict cases.

The private aggregate proof uses baseline `146bd261`. Before the saved-view
correction, all **six emitted functions and eleven nondebug sections** were
unchanged under the two iterator relocation-name replacements. After the
correction, only the picker function and its containing text section change;
the other **five functions and ten sections**, including their complete bytes,
flags and named relocations, remain identical. No filename-storage exception,
extent truncation, initializer suppression or EH omission is used. Reports
and reproducible scripts are in ignored `work/picker28/source-preservation.json`,
`check_extraction.py`, `correction-proof.json` and `check_correction.py`.
The extraction-only script documents the intermediate uncorrected extraction;
the correction script verifies the current source against the original baseline.

## Functional build review

The standalone is admitted as `reviewed-nonexact` in `config/game_build.json`.
The isolated authentic source link in `build-w50v7g1v` compiles 217/217
translation units and reduces unresolved names from 373 to 372: the picker
name disappears with no new missing symbol. All 216 previously included
production objects preserve their bytes and relocation destinations under the
separate contact/event connections. The game still does not link.

The source remains a **nonexact sample**. The native differential test compares
the complete separately compiled `/O2` candidate against the complete pinned
retail body in a private validation executable. The only patched instructions
are the six decoded direct-call operands; each original opcode and destination
is checked before replacement. The executable uses an unchanged extraction of
the canonical iterator, independently proved strict over all 162 bytes both
as a helper and in its real source TU, and the actual linked VC6 CRT conversion.
Random values, network send and replay queue behavior are scripted. The
production source never maps or dispatches into retail code.

**4,270 scenarios** compare complete initialized object/array storage, callback
order and arguments, racer and receiver identities, cursor outputs, masked FP
status, completion, C++ exceptions and native access violations. Coverage
includes zero/single/multiple players, AI and mode-4 iteration, blocked UI,
null slots/AI arrays, duplicate entries, host and replay gates, all four FP
rounding modes, and random values from 0 through 32767 including index edges.
The counting loop accepts only `outOfRace == 0`; the random retry loop skips
only `outOfRace == 1`. Values 2 and -1 are deliberately tested. Array storage
also accommodates the real iterator's load before its end-of-range check.

Mutation and exception cases cover every scripted boundary on representative
paths. They include changing the owner view across `rand()`, changing the saved
view's slots/count, setting all racer flags during random selection to exercise
the bounded retry, changing global/network/recorder pointers, and changing the
followed racer or replay flags during `Send`. The old chosen ID in the packet
and the newly read ID in `QueueRecord` are checked separately. Independent
outcome assertions cover zero/one/multiple-racer selection, wraparound, irregular
flags, ignored send/queue results, gates and the saved-view correction.

### Packet initialization boundary

The packet is twelve bytes, but retail initializes only byte **+4** to zero
and DWORD **+8** to the followed racer's network ID. The other seven bytes are
unspecified stack contents. Production does not initialize them merely to
make a comparison pass. The initial `Send` observation reads only those five
defined bytes, while also checking the twelve-byte length and other arguments.
Ordinary replay observations apply the same mask. Additional cases make the
scripted `Send` initialize all twelve bytes to a distinct pattern and require
`QueueRecord` to observe that complete pattern through the same packet pointer.

These are finite dependency-boundary tests, not live network or replay-file
integration. They do not prove arbitrary corrupt counts/arrays safe, actual
concurrent races, real packet delivery, or equality of the seven unspecified
bytes. Deliberate invalid-pointer cases compare observed faults rather than
introducing recovery or cleanup absent from retail. Link, startup and racing
are separate completion gates.

With native `VC6_ROOT` and private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_followed_racer_picker_behavior.py -v
python -m unittest discover -s tests -p test_nextbike_connections.py -v
```
