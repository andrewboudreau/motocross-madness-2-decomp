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
- `src/krusty2/vehicle/Vehicle.cpp`: 57 of 77 targets at promotion; see the
  Vehicle/Bike round-out below for the current state (the shock pass `0x00529450` and
  the loader `0x00525e20` were added later).
- `src/krusty2/vehicle/Bike.cpp`: 31 of 45 targets at promotion (`0x0040a520` added
  later as a partial).
- `src/krusty2/soultree/SoulTreePhysics.cpp`: 37 of 43 targets at promotion; see the
  SoulTreePhysics round-out below for the current state.

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
- `samples/physics/motion/D3DIMSoultreeMotnctrl.cpp` passes 24 of 26
  targets. These are the D3DIMSoultreeCharacter methods and the four vector
  `$E` pairs, with `D3DIMSoultreeMotnctrl.bindings.json`.
  - Slot 2 `0x445fc0` is one byte off.
  - Slot 4 `0x446210` is exact once the field_0x3c enable is read inside
    each branch rather than into a local before the branch.
  - Slot 11 `0x445680` is at 39.6%.

D3DIMSoultreeMotnctrl.cpp stays in `samples/` because those two targets are still partial.
Of its bindings, the seven that `tools/propose_bindings.py` could not prove
were checked by hand. One of them is `_strupr` at `0x535d3d`; that CRT
identity is provisional.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/shadow/D3DIMSoultreeShadow.cpp \
  --source samples/physics/motion/D3DIMSoultreeMotnctrl.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `43/45 strict exact` with no required failures.

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

Six near misses and one exact COMDAT are in `samples/physics/bikeai`:
- the steering direction `0x40e510` differs by an esi/edi swap and frame order;
- the jump prediction `0x40d200` (4464 bytes, 4430/4464) differs only in the
  scheduling of two stores;
- the obstacle query `0x415640` (6112 bytes, 5987/6111) differs in the order of
  its one-use temporaries and two argument-push placements;
- the two-wheel look-ahead simulation `0x40eca0` (17585 bytes, extent
  `0x40eca0..0x413150`; `0x412700` is a mid-function address) is fully
  reconstructed: 392 masked code differences, frame 0x8b8 vs 0x93c;
- the Vec3 `operator-=` COMDAT `0x413160` is strict exact; VC6 emits it after
  `0x40eca0`, whose epilogue calls it;
- the KrustyBike AI physics step `0x413200` (4112 bytes, 16.1%): calls, branches
  and the 1120 instructions match, the 0x74 frame too, but slot 11's out-vectors sit
  at other offsets and the angular-velocity cross products load their operands in
  another order;
- the KrustyBike racing-line builder `0x414370` (4802 bytes, 8.5%): block order and
  calls match; frame 0x74 vs 0x78 and the kind 1/5 gate-distance test is placed
  after the single-point and no-track paths instead of before them.

