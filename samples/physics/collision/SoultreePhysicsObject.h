// SoultreePhysicsObject and D3DIMSoultreeObject -- minimal class shapes for the slot 40
// loader 0x00503970 (SoulTreePhysics.cpp).
//
// Evidence (tier 1, analysis/rtti_classes.json and analysis/vtables.json):
//  * SoultreePhysicsObject : SoultreePhysicsBaseObject (+0), D3DIMSoultreeObject (+540).
//    COL/vtable records: 0x00557f54 @0 (41 slots, slot 40 = 0x00503970 introduced here),
//    0x00557f28 @540 (10 slots), 0x00557eb8 @552 (27), 0x00557e48 @1272 (27, the
//    SoultreePhysicsBaseObject virtual GameObject; slot 0 is the vtordisp thunk 0x00504260,
//    `sub ecx,[ecx-4]`, so a vtordisp dword sits at 0x4f4).
//  * D3DIMSoultreeObject : SoultreeObject : QuadTreeObject (+0), GameObject (+12), all
//    non-virtual (pdisp -1).  So SoultreePhysicsObject holds TWO GameObjects: the virtual
//    one from SoultreePhysicsBaseObject and a plain one at 0x21c + 12 = 0x228.
//  * D3DIMSoultreeObject primary vtable 0x005513ec: QuadTreeObject's two slots, then slots
//    2..9 (slot 9 = 0x0043f4b0, `ret 0x14`).
//
// Deliberate simplification (listed in hierarchy/MIGRATION.md): the SoultreeObject level is
// folded into D3DIMSoultreeObject, because common/SoultreeObject.h is the flat,
// non-polymorphic view of that class ("do not derive from it").  The base offsets and
// vtable shapes are the same.  Slot names are D3DIMObjectVirtualSlotN so they cannot
// override SoultreePhysicsBaseObject::UnknownVirtualSlotN by accident.  Only what the loader
// needs is declared; GameObject overrides (D3DIM slots 0/4/5/10/12/14, SoultreePhysicsObject
// slots 0/10) are left out.
#ifndef COLLISION_SOULTREE_PHYSICS_OBJECT_H
#define COLLISION_SOULTREE_PHYSICS_OBJECT_H

#include "../hierarchy/D3DIMSoultreeCharacter.h"   // SoultreePhysicsBaseObject, SoultreeLoadDesc
#include "CollisionObject.h"                       // QuadTreeObject, GameObject

class D3DIMSoultreeObject : public QuadTreeObject, public GameObject {
public:
    virtual void D3DIMObjectVirtualSlot2();       // 0x0043f950
    virtual void D3DIMObjectVirtualSlot3();       // 0x0043fe40
    virtual void D3DIMObjectVirtualSlot4();       // 0x004440a0
    virtual void D3DIMObjectVirtualSlot5();       // 0x00444140
    virtual void D3DIMObjectVirtualSlot6();       // 0x004450c0
    virtual void D3DIMObjectVirtualSlot7();       // 0x00444560
    virtual void D3DIMObjectVirtualSlot8();       // 0x00444b60
    // 0x0043f4b0, `ret 0x14`.  SoultreePhysicsObject slot 40 passes (a1, name, desc, a4, 1);
    // the argument types follow that caller (tier 2).
    virtual void D3DIMObjectVirtualSlot9(int a1, const char* a2, const SoultreeLoadDesc* a3,
                                         int a4, int a5);

    // QuadTreeObject (12) + GameObject (0x2c) = 0x38; the rest runs to the vtordisp at
    // 0x4f4 - 0x21c = 0x2d8 (extent only, tier 2).
    char d3do_field_0x38[0x2d8 - 0x38];
};

class SoultreePhysicsObject : public SoultreePhysicsBaseObject, public D3DIMSoultreeObject {
public:
    // Slot 40 (0x00503970, `ret 0x70` = 28 argument dwords, tier 1).  Grouping (tier 2):
    //  * a1..a4 go to D3DIMObjectVirtualSlot9 as (a1, a2, a3, a4, 1); a2 is also the name the
    //    ".col" path is built from, a3 is tested at +0x25.
    //  * a5..a7 are three Vec3 by value (struct-copy shape) and a8..a22 are forwarded
    //    unchanged to slot 2 as its a6..a20, so their types are slot 2's.  Slot 2 gets
    //    (a1, 1, a5, a6, a7, a8..a22).
    //  * The SceneManager caller 0x004ed3bf passes the result to GameObject::Method_0x00469190.
    //  * Returns the D3DIMSoultreeObject GameObject (+0x228), not the virtual one: the tail
    //    0x00503c16 null-checks `this`, then this+0x21c, then adds 12.
    virtual GameObject* UnknownVirtualSlot40(int a1, const char* a2, const SoultreeLoadDesc* a3,
                                             int a4, SoultreeVec3 a5, SoultreeVec3 a6,
                                             SoultreeVec3 a7, void* a8, void* a9, float a10,
                                             int a11, int a12, SoultreeSlot1f0* a13, float a14,
                                             int a15, float a16, float a17, float a18, int a19,
                                             int a20, unsigned char a21, int a22);
};

#endif
