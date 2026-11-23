# QuarryStuntEvent.cpp

`src/reconstructed/QuarryStuntEvent.cpp`, with the class in `QuarryEvent.h`.
Names are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x00572154`; its first xref is at
  `0x004de2cd`.
- **RTTI:** the `BaseQuarryEvent` vtable at `0x0055766c` points into the
  range for slots 0, 10, 12, 14, 23, 24, 27, 29, 30 and 34.
- **MemTag strings:** "entering/exiting BaseQuarryEvent::Create" confirm
  the class name.
- **`.CRT$XCU`:** its four kVec3 `$E` are entries `0x005663e4..0x005663f0`.

Extent: `0x004de2a0..0x004e1fa7`. Quantize.cpp ends at `0x004de29f` and
RaceSound's constructor starts at `0x004e1fb0`.

Exact: 26 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- `Create` `0x004de3b0`.
- Slots 10, 14, 23, 24, 27, 29, 30 and 34. Slots 14 and 34 are bodies that
  Wrecker's and Scene's vtables also use.
- The three memory-estimate helpers, the race memory estimate `0x004e0560`,
  the message toggle `0x004e0c30` and the clock reset `0x004e1f00`.
- The eight `$E`.

Source forms needed:
- The kVec3 statics are defined between slot 12 and slot 24, where their
  `$E` code sits. Defining them at the top changes the constructor.
- The render target is read through direct casts; an inline accessor
  changes the destructor's registers.
- `0x004e0560` sums into a local and picks the field_0x560 constant with
  an if/else assignment; with the ternary inside the return VC6 keeps
  g_TrackGame in a register across the first loop and spills a count
  (frame 0xc against retail's `push ecx`, 22 of 331).

Near miss, in `QuarryStuntEvent.cpp` in retail order with a `// Not exact`
status comment ([forms tried](#slot-12-0x004e14a0-forms-tried)): slot 12
`0x004e14a0` (166 of 1756; the prologue's register choice shifts the
rest). It was a whole candidate source of the game build; the unit now
provides it.

The loader `0x004de590` (7952 bytes) moved into the unit in retail order,
from the bring-up `src/bringup/Group3_QuarryLoad.cpp` (both it and the
older sample are gone). It is not exact. It allocates and calls the
canonical classes directly: BikeRace, KrustyBikeCamera (`SetupFlyby`,
FollowCamera's `SetSubject`), Fog/FogOff/FogOn, SkyCube, Terrain,
TerrainShadow, ProjectedShadow, ParticleManager, VisibilityQuadTree,
EcoSystem, SoultreeQuadTreeRenderer, the overlays and VisualCue. Its
TU-local views cover only members that other headers leave unnamed or
protected: the physics casters, the terrain's scale, the car's ground
provider, the racers' collision objects and the camera's follow block.

The follow block is the same inline as BikeRace.h's `StartFollowing`
(three times in `0x0041eb20`). The unit keeps its own copy:
including BikeRace.h changes the tail duplication of the constructor
`0x004de2a0` (218 bytes against retail's 237).

FogOn's `Create` (`0x00486a10`, folded with ToolTip::Create) is defined
in Fog.cpp.

Linked score 0.986, 2206 instructions against retail's 2208. These match:
- the call sequence;
- the allocation sizes and `__FILE__` lines (252..1251);
- the EH states, the error paths and the 0x650 frame.

What is left is all scheduling:
- the small stack slots below 0x1c;
- register choices in the memory-budget tree (`0x004de7e0..0x004ded5e`)
  and around the fog/sky block;
- the collision loop's head (retail enters a rotated loop with a jmp and
  indexes the entry table with `[ecx + edx + 4]`);
- the racer loop's esi/edi swap.

`tools/emu_cases/quarry_load_event_objects.py` replays it against retail:
0 of 600 runs differ.

Facts the loader establishes:
- The three 0x18-byte `TextureContext` records hold the
  terrain, model and sky texture settings: +0x14 is the halving level, set
  from total/available memory, the `DriverInfo\<driver>\VideoMemoryMB`
  registry value and the "MarginPercentage"/"MinMarginKBytes" settings.
- The model record goes to Scene `0x004ea7e0`/`0x004efb20`, the race
  `0x00417ed0`, slot 27 and the podium; the terrain record to `0x005059d0`;
  the sky record to SkyCube `0x004fb230`.
- The visibility quadtree `0x0052d460` returns its GameObject base at
  +0x874.

QuarryEvent.h now types BaseQuarryEvent's fields (size still 0xa4).
TrackOverlay.h, SceneManager.h, RaceView.h and TrackGame.h gained members.
SceneManager.h names CollisionCharacter +0x210 and CarProcedural
+0x38/+0x3c for the loader's collision links.

## Slot 12 `0x004e14a0`: forms tried

Carried over from the header of the former `samples/game/QuarryStuntEventNearMisses.cpp` when its body moved into QuarryStuntEvent.cpp.

```text
// Near-miss QuarryStuntEvent.cpp candidates (BaseQuarryEvent), kept out of
// src/reconstructed/QuarryStuntEvent.cpp until they match.
//
// BaseQuarryEvent::PrepareGeometry (0x004e14a0, 1756 bytes): the
// "Atmosphere" debug page (fog and light colours with the up/down keys). The
// switch, its jump table, the fog-kind strcpy chain and all prints match;
// the prologue differs: retail loads s_DebugPage into ecx and reloads
// TrackGame+0x38 for NewPage (VC6 here reuses ecx from the null test and
// puts the page in edx), and after the first ControlInterface call retail
// loads the global before moving the result to edi. Comparison order, a
// local page, nested/early-return tests, an inline overlay accessor and
// declaration placement do not change it; nor do `nextPage++` written
// out, `!= 0`, a ternary or inline predicate for the overlay test, a
// reversed page compare, an early return before the page block, a local
// overlay pointer, or s_DebugPage non-static or address-taken. Retail
// does not keep the overlay pointer from the null test (the sibling
// BikeRace 0x0041f5e0 does), which frees ecx for the page.
// Session 25: 0..5 dead initialised locals and `down`/`up` declared at
// the top (either order) and assigned later are unchanged; 6 or more dead
// locals grow the frame.
// Big-gap pass (166 of 1760 strict, 507 of 531 aligned, first divergence
// the register choice at offset 0x29, everything after it is shifted by
// that one reload): NewPage as an inline taking the overlay or the game,
// with or without a page local, and no-argument overlay/game accessors
// are unchanged; `nextPage++; return nextPage - 1` aligns 500, the
// increment stored to s_DebugPage directly or store-then-increment 510
// (1768 bytes).
```
