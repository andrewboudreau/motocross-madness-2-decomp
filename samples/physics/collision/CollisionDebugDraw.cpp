// CollisionDebugDraw.cpp -- debug drawing of the collision bounding-volume trees.
//
// TU ownership: tier 2.  These functions sit together (0x00432b30-0x004341a5) inside the
// CollisionObject.cpp address range.  They are kept in their own sample file only for
// readability.  DrawModel (0x00432b30) and DrawHull (0x00432d30) pick one of the tree
// draws by a mode number.
//
// Evidence (tier 1 unless noted):
//  * All are thiscall members of CollisionObject.  Every draw call goes through
//    `lea reg,[ecx+0xc]`, i.e. the GraphicsTest base (CollisionObject.h).
//  * The tree node is 0x24 bytes: a float at +0 compared with 0.0f (>= 0 means an
//    interior node, < 0 a leaf), a center/half-extent pair at +4/+0x10 (passed to the box
//    draw 0x0047c270), and two children at +0x1c/+0x20, both recursed into.  A triangle
//    leaf has three ushort vertex indices at +4/+6/+8 into a 12-byte vertex array and a
//    normal at +0xc.  A point-tree leaf keeps its point at +4.
//  * The trees are CollisionHullBody::field_0x188 / field_0x18c (CollisionShapeTests.h),
//    so CollisionBoxBounds is this node's box view.
//  * Retail turns the second recursive call into a loop (VC6 tail-recursion
//    elimination).  The source recurses into both children.
//
// Vector math: these functions use the Math3D Vec3.  Its constructor and operator+ are
// the out-of-line COMDATs 0x00404e60 and 0x00421cb0 that DrawTreeNormals calls.
// CollisionVec3 has no operator that constructs through the Vec3 constructor.
#include "CollisionShapeTests.h"   // CollisionObject, Math3D (Vec3, Matrix4)

// Tree node.  Names are tier 3; offsets are tier 1 (see above).
struct CollisionTreeNode {
    float field_0x00;              // >= 0: interior node (box valid); < 0: leaf
    Vec3 center;                   // +0x04 box center (a point leaf: the point)
    Vec3 halfExtents;              // +0x10
    CollisionTreeNode* child[2];   // +0x1c, +0x20
};

// Triangle leaf view of the same node (field_0x00 < 0).
struct CollisionTreeTriangle {
    float field_0x00;
    unsigned short vertex[3];      // +0x04, indices into the vertex array
    Vec3 normal;                   // +0x0c
};

// 0x00428db0 (cdecl, 3 args): moves the box (center, half extents) by the second
// matrix in place.  The motion draw calls it between two DrawBox calls.  Tier 3 name.
void Fn_00428db0(Vec3* center, Vec3* halfExtents, const Matrix4* m);

