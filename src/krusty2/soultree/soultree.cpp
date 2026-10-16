// soultree.cpp -- SoultreeObject, the scene-graph node (retail D:\aardvark\VC\krusty2\soultree.cpp).
//
// Unit evidence:
//  * __FILE__ literal 0x00574028, referenced (operand addresses) at 0x004fdb7d (line 0x346),
//    0x004fdd56 (0x370), 0x004fe03c (0x3aa) and 0x004feded (0x49b), all inside SoultreeObject
//    methods.
//  * Extent 0x004fb2b0..0x004fefd8: from the constructor (first SoultreeObject vptr write)
//    through the unit's Math3D.h vector initializers (0x004fee70..0x004fefab, vectors
//    0x00689e88..0x00689ec0) to the out-of-line COMDAT transpose 0x004fefb0. It lies between
//    SkyCube (vtable 0x00557ba8, 0x004fb1e0..0x004fb2ac, no __FILE__; alphabetically
//    SelectiveGravityModel < Shock < SkyCube < soultree) and SoultreeMaterial.cpp (0x004fefe0).
//  * The node array 0x00689ec4/0x00689ec8/0x00689ecc directly follows this unit's vectors in
//    .bss and only RegisterNode/UnregisterNode write it.
// Class identity and offsets: core/SoultreeObject.h. Method names are tier 3.
//
// Every function below is strict exact. Near misses of the same unit (WorldToLocalDirection
// 0x004fd710, SetMatrixIn 0x004fb8c0 with the transpose COMDAT 0x004fefb0, SetAxesIn 0x004fc050,
// RotateAboutPoint 0x004fd1f0) are in samples/physics/helpers/.
// Scale 0x004fd340 (inline 4x4 product) and GetWorldBounds 0x004fe8a0 are near misses in
// samples/physics/helpers/SoultreeBounds.cpp.
#define SOULTREE_OBJECT_WITH_BASES
#include <math.h>
#include <string.h>
#include "math/Math3D.h"
#include "core/DebugAlloc.h"
#include "broadphase/Quadtree.h"

// Type-id registry (0x00575744), same declaration as collision/CollisionObject.cpp.
class TypeRegistry {
public:
    char FindTypeId(const char* name);           // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;             // 0x00575744

extern QuadTree* g_quadTree;                     // 0x0068aba4

// 0x00436500 (cdecl): out = b * a in the row-vector convention (collision area,
// collision/CollisionObject.cpp).
void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b);

// 0x004a1410 (cdecl, hidden result pointer): the identity matrix.
Matrix4 IdentityMatrix();

// Identity in place (expanded inline by the constructor).
static inline void SetIdentity(Matrix4* m)
{
    memset(m, 0, sizeof(Matrix4));
    m->_44 = 1.0f;
    m->_33 = 1.0f;
    m->_22 = 1.0f;
    m->_11 = 1.0f;
}

// Rotation by 'angle' radians about the axis (x, y, z), normalized first (row-vector
// convention). Expanded inline by RotateAbout and SetRotation.
static inline void AxisAngleMatrix(Matrix4* m, float x, float y, float z, float angle)
{
    float len = (float)sqrt(x * x + y * y + z * z);
    x /= len;
    y /= len;
    z /= len;
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f - c;
    m->_11 = t * x * x + c;
    m->_12 = t * x * y + s * z;
    m->_13 = t * x * z - s * y;
    m->_14 = 0.0f;
    m->_21 = t * x * y - s * z;
    m->_22 = t * y * y + c;
    m->_23 = t * y * z + s * x;
    m->_24 = 0.0f;
    m->_31 = t * x * z + s * y;
    m->_32 = t * y * z - s * x;
    m->_33 = t * z * z + c;
    m->_34 = 0.0f;
    m->_41 = 0.0f;
    m->_42 = 0.0f;
    m->_43 = 0.0f;
    m->_44 = 1.0f;
}

// The 4x4 product of MatrixMultiply (0x0042a1a0, math/Math3D.h) expanded inline: in the
// row-vector convention *out = b * a. Same text as the out-of-line body.
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

// Node array (0x00689ec4 pointer, 0x00689ec8 count, 0x00689ecc capacity). Read by
// SoultreeQuadTreeRenderer.cpp, so external.
SoultreeObject** g_soultreeNodes;
int g_soultreeNodeCount;
int g_soultreeNodeCapacity;

// Swap across the diagonal of the upper 3x3 (retail does this in place on a stack copy).
static inline void TransposeRotation(Matrix4& m)
{
    float t;
    t = m._12; m._12 = m._21; m._21 = t;
    t = m._13; m._13 = m._31; m._31 = t;
    t = m._23; m._23 = m._32; m._32 = t;
}

