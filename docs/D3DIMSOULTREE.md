# D3DIMSoulTree.CPP

`src/reconstructed/D3DIMSoulTree.h` / `D3DIMSoulTree.cpp`. LightEmitter.h
includes the header, so every existing caller keeps its declarations and
mangled names. Names other than the RTTI classes are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x00568994`, xrefs `0x0043f2ee..0x00444f6b`.
- **RTTI:** `D3DIMSoultreeObject : SoultreeObject : QuadTreeObject (+0),
  GameObject (+0xc)`, vtables `0x005513ec` (offset 0) and `0x0055137c`
  (offset 0xc). The constructor and destructor write both. Clones allocate
  0x2d8 bytes.

Extent: `0x0043f160..0x0044523f`, from the constructor after cursor.cpp to
D3DIMSoultreeModifier.cpp. Its kVec3 `$E` set sits mid-file at
`0x00442e00..0x00442f3b` (`.CRT$XCU` 82-85). `0x00440f30..0x00442dd8` is one
large drawing function with a jump table; fnlist's starts inside it are
false.

In this tree, SoultreeObject is a view with real bases
(`UnknownSoultreeQuadTreeObject`, GameObject); the krusty2 headers are
untouched. The secondary-base slots are named `QuadTreeVirtualSlot0/1` and
`SoultreeVirtualSlot2..8`, so they cannot collide with GameObject's.

Exact: 43 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- The `.slb`/`.slt` loader (slot 9), the binary load (slot 2) and the
  materials (slot 3).
- The clone (slot 4), slots 5, 6 and 8 and GameObject slots 4, 5, 10 and 14.
- The wireframe and box outlines, the surface buffers, the LOD setters and
  preset, the two modifier lists, the material copy and texture, and the
  counts.
- The eight `$E`.

Near misses (`samples/render/D3DIMSoulTreeNearMisses.cpp`):
- The LOD `.slt` reader `0x00440060` (1955 of 1964 bytes) and the
  vertex-group transform `0x00440d40`. Retail adds `offset + base`.
- Slot 5 bounds `0x00444140`. It fails to link because VC6 keeps a bound on
  the FPU stack and needs a 64000.0 constant.
- Slot 7 copy `0x00444560`.
- The axis gizmo `0x004435b0`.
- The box outline `0x00443740`.
- The LOD chooser `0x00443de0`.

Not attempted: `0x00440810`, the drawing function `0x00440f30`, slot 12
`0x00443aa0` and three `ret` stubs.