// v * M + translation row.  This has the same signature and result as the out-of-line
// 0x0042a510 (out, v by value, m).  DrawTreeNormals calls that copy for its fourth
// transform, because VC6 stops inlining there (inline budget).  The _RC member names
// matter: the m[r][c] form costs more inline budget, and then the later operator* is
// not inlined either.
inline void DebugTransformPoint(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// The same transform with v by reference: retail reads the source vector in place,
// with no stack copy, in DrawPointTree's first transform.
inline void DebugTransformPointRef(Vec3* out, const Vec3& v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// v * M3x3 (no translation), like 0x0042a450.
inline void DebugRotateVector(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// 0x00433240 (ret 0x10).  Draws the tree `depth` levels down: at depth 0 a box node is
// drawn red and a triangle leaf blue.  Leaves above that depth are skipped.
void CollisionObject::DrawTree(CollisionTreeNode* node, int depth, const Matrix4* xf,
                               const Vec3* verts)
{
    if (depth == 0) {
        if (node->field_0x00 >= 0.0f) {
            SetDrawColor(0xff, 0, 0);
            DrawBox(&node->center, &node->halfExtents, xf);
        } else {
            SetDrawColor(0, 0, 0xff);
            const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
            Vec3 p[3];
            DebugTransformPoint(&p[0], verts[tri->vertex[0]], xf);
            DebugTransformPoint(&p[1], verts[tri->vertex[1]], xf);
            DebugTransformPoint(&p[2], verts[tri->vertex[2]], xf);
            DrawLine(&p[0], &p[1]);
            DrawLine(&p[1], &p[2]);
            DrawLine(&p[2], &p[0]);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawTree(node->child[0], depth - 1, xf, verts);
        DrawTree(node->child[1], depth - 1, xf, verts);
    }
}

// 0x004334c0 (ret 0x14).  Like DrawTree, but also shows where `motion` moves each node.
// A box is drawn, moved by 0x00428db0 and drawn again.  A triangle is drawn and each
// corner is joined (in red) to its image under `motion`.
void CollisionObject::DrawTreeMotion(CollisionTreeNode* node, int depth, const Matrix4* xf,
                                     const Matrix4* motion, const Vec3* verts)
{
    if (depth == 0) {
        if (node->field_0x00 < 0.0f) {
            SetDrawColor(0, 0, 0xff);
        } else {
            SetDrawColor(0xff, 0, 0);
        }
        Vec3 center = node->center;
        Vec3 halfExtents = node->halfExtents;
        if (node->field_0x00 >= 0.0f) {
            DrawBox(&center, &halfExtents, xf);
            Fn_00428db0(&center, &halfExtents, motion);
            DrawBox(&center, &halfExtents, xf);
        } else {
            SetDrawColor(0, 0, 0xff);
            const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
            Vec3 p[3];
            Vec3 q[3];
            DebugTransformPoint(&p[0], verts[tri->vertex[0]], xf);
            DebugTransformPoint(&p[1], verts[tri->vertex[1]], xf);
            DebugTransformPoint(&p[2], verts[tri->vertex[2]], xf);
            // q reads p in place (no stack copy), hence the by-reference transform.
            DebugTransformPointRef(&q[0], p[0], motion);
            DebugTransformPointRef(&q[1], p[1], motion);
            DebugTransformPointRef(&q[2], p[2], motion);
            DrawLine(&p[0], &p[1]);
            DrawLine(&p[1], &p[2]);
            DrawLine(&p[2], &p[0]);
            SetDrawColor(0xff, 0, 0);
            DrawLine(&p[0], &q[0]);
            DrawLine(&p[1], &q[1]);
            DrawLine(&p[2], &q[2]);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawTreeMotion(node->child[0], depth - 1, xf, motion, verts);
        DrawTreeMotion(node->child[1], depth - 1, xf, motion, verts);
    }
}

// 0x00433930 (ret 0xc).  Draws every leaf point of a point tree in green.  With a motion
// matrix, each point is drawn with its moved image and a line between them.
void CollisionObject::DrawPointTree(CollisionTreeNode* node, const Matrix4* xf,
                                    const Matrix4* motion)
{
    if (node->field_0x00 >= 0.0f) {
        DrawPointTree(node->child[0], xf, motion);
        DrawPointTree(node->child[1], xf, motion);
    } else {
        Vec3 p = node->center;
        if (motion) {
            Vec3 q;
            DebugTransformPointRef(&q, p, motion);
            DebugTransformPoint(&p, node->center, xf);
            DebugTransformPoint(&q, q, xf);
            SetDrawColor(0, 0xff, 0);
            DrawLine(&p, &q);
            DrawMarker(&p, 0.05f);
            DrawMarker(&q, 0.05f);
        } else {
            SetDrawColor(0, 0xff, 0);
            DebugTransformPoint(&p, p, xf);
            DrawMarker(&p, 0.25f);
        }
    }
}

// 0x00433be0 (ret 0xc).  For every triangle leaf: its corners as green markers, its edges
// in blue, and a red line of length 0.1 from its centroid along its normal.
void CollisionObject::DrawTreeNormals(CollisionTreeNode* node, const Vec3* verts,
                                      const Matrix4* xf)
{
    if (node->field_0x00 >= 0.0f) {
        DrawTreeNormals(node->child[0], verts, xf);
        DrawTreeNormals(node->child[1], verts, xf);
    } else {
        const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
        Vec3 a = verts[tri->vertex[0]];
        Vec3 b = verts[tri->vertex[1]];
        Vec3 c = verts[tri->vertex[2]];
        Vec3 center = (a + b + c) * (1.0f / 3.0f);
        Vec3 n = tri->normal;
        DebugRotateVector(&n, n, xf);
        SetDrawColor(0, 0xff, 0);
        DebugTransformPoint(&a, a, xf);
        DebugTransformPoint(&b, b, xf);
        DebugTransformPoint(&c, c, xf);
        DebugTransformPoint(&center, center, xf);
        n = center + n * 0.1f;   // retail reuses the normal's stack slot for the tip
        DrawMarker(&a, 0.1f);
        DrawMarker(&b, 0.1f);
        DrawMarker(&c, 0.1f);
        SetDrawColor(0, 0, 0xff);
        DrawLine(&a, &b);
        DrawLine(&b, &c);
        DrawLine(&c, &a);
        SetDrawColor(0xff, 0, 0);
        DrawLine(&center, &n);
    }
}

// 0x00434040 (ret 0xc).  Like DrawTree, for a point tree: at depth 0 a box node is drawn
// red and a leaf point as a magenta marker.
void CollisionObject::DrawBoxTree(CollisionTreeNode* node, int depth, const Matrix4* xf)
{
    if (depth == 0) {
        if (node->field_0x00 < 0.0f) {
            SetDrawColor(0, 0, 0xff);
        } else {
            SetDrawColor(0xff, 0, 0);
        }
        if (node->field_0x00 >= 0.0f) {
            DrawBox(&node->center, &node->halfExtents, xf);
        } else {
            SetDrawColor(0xff, 0, 0xff);
            Vec3 p = node->center;
            DebugTransformPoint(&p, p, xf);
            DrawMarker(&p, 0.25f);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawBoxTree(node->child[0], depth - 1, xf);
        DrawBoxTree(node->child[1], depth - 1, xf);
    }
}

// 0x00432d30 (ret 0xc).  Draws one hull's trees; `mode` (1..7) picks the view.  Mode 4
// also draws the tree under field_0x88 * field_0x108.
void CollisionObject::DrawHull(CollisionHullBody* hull, int depth, int mode)
{
    if (mode == 1) {
        DrawTree((CollisionTreeNode*)hull->field_0x188, depth, &hull->field_0x48,
                 (const Vec3*)hull->field_0x190);
    } else if (mode == 2) {
        if (hull->field_0x18c) {
            DrawBoxTree((CollisionTreeNode*)hull->field_0x18c, depth, &hull->field_0x48);
        }
    } else if (mode == 3) {
        DrawTreeMotion((CollisionTreeNode*)hull->field_0x188, depth, &hull->field_0x48,
                       &hull->field_0x08, (const Vec3*)hull->field_0x190);
    } else if (mode == 4) {
        DrawTreeMotion((CollisionTreeNode*)hull->field_0x188, depth, &hull->field_0x48,
                       &hull->field_0x08, (const Vec3*)hull->field_0x190);
        // m = field_0x88 * field_0x108 (row vectors), written out here.  Retail's term
        // order comes out only with the product in this function through local
        // references declared after m; an inline helper, or m declared last, gives a
        // different FPU order.
        Matrix4 m;
        const Matrix4& a = hull->field_0x88;
        const Matrix4& b = hull->field_0x108;
        m._11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
        m._12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
        m._13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
        m._14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
        m._21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
        m._22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
        m._23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
        m._24 = a._21 * b._14 + a._22 * b._24 + a._23 * b._34 + a._24 * b._44;
        m._31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
        m._32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
        m._33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
        m._34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
        m._41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
        m._42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
        m._43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
        m._44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
        DrawTree((CollisionTreeNode*)hull->field_0x188, depth, &m,
                 (const Vec3*)hull->field_0x190);
    } else if (mode == 5) {
        if (hull->field_0x18c) {
            DrawPointTree((CollisionTreeNode*)hull->field_0x18c, &hull->field_0x48,
                          &hull->field_0x08);
        }
    } else if (mode == 6) {
        if (hull->field_0x18c) {
            DrawPointTree((CollisionTreeNode*)hull->field_0x18c, &hull->field_0x48,
                          &hull->field_0x148);
        }
    } else if (mode == 7) {
        DrawTreeNormals((CollisionTreeNode*)hull->field_0x188, (const Vec3*)hull->field_0x190,
                        &hull->field_0x48);
    }
}

// 0x00432b30 (ret 0xc).  Draws a model's bounds in green, then each element hull.  Mode 1
// draws each element's point-tree box, colored by the two per-element flag arrays.
void CollisionObject::DrawModel(CollisionModelBody* model, int depth, int mode)
{
    SetDrawColor(0, 0xff, 0);
    DrawBox((const Vec3*)&model->center, (const Vec3*)&model->halfExtents, &model->field_0x88);
    for (int i = 0; i < model->elementCount; i++) {
        if (mode == 1) {
            if (model->elements[i].field_0x18c) {
                SetDrawColor(0, 0, 0xff);
                if (!model->elementEnabled[i]) {
                    SetDrawColor(0xff, 0, 0);
                }
                if (model->field_0x08[i]) {
                    SetDrawColor(0, 0xff, 0);
                }
                CollisionTreeNode* box = (CollisionTreeNode*)model->elements[i].field_0x18c;
                DrawBox(&box->center, &box->halfExtents, &model->elements[i].field_0x48);
            }
        } else if (mode == 2) {
            if (model->elements[i].field_0x18c) {
                DrawTree((CollisionTreeNode*)model->elements[i].field_0x188, depth, &model->elements[i].field_0x48,
                         (const Vec3*)model->elements[i].field_0x190);
            }
        } else if (mode == 3) {
            if (model->elements[i].field_0x18c) {
                DrawBoxTree((CollisionTreeNode*)model->elements[i].field_0x18c, depth, &model->elements[i].field_0x48);
            }
        } else if (mode == 4) {
            DrawTreeMotion((CollisionTreeNode*)model->elements[i].field_0x188, depth, &model->elements[i].field_0x48,
                           &model->elements[i].field_0x08, (const Vec3*)model->elements[i].field_0x190);
        } else if (mode == 5) {
            if (model->elements[i].field_0x18c) {
                DrawPointTree((CollisionTreeNode*)model->elements[i].field_0x18c, &model->elements[i].field_0x48,
                              &model->elements[i].field_0x08);
            }
        } else if (mode == 6) {
            if (model->elements[i].field_0x18c) {
                DrawPointTree((CollisionTreeNode*)model->elements[i].field_0x18c, &model->elements[i].field_0x48, 0);
            }
        } else if (mode == 7) {
            DrawTreeNormals((CollisionTreeNode*)model->elements[i].field_0x188,
                            (const Vec3*)model->elements[i].field_0x190, &model->elements[i].field_0x48);
        }
    }
}
