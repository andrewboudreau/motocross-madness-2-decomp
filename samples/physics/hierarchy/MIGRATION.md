# Migrating onto the canonical Soultree hierarchy

## Canonical headers

- `hierarchy/SoultreePhysicsCharacter.h` includes everything below it.
- `hierarchy/D3DIMSoultreeCharacter.h` holds `Character` and `D3DIMSoultreeCharacter`.
- `soultree_base/SoultreePhysicsBaseObject.h` holds `SoultreePhysicsBaseObject`. Agent A owns it. Include it; do not fork it.
- `soultree_base/GameObject.h` holds `BaseObject` and `GameObject`. `SoultreePhysicsBaseObject.h` includes it. Hierarchies outside Soultree (CollisionObject) include only this file.
- `collision/CollisionObject.h` holds `GraphicsTest`, `QuadTreeObject` and `CollisionObject`.

`hierarchy/LayoutProbe.cpp` proves the layout. There are 15/15 exact byte matches under `vc6_o2_ml`: six vbase deleting destructors, eight adjustor thunks and one vtordisp thunk. There are also compile-time `sizeof`/`offsetof` checks.

| class | secondary D3DIM | own fields | vtordisp | GameObject (vbase) | sizeof |
|---|---|---|---|---|---|
| Character | - | 0x008..0x1a0 | none | 0x1a0 (416) | 0x1cc |
| D3DIMSoultreeCharacter | - | 0x1a0..0x210 | 0x210 | 0x214 (532) | 0x240 |
| SoultreePhysicsBaseObject | - | 0x008..0x21c | 0x21c | 0x220 (544) | 0x24c |
| SoultreePhysicsCharacter | 0x21c (540) | 0x42c..0x434 | 0x434 | 0x438 (1080) | 0x464 |
| Vehicle | 0x21c | 0x434..0x5bc | 0x5bc | 0x5c0 (1472) | 0x5ec |
| Bike | 0x21c | 0x5bc..0x734 | 0x734 | 0x738 (1848) | 0x764 |
| KrustyBike | 0x21c | 0x734..0x1608 | 0x1608 | 0x160c (5644) | 0x1638 |

**The compiler inserts the vtordisp. Never declare it as a field.**

In every class below, the byte range that holds the vtordisp must not be covered by padding. If it is, the GameObject base ends up 4 bytes too high.

## Status (soultree_base, vehicle, bike, krustybike migrated)

**The generators are retired.** These files moved to `work/<area>/retired/`:

- `work/c_vehicle/{gen.py,Vehicle.h.tpl,fields.py,slots.py,heavy.cpp,build.sh}`
- `work/d_bike/{gen.py,Bike.h.in,btypes.py}`
- `work/e_krustybike/spec.py`
- `work/a_soultree_base/merge.py`

The committed files under `samples/physics/` are now the hand-edited source of truth.

- **Base classes.** Vehicle, Bike and KrustyBike derive from the canonical classes. All three vtable_counts checks print `ok` (97/12/27 for Vehicle; 103/12/27 for Bike and KrustyBike). `sizeof` is asserted in each header.
- **Slot conflicts (table below).** All 12 are resolved in `SoultreePhysicsBaseObject.h`, and each carries a tier comment. Slot 14's `a1` is non-const, because KrustyBike 0x004965e0 writes `a1->x` and `a1->z`.
- **Vehicle slot 64 is `void(float)`.** Retail 0x00528e50 materialises no return value, and it is byte-exact as `void`. Slot 63 keeps `int(float)`.
- **`field_0x1f8` is `float`.** Tier 1: KrustyBike slot 12 (0x0048de20) does `fld`/`fmul` on it.
- **Cast at the use site.** Do not use inline accessor functions for rule 6. VC6 schedules the load differently when the cast is inside an inline accessor. Bike slot 8 lost its exact match that way, and Vehicle slots 4, 19, 35 and 49 lost bytes.

## Status (collision, constraint migrated)

No generators wrote into `collision/` or `constraint/`. The scripts in `work/b2_collision`, `work/b_soultree_collision` and `work/f_constraint` only compile and match, so there was nothing to retire.

- **`collision/SoultreePhysicsCharacter.h` is deleted.** `collision/SoultreePhysicsCharacter.cpp` includes the canonical header and `SoultreePhysicsCallees.h`.
  - `field_0x21c.Fn_4a8b00()` became `ApplyRestPose()`.
  - `field_0x3bc` became `d3d_field_0x1a0`.
  - Slot 33's `a4` is `const SoultreeVec3*`.
