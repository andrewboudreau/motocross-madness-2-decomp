// Near-miss MatrixUtil.cpp candidates (src/reconstructed/MatrixUtil.cpp, MatrixUtil.h),
// kept out of src/reconstructed until they match. Both have the retail length,
// control flow, calls and constants; the differences are one instruction each:
//
// 0x004a11e0 (TriangleNormal, 281 bytes): 97.32%; retail emits the fsubp of the
//   z component before the store of the y component (`mov [eax+4], edx`), VC6
//   stores first. Edge order (c - a kept on the x87 stack, b - a spilled), the
//   cross product written as a Vector3 constructor and the dot through the index
//   accessor are all required; member-init constructors, Vector3 temporaries,
//   float temporaries and helper-function cross products change more.
// 0x004a1a50 (strided perspective transform, 175 bytes): 97.66%; only the z term
//   of the last row loads the vector before the matrix element (retail loads
//   m[2][2] first). The typed cursors with a count-down loop are what give the
//   retail +4/+8 pointer bias and the register use; VC6 reorders the sums
//   itself (retail sums z, y, x for w and y, z, x / y, x, z for the rows), so the
//   written term order does not matter.
//
// 0x004a1500 (ViewMatrix, 744 bytes): 646/744; retail length, frame, slots,
//   calls and FP code. Only the scheduling of the integer moves that store
//   the up and direction columns (and write the normalised direction back to
//   its parameter) between the cross-product x87 instructions differs. The
//   normalisation needs the out-of-line dot (0x0040ae30) and scale
//   (0x005015b0) with a named scale local; the row-3 dots need the
//   file's DotProduct(v, from). Tried without gain: column, row and mixed
//   store orders, the cross as member stores, float locals or d3dvec.inl's
//   index-store CrossProduct, a separate normalised-direction local, the
//   identity built after the normalisations and const parameters.
//
// Also tried for 0x004a11e0 without any change (274/281): the offset dot
// through const-reference aliases of `normal` or `a`, with swapped
// DotProduct arguments, as an explicit chain in either order or mixed
// per-term order, through the index accessor on both operands, through a
// float temporary; NormalizeVector written as `v->x = v->x * scale`,
// through a reference and the index accessor, or as `*v = *v * scale`; the
// cross product as three member stores, through a Cross helper or as index
// stores (those lose 20-200 bytes); and the /G6 and /ML profiles (/G6 is
// worse for both functions).
#include "../../src/reconstructed/MatrixUtil.h"

// 0x00460c00 (src/krusty2/math/FastMath.h).
float FastInvSqrt(float x);

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// The helpers as in src/reconstructed/MatrixUtil.cpp.
inline void NormalizeVector(Vector3* v)
{
    float lengthSquared = v->z * v->z + (v->x * v->x + v->y * v->y);
    if (lengthSquared == 0.0f) {
        *v = kVec3Zero;
    } else {
        float scale = FastInvSqrt(lengthSquared);
        v->x *= scale;
        v->y *= scale;
        v->z *= scale;
    }
}

inline float DotProduct(const Vector3& a, const Vector3& b)
{
    return a[2] * b[2] + (a[0] * b[0] + a[1] * b[1]);
}

inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x004a11e0: normal = normalize((b - a) x (c - a)); offset = -(normal . a).
void TriangleNormal(const Vector3* a, const Vector3* b, const Vector3* c, Vector3* normal, float* offset)
{
    Vector3 e2 = *c - *a;
    Vector3 e1 = *b - *a;
    *normal = Vector3(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
    NormalizeVector(normal);
    if (offset) {
        *offset = -DotProduct(*normal, *a);
    }
}

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

// 0x0040ae30 (cdecl): out-of-line dot product.
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);
// 0x005015b0 (cdecl): out-of-line v * scale.
Vector3 UnknownFunction5015b0(const Vector3& v, float scale);

// v scaled to unit length (unchanged when it already is) through the
// out-of-line dot product and scale.
static inline Vector3 NormalizeCall(const Vector3& v) {
    float squared = UnknownFunction40ae30(&v, &v);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return UnknownFunction5015b0(v, scale);
}

// 0x004a1500: rows 0-2 hold right = up x direction, the normalised up and
// direction as columns, row 3 the negated dot products with `from`; a
// nonzero roll then rotates the view about its z axis.
Matrix4 ViewMatrix(Vector3 from, Vector3 direction, Vector3 up, float roll) {
    Matrix4 view = IdentityMatrix();
    up = NormalizeCall(up);
    direction = NormalizeCall(direction);
    Vector3 right(up.y * direction.z - up.z * direction.y, up.z * direction.x - up.x * direction.z,
                  up.x * direction.y - up.y * direction.x);
    view(0, 0) = right.x;
    view(1, 0) = right.y;
    view(2, 0) = right.z;
    view(0, 1) = up.x;
    view(1, 1) = up.y;
    view(2, 1) = up.z;
    view(0, 2) = direction.x;
    view(1, 2) = direction.y;
    view(2, 2) = direction.z;
    view(3, 0) = -DotProduct(right, from);
    view(3, 1) = -DotProduct(up, from);
    view(3, 2) = -DotProduct(direction, from);
    if (roll != 0.0f)
        view = MatrixMult(RotateZMatrix(-roll), view);
    return view;
}
