// CollisionObjectGlobals.cpp -- small free functions in the CollisionObject.cpp / constraint
// link-order range that touch a single global.  Wave 5.
#include "CollisionShapeTests.h"

// 0x00439e00 (cdecl): installs the scratch result record the box tests write into
// (g_CollisionBoxResult, 0x00579058, read by HullVsModel and friends).
// owner: bracket only (between the 0x00439xxx shape tests and the __FILE__ xrefs of CollisionObject.cpp).
void SetCollisionBoxResult(CollisionBoxResult* result)
{
    g_CollisionBoxResult = result;
}

// 0x0043ce90 (cdecl): pre-increments a global counter at 0x0057985c and returns the new value
// (an id generator; callers unknown).
// owner: bracket only.
int g_CollisionCounter;
int NextCollisionCounter()
{
    return ++g_CollisionCounter;
}

// Vec3 with an inline constructor (retail builds the result with the arguments evaluated
// z, y, x and stored x, y, z, so the values stay on the x87 stack until the stores).
struct CollisionInlineVec3 {
    float x, y, z;
    CollisionInlineVec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};


// 0x00435610 (cdecl, returns the vector through the hidden first argument): componentwise
// minimum of a and b.  Called from the bounds code at 0x00435499 (inside SetTransform) with
// the result fed to the Vec3 operator+.
// owner: bracket only.
CollisionInlineVec3 CollisionVec3Min(const CollisionVec3& a, const CollisionVec3& b)
{
    return CollisionInlineVec3(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);
}

// 0x00435680: componentwise maximum (retail's test ah,0x41 form is `a > b`).
// owner: bracket only.
CollisionInlineVec3 CollisionVec3Max(const CollisionVec3& a, const CollisionVec3& b)
{
    return CollisionInlineVec3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
}