- **`collision/CollisionObject.h` is canonical.** `constraint/ConstraintBase.h` is deleted, and constraint includes `../collision/CollisionObject.h`.
  - The stub GameObject is gone. `GraphicsTest : GameObject` (non-virtual) now owns `char field_0x2c[0x18]`.
  - `GraphicsTest` no longer declares a slot 10 override: its vtable inherits 0x004693d0.
  - `sizeof(GraphicsTest) == 0x44` and `sizeof(CollisionObject) == 0xb8` are asserted.
  - The five non-virtual CollisionObject methods that constraint used (0x004324b0, 0x00432800, 0x00435fb0, 0x00435fe0, 0x00438e70) moved into the canonical header.
  - Overrides are `GameObjectVirtualSlot10(float)`, `GameObjectVirtualSlot14()` and `GameObjectVirtualSlot23(int,int)`. Constraint's overrides are `GameObjectVirtualSlot8/10/11/14`, and `constraint/targets.json` names them `GameObjectVirtualSlotN@ConstraintMethodCollisionModel`. No collision target named a renamed slot.
  - `QuadTreeObject::field_0x04/0x08` now share names with `BaseObject::field_0x04` and `GameObject::field_0x08`, so the CollisionObject ctor writes `QuadTreeObject::field_0x08`.
- **Why `GameObject.h` exists.** `SoultreePhysicsBaseObject.h` pulls in `common/Math3D.h`, and Math3D's four `static const Vec3` constants add dynamic initializers to every TU that includes it. Including it from CollisionObject.h renumbered constraint's TU initializer (`$E1`, 0x0043c8e0) and broke that target. The retail TU has exactly one initializer.
- **`GameObjectVirtualSlot23(int a, int b)`.** Retail 0x004695d0 is `ret 8`, and CollisionObject overrides it (0x00434970). This is the only placeholder signature changed.
- **vtable_counts all print `ok`:** CollisionObject 2/27, QuadTreeObject 2, ConstraintMethodCollisionModel 3/27, GameObject 27, SoultreePhysicsBaseObject 40/27, SoultreePhysicsCharacter 43/12/27, KrustyBike 103/12/27.
- **SoultreePhysicsCharacter slot 40 (0x00503de0) is resolved.** It is `GameObject* (int, int, const char*, void*, int, Vec3, Vec3, Vec3, void*, int, int, int, int, void*, float, int, float, float, int, char, int)`, which is 27 dwords. A compiled definition emits `ret 0x6c` and the retail `this ? vbase : 0` tail. The per-argument evidence is in the header comment.

### Leftover canonical-header disagreements

