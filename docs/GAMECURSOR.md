# cursor.cpp (GameCursor)

`src/reconstructed/GameCursor.h` / `GameCursor.cpp`. Retail calls this file
cursor.cpp; the reconstruction is named after its class.

Evidence:
- The `__FILE__` literal `D:\aardvark\VC\krusty2\cursor.cpp` at `0x0056890c`,
  used by `0x0043ebd0` at line 128.
- RTTI `GameCursor : GameObject`, vtable `0x0055130c`.
- The TU's four `.CRT$XCU` vector initializers, starting at `0x0043e870`.

Extent: `0x0043e870..0x0043f15f` (strong inference). cubedraw.cpp's code
ends at `0x0043e865`. The constructor at `0x0043f160`, which writes vptr
`0x005513ec`, starts the next unit. Two out-of-line Parameterblocks.h
stream accessors, `0x0043e9b0` and `0x0043e9e0`, sit inside the range;
they are not attempted.

GameCursor (0x5c bytes) draws a TGA cursor image (a PCTextureMap at +0x38)
over a saved background region. It follows two control bindings
(+0x3c, +0x40) or, without them, GetCursorPos. GUIManager.h now includes
GameCursor.h, and its GUICursor : GameCursor stays there; no mangled names
changed. BackgroundImage.h declares `0x00404700`, which slot 15 calls.

Exact: 15 calibration cases. These are the eight `$E` initializers, the
constructor, the destructor and its deleting wrapper, the two init
functions `0x0043ea70` and `0x0043eaf0`, the image load `0x0043ebd0` and the
position `0x0043f100`. Near miss (`samples/ui/GameCursorNearMisses.cpp`):
slot 15 `0x0043ed40`, where the final two Blt branches differ in register
allocation and in which branch receives the inlined epilogue.
