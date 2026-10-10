// SoultreeMatrix.cpp -- near misses of SoultreeObject (soultree.cpp) frame-relative matrix
// accessors (declared in core/SoultreeObject.h). The exact members of the unit are in
// src/krusty2/soultree/soultree.cpp. Class attribution tier 2; names tier 3.
#include <string.h>
#include "math/Math3D.h"

// 0x00436500: 4x4 product out = b * a (row-vector convention), owned by the collision area
// (CollisionMatrixMultiply). Declared here only so the call target is right.
void MatrixProductRows(Matrix4* out, const Matrix4* a, const Matrix4* b);

// Transposes the upper 3x3 of *m in place. GetMatrixIn and SetAxesIn expand it inline;
// SetMatrixIn calls the out-of-line COMDAT copy 0x004fefb0 (emitted at the end of the unit,
// after the vector initializers). This body matches 0x004fefb0 exactly.
inline void TransposeRotation3x3(Matrix4* m)
{
    float t;
    t = m->_12; m->_12 = m->_21; m->_21 = t;
    t = m->_13; m->_13 = m->_31; m->_31 = t;
    t = m->_23; m->_23 = m->_32; m->_32 = t;
}

// 0x0042a4b0 (src/krusty2/bvh/BoundingBoxTreeQuery.cpp, cdecl): *out = v * transpose(R(*m)).
void Vec3TransformNormalTranspose(Vec3* out, Vec3 v, const Matrix4* m);


// The 4x4 product of MatrixMultiply (0x0042a1a0) expanded inline, by reference: *out = b * a
// in the row-vector convention. The same helper makes UpdateWorldMatrix (0x004fb4f0) exact.
static inline void MatrixProduct(Matrix4* out, const Matrix4& a, const Matrix4& b)
{
    out->_11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
    out->_12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
    out->_13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
    out->_14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
    out->_21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
    out->_22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
    out->_23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
    out->_24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
    out->_31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
    out->_32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
    out->_33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
    out->_34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
    out->_41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
    out->_42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
    out->_43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
    out->_44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
}

static inline void TransposeRotation3(Matrix4& m)
{
    float t;
    t = m._12; m._12 = m._21; m._21 = t;
    t = m._13; m._13 = m._31; m._31 = t;
    t = m._23; m._23 = m._32; m._32 = t;
}

static inline Vec3 TranslationOf(const Matrix4& m) { return Vec3(m._41, m._42, m._43); }

// 0x00515600 is reached through CrossProductCall (Math3D.h out-of-line call views), so this
// TU calls it instead of expanding the inline helper.

// (y x z) written as retail orders it: the first vector's components are the left factors.
static inline Vec3 CrossOfYAndZ(const Vec3& z, const Vec3& y)
{
    Vec3 r;
    r.x = z.z * y.y - z.y * y.z;
    r.y = z.x * y.z - z.z * y.x;
    r.z = z.y * y.x - z.x * y.y;
    return r;
}

// v rotated by the transpose of m's upper 3x3 (dot with the ROWS of m). v is passed by value,
// as in retail.
static inline Vec3 RotateByRows(Vec3 v, const Matrix4& m)
{
    Vec3 r;
    r.x = v.x * m._11 + v.y * m._12 + v.z * m._13;
    r.y = v.x * m._21 + v.y * m._22 + v.z * m._23;
    r.z = v.x * m._31 + v.y * m._32 + v.z * m._33;
    return r;
}

// 0x004fca80. Frame == this: identity. Otherwise *out = world matrix; when 'frame' has a
// parent the result is re-expressed relative to that parent's world matrix: the rotation is
// (parentWorld * transpose(R(world)))^T and the translation is (t - t_parent) rotated by the
// parent's rows.
void SoultreeObject::GetMatrixIn(SoultreeObject* frame, Matrix4* out)
{
    if (frame == this) {
        memset(out, 0, sizeof(Matrix4));
        out->_44 = 1.0f;
        out->_33 = 1.0f;
        out->_22 = 1.0f;
        out->_11 = 1.0f;
        return;
    }
    UpdateWorldMatrix();
    *out = worldMatrix;
    if (frame) {
      if (frame->parent) {
        frame->parent->UpdateWorldMatrix();
        Matrix4 parentWorld = frame->parent->worldMatrix;
        Matrix4 m = *out;
        Vec3 d;
        d.x = m._41 - parentWorld._41;
        d.y = m._42 - parentWorld._42;
        d.z = m._43 - parentWorld._43;
        TransposeRotation3(m);
        MatrixProductRows(out, &m, &parentWorld);
        TransposeRotation3(*out);
        Vec3 t = RotateByRows(d, parentWorld);
        out->_41 = t.x;
        out->_42 = t.y;
        out->_43 = t.z;
        return;
      }
      *out = worldMatrix;
    }
}

