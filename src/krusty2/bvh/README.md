# BoundingBoxTreeBuild.cpp (bvh)

Evidence
- `__FILE__` string `D:\aardvark\VC\krusty2\BoundingBoxTreeBuild.cpp` at VA 0x005682bc. Its own
  xrefs span 0x42b75e..0x42e257: DebugMalloc / debug-delete line numbers 0x24d..0x59e.
- Bracket 0x4245dc..0x42f657: after bmpfile.cpp's last xref and before CarProcedural.cpp. The
  file has no `__FILE__`-free neighbour of its own, but Camera.cpp (no `__FILE__` string) sorts
  between the two and starts at 0x42e340. That code belongs to another team.
- Extent, by evidence:
  - 0x4245f0..0x42ad21: run-time box/OBB query code. It has no `__FILE__` and is called from
    CollisionShapeTests, so it is probably a separate file. Six small entry points are in
    `samples/physics/bvh/BoundingBoxTreeQuery.cpp`; the large workers are not reconstructed.
  - 0x42ad30..0x42b5eb: partition and bounds helpers. They have no `__FILE__`, but only this
    file calls them. They are in `samples/physics/bvh/BoundingBoxTreeSplit.cpp`.
  - 0x42b5f0..0x42e2a3: the file's own functions. Every one references `__FILE__`, or sits between
    functions that do and uses the same types. All are promoted here.
  - 0x42e2b0: the out-of-line rigid inverse. It has no `__FILE__` and is called only from
    CollisionObject, so it is in `samples/physics/bvh/RigidTransform.cpp`.
- RTTI: none. The file has no classes, only free functions over plain structs.
- Layout (decoded accesses; names are tier 3; see the `// +0xNN` comments in BoundingBoxTreeBuild.h):
  - Interior node: 0x24 bytes, `{volume, center, halfExtents, child[2]}`; volume >= 0 marks an
    interior node.
  - Leaves: triangle 0x1c (ushort vertex[3], normal, planeOffset), box 0x1c, point 0x10.
    Each starts with marker -1.0f.
  - Build record: 0x2c bytes, sorted with `rep movsd` (ecx = 0xb).
  - Render-model LOD/mesh/part/vertex stand-ins: strides 8 / 0x38 / 0x14 / 0x20, from the
    model-gathering functions.
- Vector constants: four `$E` initializers (0x42d250..0x42d38b) for globals 0x579680 (zero),
  0x579690 (x), 0x5796a0 (y) and 0x579670 (z). Line numbers put them in the middle of the file,
  but GatherModelTriangles uses them earlier. They are therefore declared `extern` in the header
  and defined midway (tier 3; linkage is not visible in the image).

Counts: 21 exact, 2 partial of 23 targets (8 of the exact ones are the $E thunks and bodies).
- Partial: BuildModelPointTree 0x42d390, 99.56%. Two operand-order spots: the y row of one inlined
  TransformPoint, and a base/index order.
- Partial: GatherModelTriangles 0x42c260, 99.67%. One reload order and one base/index order.
Samples (`samples/physics/bvh`): 13 exact of 13.

Notes
- BuildBoxNode 0x42d9e0 keeps a retail bug: the right half's centres are copied back from
  `rightCenters[leftCount + i]`.
- The matrix code only matches when it goes through pointer-taking inline helpers
  (`InvertRigid(&m)`, `TransformPoint(&out, v, &m)`). Writing the same statements on a local
  matrix makes VC6 order the x87 operands differently.
- DebugMalloc 0x4a2e20 is declared locally because core/DebugAlloc.h lacks it.
