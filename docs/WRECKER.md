# wrecker.cpp (Wrecker)

`src/reconstructed/Wrecker.h` / `Wrecker.cpp` / `Wrecker.bindings.json`
reconstruct part of `D:\aardvark\VC\krusty2\wrecker.cpp`: 30 functions are
strict exact under `vc6_o2_mt` (VC6 SP3 `/O2 /GR /GX /MT`), with every
relocation resolved by the bindings. Two near misses are parked in
`samples/race/WreckerNearMisses.cpp`.

## Evidence

- Confirmed: the `__FILE__` literal `D:\aardvark\VC\krusty2\wrecker.cpp` is at
  `0x00575ab8`. Its code references are `0x005300f8`..`0x0053015c` (the
  destructor, lines 0x3e..0x47), `0x00530227`/`0x00530272`/`0x005302df`
  (`new` at lines 0x5b, 0x5f, 0x68), `0x005305b9`..`0x00530690` (the four
  appends, lines 0x9d..0xb4), `0x00532938`/`0x0053297c` (lines 0x308/0x30b),
  and three EH funclets at `0x0054f183`..`0x0054f1ab`.
- Confirmed RTTI: `Wrecker : GraphicsTest : GameObject : BaseObject`, all
  single inheritance at offset 0 (BCDs: mdisp 0, pdisp -1). The primary
  vtable is `0x00558e24` (COL `0x0055fb70`, 27 slots). Wrecker overrides
  slot 0 (`0x005300a0`), slot 10 (`0x005306e0`), slot 14 (`0x004de400`) and
  slot 23 (`0x00532890`). The other slots are inherited from GraphicsTest and
  GameObject (`analysis/class_dossiers.json`). The constructor and the
  destructor write the vptr at `0x0052ffc6` and `0x005300dd`.
- Slot 10 (`0x005306e0`) calls slot 11 of the gravity model, the rigid body
  and the collision model's GraphicsTest subobject (`lea ecx,[eax+0xc]`
  without a null check) with the sub-step length: the per-step update
  `GameObjectVirtualSlot11(float dt)` of `src/krusty2/core/GameObject.h`,
  declared here through the `UnknownWreckerSteppable` view because
  `src/reconstructed/GameObject.h` gives the slot an int.
- Confirmed: Bike slot 97 (`0x00409420`) allocates 0x53c bytes and calls the
  constructor `0x0052ff90` with 1, then `0x00530190` with "rider.col"
  (`src/krusty2/vehicle/Bike.cpp`, its `BikeA604` view of this class).
- Strong inference (TU extent): the linker placed the objects in name order
  and wrecker.cpp is the last `__FILE__` name. `.CRT$XCU` lists the
  previous TU's initializers (`0x0052fdc0`, `0x0052fe10`, `0x0052fe60`,
  `0x0052feb0`, then `0x0052f080`) and then wrecker.cpp's four
  (`0x00532f00`..`0x00532ff0`, entries `0x00566574`..`0x00566580`). Import
  thunks start at `0x00533040`. Wrecker's code is therefore
  `0x0052ff90..0x0053303b`. `0x0052ff00` and `0x0052ff20` (SystemParametersInfo
  0x6e/0x6f around a PeekMessage pump, globals `0x0068ace4`/`0x0068ace8`) lie
  between the previous TU's vector initializers and the Wrecker constructor.
  Their globals directly follow the previous TU's `.bss`, so their owner is
  open. They are not claimed here; both are strict exact in their own
  provisional unit, `src/reconstructed/ImeUiState.cpp`.
- Every function start in `0x0052ff90..0x0053303b` is registered except the
  two near misses below; `0x00530000`, `0x005304c0` and `0x00530880` are
  branch targets inside the constructor, `0x005301c0` and slot 10. The
  inline-assembly rounding function `0x0052f570` (ebp frame, `fld; mov eax,
  [ebp+N]; fistp [eax]` x6), listed under wrecker.cpp in
  docs/REBUILD_GAPS.md as the nearest following `__FILE__`, lies before the
  previous unit's initializers and so belongs to that unit, not here.
