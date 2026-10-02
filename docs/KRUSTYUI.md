# KrustyUI

RTTI: `KrustyUI : GameObject : BaseObject` (vtable `0x0055488c`; 0x1364
bytes, the size TrackGame slot 4 allocates). Its `operator delete` calls
pass `D:\aardvark\VC\krusty2\krustyui.cpp` as `__FILE__` (`0x0056d67c`),
which confirms the original translation unit's name. Canonical source:
`src/reconstructed/KrustyUI.h` / `KrustyUI.cpp`. Names are provisional.
TrackGame keeps it at +0x56c (`ui`).

## Status

Exact (21 calibration cases):
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
- `0x0049bb80`: frees +0x60 (line 1453) and clears it and +0x64;
- the cdecl GUI progress callback `0x0049bda0`, which tail-calls the mode
  object's `0x00523580`;
- the overrides of slots 4 and 5 (base tail calls), 10, 18 (returns 1),
  23 (clears +0x4a4), 24 (message 0x101 sets the network object's +0x10)
  and 25 (also forwards the value to the +0x464 scene);
- `0x004999f0` and `0x00499a20`, which show and hide the +0x464 scene and
  set the owner render target's camera (+0x468, or none);
- `0x00499b00` and `0x00499b10`, which call the GUI's `0x00486630` with 1 and
  0;
- `0x0049a4a0`, which turns "MediaControl" on, hides the GUI and opens
  `Exit1Dlg` ("Exit1.dtm", line 836);
- `0x0049ba70`, which appends a 0x54-byte entry (five ints and a 64-char
  name) to the +0x60 list through gameui.cpp's resize helper `0x0047b570`.
  It returns `++count - 1`;
- `0x0049b020`, which fills an array with distinct random short strings from
  `0x0068a498`, probing a DebugCalloc'd used-table (`0x004a2fc0`, lines
  1120/1157). It scales `rand()` through a float local: written as one
  expression, VC6 folds `1/32768` and `36` into a single constant.

Near miss (`samples/ui/KrustyUINearMisses.cpp`): `0x004988a0`, TrackGame
slot 4's initialiser (872 of 1104 bytes). It loads 36 short strings
(resources 5000–5035) into `0x0068a498`, the GUI scale (0xfed) and font
(0xff2, else "Arial"). It creates the GUI (`new`, line 121) and adds the
object its setup returns, applies the font size (0x1469), loads `ui`,
`ui\wait.tga` and `ui\uires.res`, and restores "MRUProfile" and
"JoystickFilter". Offline, when asked, it opens `Intro1Dlg` ("intro1.dtm",
line 201). From the scale store on, retail picks different registers.

The GUI page and control classes are declared in
`src/reconstructed/GameUi.h`, not `KrustyUI.h`. Declaring them in a header
that TrackGame.cpp includes disturbs TrackGame slot 1 (see
[TrackGame](TRACKGAME.md)).

Not reconstructed: `0x00498cf0` (3114 bytes, with function-local statics;
the empty `0x00499920`–`0x00499970` are their exit destructors),
`0x00499b20` (2432 bytes, opens a menu), `0x0049a540`, `0x0049a690`,
`0x0049a8b0`, `0x0049b0d0`, `0x0049b560`, `0x0049b7f0`,
`0x0049bae0`, `0x0049bbb0` and `0x0049bc50`.
