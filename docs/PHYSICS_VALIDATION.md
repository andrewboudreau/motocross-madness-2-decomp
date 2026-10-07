# Physics reconstruction and verification

The physics sources include verified implementations and unfinished candidates.
The same readable C++ is retained while those two claims stay separate.

## Reviewed wave-4 slice

Authentic VC6 SP3, profile `vc6_o2_ml`, reproduces all bytes for 48 cases
(47 distinct retail address/extent pairs) in:

- `src/krusty2/visibility/VisibilityQuadTree.cpp`: 16 cases.
- `src/krusty2/effects/NormalDistribution.cpp`: 6 cases.
- `src/krusty2/effects/Nulls.cpp`: 6 cases.
- `src/krusty2/effects/Particles.cpp`: 6 cases.
- `src/krusty2/motion/Spheres.cpp`: 14 cases.

Every relocation is resolved by the adjacent binding files; none is masked.
NullManager and SphereManager both reproduce the shared 19-byte slot-8 body,
so their two cases count as one retail function. This does not establish their
original source spelling or exclusive ownership of a shared address.

The review corrected the persistent collision-tree pointer versus the active
traversal pointer, NormalDistribution's malloc-style debug allocation call,
and the two missing slot-8 overrides. Particle update flags now have provisional
names describing the decoded operations. Readability changes preserve codegen.
See the effects, motion and visibility READMEs for binding evidence.

Reproduce only the reviewed slice after private-input setup:

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/visibility/VisibilityQuadTree.cpp \
  --source src/krusty2/effects/NormalDistribution.cpp \
  --source src/krusty2/effects/Nulls.cpp \
  --source src/krusty2/effects/Particles.cpp \
  --source src/krusty2/motion/Spheres.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

## Reviewed wave-5 slice

A further 41 cases pass strict VC6 SP3 comparison under the same profile:

- `src/krusty2/shadow/D3DIMSoultreeShadow.cpp`: 16 cases, including the
  generated destructor core selected by its deleting wrapper.
- `src/reconstructed/TerrainSupport.cpp`: 25 cases covering global camera/timer
  initialization, owned-texture acquisition, and the height-range getter.

These two sources have adjacent reviewed bindings. TerrainSupport is our slice
filename, not a recovered retail TU name; the source attribution is Terrain.cpp.
Its timers reuse the canonical UnknownPeakHold type, and the texture constructor
arguments use TextureMapManager pointers. The remaining Terrain implementation
is unchanged in scope and is not included in this strict claim.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/shadow/D3DIMSoultreeShadow.cpp \
  --source src/reconstructed/TerrainSupport.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

Across the two reviewed slices there are 89 cases, or 88 unique retail
address/extent pairs. Full-corpus totals also include older targets and are
not additive to the calibration progress snapshot.

## Wave-6 promoted slice

Four retail TUs moved from `samples/physics/` into `src/krusty2/`. The ownership and
binding evidence is in `src/krusty2/README.md`. 169 cases pass strict VC6 SP3
comparison:

- `src/krusty2/collision/CollisionObject.cpp`: 44 of 61 targets.
- `src/krusty2/vehicle/Vehicle.cpp`: 57 of 77 targets.
- `src/krusty2/vehicle/Bike.cpp`: 31 of 45 targets.
- `src/krusty2/soultree/SoulTreePhysics.cpp`: 37 of 43 targets.

Three targets are `expect: "masked"` because one called constructor or helper has no
independent identity yet. The rest are documented `partial` code-generation
mismatches. `tools/propose_bindings.py` drafted the bindings. It refuses to bind a
symbol away from its own target address, and that refusal found a case-order error
that the masked check had accepted (TestHullAgainst/TestModelAgainst).

