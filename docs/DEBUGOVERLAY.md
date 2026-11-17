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

Near miss: the GDI glyph builder `0x00447a00` (`LoadFont`), in DebugOverlay.cpp
in retail order with a `// Not exact` status comment. Its body matches; in the
unit (with the includes the font builder needs) VC6 keeps one shared epilogue
and emits retail's 980 bytes, but places that epilogue after the first failure
return instead of at the end (463 of 980). The standalone sample copied the
epilogue into the returns (999 bytes, 460). See
[LoadFont: forms tried](#loadfont-forms-tried). The set-up `0x00447de0`
matches once `i` is initialised before `y = e`.

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

Visibility's object at `0x0068ab90` uses `PeakHold`, the same implementation
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

## LoadFont: forms tried

Carried over from the header of the former `samples/render` near-miss
file, which compiled LoadFont alone (999 bytes):

DebugOverlay::LoadFont (0x00447a00, 980 bytes; ours 999):
the GDI font builder. The glyph loop is a top-tested `while (c < 256)`
(a do-while lets VC6 peel the first u0 = 0 and reuse u1 as the next u0),
and the GetDC/ReleaseDC/Slot8 failures share one `failed: return 0`.
Zero in ebx, the run length in ebp and the glyph pointer in edi match.
Computing v0 and v1 before the row pointer (any order with `row` after
v0) lets the row pointer share the v0 temporary's slot, so ebp is saved
only around the GDI section as in retail and `c` is read into eax once:
strict 79 -> 460, 28 -> 17 differing instructions, all of them in the
returns. `if (CreateTextureSurface) return 1;` cross-jumps the last
failure into the CreateFontIndirect failure, but VC6 then copies the
epilogue into that block and the success path; retail keeps one
epilogue that both `xor eax, eax; jmp` blocks and `mov eax, 1` reach.
goto/result-variable/if-else forms of the tail compile identically.
Synthetic VC6 tests (2026-10-20): the EH epilogue is copied into
constant-return blocks when it is short (frame below 0x80, so disp8
operands, and a bare `ret`) and shared with a larger frame or `ret N`;
a register saved only around a region (ebp here, as in retail) brings
the copies back even with the 0x278 frame. Unchanged here: the `!Create
goto` tail (retail's block order, three copies), `?:`, `!= 0`, Release
via goto, `return (int)handle`, an else arm for the Release block, the
GDI section in its own scope or behind `if (handle)`. A returned
variable (not a constant) keeps one epilogue but loads it from a register.
Early-failure forms (s14, the ConnectUsingLobby lever): the GetDC and
ReleaseDC failures as their own `return 0` with the tail written
`if (!Create) return 0; return 1;`, `if (Create) return 1; return 0;` or
`?:` give 33, and also testing a loaded texture first gives 43.

k-scan (s22): compiled behind 0..63 prepended typedefs, 0x00447a00 has the
same code at every count, so its difference is not a declaration-count
effect (docs/VC6_OPERAND_ORDER.md section 2).
