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
  scene-object branch.
- `0x0041f1d0` and slot 23 `0x0041f5e0`: retail keeps 0 in a callee-saved
  register throughout; the candidates do not.
- `0x004210f0` (start grid), 969 of 2940 bytes: VC6's inline budget
  places the out-of-line `Vector3` constructor/scale calls differently
  (see the comment in the sample).

Not attempted: the 6.7 KB setup `0x00417ed0` (9 arguments, returns this;
16 EH states, atan2/sqrt float code) and the 13 KB loader `0x00419970`
(called by the setup).
