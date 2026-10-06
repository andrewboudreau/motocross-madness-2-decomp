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

## Status

79 functions are exact under `vc6_o2_mt`, with every relocation bound. All
of them are registered under `# TrackOverlay.cpp` in
`tools/run_calibration.py`:

- InstrumentOverlay: `0x00518720`..`0x00518cc0`, 7 functions.
- The overlay rectangle constructors `0x00518d50` and `0x00518d60`.
- NameOverlay: `0x00518d80`..`0x005190e0`, 7 functions.
- StatsOverlay: `0x00519370`..`0x00519980`, 7 functions, plus the score
  comparator `0x005199f0`.
- DropTextOverlay: `0x0051ae80`..`0x0051b1f0`, 5 functions.
- UnknownMessage and TextQueueOverlay: `0x0051b200`..`0x0051b670`, 11
  functions.
- RadarOverlay: `0x0051b690`..`0x0051c460`, 11 functions.
- The `$E` initializers and their stubs: `0x0051dba0`..`0x0051dcdc`, 8
  functions.
- ChatOverlay: `0x0051cda0`..`0x0051e800`, 14 functions, among them the
  constructor, the destructor, slots 10, 13 and 14, and the name-line
  redraw.
- UnknownChatInput: `0x0051ea50`..`0x0051eb10`, 6 functions.

### Near misses

These are in `samples/track/TrackOverlayNearMisses.cpp`, with notes:

- `0x00519000` (NameOverlay visibility test, 119 bytes): the candidate is
  118 bytes. VC6 swaps the eax/ecx choice for the camera and the world.
- `0x005198a0` (StatsOverlay view-mode redraw): the switch and its jump
  table match. Retail returns 0 through a shared `xor eax, eax`, which no
  tried source shape reproduces.
- `0x0051bb60` (RadarOverlay slot 23, the zoom keys, 89.69%): retail keeps
  a dead `field_0x178` test. It schedules the fld/fmul/fidiv sequence before
  that test; VC6 schedules it after.

### Not reconstructed

- **Inline-asm float-to-int.** `0x00518f30`, `0x0051af00` and `0x0051b070`
  use a `fistp` helper that the project rules exclude.
- **Shared tiny address.** `0x0051eae0` is a 3-byte `mov eax, ecx; ret`.
- **Large bodies, not attempted:**
  - StatsOverlay: `0x005194b0`, `0x00519a20`, `0x00519e10`, `0x00519ef0`,
    `0x0051a480`, `0x0051a560`, `0x0051aa40`.
  - RadarOverlay: `0x0051bed0`, `0x0051c4f0`, `0x0051c720`, `0x0051cb20`.
  - ChatOverlay: `0x0051cf80`, `0x0051d730`, `0x0051da30`, `0x0051de10`,
    `0x0051e3f0`, `0x0051e910`.
  - UnknownChatInput: `0x0051eb40`.

## Codegen notes

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