- Collaborators, all confirmed from constructor vptr writes: `+0x38` is a
  ConstraintMethodCollisionModel (`0x0043b8d0`, `0x10c` bytes, GameObject
  table at `+0xc`). `+0x3c` is a SelectiveGravityModel (`0x004f9760`, `0x3c`
  bytes). `+0x40` is a PhysicsRigidBody (`0x004cc120`, `0x2a8` bytes).
  `+0x11c` receives Bike's particle manager (`0x004baa50` is
  `ParticleManager::AddParticle` in `src/krusty2/effects`). The scene-node
  calls `0x004fc690`..`0x004fe850` are the SoultreeObject helpers documented
  in `src/krusty2/core/SoultreeObject.h`. They are declared here as
  `UnknownWreckerSkeleton` with address names.

## Layout (Wrecker, 0x53c bytes)

| Offset | Type | Evidence |
|---|---|---|
| `+0x00..+0x33` | GraphicsTest | vptr, GameObject `+0x04..+0x2b`, `+0x2c`/`+0x30` |
| `+0x34` | rider character | set by `0x005301c0`; `+0x1a0` is its skeleton |
| `+0x38`/`+0x3c`/`+0x40` | collision model, gravity model, rigid body | `new` at lines 0x68/0x5b/0x5f |
| `+0x44` | wreck state 0..3 | `0x00532220` sets 1, `0x00532310`/`0x00532490` set 2 |
| `+0x48`/`+0x4c`, `+0x50`/`+0x54`, `+0x58`/`+0x5c`, `+0x60`/`+0x64` | growable arrays and counts | the four appends; the destructor frees three |
| `+0x68` | `rigid->+0x178 * 1/12` | box inertia factor used by `0x00532150` |
| `+0x6c` | 4x4 frame (rows as Vector3) | old pose of `+0xb8` |
| `+0xbc`/`+0xc8` | linear and angular step | `0x00532580`, slot 10 |
| `+0xdc`, `+0xe8`/`+0xf4`/`+0x100` | vectors; box centre, half extents, frame | `0x005328b0`, `0x005329e0` |
| `+0x104`/`+0x108` | per-probe position and flag arrays | `0x00532900` grows both |
| `+0x10c`, `+0x118` | push impulse, its scale (5.0) | `0x005329e0` |
| `+0x11c`/`+0x120`/`+0x124` | particle manager, particle frames 0..12 (`0x00531da0`) and 0x1d..0x2c (`0x00531740`) | `0x00531da0`, `0x00531740` |
| `+0x128` | `float[256]` of `rand() * (1/32768)` | constructor |
| `+0x528` | index into `+0x128`, wraps at 256 | `0x00531740` |
| `+0x52c..+0x538` | flags | constructor, slot 10 |

## Functions

| VA | Size | Function | Source shape |
|---|---:|---|---|
| `0x0052ff90` | 261 | constructor | integer members first, then the two `Vector3` members (copy-assigned temporaries); `RANDOM_UNIT()` macro |
| `0x005300a0` | 30 | scalar deleting destructor | compiler generated |
| `0x005300c0` | 195 | destructor | `DebugFree(p, __FILE__, line)` per array |
| `0x00530190` | 36 | setup | qualified `GameObject::UnknownVirtualSlot8`, returns `this` |
| `0x005301c0` | 995 | builds the physics objects | `boxCenter`, `boxHalfExtents`, then `boxFrame = 0` |
| `0x005305b0`, `0x005305f0`, `0x00530630`, `0x00530680` | 60-82 | appends | `DebugRealloc(p, n * 4 + 4, ...)`; `0x00530630` returns `count - 1` |
| `0x00531da0` | 634 | probe trail particles | `step = delta / (float)count` through an inline `operator/` returning `v * (1.0f / s)`; the squared length `(y*y + x*x) + z*z` |
| `0x00532020` | 293 | probe contact pass | index the probe array in every expression (a probe pointer local reorders the loads) |
| `0x00532150` | 193 | box inertia | `y*y + z*z`, `x*x + z*z`, `x*x + y*y` |
| `0x00532220`, `0x00532310`, `0x00532490` | 230-372 | wreck state changes | `RANDOM_UNIT() * count` |
| `0x005327c0` | 193 | ends the wreck | ignore-list loop, then resets |
| `0x004de400` | 5 | slot 14 | `return GameObject::UnknownVirtualSlot14();` (folded copy) |
| `0x00532890` | 18 | slot 23 | `return GameObject::UnknownVirtualSlot23(...);` (folded with VCRDlg slot 23) |
| `0x005328b0` | 73 | stores the box | three by-value arguments |
| `0x00532900` | 210 | adds a probe | two `DebugRealloc` growths |
| `0x00532f00`..`0x0053303b` | 4 x (5 + 60) | per-TU vectors | `static const Vector3 k = Vector3(...)` |

