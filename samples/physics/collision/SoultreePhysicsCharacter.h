// SoultreePhysicsCharacter / SoultreePhysicsObject -- PROVISIONAL derived-class declarations
// for the collision area's assigned slots (SoulTreePhysicsCharacter TU: SoulTreePhysics.cpp).
// Evidence: COL vtables at object offsets 0 (primary, inherited 40 slots from
// SoultreePhysicsBaseObject + introduced slots 40..42), 540 and 1080 (GameObject vbase).
// The bytes between the base's non-virtual part (0x21c) and the virtual base at 0x438 belong
// to a secondary base (D3DIMSoultreeCharacter, vtable at 540 = 0x21c) that is not
// reconstructed here; only the members that the assigned functions touch are declared,
// with padding between them (tier 3).
#ifndef SOULTREE_PHYSICS_CHARACTER_H
#define SOULTREE_PHYSICS_CHARACTER_H

#include "../soultree_base/SoultreePhysicsBaseObject.h"
#include "../soultree_base/SoultreePhysicsCallees.h"

// Non-virtual helper on the 0x21c subobject (0x004a8b00, thiscall, no args).
struct SoultreeSubobject21c {
    char data[0x1a0];
    void Fn_4a8b00();
};

class SoultreePhysicsCharacter : public SoultreePhysicsBaseObject {
public:
    virtual void UnknownVirtualSlot1(float value);
    virtual void UnknownVirtualSlot8();
    virtual int UnknownVirtualSlot33(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                     const SoultreeVec3* a3, int a4, int a5, float a6);
    virtual void UnknownVirtualSlot40(int a1, int a2, int a3);   // 0x00503de0 (signature provisional)
    virtual void UnknownVirtualSlot41();                          // 0x00504360
    virtual int UnknownVirtualSlot42();                           // 0x00504470

    SoultreeSubobject21c field_0x21c;    // 0x21c..0x3bc
    SoultreeNode* field_0x3bc;
    char pad_0x3c0[0x6c];
    SoultreeNode* field_0x42c;
    char field_0x430;
    char field_0x431;
    char field_0x432;
    char field_0x433;
};

#endif