// v rotated by the transpose of m's upper 3x3 (dot with the ROWS of m) into *r. v is taken by
// value: GetMatrixIn passes its own delta as both, so VC6 copies v first (retail's copy of
// the delta, its x row reading the original and the y and z rows the copy). Returning a Vec3
// (named result, Vec3(...) or a by-value result assigned back) leaves the copy out (308/494).
static inline void RotateByRowsTo(Vec3* r, Vec3 v, const Matrix4& m)
{
    r->x = v.x * m._11 + v.y * m._12 + v.z * m._13;
    r->y = v.x * m._21 + v.y * m._22 + v.z * m._23;
    r->z = v.x * m._31 + v.y * m._32 + v.z * m._33;
}

// 0x004fb2b0. QuadTreeObject's inline constructor, then GameObject(a); identity local
// matrix, zero world matrix, empty links and bounds; both type-id bytes get the
// "SoultreeObject" id.
SoultreeObject::SoultreeObject(int a)
    : GameObject(a)
{
    parent = 0;
    firstChild = 0;
    prevSibling = 0;
    nextSibling = 0;
    SetIdentity(&localMatrix);
    memset(&worldMatrix, 0, sizeof(worldMatrix));
    worldValid = 0;
    field_0x190 = 1;
    quadtreeCell = 15;
    inQuadtree = 0;
    field_0x14c = 0;
    field_0x150 = 0;
    localBoundsValid = 0;
    localBoundsA.x = 0;
    localBoundsA.y = 0;
    localBoundsA.z = 0;
    localBoundsB.x = 0;
    localBoundsB.y = 0;
    localBoundsB.z = 0;
    field_0x170 = 0;
    subtreeBoundsA.x = 0;
    subtreeBoundsA.y = 0;
    subtreeBoundsA.z = 0;
    subtreeBoundsB.x = 0;
    subtreeBoundsB.y = 0;
    subtreeBoundsB.z = 0;
    name[0] = 0;
    loadedNodeCount = 0;
    field_0x1c[8] = objectTypeId = g_TypeRegistry->FindTypeId("SoultreeObject");
}

// 0x004fb430 (scalar deleting wrapper 0x004fb400). Detaches from the parent, releases the
// children (GameObject slot 2), leaves the quadtree and the node array.
SoultreeObject::~SoultreeObject()
{
    if (parent)
        parent->RemoveChild(this);
    if (firstChild) {
        while (firstChild->nextSibling)
            firstChild->nextSibling->BaseObjectVirtualSlot2();
        firstChild->BaseObjectVirtualSlot2();
    }
    RemoveFromQuadtree();
    UnregisterNode();
}

// 0x004fb4f0. worldMatrix = localMatrix * parent->worldMatrix, refreshing the parent chain
// first; a root copies its local matrix.
void SoultreeObject::UpdateWorldMatrix()
{
    if (worldValid)
        return;
    if (parent) {
        parent->UpdateWorldMatrix();
        MatrixProduct(&worldMatrix, parent->worldMatrix, localMatrix);
    } else {
        worldMatrix = localMatrix;
    }
    worldValid = 1;
}

// 0x004fb850. Marks the ancestors' subtree flag, then invalidates this node and its
// descendants if the world matrix was valid. (The siblings of THIS node are not touched.)
void SoultreeObject::InvalidateWorldMatrix()
{
    MarkSubtreeDirty();
    if (worldValid) {
        worldValid = 0;
        if (firstChild)
            firstChild->InvalidateSiblingChain();
    }
}

// 0x004fb880.
void SoultreeObject::InvalidateSiblingChain()
{
    SoultreeObject* n = this;
    for (;;) {
        if (n->nextSibling)
            n->nextSibling->InvalidateSiblingChain();
        if (!n->worldValid)
            break;
        n->worldValid = 0;
        n = n->firstChild;
        if (!n)
            break;
    }
}

// 0x004fbd10.
void SoultreeObject::SetAxes(float zx, float zy, float zz, float yx, float yy, float yz,
                                 int orthogonalize, int keepZ)
{
    Vec3 z(zx, zy, zz);
    Vec3 y(yx, yy, yz);
    SetAxesPtr(&z, &y, orthogonalize, keepZ);
}

// v / |v| through the out-of-line Vec3 helpers; v itself when |v|^2 is exactly 1.
static inline Vec3 Normalized(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float inv = FastInvSqrt(lenSq);
    Vec3 r;
    return *Vec3ScaleCall(&r, &v, inv);
}

