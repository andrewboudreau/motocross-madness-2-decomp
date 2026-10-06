# BoundingBoxTreeBuild.cpp and GR_BitString

**BoundingBoxTreeBuild.cpp** (`0x0042ad30..0x0042e2a3`,
`src/reconstructed/BoundingBoxTreeBuild.*`; layouts and evidence in
`src/krusty2/bvh/README.md`). It moved from `samples/physics/bvh`; the old
sample copies were removed. Exact: 27 calibration cases. These are the six
split and bounds helpers, the three split choosers, the triangle, point
and box node and tree builders, AxisAngleMatrix, the three readers and
the eight `$E`. Its bindings were read back from the retail relocation
sites after a masked match. Each float constant was checked against its
stored value, and the kVec3 addresses against the `$E` bodies. The stream
stand-in is now the shared `UnknownTextureStream`. Near misses
(`samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp`):
GatherModelTriangles `0x0042c260` and BuildModelPointTree `0x0042d390`.

The code before `0x0042ad30` is a separate, unattested unit. It has its
own kVec3 `$E` set (`0x00429400`), its own bss, and the five empty `$E`
pairs at `0x00424690`. The `.CRT$XCU` order is bikerace.cpp, `0x00429400`,
`0x00424690`, then `0x0042d250`. That unit is now
`src/krusty2/bvh/BoundingBoxTreeQuery.cpp` (its name is ours); see
[PHYSICS_VALIDATION](PHYSICS_VALIDATION.md).

**GR_BitString** (`0x004238c0..0x00423f6d`, `src/reconstructed/GR_BitString.cpp`;
file name not recovered). This is a separate unit (strong inference):
- It has no `__FILE__` and no `$E`.
- Its `.data` block (`0x00567f90..0x00568227`) holds byte masks, the
  bit-reverse table and mask tables used only by this code, followed by the
  GR_PixelString and GR_BitString type descriptors.
- Its `.bss` is used only here.

BlockAllocator.cpp and bmpfile.cpp, which follow it, were already fully
matched.

Exact: 8 calibration cases. These are the constructor, the destructor and
its deleting wrapper, slots 0-3, and the whole-word back shift
`0x00423940`. GR_BitString.h drops the inline `~GR_PixelString`. With it,
VC6 gives `~GR_BitString` an EH frame that retail lacks. Near misses
(`samples/render/GR_BitStringNearMisses.cpp`): the three other shift
helpers, which differ in register swaps and OR operand order. The bit
reader `0x00423ef0` looks like an `__asm` body and is not attempted.
