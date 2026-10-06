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

Exact (3 calibration cases):

| VA | Size | Role |
|---|---:|---|
| `0x004e5c70` | 33 | whether the list has a node for a racer |
| `0x004e5ca0` | 83 | frees nodes of racers whose +0x25 bit 0 is clear |
| `0x004e5f10` | 41 | frees the whole list |

`0x004e5ca0` re-reads `*list` for each access instead of keeping the node in
a local (GameObject.h grants it friend access to the +0x25 bits).

Near miss (`samples/race/RaceStatusNearMisses.cpp`): the builder
`0x004e5d00` (register allocation only; notes there).