// 0x004fbd70, ret 0x10. Builds the local rotation from row 2 = *axisZ and row 1 = *axisY
// (row 0 = y x z), keeping the local translation. With 'orthogonalize', r = y x z first
// replaces y by z x r (keepZ) or z by r x y. Both axes are normalized through the out-of-line
// helpers (0x0040ae30, 0x00460c00, 0x005015b0); row 0 comes from 0x00515600.
// The inline cross products need Math3D.h's parenthesised first products (fsubp between the
// load and the store of the copied component, docs/VC6_OPERAND_ORDER.md section 3).
void SoultreeObject::SetAxesPtr(const Vec3* axisZ, const Vec3* axisY, int orthogonalize, int keepZ)
{
    Vec3 pos;
    pos.x = localMatrix._41;
    pos.y = localMatrix._42;
    pos.z = localMatrix._43;
    Vec3 z = *axisZ;
    Vec3 y = *axisY;
    if (orthogonalize) {
        Vec3 r = CrossProduct(y, z);
        if (keepZ)
            y = CrossProduct(z, r);
        else
            z = CrossProduct(r, y);
    }
    y = Normalized(y);
    z = Normalized(z);
    Vec3 x = CrossProductCall(y, z);
    localMatrix._11 = x.x;
    localMatrix._12 = x.y;
    localMatrix._13 = x.z;
    localMatrix._14 = 0.0f;
    localMatrix._21 = y.x;
    localMatrix._22 = y.y;
    localMatrix._23 = y.z;
    localMatrix._24 = 0.0f;
    localMatrix._31 = z.x;
    localMatrix._32 = z.y;
    localMatrix._33 = z.z;
    localMatrix._34 = 0.0f;
    localMatrix._41 = pos.x;
    localMatrix._42 = pos.y;
    localMatrix._43 = pos.z;
    localMatrix._44 = 1.0f;
    InvalidateWorldMatrix();
}

// 0x004fc4f0. Local rotation rows 2 and 1; either output may be null.
void SoultreeObject::GetAxes(Vec3* axisZ, Vec3* axisY)
{
    if (axisZ) {
        axisZ->x = localMatrix._31;
        axisZ->y = localMatrix._32;
        axisZ->z = localMatrix._33;
    }
    if (axisY) {
        axisY->x = localMatrix._21;
        axisY->y = localMatrix._22;
        axisY->z = localMatrix._23;
    }
}

// 0x004fc540. World axes (rows 2 and 1 of the world matrix) in 'frame' space; (0,0,1) and
// (0,1,0) when frame == this. Either output may be null.
void SoultreeObject::GetAxesIn(SoultreeObject* frame, Vec3* axisZ, Vec3* axisY)
{
    if (frame == this) {
        if (axisZ) {
            axisZ->x = 0.0f;
            axisZ->y = 0.0f;
            axisZ->z = 1.0f;
        }
        if (axisY) {
            axisY->x = 0.0f;
            axisY->y = 1.0f;
            axisY->z = 0.0f;
        }
        return;
    }
    UpdateWorldMatrix();
    if (axisZ) {
        axisZ->x = worldMatrix._31;
        axisZ->y = worldMatrix._32;
        axisZ->z = worldMatrix._33;
    }
    if (axisY) {
        axisY->x = worldMatrix._21;
        axisY->y = worldMatrix._22;
        axisY->z = worldMatrix._23;
    }
    if (frame) {
        if (axisZ)
            *axisZ = frame->WorldToLocalDirection(*axisZ);
        if (axisY)
            *axisY = frame->WorldToLocalDirection(*axisY);
    }
}

// 0x004fc630. Sets the local translation (localMatrix row 3) and invalidates the world
// matrix. __thiscall, ret 0xc.
void SoultreeObject::SetPosition(float x, float y, float z)
{
    localMatrix._41 = x;
    localMatrix._42 = y;
    localMatrix._43 = z;
    InvalidateWorldMatrix();
}

// 0x004fc660. Same, from a vector.
void SoultreeObject::SetPosition(const Vec3& p)
{
    localMatrix._41 = p.x;
    localMatrix._42 = p.y;
    localMatrix._43 = p.z;
    InvalidateWorldMatrix();
}

// 0x004fc690.
void SoultreeObject::TranslateIn(SoultreeObject* frame, Vec3 delta)
{
    if (frame == this) {
        localMatrix._41 = delta.x + localMatrix._41;
        localMatrix._42 = delta.y + localMatrix._42;
        localMatrix._43 = delta.z + localMatrix._43;
        InvalidateWorldMatrix();
        return;
    }
    if (frame) {
        delta = frame->LocalToWorldPoint(delta);
        delta = parent->WorldToLocalPoint(delta);
    }
    SetPosition(delta);
}

// 0x004fc740.
void SoultreeObject::SetPositionInFrame(SoultreeObject* frame, const Vec3* p)
{
    Vec3 t;
    if (frame == this) {
        localMatrix._41 += p->x;
        localMatrix._42 += p->y;
        localMatrix._43 += p->z;
        InvalidateWorldMatrix();
        return;
    }
    if (frame) {
        t = frame->LocalToWorldPoint(*p);
        t = parent->WorldToLocalPoint(t);
        SetPosition(t);
        return;
    }
    if (parent)
        t = parent->WorldToLocalPoint(*p);
    else
        t = *p;
    SetPosition(t);
}

