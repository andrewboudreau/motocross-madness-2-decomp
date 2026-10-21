#pragma once

// 4x4 float matrix (D3DMATRIX layout, m[row][column]; _RC names row R,
// column C) with D3DMATRIX's D3D_OVERLOADS members.
struct Matrix4 {
    Matrix4() {}
    float& operator()(int row, int column) { return m[row][column]; }
    const float& operator()(int row, int column) const { return m[row][column]; }
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};
