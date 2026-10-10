// SoultreeBounds.cpp -- near misses of three SoultreeObject (soultree.cpp) methods that had no
// source: Scale 0x004fd340, AccumulateBounds 0x004fe2e0 and GetWorldBounds 0x004fe8a0. The
// exact members of the unit are in src/krusty2/soultree/soultree.cpp. Class attribution tier 2
// (thiscall on SoultreeObject fields, inside the unit's extent); names tier 3.
//
// Scale (0x004fd340, 630 bytes; 596/630): localMatrix = diag(x, y, z, 1) * localMatrix, written
//   in place from a copy, leaving the local translation (+0xe8..+0xf0) untouched (retail stores
//   13 of the 16 elements), then InvalidateWorldMatrix. Prologue, frame, the copy through an
//   inline helper's by-value parameter and rows 2-3 match; rows 1 and 4 emit their three
//   off-diagonal products in another order (retail k = 2,3,4 for row 1 and 2,1,3 for _44, with
//   the first row-1 product loading s first). Written term order, regrouping, a full 16-element
//   product with the translation restored, a temporary result, SetIdentity or reversed diagonal
//   stores and accumulation (`t += ...`) forms do not give both orders. With both operands by
//   value (as MatrixMultiply 0x0042a1a0 takes them; VC6 reads the unmodified `s` in place)
//   it is 596/630: row 1 then loads s12 and s14 the retail way and only its k = 3 product
//   and the order of _44's terms differ. Parameter order and factor order change nothing.
// GetWorldBounds (0x004fe8a0, 618 bytes; 541/610): the subtree box through the world matrix,
//   the half extents through |R| (a diagonal matrix times R, inline fabs). The instruction
//   stream matches up to the tail; retail places the diagonal matrix at esp+0x10 below the
//   centre and the product (frame order c, e, w, a; here c, w, a, e, which no use-count change
//   reproduces) and computes hi = w + ext with w as the destination, keeping ext on the x87
//   stack until three final pops.
// AccumulateBounds (0x004fe2e0, 1390 bytes; 80/1297): first draft. Same flow, calls and
//   arithmetic; retail's frame is 0x48 (here 0x3c): it copies each direction into a by-value
//   temporary at esp+0x40 and keeps the world centre in ebx/ebp/edx, and loads the three
//   pointer arguments only where they are used; in the `*have` branch retail builds hi through
//   the out-of-line Vec3 constructor 0x00404e60 (inline budget).
#include <string.h>
#include "math/Math3D.h"

static inline void ScaleProduct(Matrix4* out, Matrix4 m, Matrix4 s)
{
    out->_11 = s._11 * m._11 + s._12 * m._21 + s._13 * m._31 + s._14 * m._41;
    out->_12 = s._11 * m._12 + s._12 * m._22 + s._13 * m._32 + s._14 * m._42;
    out->_13 = s._11 * m._13 + s._12 * m._23 + s._13 * m._33 + s._14 * m._43;
    out->_14 = s._11 * m._14 + s._12 * m._24 + s._13 * m._34 + s._14 * m._44;
    out->_21 = s._21 * m._11 + s._22 * m._21 + s._23 * m._31 + s._24 * m._41;
    out->_22 = s._21 * m._12 + s._22 * m._22 + s._23 * m._32 + s._24 * m._42;
    out->_23 = s._21 * m._13 + s._22 * m._23 + s._23 * m._33 + s._24 * m._43;
    out->_24 = s._21 * m._14 + s._22 * m._24 + s._23 * m._34 + s._24 * m._44;
    out->_31 = s._31 * m._11 + s._32 * m._21 + s._33 * m._31 + s._34 * m._41;
    out->_32 = s._31 * m._12 + s._32 * m._22 + s._33 * m._32 + s._34 * m._42;
    out->_33 = s._31 * m._13 + s._32 * m._23 + s._33 * m._33 + s._34 * m._43;
    out->_34 = s._31 * m._14 + s._32 * m._24 + s._33 * m._34 + s._34 * m._44;
    out->_44 = s._41 * m._14 + s._42 * m._24 + s._43 * m._34 + s._44 * m._44;
}
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

// p in the frame's space: (p - translation) * transpose(R(frame)).
static inline Vec3 FramePoint(Vec3 p, const Matrix4* frame)
{
    p.x -= frame->_41;
    p.y -= frame->_42;
    p.z -= frame->_43;
    Vec3 r;
    r.x = p.x * frame->_11 + p.y * frame->_12 + p.z * frame->_13;
    r.y = p.x * frame->_21 + p.y * frame->_22 + p.z * frame->_23;
    r.z = p.x * frame->_31 + p.y * frame->_32 + p.z * frame->_33;
    return r;
}

static inline Vec3 FrameDirection(Vec3 v, const Matrix4* frame)
{
    Vec3 r;
    r.x = v.x * frame->_11 + v.y * frame->_12 + v.z * frame->_13;
    r.y = v.x * frame->_21 + v.y * frame->_22 + v.z * frame->_23;
    r.z = v.x * frame->_31 + v.y * frame->_32 + v.z * frame->_33;
    return r;
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
    w.x = c.z * worldMatrix._31;
    w.x += c.y * worldMatrix._21;
    w.x += c.x * worldMatrix._11;
    w.x += worldMatrix._41;
    w.y = c.z * worldMatrix._32;
    w.y += c.y * worldMatrix._22;
    w.y += c.x * worldMatrix._12;
    w.y += worldMatrix._42;
    w.z = c.z * worldMatrix._33;
    w.z += c.y * worldMatrix._23;
    w.z += c.x * worldMatrix._13;
    w.z += worldMatrix._43;
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

void SoultreeObject::AccumulateBounds(int* have, Vec3* lo, Vec3* hi, const Matrix4* frame)
{
    if (field_0x150) {
        Vec3 ax = Vec3(localBoundsB.x, 0.0f, 0.0f);
        Vec3 ay = Vec3(0.0f, localBoundsB.y, 0.0f);
        Vec3 az = Vec3(0.0f, 0.0f, localBoundsB.z);
        Vec3 c = LocalToWorldPoint(localBoundsA);
        ax = LocalToWorldDirection(ax);
        ay = LocalToWorldDirection(ay);
        az = LocalToWorldDirection(az);
        c = FramePoint(c, frame);
        ax = FrameDirection(ax, frame);
        ay = FrameDirection(ay, frame);
        az = FrameDirection(az, frame);
        Vec3 ext;
        ext.x = AbsValue(ax.x) + AbsValue(ay.x) + AbsValue(az.x);
        ext.y = AbsValue(ax.y) + AbsValue(ay.y) + AbsValue(az.y);
        ext.z = AbsValue(ax.z) + AbsValue(ay.z) + AbsValue(az.z);
        if (*have) {
            Vec3 l = c - ext;
            Vec3 h = c + ext;
            if (!(lo->x < l.x))
                lo->x = l.x;
            if (!(lo->y < l.y))
                lo->y = l.y;
            if (!(lo->z < l.z))
                lo->z = l.z;
            if (!(hi->x > h.x))
                hi->x = h.x;
            if (!(hi->y > h.y))
                hi->y = h.y;
            if (!(hi->z > h.z))
                hi->z = h.z;
        } else {
            *lo = c - ext;
            *hi = c + ext;
            *have = 1;
        }
    }
    for (SoultreeObject* child = firstChild; child; child = child->nextSibling)
        child->AccumulateBounds(have, lo, hi, frame);
}