// 0x004fc850.
void SoultreeObject::Translate(const Vec3* d)
{
    localMatrix._41 = d->x + localMatrix._41;
    localMatrix._42 = d->y + localMatrix._42;
    localMatrix._43 = d->z + localMatrix._43;
    InvalidateWorldMatrix();
}

// 0x004fc890.
void SoultreeObject::SetPositionIn(SoultreeObject* frame, const Vec3* p)
{
    Vec3 t;
    if (frame == this) {
        Translate(p);
        return;
    }
    if (frame) {
        t = frame->LocalToWorldDirection(*p);
        t = parent->WorldToLocalDirection(t);
        Translate(&t);
        return;
    }
    if (parent)
        t = parent->WorldToLocalPoint(*p);
    else
        t = *p;
    Translate(&t);
}

// 0x004fc970. Writes the local translation to *out and returns out.
Vec3* SoultreeObject::GetPosition(Vec3* out)
{
    out->x = localMatrix._41;
    out->y = localMatrix._42;
    out->z = localMatrix._43;
    return out;
}

// 0x004fc9a0. Position expressed in 'frame' space: the local translation when frame == this,
// otherwise the world translation, converted through frame->WorldToLocalPoint when frame is
// non-null. __thiscall, ret 8.
void SoultreeObject::GetPositionIn(SoultreeObject* frame, Vec3* out)
{
    if (frame == this) {
        out->x = localMatrix._41;
        out->y = localMatrix._42;
        out->z = localMatrix._43;
        return;
    }
    UpdateWorldMatrix();
    out->x = worldMatrix._41;
    out->y = worldMatrix._42;
    out->z = worldMatrix._43;
    if (frame)
        *out = frame->WorldToLocalPoint(*out);
}

// 0x004fca30.
void SoultreeObject::SetLocalMatrix(const Matrix4* m)
{
    localMatrix = *m;
    InvalidateWorldMatrix();
}

// 0x004fca60.
void SoultreeObject::GetLocalMatrix(Matrix4* out)
{
    *out = localMatrix;
}

// 0x004fca80. Frame == this: identity. Otherwise *out = world matrix; when 'frame' has a
// parent the result is re-expressed relative to that parent's world matrix: the rotation is
// (parentWorld * transpose(R(world)))^T and the translation is (t - t_parent) rotated by the
// parent's rows.
void SoultreeObject::GetMatrixIn(SoultreeObject* frame, Matrix4* out)
{
    if (frame == this) {
        SetIdentity(out);
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
        TransposeRotation(m);
        CollisionMatrixMultiply(out, &m, &parentWorld);
        TransposeRotation(*out);
        RotateByRowsTo(&d, d, parentWorld);
        out->_41 = d.x;
        out->_42 = d.y;
        out->_43 = d.z;
        return;
      }
      *out = worldMatrix;
    }
}

// 0x004fcc70.
void SoultreeObject::ResetMatrices()
{
    worldValid = 0;
    localMatrix = IdentityMatrix();
    worldMatrix = IdentityMatrix();
    if (nextSibling)
        nextSibling->ResetMatrices();
    if (firstChild)
        firstChild->ResetMatrices();
}

// 0x004fcce0. Rotates the local matrix about the axis (x, y, z) (normalized here) by 'angle'
// radians: localMatrix = localMatrix * R(axis, angle), keeping the local translation.
void SoultreeObject::RotateAbout(float x, float y, float z, float angle)
{
    Matrix4* local = &localMatrix;
    Vec3 pos;
    pos.x = local->_41;
    pos.y = local->_42;
    pos.z = local->_43;
    Matrix4 r;
    AxisAngleMatrix(&r, x, y, z, angle);
    MatrixMultiply(local, *local, r);
    local->_41 = pos.x;
    local->_42 = pos.y;
    local->_43 = pos.z;
    InvalidateWorldMatrix();
}

// 0x004fceb0. RotateAbout with the axis as a vector.
void SoultreeObject::Rotate(Vec3 axis, float angle)
{
    Matrix4* local = &localMatrix;
    Vec3 pos;
    pos.x = local->_41;
    pos.y = local->_42;
    pos.z = local->_43;
    Matrix4 r;
    AxisAngleMatrix(&r, axis.x, axis.y, axis.z, angle);
    MatrixMultiply(local, *local, r);
    local->_41 = pos.x;
    local->_42 = pos.y;
    local->_43 = pos.z;
    InvalidateWorldMatrix();
}

// 0x004fd090. Replaces the local rotation by the rotation about (x, y, z) by 'angle', keeping
// the local translation.
void SoultreeObject::SetRotation(float x, float y, float z, float angle)
{
    Vec3 pos;
    GetPosition(&pos);
    AxisAngleMatrix(&localMatrix, x, y, z, angle);
    SetPosition(pos);
}

