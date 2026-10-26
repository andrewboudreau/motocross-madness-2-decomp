# Race detail connection

The two detail-key branches in `BaseQuarryEvent::OnControlPress` now call the
existing `BikeRace::SetDetailLevel(int)` definition through an explicit primary
pointer cast. The existing `BikeRaceCore.h` include supplies the declaration.
No class layout, provider body, allocation, shared header or table changes.
The provisional `UnknownKrustyBikeView` is not identified as a complete BikeRace
class by this bounded connection.

## Receiver and ABI evidence

The complete 7,952-byte retail loader at `0x4de590` is decoded from its actual
entry. Its allocation at `0x4df9db` requests `0x400` bytes; the result is the
unchanged ECX receiver of the constructor call at `0x4df9ff`. The primary result
is stored at event `+0x34`, reloaded for `BikeRace::Create` at `0x4dfabf`, and its
returned primary pointer is stored back at `0x4dfac6`. The constructor's complete
632-byte source body is strict and writes vtable `0x550d24`. Its raw COL at
`0x55ac68` names BikeRace and records offset zero. The setup's raw entry saves
ECX in EBP and its successful return moves EBP to EAX. Setup is not promoted or
claimed strict by this change.

Both direct calls (`0x4e128e`, `0x4e139f`) reload event `+0x34` into ECX and push
the existing EDI integer. The complete 1,980-byte caller remains strict. The
240-byte provider at `0x423790` reads that stack integer and returns with
`RET 4`. No secondary GameObject conversion occurs at either boundary.

The provider reaches the bike model through racer `+0x3bc`, and its rider model
through racer `+0x5c4` then rider `+0x1a0`. These existing layout views remain
bounded declarations. Actual-header layout assertions and full provider bytes
protect their offsets. Local racer processing always occurs; race mode 4 skips
AI models, while network models follow the network-player byte count.

## Complete preservation

The canonical object preserves all 86 nondebug sections and 393 relocations
under exactly one external name replacement:

```
?SetDetailLevel@UnknownKrustyBikeView@@QAEXH@Z
?SetDetailLevel@BikeRace@@QAEXH@Z
```

The standalone `QuarryStuntEventNearMisses.cpp` sample preserves all 23 sections
and 95 relocations without a mapping. It does not include the canonical source
and has no changed calls. All bytes, section flags, relocation kinds, addends and
destinations are retained. No filename regions are excluded. The retained
whole-object pins enumerate only these two source filename spellings and were
checked against fresh actual-root compilation as well as the private mirror.
Both callers are compiled without project include paths in the retained test,
as required by standalone calibration.

The existing Eco light test's canonical QuarryStuntEvent pin is updated only
after reproducing the published old pin and applying the exact method-name
mapping. Its other records, including the standalone sample, are unchanged.
The six existing race-event partial/default fingerprints are independently
reproduced before and after without any changes to their pins.

## Native validation

`test_race_detail_connections.py` makes 35 strict comparisons: all 26 canonical
caller cases, the constructor, and both canonical and extracted copies of the
four complete runtime bodies. The executable uses actual extracted source for
SetDetailLevel and the three real model helpers: ApplyLodSettings (81 bytes),
SetLodThreshold (19), and SetUseLighting (23). Extraction preserves every body
byte and named relocation. It links the actual `GraphicsState.cpp`; both
160-byte writable preset tables match retail initialized data, and the active
pointer is actual zero-filled shared storage. No model helper is scripted.

Native execution compares source behavior with independent state and fault
oracles; it does not execute a mapped retail graph. Full strict comparisons
supply the separate retail-byte evidence. The 4,320 successful cases cover all
ten valid levels, both renderer selections, three race modes, four AI counts
(including a negative count), three network counts, separate and shared model
pointers, and three initial byte patterns. The oracle uses independently pinned
reciprocal result bits and expected fields rather than repeating provider calls.
It compares complete prepared race, game, racer, rider, model and pointer-array
storage with guards. Unselected models and both tables must remain unchanged.

Another 200 cases place the first model-field store on a protected page, once
at each of ten visit positions for each level and renderer. Independent oracles
require a write access violation at that exact address, the selected active
pointer, all prior completed model updates, and no later updates. The accessible
guard page is compared completely. Ordinary cases must not fault.

These are prepared-storage tests, not constructed object lifetimes or complete
keyboard/event execution. Level inputs are restricted to the real table range
0..9, with bounded valid arrays and counts. Floating-point exceptions are masked,
rounding is nearest and precision is 64-bit. Each isolated case resets x87 state:
a write access violation can leave a temporary on the register stack. Control
bits are checked, but TOP, saved exception context, unmasked traps and resumption
are not claimed equivalent. No rendering or full race session is simulated.

Run with authentic VC6 and the hash-pinned private image:

```
python -m unittest test_race_detail_connections -v
```

Build `work/game-build/build-vnogj0uu` compiles 274/274 translation units and
removes exactly the provisional detail method spelling: **178 -> 177**, none
added. All 274 production objects preserve their complete 10,868 nondebug
sections and 46,268 relocations under this single name replacement, without
filename exclusions. Source/header hashes and compiler inputs are checked;
no translation unit, layout or dependency is added. Every prior calibration
and physics score, full extent and strict result remains unchanged. Actual-path
race-detail, Eco light and race-event tests pass. The game still does not link;
startup, menus and racing remain unverified.
