// SoultreeTransform.cpp -- near misses of SoultreeObject (soultree.cpp) point/direction
// transforms. The exact members of the unit are in src/krusty2/soultree/soultree.cpp; this
// file keeps the three that do not match yet. Class attribution is tier 2 (see
// core/SoultreeObject.h); all method names are tier 3.
#include "math/Math3D.h"

// 0x004fd660. p * worldMatrix with the translation row added (same term order as
// LocalToWorldDirection 0x004fd5c0, which is exact). PARTIAL 164/168 bytes: one integer
// 'mov ecx,eax' is scheduled before the last faddp in retail.
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
    r.z += worldMatrix._43;
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
// PARTIAL 207/209 bytes: retail loads v.x before m._11 in the last x product; none of the
// tested term orders moves that operand.
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
