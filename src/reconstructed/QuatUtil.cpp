// The quaternion helpers after MatrixUtil (0x004a1d10..0x004a2350). The
// unit ends with its own copy of the four-vector constant set (XCU entries
// 179-182, globals 0x00685070..0x006850ac), the same set MatrixUtil.cpp
// defines from the shared header. Nothing reads these copies. No __FILE__
// literal or RTTI reaches the range, so the file name is ours (tier 3).
// 0x004a1d10 and 0x004a1f90 are matched in samples/physics/common/Math3D.cpp.
// PhysicsRigidBody (0x004cc6c4, 0x004cc960, 0x004cc98f) calls the three below.

#include <math.h>
#include "MatrixUtil.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);   // 0x00685080, $E 0x004a2210
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);  // 0x00685090, $E 0x004a2260
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);  // 0x006850a0, $E 0x004a22b0
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);  // 0x00685070, $E 0x004a2300

// Scalar part first (krusty2 math/Math3D.h). It has no constructors: with
// them, QuatNormalize's result components land in other registers.
struct Quat {
    float w;
    float x;
    float y;
    float z;
};

extern Quat g_QuatIdentity;   // 0x006850b0

// 0x004a1d60: q / |q|, or the identity for a zero quaternion.
Quat QuatNormalize(const Quat& q)
{
    Quat result;
    float lengthSq = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    if (lengthSq == 0.0) {
        result = g_QuatIdentity;
    } else {
        float scale = 1.0f / (float)sqrt(lengthSq);
        result.w = q.w * scale;
        result.x = q.x * scale;
        result.y = q.y * scale;
        result.z = q.z * scale;
    }
    return result;
}

// 0x004a1e10: the rotation matrix of q (row vectors, D3D layout), scaled by
// 2 / |q|^2 so that q need not be normalized.
Matrix4 QuatToMatrix(const Quat& q)
{
    Matrix4 m;
    float lengthSq = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    float s = lengthSq > 0.0f ? 2.0f / lengthSq : 0.0f;
    float xs = q.x * s;
    float ys = q.y * s;
    float zs = q.z * s;
    float wx = q.w * xs;
    float wy = q.w * ys;
    float wz = q.w * zs;
    float xy = q.x * ys;
    float xz = q.x * zs;
    float yz = q.y * zs;
    float xx = q.x * xs;
    float yy = q.y * ys;
    float zz = q.z * zs;

    m(0, 0) = 1.0f - (yy + zz);
    m(0, 1) = xy + wz;
    m(0, 2) = xz - wy;
    m(0, 3) = 0.0f;
    m(1, 0) = xy - wz;
    m(1, 1) = 1.0f - (xx + zz);
    m(1, 2) = yz + wx;
    m(1, 3) = 0.0f;
    m(2, 0) = xz + wy;
    m(2, 1) = yz - wx;
    m(2, 2) = 1.0f - (xx + yy);
    m(2, 3) = 0.0f;
    m(3, 0) = 0.0f;
    m(3, 1) = 0.0f;
    m(3, 2) = 0.0f;
    m(3, 3) = 1.0f;
    return m;
}

// 0x004a2040: the unit quaternion of the rotation in m's upper 3x3, from
// the largest of the four squared components.
Quat QuatFromMatrix(const Matrix4& m)
{
    Quat q;
    float s;
    float ww = (m(0, 0) + m(1, 1) + m(2, 2) + 1.0f) * 0.25f;
    float xx = (m(0, 0) + 1.0f - m(1, 1) - m(2, 2)) * 0.25f;
    float yy = (1.0f - m(0, 0) + m(1, 1) - m(2, 2)) * 0.25f;
    float zz = (1.0f - m(0, 0) - m(1, 1) + m(2, 2)) * 0.25f;

    if (ww > xx && ww > yy && ww > zz) {
        q.w = (float)sqrt(ww);
        s = 0.25f / q.w;
        q.x = (m(1, 2) - m(2, 1)) * s;
        q.y = (m(2, 0) - m(0, 2)) * s;
        q.z = (m(0, 1) - m(1, 0)) * s;
    } else if (xx > yy && xx > zz) {
        q.x = (float)sqrt(xx);
        s = 0.25f / q.x;
        q.w = (m(1, 2) - m(2, 1)) * s;
        q.y = (m(1, 0) + m(0, 1)) * s;
        q.z = (m(2, 0) + m(0, 2)) * s;
    } else if (yy > zz) {
        q.y = (float)sqrt(yy);
        s = 0.25f / q.y;
        q.w = (m(2, 0) - m(0, 2)) * s;
        q.x = (m(1, 0) + m(0, 1)) * s;
        q.z = (m(2, 1) + m(1, 2)) * s;
    } else {
        q.z = (float)sqrt(zz);
        s = 0.25f / q.z;
        q.w = (m(0, 1) - m(1, 0)) * s;
        q.x = (m(2, 0) + m(0, 2)) * s;
        q.y = (m(2, 1) + m(1, 2)) * s;
    }
    return q;
}
