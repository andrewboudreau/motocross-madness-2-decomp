# Race event clock and message providers

Race restarts, the loading dialog and sound-option controls now call the
existing `BaseQuarryEvent` implementations. This retires two unresolved method
names, with six unchanged direct call instructions.

| Method | Complete retail body | Callers |
|---|---|---|
| ResetClock | `0x004e1f00`, 9 bytes | BikeRace::RestartRace and LoadingDlg::HandleEvent, two calls each |
| ShowOnOffMessage | `0x004e0c30`, 175 bytes | RaceSound::OnControlPress, two calls |

Both providers are complete strict matches with resolved relocations. ResetClock
writes zero DWORDs only at receiver `+0x70` and `+0x74`. ShowOnOffMessage loads
the string identifier and on/off text, creates a temporary message, and passes
it to the text queue at `+0x6c`. The original calling convention, receiver
address, arguments and cleanup remain unchanged.

## Receiver evidence and declaration scope

`TrackGameViewOwner` remains a provisional view spanning several race modes.
The connection uses the verified common prefix; it does not claim that every
possible owner has one recovered original class. Its message queue and clock
fields agree with BaseQuarryEvent at `+0x6c/+0x70/+0x74`.

EventManager's mode-zero construction stores the result of
`BaseQuarryEvent::Create` into `quarryEvent`; mode two stores NationalRace into
`nationalEvent`. Hash-pinned RTTI independently identifies BaseQuarryEvent's
GameObject base and NationalRace's BaseQuarryEvent base at offset zero with no
virtual-base displacement. FindRaceMode returns the selected stored owner
pointer directly. The tag and other provisional views are not reclassified.
Full retail caller bytes, decoded direct destinations and field-layout checks
establish these invocations independently of the tiny clock body's address.

The complete BaseQuarryEvent declaration and progress-callback typedef move
verbatim into `BaseQuarryEvent.h`. `QuarryEvent.h` includes that header at the
original declaration location. This lets callers use the actual provider
without importing the unrelated provisional VisualCue definition, which
conflicts with the canonical VisualCue used by the BikeRace sample aggregate.
No class fields, virtual slots, inheritance or provider bodies change.

The two unused method declarations in TrackGameViewOwner remain with that
provisional view. Removing them changed the order of independent MOV/XOR
instructions in the existing CreateRacers partial. Keeping the view intact and
connecting only the actual call sites preserves the entire emission. No old
method symbol is emitted by the affected caller or sample objects; no aliases,
forwarding bodies or link substitutions supply those names.

## Validation

Private same-path baseline/current builds from `146bd261` cover all three
callers, the provider, EventManager, TrackGame and all three affected sample
aggregates. All 1,773 nondebug sections and 12,442 relocation destinations are
preserved under exactly the two documented method renames, including bytes,
flags and sizes.
There are no data or filename exceptions. Evidence is in
`work/game-build/race-event28/{objects,renames}.json`.

`tests/test_race_event_connections.py` checks **397 strict bodies**, **25 full
partial COFF fingerprints**, six independently decoded retail call boundaries,
the known RTTI bases and eleven overlapping field offsets. Partial fingerprints
preserve whole function emissions and relocation identities; they do not claim
complete retail matching or promote unresolved sample bindings to strict proof.
Only recognized physical source-path suffixes are normalized in portable test
fingerprints; the separate whole-object comparison needs no normalization.

The native test executes the unchanged source ResetClock body after requiring
both original and extracted emissions to match all nine retail bytes. **256
full-storage cases** check both writes, repeated calls, and every surrounding
byte of a guarded provisional owner. It does not script or replace the provider.
Message-display integration is not exercised by this clock fixture.

The isolated real link in `build-fidfd7u5` compiles **216/216 translation units**
and changes unresolved names **377 → 375**, with exactly those two removals and
no additions. At this slice the game was still unlinked (it has linked and raced since; see [GAME_BUILD](GAME_BUILD.md)); loading-screen completion, menus and
racing still require integrated runtime verification.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_race_event_connections.py -v
python tools/build_game.py -j6
```
