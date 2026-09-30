// SoultreePhysicsCharacter -- canonical class shape (SoulTreePhysics.cpp, tier 2 TU:
// __FILE__ references at 0x00503b13/0x00503f87).
//
// Evidence (tier 1 unless noted):
//  * RTTI: SoultreePhysicsCharacter : SoultreePhysicsBaseObject (offset 0),
//    D3DIMSoultreeCharacter (offset 540), virtual GameObject.  vtables (analysis/vtables.json):
//      0x005580a8 @0    43 slots: SoultreePhysicsBaseObject's 40 + slots 40..42
//      0x00558074 @540  12 slots: identical to D3DIMSoultreeCharacter's primary vtable
//      0x00558004 @1080 27 slots: GameObject virtual base
//  * vbase deleting dtor 0x005042f0: `lea esi,[ecx-0x438]`, calls core 0x00503d40 then
//    ??1GameObject 0x00468d60 -> GameObject at 0x438 (1080).
//  * vbase slot 10 is the vtordisp thunk 0x00504350 (`sub ecx,[ecx-4]`) -> 0x00504210
//    (ret 4): this class overrides GameObject slot 10 and has a vtordisp at 0x434.
//    Vbase slots 4/5 are static adjustors `sub ecx,8` -> D3DIM's 0x00446640/0x00446620
//    (0x438 - (0x21c + 0x214) = 8), i.e. inherited, not overridden.
//  * Primary-vtable diff vs SoultreePhysicsBaseObject: slots 1, 8, 33 overridden,
//    40..42 introduced (0x00503de0, 0x00504360, 0x00504470).
//  * Layout: SoultreePhysicsBaseObject non-virtual part 0x000..0x21c,
//    D3DIMSoultreeCharacter 0x21c..0x42c, own fields 0x42c..0x434, vtordisp 0x434,
//    GameObject 0x438..0x464.  sizeof == 0x464.  Proven by hierarchy/LayoutProbe.cpp.
//  * Own fields: 0x42c is a node pointer (slot 8 calls 0x004fd7f0 on it); 0x430..0x433
//    are four byte flags cleared by slot 1 (0x005040c0).  The pointer the collision area
//    called field_0x3bc is D3DIMSoultreeCharacter::d3d_field_0x1a0.
#ifndef SOULTREE_PHYSICS_CHARACTER_CANONICAL_H
#define SOULTREE_PHYSICS_CHARACTER_CANONICAL_H

#include "../soultree_base/SoultreePhysicsBaseObject.h"
#include "D3DIMSoultreeCharacter.h"

class SoultreePhysicsCharacter : public SoultreePhysicsBaseObject, public D3DIMSoultreeCharacter {
public:
    SoultreePhysicsCharacter();
    virtual ~SoultreePhysicsCharacter();        // core 0x00503d40, deleting 0x005042f0
    virtual void GameObjectVirtualSlot10();     // 0x00504210 via vtordisp thunk 0x00504350
                                                // (ret 4: takes one dword; the shared
                                                // GameObject declaration is still void())
    // --- vtable 0x005580a8 (offset 0) ---
    virtual void UnknownVirtualSlot1(float value);                // 0x005040c0
    virtual void UnknownVirtualSlot8();                           // 0x005041c0
    virtual int UnknownVirtualSlot33(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                     const SoultreeVec3* a3, int a4, int a5, float a6); // 0x005040f0
    virtual void UnknownVirtualSlot40(int a1, int a2, int a3);   // 0x00503de0 (signature provisional)
    virtual void UnknownVirtualSlot41();                          // 0x00504360
    virtual int UnknownVirtualSlot42();                           // 0x00504470

    SoultreeNode* field_0x42c;
    char field_0x430;
    char field_0x431;
    char field_0x432;
    char field_0x433;
};

#endif
