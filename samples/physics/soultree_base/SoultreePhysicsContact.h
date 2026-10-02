// PROVISIONAL layout of the objects the SoultreePhysicsBaseObject arrays point to.
// Only offsets that decoded accesses in SoulTreePhysics.cpp touch are declared.
#ifndef SOULTREE_PHYSICS_CONTACT_H
#define SOULTREE_PHYSICS_CONTACT_H

#include "SoultreePhysicsTypes.h"
#include "collision/CollisionObject.h"

// Element of SoultreePhysicsBaseObject::field_0x12c (count in field_0x130).
// Has a vfptr at +0 (slot 1 is called with no arguments by slot 13).  Tier 3 names.
// Slot 0 is the deleting destructor: the SoultreePhysicsBaseObject destructor 0x00501260
// deletes each element with `push 1; call [vptr]` (tier 1).  This is a stand-in view of
// collision/CollisionPoint.h's CollisionPoint (vtable 0x005511d8, slot 0 = 0x0043b300).
class SoultreeContact {
public:
    virtual ~SoultreeContact();
    virtual void UnknownVirtualSlot1();
    void* ownerNode;  // +0x04 slot 31 skips points whose ownerNode is null (contactState = 0); the destructor 0x00501260 deletes only points with a non-null ownerNode
    char field_0x08[0x0c];
    Vec3 worldPosition;  // +0x14 slot 7 weights each touching point by |centerOfMass - worldPosition|; 0x0043a640 recomputes the lever arm from it
    Vec3 field_0x20;
    Vec3 contactNormal;  // +0x2c slot 13: d = dot(loadShare * force, contactNormal); normalForce = contactNormal * -d when d < 0
    Vec3 leverArm;  // +0x38 0x0043a640 (slot 31 passes &leverArm) sets it relative to centerOfMass; slot 13 adds cross(leverArm, frictionForce) to the torque
    Vec3 forceShare;  // +0x44 slot 7: forceShare = force * loadShare for touching points (the whole force for a lone point), zero otherwise
    char field_0x50[0x18];
    Vec3 normalForce;  // +0x68 slot 13: normalForce = contactNormal * -d when the load pushes into the surface, else zero
    float normalForceMagnitude;  // +0x74 slot 13: normalForceMagnitude = -d (or 0); read by the point's slot 1 friction update
    Vec3 frictionForce;  // +0x78 produced by the point's slot 1 (called from slot 13); slot 13 adds it to the force and cross(leverArm, frictionForce) to the torque
    char field_0x84[0x14];
    float field_0x98;
    char field_0x9c[4];
    float loadShare;  // +0xa0 slot 7: 1 - dist/sumDist per touching point (1.0 for a lone point, 0 when not touching); slot 13 scales the body force by it
    int touching;  // +0xa4 slots 7 and 18 only use points with touching set; set by the Vehicle contact query
    int effectSpawned;  // +0xa8 slot 18 sets it when it places the type-1 attachment at this point and skips points already set; slot 30 clears it every step
    int field_0xac;
    int field_0xb0;
    int contactState;  // +0xb4 slot 31: 0 = no contact, 1 = in contact (left at 2 when already 2); slot 13 skips 0 and, for 2, scales the torque by a3 and resets it to 0
    char field_0xb8[4];
    char materialId;  // +0xbc slot 18 indexes the track's per-material byte table (+0x400) with (unsigned char)materialId

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
    Vec3 currentPosition;  // +0x44 written by 0x004b8d90 (slot 18); copied to previousPosition on a trail reset
    Vec3 previousPosition;  // +0x50 slot 18: previousPosition = currentPosition when the attachment's resetTrail is set
    int pad_0x5c;     // placeholder so +0x60 below lines up
    int activeThisFrame;  // +0x60 slot 21 clears it every frame; slot 18 sets it to 1 when the emitter is placed at a contact
    char field_0x64[0x10];
    int renderFlags;                          // +0x74 flag word (slot 21 sets/clears bit 0x800)
    void Fn_4b8d90(Vec3 v, int a);   // thiscall, callee pops 0x10
    void Fn_4b8dd0(int a, int b, int c);     // thiscall, callee pops 0xc (slot 21: 0x40/0xff tint)
};

// Object at SoultreeAttachment::field_0x10 (slot 17 sums its Vec3 at +0x20).
class SoultreeAttachTarget {
public:
    char field_0x00[0x20];
    Vec3 anchorPosition;  // +0x20 slot 17 sums anchorPosition over the attachments' anchors and divides by their count
};

// 40-byte record of SoultreePhysicsBaseObject::field_0x1d4 (slot 37 builds them).
// type 1..4 selects which of field_0x04/08/0c/14 receives the argument.
class SoultreeAttachment {
public:
    int type;
    SoultreeAttachedObject* contactEmitter;  // +0x04 type-1 target (slot 37); slot 18 places it at a touching contact (0x004b8d90) whose material allows it and marks it active (+0x60); slot 21 tints it (0x004b8dd0) and toggles its flag 0x800
    void* field_0x08;
    void* field_0x0c;
    SoultreeAttachTarget* anchor;  // +0x10 slot 37's a3; slot 17 averages the anchors' +0x20 positions (non-type-4 records) as the shadow probe origin
    void* pointEmitter;  // +0x14 type-4 target (slot 37); slot 21 moves it to the world point of localOffset (0x004ba2c0), starts/stops it (0x004ba300/0x004ba320) and tints it (0x004ba360)
    Vec3 localOffset;  // +0x18 type 4: node-local point given to slot 37; slot 21 maps it with LocalToWorldPoint each frame
    char resetTrail;  // +0x24 slot 21: resetTrail |= justReset; when set, slots 18/21 copy the target's current position over its previous one, then clear it
};

// Object at SoultreePhysicsBaseObject::field_0x1f0 (slot 18: byte table at +0x400 of *(+0xa4)).
class SoultreeFlagBlock {
public:
    char field_0x00[0x400];
    char materialEffectFlags[256];  // +0x400 slot 18: nonzero entry for the contact's material id allows the type-1 effect
};
class SoultreeSlot1f0 {
public:
    char field_0x00[0xa4];
    SoultreeFlagBlock* trackData;  // +0xa4 slot 18 reads trackData->field_0x400[materialId] to decide whether a contact may spawn the effect
};

#endif
