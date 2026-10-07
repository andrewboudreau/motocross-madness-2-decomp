#pragma once

// D3D-style matrix and vector types and the free matrix helpers at
// 0x004a13e0-0x004a18db. No source string attributes these functions (the
// nearest references are Lzw.cpp before and MorphBastardModifier.cpp after),
// so the file name is descriptive. Their order and bodies closely follow the
// DirectX 5 SDK sample d3dutils.cpp (ZeroMatrix, IdentityMatrix,
// ProjectionMatrix, ViewMatrix, RotateZMatrix, MatrixMult); that is outside
// context for the names, not proof. Retail's projection takes an aspect ratio
// and its MatrixMult takes both matrices by value, unlike the SDK.

// 4x4 float matrix (D3DMATRIX layout, m[row][column]) with D3DMATRIX's
// D3D_OVERLOADS members. Both matter to codegen: the empty user-declared
// constructor makes VC6 copy a by-value result straight from the returned
// pointer (see samples/physics/common/Math3D.h), and MatrixMult only matches
// when written with the operator() accessor (plain m[][] indexing swaps the
// fld/fmul operands).
struct Matrix4 {
    Matrix4() {}
    float& operator()(int row, int column) { return m[row][column]; }
    const float& operator()(int row, int column) const { return m[row][column]; }
    float m[4][4];
};

// 12-byte float triple with D3DVECTOR-style constructors and the d3dvec.inl
// index accessor (D3D_OVERLOADS). The accessor matters to codegen the same
// way operator() does for Matrix4: 0x004a1300's dot products match only when
// the components are read through it (see MatrixUtil.cpp).
struct Vector3 {
    Vector3() {}
    Vector3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float x;
    float y;
    float z;

    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }
    Vector3 operator*(float scale) const { return Vector3(x * scale, y * scale, z * scale); }
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
};

inline Vector3 operator*(float scale, const Vector3& v) {
    return Vector3(scale * v.x, scale * v.y, scale * v.z);
}

// 0x004a10e0: reflects `v` about the plane with normal `n` and normalizes the
// result into `out`.
void UnknownFunction4a10e0(const Vector3* v, const Vector3* n, Vector3* out);
// 0x004a11e0: unit normal of the triangle a, b, c and, when `offset` is
// given, the plane offset -(normal . a). Near miss (one store scheduled one
// instruction apart): samples/render/MatrixUtilNearMisses.cpp.
void TriangleNormal(const Vector3* a, const Vector3* b, const Vector3* c, Vector3* normal, float* offset);
// 0x004a1300: where the line through `from` and `to` meets the plane of the
// triangle a, b, c.
void UnknownFunction4a1300(const Vector3* from, const Vector3* to, const Vector3* a, const Vector3* b,
                           const Vector3* c, Vector3* out);
Matrix4 ZeroMatrix();                          // 0x004a13e0
Matrix4 IdentityMatrix();                      // 0x004a1410
// 0x004a1460: perspective projection; fov in radians, aspect scales row 1.
Matrix4 ProjectionMatrix(float nearPlane, float farPlane, float fov, float aspect);
Matrix4 RotateZMatrix(float radians);          // 0x004a17f0
Matrix4 MatrixMult(Matrix4 a, Matrix4 b);      // 0x004a1860
// 0x004a18e0: inverse of the upper 3x3 of `m` (ZeroMatrix when singular); the
// fourth row and column of the result are left unset.
Matrix4 MatrixInverse(Matrix4 m);
// 0x004a1a50: transforms `count` points by `matrix` with the perspective
// divide; `sourceStride` 0 means `targetStride`. Near miss (one fld/fmul
// operand pair): samples/render/MatrixUtilNearMisses.cpp.
void UnknownFunction4a1a50(void* target, const void* source, const Matrix4* matrix, int count,
                           int targetStride, int sourceStride);
