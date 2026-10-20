// SoultreeTransform.cpp -- SoultreeObject (soultree.cpp) WorldToLocalDirection, the near
// miss of the unit's point/direction transforms (LocalToWorldPoint 0x004fd660 and
// WorldToLocalPoint 0x004fd7f0 are exact in src/krusty2/soultree/soultree.cpp). Class
// attribution is tier 2 (see core/SoultreeObject.h); all method and helper names are tier 3.
#include "math/Math3D.h"

// Swap across the diagonal of the upper 3x3 (retail does this in place on a stack copy).
// The self-assignment of m._11 emits nothing. It is the store that makes m._11 a newer
// leaf than v.x (VC6_OPERAND_ORDER.md 1.2), so the last x product loads v.x first, as
// retail does. Without it, m._11 is the only element of that product the transpose
// never writes, and VC6 loads it first.
static inline void TransposeRotation(Matrix4& m)
{
    float t;
    t = m._12; m._12 = m._21; m._21 = t;
    t = m._13; m._13 = m._31; m._31 = t;
    t = m._23; m._23 = m._32; m._32 = t;
    m._11 = m._11;
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
// Exact (209/209). Before the m._11 store in TransposeRotation it was 207/209: retail
// loads v.x before m._11 in the last x product. These forms did not move that operand:
// all 768 term orders, operand orders, bracketings and parenthesised products of the x
// row; per-statement accumulation of the x row in all six orders, into r.x or a local;
// an embedded store `(t = v.x)`; the 48 swap orders/directions of the transpose; 4608 y/z
// term orders; reference and pointer aliases of v and m; 0..33 dead locals; and a loop,
// swap-helper (diagonal included), full-copy or three-statement diagonal swap transpose.
Vec3 SoultreeObject::WorldToLocalDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    TransposeRotation(m);
    return RotateVectorRetail(v, m);
}