// 0x004fd5c0. v * R(world): the upper 3x3 of the world matrix, no translation.
Vec3 SoultreeObject::LocalToWorldDirection(const Vec3& v)
{
    UpdateWorldMatrix();
    Vec3 r;
    r.x = v.y * worldMatrix._21;
    r.x += v.z * worldMatrix._31;
    r.x += v.x * worldMatrix._11;
    r.y = v.y * worldMatrix._22;
    r.y += v.x * worldMatrix._12;
    r.y += v.z * worldMatrix._32;
    r.z = v.y * worldMatrix._23;
    r.z += v.x * worldMatrix._13;
    r.z += v.z * worldMatrix._33;
    return r;
}

// 0x004fd660. p * worldMatrix with the translation row added. x and y are accumulated
// statement by statement like 0x004fd5c0; z is one expression whose first two products
// are parenthesised (docs/VC6_OPERAND_ORDER.md section 3), which issues retail's
// 'mov ecx,eax' (the result pointer) before the last faddp of z; z written as statements
// keeps the faddp first (164/168).
Vec3 SoultreeObject::LocalToWorldPoint(const Vec3& p)
{
    UpdateWorldMatrix();
    Vec3 r;
    r.x = p.y * worldMatrix._21;
    r.x += p.z * worldMatrix._31;
    r.x += p.x * worldMatrix._11;
    r.x += worldMatrix._41;
    r.y = p.y * worldMatrix._22;
    r.y += p.x * worldMatrix._12;
    r.y += p.z * worldMatrix._32;
    r.y += worldMatrix._42;
    r.z = (p.y * worldMatrix._23) + (p.x * worldMatrix._13) + p.z * worldMatrix._33 + worldMatrix._43;
    return r;
}

// In-place inverse of a rigid transform: transpose the rotation, then the translation
// becomes -(t * R^T). Stored through a Vec3 temporary into the translation row. Reading
// the translation into scalars z, y, x first gives retail's y-row term order and the
// second swap's integer move schedule (with m._41.. read in place: 273/285).
static inline void InvertRigid(Matrix4& m)
{
    TransposeRotation(m);
    float z = m._43, y = m._42, x = m._41;
    Vec3 t;
    t.x = -(x * m._11 + y * m._21 + z * m._31);
    t.y = -(x * m._12 + y * m._22 + z * m._32);
    t.z = -(x * m._13 + y * m._23 + z * m._33);
    m._41 = t.x;
    m._42 = t.y;
    m._43 = t.z;
}

// p * m with the translation row added.
static inline Vec3 TransformPoint(const Vec3& p, const Matrix4& m)
{
    Vec3 r;
    r.x = p.x * m._11 + p.y * m._21 + p.z * m._31 + m._41;
    r.y = p.x * m._12 + p.y * m._22 + p.z * m._32 + m._42;
    r.z = p.x * m._13 + p.y * m._23 + p.z * m._33 + m._43;
    return r;
}

// 0x004fd7f0. p * inverse(worldMatrix) for a rigid transform: copy worldMatrix, invert the
// copy in place, transform the point.
Vec3 SoultreeObject::WorldToLocalPoint(const Vec3& p)
{
    UpdateWorldMatrix();
    Matrix4 m = worldMatrix;
    InvertRigid(m);
    return TransformPoint(p, m);
}

// 0x004fd910.
void SoultreeObject::AddChild(SoultreeObject* child)
{
    if (child->parent)
        parent->RemoveChild(child);
    child->parent = this;
    if (!firstChild)
        firstChild = child;
    else
        firstChild->AppendSibling(child);
}

// 0x004fd960. Appends 'node' after the last sibling of this node.
void SoultreeObject::AppendSibling(SoultreeObject* node)
{
    if (nextSibling) {
        nextSibling->AppendSibling(node);
    } else {
        nextSibling = node;
        node->prevSibling = this;
    }
}

// 0x004fd990. Unlinks 'child' from this node's child list. The list search has no null
// check (the child is assumed to be present). Each link is cleared only if it is set.
void SoultreeObject::RemoveChild(SoultreeObject* child)
{
    child->InvalidateWorldMatrix();
    SoultreeObject* n = firstChild;
    while (n != child)
        n = n->nextSibling;
    if (n == firstChild)
        firstChild = n->nextSibling;
    if (n->nextSibling)
        n->nextSibling->prevSibling = n->prevSibling;
    if (n->prevSibling)
        n->prevSibling->nextSibling = n->nextSibling;
    if (n->parent)
        n->parent = 0;
    if (n->nextSibling)
        n->nextSibling = 0;
    if (n->prevSibling)
        n->prevSibling = 0;
}

// 0x004fda30. Nodes in this node's sibling chain and below it, this node included.
int SoultreeObject::CountNodes()
{
    int n = 0;
    if (nextSibling)
        n = nextSibling->CountNodes();
    if (firstChild)
        n += firstChild->CountNodes();
    return n + 1;
}

