# KrustyUI

RTTI: `KrustyUI : GameObject : BaseObject` (vtable `0x0055488c`; 0x1364
bytes, the size TrackGame slot 4 allocates). Its `operator delete` calls
pass `D:\aardvark\VC\krusty2\krustyui.cpp` as `__FILE__` (`0x0056d67c`),
which confirms the original translation unit's name. Canonical source:
`src/reconstructed/KrustyUI.h` / `KrustyUI.cpp`. Names are provisional.
TrackGame keeps it at +0x56c (`ui`).

## Status

Exact (6 calibration cases):
- the constructor `0x004987f0`. Eight 8-byte `{int, char}` records at
  +0x634 have an inline constructor, so VC6 emits the clearing loop before
  the vtable store; the body then clears the other fields in retail order;
- the scalar deleting destructor `0x00498880` and the destructor
  `0x0049b470`. The destructor runs `0x004999b0`, then frees the
  DebugMalloc'd buffers at +0x48, +0x50, +0x58 and +0x60 (retail lines
  1306–1309);
- `0x004999b0`: the shutdown step. It calls the cdecl `0x005053b0(0)`,
  calls the GUI's `0x00485d50` unless Game+0x2d5 bit 1 (shutdown) is set,
  and releases +0x464;
- `0x0049b530`: tells the "ProgressBar" control of the +0x490 page 1;
- `0x0049bb80`: frees +0x60 (line 1453) and clears it and +0x64.

The GUI page and control classes are declared in
`src/reconstructed/GameUi.h`, not `KrustyUI.h`. Declaring them in a header
that TrackGame.cpp includes disturbs TrackGame slot 1 (see
[TrackGame](TRACKGAME.md)).

Not reconstructed: `0x004988a0` (1104 bytes, TrackGame slot 4's
initialiser), `0x00498cf0` (3114 bytes, with function-local statics),
`0x00499b20` (2432 bytes, opens a menu) and the other helpers in
`0x00499980`–`0x0049bc50`.
