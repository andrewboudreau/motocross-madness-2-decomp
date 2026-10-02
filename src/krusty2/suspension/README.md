# Suspension (SelectiveGravityModel.cpp)

Evidence
- `__FILE__` string `D:\aardvark\VC\krusty2\SelectiveGravityModel.cpp` at VA 0x573f54
  (file offset 0x173f54, verified through the PE section mapping).
- Bracket 0x4f804d..0x4fdb7d; own `__FILE__` xrefs 0x4f97e5..0x4f9826
  (SelectiveGravityModel AddBody/slot 10: delete and realloc with line numbers 0x12, 0x18).
- RTTI: Shock (vptr +0, non-polymorphic MovingPart base +4), InlineShock and
  RotatingShock derive from Shock; SelectiveGravityModel derives from GameObject.
  The Shock family sits contiguously with the file's own methods (0x4f9a60 onward),
  which is the promotion basis.
- TU-private constant Vec3s at 0x689e58 (zero), 0x689e68 (+X), 0x689e78 (+Y), 0x689e48 (+Z).
  Their dynamic initializers are the bodies at 0x4fafe0/0x4fb030/0x4fb080/0x4fb0d0 (60 bytes,
  `_$E1/_$E4/_$E7/_$E10`); the addresses 0x4fafd0, 0x4fb020, 0x4fb070 and 0x4fb0c0 are
  5-byte `jmp` thunks (incremental-link entries) and are not part of the objects.
- RotatingShock::SolveContact (0x4fac60) uses a 4-entry jump table at 0x4fafc0; the extent
  is 880 bytes including the table.

Counts: 26 exact, 4 partial of 30 targets
- partial: slot 11 0x4f9890 (5.9%, stack-temp shape); UpdateAxis 0x4f9f90 (98.7%, `1.0f/len`
  gives `fld [1.0]; fdiv st(1)` instead of `fld st(0); fdivr [1.0]`, tried named locals,
  reordering, and component-wise forms); InlineShock::SolveContact 0x4fa400 (758/764 bytes,
  68%: the projection temporary is allocated in its own slot rather than reusing the dead
  `d` slot); RotatingShock::SolveContact 0x4fac60 (880/880 bytes, 94.1%: only the placement of
  the `mov [r.x], ecx` struct-copy relative to `fsubp` differs in the four cross-product cases).

Field and method names are tier 3 (provisional).