`0x40eca0` exhausts VC6's per-function inline budget. Its first part expands every
Vec3 operator; from `0x410d00` the operators are expanded but call the
out-of-line constructor `0x404e60`; the epilogue calls whole operators
(`0x428090`, `0x421cb0`, `0x421d00`, `0x5015b0`, `0x413160`). Natural Math3D
operators reproduce this pattern call for call (one site differs). The budget is
consumed per inline expansion, so source forms before a switch-over point move
it. `0x413200` and `0x414370` are KrustyBike members placed in BikeAI.cpp's code
(the first reads and writes the file-static filter `0x577ac0`, the second holds the
unit's `__FILE__` xrefs); they use the promoted `src/krusty2/vehicle/KrustyBike.h`.

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/vehicle/BikeAI.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/run_physics_samples.py --strict \
  --source samples/physics/bikeai/BikeAINearMisses.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The first run reports `23/23 strict exact`, the second `1/7 strict exact` (the
COMDAT; the six near misses are registered as partial).

## Wave-10 soultree and box-tree query slice

Two units are promoted and pass strict comparison.

**`src/krusty2/soultree/soultree.cpp`** (SoultreeObject,
`0x004fb2b0..0x004fefd8`) passes 56 cases, including the former 78.7%
partial `UpdateWorldMatrix`.
- `core/SoultreeObject.h` gives the real QuadTreeObject + GameObject bases
  only when `SOULTREE_OBJECT_WITH_BASES` is defined. Other physics code
  keeps the flat view.
- `SetAxes`'s seventh argument is an int.
- The helpers samples keep the near misses: LocalToWorldPoint,
  WorldToLocalDirection, WorldToLocalPoint, SetMatrixIn, GetMatrixIn,
  SetAxesIn and RotateAboutPoint. SetAxesPtr `0x004fbd70` is exact in
  soultree.cpp once its cross products parenthesise the first product
  (VC6_OPERAND_ORDER.md section 3).

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

## Vehicle and Bike round-out

Strict exact under `vc6_o2_ml` with `Vehicle.bindings.json` / `Bike.bindings.json`:

- Vehicle.cpp, new: `~Vehicle` `0x00526380` (body entered through the vbase-adjusted
  deleting destructor; frees the smoothers, the engine/steer states through their
  out-of-line destructors `0x00464e90`/`0x00504c50`, the three arrays, then detaches
  the input map's value source via its slots 15/16), the GameObject slot 10 override
  `0x0052a830` (vtordisp thunk `0x0052b690`; calls `GameObject::GameObjectVirtualSlot10`
  directly, not SoultreePhysicsCharacter's) and the wheel placement pass
  `PlaceWheels` `0x00528eb0` (shock retract 0.25/0.75 of the last step via
  `0x004fa310`/`0x004fab60`, probe `0x00514550`, `ClearForces` `0x004f9c70` called
  non-virtually; the flag `unflagged = 1` must be set after the `wheelCount == 0` return).
- Vehicle.cpp, former partials: slot 58 `0x005289c0` (`top = wheelList[i]; second = top;`
  instead of `second = top = w`), `GetAverageGroundNormal` `0x00528400`, slot 54 `0x00528530`,
  slot 75 `0x0052b790` and `AccumulateWheelContacts` `0x00529c20` (plain `*up += ...;
  scratchVector2 = slot76(...); scratchVector = WorldToLocalDirection(...)`).
- Bike.cpp: slot 76 `0x00406840`; slots 1, 5, 39, 41, 56, 57, 59, 73 and 75 were already
  exact with the promoted source and only carried stale `partial` notes.
- Bike.cpp slots 54 `0x0040c8b0` and 55 `0x0040ce60`: `BikeNormalized` receives an operator
  temporary (`(front + rear) * 0.5f`, `front - rear`); a local Vec3 built component by
  component left `x` scalar-first.
- Vehicle.cpp impact handlers, slots 18 `0x00529dc0`, 19 `0x00529fe0` and 20 `0x0052a290`:
  each scan posts and returns from inside its loop; slots 18/19 scan with guarded do-while
  loops (unrotated back edge), slot 20 with `for` loops. Slot 19/20's sign factor is
  `turnAngle >= 0 ? -1 : 1` and the scrape clamps go through a by-value min helper.

x87 operand order facts measured on these targets (they add to the list below):

- A scalar multiplied into a vector is loaded first (`fld st(0); fmul [v.x]`) when it
  is a local of the inlined helper that computes it (`VehMean`: `k = 1.0f / n` inside
  the helper) or when the vector is the helper's by-value parameter (`VehNormalizedV(Vec3
  v)`); a caller-side `k = 1.0f / n; sum * k` or a `const Vec3&` parameter loads the
  vector component first.  Squared lengths need `(x*x + y*y) + z*z`, or
  `z*z + (x*x + y*y)` through a reference helper (`VehLenSqZ`, slot 75).
- Bike slot 76 scales v.x by the freshly computed factor and v.y/v.z by its stored
  copy, then pops it: only `v.x *= (s = expr); v.y *= s; v.z *= s;` reproduces that
  (a named `s` first, `v *= s`, `Vec3::operator*=` or a scaling helper give 389/395).
- VC6 always forms the destination address of a `Vec3` struct assignment in a register
  (`lea`/`add`); retail `Method_00527A20` `0x00527a20` stores through `[wheel+disp]`
  directly, which memberwise helpers reproduce for member copies but not for the
  `(0,0,0)` temporary (VC6 folds it).  That function stays `partial` (349/866, first
  0x120 bytes exact).  Vehicle slot 55 `0x0052b6d0` reloads `out` between its two
  copies (VC6 keeps it in a register: 28/59).  Bike slot 72 `0x00405db0` is exact once
  the in-place normalisation of `*out` goes through an inline helper, which resets the
  operand tracking for the following cross product.

Large functions of the two units (all registered in `src/krusty2/vehicle/targets.json`):

- `Vehicle::Method_00529450` `0x00529450` (1415 bytes) is strict exact: the per-wheel shock
  pass projects the shock's spring force (+0x58) on the ground normal and a pending
  displacement impulse (+0x94 * +0x74) on the Y axis, scales both by `leanCos`, and folds
  them into the wheel's applied share, `*up` and (as a body-space x moment) `*zero`.  Its
  inline budget is reproduced with per-site call views (`VehVec3Call` for the constructor
  `0x00404e60`, `VehVec3AddAssign` for `+=` `0x00428060`, `Vec3DotCall`, `Vec3ScaleCall`,
  `CrossProductCall`, and the equality `0x005299e0` called at three of its four sites while
  `VehVec3EqualInline` expands the first).  The secondary-shock `*up += scratchVector2`
  loads `scratchVector2.x` first only through a reference (`Vec3& impulse = scratchVector2`).
- `Vehicle::LoadVehicle` `0x00525e20` (1373 bytes, `ret 0x94`; `allfn` lists 895 because a
  jump target splits it) is not a vtable entry: Bike's loader `0x004079c0` calls it directly
  after the three by-value vectors, and the Vehicle vtable's slot 40 is the inherited
  `0x00503de0`.  It allocates the wheel and ticker arrays (`new(__FILE__, 0x151/0x15c/0x167)`),
  the two 0x14-byte smoothers (inline constructor: 0, time constant, FLT_MAX, -FLT_MAX, 1),
  the 0x1e4-byte gearbox `0x004d2940` with the caller's tables or zeros (EH states 0/1),
  runs `SoultreePhysicsCharacter` slot 40 with `(0.02f, 100, 0.001f, 0.1f, 3)` and the track
  byte, builds the spark emitter `0x004b9830` (state 2, child via its slot 27 and
  `GameObject::Method_0x00469190`) and the steering control `0x00504b60` (state 3, axis
  `a8 * -1.0f`), then installs `VehicleHit`/`VehicleHitBy`.  Exact: the gearbox's name
  argument is the second parameter (`[esp+0x48]` at `0x005260d3` is the a2 argument slot
  after seven pushes, not the `new` temporary); read as an uninitialised local instead, VC6
  homed it in the dead `gearArg` slot (`[esp+0xc0]`), which cost 3 bytes.
  `maxLeanAngle`/`maxLeanRate` are
  `40.0f * 0.01745329f` / `120.0f * 0.01745329f` (the `(float)(deg * pi / 180)` forms are
  one ulp off).
- `Bike::Method_0x0040a520` `0x0040a520` (2272 bytes) is a partial (2265 bytes, same calls
  and flow): a steering torque from a quarter of `weightForce`, the front wheel's contact
  offset x `w_0x120`, the active `+0x5f8`/`+0x5fc` contacts (offset x axis scaled by the
  negative projection) or `w_0x274 * w_0x20c` while airborne, normalised and projected on
  the wheel frame's axis into the integrator `field_0x61c`; the integrator's length, signed
  by that axis, is the steer angle unless 0.785 rad is reached.  The by-value views
  (`BikeVec3Sub`, `BikeVec3Scale`, `BikeVec3Dot`) give retail's temp-then-copy shape; what
  remains is the local slot layout (retail: result 0xc, len 0x10, cross temp 0x14, offset
  0x20, torque 0x2c, base 0x38, temps 0x44, unmoved by declaration order), `frontWheel` in
  eax and the constant 2 of the state stores kept in edi.
- `Method_00527A20` `0x00527a20` stays at 349/866: references to the wheel or to the
  destination vector do not change the struct-assignment addressing.
- `Bike::LoadBike` `0x004079c0` (6745 bytes, `ret 0xa0` = 40 argument dwords, 8 EH states,
  0x2cc-byte frame) is written out as a partial (strict 2078/6718, instruction ratio 0.88,
  every call, EH state and x87 sequence in retail order).  It runs `LoadVehicle` with
  `(a1, engineName, a3, desc, info, position, forward, up, a9, map, a10, a11 (230 when <= 0),
  0.9f, 0.698f, device, 2, 3, 3, 1, 1, defaultEngine, tables, 0.18f, axes, track, a21)`
  (the gear argument is a float), copies the 16-byte name, sets `loadWeight = 165`,
  `field_0x724 = 32`, `sideLieThreshold = cos(5 * 0.01745329)`, the pose bounds 0.85/0.55/
  0.8/1.8 and `field_0x28 = 32.2 / loadWeight`, builds the rider (`new(__FILE__, 0x583)
  D3DIMSoultreeCharacter(statusFlags & 1)`, loaded through its slot 11 and dropped when
  `GameObject::Method_0x00469190` answers 0, which is why that method now returns int), the
  handlebar part `BikeA60C("Handlebars", modelNode, steerState)` (line 0x660) and the two
  wheels: per wheel a local name table (`"Inline"`/`"InlineRear"`), a `{name[20], kind}`
  table of four kinds matched with the library `strcmp` (`#pragma function(strcmp)`), a
  rotating shock (`0x004fa700`, line 0x683, `AddLateTicker`) for the rear wheel or an inline
  shock (`0x004f9ee0`, line 0x696, `AddEarlyTicker`) for the front, the tire (`0x00512f10`,
  line 0x6b8, registered through its GameObject subobject at +0xc and `AddWheel`), the wheel
  position in model space, and the shock arm length (`BikeDistance`, x and y squares in
  named temporaries).  The front/rear wheels are the largest/smallest model z; the
  `"NullFrame"` node becomes `centerNode`; the dust/dirt/steam emitters (lines 0x70f..0x711)
  are attached through slot 37 (types 1, 2 and 4 at (0, 2.5, -2.75)); three contact points
  (`AddCollisionPoint` 0x0043a330 at (+-1.305, 0.225, -0.228) on the handlebar node and
  (0, 3.345, -3.206) on the model) feed `field_0x5f8/0x5fc/0x600`; the wheel base and the
  rear/front load shares, the box inertia of the bike (length from the model, width and
  height from the rider), of the rider and of the steering integrator gains, the start
  orientation, 29 pose-handle lookups, the collision callbacks and the four smoothers end
  it.  What still differs: the register spill homes (retail keeps the wheel-position array,
  the loop index and the front z at `0x10/0x14/0x28`), the matched wheel kind (esi in
  retail, spilled here), and the store scheduling of the local string tables and the
  Vec3 constants.
- `0x00526e80` is slot 38's jump table, `0x0040ca40`/`0x0052b690` are vtordisp thunks
  (`0x0040ca40`, `??_EBike@@$4PPPPPPPM@A@AEPAXI@Z`, is strict exact in Bike.cpp's targets);
  `0x0040ae00` is Bike.cpp's COMDAT copy of `Vec3::operator*=` (the `BikeVec3ScaleAssign`
  view calls it).

```bash
python tools/run_physics_samples.py --strict --root src/krusty2/vehicle \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

This run reports `139/164 strict exact` with no required failures (`0x409420` is
strict exact since the BikeA604 constructor call is bound to `0x0052ff90`).

## SoulTreePhysics round-out

Strict exact under `vc6_o2_ml` with `SoulTreePhysics.bindings.json`:

- The GameObject slot 10 override `0x005036f0` was `masked` only because the contact
  refresh it calls had no binding; `SoultreeRefreshContacts` is `0x0043ad80`
  (collision/CollisionContactUpdate.cpp) and the target is strict exact.
- SoultreePhysicsObject (`soultree/SoultreePhysicsObject.h`, formerly a collision sample
  header): ctor `0x005037c0` (`GameObject(1)`, `SoultreePhysicsBaseObject(flags)`,
  `D3DIMSoultreeObject(flags)`, then `sceneNode = this` converted to the D3DIM subobject),
  dtor core `0x005038d0` (empty body), GameObject slot 10 `0x00503c50`
  (`SoultreePhysicsBaseObject::GameObjectVirtualSlot10(dt)` then
  `D3DIMSoultreeObject::GameObjectVirtualSlot10(dt)`, 0x00443490), and the compiler-emitted
  deleting destructor `0x00503890` and vbase-vtable thunks `0x00504260`/`0x00504270`.
  The class has two GameObject bases (the virtual one and D3DIMSoultreeObject's plain one at
  +0x228); VC6 compiles the destructor and the slot 10 override against the plain one, which
  is why those bodies take `this` at +0x228 and the vbase thunks subtract
  `0x2d0 = 0x4f8 - 0x228`.  Declaring D3DIMSoultreeObject with QuadTreeObject and GameObject
  as direct bases (its own dtor 0x0043f2b0 writes the primary vptr through `edi-0xc`)
  reproduces all of it.
- The slot 40 loaders `0x00503970` (SoultreePhysicsObject, `ret 0x70`) and `0x00503de0`
  (SoultreePhysicsCharacter, `ret 0x6c`), moved from
  `samples/physics/collision/SoultreePhysicsCharacter.cpp`.  Both allocate the 0x134-byte
  `.col` file object with `new(SP_FILE, 0x84e/0x8cd)`, and retail pushes this TU's
  `__FILE__` string `0x00574320` at `0x00503b13`/`0x00503f87`, so they belong here and
  bind strictly through `__FILE__:soultreephysics.cpp`.

Still partial in SoulTreePhysics.cpp:

- The sub-step loop `RunSteps` `0x00502f60` (1921 bytes) is now written out:
  1366/1942 bytes, every call, branch and x87 sequence in retail order.  What was learned:
  `prevSpeed = linearSpeed;` followed by a test of `prevSpeed` (not `linearSpeed`) gives
  retail's single `fld`/`fst`/`fcom`; the loop is `if (steps <= 0) return; do { ... } while
  (steps > 0);` (the rematerialised `xor ebx,ebx` sits on the back edge); the leftover
  remainder block reads `stepRemainder` once into a local (`stepTime = t; invStepTime =
  1.0f / t;`).  The residue is stack-slot packing: retail packs the integration
  temporary into the slot of the respawn path's first vector and keeps the merge outputs
  `o3`/`o1` in the slots of its other two, with `impulse` below `o2`; VC6 here packs the
  respawn vectors into the merge outputs instead (frame size and every other slot agree).
  Declaration order and block scoping of the locals do not move the packing; sharing the
  vectors between the two paths or hoisting them to function scope makes it worse.
- Slot 4 `0x005013d0`: retail loads the a3 component first in all six products of the
  first cross product; VC6 loads a4 first in the term it evaluates second (87.5%).
- SteeringControl::SetAxisFromPoints `0x00504d30` (samples/physics/motion): only the
  scaling differs.  Retail keeps the inverse length duplicated (`fld st(0); fmul st(3)` for
  x, `fld [d.y]; fmul st(1)` for y, `fxch; fmul` for z) and pops `d.x` at the end; the
  `Vec3(d.x * inv, ...)`, `d * inv`, `inv * d` and `d * (1.0f / len)` forms all consume
  `d.x` first (`fxch st(2); fmul st(2)`).

```bash
python tools/run_physics_samples.py --strict --source src/krusty2/soultree/SoulTreePhysics.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

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
- The `fld st0; fxch st(2); fxch st(1); fxch st(2); fpatan; fxch st(1); fstp st(0)`
  sequence of 0x48e280 (a dead duplicate of the x argument) comes from
  `(float)(atan2(d.x, d.z)) - savedYaw` with the call parenthesised inside the
  cast: VC6 then pops the duplicate before the yaw `fsub`, as retail does, and
  0x48e280 is exact. `(float)(atan2(...) - yaw)` pops it after the `fsub`;
  `(float)atan2(...)` gives a plain `fpatan`.

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

The near misses of the tree builder, D3DIMSoultree motion control, Motnctrl (the .VUE loader and pose
helpers), steering (SetAxisFromPoints), projected shadow and the visibility traversal remain under
`samples/physics/` until their remaining byte/relocation evidence is
complete. Shared layout headers remain under `src/krusty2/` for both verified
source and samples; a header alone does not claim a reconstructed
implementation. Retail-file attribution is retained
where supported, without promoting that evidence into exact-code status.

`expect: "masked"` requests a diagnostic regression check only. It does not
count as strict validation. `expect: "partial"` retains known code-generation
mismatches. Full-root status of the `--strict` audit:

- `--root src/krusty2`: 682/738 strict exact, 0 required failures.
- `--root samples/physics`: 311/403 strict exact, 0 required failures.

Every other required target's relocations are bound in a `*.bindings.json` next to its
source. The bindings came from `tools/propose_bindings.py` on masked-exact targets; the
few it could not prove were checked by hand against retail (a call target that is a strict
calibration target or a decoded function start, or a data address documented in the
headers and consistent at every relocation site).

```bash
python tools/run_physics_samples.py --strict --root src/krusty2 --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
python tools/run_physics_samples.py --strict --root samples/physics --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The default (diagnostic) run and a full JSON report:

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
  Also exact: the .col readers `0x00439ed0` (hull) and `0x0043a050` (model), the
  polyline-tree debug draws `0x004341b0` / `0x00434340` (segment leaves), the broad-phase
  query `0x00438e70` (quadtree, vegetation and game-object-tree paths; the three hit
  blocks are one inline `ReportHit`), and `0x00432800` once the CollisionFileStream
  constructor/destructor are bound. Slot 14 `0x00434540` is 1054/1068 (size and frame
  exact; only the capsule end-point transforms differ in x87 operand order).
- BoundingBoxTreeQuery.cpp: SegmentTreeQuery `0x00429e90`, LeafNodeQuery `0x004269e0` and
  LeafPairQuery `0x004275f0` (see `src/krusty2/bvh/README.md`).
- VisibilityQuadTree.cpp: the unit's 18 initializer functions (see its README) and the
  five VisibilityClipper helpers 0x0052f0a0, 0x0052f140, 0x0052f340, 0x0052f4d0,
  0x0052fac0, which are thiscall methods of the object at 0x00575a98.
- SoultreeQuadTreeRenderer.cpp: slot 23 `0x005048d0` (debug key 0x2d toggles the nodes).
- SteeringControl.cpp: promoted from samples (see `src/krusty2/motion/README.md`).

## KrustyBike and Tire with bindings

KrustyBike.cpp is promoted: its 70 exact targets are in
`src/krusty2/vehicle/KrustyBike.cpp` (bindings `KrustyBike.bindings.json`, entries
in `src/krusty2/vehicle/targets.json`), its 11 partial targets in
`samples/physics/krustybike/KrustyBikeNearMisses.cpp`, which includes the canonical
file. `samples/physics/tire/Tire.bindings.json` resolves every relocation of the
Tire sample's exact targets:

- KrustyBike.cpp: the 56 exact targets of the first slice, plus the `.CRT$XCU` 155-158
  set (`0x00491190..0x004912cb`, vectors `0x0067c348`, `0x0067c358`,
  `0x0067c368`, `0x0067c338`; the constructor and `0x00492670` read the zero
  vector), the two vtordisp thunks `0x00497c30`/`0x00497ca0` and the network
  message 13 decoder `0x004933e0`. The trick end `0x00495ff0` is a near miss
  (99.17%, 895/895): its stack slots match once the float bonus is left as the
  CSE temporary of `(float)points` instead of a named local; only the order of
  the +0x153f flag load and the fsubr after the fmod call differs (retail loads
  the flag first; a local read before the call anchors it before the call, one
  read after the call is forward-substituted and scheduled after the fsubr).
  Slot 97's six clip names come from a
  `{id, name[32]}` table at `0x0056cb88` (the two "BackOver" entries are
  separate records). The constructor `0x0048fa60` (`GameObject(1), Bike(flags)`,
  ret 8) is a near miss (86.36%, 532/532): VC6 hoists two stores into the load
  delay of the +0x1540 vector copy and picks them at a fixed distance from the
  end of the block, counted in statements; retail's pick (+0x11b8/+0x604) is
  reproduced only by dropping the last eight stores, so retail has eight fewer
  scheduling units after the copy for a reason not found (statement order,
  init-list members, copy forms, duplicate stores and inline helpers do not move
  it). `0x0048d780` uses an inline-asm rounding helper and stays excluded.
  Exact as well: GameObject slot 10 `0x004977a0`
  (the per-frame update reached through the virtual base: update path by
  network state and mode, the mode 4 player re-target, the end of start-up
  ghosting, and the "KrustyBike" debug overlay page with the static
  "TimeTo60" stopwatch `0x0056d0a0`/`0x0067c3b0`/`0x0067c3b4`; the overlay
  line index needs the `int line = lineCount++` form) and the message 1
  builder `0x00492670` (0x58-byte `KbBikeMessage` with pose nibbles and flag
  bitfields, sent through Net or the recorder, copied into a 0x6c-byte
  `KbBikeState`; the stores of +0x11c0, +0x74c, +0x750, +0x770 need that
  order). The two `KbBikeState`s are KrustyBike+0x1390 (recorded) and
  +0x1560 (network); their +0x68 is the float timer the constructor sets to
  `0x7effffff`. The message 13 encoder `0x00492ad0` is a near miss: byte
  deltas with carried errors (`diff = a - b; d = diff + error` gives retail's
  x87 order, `coarse:1, count:7` its time byte) match, but VC6 packs `diff`
  over the int temporary (frame 0x30, retail 0x34) and keeps the full
  message 1 call after the record-interval test in place instead of
  cross-jumping it to the last call site. Not reconstructed: `0x0048e3e0`
  (inline-asm `fistp` rounding). Slot 102 `0x0048eea0` (2920 bytes, the head-turn
  update) is a partial: calls, frame and constants match; branch cross-jumping,
  the trick-chain switch tails and one spilled clamp differ. The loader `0x0048fc80` (5.4 KB, 31 argument
  dwords) is exact and the network update `0x00493660` (8.5 KB) is a partial.
- TerrainShadow.cpp (`samples/physics/shadow`): slot 14 `0x0050a1a0` is exact
  (D3DIMSoultreeShadow slot 14's draw of the +0x34 vertices without the world
  matrix). Slot 30 `0x00509aa0` rounds with inline-asm `fistp`; slot 27
  `0x00508bc0` (3087 bytes) calls the Vector3 helpers out of line (inline
  budget, see VC6_INLINE_BUDGET.md) and is not reconstructed.
- Tire.cpp: the 5 existing exact targets, plus the `.CRT$XCU` 319-322 set
  (`0x00515740..0x0051587b`; the former `g_TireZeroVec3`/`g_TireVec3_68a3c0`/
  `g_TireVec3_68a3f0` are its zero, z and y vectors), CollisionPoint's inline
  destructor `0x00515b40`, the roll helpers `0x00513560`/`0x005135b0`, the
  attachment pose `0x00515660` and the roll update `0x005143d0`. Near misses in
  the same file: `0x00513f90` (470/479, stack slots), `0x00514170` and
  `0x00515c90` (x87 operand order).

Check:

```bash
python tools/run_physics_samples.py --strict \
  --source src/krusty2/vehicle/KrustyBike.cpp \
  --source samples/physics/krustybike/KrustyBikeNearMisses.cpp \
  --source samples/physics/tire/Tire.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The KrustyBike sources give 71 of 82 strict exact (the 11 partials are the near
misses listed above and in docs/NEAR_MISS_INDEX.md).
