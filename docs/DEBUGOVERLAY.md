# D3DIMSoultreeModifier.cpp and DebugOverlay.cpp

These are the units between D3DIMSoulTree.CPP and dirlist.cpp. The two
physics units in the same stretch, D3DIMSoultreeMotnctrl.cpp and
D3DIMSoultreeShadow.cpp, are validated by the physics runner; see
[PHYSICS_VALIDATION](PHYSICS_VALIDATION.md).

| Unit | Extent | Where |
|---|---|---|
| D3DIMSoultreeModifier.cpp | `0x00445240..0x0044545f` | `src/reconstructed/D3DIMSoultreeModifier.*` |
| D3DIMSoultreeMotnctrl.cpp | `0x00445460..0x00446830` | `samples/physics/motion/` |
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

`0x00447910` stores the window handle (the CreateWindowExA result, caller
`0x004a0d30`) in `0x0059ada4`. It is placed here by adjacency only (tier
3): Shadow's `$E` pairs end the previous unit, and its global follows
Shadow's data.

Near misses (`samples/render/DebugOverlayNearMisses.cpp`): the GDI glyph
builder `0x00447a00` and the set-up `0x00447de0`. Both differ in register
assignment.

Not attributed: `0x00448560..0x00449e5f` has no source literal and no RTTI.
It includes the TrackGame+0x33fc constructor `0x00448960` and the
keyboard-layout code before dirlist.cpp.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00447e90` `SetRowText`
- `0x00448200` `DrawString`
