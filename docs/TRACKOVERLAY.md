# TrackOverlay

The literal `__FILE__` `D:\aardvark\VC\krusty2\TrackOverlay.cpp`
(`0x00575290`) is referenced from `0x0051b550`..`0x0051d850` and from the
unwind funclets at `0x0054ecbe`..`0x0054ed5d`. The reconstructed source is
`src/reconstructed/TrackOverlay.h`, `TrackOverlay.cpp` and
`TrackOverlay.bindings.json`. TextQueueOverlay and TextQueueMessage are
declared in `RaceView.h`, and the Overlay base is in `Overlay.h` and
`OverlayRect.h`. Class names come from RTTI. Every member and function name
is provisional.

## Extent

The TU covers `0x00518720..0x0051ee90`. TrackRecord.cpp starts at
`0x0051ee90` (see [TrackRecord](TRACKRECORD.md)).

- **Confirmed.** No `__FILE__` reference falls after `0x0051d850`.
- **Strong inference.** The four per-TU `$E` initializers of the Math3D.h
  axis vectors sit mid-TU at `0x0051dba0..0x0051dcdc`. They build (0,0,0)
  at `0x0068a410`, (1,0,0) at `0x0068a420`, (0,1,0) at `0x0068a430` and
  (0,0,1) at `0x0068a400`. ChatOverlay methods appear on both sides of them.
- **Strong inference.** Only TrackOverlay code reads `0x0068a410`, and the
  file statics `0x0068a444` and `0x0068a448` follow it in .bss. Together
  these place `0x0051dce0..0x0051ee90` (ChatOverlay and ChatInput) in
  this TU.

## Classes

| Class (RTTI) | Base | Vtable | Size |
|---|---|---|---|
| InstrumentOverlay | Overlay | `0x00558590` | 0x188 |
| NameOverlay | Overlay | `0x0055860c` | 0x174 |
| StatsOverlay | Overlay | `0x00558680` | 0x9b8 |
| DropTextOverlay | GameObject | `0x005586f0` | 0xc8 |
| TextQueueOverlay | GameObject | `0x00558760` | 0x58 |
| RadarOverlay | Overlay | `0x005587d0` | not recovered |
| ChatOverlay | Overlay | `0x00558854` | 0x3e0 (the allocation at `0x004e08e4`) |

ChatInput (0x1c4 bytes, no vtable) lives at ChatOverlay+0x13c. It
holds the line being typed and four history entries.
ChatOverlay+0x140 is the chat rectangle (centred, 256 x 68), +0x150 the cue
rectangle the loader is given, and +0x1c8/+0x1cc two plain Overlay side
panels created only when the screen is wider than 512 pixels (allocations
at lines 2284 and 2294).

BikeRace now uses this same `ChatOverlay` declaration for its borrowed pointer
at `+0x19c`, replacing the provisional `UnknownBikeRaceOverlay19c` view.
Retail directly calls `HandleChatKey` at `0x0041f5b1`, `HandleChatInput` at
`0x0041f675`, `HideInputLine` at `0x0041e135`, `ShowInputLine` at `0x0041f731`
and `AddMessage` at `0x00420440`. The event's `0x3e0` allocation at
`0x004e08e4` calls the ChatOverlay constructor at `0x004e090a`; the constructor
installs the primary vtable above. Both caller and provider use the typing
flag at `+0x3dc`. The existing integer text argument of `AddMessage` is retained
with an explicit pointer cast; its provider interprets those bits as a string.
This connection changes no ownership or lifetime behavior. All 136 strict
BikeRace and TrackOverlay calibration cases remain exact. The native VC6
regression in `tests/test_chat_overlay_connections.py` checks the retail calls,
compiled layouts and imports against the real provider object.

## Status

97 functions are exact under `vc6_o2_mt`, with every relocation bound:

- InstrumentOverlay: `0x00518720`..`0x00518cc0`, 7 functions.
- The overlay rectangle constructors `0x00518d50` and `0x00518d60`.
- NameOverlay: `0x00518d80`..`0x005190e0`, 8 functions, among them the
  visibility test `0x00519000`.
