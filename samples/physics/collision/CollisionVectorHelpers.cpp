// Small vector helpers reached by direct call from the collision code (tier 3 names).
// Their true translation unit is unknown: 0x0043b190 and 0x0043c890 sit inside the
// CollisionPoint/ConstraintMethodCollisionModel range, 0x00435ec0 inside CollisionObject.
#include "collision/CollisionTypes.h"
#include "../common/Math3D.h"   // FastSqrt (0x00460b50)

// 0x0043b190 (cdecl): out = a - b * (a . b), the part of a perpendicular to the (unit) vector b.
// Returns 0 when the result is exactly the zero vector, else out.
CollisionVec3* CollisionRejectFrom(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b)
{
    float d = CollisionDot(*b, *a);
    CollisionVec3 t = *b * d;
    *out = *a - t;
    if (out->x == 0.0f && out->y == 0.0f && out->z == 0.0f)
        return 0;
    return out;
}

// 0x00435ec0 (cdecl): vector length; returns 1.0f without a square root when |v|^2 == 1.
float CollisionLength(const CollisionVec3* v)
{
    // Written out rather than CollisionDot(*v, *v): the inline form schedules z*z first.
    float s = (v->x * v->x + v->y * v->y) + v->z * v->z;
    return s == 1.0f ? 1.0f : FastSqrt(s);
}

// 0x0043c890 (cdecl): *out = *v * (1 / len); returns out.
CollisionVec3* CollisionDivide(CollisionVec3* out, const CollisionVec3* v, float len)
{
    *out = *v * (1.0f / len);
    return out;
}

// 0x0043ca20 (cdecl): out = v * M3x3 (rotate by the matrix rows, no translation), same result as 0x0042a450.
// It sits in the 0x0043c8xx..0x0043cax run (not CollisionObject/CollisionPoint code); tier 3 name.
void CollisionRotateVector(CollisionVec3* out, const CollisionVec3* v, const Matrix4* m)
{
    *(Vec3*)out = Vec3(v->z * m->_31 + v->y * m->_21 + v->x * m->_11,
                       v->z * m->_32 + v->y * m->_22 + v->x * m->_12,
                       v->z * m->_33 + v->y * m->_23 + v->x * m->_13);
}
