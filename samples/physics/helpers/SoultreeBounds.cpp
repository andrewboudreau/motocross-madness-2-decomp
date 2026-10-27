// SoultreeBounds.cpp -- near misses of two SoultreeObject (soultree.cpp) methods that had no
// source: Scale 0x004fd340 and GetWorldBounds 0x004fe8a0. The
// exact members of the unit are in src/krusty2/soultree/soultree.cpp. Class attribution tier 2
// (thiscall on SoultreeObject fields, inside the unit's extent); names tier 3.
//
// Scale (0x004fd340, 630 bytes; 618/630): localMatrix = diag(x, y, z, 1) * localMatrix, written
//   in place from a copy, leaving the local translation (+0xe8..+0xf0) untouched (retail stores
//   13 of the 16 elements), then InvalidateWorldMatrix. Prologue, frame, the copy through an
//   inline helper's by-value parameter and rows 2-3 match. Writing _11.._13 as k = 1,3,2,4 and
//   _14 as k = 3,1,2,4 (found by a greedy per-row search of all 24 term orders) lifts it from
//   596 to 618; what remains is operand order inside the s13 product of _11.._14 (retail loads
//   m3j first, then fmul s13) and two _44 products. Factor order, parameter order, `const&`
//   for either operand, dropping the `local` pointer and regrouping (four groupings) do not
//   move the s13 operands. Dead-local counts change the term order mod 8 (the count in Scale
//   and in ScaleProduct add up): seven extra locals reach 620/630 with _14 back in written
//   order, still with the s13 operand order. Earlier forms: a full 16-element product with
//   the translation restored, a temporary result, SetIdentity or reversed diagonal stores and
//   accumulation (`t += ...`); with both operands by value (as MatrixMultiply 0x0042a1a0 takes
//   them; VC6 reads the unmodified `s` in place).
//   Rechecked 2026-10-11: the count windows repeat every 32 locals, not 8 (n and n + 32
//   give the same code; windows at 6-7 (_44), 9-10 (row 3), 12-13 (row 2), 15 (row 1));
//   with this form every count 0..31 stays at 618, and the written k order with 0..31
//   locals is no better. Locals of any type count one each; uninitialised unused ones
//   and extra statements count nothing.
// GetWorldBounds (0x004fe8a0, 618 bytes; 606/618): the subtree box through the world matrix,
//   the half extents through |R| (the diagonal half-extent axes times R, inline fabs).
//   Session 16: three Math3D Vec3 axis locals (zero stores dead) give retail's frame
//   (c, axes, w, a), the centre w built by the Vec3 constructor makes VC6 consume w in the
//   hi adds and pop ext at the end as retail does, and naming the nine |a| terms as float
//   locals removes the fxch pairs the plain sums get (551/610 -> 606/618, full length).
//   Left: pop esi is scheduled after the first fcom/fnstsw/test instead of before the
//   a[2][2] store. Centre row groupings, the c copy form, product operand order, ext
//   declaration placement, AbsValue spellings and 0..11 dead locals do not move it.
//   Earlier forms: a reinterpret_cast Vec3 row store of the diagonal (same frame), six
//   ext sum orders, w/ext operand orders, Vec3 operators, inline Set helpers and += forms
//   for hi, and storing hi before lo (rejected: retail stores lo first).
#include <string.h>
#include "math/Math3D.h"

#include "SoultreeScaleProduct.h"

void SoultreeObject::Scale(float x, float y, float z)
{
    Matrix4* local = &localMatrix;
    Matrix4 s;
    memset(&s, 0, sizeof(Matrix4));
    s._11 = x;
    s._22 = y;
    s._33 = z;
    s._44 = 1.0f;
    ScaleProduct(local, *local, s);
    InvalidateWorldMatrix();
}

static inline float AbsValue(float v)
{
    return v < 0.0f ? -v : v;
}

void SoultreeObject::GetWorldBounds(Vec3* lo, Vec3* hi)
{
    if (subtreeDirty)
        UpdateSubtreeBounds();
    Vec3 axisX(subtreeBoundsB.x, 0.0f, 0.0f);
    Vec3 axisY(0.0f, subtreeBoundsB.y, 0.0f);
    Vec3 axisZ(0.0f, 0.0f, subtreeBoundsB.z);
    UpdateWorldMatrix();
    Vec3 c = subtreeBoundsA;
    Vec3 w(c.x * worldMatrix._11 + (c.y * worldMatrix._21 + c.z * worldMatrix._31) + worldMatrix._41,
           c.x * worldMatrix._12 + (c.y * worldMatrix._22 + c.z * worldMatrix._32) + worldMatrix._42,
           c.x * worldMatrix._13 + (c.y * worldMatrix._23 + c.z * worldMatrix._33) + worldMatrix._43);
    float a[3][3];
    a[0][0] = axisX.x * worldMatrix._11;
    a[0][1] = axisX.x * worldMatrix._12;
    a[0][2] = axisX.x * worldMatrix._13;
    a[1][0] = axisY.y * worldMatrix._21;
    a[1][1] = axisY.y * worldMatrix._22;
    a[1][2] = axisY.y * worldMatrix._23;
    a[2][0] = axisZ.z * worldMatrix._31;
    a[2][1] = axisZ.z * worldMatrix._32;
    a[2][2] = axisZ.z * worldMatrix._33;
    Vec3 ext;
    float x0 = AbsValue(a[0][0]);
    float x1 = AbsValue(a[1][0]);
    float x2 = AbsValue(a[2][0]);
    ext.x = x0 + x1 + x2;
    float y0 = AbsValue(a[0][1]);
    float y1 = AbsValue(a[1][1]);
    float y2 = AbsValue(a[2][1]);
    ext.y = y0 + y1 + y2;
    float z0 = AbsValue(a[0][2]);
    float z1 = AbsValue(a[1][2]);
    float z2 = AbsValue(a[2][2]);
    ext.z = z0 + z1 + z2;
    lo->x = w.x - ext.x;
    lo->y = w.y - ext.y;
    lo->z = w.z - ext.z;
    hi->x = ext.x + w.x;
    hi->y = ext.y + w.y;
    hi->z = ext.z + w.z;
}