- StatsOverlay: `0x00519370`..`0x00519980`, 7 functions, the score
  comparator `0x005199f0`, the view-mode redraw `0x005198a0`, the panels
  `0x00519e10`, `0x00519ef0`, `0x0051a480`, `0x0051a560` and `0x0051aa40`,
  and the loader `0x005194b0`. The loader's LOGFONT sits below the two
  rectangles (retail's frame) once the face-name copy is written in each
  branch: the second `lfFaceName` address use raises its uses per byte
  above the rectangles' (docs/VC6_FRAME_LAYOUT.md fact 2). Retail stores
  lfItalic and lfHeight after the other fields and the source rectangle's
  top/bottom before left/right; the source writes them in that order.
- DropTextOverlay: `0x0051ae80`..`0x0051b1f0`, 5 functions.
- TextQueueMessage and TextQueueOverlay: `0x0051b200`..`0x0051b670`, 11
  functions.
- RadarOverlay: `0x0051b690`..`0x0051c460`, 12 functions, among them the
  zoom keys (slot 23, `0x0051bb60`): the dead `largeMap` test stays only with
  the scale written `(mapRadius / 180.0f) / zoom`, which schedules the
  fld/fmul/fidiv before it (docs/VC6_OPERAND_ORDER.md section 3).
- The `$E` initializers and their stubs: `0x0051dba0`..`0x0051dcdc`, 8
  functions.
- RadarOverlay's gate drawing `0x0051cb20` (the current and next gate as
  a line between their posts).
- ChatOverlay: `0x0051cda0`..`0x0051e800`, 17 functions, among them the
  typed-key handler `0x0051da30`, the input/history redraw `0x0051de10`
  and the loader `0x0051cf80`. The loader's side panels write the source
  bottom as `source.top + (largeLayout ? 68 : 34)`. With the literal
  `largeLayout ? 137 : 103` VC6 loaded largeLayout into ecx and formed the
  right panel's width in eax, where retail has eax and ecx. No statement
  order, ternary spelling or width local changed that.
- ChatOverlay's `CollectRacers` `0x0051d730` and `DrawNameTag` `0x0051e3f0`,
  which moved in from their samples (see the near misses below).
- ChatInput: `0x0051ea50`..`0x0051eb40`, 7 functions.

Their calibration cases are under `# TrackOverlay.cpp` in
`tools/run_calibration.py`.

`0x0051e910` (redraws one name tag or all) is exact now that
`Overlay::UnknownFunction4b6880` takes an `unsigned short` colour. Retail
builds the 4444 grey in 16-bit registers and pushes it without
zero-extension.

### Near misses

