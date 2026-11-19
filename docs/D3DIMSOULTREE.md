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

Near misses, in `D3DIMSoulTree.cpp` in retail order with `// Not exact`
status comments ([forms tried](#near-misses-forms-tried)). They replaced the
game build's bring-up copy `src/bringup/Group5_D3DIMSoulTreeDraw.cpp`, whose
bodies they are:
- Slot 12 `0x00443aa0` (802 of 832): row 1 of the inline view-matrix
  product adds its terms in another order. VC6 canonicalises each sum by
  the kind of its leaves and destination, not by the source order. `_21`,
  `_23` and `_24` are bracketed in retail's addition order, which
  `tools/emu_cases/soultree_prepare_geometry.py` shows at 24-bit precision.
- The drawing function `0x00440f30..0x00442dd8` (751 of 7851): a nine-way switch on the
  material mapping type, then modifiers, draw and debug overlays. Its
  function-local statics are the .data ints `0x00568930..0x0056893c`, the
  float `0x00568940` and three vectors (`0x0057eef8`, `0x0057ef38`,
  `0x0057eee8`, guard `0x0057ef24`) with an empty destructor; the `ret`
  stubs `0x00442f40/50/60` are their atexit entries (`_$E4`, `_$E3`,
  `_$E2`, strict exact), so they belong to this TU. Defining the drawing
  function before the kVec3 constants numbers those three first and moves
  the eight kVec3 `$E` up by four (`_$E5`..`_$E15`). Mode 5's group index
  `k` starts at 0 (best guess: retail reads the restore loop's leftover
  value, uninitialised before the first restore). Mode 2's scroll distance
  uses its own `ScrollLength` (y*y, x*x, z*z): with the unit's `Length`
  grouping the emu case differs from retail in 17 of 480 runs (751 against
  758 bytes for the scored form, which is the one kept).

Header note: adding the node calls `0x004fca80`, `0x004fc9a0`,
`0x004fd710`, `0x004fd7f0` and `0x004fd5c0` to `SoultreeObject` here
changes VC6's operand order in SceneManager.cpp's exact `0x004eb570`, so
the unit declares them on a local view class (`DrawSoultreeObjectView`,
forwarded by `src/bringup/Group5_SoultreeNodeView.cpp`).

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

## Near misses: forms tried

Carried over from the header of the former `samples/render/D3DIMSoulTreeNearMisses.cpp` when its bodies (in the bring-up copy's linked form) moved into D3DIMSoulTree.cpp. Scores below are the sample's.

```text
// Near-miss D3DIMSoulTree.CPP candidates (D3DIMSoultreeObject), kept out of
// src/reconstructed/D3DIMSoulTree.cpp until they match. The class, its
// evidence and the exact functions are in src/reconstructed/D3DIMSoulTree.h
// and .cpp. Bindings: D3DIMSoulTreeNearMisses.bindings.json.
//
// PrepareGeometry (0x00443aa0, 832 bytes): 799/832. The inline view
// matrix product only gets retail's `add eax, 0xec` base when the camera
// is not held in a local; named result fields (_11.._44) fix the term
// order of rows 0, 2 and 3. Row 1 still adds its four products in a
// different order. VC6 canonicalises each sum (the source term order and
// the statement order of the rows do not matter); the order changes with
// the kind of the leaves and of the destination (a row pointer for row 1:
// 808 of 832; operator(): 787; `view.m[r][c]` stores: 768; a `float*`
// or pointer/reference to `view`: 739), not with symbol numbering (dummy
// locals), a plain struct instead of the union, projection pointers or
// flattened indices (unchanged), and no uniform form gives retail's row 1.
// The product as an inline helper (reference or pointer result, by-value
// left operand, or returning the matrix) is not inlined by VC6. Redundant
// parentheses (docs/VC6_OPERAND_ORDER.md section 3): one bracketing for the
// whole row gives 806; a different one per element of row 1 reaches 826
// (_21..._23 retail's order, _24 still t1, t3, t2, t0 where retail sums t3,
// t2, t1, t0, unchanged by its 264 orders); not kept here, since nothing ties
// those per-element forms to the source. The linked copy
// (src/bringup/Group5_D3DIMSoulTreeDraw.cpp) does bracket _21, _23 and _24:
// there every row 1 sum gets retail's grouping (_22 differs only by
// commutation), which tools/emu_cases/soultree_prepare_geometry.py shows
// at 24-bit precision. Any extra local, even an unused
// `const Matrix4& p = projection;` or `Matrix4* w = &world;`, reorders the
// sums globally (737); reference aliases of world/projection on one or all
// row 1 leaves score the same as the bare declaration (references to locals
// fold), and a second reference to the camera's view matrix gives 160.
// Dead initialised int locals (2026-10-11): 0..33 at the top and 0..31 just before row 1
// give the same scores by count (799 at 0 and 29..33, otherwise 706..776), so the count
// moves all rows together and no count gives retail's row 1.
// One term per statement (2026-10-20, 625 forms: row 1 summed into the
// field or into a float local, terms t0 up or t3 down): only `_24` summed
// into a local from t3 down moves it (805); the same form on _21.._23 gives
// 739..741, so no uniform form exists and none is kept.
//
// DrawCurrentLod (0x00440f30..0x00442dd8, 7848 bytes): the drawing
// function. A nine-way switch on the material mapping type (+0xc8; source
// order 0, 7, 1, 3, 4, 8, 2, 5, 6 from the case layout) rewrites the
// source texture coordinates, then the modifiers, the texture
// coordinate transform 0x00510910, the draw (slot 15) and the debug
// overlays run, and the coordinates are restored from +0x28. The .data
// ints 0x00568930..0x0056893c (all 1) and the float 0x00568940 (20.0) are
// function-local statics, as are the three vectors 0x0057eef8,
// 0x0057ef38 and 0x0057eee8 (guard 0x0057ef24 bits 1, 2, 4) whose type has
// an empty destructor: their atexit entries are the `ret` stubs
// 0x00442f60, 0x00442f50 and 0x00442f40. Same structure (1895 of 1893
// instructions, 484 differing, then 7832 bytes): the three mode 8 and mode 2
// normalisations return through the out-of-line Vector3 constructor
// 0x00404e60 as retail does (Vector3Call; the inline budget, with the
// constructor expanded in the mode 5 and 6 normalisations). Mode 5's
// `lines` has 8 entries (4 used). This is a strong inference from the
// frame, not confirmed: retail leaves 0x80 bytes after it that nothing
// touches, and with 8 entries circle, ring, identity and line all sit
// 0x40 below their retail slots (tools/frame_layout.py). Sweeping 13
// choice points (8192 forms; insn diffs 507 to 495, aligned 1386 to 1398)
// kept identity(r, c) stores, a reference to the light position (since
// replaced by retail's by-value copy, see below) and a
// separate normalisation in mode 3. Accumulating the squared lengths and
// the mode 5 and 6 dot products one term per statement (512 forms; the
// MPBikeRiderDlg lever in docs/VC6_OPERAND_ORDER.md) gives 484 differing
// and 1409 aligned; Length adds y first, the normalisations x first. Left: retail's frame is 0x9e8 (ours
// 0x9a8) with temporaries the source does not have yet (mode 2 reads the
// vertex's y and z through a stack copy, x straight from the vertex; a
// named copy, a copy constructor, a by-value difference and three-float
// constructor forms all copy x too; x subtracted before a named copy keeps
// retail's loads but adds a dead x store (713); y and z in float locals,
// alone or with x read in place, are folded away (744)), `this` is in esi where ours is in
// ebx, and the identity diagonal is stored from m[3][3] down (every order
// and spelling of the four stores is emitted from m[0][0] up).
// Session 16 (masked linked metric 0.8842 -> 0.8911, 342 -> 317 differing
// instructions; every float-constant operation is now retail's, in
// retail's order, checked by value): mode 2 halves the reflection into
// two float locals before adding 0.5 (a Vector3 half multiplies both
// components before either add; `x * 0.5f + 0.5f` is factored by VC6
// to `(x + 1) * 0.5`, not retail's code), and the scroll step is a named
// `moved` added after lastScrollPosition is stored, which gives retail's
// `fcom 100; fst textureScroll` (the in-place `+=` before the store
// reloads it). Mode 3 copies the light position by value, as retail
// does (add edi, 0x64, then three dword stores to the frame); it costs
// strict bytes but removes differing instructions. No change: every order
// of the mode 2 and mode 3/4 dot-product terms, the third world-normal
// row's order, by-value operator- operands (worse), scale-first or
// mixed operand order in the normalisations (identical code).
// The mode 5 source keeps two retail bugs: `circle[i + 1]` reads past the
// 16 points, and the group loop indexes with the restore loop's `k`.
// Behaviour (tools/emu_cases/soultree_draw_current_lod.py, 2026-11-11): several
// sums were added in a different order from retail's, an ulp off at 24 bits:
// the mode 3/4 tu dot products, mode 2's world normal and facing, and mode 5's
// world normal. Mode 5's ray also stored dx to a float where retail keeps
// dx/dz on the x87 stack (an ulp at 53 bits). The forms here follow retail;
// together they raise the linked masked metric from 0.891 to 0.898 (this copy:
// 473 differing, 1422 aligned).
//
// k-scan (s22): compiled behind 0..63 prepended typedefs, 0x00440f30,
// 0x00443aa0 have the same code at every count, so their differences are not
// a declaration-count effect (docs/VC6_OPERAND_ORDER.md section 2).
```
