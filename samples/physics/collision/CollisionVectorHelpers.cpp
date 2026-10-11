// Small vector helpers reached by direct call from the collision code (tier 3 names).
// Their translation unit is unknown: they sit between the CollisionPoint.cpp and cube.cpp
// __FILE__ references, outside CollisionObject.cpp (src/krusty2/collision).  The reject
// helper 0x0043b190, inside CollisionPoint.cpp's extent, is in src/krusty2/collision/CollisionPoint.cpp.
#include "collision/CollisionTypes.h"
#include "math/Math3D.h"   // FastSqrt (0x00460b50)

// 0x0043c890 (cdecl): *out = *v * (1 / len); returns out.
CollisionVec3* CollisionDivide(CollisionVec3* out, const CollisionVec3* v, float len)
{
    *out = *v * (1.0f / len);
    return out;
}

// 0x0043ca20 (cdecl): out = v * M3x3 (rotate by the matrix rows, no translation), same result as 0x0042a450.
// It sits in the 0x0043c8xx..0x0043cax run (not CollisionObject/CollisionPoint code); tier 3 name.
// Partial (targets.json); 0..33 dead initialised int locals at the top of the function change nothing.
void CollisionRotateVector(CollisionVec3* out, const CollisionVec3* v, const Matrix4* m)
{
    *(Vec3*)out = Vec3(v->z * m->_31 + v->y * m->_21 + v->x * m->_11,
                       v->z * m->_32 + v->y * m->_22 + v->x * m->_12,
                       v->z * m->_33 + v->y * m->_23 + v->x * m->_13);
}

// 0x0043ce90 (cdecl): pre-increments a global counter at 0x0057985c and returns the new value
// (an id generator; callers unknown).
// owner: bracket only.
int g_CollisionCounter;
int NextCollisionCounter()
{
    return ++g_CollisionCounter;
}
