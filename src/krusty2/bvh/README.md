Candidate implementations for this area are preserved in `samples/physics/bvh/`.
Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# BoundingBoxTreeBuild.cpp (bvh)

The builder itself is now matched in `src/reconstructed/BoundingBoxTreeBuild.cpp`
(calibration; see docs/BOUNDINGBOXTREE.md). This header stays here for the
physics samples.

Evidence
- `__FILE__` string `D:\aardvark\VC\krusty2\BoundingBoxTreeBuild.cpp` at VA 0x005682bc. Its own
  xrefs span 0x42b75e..0x42e257: DebugMalloc / debug-delete line numbers 0x24d..0x59e.
- Extent: 0x42ad30..0x42e2a3.
  - 0x42ad30..0x42b5eb holds the partition and bounds helpers. Only this file
    calls them.
  - The code before 0x42ad30 is a separate, unattested unit (0x424690..0x42ad2f), with
    its own kVec3 `$E` set at 0x429400 and its own bss: the run-time queries in
    `BoundingBoxTreeQuery.cpp` (file name ours; near misses in
    `samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp`).
  - 0x42e2b0, the rigid inverse, is called only from CollisionObject. It is in
    `samples/physics/bvh/RigidTransform.cpp`.
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

Partials (`samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp`):
- GatherModelTriangles 0x42c260: one reload order and one base/index order.
- BuildModelPointTree 0x42d390: two operand-order spots.

Notes
- BuildBoxNode 0x42d9e0 keeps a retail bug: the right half's centres are copied back from
  `rightCenters[leftCount + i]`.
- The matrix code only matches when it goes through pointer-taking inline helpers
  (`InvertRigid(&m)`, `TransformPoint(&out, v, &m)`). Writing the same statements on a local
  matrix makes VC6 order the x87 operands differently.
- DebugMalloc 0x4a2e20 uses the shared declaration in core/DebugAlloc.h.
