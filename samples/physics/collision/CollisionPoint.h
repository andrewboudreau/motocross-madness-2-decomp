// CollisionPoint -- reconstructed layout (CollisionPoint.cpp).
// Evidence: RTTI CollisionPoint, no bases, vtable 0x005511d8 (2 slots: 0 = deleting
// destructor 0x0043b300, 1 = 0x0043b240).  Size 0xc4 comes from the allocation
// in 0x0043a330 (operator new(0xc4, __FILE__, 27)).  Member names are provisional.
#ifndef COLLISION_POINT_H
#define COLLISION_POINT_H

#include "CollisionTypes.h"

// The object a contact point belongs to (CollisionPoint::field_0x04).  Only one method
// is used: 0x004fd660 (thiscall, ret 8), which the scene node class (SoultreeObject)
// also has, so this is a provisional view of that node.  Tier 3 name.
class CollisionContactOwner {
public:
    // Returns a pointer to a temporary vector computed from the owner and a point.
    CollisionVec3* Fn_004fd660(CollisionVec3* tmp, const CollisionVec3* p);
};

class CollisionPoint {
public:
    CollisionPoint(float a, int b);
    virtual ~CollisionPoint() {}               // slot 0 (deleting destructor 0x0043b300; core inlined)
    // slot 1 (0x0043b240).  Named apart from QuadTreeObject::UnknownVirtualSlot1 so that
    // Tire, which derives from both, can override this one alone.
    virtual void CollisionPointVirtualSlot1();

    // Not vtable slots: table-management helpers in CollisionPoint.cpp.
    // 0x0043a640 (thiscall, ret 0x14).  Recomputes the relative position (field_0x38) and the
    // tangent direction/magnitude pair (field_0x50/field_0x94) from the contact normal
    // field_0x2c (tier 3 semantics, see CollisionPoint.cpp).
    void Fn_0043a640(const CollisionVec3* a1, const CollisionVec3* a2, const CollisionVec3* a3,
                     const CollisionVec3* a4, float a5);

    CollisionContactOwner* field_0x04;       // owner; may be null (set by AddCollisionPoint)
    CollisionVec3 field_0x08;
    CollisionVec3 field_0x14;
    CollisionVec3 field_0x20;
    CollisionVec3 field_0x2c;
    CollisionVec3 field_0x38;
    CollisionVec3 field_0x44;
    CollisionVec3 field_0x50;                // scaled by field_0x84 into field_0x78 (slot 1)
    CollisionVec3 field_0x5c;
    CollisionVec3 field_0x68;
    float field_0x74;
    CollisionVec3 field_0x78;
    float field_0x84;
    float field_0x88;
    float field_0x8c;
    float field_0x90;
    float field_0x94;
    float field_0x98;
    float field_0x9c;
    float field_0xa0;
    float field_0xa4;
    float field_0xa8;
    float field_0xac;
    float field_0xb0;
    float field_0xb4;
    float field_0xb8;
    char field_0xbc;
    int field_0xc0;
};

// Vector helpers used by the collision code (see CollisionVectorHelpers.cpp / CollisionPoint.cpp).
CollisionVec3* CollisionRejectFrom(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b); // 0x0043b190
float CollisionLength(const CollisionVec3* v);                                     // 0x00435ec0
CollisionVec3* CollisionDivide(CollisionVec3* out, const CollisionVec3* v, float len); // 0x0043c890
float Fn_0040ae30(const CollisionVec3* a, const CollisionVec3* b);                 // dot product, cdecl
CollisionVec3* Fn_005087b0(CollisionVec3* out, const CollisionVec3* src);          // normalize (out = src / |src|), cdecl

// 0x0043a330 (cdecl).  Allocates a CollisionPoint into points[*count] if *count < capacity.
CollisionPoint* AddCollisionPoint(int capacity, CollisionPoint** points, const CollisionVec3* position,
                                  CollisionContactOwner* owner, float a4, int* count, float a6,
                                  int a7);

#endif