// 0x004fda60.
void SoultreeObject::CollectDescendants(int* count, SoultreeObject** out)
{
    SoultreeObject* c = firstChild;
    while (c) {
        out[*count] = c;
        c = c->nextSibling;
        (*count)++;
    }
    for (c = firstChild; c; c = c->nextSibling)
        c->CollectDescendants(count, out);
}

// 0x004fdab0.
void SoultreeObject::MarkSubtreeDirty()
{
    subtreeDirty = 1;
    for (SoultreeObject* p = parent; p; p = p->parent)
        p->subtreeDirty = 1;
}

// 0x004fdae0. Depth-first, case-insensitive: siblings before children.
SoultreeObject* SoultreeObject::FindByName(const char* n)
{
    SoultreeObject* found = 0;
    if (!_stricmp(name, n))
        return this;
    if (nextSibling) {
        found = nextSibling->FindByName(n);
        if (found)
            return found;
    }
    if (firstChild)
        found = firstChild->FindByName(n);
    return found;
}

// 0x004fdb40.
void SoultreeObject::SetFlag14c()
{
    field_0x14c = 1;
}

// 0x004fdb50.
void SoultreeObject::ClearFlag14c()
{
    field_0x14c = 0;
}

// 0x004fdb60.
void SoultreeObject::LoadFromParameters(UnknownParameterStream* stream, int offset)
{
    parameterBlock = new(__FILE__, 0x346) UnknownParameterBlock;
    parameterBlock->Attach(stream, offset, 1);
    UnknownVirtualSlot3();
    if (parameterBlock)
        delete parameterBlock;
}

// 0x004fdc00. Slot 2: reads a whole hierarchy from a binary stream: the node count, then per
// node its name, local matrix and bounds fields (new nodes come from slot 4; node 0 is this
// one), then one parent index per node (-1 = none, __FILE__ line 0x370).
void SoultreeObject::UnknownVirtualSlot2(SoultreeFileStream* stream)
{
    SoultreeObject* nodes[256];
    int i;
    stream->Read(&loadedNodeCount, 4, 1);
    nodes[0] = this;
    for (i = 0; i < loadedNodeCount; i++) {
        if (i != 0)
            UnknownVirtualSlot4(&nodes[i]);
        stream->Read(nodes[i]->name, 0x80, 1);
        stream->Read(&nodes[i]->localMatrix, 0x40, 1);
        stream->Read(&nodes[i]->field_0x14c, 4, 1);
        stream->Read(&nodes[i]->localBoundsValid, 4, 1);
        stream->Read(&nodes[i]->localBoundsA, 0xc, 1);
        stream->Read(&nodes[i]->localBoundsB, 0xc, 1);
        stream->Read(&nodes[i]->field_0x170, 4, 1);
        stream->Read(&nodes[i]->subtreeBoundsA, 0xc, 1);
        stream->Read(&nodes[i]->subtreeBoundsB, 0xc, 1);
        nodes[i]->parent = 0;
        nodes[i]->firstChild = 0;
        nodes[i]->nextSibling = 0;
        nodes[i]->prevSibling = 0;
        nodes[i]->worldValid = 0;
        nodes[i]->parameterBlock = 0;
    }
    int* parents = new(__FILE__, 0x370) int[loadedNodeCount];
    stream->Read(parents, 4, loadedNodeCount);
    for (i = 0; i < loadedNodeCount; i++) {
        if (parents[i] != -1)
            nodes[parents[i]]->AddChild(nodes[i]);
    }
}

// 0x004fddc0. Slot 3: reads the "Object Hierarchy" section of the parameter block
// parameterBlock: one row per node with its name, its parent's name and the 3x4 local matrix.
// Rows whose parent is not "NONE" are attached to the node of that name.
void SoultreeObject::UnknownVirtualSlot3()
{
    SoultreeObject* nodes[256];
    char parentNames[256][256];
    SoultreeObject* savedParent = parent;
    int count;
    int i, j;
    parent = 0;
    count = parameterBlock->BeginRows("Object Hierarchy");
    loadedNodeCount = count;
    nodes[0] = this;
    for (i = 1; i < count; i++)
        UnknownVirtualSlot4(&nodes[i]);
    for (i = 0; i < count; i++) {
        parameterBlock->ReadRow(0);
        parameterBlock->GetRowString(0, nodes[i]->name);
        parameterBlock->GetRowString(1, parentNames[i]);
        parameterBlock->GetRowFloat(2, &nodes[i]->localMatrix._11);
        parameterBlock->GetRowFloat(3, &nodes[i]->localMatrix._12);
        parameterBlock->GetRowFloat(4, &nodes[i]->localMatrix._13);
        parameterBlock->GetRowFloat(5, &nodes[i]->localMatrix._21);
        parameterBlock->GetRowFloat(6, &nodes[i]->localMatrix._22);
        parameterBlock->GetRowFloat(7, &nodes[i]->localMatrix._23);
        parameterBlock->GetRowFloat(8, &nodes[i]->localMatrix._31);
        parameterBlock->GetRowFloat(9, &nodes[i]->localMatrix._32);
        parameterBlock->GetRowFloat(10, &nodes[i]->localMatrix._33);
        parameterBlock->GetRowFloat(11, &nodes[i]->localMatrix._41);
        parameterBlock->GetRowFloat(12, &nodes[i]->localMatrix._42);
        parameterBlock->GetRowFloat(13, &nodes[i]->localMatrix._43);
    }
    for (i = 0; i < count; i++) {
        if (_stricmp(parentNames[i], "NONE")) {
            for (j = 0; j < count; j++) {
                if (!_stricmp(parentNames[i], nodes[j]->name))
                    nodes[j]->AddChild(nodes[i]);
            }
        }
    }
    parent = savedParent;
}

