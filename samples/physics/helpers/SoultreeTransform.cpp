// SoultreeTransform.cpp -- SoultreeObject (soultree.cpp) point/direction transforms that
// are not in src/krusty2/soultree/soultree.cpp: LocalToWorldPoint (exact here) and the two
// near misses. Class attribution is tier 2 (see core/SoultreeObject.h); all method and
// helper names are tier 3.
#include "math/Math3D.h"

// 0x004fd660. p * worldMatrix with the translation row added (same term order as
// LocalToWorldDirection 0x004fd5c0, which is exact). EXACT. x and y are accumulated
// statement by statement like 0x004fd5c0; z is one expression whose first two products
// are parenthesised (docs/VC6_OPERAND_ORDER.md section 3), which issues retail's
// 'mov ecx,eax' (the result pointer) before the last faddp of z. Written with z as
// statements, every one of the 4096 combinations of parenthesised products and
// translation terms keeps the faddp first (164/168 bytes, 160/164 masked). The other
// exact form found is z as one unparenthesised expression with the first x statement
// written 'r.x = (p.y * worldMatrix._21);'. All three rows as single expressions score
// at most 156/164.
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
    r.z = (p.y * worldMatrix._23) + (p.x * worldMatrix._13) + p.z * worldMatrix._33 + worldMatrix._43;
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
// Also unchanged: reference and pointer aliases of v and m with single and pairwise
// flips of all 18 reads (171 variants each), and 0..33 dead initialised int locals at the top of
// the function (207 except 16-17: 40, 18-19: 203).
// PARTIAL 207/209 bytes (203/205 masked): retail loads v.x before m._11 in the last x
// product (m._11 is the only element of the product the transpose does not touch). No
// form moves that operand: all 768 term orders, operand orders, bracketings and
// parenthesised products of the x row; the 48 swap orders/directions of the transpose;
// 4608 y/z term orders; and, each with 48 to 768 x forms, the rotate helper returning a
// constructor, taking a reference and copying it, a pointer, a non-const matrix, an
// out pointer (void or returning it), the transpose through a pointer, m.m[i][j], a Swap
// helper, block-scoped or three temporaries, the matrix by value into a helper that
// transposes it (both parameter orders, one or two helper levels), and the copy and
// transpose written inline. The constructor and copy-in-helper forms reorder y (199).
Vec3 SoultreeObject::WorldToLocalDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    TransposeRotation(m);
    return RotateVectorRetail(v, m);
}

// In-place inverse of a rigid transform: transpose the rotation, then the translation
// becomes -(t * R^T). Stored through a Vec3 temporary into the translation row.
static inline void InvertRigid(Matrix4& m)
{
    TransposeRotation(m);
    Vec3 t;
    t.x = -(m._41 * m._11 + m._42 * m._21 + m._43 * m._31);
    t.y = -(m._41 * m._12 + m._42 * m._22 + m._43 * m._32);
    t.z = -(m._41 * m._13 + m._42 * m._23 + m._43 * m._33);
    m._41 = t.x;
    m._42 = t.y;
    m._43 = t.z;
}

// p * m with the translation row added.
static inline Vec3 TransformPoint(const Vec3& p, const Matrix4& m)
{
    Vec3 r;
    r.x = p.x * m._11 + p.y * m._21 + p.z * m._31 + m._41;
    r.y = p.x * m._12 + p.y * m._22 + p.z * m._32 + m._42;
    r.z = p.x * m._13 + p.y * m._23 + p.z * m._33 + m._43;
    return r;
}

// 0x004fd7f0. p * inverse(worldMatrix) for a rigid transform: copy worldMatrix, invert the
// copy in place, transform the point. PARTIAL 273/285 bytes (269/281 masked): in retail
// (0..33 dead initialised int locals at the top of the function or inside InvertRigid change nothing;
// all six written orders of the t.y sum give the same code.)
// the second swap's integer moves come after the first's and the y translation sums
// _22*_42, _12*_41, then _32*_43; the candidate sums _22*_42, _32*_43, _12*_41.
// Writing t.y as '((m._41 * m._12) + (m._42 * m._22)) + (m._43 * m._32)' fixes both but
// sinks the last fadd/fstp of r.z after the result copy (255/281), and no single or
// pairwise change of the other five rows (term order, bracketing, parenthesised products,
// translation position), statement or store order, temporaries, return form or
// declaration count (0..63 typedefs) undoes that. The earlier form (rotate the point by
// the transposed copy and add a separate -(t * R^T) vector) scored 259/281; a Vec3
// constructor or float temporaries in InvertRigid change the frame.
Vec3 SoultreeObject::WorldToLocalPoint(const Vec3& p)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    InvertRigid(m);
    return TransformPoint(p, m);
}
