// CollisionPoint -- reconstructed layout (CollisionPoint.cpp).
// Evidence: RTTI CollisionPoint, no bases, vtable 0x005511d8 (2 slots: 0 = deleting
// destructor 0x0043b300, 1 = 0x0043b240).  Size 0xc4 comes from the allocation
// in 0x0043a330 (operator new(0xc4, __FILE__, 27)).  Member names are provisional.
#ifndef COLLISION_POINT_H
#define COLLISION_POINT_H

#include "collision/CollisionTypes.h"

// The object a contact point belongs to (CollisionPoint::field_0x04).  Only one method
// is used: 0x004fd660 (thiscall, ret 8), which the scene node class (SoultreeObject)
// also has, so this is a provisional view of that node.  Tier 3 name.
class CollisionContactOwner {
public:
    // Returns a pointer to a temporary vector computed from the owner and a point.
    CollisionVec3* LocalToWorldPoint(CollisionVec3* tmp, const CollisionVec3* p);
};

class CollisionPoint {
public:
    CollisionPoint(float a, int b);
    virtual ~CollisionPoint() {}               // slot 0 (deleting destructor 0x0043b300; core inlined)
    // slot 1 (0x0043b240).  Named apart from QuadTreeObject::UnknownVirtualSlot1 so that
    // Tire, which derives from both, can override this one alone.
    virtual void CollisionPointVirtualSlot1();

    // Not vtable slots: table-management helpers in CollisionPoint.cpp.
    // 0x0043a640 (thiscall, ret 0x14).  Recomputes the relative position (relativePosition) and the
    // tangent direction/magnitude pair (frictionDirection/spinSpeed) from the contact normal
    // surfaceNormal (tier 3 semantics, see CollisionPoint.cpp).
    void UpdateRelativeMotion(const CollisionVec3* a1, const CollisionVec3* a2, const CollisionVec3* a3,
                     const CollisionVec3* a4, float a5);

    CollisionContactOwner* ownerNode;       // +0x04 owner; may be null (set by AddCollisionPoint)
    CollisionVec3 localPosition;  // +0x08 AddCollisionPoint stores its position argument here; 0x0043ad80 converts it with ownerNode->LocalToWorldPoint
    CollisionVec3 worldPosition;  // +0x14 result of the LocalToWorldPoint in 0x0043ad80 (or offset - k*normal for a single contact); UpdateRelativeMotion computes relativePosition = this - a1
    CollisionVec3 surfacePosition;  // +0x20 copy of worldPosition passed in/out to the ground query 0x00507c10 which snaps it to the surface; penetration uses (surfacePosition.y - worldPosition.y)
    CollisionVec3 surfaceNormal;  // +0x2c normal output of the ground query 0x00507c10 (2nd argument); projection axis in UpdateRelativeMotion, summed in 0x0043aa30
    CollisionVec3 relativePosition;  // +0x38 UpdateRelativeMotion: = worldPosition - a1 (position relative to the body); Tire computes the same
    CollisionVec3 field_0x44;
    CollisionVec3 frictionDirection;                // +0x50 scaled by frictionMagnitude into frictionForce (slot 1)
    CollisionVec3 field_0x5c;
    CollisionVec3 field_0x68;
    float frictionCoefficient;  // +0x74 slot 1: frictionMagnitude = -normalForce * surfaceGrip * frictionCoefficient; Tire overrides the same expression with its own coefficients
    CollisionVec3 frictionForce;  // +0x78 slot 1: = frictionDirection * frictionMagnitude (zero when both slip terms are below 0.001)
    float frictionMagnitude;  // +0x84 slot 1: = -normalForce*surfaceGrip*coefficient, then made non-negative
    float normalForce;  // +0x88 inlined ctor argument a (AddCollisionPoint a6); slot 1 negates it, so it holds a negative load
    float surfaceGrip;  // +0x8c 0x0043ad80: = surface table[surfaceType] (table at owner+0xa4, +0x3a0) or 1.0f without an owner; slot 1 multiplies it in
    float tangentSpeed;  // +0x90 UpdateRelativeMotion: dot(tangent, a3), the slip component along the surface; slot 1 tests it against 0.001
    float spinSpeed;  // +0x94 UpdateRelativeMotion: |cross(n*(n.a2), a4)|, contact speed from rotation about the normal; slot 1 tests it against 0.001
    float penetration;  // +0x98 0x0043ad80: = (surfacePosition.y - worldPosition.y) * surfaceNormal.y; AddCollisionPoint initialises it to threshold - 999
    float penetrationThreshold;  // +0x9c AddCollisionPoint a4; 0x0043ad80: inContact = (penetration >= penetrationThreshold)
    float field_0xa0;
    float inContact;  // +0xa4 0x0043ad80 sets 1.0f/0.0f from the penetration test and counts it; 0x0043aa30 only merges points with it set
    float field_0xa8;
    float field_0xac;
    float field_0xb0;
    float field_0xb4;
    float field_0xb8;
    char surfaceType;  // +0xbc low three bits of the flag byte written by the ground query (&= 7 in 0x0043ad80); indexes the per-surface grip table
    int surfaceOwner;  // +0xc0 inlined ctor argument b (AddCollisionPoint a7); 0x0043ad80 reads the surface table from [this+0xa4]+0x3a0 when nonzero
};

// Vector helpers used by the collision code (see CollisionVectorHelpers.cpp / CollisionPoint.cpp).
CollisionVec3* CollisionRejectFrom(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b); // 0x0043b190
float CollisionLength(const CollisionVec3* v);                                     // 0x00435ec0
CollisionVec3* CollisionDivide(CollisionVec3* out, const CollisionVec3* v, float len); // 0x0043c890
float Vec3DotCall(const CollisionVec3* a, const CollisionVec3* b);                 // dot product, cdecl
CollisionVec3* Vec3Normalize(CollisionVec3* out, const CollisionVec3* src);          // normalize (out = src / |src|), cdecl

// 0x0043a330 (cdecl).  Allocates a CollisionPoint into points[*count] if *count < capacity.
CollisionPoint* AddCollisionPoint(int capacity, CollisionPoint** points, const CollisionVec3* position,
                                  CollisionContactOwner* owner, float a4, int* count, float a6,
                                  int a7);

#endif