Four bodies are still near misses. They sit in [TrackOverlay.cpp](../src/reconstructed/TrackOverlay.cpp) (retail `TrackOverlay.cpp`) in retail
order, each with a `// Not exact` status comment; the source forms tried are
in [Near misses: forms tried](#near-misses-forms-tried). Two former samples
moved in with them and are exact calibration cases now:

- `0x0051e3f0` (draws one name tag, 974 bytes): the growing arm of
  the width test returns through its own `SelectObject(dc, font)` and the
  positions block reads `racers[index]->position` directly; both move the
  scratch-register rotation, not the instruction shapes. See
  [CHAT_NAME_TAG_DRAW.md](CHAT_NAME_TAG_DRAW.md) for the shared flag at
  `0x68a444`, the actual CRT and finite native evidence.
- `0x0051d730` (racer name tags, 586 bytes): both `Create` calls read
  `renderContext` directly instead of through `Target()`. Its iterator call
  casts to `BikeRace` like the other callers ([NEXTBIKE_CONNECTIONS.md](NEXTBIKE_CONNECTIONS.md)).

The near misses:

- `0x0051bed0` (the map update, 1160 bytes; 1129 match): places every
  racer's name tag through the view's ChatOverlay (+0x19c) and draws the
  map dots, then calls `0x0051c720` (modes 2, 3) or `0x0051cb20` (modes
  1, 5). Calls, loops and stores match; retail allocates the int-to-float
  slot of the clip result right above `x` and computes the distance
  products before the pushes of the `0x0051d9c0` call.
- `0x00519a20` (standings, 1002 bytes; candidate 1004, 605 match): retail
  keeps the row comparison index in edx and `this` in its spill slot. VC6
  here does that only when the index is read after the loop, which adds a
  redundant compare. See [STATS_STANDINGS_DRAW.md](STATS_STANDINGS_DRAW.md).
- `0x0051c4f0` (line/circle intersection, 560 bytes; 404 match): frame
  slot assignment of the float temporaries differs.
- `0x0051c720` (track outline, 1016 bytes; candidate 1002, 254 match):
  walks the track graph (`Track.h`: TrackNode, TrackSegment, TrackListItem;
  the view's +0x48 is the Track) and draws each node's segment edge clipped
  to the map circle. Code and stores match with the node read through a
  reference to the list entry, an if/else for visited nodes and a
  `do ... while` segment walk; the frame (0x8c in retail, 0x74 here) and
  two store/reload orders differ.

All four agree with retail under `tools/emu_diff.py` (`radar_draw_racers`,
`stats_draw_standings`, `radar_line_circle`, `radar_draw_track_outline`;
400 runs each), as does `chat_overlay_collect_racers`.

### Not reconstructed

- **Inline-asm float-to-int.** `0x00518f30`, `0x0051af00` and `0x0051b070`
  use a `fistp` helper that the project rules exclude, so none can be
  byte-exact. `0x00518f30` (NameOverlay slot 10, the name-tag fade) is
  reconstructed for behaviour in `src/reconstructed/TrackOverlay.cpp`, with
  `src/bringup/X87FloatToInt.h` standing in for the helper. It is not a
  calibration target. `0x0051af00` is linked from
  `samples/track/DropTextOverlaySetup.cpp` with `/QIfist`
  ([GAME_BUILD.md](GAME_BUILD.md#inline-fistp-rounding-samples)); `0x0051b070`
  has a candidate sample in `samples/track/DropTextOverlayTick.cpp`.
- `0x00518c20` is not a function start (inside the InstrumentOverlay
  destructor `0x00518c00`).
- `0x0051eae0` is a 3-byte `mov eax, ecx; ret`: ChatInput's line
  accessor `GetLine` (exact). Its only callers are `0x0051da30`'s two calls
  (`0x0051dae5`, `0x0051db09`), and no vtable lists it.
- Every RadarOverlay function is now exact or a documented near miss.

## Codegen notes

- Every GetDC/ReleaseDC redraw returns 0 through one block: retail matches
  `if (GetDC(...) != 0) goto fail; ... if (ReleaseDC(dc) != 0) { fail: return 0; }`.
  Early `return 0` statements emit a separate epilogue. The same `goto fail`
  shape gives 0x005198a0's shared `xor eax, eax`.
- The row comparisons of the StatsOverlay panels are `for (...) if
  (strcmp(...) != 0) goto draw; return 1; draw: ...`.
- Read TrackGame fields through `g_TrackGame` each time. A cached
  game or mode local changes branch threading (0x0051a560) and register
  choice (0x0051aa40).
- Block scope changes the frame: 0x0051aa40 needs `iterator` and `own`
  declared inside the `if` that uses them. 0x0051e3f0 gets
  retail's frame only with one SIZE at function scope and one in the other
  branch.
- 0x00519e10 needs the limit in a float local:
  `float limit = ...field_0x140 * 60.0f; f(limit - (a * 60.0f + b));`.
- `x / 180.0f` compiles to a multiply by the reciprocal constant
  (0x0055076c).
- Zeroing an array with a for-loop gives the retail `rep stosd` with
  `lea edi` first. `memset` moves the `lea` after `mov ecx` and `xor eax`.
- The clamp `int length = n > 0x103 ? 0x103 : n;` matches; an if-assign does
  not.
- Compute a call's result into a float local before initialising a result
  flag. Otherwise the flag lands in a callee-saved register.
- `x / 180.0f / zoom` with a known `x` folds to a constant.
- 0x0051cb20's gate posts are `CrossProduct(dir, Vector3(0, 1, 0))` and
  `CrossProduct(Vector3(0, 1, 0), dir)` with the cross product written as
  `((a.y * b.z) - (a.z * b.y))`: the doubled parentheses make VC6 load the
  non-folded operand first (`fld; fsubr st(1)`) and keep the folded zero on
  the x87 stack. The result is copied into a local, scaled in x and z only
  (the zero y is not multiplied) and offset with `+=`. The rim point of
  `0x0051c360` is a `Vector3` (its z slot is unused, which gives the frame
  size), and the colour is an if/else with direct stores.

## Reproduce

```bash
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
PYTHONPATH=. python tools/disasm_fn.py --exe work/game/mcm2.exe 0x51e240
```

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00518c60` `PlaceNeedle`
- `0x00518cc0` `PlaceNeedleTip`
- `0x00519000` `IsPointVisible`
- `0x005198a0` `Redraw`
- `0x00519e10` `DrawTimeLeft`
- `0x00519ef0` `DrawViewModePanel`
- `0x0051a480` `DrawLapTime`
- `0x0051a560` `DrawRacePanel`
- `0x0051aa40` `DrawTagStandings`
- `0x0051bc60` `DrawForViewMode`
- `0x0051bdc0` `DrawFrameRate`
- `0x0051bed0` `DrawRacers`
- `0x0051c360` `WorldToMap`
- `0x0051c460` `LineThrough`
- `0x0051cb20` `DrawGates`
- `0x0051de10` `RedrawChat`
- `0x0051e3f0` `DrawNameTag`
- `0x0051e800` `RedrawNameLine`
- `0x0051ea80` `AppendChar`
- `0x0051eab0` `Backspace`
- `0x0051ead0` `ClearLine`
- `0x0051eae0` `GetLine`
- `0x0051eaf0` `GetLineTail`
- `0x0051eb10` `GetHistory`
- `0x0051eb40` `AddHistory`
- `0x00519a20` `DrawStandings` ([reviewed nonexact source provider](STATS_STANDINGS_DRAW.md),
  complete 1,002-byte retail extent; 1,004-byte candidate)
- `0x0051baf0` `TakeRacers` and `0x0051d730` `CollectRacers` (the loader
  sample's view uses `TakeRacers`; its `0x0051d730` stays address-named
  because VisualCue's `CollectRacers` has the same view signature)
- `0x0051da30` `ChatOverlay::HandleChatKey` (BikeRace.h's view of +0x19c uses the same name)
- `0x0051dd40` `ChatOverlay::HideInputLine` (also in BikeRace.h's view); its
  neighbour `0x0051dd10` (shows the line) stays address-named because
  QuarryStuntEvent.cpp declares it

## Field names

Named from what the loaders create and the draw functions use (tier 3;
each declaration keeps its `// +0xNN`): StatsOverlay `largeFont` (+0x11c,
`lfHeight` 16, rows 0-1) and `smallFont` (+0x120, height 12, the other
rows); ChatOverlay `pen` (+0x11c, `CreatePen(0, 3, 0xc0c0c0)`), `chatFont`
(+0x120, height 9, RedrawChat), `nameTagFont` (+0x124, height 10,
DrawNameTag), `racerCount` (+0x198, the `racers` fill count) and
`leftPanel`/`rightPanel` (+0x1c8/+0x1cc, the overlays attached left and
right of `chatRect` when the screen is wider than 512); NameOverlay
`drawWidth` (+0x124, the width of its screen rectangle).
RadarOverlay (from DrawRacers, `0x0051bed0`, and WorldToMap): `mapOrigin`
(+0x17c, the followed racer's position), `mapAngle` (+0x188, its negated
heading), `heldMapAngle` (+0x18c, the angle kept while the racer's +0x444 is
set), `mapAngleCos`/`mapAngleSin` (+0x190/+0x194) and `lineStart`/`lineEnd`
(+0x1b8/+0x1d8, the two-vertex line strip DrawGates and DrawTrackOutline
submit). +0x170 (the followed racer's index) keeps its offset name because
QuarryStuntEvent.cpp reads it.

## Near misses: forms tried

Carried over from the header of the former `samples/track` aggregate when
its bodies moved into TrackOverlay.cpp.

k-scan (s22): compiled behind 0..63 prepended typedefs, 0x00519a20,
0x0051bed0, 0x0051c4f0, 0x0051c720, 0x0051d730, 0x0051e3f0 have the same
code at every count, so their differences are not a declaration-count
effect (docs/VC6_OPERAND_ORDER.md section 2).

### IntersectLineCircle `0x0051c4f0`

RadarOverlay::IntersectLineCircle (0x0051c4f0, 560 bytes, 334/520 masked):
the candidate is 552 bytes; `(ex * dx) + ey * dy` and the final
`... < (by * by) + bx * bx` (best of 3328 parenthesisations of the sums and
products) gain 13 bytes. The arithmetic and branch structure match; VC6
here assigns the scale, discriminant and intersection temporaries to
other frame slots (retail shares one slot for the scale and the
discriminant and reuses the parameter slots for the direction and x1/x2),
and computes proj + q with fxch/fadd instead of fld/faddp st(2).
Declaration order, operand order of the distance sums and the store order
of the result do not change it.

### DrawStandings `0x00519a20`

StatsOverlay::DrawStandings (0x00519a20, 1002 bytes): written
like the exact 0x0051aa40, the candidate is 1004 bytes. Retail keeps the
row-comparison index in edx and `this` in its spill slot; VC6 here does so
only when the index is read after the loop (the `row < i` test),
which leaves a redundant compare on the mismatch path and swaps edi and
ebp for `this` and the view. A goto out of the loop, a flag, drawing
inside the loop and separate loop variables spill the index instead.
Session 25: `i > row`, a `&&` while/for condition and a repeated test
are unchanged (605). Returning 1 from inside the loop or a goto out of it
gives retail's `return 1` after the loop and edi/ebp (23 differing
instructions) but spills the index and keeps `this` in edi (540, 1012
bytes), whichever of the three loops own the variable. `i == row + 1`
gives a 1002-byte body with 98 differing instructions.

### DrawTrackOutline `0x0051c720`

RadarOverlay::DrawTrackOutline (0x0051c720, 1016 bytes; candidate
1002): draws the track outline through a TrackListItem work list. The
control flow, the calls and the stores match once the node is read
through a reference to the list entry (`TrackNode*& node`, retail keeps
&list->node in edi), the visited/unvisited branches are an if/else
and the segment walk is `if (segment) do ... while (segment)`. Left: the
frame (retail 0x8c bytes, here 0x74; retail groups the clip arrays as
both circles, both line ends, both hits, and keeps the work list and the
node reference in frame slots below the point), the store order of the
entering clip's hit point, and the child loop, where retail reloads the
node from edi in the body instead of reusing the loop test's eax.

### DrawRacers `0x0051bed0`

RadarOverlay::DrawRacers (0x0051bed0, 1160 bytes; 1129 match):
the map update. Every call, loop and store matches with the racer read
through `racers[i]` each time, the own position through a reference,
the rim point as a Vector3 (its third slot is unused) and the chosen
screen point as ternaries. Left: retail allocates the
int-to-float slot of `outside` right above `x` (here it is the last
scalar slot, so y and the loop temporaries shift by one), and schedules
the distance products before the name address and the pushes of the
0x0051d9c0 call. Declaration order and scope, an int copy, a float copy
and a distance local do not move either; nor do the distance written
inline in the call, a name or overlay local, or the sum written x first.
Bracketing one product (`dz * dz + (dx * dx)`) moves the products ahead
of the name address and the pushes (1143, one hunk) but computes dx*dx
first where retail does dz*dz first, and aligns one instruction fewer;
not kept. Square helpers, casts and other bracket placements do not move.
s22: a 256-form sweep of the sum (term order, brackets on each product,
each factor and the whole sum) has no exact form; 1143 comes only from
`(dx * dx) + dz * dz` (factor brackets make no difference) and from
`distance = dx * dx; distance += dz * dz;`. The products move ahead of
the pushes only when the first-declared difference is squared first:
with dz declared before dx the loads swap too (1121-1135). Retail loads
x first and squares z first. Double casts are far worse (393).
Also 1129: the sum in an inline helper taking the two positions (by
reference or pointer, as an argument or through a distance local) or
the two differences; named squares `zz`/`xx` in either order; in-place
`dz *= dz; dx *= dx;`. A named `xx = dx * dx` used in the sum gives 1143
like the bracketed form; the helper called with the two differences
written in the call gives 1139 with 5 differing instructions.
Session 26 (all 1129 unless noted): an extra unused dy difference, the
differences as one Vector3 (all three or only x/z set), `Vector3 d =
position; d.x -= ...; d.z -= ...;` (393). Probe reads of `outside` (one to
three extra uses) move its int-to-float slot to 0x24 or 0x20, never to
retail's 0x18; extra reads of `x` change nothing. frame_layout reports
retail as if `x` had one more use, but no natural use was found.

### CollectRacers `0x0051d730` (exact)

ChatOverlay::CollectRacers (0x0051d730, 586 bytes): strict exact. Both
Create calls read `renderContext` in the argument list. Through the
inline Target() accessor VC6 loaded it ahead of the other arguments, which
swapped ebx/ebp for the zero and the count and made the body 587 bytes
(261 matching).
