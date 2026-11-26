Shared constraint types (`ConstraintTypes.h`) and the promoted unit live here. The
remaining near miss, ApplyContactImpulse, sits in its retail position in the unit.
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
  (`0x0043b320..0x0043b7f9`, all exact in `src/reconstructed/PixelBlend.cpp`) have no literal or caller
  tying them to this unit; their ownership is open.

Functions (all in the extent)
- Exact: callback `0x0043b800`, constructor `0x0043b8d0`, deleting destructor `0x0043b950`, destructor core
  `0x0043b980`, slot 8 `0x0043b9a0`, SetBody `0x0043b9e0`, slot 10 `0x0043ba40`, slot 11 `0x0043ba70`,
  AddProbePoint `0x0043c7f0`, slot 14 `0x0043c880`, the eight `$E` functions `0x0043c8e0..0x0043c9e0`.
  The primary slot 2 target `0x0044d710` is the bare `ret` shared by six vtables; it lies outside the extent.
- Near miss: ApplyContactImpulse `0x0043bdb0` (2515/2619 strict, the `partial` target; the status comment
  lists what differs, the forms tried are below).
- Out-of-line copies of header inline vector operators, called by ApplyContactImpulse and emitted with this
  unit: `v / s` at `0x0043c890` (between slot 14 and the `$E` set) and the 3x3 row-vector rotation at
  `0x0043ca20` (after the `$E` set, ends `0x0043ca93`). Both are registered in
  `src/reconstructed/CollisionVectorHelpers.cpp` (both strict exact). This
  source declares them out of line near the top of the .cpp, so it does not emit them itself: in retail their
  constructor is expanded inside them while ApplyContactImpulse calls the constructor out of line, which is
  VC6's inline budget (docs/VC6_INLINE_BUDGET.md) and needs the original header's inline structure.

Forms tried for ApplyContactImpulse

Session 14 also tried: num and termA's dots with every grouping (inert once termB is
grouped); every parenthesisation of the six products in vA's cross (64 forms, the kept
one is best; A=0 fewer runs (10) but 2495); every parenthesisation of termB's two
crosses, wB x rB and axisA x rl (64 forms each, all 2499 or lower).
Forms tried without reaching those (about 35): dot product as by-reference or by-value
helper with every grouping of the three terms (x+y)+z, flat, z+(x+y), (y+x)+z, x+(y+z),
(x+z)+y, y+(x+z), both argument orders, written out with a named cross local; termB through
a helper function (exceeds the inline budget: the transform is then called out of line);
the declaration/initialisation order of invMassB/termB and the denominator grouping; axisB
as a declared-then-assigned local or folded into one expression; rA as a separate cA local
(wrong frame); vB declared at other positions; parenthesised products in the cross product
(Math3D's form also exceeds the inline budget: J's scale is then called out of line).
Session 12: the cross product's term order and operand order and the dot product's grouping
and argument order (32 combinations through separate helpers) all compile to the same bytes:
VC6 canonicalises them here, so the termB difference is not an operand-order choice.
Also identical: termB's transform or either cross computed into float locals before the
result is filled (the store-schedule lever of docs/VC6_OPERAND_ORDER.md 1.3); with both
crosses and the transform in that form the inline budget runs out (ConScale is called).
