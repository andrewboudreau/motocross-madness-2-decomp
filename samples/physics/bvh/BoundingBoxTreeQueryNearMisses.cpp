// BoundingBoxTreeQueryNearMisses.cpp -- near misses of the run-time box-tree query unit
// (src/krusty2/bvh/BoundingBoxTreeQuery.cpp, 0x00424690..0x0042ad2f).  Names are tier 3.
//
// Status (VC6 SP3, vc6_o2_ml; tools/run_physics_samples.py scores):
// - PointInTriangle 0x004278d0 (830/830 bytes, 93%): instruction-identical except where VC6
//   schedules two copies and the `lea ebx` of the normal pointer in the first half.
// - SphereTreeQuery 0x00429570 (831 vs 789 bytes): retail shares one `return 0` / `return 1`
//   tail between the triangle and point branches and keeps the second recursive call
//   un-merged (it returns the first child's nonzero result as is); this source gives
//   separate epilogues.  An if / else-if form with `int hit = child0; if (!hit) hit = child1;`
//   gives retail's block order and tails but moves the triangle branch's stack slots.
// - GrowBoxByTransformedBox 0x004290d0 (813 vs 805 bytes): one store of the transformed
//   centre's x, which retail keeps on the FPU stack, and stack slots.  The x and z rows of the
//   transform need the parenthesised grouping below (MoveBox prefers the same grouping).
// - MoveBox 0x00428db0 (797 vs 789 bytes): the GrowBoxByTransformedBox body on pointer
//   arguments; stack slots of the axis temporaries and of the moved centre differ.
//
// LeafPairQuery 0x004275f0, LeafNodeQuery 0x004269e0 and SegmentTreeQuery 0x00429e90 are
// exact in the src file.
//
// Not attempted yet: 0x00424730 (box swept by a motion matrix, then BoxOverlap),
// BoxOverlap 0x00424ab0 (15-axis SAT through a jump table), 0x00425900 (capsule against
// box), BoxTriangleQuery 0x00426be0, SegmentTriangleQuery 0x00427c10, TreeTreeQueryNodes
// 0x004280e0 (inlines the 15-axis SAT), CapsuleTreeQuery 0x004298c0 (calls most Vec3
// helpers out of line), 0x0042a640 (capsule against triangle).
#include "bvh/BoundingBoxTreeQuery.h"

// The unit's file statics (bound to the same addresses as in the src file).
static Vec3* s_vertices;            // 0x00578f4c
static Matrix4 s_relativeAB;        // 0x00578ef0
static Matrix4 s_relativeBA;        // 0x00578eb0
static Matrix4 s_motion;            // 0x00579018
static int s_mode;                  // 0x0057905c
extern QueryHit* g_CollisionBoxResult;   // 0x00579058

// 0x004278d0.  Nonzero when p (already in the triangle's plane) lies inside the triangle.
int PointInTriangle(const Vec3* p, const QueryTriangle* tri)
{
    Vec3 e0 = s_vertices[tri->vertex[2]] - s_vertices[tri->vertex[1]];
    Vec3 e1 = s_vertices[tri->vertex[0]] - s_vertices[tri->vertex[1]];
    Vec3 d = *p - s_vertices[tri->vertex[1]];
    Vec3 c = CrossProduct(e0, d);
    if (QueryDot(c, tri->normal) < 0.0f)
        return 0;
    c = CrossProduct(d, e1);
    if (QueryDot(c, tri->normal) < 0.0f)
        return 0;
    e0 = SubtractCtorCall(s_vertices[tri->vertex[0]], s_vertices[tri->vertex[2]]);
    e1 = SubtractCtorCall(s_vertices[tri->vertex[1]], s_vertices[tri->vertex[2]]);
    d = SubtractCtorCall(*p, s_vertices[tri->vertex[2]]);
    c = CrossProductCall(e0, d);
    if (Vec3DotCall(&tri->normal, &c) < 0.0f)
        return 0;
    c = CrossProductCall(d, e1);
    if (Vec3DotCall(&tri->normal, &c) < 0.0f)
        return 0;
    return 1;
}

// 0x00429570.  Sphere (world centre, radius) against a tree: a triangle tree when
// triangles != 0 (the vertices are in s_vertices), else a point tree.
int SphereTreeQuery(const Vec3* center, float radius, float radiusSq, QueryTreeNode* node,
                    const Matrix4* xf, int triangles)
{
    if (node->volume < 0.0f) {
        if (triangles) {
            QueryTriangle* tri = &((QueryTriangleLeaf*)node)->tri;
            Vec3 p = *center;
            InverseTransformPoint(&p, p, xf);
            const Vec3& v0 = s_vertices[tri->vertex[0]];
            Vec3 d = Vec3Call(p.x - v0.x, p.y - v0.y, p.z - v0.z);
            float dist = QueryDot(d, tri->normal);
            if (dist > radius)
                return 0;
            Vec3 t, q;
            q = *Vec3SubtractCall(&q, &p, Vec3ScaleCall(&t, &tri->normal, dist));
            if (PointInTriangle(&q, tri))
                return 1;
            if (PointSegmentDistance(&p, &s_vertices[tri->vertex[0]], &s_vertices[tri->vertex[1]]) < radiusSq)
                return 1;
            if (PointSegmentDistance(&p, &s_vertices[tri->vertex[1]], &s_vertices[tri->vertex[2]]) < radiusSq)
                return 1;
            if (PointSegmentDistance(&p, &s_vertices[tri->vertex[2]], &s_vertices[tri->vertex[0]]) < radiusSq)
                return 1;
            return 0;
        }
        Vec3 p = ((QueryPointLeaf*)node)->point;
        Vec3 w;
        w.x = p.x * xf->_11 + p.y * xf->_21 + p.z * xf->_31 + xf->_41;
        w.y = p.x * xf->_12 + p.y * xf->_22 + p.z * xf->_32 + xf->_42;
        w.z = p.x * xf->_13 + p.y * xf->_23 + p.z * xf->_33 + xf->_43;
        Vec3 d = w - *center;
        if (QueryDot(d, d) < radiusSq)
            return 1;
        return 0;
    }
    if (!BoxSphereOverlap(&node->center, &node->halfExtents, *center, radius, radiusSq, xf))
        return 0;
    return SphereTreeQuery(center, radius, radiusSq, node->child[0], xf, triangles) ||
           SphereTreeQuery(center, radius, radiusSq, node->child[1], xf, triangles);
}

