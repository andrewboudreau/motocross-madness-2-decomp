# Race iterator connections

Seven production callers now call the existing `BikeRace::NextBike(int*)`
provider at **0x004204e0, 162 bytes**. The callers are EventManager, FollowRacer,
RaceSound, RaceStatus, TrackOverlay, Krusty3DObjects and KrustyBike. They retain
their partial object views for other operations and explicitly cast only at
this call boundary. There is no forwarding implementation or linker alias.

| Retired production method name | Existing caller view |
| --- | --- |
| `UnknownKrustyBikeView::NextBike` | EventManager, FollowRacer, RaceSound, RaceStatus, TrackOverlay |
| `UnknownStatsSource::NextBike` | TrackOverlay |
| `UnknownVisualCueView::NextBike` | Krusty3DObjects |
| `KbRace::NextBike` | KrustyBike |

`UnknownChatView::NextBike` remains a sample-only declaration and is not counted
as a removed production dependency.

## Receiver and result evidence

Confirmed retail instructions use ECX for the receiver, a stack pointer to the
integer cursor, EAX for the returned pointer, and `ret 4`. The complete provider
reads the race's local racer at +0x38, slot array at +0x3c and AI array at +0x40.
It returns their pointer values directly, without a base adjustment. Complete
caller byte comparisons preserve the instructions supplying ECX and consuming
EAX, including the callers' virtual-base accesses.

BikeRace's primary COL at 0x0055ac68 and its hierarchy at 0x0055ac58 identify
BikeRace, GraphicsTest, GameObject and BaseObject at displacement zero. This
confirms BikeRace's own inheritance; it does not independently identify every
provisional receiver view. That connection also uses the callers and lifecycle:
EventManager's `FindRaceView` at 0x0045d2f0 returns the event's race pointer;
QuarryStuntEvent's loader constructs that object with 0x00417c30 and supplies it
to VisualCue and the statistics overlay. `StatsOverlay::SetStatsSource` at
0x00519880 stores the same pointer. KrustyBike's loader stores its race argument
at +0x740. The accessed receiver fields agree with the canonical race layout.

The returned racer boundary remains deliberately narrow. BikeRace's racer
creation uses the KrustyBike constructor at 0x0048fa60 and the 0x1638-byte
allocation. KrustyBike has a primary subobject at zero, a character subobject
at +0x21c and a virtual GameObject at +0x160c (retail COLs 0x0055d078,
0x0055d008 and 0x0055cff0). These secondary pointers must not replace the
primary pointer returned by the iterator. The explicit result casts preserve
that primary value. The event/visual views' position at +0x0c and the
event/canonical/KrustyBike race pointer at +0x740 are checked under native VC6.
This does **not** establish full type or virtual-base layout equivalence among
`UnknownBikeRaceRacer`, `UnknownEventRacer`, `UnknownVisualCueRacer` and
`KrustyBike`; their partial declarations and existing access code are retained.

## Shared declaration

`BikeRaceCore.h` contains the existing complete canonical BikeRace declaration;
`BikeRaceNode.h` contains its existing 0x3c-byte embedded gate/probe declaration.
`BikeRace.h` includes both and continues to expose the original helper types.
The extraction preserves the 0x400-byte race, fields, virtual slots and method
signatures. Pointer-only helper dependencies are forward-declared in the core.
`UnknownFunctionFreeNodes` remains inline in `BikeRace.h`, including the
original `__FILE__` diagnostic context.

This split lets callers include the actual class without importing unrelated
collision, input and platform helper declarations from the broad header.
It is a source dependency change, not evidence that these were separate
original retail headers. Existing source attribution to bikerace.cpp and
bikerace.h is unchanged.

## Verification

With native `VC6_ROOT` and the hash-pinned private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python tests/test_nextbike_connections.py -v
```

The regression checks:

- All **338** registered strict cases in the seven caller TUs and BikeRace.
- The complete relocation-resolved hashes of **eight** existing partial bodies
  in FollowRacerNearMisses and KrustyBikeNearMisses, without claiming those
  partial bodies match retail. The picker's fingerprint now records its
  separately demonstrated [saved-view correction](FOLLOWED_RACER_PICKER.md);
  the other seven retain their original fingerprints.
- Actual COFF direct-call imports of the canonical provider and absence of the
  four retired imports; the provider's full 162-byte COFF function extent and
  strict relocation-resolved bytes. Its only referenced external is the
  existing `g_TrackGame` pointer.
- Native layouts for the receiver/result boundaries and raw retail RTTI
  displacement evidence.
- **14,400** iterator cases using the unchanged function extracted from
  canonical BikeRace.cpp. Both that execution object and the actual full
  production TU independently match all 162 retail bytes. The fixture covers
  zero/single/multiple players, blocked state, mode 4, AI counts, null AI arrays,
  null racer slots, cursor termination/reset and repeated calls, comparing
  returned identity, cursor and complete race/game storage. The blocked path
  also accepts a null cursor. The executable constructs no game or racer
  objects and makes no whole-game claim.

Retail loads a slot before testing whether the advanced cursor exceeds the
limit; the fixture allocates valid storage for these sentinel reads. It does
not claim that arbitrary invalid cursor or array pointers are safe.

The isolated real source build compiled **195/195** translation units and
reduced unresolved names **558 to 554**, exactly the four names above, with
none added. Link, startup, menus and racing remain unfinished. The unrelated
TrackOverlayNearMisses sample already failed compilation before this slice
because its GUIManager declaration was incomplete; it is not included in the
successful caller-compilation claim.
