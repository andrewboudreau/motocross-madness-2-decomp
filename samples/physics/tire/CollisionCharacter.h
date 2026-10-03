// CollisionCharacter.h -- CollisionCharacter : D3DIMSoultreeCharacter (CollisionCharacter.cpp,
// tier 1: __FILE__ 'D:\aardvark\VC\krusty2\CollisionCharacter.cpp' pushed at 0x00431a11).
//
// Evidence:
//  * RTTI/vtables (analysis/vtables.json, rtti_classes.json): primary vtable 0x0055110c
//    (12 slots, all inherited from D3DIMSoultreeCharacter), GameObject virtual base vtable
//    0x0055109c at object offset 616 (0x268).  Tier 1.
//  * The constructor 0x004318d0 writes the vbptr 0x0055113c to +4, the primary vtable to +0,
//    the vbase vtable to [vbptr[1] + this] and the vtordisp (-0x264 relative to the vbase)
//    at 0x264.  So the class carries a vtordisp, D3DIM's non-virtual part ends at 0x210,
//    the own fields are 0x210..0x264 and GameObject sits at 0x268.  Tier 1 (decoded).
//  * vtable_thunks.json: vbase slots 0/10/14 are `sub ecx,[ecx-4]` thunks to
//    0x00431d20 (deleting dtor), 0x00431b30 and 0x00431cf0: the class overrides GameObject
//    slots 0, 10 and 14 (slot 14 is a plain tail call to the base).  Tier 1.
//  * The overriders 0x00431b30/0x00431cf0 are compiled with this == the GameObject subobject
//    (static `[ebx-0x58]` reaches field 0x210), which is what VC6 emits for an override of a
//    virtual-base function; the C++ below is an ordinary member function.
//  * Field semantics are tier 3.  CollisionCharacter attaches a CollisionObject (0x210) to a
//    scene node (0x214) and, every update, turns the node's movement into a velocity.
#ifndef TIRE_COLLISION_CHARACTER_H
#define TIRE_COLLISION_CHARACTER_H

#include "motion/D3DIMSoultreeCharacter.h"
#include "collision/CollisionObject.h"

class CollisionCharacter : public D3DIMSoultreeCharacter {
public:
    CollisionCharacter(int a);                  // 0x004318d0 (ret 8: a + hidden vbase flag)
    virtual ~CollisionCharacter();              // core 0x00431980, deleting 0x00431d20
    virtual int GameObjectVirtualSlot10(float dt);   // 0x00431b30 (ret 4), thunk 0x00431d80
    virtual int GameObjectVirtualSlot14();           // 0x00431cf0, thunk 0x00431d90

    // 0x004319c0 (ret 0x1c), non-virtual.  Not a vtable slot: it is absent from every
    // vtable, and the 0x00445680 call is a direct call.  Tier 3 name/argument roles.
    GameObject* Load(int a1, const char* name, const char* colPath, const SoultreeLoadDesc* desc,
                     int a5, int a6, int a7);

    CollisionObject* collisionObject;   // +0x210 allocated 0xb8 bytes (CollisionObject), created by Load
    SoultreeObject* sceneNode;      // +0x214 scene node (D3DIM::d3d_field_0x1a0 copy), tier 3
    Vec3 lastNodePosition;       // +0x218 last node position (updated by slot 10)
    Vec3 nodeVelocity;        // +0x224 velocity = (position - last position) / dt
    char field_0x230[0x18];         // not accessed by any target in this file
    Vec3 field_0x248;       // copy of *(Vec3*)(contact record + 0x18)
    Vec3 field_0x254;       // copy of *(Vec3*)(contact record + 0x0c)
    int field_0x260;
};

typedef char collision_character_assert_field[(sizeof(CollisionCharacter) == 0x268 + 0x2c) ? 1 : -1];

#endif
