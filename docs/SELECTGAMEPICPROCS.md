# SelectGamePicProcs.cpp

`src/reconstructed/SelectGamePicProcs.h` / `SelectGamePicProcs.cpp`. These
are the multiplayer lobby dialogs, written in the dialog-procedure style
of [DIALOGPROCS](DIALOGPROCS.md). Names are provisional.

Extent: `0x004f17a0..0x004f975b`. Evidence:
- **`__FILE__`:** the literal at `0x00573c6c`; its first xref is at
  `0x004f19f3`. SceneManager.cpp's last xref is at `0x004f0ee5`.
- **RTTI:** the vtables of MPEventDlg (`0x00557a88`), MPBikeRiderDlg
  (`0x00557a04`), MPOptionsDlg (`0x005578fc`) and MultiPlayerDlg (slots 24
  and 29) point into the range.
- **End:** the unit ends with its four kVec3 `$E` pairs, whose vectors only
  this unit reads. SelectiveGravityModel.cpp starts at `0x004f9760`.

The player-record array at `0x00689d08` and its initializers
`0x004f1740..0x004f1793` are probably this unit's. Only this code reads
the array. They are still defined and registered in SceneManager.cpp
(strong inference, not acted on).

Exact: 43 calibration cases:
- MultiPlayerDlg: 11 functions.
- The lobby slot table at MultiPlayerDlg+0x7f68: the constructor, its
  methods and the qsort comparator.
- The picture list by game type, `0x004f3080`.
- MPEventDlg: 8 functions.
- MPBikeRiderDlg: 6 functions.
- MPOptionsDlg: 4 functions.
- The eight `$E`.

Source forms needed:
- A call on `find(...)->field_0x1fc` takes a temporary, because retail
  pushes the arguments after the find.
- The race settings at TrackGame+0x2d70 are read through a view-struct
  pointer.
- The slot table is copied into the settings message with `memcpy`.

Near misses (`samples/ui/SelectGamePicProcsNearMisses.cpp`):
- The chat line `0x004f3720`: block layout differs.
- The bike and rider lists `0x004f8220`: registers differ.
- MPBikeRiderDlg slot 29 `0x004f78a0`, 2415 of 2419 bytes: only the x87
  operand order of a distance differs.

Not attempted: `0x004f17a0`, `0x004f2340`, MultiPlayerDlg slots 24 and 29,
MPBikeRiderDlg slot 10 and `0x004f8d20`.
