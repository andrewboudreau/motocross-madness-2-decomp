#include <math.h>

#include "MatrixUtil.h"

// Free matrix helpers in retail address order. Names are provisional and
// describe the decoded behavior; see MatrixUtil.h. The unit is
// 0x004a10e0..0x004a1d0b: the vector helpers, the matrix helpers, then this
// file's vector set, which 0x004a10e0 and 0x004a11e0 read (strong inference).

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x00685040, 0x00685050, 0x00685060 and
// 0x00685030, initialised by 0x004a1bd0..0x004a1d0b (.CRT$XCU 175-178).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00460c00 (src/krusty2/math/FastMath.h).
float FastInvSqrt(float x);

// Normalizes `v` in place; the zero vector stays zero. The squared length is
// summed as z^2 + (x^2 + y^2): the natural x + y + z order swaps the x87
// loads in 0x004a10e0 and 0x004a11e0.
inline void NormalizeVector(Vector3* v) {
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

// Dot product through the d3dvec.inl index accessor, summed as
// z + (x + y). Both the accessor and the association are needed for
// 0x004a1300's fld/fmul operand order (0x004a10e0 writes its dot inline).
inline float DotProduct(const Vector3& a, const Vector3& b) {
    return a[2] * b[2] + (a[0] * b[0] + a[1] * b[1]);
}

inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x004a10e0: out = normalize(v - 2 (v . n) n).
void UnknownFunction4a10e0(const Vector3* v, const Vector3* n, Vector3* out) {
    float d = v->z * n->z + (v->x * n->x + v->y * n->y);
    Vector3 twice(n->x + n->x, n->y + n->y, n->z + n->z);
    Vector3 s = twice * d;
    *out = Vector3(v->x - s.x, v->y - s.y, v->z - s.z);
    NormalizeVector(out);
}

// 0x004a11e0 (TriangleNormal) is a near miss: samples/render/MatrixUtilNearMisses.cpp.

// 0x004a1300: the triangle's plane (normal, offset) from TriangleNormal, then
// from + (to - from) * t with t = -(normal . from + offset) / (normal . direction).
void UnknownFunction4a1300(const Vector3* from, const Vector3* to, const Vector3* a, const Vector3* b,
                           const Vector3* c, Vector3* out) {
    Vector3 normal;
    float offset;
    TriangleNormal(a, b, c, &normal, &offset);
    Vector3 direction = *to - *from;
    float t = -((DotProduct(normal, *from) + offset) / DotProduct(normal, direction));
    Vector3 step = direction * t;
    *out = Vector3(step.x + from->x, step.y + from->y, step.z + from->z);
}

// 0x004a13e0
Matrix4 ZeroMatrix() {
    Matrix4 result;
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++)
            result.m[row][column] = 0.0f;
    }
    return result;
}

// 0x004a1410
Matrix4 IdentityMatrix() {
    Matrix4 result;
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++)
            result.m[row][column] = (row == column) ? 1.0f : 0.0f;
    }
    return result;
}

// 0x004a1460: cot(fov / 2) on the diagonal (row 1 scaled by aspect),
// Q = 1 / (1 - near / far) in [2][2], -Q * near in [3][2] and 1 in [2][3].
Matrix4 ProjectionMatrix(float nearPlane, float farPlane, float fov, float aspect) {
    float c = (float)cos(fov * 0.5);
    float s = (float)sin(fov * 0.5);
    float cotangent = c / s;
    float q = 1.0f / (1.0f - nearPlane / farPlane);
    Matrix4 result = ZeroMatrix();
    result.m[0][0] = cotangent;
    result.m[1][1] = cotangent * aspect;
    result.m[2][2] = q;
    result.m[3][2] = -q * nearPlane;
    result.m[2][3] = 1.0f;
    return result;
}

// 0x004a17f0
Matrix4 RotateZMatrix(float radians) {
    float cosine = (float)cos(radians);
    float sine = (float)sin(radians);
    Matrix4 result = IdentityMatrix();
    result.m[0][0] = cosine;
    result.m[1][1] = cosine;
    result.m[0][1] = -sine;
    result.m[1][0] = sine;
    return result;
}

// 0x004a1860: result(i, j) = sum over k of a(k, j) * b(i, k), i.e. b * a.
Matrix4 MatrixMult(Matrix4 a, Matrix4 b) {
    Matrix4 result = ZeroMatrix();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++)
                result(i, j) += a(k, j) * b(i, k);
        }
    }
    return result;
}

// 0x004a18e0: inverse of the upper 3x3 by cofactors, in the d3dmath.cpp
// D3DMath_MatrixInvert shape (inverse * cofactor, the first-column cofactors
// repeated from the determinant and shared by the compiler). Named cofactor
// locals keep c11 in a register and swap the determinant's fld/fmul; the
// fourth row and column of the result are left unset.
Matrix4 MatrixInverse(Matrix4 m) {
    Matrix4 result;
    float determinant = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2])
                        - m.m[0][1] * (m.m[2][2] * m.m[1][0] - m.m[1][2] * m.m[2][0])
                        + m.m[0][2] * (m.m[2][1] * m.m[1][0] - m.m[1][1] * m.m[2][0]);
    if (determinant != 0.0f) {
        float inverse = 1.0f / determinant;
        result.m[0][0] = inverse * (m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
        result.m[0][1] = -inverse * (m.m[2][2] * m.m[0][1] - m.m[2][1] * m.m[0][2]);
        result.m[0][2] = inverse * (m.m[1][2] * m.m[0][1] - m.m[1][1] * m.m[0][2]);
        result.m[1][0] = -inverse * (m.m[2][2] * m.m[1][0] - m.m[1][2] * m.m[2][0]);
        result.m[1][1] = inverse * (m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]);
        result.m[1][2] = -inverse * (m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
        result.m[2][0] = inverse * (m.m[2][1] * m.m[1][0] - m.m[1][1] * m.m[2][0]);
        result.m[2][1] = -inverse * (m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]);
        result.m[2][2] = inverse * (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]);
    } else {
        result = ZeroMatrix();
    }
    return result;
}

// 0x004a1a50 (the strided perspective transform) is a near miss:
// samples/render/MatrixUtilNearMisses.cpp.

// 0x004a1b00 is hand-scheduled x87 code (an ebp frame, fxch pairing and a
// negative-index loop ending in `cmp edx, 0`): inline assembly in the
// original, not reconstructed (src/reconstructed/D3DIMSoulTree.h declares it).