1. **SoultreePhysicsBaseObject slot 2 (0x00500c50)** is still 26 `int`s. Its arguments 2..10 are three `Vec3` by value: SPC slot 40 forwards them with the struct-copy shape.
   - The slot 40 body cannot be written readably until slot 2 changes. Both slot-40 targets (0x00503de0 and SoultreePhysicsObject's 0x00503970) remain unimplemented.
   - SoultreePhysicsObject slot 40 is `ret 0x70`. Its SceneManager caller is at 0x004ed3bf.
2. **GameObject placeholders.** They are still `void()`, but retail default bodies pop arguments:
   - `ret 4`: slots 9, 16, 19, 20, 21, 25
   - `ret 8`: slot 22
   - `ret 0x14`: slot 24
   Any future override must fix the placeholder first, as was done for slot 23.
3. **`collision/ConstraintMethodCollisionModel.{h,cpp}` is a second, minimal declaration of the constraint class.** It covers only primary slot 2 (0x0044d710, which is also a collision target).
4. **Three vector types.**
   - `CollisionVec3` (collision/CollisionTypes.h) and `ConVec3` (constraint/ConstraintTypes.h) are the Math3D `Vec3` layout.
   - They stay separate on purpose: they declare out-of-line operators (to force retail calls), while Math3D defines them inline.
   - Unifying them needs the Math3D static-initializer issue solved first.

## General rules

1. **No `field_0x04`.** Offset +4 is the vbptr. Never declare it or a stand-in for it.
2. **Reach GameObject members through the virtual base.** Code in derived classes names GameObject members directly, for example `field_0x18`. VC6 then emits `mov r,[this+4]; mov r,[r+4]; ... [r+this+4+disp]`.
   - The displacement in that retail code is **GameObject offset + 4**, because it is relative to the vbptr at +4.
   - So retail `[ecx+esi+0x1c]` is `GameObject::field_0x18`, not `field_0x1c`.
   - **Bike slot 97 fix:** this is the Bike "+4" problem. `BikeGameObject::go_0x1c` is really `GameObject+0x18`. `field_0x4.Method_0x00469190` is really `GameObject::Method_0x00469190`, with `this` equal to the GameObject itself (`lea ecx,[edx+esi+4]`).
   - The vtordisp does **not** cause the Bike problem. VC6 puts the stand-in Bike's GameObject at 0x738 too (`sizeof` 0x758 − 0x20).
3. **Override signatures must match the base declaration exactly.** VC6 does not report a parameter-list mismatch. The function silently becomes a new virtual and the vtable grows.
   - After migrating, run `PYTHONPATH=. python samples/physics/hierarchy/vtable_counts.py <area obj>`. It must print `ok` for every real class.
   - In the LayoutProbe object, `DIFF` is expected for the Vehicle, Bike and KrustyBike stubs.
4. **Slot names in the canonical hierarchy:**
   - GameObject slots use `BaseObjectVirtualSlot1..3` and `GameObjectVirtualSlot4..26`, plus `~GameObject` for slot 0.
   - The primary slots of SoultreePhysicsBaseObject and its descendants are `UnknownVirtualSlot0..`.
   - Character and D3DIM slots use `CharacterVirtualSlot0..10` and `D3DIMVirtualSlot11`. The distinct prefix avoids ambiguous lookups and accidental overrides between the two bases of SoultreePhysicsCharacter.
5. **D3DIM fields have a class prefix.** Character and D3DIM fields are named `chr_field_0xNN` and `d3d_field_0xNN`, relative to their own class start. Plain `field_0x1a0` exists in both SoultreePhysicsBaseObject and D3DIM, so an unprefixed name would be ambiguous.
   - Absolute 0x3bc = `d3d_field_0x1a0`.
6. **The node class has three names.** `SoultreeNode` (soultree_base), `SoultreeObject` (`common/SoultreeObject.h`), `VehicleXform`, `BikeXform` and `KbXform` are one class. Their method addresses overlap: 0x4fc050, 0x4fc540, 0x4fc970, 0x4fd710 and 0x4fd7f0.
   - Unifying them is a follow-up (tier 2). Until then, cast at use sites, or give area-local wrappers no fields.

## Required edits to `soultree_base/SoultreePhysicsBaseObject.h` first (Agent A)

1. **GameObject slot signatures.** These come from the constraint area's retail evidence and from `ret N`:
   - `virtual GameObject* GameObjectVirtualSlot8(int a);` (0x004692f0: `field_0x18 = a; return this`).
   - `virtual int GameObjectVirtualSlot10(float dt);` (0x004693d0 and every override use `ret 4`: 0x005036f0, 0x00504210, 0x0052a830, 0x004977a0).
   - `virtual int GameObjectVirtualSlot11(float dt);` (0x004da540: `mov eax,1; ret 4`).
   - After this change, update the SoultreePhysicsBaseObject slot-10 override, and the SoultreePhysicsCharacter one in `hierarchy/SoultreePhysicsCharacter.h`, to `int GameObjectVirtualSlot10(float dt)`. Both are currently `void()` to match the shared header.
2. **GameObject fields.** Replace `char field_0x08[0x24]` with `char field_0x08[0x10]; void* field_0x18; char field_0x1c[0x10];`.
   - Add the non-virtual `void Method_0x00469190(void* a, int b);` (bike slot 97).
3. **Reconcile slot signatures.** SoultreePhysicsBaseObject uses `int` placeholders where areas have typed evidence.
   - Pick one signature per slot. Ideally the area with a byte-exact body wins.
   - Conflicts:

     | Slot | Conflict |
     |---|---|
     | 1 | vehicle/bike/kb use `(int)`; canonical `(float)` is byte-exact in the SoultreePhysicsBaseObject and SoultreePhysicsCharacter code |
     | 3 | vehicle has `(Vec3*,Vec3*,Vec3*,Vec3*,int,int,Vec3*)`; kb ends in `int*` |
     | 4 | kb has `int*` at a13 |
     | 9 | vehicle has `(int frame, int*)` vs canonical `(float dt, int*)` |
     | 11 | vehicle has `void(int,Vec3*,Vec3*,Vec3*,int*)` vs canonical `int(int×5)` |
     | 12 | canonical is `int(int)` |
     | 14 | vehicle/kb have `(Vec3*,Vec3*,Vec3*)` vs canonical `(const Vec3*,const Vec3*,int)` |
     | 15 | kb has `(Vec3*,float*)` |
     | 16 | bike takes `const Vec3&`, kb takes `Vec3*`; canonical is `const Vec3*` |
     | 33 | vehicle has 4×`Vec3*`,int,float; kb has 6 ints |
     | 35 | vehicle returns `void`; canonical returns `int` |
     | 38 | vehicle has `(int,int,void*)` |

   - Any of these left unreconciled becomes a phantom new slot in the migrating area.

## Collision: `collision/SoultreePhysicsCharacter.{h,cpp}`

A trial migration compiled against the canonical header and reproduced the same results: 5 exact, 2 partial, plus the 2 unimplemented slot-40 entries.

- **Delete** `collision/SoultreePhysicsCharacter.h` entirely, including `SoultreeSubobject21c`.
- **Includes in the .cpp:**
  - `#include "../hierarchy/SoultreePhysicsCharacter.h"`
  - `#include "../soultree_base/SoultreePhysicsCallees.h"`
- **Code:**
  - `field_0x21c.Fn_4a8b00()` becomes `ApplyRestPose()`.
  - `field_0x3bc->` becomes `d3d_field_0x1a0->`.
- **Offset fixes:** none. 0x42c and 0x430..0x433 already sit in SoultreePhysicsCharacter.
- **Placement:** `SoultreePhysicsObject` and `D3DIMSoultreeObject` are declared in `src/krusty2/soultree/SoultreePhysicsObject.h` (the SoultreeObject level is folded into D3DIMSoultreeObject); its ctor/dtor/slot 10 are in `SoulTreePhysics.cpp`, slot 40 (0x00503970) stays in collision.

## Vehicle: `vehicle/Vehicle.h`, generated from `work/c_vehicle/Vehicle.h.tpl` and `gen.py`

- **Delete** the whole stand-in `class SoultreePhysicsCharacter { ... };` (Vehicle.h lines 206–323, slots 0–42 and `field_0x04` .. `field_0x42c`).
  - Also delete the `VEH_CHECK_OFFSET(SoultreePhysicsCharacter, ...)` lines. `hierarchy/LayoutProbe.cpp` covers them.
- **Include** `"../hierarchy/SoultreePhysicsCharacter.h"`.
  - `VehVec3` can become `typedef Vec3 VehVec3;` because both are Math3D Vec3.
  - Keep `VehicleXform` only as a cast target (general rule 6).
- **Declare** `class Vehicle : public SoultreePhysicsCharacter`, with a `Vehicle(); virtual ~Vehicle();` pair and `int GameObjectVirtualSlot10(float dt);` (0x0052a830).
  - Vehicle overrides GameObject slots 0 and 10 only.
  - Slots 4 and 5 are adjustor thunks to D3DIM, generated automatically.
- **Offset fixes:**
  - Remove `bool field_0x430; char pad_0x431[2]; char field_0x433;` from Vehicle. They are `SoultreePhysicsCharacter::field_0x430..0x433` (char). Check `bool` vs `char` compares in the Vehicle bodies.
  - Vehicle's first own field is `field_0x434`.
  - The tail `char pad_0x5B8[0x8]` becomes `char pad_0x5B8[0x4]`. Vehicle's own data ends at 0x5bc; the vtordisp is 0x5bc..0x5c0.
  - `field_0x3bc` becomes `d3d_field_0x1a0`.
  - `field_0x42c` changes from `int` to `SoultreeNode*`, the canonical pointer type. Adjust casts.
  - Stand-in field types that differ from SoultreePhysicsBaseObject: 0x24 `int`→`float`, 0x2c/0x48 `VehBlock7`→7 floats, 0x14c and 0x160 `int`→`float`, the `bool`→`char` flags, and pointer types at 0x128, 0x12c, 0x1f0 and 0x218. Use casts or reconcile the types in soultree_base.
- **Primary slots:** override 0..39 with the reconciled SoultreePhysicsBaseObject signatures. Keep 40..42 SoultreePhysicsCharacter-compatible and introduce 43..96.
  - Run `vtable_counts.py` and expect 97 / 12 / 27.

## Bike: `bike/Bike.h`, generated from `work/d_bike`

- **Delete** these stand-ins:
  - `BikeGOSub`
  - `BikeGameObject`
  - the stand-in `SoultreePhysicsCharacter : public virtual BikeGameObject`
  - `D3IMSoultreeCharacter` (misspelled; the real name is **D3DIM**)
  - the stand-in `Vehicle`
- **Include** `"../vehicle/Vehicle.h"` once it is migrated. Until then, include `"../hierarchy/SoultreePhysicsCharacter.h"` and keep a Vehicle stand-in with `char pad_0x434[0x188]` for 0x434..0x5bc, plus Vehicle's slot list.
- **Code:**
  - `D3IMSoultreeCharacter::ApplyRestPose/8bf0/8c50` becomes `D3DIMSoultreeCharacter::` (declared on `Character`, reachable through D3DIM).
  - `go_0x1c` becomes `field_0x18`, and `BikeGameObject::field_0x4.Method_0x00469190(...)` becomes `GameObject::Method_0x00469190(...)`.
- **Offset fixes:**
  - The stand-in Vehicle's `int field_0x5bc` is Bike's own first field, since Vehicle's data ends at 0x5bc. Move it into Bike.
  - Bike's `char pad_0x728[16]` becomes `char pad_0x728[12]`. The vtordisp occupies 0x734..0x738, and the compiler places it.
  - Bike overrides only the GameObject destructor. GameObject slot 10 is inherited through the thunk `sub ecx,[ecx-4]; sub ecx,0x178` (0x0040cab0, byte-matched).
  - Expect vtable counts of 103 / 12 / 27.

## KrustyBike: `krustybike/KrustyBikeTypes.h` and `KrustyBike.h` (now `src/krusty2/vehicle/`), generated from `work/e_krustybike/spec.py`

- **Delete** the flat stand-in `class Bike { ... };` in KrustyBikeTypes.h.
- **Include** `"../bike/Bike.h"` once it is migrated.
- **Offset fixes:**
  - The stand-in Bike ends at 0x740. The real Bike's data ends at **0x734**, so `field_0x734`, `field_0x735` and `field_0x736` (and `pad_0x0737`) belong to **KrustyBike**. Move them in.
  - KrustyBike's own block is 0x734..0x1608. The stand-in stops at `field_0x15e5` (`sizeof` 0x15e8). Add trailing padding to 0x1608 so that the vbase lands at 0x160c.
  - Add `KrustyBike(); virtual ~KrustyBike(); int GameObjectVirtualSlot10(float dt);` (0x004977a0).
  - Expect vtable counts of 103 / 12 / 27.

## CollisionObject chain: `collision/CollisionObject.h` vs `constraint/ConstraintBase.h`

**`collision/CollisionObject.h` becomes canonical.** It is the full chain: shape structs, narrow phase and ctor evidence. `constraint/ConstraintBase.h` is deleted once the fixes below land. Constraint then includes `"../collision/CollisionObject.h"`.

Layout (tier 1, see the CollisionObject.h header comment):

- `class CollisionObject : public QuadTreeObject /*@0, 12 bytes*/, public GraphicsTest /*@12*/`
- `GraphicsTest : GameObject` is **non-virtual** inheritance (pdisp −1). So none of the vbptr, vtordisp or `+4` displacement rules above apply here.
- sizeof(CollisionObject) is 0xb8, and its fields start at 0x50.

Fixes in CollisionObject.h:

1. **Replace the stub.** Replace its flat `GameObject` stub with the canonical `BaseObject`/`GameObject` (include `../soultree_base/SoultreePhysicsBaseObject.h`, or a future split-out `GameObject.h`).
   - The stub's `int field_0x04[5]; void* field_0x18; char field_0x1c[0x28];` becomes:
     - `BaseObject::field_0x04`
     - `GameObject::field_0x08[0x10]`
     - `GameObject::field_0x18`
     - `GameObject::field_0x1c[0x10]` (GameObject ends at 0x2c)
     - **`GraphicsTest`** gets `char field_0x2c[0x18]` (0x2c..0x44). These bytes are GraphicsTest's own fields, not GameObject's.
2. **Corrected slot signatures** (constraint area evidence, commit 49ef70c):
   - slot 8 `GameObject* (int a)`
   - slot 10 `int (float dt)`
   - slot 11 `int (float dt)`
   - The stub currently has `void(int)`, `void(int)` and `int(int)`.
3. **Rename** GameObject-slot overrides to the canonical names:
   - `UnknownVirtualSlot10` → `GameObjectVirtualSlot10(float dt)` (CollisionObject 0x00499ae0, GraphicsTest)
   - `UnknownVirtualSlot14` → `GameObjectVirtualSlot14`
   - `UnknownVirtualSlot23` → `GameObjectVirtualSlot23`
   - Constraint's `UnknownVirtualSlot8/10/11` → `GameObjectVirtualSlot8/10/11`
   - Slots 1–3 are `BaseObjectVirtualSlot1..3`.
   - Mangled names change, so update the `candidate_symbol_contains` values in collision and constraint `targets.json` (for example `GameObjectVirtualSlot10@CollisionObject`).
