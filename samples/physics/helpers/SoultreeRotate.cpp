// SoultreeRotate.cpp -- near miss of SoultreeObject (soultree.cpp) pivot rotation. Rotate
// (0x004fceb0), RotateAbout and SetRotation are exact in src/krusty2/soultree/soultree.cpp.
// Class attribution tier 2; names tier 3.
#include "math/Math3D.h"

// 0x004fd1f0. Semantics (tier 3, from the arithmetic): the point 'pivot' in local space is
// converted to the parent's space (p = pivot * local, translation included) and stored as the
// position; the node is then rotated by (axis, angle) via 0x004fceb0; finally the position is
// set to p - pivot * R(new local), i.e. translation + (-pivot) * rotation, so that the pivot
// keeps its place after the rotation.
void SoultreeObject::RotateAboutPoint(Vec3 pivot, Vec3 axis, float angle)
{
    Vec3 p;
    p.x = pivot.y * localMatrix._21;
    p.x += pivot.z * localMatrix._31;
    p.x += pivot.x * localMatrix._11;
    p.x += localMatrix._41;
    p.y = pivot.y * localMatrix._22;
    p.y += pivot.z * localMatrix._32;
    p.y += pivot.x * localMatrix._12;
    p.y += localMatrix._42;
    p.z = pivot.y * localMatrix._23;
    p.z += pivot.z * localMatrix._33;
    p.z += pivot.x * localMatrix._13;
    p.z += localMatrix._43;
    SetPosition(p);
    Rotate(axis, angle);
    // Retail builds both positions directly in the pushed Vec3 temporary (the address is
    // pushed before the first block is computed). Writing the blocks as Vec3(x, y, z)
    // constructions reproduces that and the -pivot block exactly, but not the first block's
    // x87 term order (y*_21, z*_31, x*_11), and scores lower overall (85.4%).
    float nx = -pivot.x, ny = -pivot.y, nz = -pivot.z;
    p.x = nz * localMatrix._31 + ny * localMatrix._21 + nx * localMatrix._11 + localMatrix._41;
    p.y = nz * localMatrix._32 + ny * localMatrix._22 + nx * localMatrix._12 + localMatrix._42;
    p.z = nz * localMatrix._33 + ny * localMatrix._23 + nx * localMatrix._13 + localMatrix._43;
    SetPosition(p);
}
