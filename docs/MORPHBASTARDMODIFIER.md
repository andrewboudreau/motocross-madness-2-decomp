# MorphBastardModifier.cpp

`src/reconstructed/MorphBastardModifier.h` / `MorphBastardModifier.cpp`.
Names are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x0056df3c`, xrefs
  `0x004a3215..0x004a5447`.
- **RTTI:** `MorphBastardModifier : D3DIMSoultreeModifier : GraphicsTest :
  GameObject`, vtable `0x00555230`. It overrides slot 0 and the pure slot
  27.
- **Caller:** KrustyBike.cpp `0x004910de` allocates 0x58 bytes, constructs
  it, and loads "RiderMorph.mbf" through `0x004a33b0`.

Extent: `0x004a3150..0x004a562b`.
- `0x004a3120` before it is a memcpy helper used only by DebugRealloc, so
  it belongs to the allocator's unit.
- The unit ends with its four kVec3 `$E` pairs.
- Motnctrl.cpp starts at `0x004a5630`.

Exact: 15 calibration cases. These are the constructor, the destructor and
its deleting wrapper, the qsort comparator `0x004a3bb0`, the vector angle
`0x004a3be0`, the per-object channel update `0x004a4bb0`, the mesh copy
`0x004a5290` and the eight `$E`. Source forms needed:
- The dot product in `0x004a3be0` is written `a.z*b.z + a.x*b.x + a.y*b.y`.
- MorphBastardObject has an empty constructor, which gives retail's
  inlined `new[]` countdown.
- The mesh's +0x20/+0x24 pair is one 8-byte struct.

Near misses (`samples/render/MorphBastardModifierNearMisses.cpp`):
- The 2046-byte parameter-file loader `0x004a33b0` (1938 of 2046): an
  existing channel records the target index `j`; the stack homes of the loop
  scalars and the operand order of the channel inverse's products differ.
- Slot 27 `0x004a4c60`.
- The controller-angle solver `0x004a3c80..0x004a4ba5` (3878 bytes, not
  2340): structure reconstructed; retail calls the out-of-line Vector3
  constructor `0x00404e60` and has a larger frame.

Slot 27 differs in stack-slot and register assignment.

D3DIMSoultreeModifier.h now gives slot 27 its signature `(object, mesh,
out)`, taken from the call site `0x00440ded`.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004a3bb0` `CompareTargets`
- `0x004a3be0` `AngleBetween`
- `0x004a4bb0` `UpdateChannels`
- `0x004a5290` `CopyMesh`