// 0x004fe020. Slot 4: the node factory used by the clone.
void SoultreeObject::UnknownVirtualSlot4(SoultreeObject** out)
{
    *out = new(__FILE__, 0x3aa) SoultreeObject(1);
}

// 0x004fe0a0.
void SoultreeObject::GetLocalBounds(Vec3* a, Vec3* b)
{
    if (!localBoundsValid)
        UnknownVirtualSlot5();
    *a = localBoundsA;
    *b = localBoundsB;
}

// 0x004fe0f0. Bounding box of this node (local bounds as center +- half size, when
// field_0x150 is set) grown by every child's subtree, stored as center and half extents.
void SoultreeObject::UpdateSubtreeBounds()
{
    UpdateWorldMatrix();
    Vec3 lo, hi;
    int have;
    lo.z = 0.0f;
    lo.y = 0.0f;
    lo.x = 0.0f;
    hi.z = 0.0f;
    hi.y = 0.0f;
    hi.x = 0.0f;
    have = 0;
    if (field_0x150) {
        have = 1;
        lo = localBoundsA - localBoundsB;
        hi = localBoundsA + localBoundsB;
    }
    for (SoultreeObject* c = firstChild; c; c = c->nextSibling)
        c->AccumulateBounds(&have, &lo, &hi, &worldMatrix);
    subtreeBoundsA = (hi + lo) * 0.5f;
    subtreeBoundsB = (hi - lo) * 0.5f;
}

// Vec3 whose (x, y, z) constructor is out of line in retail (0x00404e60, thiscall, ret 0xc);
// declared only, so VC6 emits the call. AccumulateBounds builds its upper corner through it.
struct SoultreeOolVec3 : public Vec3 {
    SoultreeOolVec3(float x_, float y_, float z_);
};

static inline float AbsValue(float v)
{
    return v < 0.0f ? -v : v;
}
static inline float MinValue(float a, float b)
{
    return a < b ? a : b;
}
static inline float MaxValue(float a, float b)
{
    return a > b ? a : b;
}

// p in m's space into *r: (p - translation) rotated by the transpose of m's upper 3x3. p is
// taken by value (AccumulateBounds passes its own centre as both).
static inline void FramePointTo(Vec3* r, Vec3 p, const Matrix4& m)
{
    p.x -= m._41;
    p.y -= m._42;
    p.z -= m._43;
    r->x = p.x * m._11 + p.y * m._12 + p.z * m._13;
    r->y = p.x * m._21 + p.y * m._22 + p.z * m._23;
    r->z = p.x * m._31 + p.y * m._32 + p.z * m._33;
}

// 0x004fe2e0. Grows (*lo, *hi) by this node's local box expressed in frame's space (the box's
// world centre and half-axes moved into frame, the half extents as the sum of |axis|), then
// recurses into the children. *have says whether the box already holds a point. The in-place
// transforms (x row from the original, y and z rows from the by-value copy), the out-of-line
// constructor for hi and the Min/Max helpers (the loaded corner stays on the x87 stack and is
// stored unconditionally) are all needed for the exact bytes.
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
        FramePointTo(&c, c, *frame);
        RotateByRowsTo(&ax, ax, *frame);
        RotateByRowsTo(&ay, ay, *frame);
        RotateByRowsTo(&az, az, *frame);
        Vec3 ext;
        ext.x = AbsValue(ax.x) + AbsValue(ay.x) + AbsValue(az.x);
        ext.y = AbsValue(ax.y) + AbsValue(ay.y) + AbsValue(az.y);
        ext.z = AbsValue(ax.z) + AbsValue(ay.z) + AbsValue(az.z);
        if (*have) {
            Vec3 l = c - ext;
            Vec3 h = SoultreeOolVec3(c.x + ext.x, c.y + ext.y, c.z + ext.z);
            lo->x = MinValue(lo->x, l.x);
            lo->y = MinValue(lo->y, l.y);
            lo->z = MinValue(lo->z, l.z);
            hi->x = MaxValue(hi->x, h.x);
            hi->y = MaxValue(hi->y, h.y);
            hi->z = MaxValue(hi->z, h.z);
        } else {
            *lo = c - ext;
            *hi = c + ext;
            *have = 1;
        }
    }
    for (SoultreeObject* child = firstChild; child; child = child->nextSibling)
        child->AccumulateBounds(have, lo, hi, frame);
}

