# D3DIMSoulTree.CPP

`src/reconstructed/D3DIMSoulTree.h` / `D3DIMSoulTree.cpp`. LightEmitter.h
includes the header, so its consumers share the same declarations.
Names other than the RTTI classes are provisional.

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
(`UnknownSoultreeQuadTreeObject`, GameObject). Its primary hierarchy methods
now share the [loading and cloning declarations](SOULTREE_HIERARCHY_CONNECTIONS.md)
with the canonical soultree provider. Their slots remain 2, 3, 6, 7 and 8;
the separate GameObject base remains at offset `+0xc`.

Exact: 52 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- The `.slb`/`.slt` loader (slot 9), the binary load (slot 2) and the
  materials (slot 3).
- The clone (slot 4), slots 5, 6 and 8 and GameObject slots 4, 5, 10 and 14.
- The wireframe and box outlines, the normals `0x00442fe0` (static scale
  `0x00568944`), the vertex crosses `0x004431f0` (static scale
  `0x00568948`), the axis gizmo `0x004435b0`, the box outline
  `0x00443740`, the LOD chooser `0x00443de0`, the surface buffers, the LOD
  setters and preset, the two modifier lists, the material copy and
  texture, and the counts.
- Slot 5 (the local bounds) writes the centre sum per component (through
  operator+ VC6 adds y and z high-first) and stores the valid flag last.
- The texture-density pass `0x00440810` (1325 bytes; the old 1349 was the
  candidate's length) copies the three corner positions per component, as
  from a D3DTLVERTEX's sx/sy/sz. With whole-Vector3 copies VC6 keeps a
  second pointer to each vertex for the copy, runs out of registers and
  spills the face offset (frame 0x84); retail keeps it in ebx. Its squared
  length is grouped `z*z + (x*x + y*y)` like the LOD chooser's.
- What made the last four exact: the transformed points are copied into
  the vertices per component (normals, gizmo, box), the box corners are
  stored per component (VC6 then shares each negated extent, retail's
  fchs/fst chains), the normals' `from` and `line` are function-scope
  locals, and the LOD chooser groups its squared lengths
  `z*z + (x*x + y*y)` and its products `(scale * a) * b` and sets the
  bounds pointers before the bounds call.
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
- Slot 7 `0x00444560` matches once the surface pointer `d` is declared
  before `s`.
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

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00440060` `ReadLods`
- `0x00440810` `UpdateMaterialTexelRatios`
- `0x00440d40` `TransformVertexGroups`
- `0x00440f30` `DrawCurrentLod`
- `0x00442f70` `DrawWireframe`
- `0x00442fe0` `DrawNormals`
- `0x004431f0` `DrawVertexCrosses`
- `0x004434b0` `ComputeSubtreeBounds`
- `0x004435b0` `DrawAxisGizmo`
- `0x00443740` `DrawBoxOutline`
- `0x00443de0` `SelectLod`
- `0x00445030` `GetSurfaceCount`
- `0x00445060` `NodeMovesVertices`
- `0x004452f0` `AddObject`
- `0x00445360` `RemoveObject`
- `0x004a1b00` `TransformPoints`
- `0x004fb4f0` `UpdateWorldMatrix`
- `0x004fca60` `GetLocalMatrix`, `0x004fca80` `GetMatrixIn`, `0x004fd5c0`
  `LocalToWorldDirection`, `0x004fd660` `LocalToWorldPoint` and `0x004fd710`
  `WorldToLocalDirection` (krusty2's names for those addresses; the lane
  views' declarations use them, the classes stay local views)
- `0x004fda60` `CollectDescendants`
- `0x004fdb60` `LoadFromParameters`
- `0x004fedb0` `RegisterNode`

## Field names

`SoultreeObject` fields named from their uses here and in
`src/krusty2/core/SoultreeObject.h` (tier 3 names, tier 1 offsets; each
declaration keeps its `// +0xNN`): `nodeName` (+0x038, compared by
`FindByName` 0x004fdae0), `localBoundsValid`/`localBoundsCenter`/
`localBoundsHalfExtent` (+0x154/+0x158/+0x164, written from the low/high
box as centre and half extent by `D3DIMSoultreeObject` slot 5,
0x00444140), `subtreeBoundsCenter`/
`subtreeBoundsHalfExtent` (+0x174/+0x180, refreshed by 0x004fe0f0) and
`subtreeDirty` (+0x18c, set up the parent chain by 0x004fdab0).
`D3DIMSoultreeObject` +0x27c is `currentLod`, the current level of detail.

## Inherited model helpers

The render declaration now inherits these four existing `SoultreeObject`
providers instead of declaring duplicate D3DIMSoultreeObject methods:

| Provider | Retail VA | Complete bytes |
| --- | --- | ---: |
| `SetAxesPtr(const Vec3*, const Vec3*, int, int)` | `0x004fbd70` | 733 |
| `SetPosition(const Vec3&)` | `0x004fc660` | 38 |
| `Rotate(Vec3, float)` | `0x004fceb0` | 465 |
| `GetSubtreeBounds(Vec3*, Vec3*)` | `0x004fe850` | 74 |

The bodies are in `src/krusty2/soultree/soultree.cpp`. Retail RTTI places the
SoultreeObject base at offset zero within D3DIMSoultreeObject and GameObject
at `+0x0c`; these calls use the complete node pointer, without a GameObject
adjustment. `SetPosition` receives the address of three floats and invalidates
the world matrix after storing the local translation. `Rotate` receives its
axis by value, preserves the local translation around the rotation product,
and invalidates the world matrix. `GetSubtreeBounds` refreshes dirty bounds
before copying the center and extents at `+0x174/+0x180`. The two axes passed
to `SetAxesPtr` remain pointers, with their two integer flags unchanged.

ArcadeObject, Krusty3DObjects and D3DIMSoulTree keep their local `Vector3`
buffers and use explicit casts at the canonical `Vec3` boundaries. The
three-float layouts are checked with VC6; the by-value Rotate call still
copies the complete vector before calling. No wrapper or helper body is
added. The render header keeps its existing multiple inheritance; the
separate flat physics header is not imported into these callers. These
connections establish four provider identities, not original TU ownership.

`SetPositionVec3` and `Scale` remain unresolved boundaries. The former shares
the retail three-float entry at `0x004fc630`, but replacing LightEmitter's
vector argument with three scalar arguments changes its complete emitted
body; that change is not included. Scale's body is still a partial sample.

`tests/test_render_node_connections.py` checks the four complete providers,
actual COFF imports/definitions, RTTI offsets, native layouts, and all 125
calibration cases from D3DIMSoulTree, ArcadeObject, Krusty3DObjects and
LightEmitter. Together with the vehicle-model regression, 477 focused cases
remain strict. The affected partial D3DIMSoulTree, Krusty3DObjects and
SceneManager sample bodies retain their previous sizes and matching-position
counts; they remain samples. The combined build with the separate Bike query
connections removes six unresolved names. With the separately reviewed
Character playback/VUE candidate and camera/Wrecker math connections, the
current build compiles 184/184 units and has 582 unresolved names.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_render_node_connections.py -v
python -m unittest discover -s tests -p test_vehicle_model_connections.py -v
```
