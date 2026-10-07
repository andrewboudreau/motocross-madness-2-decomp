// Near-miss MatrixUtil.cpp candidates (src/reconstructed/MatrixUtil.cpp, MatrixUtil.h),
// kept out of src/reconstructed until they match. All five have the retail control flow,
// calls and constants; the differences are x87 operand order and register choice:
//
// 0x004a10e0 (reflect, 247 bytes): 234/247; the second dot-product term loads n before v,
//   and retail loads the vector to normalize as y, x, z.
// 0x004a11e0 (TriangleNormal, 283 bytes): same shape; edge vectors are kept on the x87
//   stack in a different order.
// 0x004a1300 (line/plane, 211 bytes): 202/211 at best; no permutation of the dot-product
//   terms gives retail's normal-first operand order in the denominator.
// 0x004a18e0 (MatrixInverse, 362 bytes): cofactor operand order.
// 0x004a1a50 (projected transform, 175 bytes): retail biases the source and target
//   pointers (+4, +8) and sums the terms in another order.
#include "../../src/reconstructed/MatrixUtil.h"

float UnknownFunction460c00(float value);

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Normalizes `v` in place; the zero vector stays zero.
inline void NormalizeVector(Vector3* v)
{
    float lengthSquared = v->x * v->x + v->y * v->y + v->z * v->z;
    if (lengthSquared == 0.0f) {
        *v = kVec3Zero;
    } else {
        float scale = UnknownFunction460c00(lengthSquared);
        v->x *= scale;
        v->y *= scale;
        v->z *= scale;
    }
}

// 0x004a10e0
void UnknownFunction4a10e0(const Vector3* v, const Vector3* n, Vector3* out)
{
    float d = v->x * n->x + v->y * n->y + v->z * n->z;
    Vector3 twice(n->x + n->x, n->y + n->y, n->z + n->z);
    Vector3 s = twice * d;
    *out = Vector3(v->x - s.x, v->y - s.y, v->z - s.z);
    NormalizeVector(out);
}

// 0x004a11e0
void TriangleNormal(const Vector3* a, const Vector3* b, const Vector3* c, Vector3* normal, float* offset)
{
    Vector3 e1(b->x - a->x, b->y - a->y, b->z - a->z);
    Vector3 e2(c->x - a->x, c->y - a->y, c->z - a->z);
    *normal = Vector3(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
    NormalizeVector(normal);
    if (offset) {
        *offset = -(normal->x * a->x + normal->y * a->y + normal->z * a->z);
    }
}

// 0x004a1300
void UnknownFunction4a1300(const Vector3* from, const Vector3* to, const Vector3* a, const Vector3* b,
                           const Vector3* c, Vector3* out)
{
    Vector3 normal;
    float offset;
    TriangleNormal(a, b, c, &normal, &offset);
    Vector3 direction(to->x - from->x, to->y - from->y, to->z - from->z);
    float t = -((normal.x * from->x + normal.y * from->y + normal.z * from->z + offset)
                / (normal.x * direction.x + normal.y * direction.y + normal.z * direction.z));
    Vector3 step = direction * t;
    *out = Vector3(step.x + from->x, step.y + from->y, step.z + from->z);
}

// Free matrix helpers in retail address order. Names are provisional and
// describe the decoded behavior; see MatrixUtil.h.


// 0x004a18e0
Matrix4 MatrixInverse(Matrix4 m) {
    Matrix4 result;
    float c11 = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
    float c12 = m.m[1][0] * m.m[2][2] - m.m[2][0] * m.m[1][2];
    float c13 = m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1];
    float determinant = c11 * m.m[0][0] - c12 * m.m[0][1] + c13 * m.m[0][2];
    if (determinant == 0.0f) {
        result = ZeroMatrix();
    } else {
        float inverse = 1.0f / determinant;
        result.m[0][0] = c11 * inverse;
        result.m[0][1] = -((m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) * inverse);
        result.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) * inverse;
        result.m[1][0] = -(c12 * inverse);
        result.m[1][1] = (m.m[2][2] * m.m[0][0] - m.m[0][2] * m.m[2][0]) * inverse;
        result.m[1][2] = -((m.m[1][2] * m.m[0][0] - m.m[0][2] * m.m[1][0]) * inverse);
        result.m[2][0] = c13 * inverse;
        result.m[2][1] = -((m.m[2][1] * m.m[0][0] - m.m[2][0] * m.m[0][1]) * inverse);
        result.m[2][2] = (m.m[1][1] * m.m[0][0] - m.m[1][0] * m.m[0][1]) * inverse;
    }
    return result;
}

// 0x004a1a50
void UnknownFunction4a1a50(void* target, const void* source, const Matrix4* matrix, int count,
                           int targetStride, int sourceStride) {
    if (!sourceStride)
        sourceStride = targetStride;
    for (int i = 0; i < count; i++) {
        const Vector3* p = (const Vector3*)source;
        Vector3* q = (Vector3*)target;
        float w = 1.0f / (p->x * matrix->m[0][3] + p->y * matrix->m[1][3] + p->z * matrix->m[2][3]
                          + matrix->m[3][3]);
        q->x = (p->x * matrix->m[0][0] + p->y * matrix->m[1][0] + p->z * matrix->m[2][0] + matrix->m[3][0]) * w;
        q->y = (p->x * matrix->m[0][1] + p->y * matrix->m[1][1] + p->z * matrix->m[2][1] + matrix->m[3][1]) * w;
        q->z = (p->x * matrix->m[0][2] + p->y * matrix->m[1][2] + p->z * matrix->m[2][2] + matrix->m[3][2]) * w;
        source = (const char*)source + sourceStride;
        target = (char*)target + targetStride;
    }
}

