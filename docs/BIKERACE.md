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

Exact: 39 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- Slots 14, 16, 20, 22 and 24.
- Network, replay, restart, ghost, racer and debug-draw helpers.
- The recorder callback `0x004230e0` and two sort helpers.
- KrustyVCR's implicit destructor and its deleting wrapper.
- The eight `$E`.

Near misses (`samples/race/BikeRaceNearMisses.cpp`): the camera target
`0x0041eb20` (1699 of 1710 bytes), the next/previous target `0x0041f1d0`,
and slot 23 `0x0041f5e0` (the key handler). Not attempted: the 6.7 KB setup
`0x00417ed0`, the 13 KB loader `0x00419970`, slot 10 `0x0041d2b0`, the
start-grid `0x004210f0` and the recorder worker `0x00421d50`.