## Source details retail needs

- The random factor is a macro. As an inline function, VC6 moves the
  constant `1/32768` behind the caller's multiply (and once even negates it).
  Retail multiplies `rand()` by the constant first.
- `0x00531da0` divides the probe delta by the particle count through an
  inline `operator/` that multiplies by `1.0f / s`. Inlined, it keeps the
  reciprocal on the stack for the first component (`fld st(0); fmul
  [delta.x]`); the same product written in the body (`delta * (1.0f /
  count)`, a `scale` local, free or member `operator*`, per-component
  constructors) loads `delta.x` first.
- In the constructor the integer members are zeroed before the two vector
  members. Assigning `hitCenter` first makes VC6 build the temporary with
  immediates instead of the zero register.
- `0x005301c0` stores `boxCenter`, then `boxHalfExtents`, then `boxFrame`.
  The other order makes VC6 address `boxHalfExtents` through `ebx`.
- `0x00532580` (the linear and angular step) needs two references: `target`
  for the new forward row in the equality tests (its components become newer
  leaves than the copied old forward, so each cross product loads them first)
  and `up` for the old up row, which gives the schedule of the step's last
  store and the forward copy around the up-row loads.
- Slot 10 (`0x005306e0`, the per-frame update) comes from the rider
  carry-over written with inline helpers: the rigid inverse
  (`WreckerInvertRigid`, its `t` sharing a slot with the axis rebuild's
  `forward`), two products with both operands by value (MatrixMultiply's
  text; inlined, VC6 copies only the member pose in the first and the output
  operand in the second), the axis rebuild `WreckerOrthonormalize` (up =
  forward x right), the old position copied through `WreckerCopy` from a
  pointer to `lastPose` and the subtract called by value. The last term
  pair (`relative._23` adds `a._23 * b._22` before `a._33 * b._23`) needs the
  named view pointer in `WreckerStep`: VC6 orders those equal terms by the
  count of referenced local symbols created before the product, modulo 8
  (two to seven more than the cast written inline give retail's order).
- `0x005301c0` passes the literal "Fall02" (`0x0056d418`) to `0x004a8b10`.
- `GameObject.h` gains `friend class Wrecker`: Wrecker clears and sets
  bit 0 of its rigid body's `+0x25` byte (bitfield code: `and cl, 0xfe` and
  `or cl, bl`).

## Near misses (`samples/race/WreckerNearMisses.cpp`)

