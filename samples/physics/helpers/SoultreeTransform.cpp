// SoultreeTransform.cpp -- SoultreeObject (soultree.cpp) WorldToLocalDirection, the near
// miss of the unit's point/direction transforms (LocalToWorldPoint 0x004fd660 and
// WorldToLocalPoint 0x004fd7f0 are exact in src/krusty2/soultree/soultree.cpp). Class
// attribution is tier 2 (see core/SoultreeObject.h); all method and helper names are tier 3.
#include "math/Math3D.h"

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
// Session 14: transposes that also write the diagonal (a j <= i loop, a swap helper
// with a diagonal self-swap, a full copy assigned back, a nine-entry transpose into a
// destination matrix, a by-value Matrix3 Transposed()) change the code shape (50+ lines).
// Only a no-op store to m._11 (a non-source probe, not kept) gives 209/209.
Vec3 SoultreeObject::WorldToLocalDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    TransposeRotation(m);
    return RotateVectorRetail(v, m);
}
