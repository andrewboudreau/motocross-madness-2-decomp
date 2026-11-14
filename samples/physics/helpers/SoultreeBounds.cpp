// SoultreeBounds.cpp -- near miss of the SoultreeObject (soultree.cpp) method Scale 0x004fd340.
// GetWorldBounds 0x004fe8a0, formerly here, is strict exact in src/krusty2/soultree/soultree.cpp
// with the other exact members of the unit. Class attribution tier 2
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
//   Session 15: the 13 products written out in Scale itself on a `Matrix4 m = localMatrix`
//   copy (no helper parameters), in written or the k = 1,3,2,4 order: 45-52 (661 bytes)
//   stored to the member, 580-586 through the `local` pointer; s13 stays s-first.
//   Round 9 (2026-11-14): retail's emitted order per row is k = 2, 3, 4, 1 for row 1 (m
//   first in the k = 3 and 4 products, s._12 loaded before the copy), 4, 3, 1, 2 for row
//   2, 4, 2, 1, 3 for row 3 and 2, 1, 3, 4 for _44, the diagonal term last each time
//   (folded to the parameter or to m._44). Writing row 1 literally that way is 550, with
//   the three off-diagonal terms parenthesised 591 or 547, m._3J first in the present
//   order 606; the diagonal stores in reverse order 618; no `local` pointer 38.
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