inline void TransformPointGB(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + (v.y * m->_21 + v.z * m->_31) + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + (v.y * m->_23 + v.z * m->_33) + m->_43;
}
// 0x004290d0.  Grows the box (outCenter, outHalfExtents) to contain the box (center,
// halfExtents) moved by m.
void GrowBoxByTransformedBox(Vec3* outCenter, Vec3* outHalfExtents, Vec3 center,
                             Vec3 halfExtents, const Matrix4* m)
{
    Vec3 axes[3];
    RotateVectorInline(&axes[0], Vec3(halfExtents.x, 0.0f, 0.0f), m);
    RotateVectorInline(&axes[1], Vec3(0.0f, halfExtents.y, 0.0f), m);
    Vec3 grown;
    grown.x = outHalfExtents->x;
    grown.y = outHalfExtents->y;
    grown.z = outHalfExtents->z;
    RotateVectorInline(&axes[2], Vec3(0.0f, 0.0f, halfExtents.z), m);
    Vec3& ax = axes[0];
    Vec3& ay = axes[1];
    Vec3& az = axes[2];
    Vec3 half;
    half.x = QueryAbs(ax.x) + QueryAbs(ay.x) + QueryAbs(az.x);
    half.y = QueryAbs(ax.y) + QueryAbs(ay.y) + QueryAbs(az.y);
    half.z = QueryAbs(ax.z) + QueryAbs(ay.z) + QueryAbs(az.z);
    TransformPointGB(&center, center, m);
    Vec3 d;
    d.x = center.x - outCenter->x;
    d.y = center.y - outCenter->y;
    d.z = center.z - outCenter->z;
    for (int i = 0; i < 3; i++) {
        float lo = d[i] - half[i];
        float hi = half[i] + d[i];
        if (-grown[i] < lo)
            lo = -grown[i];
        hi = QueryMax(grown[i], hi);
        center[i] = (hi + lo) * 0.5f;
        grown[i] = (hi - lo) * 0.5f;
    }
    *outCenter += center;
    *outHalfExtents = grown;
}

inline void TransformPointPtrInline(Vec3* out, const Vec3* v, const Matrix4* m)
{
    out->x = v->x * m->_11 + (v->y * m->_21 + v->z * m->_31) + m->_41;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42;
    out->z = v->x * m->_13 + (v->y * m->_23 + v->z * m->_33) + m->_43;
}
// 0x00428db0.  Grows the box (center, halfExtents) in place to also contain itself moved by m.
void MoveBox(Vec3* center, Vec3* halfExtents, const Matrix4* m)
{
    Vec3 hx(halfExtents->x, 0.0f, 0.0f);
    Vec3 hy(0.0f, halfExtents->y, 0.0f);
    Vec3 hz(0.0f, 0.0f, halfExtents->z);
    Vec3 axes[3];
    RotateVectorInline(&axes[0], hx, m);
    RotateVectorInline(&axes[1], hy, m);
    Vec3 grown;
    grown.x = halfExtents->x;
    grown.y = halfExtents->y;
    grown.z = halfExtents->z;
    RotateVectorInline(&axes[2], hz, m);
    Vec3& ax = axes[0];
    Vec3& ay = axes[1];
    Vec3& az = axes[2];
    Vec3 half;
    half.x = QueryAbs(ax.x) + QueryAbs(ay.x) + QueryAbs(az.x);
    half.y = QueryAbs(ax.y) + QueryAbs(ay.y) + QueryAbs(az.y);
    half.z = QueryAbs(ax.z) + QueryAbs(ay.z) + QueryAbs(az.z);
    Vec3 moved;
    TransformPointPtrInline(&moved, center, m);
    Vec3 d;
    d.x = moved.x - center->x;
    d.y = moved.y - center->y;
    d.z = moved.z - center->z;
    for (int i = 0; i < 3; i++) {
        float lo = d[i] - half[i];
        float hi = half[i] + d[i];
        if (-grown[i] < lo)
            lo = -grown[i];
        hi = QueryMax(grown[i], hi);
        moved[i] = (hi + lo) * 0.5f;
        grown[i] = (hi - lo) * 0.5f;
    }
    *center += moved;
    *halfExtents = grown;
}
