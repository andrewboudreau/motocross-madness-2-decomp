# TrackOverlay

The literal `__FILE__` `D:\aardvark\VC\krusty2\TrackOverlay.cpp`
(`0x00575290`) is referenced from `0x0051b550`..`0x0051d850` and from the
unwind funclets at `0x0054ecbe`..`0x0054ed5d`. The reconstructed source is
`src/reconstructed/TrackOverlay.h`, `TrackOverlay.cpp` and
`TrackOverlay.bindings.json`. TextQueueOverlay and UnknownMessage are
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
  these place `0x0051dce0..0x0051ee90` (ChatOverlay and UnknownChatInput) in
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

UnknownChatInput (0x1c4 bytes, no vtable) lives at ChatOverlay+0x13c. It
holds the line being typed and four history entries.
ChatOverlay+0x140 is the chat rectangle (centred, 256 x 68), +0x150 the cue
rectangle the loader is given, and +0x1c8/+0x1cc two plain Overlay side
panels created only when the screen is wider than 512 pixels (allocations
at lines 2284 and 2294).

## Status

89 functions are exact under `vc6_o2_mt`, with every relocation bound:

- InstrumentOverlay: `0x00518720`..`0x00518cc0`, 7 functions.
- The overlay rectangle constructors `0x00518d50` and `0x00518d60`.
- NameOverlay: `0x00518d80`..`0x005190e0`, 8 functions, among them the
  visibility test `0x00519000`.
- StatsOverlay: `0x00519370`..`0x00519980`, 7 functions, the score
  comparator `0x005199f0`, the view-mode redraw `0x005198a0` and the panels
  `0x00519e10`, `0x00519ef0`, `0x0051a480`, `0x0051a560` and `0x0051aa40`.
- DropTextOverlay: `0x0051ae80`..`0x0051b1f0`, 5 functions.
- UnknownMessage and TextQueueOverlay: `0x0051b200`..`0x0051b670`, 11
  functions.
- RadarOverlay: `0x0051b690`..`0x0051c460`, 11 functions.
- The `$E` initializers and their stubs: `0x0051dba0`..`0x0051dcdc`, 8
  functions.
- ChatOverlay: `0x0051cda0`..`0x0051e800`, 16 functions, among them the
  typed-key handler `0x0051da30` and the input/history redraw `0x0051de10`.
- UnknownChatInput: `0x0051ea50`..`0x0051eb40`, 7 functions.

Their calibration cases are under `# TrackOverlay.cpp` in
`tools/run_calibration.py`.

`0x0051e910` (redraws one name tag or all) is exact now that
`Overlay::UnknownFunction4b6880` takes an `unsigned short` colour. Retail
builds the 4444 grey in 16-bit registers and pushes it without
zero-extension.

### Near misses

These are in `samples/track/TrackOverlayNearMisses.cpp`, with notes:

- `0x0051e3f0` (draws one name tag, 974 bytes, 964 match): the frame matches;
  in the racer-tag branch retail picks eax/ecx/edx for the racer, the buffer
  and the game pointer, VC6 here edx/eax/ecx.
- `0x00519a20` (standings, 1002 bytes; candidate 1004): retail keeps the row
  comparison index in edx and `this` in its spill slot. VC6 here does that
  only when the index is read after the loop, which adds a redundant compare.
- `0x005194b0` (StatsOverlay loader, 976 bytes; candidate 956): retail
  places the 0x3c-byte LOGFONT below the two rectangles in the frame; VC6
  here orders them the other way (it matches the retail order for a LOGFONT
  of 0x38 bytes or less).
- `0x0051d730` (racer name tags, 586 bytes; candidate 587): the zero and the
  incremented count swap ebx and ebp.
- `0x0051c4f0` (line/circle intersection, 560 bytes; candidate 550): frame
  slot assignment of the float temporaries differs.
- `0x0051bb60` (RadarOverlay slot 23, the zoom keys, 89.69%): retail keeps a
  dead `field_0x178` test. It schedules the fld/fmul/fidiv sequence before
  that test; VC6 schedules it after.
- `0x0051c720` (track outline, 1016 bytes; candidate 1002): walks the
  track graph (`Track.h`: TrackNode, TrackSegment, TrackListItem; the
  view's +0x48 is the Track) and draws each node's segment edge clipped to
  the map circle. Code and stores match with the node read through a
  reference to the list entry, an if/else for visited nodes and a
  `do ... while` segment walk; the frame (0x8c in retail, 0x74 here) and
  two store/reload orders differ.
- `0x0051cf80` (ChatOverlay loader, 1953 bytes; candidate 1947): frame and
  calls match with the side-panel rectangle declared inside the wide-screen
  block; that block's scheduling, the LOGFONT store order and the
  thirteenth name-tag rectangle's temporary slot differ.

### Not reconstructed

- **Inline-asm float-to-int.** `0x00518f30`, `0x0051af00` and `0x0051b070`
  use a `fistp` helper that the project rules exclude.
- `0x00518c20` is not a function start (inside the InstrumentOverlay
  destructor `0x00518c00`).
- **Shared tiny address.** `0x0051eae0` is a 3-byte `mov eax, ecx; ret`
  (UnknownChatInput's line accessor; `0x0051da30` calls it).
- **Not attempted or abandoned:**
  - RadarOverlay: `0x0051bed0` (1160 bytes; calls `0x0051c720` with modes
    1 and 2 and `0x0051cb20`), `0x0051cb20` (640 bytes; the gate offsets are
    an inlined cross product with (0, 1, 0), whose temporaries no tried
    Vector3 form reproduces).

## Codegen notes

- Every GetDC/ReleaseDC redraw returns 0 through one block: retail matches
  `if (GetDC(...) != 0) goto fail; ... if (ReleaseDC(dc) != 0) { fail: return 0; }`.
  Early `return 0` statements emit a separate epilogue. The same `goto fail`
  shape gives 0x005198a0's shared `xor eax, eax`.
- The row comparisons of the StatsOverlay panels are `for (...) if
  (strcmp(...) != 0) goto draw; return 1; draw: ...`.
- Read TrackGame fields through `g_UnknownGlobal56e26c` each time. A cached
  game or mode local changes branch threading (0x0051a560) and register
  choice (0x0051aa40).
- Block scope changes the frame: 0x0051aa40 needs `iterator` and `own`
  declared inside the `if` that uses them. The near miss 0x0051e3f0 gets
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

## Reproduce

```bash
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
PYTHONPATH=. python tools/disasm_fn.py --exe work/game/mcm2.exe 0x51e240
```
