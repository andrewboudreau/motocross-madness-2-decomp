#include <math.h>

#include "MatrixUtil.h"

// Free matrix helpers in retail address order. Names are provisional and
// describe the decoded behavior; see MatrixUtil.h.

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