// 0x004fe850.
void SoultreeObject::GetSubtreeBounds(Vec3* center, Vec3* extents)
{
    if (subtreeDirty)
        UpdateSubtreeBounds();
    *center = subtreeBoundsA;
    *extents = subtreeBoundsB;
}

// 0x004feb10. Slot 7: makes this node a parentless copy of src's subtree.
void SoultreeObject::UnknownVirtualSlot7(SoultreeObject* src)
{
    parent = 0;
    UnknownVirtualSlot8(src, this);
}

// 0x004feb30. Slot 8: copies src into dst, then clones src's later siblings and children
// through the factory (slot 4).
void SoultreeObject::UnknownVirtualSlot8(SoultreeObject* src, SoultreeObject* dst)
{
    strcpy(dst->name, src->name);
    dst->localMatrix = src->localMatrix;
    dst->worldValid = 0;
    dst->field_0x14c = src->field_0x14c;
    dst->localBoundsValid = 0;
    if (src->nextSibling) {
        UnknownVirtualSlot4(&dst->nextSibling);
        dst->nextSibling->parent = dst->parent;
        dst->nextSibling->prevSibling = dst;
        UnknownVirtualSlot8(src->nextSibling, dst->nextSibling);
    } else {
        dst->nextSibling = 0;
    }
    if (src->firstChild) {
        UnknownVirtualSlot4(&dst->firstChild);
        dst->firstChild->parent = dst;
        UnknownVirtualSlot8(src->firstChild, dst->firstChild);
    } else {
        dst->firstChild = 0;
    }
}

// 0x004fec30. GameObject slot 25: forwards to the children first.
int SoultreeObject::GameObjectVirtualSlot25(int a)
{
    for (SoultreeObject* c = firstChild; c; c = c->nextSibling)
        c->GameObjectVirtualSlot25(a);
    return GameObject::GameObjectVirtualSlot25(a);
}

// 0x004fec70. Slot 6: identity local matrices for this node, its later siblings and its
// descendants.
void SoultreeObject::UnknownVirtualSlot6()
{
    worldValid = 0;
    localMatrix = IdentityMatrix();
    if (nextSibling)
        nextSibling->UnknownVirtualSlot6();
    if (firstChild)
        firstChild->UnknownVirtualSlot6();
}

// 0x004fecd0.
void SoultreeObject::UpdateQuadtreeCell()
{
    Vec3 lo, hi;
    GetWorldBounds(&lo, &hi);
    unsigned int code = g_quadTree->ComputeCode(lo.x, lo.z, hi.x, hi.z);
    if (code != (unsigned int)quadtreeCell) {
        if (quadtreeCell != 15)
            g_quadTree->Remove(this, quadtreeCell);
        quadtreeCell = code;
        if (code != 15)
            g_quadTree->Insert(this, code, lo.y, hi.y);
    } else {
        g_quadTree->UpdateRange(this, quadtreeCell, lo.y, hi.y);
    }
}

// 0x004fed70.
void SoultreeObject::RemoveFromQuadtree()
{
    if (inQuadtree && g_quadTree && quadtreeCell != 15)
        g_quadTree->Remove(this, quadtreeCell);
    inQuadtree = 0;
    quadtreeCell = 15;
}

// 0x004fedb0. Appends this node to the node array unless it is listed already; the array
// grows by 20 entries (__FILE__ line 0x49b).
void SoultreeObject::RegisterNode()
{
    int i;
    for (i = 0; i < g_soultreeNodeCount; i++)
        if (g_soultreeNodes[i] == this)
            return;
    if (g_soultreeNodeCount + 1 > g_soultreeNodeCapacity) {
        g_soultreeNodes = (SoultreeObject**)DebugRealloc(g_soultreeNodes,
            (g_soultreeNodeCapacity + 20) * sizeof(SoultreeObject*), __FILE__, 0x49b);
        g_soultreeNodeCapacity += 20;
    }
    g_soultreeNodes[g_soultreeNodeCount] = this;
    g_soultreeNodeCount++;
}

// 0x004fee30. Removes this node from the node array (the last entry takes its place).
void SoultreeObject::UnregisterNode()
{
    int i;
    for (i = 0; i < g_soultreeNodeCount; i++) {
        if (g_soultreeNodes[i] == this) {
            if (i < g_soultreeNodeCount)
                g_soultreeNodes[i] = g_soultreeNodes[g_soultreeNodeCount - 1];
            g_soultreeNodeCount--;
            return;
        }
    }
}
