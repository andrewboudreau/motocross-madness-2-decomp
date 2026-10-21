# D3DIMSoultreeModifier.cpp and DebugOverlay.cpp

These are the units between D3DIMSoulTree.CPP and dirlist.cpp. The two
physics units in the same stretch, D3DIMSoultreeMotnctrl.cpp and
D3DIMSoultreeShadow.cpp, are validated by the physics runner; see
[PHYSICS_VALIDATION](PHYSICS_VALIDATION.md).

| Unit | Extent | Where |
|---|---|---|
| D3DIMSoultreeModifier.cpp | `0x00445240..0x0044545f` | `src/reconstructed/D3DIMSoultreeModifier.*` |
| D3DIMSoultreeMotnctrl.cpp | `0x00445460..0x00446830` | `src/krusty2/motion/` |
| D3DIMSoultreeShadow.cpp | `0x00446840..0x00447908` | `src/krusty2/shadow/` |
| DebugOverlay.cpp | `0x00447920..0x00448559` | `src/reconstructed/DebugOverlay.*` |

**D3DIMSoultreeModifier.cpp.** Evidence: the `__FILE__` literal at
`0x00568b24` (debug-allocator lines 26-80) and RTTI
`D3DIMSoultreeModifier : GraphicsTest : GameObject` (vtable `0x0055144c`;
slot 27 is `_purecall`). The constructor at `0x00445240` is the first
function of the unit. Exact: all 7 functions. Slot 8, `0x004452e0`, is the
same body that LightManager's vtable uses.

**DebugOverlay.cpp.** Evidence: the `__FILE__` literal at `0x00568d04`
(line 59) and RTTI `DebugOverlay : GameObject` (vtable `0x0055166c`), whose
overridden slots 0, 14 and 25 lie in this range. The overlay holds pages of
0x98-byte text rows. It draws them as textured quads from a glyph table
(+0x26d0) that `0x00447a00` builds with GDI. Exact: 12 functions,
including the 748-byte text drawer `0x00448200` and slot 14.

`StoreMainWindow` (`0x00447910`) stores the window handle (the CreateWindowExA result, caller
`0x004a0d30`) in `0x0059ada4`. It is placed here by adjacency only (tier
3): Shadow's `$E` pairs end the previous unit, and its global follows
Shadow's data.

Near miss (`samples/render/DebugOverlayNearMisses.cpp`): the GDI glyph
builder `0x00447a00` (`LoadFont`), whose body matches; only the return
epilogues differ (VC6 copies the epilogue into the returns, retail shares one). The
set-up `0x00447de0` matches once `i` is initialised before `y = e`.

Not attributed: `0x00448560..0x00449e5f` has no source literal and no RTTI.
It includes the TrackGame+0x33fc constructor `0x00448960` and the
keyboard-layout code before dirlist.cpp.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00447e90` `SetRowText`
- `0x00448200` `DrawString`

## Scene profiling connections

Terrain, VisibilityQuadTree and SoultreeQuadTreeRenderer use the shared
`DebugOverlay` declaration for their profile pages. Their receiver comes from
the Game member at `+0x38`; retail RTTI places Game at offset zero within
TrackGame. Page allocation increments `DebugOverlay::nextPage` at `+0x26c0`.
The complete callers match retail after replacing the local overlay views and
free-function declarations with `Print` (`0x00447f40`, 94 bytes) and `Title`
(`0x00447fa0`, 94 bytes). Both are variadic cdecl members: the receiver remains
the first stack argument, and the caller still removes all arguments.

Visibility's object at `0x0068ab90` uses `UnknownPeakHold`, the same implementation
as Game's profile counters. The initializer at `0x0052d330` passes 5000 to the
constructor at `0x004cb670`; the object stores a timestamp, peak and hold interval
at offsets 0, 4 and 8. This requires 12 bytes of writable storage. The former
empty `VisibilityStat` declaration reserved only one byte. The canonical type
fixes that storage defect and connects `Record` (`0x004cb6b0`) and `Peak`
(`0x004cb690`). These names describe observed behavior; the original class name
and owning source file remain unknown. Visibility also uses the existing
unsigned `ReadClock` return (`0x004bfa80`) and unsigned elapsed subtraction.

`tests/test_scene_profiler_connections.py` compiles the actual three consumer
and three provider translation units with VC6 SP3, verifies six complete retail
provider bodies and four complete caller/initializer bodies, checks imports
and unique definitions, and guards the actual timer's writable COFF storage.
Its native layout fixture checks the owner/member offsets. Focused validation
preserves all 72 strict cases across the three consumer units; Terrain's two
partial cases are unchanged. Ten formerly missing symbol spellings now reach
existing providers. This establishes these profiling connections, not complete
scene rendering or a linked game.
