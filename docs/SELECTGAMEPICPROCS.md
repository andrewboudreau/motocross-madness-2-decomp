# SelectGamePicProcs.cpp

`src/reconstructed/SelectGamePicProcs.h` / `SelectGamePicProcs.cpp`. These
are the multiplayer lobby dialogs, written in the dialog-procedure style
of [DIALOGPROCS](DIALOGPROCS.md). Names are provisional.

Extent: `0x004f1740..0x004f975b`. Evidence:
- **`__FILE__`:** the literal at `0x00573c6c`; its first xref is at
  `0x004f19f3`. SceneManager.cpp's last xref is at `0x004f0ee5`.
- **RTTI:** the vtables of MPEventDlg (`0x00557a88`), MPBikeRiderDlg
  (`0x00557a04`), MPOptionsDlg (`0x005578fc`) and MultiPlayerDlg (slots 24
  and 29) point into the range.
- **End:** the unit ends with its four kVec3 `$E` pairs, whose vectors only
  this unit reads. SelectiveGravityModel.cpp starts at `0x004f9760`.

- **Start:** the player-record array at `0x00689d08` and its initializers
  `0x004f1740..0x004f1793` open the unit. `.CRT$XCU` lists `0x004f1740`
  right after this unit's vector initializers, and not next to
  SceneManager.cpp's. The array lies inside this unit's vector `.bss`
  block, and only this code reads it (see [INITIALIZERS](INITIALIZERS.md)).

Exact: 48 calibration cases:
- MultiPlayerDlg: 11 functions.
- The lobby slot table at MultiPlayerDlg+0x7f68: the constructor, its
  methods and the qsort comparator.
- The picture list by game type, `0x004f3080`.
- MPEventDlg: 8 functions.
- MPBikeRiderDlg: 6 functions.
- MPOptionsDlg: 4 functions.
- The eight vector `$E` and the four player-record `$E`.

Source forms needed:
- A call on `find(...)->field_0x1fc` takes a temporary, because retail
  pushes the arguments after the find.
- The race settings at TrackGame+0x2d70 are read through a view-struct
  pointer.
- The slot table is copied into the settings message with `memcpy`.

Also exact: MPBikeRiderDlg `0x004f8d20`.

Near misses (`samples/ui/SelectGamePicProcsNearMisses.cpp`):
- The chat line `0x004f3720`: block layout differs.
- The bike and rider lists `0x004f8220`: registers differ.
- MPBikeRiderDlg slot 29 `0x004f78a0`, 2415 of 2419 bytes: only the x87
  operand order of a distance differs.
- The picture-list scan `0x004f17a0`, 2244 of 2258 bytes: two buffers'
  frame slots are swapped.
- MPBikeRiderDlg slot 10 `0x004f8820`: only the scheduling of the by-value
  vector copies for the camera call differs.

Not attempted: `0x004f2340` (the lobby start message, type 0x83) and
MultiPlayerDlg slots 24 and 29.