```bash
python tools/run_physics_samples.py --strict \
  --root src/krusty2/collision \
  --root src/krusty2/vehicle \
  --root src/krusty2/soultree \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `169/226 strict exact` and exits nonzero. `--strict` counts the three
`masked` targets as required failures because they lack bindings. Those failures mark
incomplete evidence, not byte mismatches.

## Wave-7 D3DIMSoultree slice

Two more sources pass strict comparison:

- `src/krusty2/shadow/D3DIMSoultreeShadow.cpp` now passes 17 cases. Slot 14,
  `0x447540`, was added.
- `samples/physics/motion/D3DIMSoultreeMotnctrl.cpp` passes 23 of 26
  targets. These are the D3DIMSoultreeCharacter methods and the four vector
  `$E` pairs, with `D3DIMSoultreeMotnctrl.bindings.json`.
  - Slot 2 `0x445fc0` is one byte off.
  - Slot 4 `0x446210` (98.6%) differs in register choice.
  - Slot 11 `0x445680` is at 39.6%.

Motnctrl stays in `samples/` because those three targets are still partial.
Of its bindings, the seven that `tools/propose_bindings.py` could not prove
were checked by hand. One of them is `_strupr` at `0x535d3d`; that CRT
identity is provisional.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/shadow/D3DIMSoultreeShadow.cpp \
  --source samples/physics/motion/D3DIMSoultreeMotnctrl.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `40/43 strict exact` with no required failures.

## Wave-8 promoted slice

`src/krusty2/soultree/SoultreeQuadTreeRenderer.cpp` (8 cases, moved from
`samples/physics/soultree_base/`) and `src/krusty2/gravity/SelectiveGravityModel.cpp`
(15 cases) pass strict comparison.
- SelectiveGravityModel's slot 11 needs `Vec3(0,-1,0) * (b->mass * gravity)`.
- Its SetGravity is an inline virtual, which places it after the `$E` code
  as retail does.
- The QuadTreeRenderer node views use the RTTI names, so their type
  descriptors bind exactly.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/gravity/SelectiveGravityModel.cpp \
  --source src/krusty2/soultree/SoultreeQuadTreeRenderer.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `23/23 strict exact`.

## Wave-9 BikeAI slice

`src/krusty2/vehicle/BikeAI.cpp` passes 23 strict cases:
- the filter-static and five empty `$E` pairs;
- the Math3D kVec3 `$E` set;
- the Vec3 normalize `0x40d120`;
- the landing prediction `0x40e370`;
- the length helper `0x413190`.

Extent: `0x40d070..0x416e1f` (strong inference). Bike.cpp ends at
`0x40d063`, and the BikeAI `__FILE__` xrefs lie inside `0x414370` and
`0x415640`. A second Math3D set at `0x417350` means BikeCamera code
(`0x416e20..0x417aff`) is a separate, unattested unit.

The near miss, steering direction `0x40e510`, is in
`samples/physics/bikeai`; it differs by an esi/edi swap and frame order.
The two KrustyBike methods `0x413200` and `0x414370` wait on KrustyBike.h's
promotion. The five other large functions are not attempted.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/vehicle/BikeAI.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `23/23 strict exact`.

## Wave-10 soultree and box-tree query slice

Two units are promoted and pass strict comparison.

**`src/krusty2/soultree/soultree.cpp`** (SoultreeObject,
`0x004fb2b0..0x004fefd8`) passes 56 cases, including the former 78.7%
partial `UpdateWorldMatrix`.
- `core/SoultreeObject.h` gives the real QuadTreeObject + GameObject bases
  only when `SOULTREE_OBJECT_WITH_BASES` is defined. Other physics code
  keeps the flat view.
- `Fn_004fbd10`'s seventh argument is an int.
- The helpers samples keep the near misses: LocalToWorldPoint,
  WorldToLocalDirection, WorldToLocalPoint, SetAxesPtr, SetMatrixIn,
  GetMatrixIn, SetAxesIn and RotateAboutPoint.

**`src/krusty2/bvh/BoundingBoxTreeQuery.cpp`** (an unattested unit,
`0x00424690..0x0042ad2f`) passes 33 cases:
- its 18 `$E` (the kVec3 set and five empty statics);
- the segment and sphere box tests;
- the tree-tree and tree-box queries and the three entry points;
- the matrix and point helpers.

Dot products need `z*z + (x*x + y*y)`. Near misses are in
`samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp`.

```bash
python tools/run_physics_samples.py --strict --root src/krusty2/soultree \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/run_physics_samples.py --strict \
  --source src/krusty2/bvh/BoundingBoxTreeQuery.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The second run reports `33/33 strict exact`. The soultree root reports one
required failure, SoulTreePhysics'
`0x5036f0`, which predates this slice.

## Code-generation limits behind the remaining partials

These were measured with VC6 SP3 `/O2` on the real targets and on small synthetic
sources. They describe compiler behaviour, not original source.

**x87 operand load order.** Cases include the Vehicle slot 34/35 cross product,
the inlined cross products in PoseRotation 0x4a7fd0, WorldToLocalDirection
0x4fd710 and Bike slot 76 0x406840.

- These partials differ only in which memory operand of a product is loaded with
  `fld` first.
- The written factor order has no effect: `a*b` and `b*a` emit identical code in
  every test.
- The order does move with statement order, parameter order, named temporaries,
  and whether the result goes to an out-pointer or a return value.
