// SoultreeRotate.cpp -- SoultreeObject (soultree.cpp) pivot rotation. Class attribution tier 2;
// names tier 3.
#include "../common/Math3D.h"

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
    Vec3 n = -pivot;
    p.x = n.z * localMatrix._31;
    p.x += n.y * localMatrix._21;
    p.x += n.x * localMatrix._11;
    p.x += localMatrix._41;
    p.y = n.z * localMatrix._32;
    p.y += n.y * localMatrix._22;
    p.y += n.x * localMatrix._12;
    p.y += localMatrix._42;
    p.z = n.z * localMatrix._33;
    p.z += n.y * localMatrix._23;
    p.z += n.x * localMatrix._13;
    p.z += localMatrix._43;
    SetPosition(p);
}
