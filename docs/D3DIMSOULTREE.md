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

Near misses (`samples/render/D3DIMSoulTreeNearMisses.cpp`, details in its
header):
- The LOD `.slt` reader `0x00440060` (1955 of 1964 bytes) and the
  vertex-group transform `0x00440d40`. Retail adds `offset + base`; a
  probe with only the face loop reproduces retail, so the swap is a
  whole-function effect.
- Slot 12 `0x00443aa0` (799 of 832): row 1 of the inline view-matrix
  product adds its terms in another order. VC6 canonicalises each sum by
  the kind of its leaves and destination, not by the source order.
- Slot 5 bounds `0x00444140` (610 of 764): the y/z sums of the centre.
- The texture-density pass `0x00440810` (same instruction count; one
  extra frame slot), the normals `0x00442fe0` and vertex crosses
  `0x004431f0` (their scales are function-local statics in .data at
  `0x00568944`/`0x00568948`), the LOD chooser `0x00443de0`, slot 7
  `0x00444560`, the axis gizmo `0x004435b0` and the box outline
  `0x00443740`.
- The drawing function `0x00440f30..0x00442dd8`: a nine-way switch on the
  material mapping type, then modifiers, draw and debug overlays. Its
  function-local statics are the .data ints `0x00568930..0x0056893c`, the
  float `0x00568940` and three vectors (`0x0057eef8`, `0x0057ef38`,
  `0x0057eee8`, guard `0x0057ef24`) with an empty destructor; the `ret`
  stubs `0x00442f40/50/60` are their atexit entries, so they belong to
  this TU and are matched only together with the drawing function.

Header note: adding the node calls `0x004fca80`, `0x004fc9a0`,
`0x004fd710`, `0x004fd7f0` and `0x004fd5c0` to `SoultreeObject` here
changes VC6's operand order in SceneManager.cpp's exact `0x004eb570`, so
the near-miss file declares them on a local view class.