- About 400 such variants of the cross product were tried; none reproduces
  retail's order (best 90.35%).
- No flag fixes it. `/G3`, `/G5`, `/Ob1`, `/Ob2`, `/Oa`, `/Ow`, `/Ot`, `/Ox`, `/Oi-`,
  `/Gf` and `/Gy` change nothing. `/G6`, `/Op`, `/Og-`, `/Ob0`, `/Oy-`, `/Os`, `/O1`
  and `/Od` make it worse.
- Treat operand-order-only partials as low priority. The only remaining approach
  is a whole-function brute force over statement order, temporaries and
  destination form.
- The `fld st0 ... fpatan ... fstp st0` sequence in 0x48e280 (a dead duplicate,
  popped after the call) was not produced by any source form tried.

**Inline budget.** Motnctrl expands some helpers inline at some sites and calls their
out-of-line copies at others. Synthetic tests show (measured in detail in
[VC6_INLINE_BUDGET](VC6_INLINE_BUDGET.md)):

- VC6 gives each caller its own size budget for inline expansion, shared by
  nested expansions.
- Expansion degrades in source order. Late sites first lose the nested inline
  (for example the Vec3 constructor is called), then the outer one.
- For a helper of about three statements, the limit is about 17–18 expansions
  per caller. Larger helper bodies lower it. Callers with more than about 15
  other statements raise it to roughly their own statement count.
- Earlier uses in the TU, taking the helper's address, `__inline`, `/Ob1` vs
  `/Ob2` and the `/G`, `/O` variants make no difference.
- `#pragma inline_depth(1)` only stops nested expansion.

PoseRotation's retail pattern fits this model if DotProduct is a non-inline
function. All seven of its DotProduct sites are calls. Declaring it out of line
reproduces the first five operator* sites but still inlines the sixth. It also
inlines ClampFloat 0x4a8440 at both sites, which removes that matched
out-of-line copy. So the original source is unresolved.

One change was kept: UnitVector names the inverse length before scaling,
matching retail's stack temp. PoseRotation went from 26.9% to 55.7% and
InterpolatePose from 10.7% to 19.4%, with no other motion target changing.

## Preserved candidates

The tree builder, D3DIMSoultree motion control, Motnctrl loaders/playback, steering, projected shadow,
and the visibility traversal remain under `samples/physics/` until their
remaining byte/relocation evidence is complete. Shared layout headers remain
under `src/krusty2/` for both verified source and samples; a header alone does
not claim a reconstructed implementation. Retail-file attribution is retained
where supported, without promoting that evidence into exact-code status.

`expect: "masked"` requests a diagnostic regression check only. It does not
count as strict validation. `expect: "partial"` retains known code-generation
mismatches. The full `--strict` audit continues to fail on required targets
with unresolved relocations, including pre-existing physics candidates:

```bash
python tools/run_physics_samples.py --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/run_physics_samples.py --strict --vc6-root "$VC6_ROOT" \
  --exe "$MCM2_EXE" --json-out work/physics-strict.json
```

Missing bindings, compilation errors, and completed byte mismatches are distinct
statuses in the report. Never infer incorrect C++ merely from an unresolved
symbol, or claim a strict match from a masked result. No target length trimming
or relocation-byte fitting is used to make a candidate pass.

`make progress` regenerates the public source inventory and the separately
reviewed calibration snapshot. It does not compile or count this physics suite.
A complete linked game remains unverified.

## Per-unit vector initializers and collision callbacks

Strict exact with the units' existing sources (registration pending):
- Bike.cpp: its Math3D.h set `0x00407880..0x004079bb` (vectors `0x005778a8`, `0x005778b8`,
  `0x005778c8`, `0x00577898`, bound in `Bike.bindings.json`) and the two collision
  callbacks `0x00405cd0`/`0x00405d70` that `0x004079c0` stores at the collision object's
  +0x88/+0x8c (`0x004092c0`, `0x004092d0`).
- CollisionObject.cpp: its Math3D.h set `0x004356f0..0x0043582b` (vectors `0x005797a0..`)
  and the sphere query `0x004394f0` (ObjectPicker.cpp's caller `0x004b0a46`). The capsule
  query `0x00439600` is a near miss (516/532) in
  `samples/physics/collision/CollisionObjectNearMisses.cpp`.
- VisibilityQuadTree.cpp: the unit's 18 initializer functions (see its README).
- SoultreeQuadTreeRenderer.cpp: slot 23 `0x005048d0` (debug key 0x2d toggles the nodes).
- SteeringControl.cpp: promoted from samples (see `src/krusty2/motion/README.md`).
