// LayoutProbe.cpp -- VC6-compiled proof that the canonical Soultree physics hierarchy
// reproduces the retail secondary-base and virtual-base offsets.
//
// Two kinds of proof:
//  1. Compile-time: sizeof / offsetof checks below (a failing check is a compile error).
//  2. Byte-level (hierarchy/targets.json): the compiler-generated vbase deleting
//     destructors (`lea esi,[ecx-VB]` encodes the GameObject offset) and the adjustor
//     thunks (`sub ecx,N` encodes vbase - (D3DIM base + 0x214), i.e. the 540 secondary
//     base offset) must match retail byte for byte.
//
// Vehicle, Bike and KrustyBike are reduced probe classes here: only their member-block
// sizes (read from vehicle/Vehicle.h, bike/Bike.h and krustybike/KrustyBike.h, corrected
// for the vtordisp) and the GameObject virtuals they override (from the vbase vtables)
// are modelled.  The full reconstructions live in those directories.
//
// The constructors defined here are probe-only empty bodies whose sole purpose is to
// make VC6 emit the vtables, ??_G and thunks; they are not reconstructions of the
// retail constructors.
#include <stddef.h>
#include "SoultreePhysicsCharacter.h"

#define HIER_CHECK(name, cond) typedef char hier_check_##name[(cond) ? 1 : -1]

// --- Vehicle stub: RTTI vtables @0 (97 slots), @540 (12), @1472 (27). -----------------
// vbase deleting dtor 0x0052b630 (`lea esi,[ecx-0x5c0]`); vbase slot 10 is the
// vtordisp thunk 0x0052b690 -> 0x0052a830 (ret 4).  Own fields 0x434..0x5bc.
class Vehicle : public SoultreePhysicsCharacter {
public:
    Vehicle();
    virtual ~Vehicle();                         // core 0x00526380
    virtual int GameObjectVirtualSlot10(float dt);  // 0x0052a830
    char veh_block_0x434[0x5bc - 0x434];        // 392 bytes; vtordisp at 0x5bc, GameObject at 0x5c0
};

// --- Bike stub: RTTI vtables @0 (103 slots), @540 (12), @1848 (27). -------------------
// vbase deleting dtor 0x0040ca50 (`lea esi,[ecx-0x738]`); Bike does not override slot 10
// (vbase slot 10 = 0x0040cab0 `sub ecx,[ecx-4]; sub ecx,0x178; jmp 0x0052a830`).
class Bike : public Vehicle {
public:
    Bike();
    virtual ~Bike();                            // core 0x00409a10
    char bike_block_0x5bc[0x734 - 0x5bc];       // 376 bytes; vtordisp at 0x734, GameObject at 0x738
};

// --- KrustyBike stub: RTTI vtables @0 (103 slots), @540 (12), @5644 (27). ------------
// vbase deleting dtor 0x00497c40 (`lea esi,[ecx-0x160c]`); vbase slot 10 is the
// vtordisp thunk 0x00497ca0 -> 0x004977a0.
class KrustyBike : public Bike {
public:
    KrustyBike();
    virtual ~KrustyBike();                      // core 0x00491540
    virtual int GameObjectVirtualSlot10(float dt);  // 0x004977a0
    char kb_block_0x734[0x1608 - 0x734];        // 3796 bytes; vtordisp at 0x1608, GameObject at 0x160c
};

