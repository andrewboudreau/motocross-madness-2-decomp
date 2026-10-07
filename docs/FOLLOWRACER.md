# FollowRacer (followed-racer and ghost helpers)

Canonical reconstruction: `src/reconstructed/FollowRacer.*` (3 exact). The
two remaining functions are near misses in
`samples/race/FollowRacerNearMisses.*`. The unit has no RTTI, vtable or
`__FILE__` literal; the file name and every function name are ours (tier 3).

## Extent

`0x004a9aa0..0x004aa001`, between Motnctrl.cpp (last `__FILE__` xref
`0x004a9a87`) and MSZoneInterface.cpp (first xref `0x004aa36c`). Five
functions, all `thiscall`:

| VA | Size | Owner (object) | State |
| --- | --- | --- | --- |
| `0x004a9aa0` | 617 | ghost bike (TrackGameViewOwner+0xdc) | exact |
| `0x004a9d10` | 13 | ghost bike | exact |
| `0x004a9d20` | 340 | TrackGameViewOwner | near miss, 20% |
| `0x004a9e80` | 272 | TrackGameViewOwner | near miss, 95% |
| `0x004a9f90` | 113 | TrackGameViewOwner | exact |

Callers (confirmed xrefs): BikeRace.cpp `0x00422dec` applies replay deltas
through `0x004a9aa0` and `0x00420a91` hands the recorder to `0x004a9d10`;
DlgProcs.cpp `0x0042062a`/`0x004527ef` call the picker `0x004a9d20`;
KrustyBike.cpp `0x0048db13`/`0x00497986` call the switch `0x004a9e80`, which
calls `0x004a9f90`. The grouping of the two objects into one unit is
provisional (proximity only).

## Behaviour (decoded)

- `0x004a9aa0` takes a kind-0x10 replay delta (`UnknownBikeRaceGhostDelta`:
  a kind byte, four signed byte triples and a step byte) and a ghost part.
  Two triples scale by 0.234375 (`0x00554714`) into the position-like
  vectors at +0x5d4/+0x5e0, two by pi/64 = 0.049087387 (`0x0055534c`) into
  +0x5ec and the three scalars +0x5f8/+0x5fc/+0x600. The step byte is
  halved; an odd step byte means the half is in units of eight. The step is
  stored in the part's `field_0x02`, accumulated into +0x604, and the whole
  state is copied into the part. It has the shape of KrustyBike.cpp
  `0x004933e0`.
- `0x004a9d10` stores the `KrustyVCR*` at +0x708.
- `0x004a9d20` runs on the network host only. It counts racers whose +0x4a0
  is clear: none -> the view's own racer (+0x38 of the race view), one ->
  that racer, more -> a `rand()`-picked index skipping flagged racers (at
  most `count` retries). It sends message 0x87 (`UnknownBikeRaceVcrFollow`,
  12 bytes, the racer's +0x11bc id) to the local player and, when a replay
  is being recorded, queues the same record.
- `0x004a9e80` switches the followed racer: the previous racer banks its
  run (+0x75c into +0x764, the best of +0x75c/+0x760 into +0x760), both
  racers receive their own virtual slot 50 call `(1, 5.0f, 0)`, every other
  racer hears about the change (`0x004a9f90`), the ghost's +0x10a flag is
  cleared, +0xac restarts at 5.0 and the view's own racer taking over from
  someone else scores 2500 through the score board at race view +0xbc.
- `0x004a9f90` loops the race view's racer iterator (`0x004204e0`) and
  calls `0x00496f90` on the new racer for each other racer, on each other
  racer for the previous one (unless it is that racer) and for the new one.

## Source forms that mattered

- The delta scaling multiplies each signed byte by the float constant and
  builds a `Vector3` with the three products; the step halving is an
  `if`/`else` on `step & 1`, and the part's step is written before the
  accumulation reads it back from the part.
- `0x004a9f90` is a `for` loop whose increment is the iterator call, with a
  `continue` for the new racer itself.

## Near misses

- `0x004a9d20` (340 B, 346 B candidate, 20%): every instruction is retail's
  with two register pairs swapped. Retail keeps `this` in edi and the
  still-racing count in esi; retail's zero register ebp is the retry
  counter, while VC6 here merges the zero with the last racer seen (so the
  loop compares with `test` instead of `cmp reg, ebp`). All 24 declaration
  orders, hoisting the locals above the host check, `for`/`while`/assign-in-
  condition loops, a cached racer array, a single pick variable, renaming
  and extra declarations in the TU leave both swaps.
- `0x004a9e80` (272 B, 95%): one scheduling difference. Retail pushes the
  three arguments of the first slot-50 call before the `fstp` into +0x760;
  VC6 here stores after the first push. The if/else, max-helper,
  local-result, int-zero, double-literal and statement-order forms move the
  compare or the stores instead. The best-of compare is the EventManager.cpp
  idiom (two locals compared, the fields in the ternary arms).

## Reproduction

```bash
PYTHONPATH=. python tools/run_calibration.py --exe work/game/mcm2.exe --compiler vc6 --vc6-root "$VC6_ROOT"
```

The five cases are the `FollowRacer.cpp` and `FollowRacerNearMisses.cpp`
entries. Shared headers touched for this unit: `RaceView.h`
(`UnknownFunction4925a0`/`UnknownFunction496f90`, `UnknownScoreBoard`, race
view +0xbc), `TrackGame.h` (ghost fields +0x10a/+0x708, the method
declarations, view owner +0xac as an int/float union) and `BikeRace.h`
(`UnknownBikeRaceGhostDelta`, `UnknownBikeRaceVcrFollow`, the ghost record's
float fields). The racer's slot 50 is viewed by a struct local to the
near-miss sample: 50 placeholder slots in `RaceView.h` flipped a register
choice in TrackGame slot 1 (`0x00520ab0`, header-content chaos; removing
`UnknownScoreBoard` flips it too), and the +0xac float on its own turned
the StatsOverlay race panel's `%d` (`0x0051a560`) into a float load.
