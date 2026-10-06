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
- The 1982-byte parameter-file loader `0x004a33b0`.
- Slot 27 `0x004a4c60`.

Both differ in stack-slot and register assignment. Not written: the
2340-byte axis-angle solver `0x004a3c80`; the sample's header describes
it.

D3DIMSoultreeModifier.h now gives slot 27 its signature `(object, mesh,
out)`, taken from the call site `0x00440ded`.