// 0x004fc050. Inverse of GetAxesIn: builds a rotation whose row 2 is *axisZ and row 1 is
// *axisY (both given in 'frame' space), row 0 = cross(row1, row2), and stores it into
// localMatrix, keeping the old local translation. See SoultreeObject.h for the data flow.
void SoultreeObject::SetAxesIn(SoultreeObject* frame, const Vec3* axisZ, const Vec3* axisY,
                               int orthogonalize, int keepZ)
{
    Vec3 pos;
    pos.x = localMatrix._41;
    pos.y = localMatrix._42;
    pos.z = localMatrix._43;
    Matrix4 m;
    {
        Vec3 z = *axisZ;
        Vec3 y = *axisY;
        if (orthogonalize) {
            Vec3 r = CrossOfYAndZ(z, y);
            if (keepZ)
                y = CrossProductCall(z, r);
            else
                z = CrossProductCall(r, y);
        }
        y = Vec3Normalize(y);
        z = Vec3Normalize(z);
        *(Vec3*)&m._11 = CrossProductCall(y, z);
        m._14 = 0.0f;
        m._21 = y.x; m._22 = y.y; m._23 = y.z; m._24 = 0.0f;
        m._31 = z.x; m._32 = z.y; m._33 = z.z; m._34 = 0.0f;
    }
    m._41 = pos.x; m._42 = pos.y; m._43 = pos.z; m._44 = 1.0f;

    if (frame == this) {
        MatrixMultiply(&localMatrix, m, localMatrix);
        InvalidateWorldMatrix();
        return;
    }
    if (frame) {
        frame->UpdateWorldMatrix();
        MatrixMultiply(&m, frame->worldMatrix, m);
    }
    if (parent) {
        parent->UpdateWorldMatrix();
        Matrix4 parentWorld = parent->worldMatrix;
        Matrix4 t = m;
        Vec3 d;
        d.x = t._41 - parentWorld._41;
        d.y = t._42 - parentWorld._42;
        d.z = t._43 - parentWorld._43;
        TransposeRotation3(t);
        MatrixProductRows(&localMatrix, &t, &parentWorld);
        TransposeRotation3(localMatrix);
        *(Vec3*)&localMatrix._41 = RotateByRows(d, parentWorld);
    } else {
        localMatrix = m;
    }
    localMatrix._41 = pos.x;
    localMatrix._42 = pos.y;
    localMatrix._43 = pos.z;
    InvalidateWorldMatrix();
}

// 0x004fb8c0. Inverse direction of GetMatrixIn: makes this node's matrix equal *m given in
// 'frame' space. frame == this stores *m as the local matrix (0x004fca30); otherwise *m is
// first carried into world space in place (*m = *m * frame->worldMatrix) and then
// re-expressed relative to this node's parent. Tier 2 data flow, tier 3 names.
// PARTIAL: same instructions and frame (0x90) as retail, including both out-of-line calls of
// 0x004fefb0, but VC6 here gives the first copied matrix of each block the 0x50 slot and the
// second the 0x10 slot; retail has them the other way round. Declaration order, statement
// order, block- or function-scope matrices and a by-value inline product did not swap them.
void SoultreeObject::SetMatrixIn(SoultreeObject* frame, Matrix4* m)
{
    if (frame == this) {
        SetLocalMatrix(m);
        return;
    }
    if (frame) {
        frame->UpdateWorldMatrix();
        Matrix4 copy = *m;
        Matrix4 world = frame->worldMatrix;
        MatrixProduct(m, world, copy);
    }
    Matrix4* local = &localMatrix;
    if (parent) {
        parent->UpdateWorldMatrix();
        Matrix4 parentWorld = parent->worldMatrix;
        Matrix4 t = *m;
        Vec3 d;
        d.x = t._41 - parentWorld._41;
        d.y = t._42 - parentWorld._42;
        d.z = t._43 - parentWorld._43;
        TransposeRotation3x3(&t);
        MatrixProductRows(local, &t, &parentWorld);
        TransposeRotation3x3(local);
        Vec3TransformNormalTranspose(&d, d, &parentWorld);
        local->_41 = d.x;
        local->_42 = d.y;
        local->_43 = d.z;
    } else {
        *local = *m;
    }
    InvalidateWorldMatrix();
}

