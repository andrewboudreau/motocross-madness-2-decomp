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
// GetWorldBounds (0x004fe8a0, 618 bytes; 551/610): the subtree box through the world matrix,
//   the half extents through |R| (a diagonal matrix times R, inline fabs). The instruction
//   stream matches up to the tail; retail places the diagonal matrix at esp+0x10 below the
//   centre and the product (frame order c, e, w, a; here c, w, a, e, which no use-count change
//   reproduces) and computes hi = w + ext with w as the destination, keeping ext on the x87
//   stack until three final pops.
//   0..33 dead initialised int locals at the top of the function change nothing.
//   Session 12: the diagonal as three Vec3 locals (`Vec3 ex(b.x, 0, 0)`, ey, ez; zero
//   stores dead) gives retail's frame exactly (c, e, w, a: all 12-byte keys under the
//   small-frame size rule), but VC6 then adds an fxch pair to the ext.x sum (394-396).
//   Tail forms that change nothing: w + ext operand order, `+=` from a copy, Vec3
//   operators, inline Set helpers by reference/pointer/value, six ext sum orders.
//   Session 14: the centre as one expression per row, x + (y + z) + translation (or
//   (y + z) + x), places the pop esi as retail (541 -> 551); per-statement accumulation
//   pops it one instruction late. Declaring ext before or with w does not move the tail.
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

static inline void DiagonalMatrix(float m[3][3], float x, float y, float z)
{
    m[0][0] = x;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = y;
    m[1][2] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = z;
}

void SoultreeObject::GetWorldBounds(Vec3* lo, Vec3* hi)
{
    if (subtreeDirty)
        UpdateSubtreeBounds();
    float e[3][3];
    DiagonalMatrix(e, subtreeBoundsB.x, subtreeBoundsB.y, subtreeBoundsB.z);
    UpdateWorldMatrix();
    Vec3 c = subtreeBoundsA;
    Vec3 w;
    w.x = c.x * worldMatrix._11 + (c.y * worldMatrix._21 + c.z * worldMatrix._31) + worldMatrix._41;
    w.y = c.x * worldMatrix._12 + (c.y * worldMatrix._22 + c.z * worldMatrix._32) + worldMatrix._42;
    w.z = c.x * worldMatrix._13 + (c.y * worldMatrix._23 + c.z * worldMatrix._33) + worldMatrix._43;
    float a[3][3];
    a[0][0] = e[0][0] * worldMatrix._11;
    a[0][1] = e[0][0] * worldMatrix._12;
    a[0][2] = e[0][0] * worldMatrix._13;
    a[1][0] = e[1][1] * worldMatrix._21;
    a[1][1] = e[1][1] * worldMatrix._22;
    a[1][2] = e[1][1] * worldMatrix._23;
    a[2][0] = e[2][2] * worldMatrix._31;
    a[2][1] = e[2][2] * worldMatrix._32;
    a[2][2] = e[2][2] * worldMatrix._33;
    Vec3 ext;
    ext.x = AbsValue(a[0][0]) + AbsValue(a[1][0]) + AbsValue(a[2][0]);
    ext.y = AbsValue(a[0][1]) + AbsValue(a[1][1]) + AbsValue(a[2][1]);
    ext.z = AbsValue(a[0][2]) + AbsValue(a[1][2]) + AbsValue(a[2][2]);
    lo->x = w.x - ext.x;
    lo->y = w.y - ext.y;
    lo->z = w.z - ext.z;
    hi->x = ext.x + w.x;
    hi->y = ext.y + w.y;
    hi->z = ext.z + w.z;
}
