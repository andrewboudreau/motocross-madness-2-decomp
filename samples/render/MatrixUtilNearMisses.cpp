// Near-miss MatrixUtil.cpp candidate (src/reconstructed/MatrixUtil.cpp,
// MatrixUtil.h), kept out of src/reconstructed until it matches.
//
// 0x004a1a50 (strided perspective transform, 175 bytes): 171/175; only the z
//   term of the last row loads the vector before the matrix element (retail
//   loads m[2][2] first). The typed cursors with a count-down loop are what
//   give the retail +4/+8 pointer bias and the register use; VC6 reorders the
//   sums itself (retail sums z, y, x for w and y, z, x / y, x, z for the
//   rows), so the written term order does not matter.
//   Also tried without gain: for/while/guarded do-while and count-up loops,
//   the increments in the for header or reordered, float*/char* cursors, the
//   point copied into locals or a Vector3, a stride or matrix-reference local,
//   an inline point-transform helper (pointer, reference or by-value point),
//   the rows computed into x/y/z/w locals and the divide written as `/ w`.
//   Every extra live local (stride, counter, reference) reorders many products
//   at once. Redundant parentheses (docs/VC6_OPERAND_ORDER.md section 3): all
//   2112 bracketings, term orders and parenthesised products of the z row,
//   and all 4096 combinations of parenthesised products over the four sums,
//   in this file and inside src/reconstructed/MatrixUtil.cpp, give 171 at
//   best.
//
// TriangleNormal 0x004a11e0 and ViewMatrix 0x004a1500 are exact in
// src/reconstructed/MatrixUtil.cpp (parenthesised products).
#include "../../src/reconstructed/MatrixUtil.h"

// 0x004a1a50: `count` points transformed by `matrix` with the perspective
// divide; a zero `sourceStride` means `targetStride`.
void UnknownFunction4a1a50(void* target, const void* source, const Matrix4* matrix, int count,
                           int targetStride, int sourceStride)
{
    const Vector3* p = (const Vector3*)source;
    Vector3* q = (Vector3*)target;
    if (!sourceStride)
        sourceStride = targetStride;
    while (count > 0) {
        float w = 1.0f / (p->x * matrix->m[0][3] + p->y * matrix->m[1][3] + p->z * matrix->m[2][3]
                          + matrix->m[3][3]);
        q->x = (p->x * matrix->m[0][0] + p->y * matrix->m[1][0] + p->z * matrix->m[2][0] + matrix->m[3][0]) * w;
        q->y = (p->x * matrix->m[0][1] + p->y * matrix->m[1][1] + p->z * matrix->m[2][1] + matrix->m[3][1]) * w;
        q->z = (p->x * matrix->m[0][2] + p->y * matrix->m[1][2] + p->z * matrix->m[2][2] + matrix->m[3][2]) * w;
        p = (const Vector3*)((const char*)p + sourceStride);
        q = (Vector3*)((char*)q + targetStride);
        count--;
    }
}
