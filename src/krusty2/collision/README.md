Collision units: `CollisionObject.cpp` and `CollisionPoint.cpp`, with their shared layout
headers (`CollisionObject.h`, `CollisionPoint.h`, `CollisionPointCtor.h`, `CollisionShapeTests.h`,
`CollisionTypes.h`). See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

## Collision setup and caller connections

`CollisionObject::Configure` at `0x004320f0` is supplied by the canonical
`CollisionObject.cpp`; its former standalone sample is removed. All 40 bytes,
including the call to `GameObject::SetRenderContext`, match retail. The first
argument passes through as the render-context pointer. Physics and scene callers
now share its `void*` signature; the pointee's complete type remains unproven.

ObjectPicker, CarProcedural and Wrecker now use this shared CollisionObject
declaration. Their constructor, setup, shape, placement and ignore-list calls
resolve to the existing implementations. Model/owner parameters remain opaque
where their complete types are unproven.

GraphicsTest's extent is modeled as `0x34` (tier 2): its constructor initializes
`+0x2c` and `+0x30`; CarProcedural and Wrecker place their first pointers at
`+0x34`, while SoultreeQuadTreeRenderer keeps its own node count there.
PhysicsBody's `+0x34` field is declared in that derived class. CollisionObject's
bounding-sphere center belongs to CollisionObject at `+0x40`, immediately after
the secondary GraphicsTest base at `+0xc`. It no longer aliases a purported
GraphicsTest field. CollisionObject remains `0xb8` bytes and the Wrecker
collision-model view remains `0x10c`; all accessed field offsets are retained.

`tests/test_collision_link.py` checks these layouts with VC6 and verifies that
real production caller relocations name the real collision implementations.
This does not establish the original header structure or reconcile every
remaining scene-node and constraint-model boundary view.

# CollisionPoint.cpp (collision)

Extent: `0x0043a1e0..0x0043b320` (CollisionObject.cpp's last function `0x0043a050` precedes it,
the pixel blend helpers at `0x0043b320` follow). 16 targets of `src/krusty2/collision/targets.json`
pass strict VC6 SP3 comparison with `CollisionPoint.bindings.json`.

Evidence
- `__FILE__` string `D:\aardvark\VC\krusty2\CollisionPoint.cpp` at `0x00568730`, one xref
  (`0x0043a347`, the debug `new(0xc4, __FILE__, 27)` in AddCollisionPoint `0x0043a330`).
- RTTI `CollisionPoint` (no bases), vtable `0x005511d8`: slot 0 the deleting destructor `0x0043b300`,
  slot 1 `0x0043b240`.
- `.CRT$XCU` 62-65 are the unit's four vector initializer pairs `0x0043b050..0x0043b18b` (zero vector
  `0x005797f0` and y axis `0x00579810` are read by the unit's code).
- The two sphere tests `0x0043a1e0`/`0x0043a270` before AddCollisionPoint have no literal tying them to
  either neighbour; whether they open this unit or close CollisionObject.cpp is open (tier 3).

Functions (all in the extent)
- Exact in `CollisionPoint.cpp`: AddCollisionPoint `0x0043a330` (inline constructor from
  `CollisionPointCtor.h`), AddExistingCollisionPoint `0x0043a570`, RemoveCollisionPoint `0x0043a5e0`,
  UpdateRelativeMotion `0x0043a640`, UpdateCollisionPointWorldPositions `0x0043aff0`, the eight `$E`
  functions `0x0043b050..0x0043b150`, CollisionRejectFrom `0x0043b190`, slot 1 `0x0043b240` and the
  deleting destructor `0x0043b300`.
- Exact in `samples/physics/collision/CollisionQueries.cpp` (ownership open, see above):
  SphereContainsPointXZ `0x0043a1e0`, BoundingSpheresOverlap `0x0043a270`.
- Near misses in `samples/physics/collision/CollisionPointNearMisses.cpp` (includes this file): the
  contact merge `0x0043aa30` and the contact refresh `0x0043ad80`; their notes list what differs.

Source shapes that matter (tier 3: they reproduce retail, the original text is unknown)
- UpdateRelativeMotion uses header-shaped inline helpers: a dot product grouped `z + (x + (y))` at both
  expanded sites, the cross product with parenthesised products, and operators returning through the
  three-float constructor. The function exhausts VC6's inline budget, so retail calls the out-of-line
  constructor `0x00404e60` inside the expanded operators and the out-of-line dot `0x0040ae30`, length
  `0x00435ec0` and division `0x0043c890` at its later sites (docs/VC6_INLINE_BUDGET.md).
- Slot 1 returns its product through an inline three-float constructor (`CollisionPointVec3`); with
  CollisionTypes.h's member-store `operator*` VC6 issues the y copy store before the z `fstp`.
- CollisionRejectFrom's dot product is `a.x*b.x + (a.y*b.y) + a.z*b.z`: retail emits y, x, z with
  `b.y` loaded first.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/collision/CollisionPoint.cpp \
  --source samples/physics/collision/CollisionPointNearMisses.cpp \
  --source samples/physics/collision/CollisionQueries.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The run reports `18/20 strict exact` (the two near misses are partial targets).
