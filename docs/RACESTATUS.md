# RaceStatus.cpp

`src/reconstructed/RaceStatus.h` / `RaceStatus.cpp`: cdecl helpers over the
per-racer status list a race view builds. Evidence: the `__FILE__` literal
`D:\aardvark\VC\krusty2\RaceStatus.cpp` (`0x00572990`) passed to
DebugCalloc (lines 214, 235, 257) and the matching delete (lines 193, 285)
from `0x004e5cd1`–`0x004e5f24`. Names are provisional.

The nodes are `UnknownEventRacerPart` (RaceView.h, 0x54 bytes): the
1-based creation order (+0x00), the racer (+0x04), the float EventManager
sorts by (+0x0c), the racer's position when created (+0x28, copied from the
racer's +0x0c), the view's +0xc8 (+0x34) and the next node (+0x50). Each
racer points back at its node from +0x744. Freeing tests the racer's
GameObject +0x25 bit 0 through the virtual-base pointer.

Extent: `0x004e58a0..0x004e6f7f` (strong inference). It opens with one
empty `$E` pair (`0x004e58a0`), for the two arrow arrays at
`0x00689c30`/`0x00689c48` that `0x004e59c0` writes. Its kVec3 `$E` set at
`0x004e6e40..0x004e6f7b` ends it. Those vectors share this unit's bss
block, and the `.CRT$XCU` order is racesnd.cpp's set, this unit's
vectors, then `0x004e58a0`. A unit's entries are contiguous; this follows
the cube.cpp pattern. recorder.cpp therefore starts at `0x004e6f80`, and
the eight `$E` cases moved here from Recorder.cpp.

Exact: 20 calibration cases:
- The three list helpers `0x004e5c70`, `0x004e5ca0` and `0x004e5f10`.
- The lap and gate counters `0x004e58c0` and `0x004e59c0`.
- The orderings by laps and by gates, `0x004e5f40` and `0x004e6120`.
- The time behind the leader `0x004e6210`.
- The score ranking `0x004e62d0`.
- The 1646-byte lap-race update `0x004e63e0`.
- The ten `$E`.

Source forms needed:
- Fieldwise TrackPos and Vector3 copies.
- An integer scale on the random unit, because a float scale lets VC6
  fold the constants.
- The racer index computed twice in `0x004e6210`.

`0x004e5ca0` re-reads `*list` for each access instead of keeping the node in
a local (GameObject.h grants it friend access to the +0x25 bits).

Near misses (`samples/race/RaceStatusNearMisses.cpp`, notes there):
- The builder `0x004e5d00`: register allocation only.
- The gate-race update `0x004e6a50`: one instruction off. Retail compares
  `>= 1`; written that way, VC6 moves the else branch out of line.
