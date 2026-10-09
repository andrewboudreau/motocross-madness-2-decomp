// PROVISIONAL shared types for the collision translation units (tier 3).
// CollisionVec3 is kept apart from the Math3D Vec3 because these translation units call
// the Vec3 constructor out of line (0x00404e60); see the constructor comment below.
// Only the 12-byte {x,y,z} float layout is confirmed (three dword moves / fld+fmul at +0,+4,+8).
#ifndef COLLISION_TYPES_H
#define COLLISION_TYPES_H

#include "core/DebugAlloc.h"
#include "math/FastMath.h"

struct CollisionVec3 {
    float x, y, z;
    CollisionVec3() {}
    // Retail calls an out-of-line constructor (thiscall, ret 0xc) at 0x00404e60 from
    // 0x0043a640; the body is three dword stores.  Declared only, not defined here.
    CollisionVec3(float x_, float y_, float z_);
};

inline CollisionVec3 operator-(const CollisionVec3& a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

inline CollisionVec3 operator+(const CollisionVec3& a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

inline float CollisionDot(const CollisionVec3& a, const CollisionVec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Same component formula as CrossProduct in ../common/Math3D.h.
inline CollisionVec3 CollisionCross(const CollisionVec3& a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

inline CollisionVec3 operator*(const CollisionVec3& v, float s) {
    CollisionVec3 r;
    r.x = v.x * s;
    r.y = v.y * s;
    r.z = v.z * s;
    return r;
}

// Zero vector in .bss at 0x005797f0 (three dwords copied by every vector reset).
extern CollisionVec3 g_CollisionZeroVec3;
// Third global vec3 at 0x005797b0 (initial value of CollisionObject::field_0xa0/field_0xac).
extern CollisionVec3 g_CollisionVec3_5797b0;
// Second global vec3 at 0x00579810 (source of CollisionPoint::field_0x2c initial value; (0,1,0), stored by the initializer 0x0043b100).
extern CollisionVec3 g_CollisionVec3_579810;

// The debug operator new/delete (size or ptr, __FILE__, __LINE__) are in DebugAlloc.h.

// 4x4 float matrix, 16 dwords (rep movsd 0x10 in the shape setups); row-major guess.
struct CollisionMatrix4 {
    float m[16];
};

#endif
