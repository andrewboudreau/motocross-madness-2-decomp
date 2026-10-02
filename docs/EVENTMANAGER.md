# EventManager

RTTI: `EventManager : GameObject : BaseObject` (vtable `0x0055259c`;
0xd08 bytes, the size TrackGame slot 4 allocates). Its constructor
`0x0045c9e0` writes the vtable. Canonical source is
`src/reconstructed/EventManager.h` / `EventManager.cpp`; the TU is not
established. Names are provisional.

TrackGame keeps it at +0x570 (it was the placeholder `TrackGameList`).

## Status

Exact (9 calibration cases):
- the constructor (11 0x50-byte entries at +0x50; -1000 in each component
  of +0x3c4) and both destructors;
- slot 8, which reads "KeepAliveTimeout" (default 20) into +0x2c;
- `0x0045d2b0`, `0x0045d2f0` and `0x0045d340`: the first race-mode object of
  TrackGame+0x558..+0x568 present, its +0x34 view and its +0x6c target;
- `0x0045d390`: whether any is present;
- `0x0045d270`: slot 5 on all three.

The race-mode objects, their views and targets are treated as GameObjects.
That is inference from their use (slots 4 and 5, the +0x25 flag bits), and
it fits KrustyBike's primary base chain for the views. Their classes are not
established; `src/reconstructed/RaceView.h` declares them.

Not reconstructed:
- slots 10 and 22-24 (`0x0045f200`, `0x0045f3a0`..);
- `0x0045cb20`, `0x0045cb70`, `0x0045cdc0`, and the comparison
  `0x0045d3d0`.
