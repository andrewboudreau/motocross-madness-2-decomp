# cube.cpp and cubedraw.cpp

`src/reconstructed/Cube.*` and `CubeDraw.*`. Names are provisional unless
RTTI gives them.

**cube.cpp** (`0x0043d080..0x0043d88f`, strong inference). Evidence: RTTI
`Cube : BaseObject` (vtable `0x00551288`), whose vptr is written by
`0x0043d1f0` and `0x0043d6c0`. The `__FILE__` literal at `0x0056888c` is
used in `0x0043d460` (lines 0x2c3 and 0x2c8). The four vector initializers
`0x0043d750..0x0043d88b` build `0x00579870` and the next three vectors;
`0x0043d110` reads the first. `0x0043d080` is called only from cube.cpp
(`0x0043d4aa`) and cubedraw.cpp (`0x0043e179`), so it probably belongs
here. It stays registered under ControlInterface.cpp for now.

**cubedraw.cpp** (`0x0043d890..0x0043e9ff`, strong inference). Evidence:
RTTI `DrawableCube : GameObject` (vtable `0x0055129c`), overriding slots 0,
12, 13 (the shared `0x00467ae0`) and 14. The `__FILE__` literal is at
`0x005688cc`.

`.CRT$XCU` lists these groups in order: cube's vectors, then the
`0x0043e870` vectors, then the `0x0043d890` arrays, then
D3DIMSoulTree.CPP's. A unit's own arrays get their `$E` in place at the
top of the file, while the shared header's vectors are emitted after the
functions but listed first. AuralScape.cpp shows the same pattern. So the
`0x0043e870` group is cubedraw's, and cursor.cpp has none. Those eight
cases moved here from GameCursor.cpp. The Parameterblocks.h copies
`0x0043e9b0` and `0x0043e9e0`, first called from `0x0043d980`, also fall
in this file's range; they are not claimed.

Exact:
- cube.cpp (17): the stream alignment helper, reset, the constructor, the
  header reader `0x0043d230` (its three argument stores must come in the
  order group, stream, baseOffset: VC6 homes `flags`, the face pointer and
  the face counter in the dead argument slots in that order,
  docs/VC6_FRAME_LAYOUT.md), the destructor and its deleting wrapper, the
  vectors and texture loaders, and the eight `$E`.
- cubedraw.cpp (22): the constructor, the destructor and its deleting
  wrapper, the loader `0x0043d980`, `0x0043e210`, slot 12, the eight empty
  array `$E` (their order cannot be checked because the bodies are
  identical) and the eight vector `$E`.

Near misses:
- `samples/render/CubeDrawNearMisses.cpp`: the geometry builder
  `0x0043dc60`, the texture coordinates `0x0043e0b0`, the visibility test
  `0x0043e330` and slot 14 `0x0043e4c0`. They differ in register use,
  strength reduction and constant materialisation.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x0043d110` `Reset`
- `0x0043d460` `LoadTexture`
- `0x0043d630` `LoadMissingTextures`
- `0x0043d980` `Load`
- `0x0043e210` `RecordTextureUse`
