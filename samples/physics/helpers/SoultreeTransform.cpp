// SoultreeTransform.cpp -- SoultreeObject (soultree.cpp) transform helpers declared in
// ../common/SoultreeObject.h. Class attribution is tier 2 (see that header); all
// method names are tier 3.
#include "../common/Math3D.h"

// 0x004fd5c0. v * R(world): the upper 3x3 of the world matrix, no translation.
Vec3 SoultreeObject::LocalToWorldDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Vec3 r;
    r.x = v.y * worldMatrix._21;
    r.x += v.z * worldMatrix._31;
    r.x += v.x * worldMatrix._11;
    r.y = v.y * worldMatrix._22;
    r.y += v.x * worldMatrix._12;
    r.y += v.z * worldMatrix._32;
    r.z = v.y * worldMatrix._23;
    r.z += v.x * worldMatrix._13;
    r.z += v.z * worldMatrix._33;
    return r;
}

// 0x004fd660. p * worldMatrix with the translation row added (same term order as above).
Vec3 SoultreeObject::LocalToWorldPoint(const Vec3& p)
{
    UpdateWorldMatrix();
    Vec3 r;
    r.x = p.y * worldMatrix._21;
    r.x += p.z * worldMatrix._31;
    r.x += p.x * worldMatrix._11;
    r.x += worldMatrix._41;
    r.y = p.y * worldMatrix._22;
    r.y += p.x * worldMatrix._12;
    r.y += p.z * worldMatrix._32;
    r.y += worldMatrix._42;
    r.z = p.y * worldMatrix._23;
    r.z += p.x * worldMatrix._13;
    r.z += p.z * worldMatrix._33;
    r.z = r.z + worldMatrix._43;
    return r;
}

// Swap across the diagonal of the upper 3x3 (retail does this in place on a stack copy).
static inline void TransposeRotation(Matrix4& m)
{
    float t;
    t = m._12; m._12 = m._21; m._21 = t;
    t = m._13; m._13 = m._31; m._31 = t;
    t = m._23; m._23 = m._32; m._32 = t;
}

// v * (upper 3x3 of m), with the per-component term order that reproduces retail's x87
// load order (x: x,z,y). Same math as RotateVector in Math3D.h.
static inline Vec3 RotateVectorRetail(Vec3 v, const Matrix4& m)
{
    Vec3 r;
    r.x = v.x * m._11 + v.z * m._31 + v.y * m._21;
    r.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    r.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    return r;
}

// 0x004fd710. v * transpose(R(world)): copies worldMatrix, transposes the copy, rotates.
// PARTIAL (99.02%): one commuted fmul operand pair differs (fld v.x; fmul m11).
Vec3 SoultreeObject::WorldToLocalDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    TransposeRotation(m);
    return RotateVectorRetail(v, m);
}

// 0x004fd7f0. PARTIAL (~88% instruction similarity; x87 term order of the y/z components
// and the negation temps differ). p * inverse(worldMatrix) for a rigid transform: rotate by the transposed
// 3x3 and add -(t * R^T), where t is the world translation.
Vec3 SoultreeObject::WorldToLocalPoint(const Vec3& p)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    TransposeRotation(m);
    Vec3 inv;
    inv.x = -(m._11 * m._41 + m._21 * m._42 + m._31 * m._43);
    inv.y = -(m._12 * m._41 + m._22 * m._42 + m._32 * m._43);
    inv.z = -(m._13 * m._41 + m._23 * m._42 + m._33 * m._43);
    Vec3 r;
    r.x = p.x * m._11 + p.y * m._21 + p.z * m._31 + inv.x;
    r.y = p.x * m._12 + p.y * m._22 + p.z * m._32 + inv.y;
    r.z = p.x * m._13 + p.y * m._23 + p.z * m._33 + inv.z;
    return r;
}

// 0x004fc630. Sets the local translation (localMatrix row 3) and invalidates the world
// matrix. __thiscall, ret 0xc.
void SoultreeObject::SetPosition(float x, float y, float z)
{
    localMatrix._41 = x;
    localMatrix._42 = y;
    localMatrix._43 = z;
    InvalidateWorldMatrix();
}

// 0x004fc660. Same, from a vector.
void SoultreeObject::SetPosition(const Vec3& p)
{
    localMatrix._41 = p.x;
    localMatrix._42 = p.y;
    localMatrix._43 = p.z;
    InvalidateWorldMatrix();
}

// 0x004fc970. Returns the local translation.
Vec3 SoultreeObject::GetPosition()
{
    return *(Vec3*)&localMatrix._41;
}

// 0x004fc9a0. Position expressed in 'frame' space: the local translation when frame == this,
// otherwise the world translation, converted through frame->WorldToLocalPoint when frame is
// non-null. __thiscall, ret 8.
void SoultreeObject::GetPositionIn(SoultreeObject* frame, Vec3* out)
{
    if (frame == this) {
        out->x = localMatrix._41;
        out->y = localMatrix._42;
        out->z = localMatrix._43;
        return;
    }
    UpdateWorldMatrix();
    out->x = worldMatrix._41;
    out->y = worldMatrix._42;
    out->z = worldMatrix._43;
    if (frame)
        *out = frame->WorldToLocalPoint(*out);
}

// 0x004fc540. World axes (rows 2 and 1 of the world matrix) in 'frame' space; (0,0,1) and
// (0,1,0) when frame == this. Either output may be null.
void SoultreeObject::GetAxesIn(SoultreeObject* frame, Vec3* axisZ, Vec3* axisY)
{
    if (frame == this) {
        if (axisZ) {
            axisZ->x = 0.0f;
            axisZ->y = 0.0f;
            axisZ->z = 1.0f;
        }
        if (axisY) {
            axisY->x = 0.0f;
            axisY->y = 1.0f;
            axisY->z = 0.0f;
        }
        return;
    }
    UpdateWorldMatrix();
    if (axisZ) {
        axisZ->x = worldMatrix._31;
        axisZ->y = worldMatrix._32;
        axisZ->z = worldMatrix._33;
    }
    if (axisY) {
        axisY->x = worldMatrix._21;
        axisY->y = worldMatrix._22;
        axisY->z = worldMatrix._23;
    }
    if (frame) {
        if (axisZ)
            *axisZ = frame->WorldToLocalDirection(*axisZ);
        if (axisY)
            *axisY = frame->WorldToLocalDirection(*axisY);
    }
}