// --- compile-time layout checks --------------------------------------------------------
// sizeof(GameObject) == 0x2c (GameObject ctor 0x00468ca0 field extent, tier 2).
HIER_CHECK(gameobject_size, sizeof(GameObject) == 0x2c);
// Character: non-virtual 0x1a0, no vtordisp, GameObject at 0x1a0 (416).
HIER_CHECK(character_size, sizeof(Character) == 0x1a0 + 0x2c);
// D3DIM: non-virtual 0x210, vtordisp 0x210, GameObject at 0x214 (532).
HIER_CHECK(d3dim_field, offsetof(D3DIMSoultreeCharacter, d3d_field_0x1a0) == 0x1a0);
HIER_CHECK(d3dim_size, sizeof(D3DIMSoultreeCharacter) == 0x214 + 0x2c);
// SoultreePhysicsBaseObject: GameObject at 0x220 (544), non-virtual part 0x21c.
HIER_CHECK(spbo_size, sizeof(SoultreePhysicsBaseObject) == 0x220 + 0x2c);
HIER_CHECK(spbo_last, offsetof(SoultreePhysicsBaseObject, field_0x218) == 0x218);
// SoultreePhysicsCharacter: D3DIM at 540 (0x21c) ends at 0x42c, own fields, GameObject at 1080.
HIER_CHECK(spc_node, offsetof(SoultreePhysicsCharacter, field_0x42c) == 0x42c);
HIER_CHECK(spc_flags, offsetof(SoultreePhysicsCharacter, field_0x433) == 0x433);
HIER_CHECK(spc_d3d, offsetof(SoultreePhysicsCharacter, d3d_field_0x1a0) == 0x3bc);
HIER_CHECK(spc_size, sizeof(SoultreePhysicsCharacter) == 1080 + 0x2c);
HIER_CHECK(veh_block, offsetof(Vehicle, veh_block_0x434) == 0x434);
HIER_CHECK(veh_size, sizeof(Vehicle) == 1472 + 0x2c);
HIER_CHECK(bike_block, offsetof(Bike, bike_block_0x5bc) == 0x5bc);
HIER_CHECK(bike_size, sizeof(Bike) == 1848 + 0x2c);
HIER_CHECK(kb_block, offsetof(KrustyBike, kb_block_0x734) == 0x734);
HIER_CHECK(kb_size, sizeof(KrustyBike) == 5644 + 0x2c);

// --- probe-only constructors (emit vtables, ??_G and thunks) ---------------------------
// GameObject has only the explicit GameObject(int) ctor (0x00468ca0), so each most-derived
// probe ctor names the virtual-base initializer.  D3DIMSoultreeCharacter's retail ctor
// 0x004455b0 passes 1 (tier 1); the other arguments are probe-only.
Character::Character() : GameObject(0) {}
D3DIMSoultreeCharacter::D3DIMSoultreeCharacter(int) : GameObject(1) {}
Vehicle::Vehicle() : GameObject(0), SoultreePhysicsCharacter(0) {}
Bike::Bike() : GameObject(0) {}
KrustyBike::KrustyBike() : GameObject(0) {}

// Secondary-base offset probe: the D3DIM subobject of a SoultreePhysicsCharacter.
// VC6 emits `lea eax,[ecx+0x21c]` guarded by a null test; kept for disassembly review.
D3DIMSoultreeCharacter* LayoutProbe_SpcToD3DIM(SoultreePhysicsCharacter* p)
{
    return p;
}

// --- SoultreePhysicsCharacter constructor (0x00503c70, retail TU SoulTreePhysics.cpp) ----
// A reconstruction, not a probe: the virtual base is built with GameObject(1) (`push 1`
// before 0x00468ca0), both non-virtual bases receive the caller's argument, and the only
// own-member store is the node pointer.
SoultreePhysicsCharacter::SoultreePhysicsCharacter(int flags)
    : GameObject(1), SoultreePhysicsBaseObject(flags), D3DIMSoultreeCharacter(flags)
{
    field_0x42c = 0;
}

// Destructor core (0x00503d40): no own cleanup; VC6 restores the vptrs/vtordisp and runs
// ~D3DIMSoultreeCharacter (0x004459a0) then ~SoultreePhysicsBaseObject (0x00501260).
SoultreePhysicsCharacter::~SoultreePhysicsCharacter()
{
}

// GameObject slot 10 override (0x00504210, reached through the vtordisp thunk 0x00504350).
// When slot 42 reports true, field_0x430 is cleared and field_0x431 set (both are cleared by
// slot 1); then the physics update (SoultreePhysicsBaseObject, 0x005036f0) and the plain
// GameObject update (0x004693d0) run with the same frame time.  Tier 1 control flow.
int SoultreePhysicsCharacter::GameObjectVirtualSlot10(float dt)
{
    if (UnknownVirtualSlot42()) {
        field_0x430 = 0;
        field_0x431 = 1;
    }
    SoultreePhysicsBaseObject::GameObjectVirtualSlot10(dt);
    return GameObject::GameObjectVirtualSlot10(dt);
}
