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

Exact: 51 calibration cases:
- MultiPlayerDlg: 12 functions, including the dialog procedure (slot 29,
  `0x004f4520`, 4764 bytes with its jump table).
- The two grid qsort comparators `0x004f2080` (by the byte at +4) and
  `0x004f20a0` (by the float at +4), both largest first.
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
- PlayerInfoType slot 1 (`0x004add00`) is declared in Net.h and defined in
  Net.cpp: slot 29 calls it out of line on each `0x00689d08` record.
- Slot 29's two 0x80-byte message buffers are block-scoped (case 5 and the
  "ButRdyUser" branch each declare their own); with function-scoped
  buffers VC6 swaps their frame slots.
- The race settings (+0x14c) hold eight 8-byte grid entries (id, points,
  racer index) and, at +0x18c, a copy of the 0x60-byte lobby slot table.
  TrackGame.h does not name the entry's racer index byte (+5): adding it
  together with this unit's other TrackGame.h additions breaks TrackGame
  slot 1 (`0x00520ab0`). The near-miss sample reads +5 through a local
  view struct.

Messages (layouts from the senders and the receiver, slot 24):
- Type 2, 0xd4 bytes: the player's state (ready flag at +1, bike model,
  bike and rider names at +0x42, +0x02 and +0x82, the send time at +0xc4).
  Slot 29 sends it; slot 24 stores it in the sender's racer slot and keeps
  the smallest arrival delay at racer slot +0xd0.
- Type 0x83, 0x8d8 bytes: the start message (`0x004f2340`): a start flag
  at +4, the race settings at +8, and per racer +0xc0, name and the three
  model names.
- Type 0xf: the bit-packed settings `0x004f2b90` sends.

Also exact: MPBikeRiderDlg `0x004f8d20`.

Near misses (`samples/ui/SelectGamePicProcsNearMisses.cpp`):
- The chat line `0x004f3720`: block layout differs.
- The bike and rider lists `0x004f8220`: registers differ.
- MPBikeRiderDlg slot 29 `0x004f78a0`, 2415 of 2419 bytes: only the x87
  operand order of a distance differs.
- The picture-list scan `0x004f17a0`, 2244 of 2258 bytes: the selected
  and fallback buffers' frame slots are swapped. VC6 orders the frame by
  reference count. With one reference fewer to the selected buffer, the
  two buffers tie and VC6 gives the retail order. Declaration order, names,
  block scope and the loop form do not change it.
- MPBikeRiderDlg slot 10 `0x004f8820`: only the scheduling of the by-value
  vector copies for the camera call differs.
- The lobby start message `0x004f2340` (2120 bytes): the same flow,
  calls and frame size. Registers differ: retail keeps the racer total in
  memory and the bike, rider and name arrays in registers. VC6 here keeps
  the total in a register instead.
- MultiPlayerDlg slot 24 `0x004f3a70`, 2714 of 2729 bytes: only the order
  of two instructions in the arrival-delay conversion differs.

Every other start in the extent is a jump table or a branch target inside
one of these functions.
