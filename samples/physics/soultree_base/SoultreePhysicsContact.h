// PROVISIONAL layout of the objects the SoultreePhysicsBaseObject arrays point to.
// Only offsets that decoded accesses in SoulTreePhysics.cpp touch are declared.
#ifndef SOULTREE_PHYSICS_CONTACT_H
#define SOULTREE_PHYSICS_CONTACT_H

#include "SoultreePhysicsTypes.h"
#include "collision/CollisionObject.h"

// Element of SoultreePhysicsBaseObject::field_0x12c (count in field_0x130).
// Has a vfptr at +0 (slot 1 is called with no arguments by slot 13).  Tier 3 names.
class SoultreeContact {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    void* field_0x04;
    char field_0x08[0x0c];
    Vec3 field_0x14;
    Vec3 field_0x20;
    Vec3 field_0x2c;
    Vec3 field_0x38;
    Vec3 field_0x44;
    char field_0x50[0x18];
    Vec3 field_0x68;
    float field_0x74;
    Vec3 field_0x78;
    char field_0x84[0x14];
    float field_0x98;
    char field_0x9c[4];
    float field_0xa0;
    int field_0xa4;
    int field_0xa8;
    int field_0xac;
    int field_0xb0;
    int field_0xb4;
    char field_0xb8[4];
    char field_0xbc;

    // 0x0043a640, thiscall, callee pops 5 args
    void Fn_43a640(Vec3* a, Vec3* b, Vec3* c, Vec3* d, float e);
};

// The object at SoultreePhysicsBaseObject::field_0x128 is a CollisionObject (0xb8 bytes,
// ctor 0x00431e70; tier 1) -- see collision/CollisionObject.h (src/krusty2).  Its field_0xa0 (contact
// point) and field_0xac (contact normal) are CollisionVec3 there, which slots 28/38 view
// as Vec3 until the collision vector type is unified with Vec3.

// Object at SoultreeAttachment::field_0x04 (slot 18: position at +0x44/+0x50).
class SoultreeAttachedObject {
public:
    char field_0x00[0x44];
    Vec3 field_0x44;
    Vec3 field_0x50;
    int pad_0x5c;     // placeholder so +0x60 below lines up
    int field_0x60;
    char field_0x64[0x10];
    int field_0x74;                          // flag word (slot 21 sets/clears bit 0x800)
    void Fn_4b8d90(Vec3 v, int a);   // thiscall, callee pops 0x10
    void Fn_4b8dd0(int a, int b, int c);     // thiscall, callee pops 0xc (slot 21: 0x40/0xff tint)
};

// Object at SoultreeAttachment::field_0x10 (slot 17 sums its Vec3 at +0x20).
class SoultreeAttachTarget {
public:
    char field_0x00[0x20];
    Vec3 field_0x20;
};

// 40-byte record of SoultreePhysicsBaseObject::field_0x1d4 (slot 37 builds them).
// type 1..4 selects which of field_0x04/08/0c/14 receives the argument.
class SoultreeAttachment {
public:
    int type;
    SoultreeAttachedObject* field_0x04;
    void* field_0x08;
    void* field_0x0c;
    SoultreeAttachTarget* field_0x10;
    void* field_0x14;
    Vec3 field_0x18;
    char field_0x24;
};

// Object at SoultreePhysicsBaseObject::field_0x1f0 (slot 18: byte table at +0x400 of *(+0xa4)).
class SoultreeFlagBlock {
public:
    char field_0x00[0x400];
    char field_0x400[256];
};
class SoultreeSlot1f0 {
public:
    char field_0x00[0xa4];
    SoultreeFlagBlock* field_0xa4;
};

#endif
