# bikerace.cpp

`src/reconstructed/BikeRace.h` / `BikeRace.cpp`. Names are provisional.

Evidence:
- **`__FILE__`:** the bikerace.cpp literal at `0x00567c74`, xrefs
  `0x0041807d..0x004233ab`. The bikerace.h literal at `0x00567de0` is used
  at `0x0041cfd8`, by the inline node-list free in the destructor.
- **RTTI:** `BikeRace : GraphicsTest : GameObject` (vtable `0x00550d24`).
  Only the constructor `0x00417c30` and destructor `0x0041cf30` write it.
- **Same object as RaceView.h:** RaceView.h's `UnknownKrustyBikeView`
  describes the same object at the same offsets (strong inference); its
  callers' mangled names are unchanged.

Extent: `0x00417b00..0x004238bb` (strong inference). The plate-number
painter at `0x00417500..0x00417aff` may also belong here and is
unresolved. From `0x004238c0` the code is GR_BitString's; BlockAllocator.cpp
starts at `0x00423f70`. The four kVec3 `$E` pairs sit mid-file at
`0x0041cdf0..0x0041cf2b`.

Exact: 41 functions (39 earlier calibration cases plus two):
- The constructor, the destructor and its deleting wrapper.
- Slots 10, 14, 16, 20, 22 and 24.
- Network, replay, restart, ghost, racer and debug-draw helpers.
- The recorder worker `0x00421d50`, its callback `0x004230e0` and two sort
  helpers.
- KrustyVCR's implicit destructor and its deleting wrapper.
- The eight `$E`.

Source forms needed:
- Slot 10 `0x0041d2b0`: the message buffer is `char[0x100]` (only 0x80 is
  passed), and `Vector3 position` is declared at the top of the block
  after the network-start branch; otherwise VC6 packs the stack
  differently. Its prologue schedules a global load between the fs:[0]
  load and `push -1`; `mcm2tool/resolved_match.py` recognises that shape.
- `0x00421d50`: float comparisons with -1.0f compile to integer compares;
  the loop index of the message-13 branch is initialised before the mode
  test. The replay tick time is the file static `0x00578e8c`.

Near misses (`samples/race/BikeRaceNearMisses.cpp`):
- `0x0041eb20` (camera target), 1699 of 1710 bytes: register swap in the
  scene-object branch (retail loads the index before the table; no
  index/entry spelling or flag changes it).
- `0x0041f1d0` and slot 23 `0x0041f5e0`: retail keeps 0 in a callee-saved
  register throughout; the candidates do not.
- `0x004210f0` (start grid), 969 of 2940 bytes: VC6's inline budget
  places the out-of-line `Vector3` constructor/scale calls differently
  (see the comment in the sample); every frame slot is at retail's
  offset.
- `0x00417ed0` (setup, 6745 bytes with the jump table at `0x0041992c`;
  ret 0x24; QuarryStuntEvent.cpp's loader calls it after the
  constructor), 2800 of 6756 positions: all 166 calls in retail order, the
  EH states 0..0x10 (`new` lines 0x193..0x367) and the 0x4e8-byte frame with
  the buffers at retail's offsets. Retail spills `z*z` of both vector
  lengths to a stack temporary (`fstp [t]; faddp; fadd [t]`, a shape found
  nowhere else in the binary) and orders the girl block's temporaries
  differently, so most esp offsets below +0x44 differ. The spill is a
  store-and-reload of a memory temporary: a `volatile float zz = v.z *
  v.z` in the length helper reproduces the `fstp [t]`, the `fadd [t]`,
  the slot (`angle`'s dead `[esp+0x34]`) and the rest of the frame (6727
  of 6768 positions), but VC6 then evaluates the square before `x * x`
  and `y * y` where retail evaluates it last; assignments inside the
  expression, casts, comma forms, accumulations and inline helpers keep
  the products on the x87 stack. Slot map below `+0x44`: retail's `+0x20`
  (10 uses) and `+0x24` (9) are the classes here at `+0x2c` (8) and
  `+0x30` (`height` / `angle`, 7), the two extra uses being the spilled
  square at each site; `axis` moves from `+0x20` to `+0x28` and the
  seven-use classes from `+0x24` / `+0x28` to `+0x2c` / `+0x30`.
- `0x00419970` (loader, 13428 bytes; ret 0x14, frame 0xab0), 1380 of
  13322 positions: completely decoded (pro circuit, network, offline,
  ghost and AI racers, collision pairing; EH states for `new` lines
  0x3b6..0x78f, frees at lines 0x4ff..0x501, 0x550..0x552 and
  0x7f7..0x7f9); 194 of retail's 195 calls in order. The candidate's frame
  (0xac4) orders the locals differently, and register choice follows:
  retail's 23-use classes at `+0` and `+0x4` are `i` (15 here, `+0x14`)
  with `fromEvent` (16, `+0x10`) and `riderModels` (29 here, `+0`) with
  `skill`; its `+0x8` / `+0xc` are `recordIndex` / `id` (here `+0x4` /
  `+0x8`), `position` and `direction` sit at `+0x14` / `+0x20` (here
  `+0x20` / `+0x2c`), `playerId` falls to `+0x48` (16 uses here, 10
  there), `names` to `+0x30`, `plate` to `+0x40`, `setup` to `+0x6c`;
  the candidate's `slots` (11 uses) has no retail slot, and retail
  shares its temporaries into fewer classes (0x14 bytes of frame).

Facts the two functions establish:
- Racers are KrustyBike (constructor `0x0048fa60`, 0x1638 bytes, the
  GameObject virtual base at +0x160c), set up by `0x0048fc80` (25
  arguments; its 12th is a char AI index). KrustyBike slot 45 takes no
  argument and slot 50 three (`ret 0`, `ret 0xc`).
- KrustyVCR `0x0049c070` takes a char second argument and a name (the
  player name, `TrackGameMode+0`) fourth; `0x0049c1e0` reads a record back.
- The track file name comes from `strrchr(path, '.')` (`0x00534a60`).
- `0x00515ed0` is Track's loader (start and finish probes); `new Track`
  inlines a constructor that clears +0.
- The credits dialog is DemoDlg (vtable `0x00550e18`), constructed inline
  as `UIDialog(1, "credits.dtm")`; DlgProcs.h declares no constructor, so
  the sample uses a local stand-in class bound to that vtable.
