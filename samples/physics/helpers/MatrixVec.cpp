// MatrixVec.cpp -- vector/matrix product helpers called from the broadphase/collision
// code (BoundingBoxTreeBuild.cpp range 0x00428xxx-0x0042axxx by proximity; tier 3).
#include "../common/Math3D.h"

// 0x0042a450. out = v * R(m): row-vector times the upper 3x3, no translation. cdecl, the Vec3
// is passed by value (3 floats on the stack, the caller copies it), 5 stack dwords.
void Vec3TransformNormal(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// 0x0042a4b0. out = R(m) * v: column-vector product, i.e. v * transpose(R).
void Vec3TransformNormalTranspose(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_12 + v.z * m->_13;
    out->y = v.x * m->_21 + v.y * m->_22 + v.z * m->_23;
    out->z = v.x * m->_31 + v.y * m->_32 + v.z * m->_33;
}

// 0x0042a510. out = v * m with the translation row (_41.._43) added.
void Vec3TransformPoint(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}
