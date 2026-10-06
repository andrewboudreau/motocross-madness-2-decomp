# cursor.cpp (GameCursor)

`src/reconstructed/GameCursor.h` / `GameCursor.cpp`. Retail calls this file
cursor.cpp; the reconstruction is named after its class.

Evidence:
- The `__FILE__` literal `D:\aardvark\VC\krusty2\cursor.cpp` at `0x0056890c`,
  used by `0x0043ebd0` at line 128.
- RTTI `GameCursor : GameObject`, vtable `0x0055130c`.

Extent: `0x0043ea00..0x0043f15f` (strong inference). The constructor at
`0x0043f160`, which writes vptr `0x005513ec`, starts the next unit. The
vector initializers `0x0043e870..0x0043e9ab` and the Parameterblocks.h
copies before `0x0043ea00` belong to cubedraw.cpp; see [CUBE](CUBE.md).
cursor.cpp has no `.CRT$XCU` entries of its own.

GameCursor (0x5c bytes) draws a TGA cursor image (a PCTextureMap at +0x38)
over a saved background region. It follows two control bindings
(+0x3c, +0x40) or, without them, GetCursorPos. GUIManager.h now includes
GameCursor.h, and its GUICursor : GameCursor stays there; no mangled names
changed. BackgroundImage.h declares `0x00404700`, which slot 15 calls.

Exact: 7 calibration cases. These are the constructor, the destructor and
its deleting wrapper, the two init functions `0x0043ea70` and `0x0043eaf0`,
the image load `0x0043ebd0` and the position `0x0043f100`. Near miss (`samples/ui/GameCursorNearMisses.cpp`):
slot 15 `0x0043ed40`, where the final two Blt branches differ in register
allocation and in which branch receives the inlined epilogue.
