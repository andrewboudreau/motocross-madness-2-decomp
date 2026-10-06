# Suspension (Shock, InlineShock, RotatingShock)

Evidence
- RTTI: Shock (vptr +0, non-polymorphic MovingPart base +4); InlineShock and
  RotatingShock derive from Shock.
- The family occupies 0x4f9a60..0x4fb1d7, directly after SelectiveGravityModel.cpp's code
  (promoted to `src/krusty2/gravity/`) and before soultree.cpp's first `__FILE__` xref
  (0x4fdb7d). It references no `__FILE__` string of its own.
- It is a separate translation unit from SelectiveGravityModel.cpp (strong inference): each TU
  that includes math/Math3D.h builds its own four constant vectors, and the two code ranges
  build two different sets. SelectiveGravityModel uses 0x689e08..0x689e40 (initializers at
  0x4f9910..0x4f9a4b); this family uses the zero 0x689e58, +X 0x689e68, +Y 0x689e78 and
  +Z 0x689e48 (initializers below). The retail file name is unknown; `Shock.cpp` is ours.
- The initializers are the `$E2`/`$E1` .. `$E11`/`$E10` pairs: 5-byte `jmp` wrappers at
  0x4fafd0, 0x4fb020, 0x4fb070 and 0x4fb0c0, and the 60-byte bodies at 0x4fafe0, 0x4fb030,
  0x4fb080 and 0x4fb0d0.
- RotatingShock::SolveContact (0x4fac60) uses a 4-entry jump table at 0x4fafc0; the extent
  is 880 bytes including the table.

Diagnostic counts: 20 relocation-masked matches, 3 partial of 23 targets.
These are not strict acceptance; reviewed bindings are still required.
- partial: UpdateAxis 0x4f9f90 (98.7%, `1.0f/len` gives `fld [1.0]; fdiv st(1)` instead of
  `fld st(0); fdivr [1.0]`, tried named locals, reordering, and component-wise forms);
  InlineShock::SolveContact 0x4fa400 (758/764 bytes, 68%: the projection temporary is
  allocated in its own slot rather than reusing the dead `d` slot); RotatingShock::SolveContact
  0x4fac60 (880/880 bytes, 94.1%: only the placement of the `mov [r.x], ecx` struct-copy
  relative to `fsubp` differs in the four cross-product cases).

Field and method names are tier 3 (provisional).
