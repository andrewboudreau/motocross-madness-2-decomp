Shared constraint types (`ConstraintTypes.h`) and the promoted unit live here. The
remaining near miss is in `samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp`.
See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# ConstraintMethodCollisionModel.cpp (constraint)

Extent: `0x0043b800..0x0043caa0` (ControlInterface.cpp code follows). 19 targets of `src/krusty2/constraint/targets.json` pass strict
VC6 SP3 comparison with `ConstraintMethodCollisionModel.bindings.json`.

Evidence
- `__FILE__` string `D:\aardvark\VC\krusty2\ConstraintMethodCollisionModel.cpp` at `0x00568818`, one xref
  (`0x0043c7f9`, the DebugRealloc of AddProbePoint, line 331).
- RTTI `ConstraintMethodCollisionModel : CollisionObject`; primary vtable `0x00551260`, the GameObject-side
  vtable `0x005511f0` at object offset 12. sizeof `0x10c` (`new(0x10c, "wrecker.cpp", 0x68)` at `0x005302e8`).
- `.CRT$XCU` 66-69 are the unit's four kVec3 initializer pairs `0x0043c8e0..0x0043ca1b`.
- Start: the contact callback `0x0043b800`, which only slot 8 installs. The four pixel blend helpers before it
  (`0x0043b320..0x0043b7f9`, `samples/physics/constraint/PixelBlend.cpp`, all exact) have no literal or caller
  tying them to this unit; their ownership is open.

Functions (all in the extent)
- Exact: callback `0x0043b800`, constructor `0x0043b8d0`, deleting destructor `0x0043b950`, destructor core
  `0x0043b980`, slot 8 `0x0043b9a0`, SetBody `0x0043b9e0`, slot 10 `0x0043ba40`, slot 11 `0x0043ba70`,
  AddProbePoint `0x0043c7f0`, slot 14 `0x0043c880`, the eight `$E` functions `0x0043c8e0..0x0043c9e0`.
  The primary slot 2 target `0x0044d710` is the bare `ret` shared by six vtables; it lies outside the extent.
- Near miss: ApplyContactImpulse `0x0043bdb0` (2499/2619 strict; the source note lists what differs and the forms
  tried).
- Out-of-line copies of header inline vector operators, called by ApplyContactImpulse and emitted with this
  unit: `v / s` at `0x0043c890` (between slot 14 and the `$E` set) and the 3x3 row-vector rotation at
  `0x0043ca20` (after the `$E` set, ends `0x0043ca93`). Both are registered in
  `src/reconstructed/CollisionVectorHelpers.cpp` (both strict exact). This
  source declares them out of line in `ConstraintTypes.h`, so it does not emit them itself: in retail their
  constructor is expanded inside them while ApplyContactImpulse calls the constructor out of line, which is
  VC6's inline budget (docs/VC6_INLINE_BUDGET.md) and needs the original header's inline structure.