| VA | Size | Score | Remaining difference |
|---|---:|---|---|
| `0x005329e0` | 1312 | 763/1312 | frame 0x94 (retail's) with the scaled temporary passed straight to the out-of-line scale `0x005015b0`; the clip sums written `out = out + (extent - p)` (with `+=` the probe-array address is formed base first, 240/1312) and the mean through the unit's inline `operator/` give retail's instructions except one block: retail keeps the z clip's two `outPrevious` updates separate. The slot order differs (retail: `push`, the y and x extent copies, `p`, the z extent copy, `previousPush`, `sum`) |
| `0x00531740` | 1624 | 1606/1624 | contact particle spray; frame 0xc4 and slot order match with the two normalisations written as nested expressions (`WreckerNormalizeFirst(velocity) * (speed + 2.0f)`, `WreckerNormalizeTail(WreckerCross(...))`) and the per-particle gravity drift as the nested out-of-line scale. Left: the argument push of two constructor calls in the loop comes one load earlier (before the particle-pointer load and before `along.y` is copied), which changes two registers; not sensitive to the declaration count |

## Naming

Field names of the collaborator views follow the canonical krusty2 layouts:
`CollisionObject.h` and `ConstraintMethodCollisionModel.h` (collision model,
probe record), `samples/physics/rigidbody/PhysicsBody.h` (rigid body),
`SoultreeObject.h` (`firstChild`, `subtreeDirty`) and `ParticleManager.h`
(particle manager and particle). Wrecker's own fields are named from the
data flow in the layout table: the three `int` arrays hold values passed to
the rider's animation calls `0x004a8b40`/`0x004a8b80` (`indexedAnimations`
by the index `0x00530630` returns, `randomAnimations` by `RANDOM_UNIT()`,
`contactAnimations` when the collision model reports contact), the
`ignoreObjects` list feeds `0x00439410` both ways, and `hitCenter` is the
mean of the hit probes' positions (`anyProbeHit` set when one hits).
`contactThisFrame` (+0xb0) is cleared before the frame's physics steps and
set when the collision model reports contact; `hadContact` (+0xac) is set
with it and cleared only when the wreck starts or is reset.
Renaming changed no calibration bytes.

Function names (tier 3, from behaviour): `0x00531da0` `EmitProbeTrail`
(particles along a probe's movement) and `0x00532580` `RebuildPoseSteps`
(the pose's linear and angular steps from the node's frame), and the
near miss `0x00531740` `SprayContactParticles`.

Calls into code another unit owns now use that unit's canonical name where
krusty2 already names the address and no other lane's header shares the
declaration: `AddIgnoredOwner` `0x00439410`, `SoultreeObject`'s
`SetMatrixIn` `0x004fb8c0`, `SetPositionIn` `0x004fc890`, `TranslateIn`
`0x004fc690`, `GetPosition` `0x004fc970` and `GetAxes` `0x004fc4f0`,
`Vec3ScaleCall` `0x005015b0` and `FastInvSqrt` `0x00460c00` (in the Wrecker,
CarProcedural, BikeRace, Krusty3DObjects, RaceSound, VehicleCamera,
FollowCamera and D3DIMSoulTree views). `FastSqrt` `0x00460b50`, `FindByName`
`0x004fdae0`, `GetPositionIn` `0x004fc9a0`, `CountNodes` `0x004fda30`,
`SetPosition` `0x004fc660`, `QueryGround` `0x00507c10` and `SelectQuality`
`0x00507960` keep their address names because AuralScape.cpp, BikeCamera.cpp,
ArcadeObject.cpp, QuarryStuntEvent.cpp or a game sample reach them through
these headers.

## Remaining uncertainty

All method and class names are provisional, as are the field names above
that come from data flow rather than literal strings. Field meanings beyond the offsets above are
inferred from the decoded data flow. Whether `+0x34` belongs to GraphicsTest
(the krusty2 tree gives GraphicsTest a `+0x34` field) is open. The X-axis
clip in `0x005329e0` reads the previous position's `y`; that is what retail
does, not a reconstruction error.

## Reproduce

```bash
python tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" --profile vc6_o2_mt \
  src/reconstructed/Wrecker.cpp -o work/wrecker.obj
python tools/match.py --exe "$MCM2_EXE" --target-va 0x005301c0 --target-size 995 \
  --obj work/wrecker.obj \
  --symbol '?UnknownFunction5301c0@Wrecker@@QAEXPAVUnknownWreckerCharacter@@PBD@Z' \
  --bindings src/reconstructed/Wrecker.bindings.json --json
```
