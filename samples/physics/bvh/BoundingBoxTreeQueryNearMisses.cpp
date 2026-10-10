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
// - SweptBoxOverlap 0x00424730 (891/891 bytes, 855 match): instruction-identical; only the
//   frame slots of half / moved / ext / delta differ (retail half, ext, delta, moved from esp
//   up; here half, moved, ext, delta).  The three axes must be one Vec3 array (separate
//   locals reorder every slot), the half extents a local copy written back to the by-value
//   parameter, and the clamps QueryMin / QueryMax (their parameter copy is retail's [esp+8]).
//
// - CapsuleTreeQuery 0x004298c0 (1418 vs 1436 bytes, 532 match): the frame (0x54), the
//   inline transforms and the out-of-line Vec3 operator calls match through the |dist| test
//   (the point branch needs its own block so its locals share the triangle branch's slots);
//   from there VC6 here assigns the operator result temporaries other slots and shares the
//   `return 1` of the PointInTriangle test, where retail returns inline.  Calls the
//   unreconstructed 0x0042a640 (SegmentSegmentDistance) for the three edges.
//
// - SegmentTriangleQuery 0x00427c10 (1073 vs 1101 bytes, 371 match): first draft.  The
//   segment is moved into the triangle's frame relative to the sweep record's offset, clipped
//   against the face plane, and on a hit inside the triangle the record's offset, contact,
//   scale, count and the contact normal list (0x00579068/0x00579014, deduplicated through
//   0x005299e0 unless 0x00579668 is set) are updated.  Retail reloads the record pointer
//   0x00579058 at every use and holds other registers; the two flag branches are literal
//   (the set-flag branch adds the push vector a second time and replaces the contact).
//
// Not attempted yet: BoxOverlap 0x00424ab0 (15-axis SAT through a jump table), 0x00425900 (capsule against
// box), BoxTriangleQuery 0x00426be0, TreeTreeQueryNodes
// 0x004280e0 (inlines the 15-axis SAT), 0x0042a640 (capsule against triangle).
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

// 0x00424730.  Overlap of box B (bCenter, bHalf, in A's frame through rel) with box A swept by
// the motion xfA: A's half extents rotated by xfA give the moved box's extents, the union of
// the box before and after the move (relative to aCenter) gives the swept centre offset and
// half extents, then BoxOverlap.
int SweptBoxOverlap(Vec3 aCenter, Vec3 aHalf, const Vec3* bCenter, const Vec3* bHalf,
                    const Matrix4* rel, const Matrix4* xfA)
{
    Vec3 axis[3];
    axis[0] = Vec3(aHalf.x, 0.0f, 0.0f);
    axis[1] = Vec3(0.0f, aHalf.y, 0.0f);
    axis[2] = Vec3(0.0f, 0.0f, aHalf.z);
    Vec3TransformNormal(&axis[0], axis[0], xfA);
    Vec3TransformNormal(&axis[1], axis[1], xfA);
    Vec3TransformNormal(&axis[2], axis[2], xfA);
    Vec3 half = aHalf;
    Vec3 ext;
    ext.x = QueryAbs(axis[0].x) + QueryAbs(axis[1].x) + QueryAbs(axis[2].x);
    ext.y = QueryAbs(axis[0].y) + QueryAbs(axis[1].y) + QueryAbs(axis[2].y);
    ext.z = QueryAbs(axis[0].z) + QueryAbs(axis[1].z) + QueryAbs(axis[2].z);
    Vec3 moved;
    TransformPointPtr(&moved, &aCenter, xfA);
    Vec3 delta;
    delta.x = moved.x - aCenter.x;
    delta.y = moved.y - aCenter.y;
    delta.z = moved.z - aCenter.z;
    for (int i = 0; i < 3; i++) {
        float lo = delta[i] - ext[i];
        float hi = ext[i] + delta[i];
        lo = QueryMin(-half[i], lo);
        hi = QueryMax(half[i], hi);
        moved[i] = (hi + lo) * 0.5f;
        half[i] = (hi - lo) * 0.5f;
    }
    aHalf = half;
    aCenter += moved;
    return BoxOverlap(&aCenter, &aHalf, *bCenter, *bHalf, rel);
}

// 0x004298c0.  Capsule (segment ends[0]..ends[1], radius) against the tree; the triangle
// branch clips the segment against the face plane, the point branch measures the distance
// from the transformed point to the segment.  See the header for what differs.
int CapsuleTreeQuery(const Vec3* ends, float radius, float radiusSq, QueryTreeNode* node,
                     const Matrix4* xf, int triangles)
{
    if (node->volume < 0.0f) {
        if (triangles) {
            QueryTriangle* tri = &((QueryTriangleLeaf*)node)->tri;
            Vec3 p;
            InverseTransformPointInline(&p, *ends, xf);
            Vec3 d;
            Vec3TransformNormalTranspose(&d, Vec3Sub(ends[1], ends[0]), xf);
            float t = -(Vec3Dot(tri->normal, p) + tri->planeOffset) / Vec3Dot(tri->normal, d);
            if (!(t < 1.0f))
                t = 1.0f;
            else if (t <= 0.0f)
                t = 0.0f;
            Vec3 c = Vec3Add(Vec3Scale(d, t), p);
            float dist = Vec3Dot(Vec3Sub(c, s_vertices[tri->vertex[0]]), tri->normal);
            if (QueryAbs(dist) <= radius) {
                Vec3 q = Vec3Sub(c, Vec3ScaleLeft(dist, tri->normal));
                if (PointInTriangle(&q, tri))
                    return 1;
                Vec3 e = Vec3Sub(s_vertices[tri->vertex[1]], s_vertices[tri->vertex[0]]);
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[0]], &e) > radius)
                    return 1;
                e = Vec3Sub(s_vertices[tri->vertex[2]], s_vertices[tri->vertex[1]]);
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[1]], &e) > radius)
                    return 1;
                e = Vec3Sub(s_vertices[tri->vertex[0]], s_vertices[tri->vertex[2]]);
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[2]], &e) > radius)
                    return 1;
            }
            return 0;
        } else {
            Vec3 p;
            TransformPointInline(&p, ((QueryPointLeaf*)node)->point, xf);
            Vec3 dir = Vec3Call(ends[1].x - ends[0].x, ends[1].y - ends[0].y, ends[1].z - ends[0].z);
            float t = Vec3Dot(dir, Vec3Sub(p, *ends)) / Vec3Dot(dir, dir);
            if (!(t < 1.0f))
                t = 1.0f;
            else if (t <= 0.0f)
                t = 0.0f;
            Vec3 r = Vec3Sub(p, Vec3Add(Vec3ScaleLeft(t, dir), *ends));
            if (Vec3LengthCall(&r) < radius)
                return 1;
            return 0;
        }
    }
    if (!BoxCapsuleOverlap(&node->center, &node->halfExtents, ends, radius, radiusSq, xf))
        return 0;
    return CapsuleTreeQuery(ends, radius, radiusSq, node->child[0], xf, triangles) ||
           CapsuleTreeQuery(ends, radius, radiusSq, node->child[1], xf, triangles);
}

// 0x00427c10.  Segment (moved by m) against a triangle, accumulating into the sweep record.
extern Vec3 g_CollisionScratchPoints[128];   // 0x00579068
extern int g_CollisionScratchCount;          // 0x00579014
extern int g_unknown579668;                  // 0x00579668

// The sweep record SegmentTriangleQuery accumulates into (the same pointer as
// g_CollisionBoxResult; CollisionShapeTests.h's CollisionSweepQuery).
struct QuerySweepRecord {
    Vec3AddAssignCall offset;       // +0x00
    unsigned char field_0x0c[0x0c];
    Vec3AddAssignCall contact;      // +0x18
    float scale;                    // +0x24
    int count;                      // +0x28
};

int Vec3Equal(const Vec3* a, const Vec3* b);   // 0x005299e0

int SegmentTriangleQuery(const QueryTriangle* tri, const Vec3* segment, const Matrix4* m)
{
    QuerySweepRecord* record = (QuerySweepRecord*)g_CollisionBoxResult;
    Vec3 start;
    TransformPointInline(&start, segment[0], m);
    start.x -= record->offset.x;
    start.y -= record->offset.y;
    start.z -= record->offset.z;
    Vec3 dir;
    TransformPointInline(&dir, segment[1], m);
    dir.x -= start.x;
    dir.y -= start.y;
    dir.z -= start.z;
    if (QueryDot(dir, tri->normal) <= 0.0f)
        return 0;
    float t = -((QueryDot(start, tri->normal) + tri->planeOffset) / QueryDot(dir, tri->normal));
    if (t < 0.0f || t > 1.0f)
        return 0;
    record->scale *= t;
    Vec3 step = Vec3Call(dir.x * t, dir.y * t, dir.z * t);
    Vec3 hit = Vec3Call(step.x + start.x, step.y + start.y, step.z + start.z);
    Vec3 push;
    if (!g_unknown579668) {
        Vec3 v = Vec3Call(step.x, step.y, step.z);
        float s = -QueryDot(v, tri->normal);
        push = Vec3Call(s * tri->normal.x, s * tri->normal.y, s * tri->normal.z);
    } else {
        Vec3 v = Vec3Call(step.x, step.y, step.z);
        push = Vec3ScaleLeft(-QueryDot(v, tri->normal), tri->normal);
    }
    if (!PointInTriangle(&hit, tri))
        return 0;
    record->offset += push;
    record->contact += hit;
    if (!g_unknown579668) {
        int i;
        for (i = 0; i < g_CollisionScratchCount; i++) {
            if (Vec3Equal(&g_CollisionScratchPoints[i], &tri->normal))
                break;
        }
        if (i >= g_CollisionScratchCount)
            g_CollisionScratchPoints[g_CollisionScratchCount++] = tri->normal;
        record->count++;
        return 1;
    }
    record->offset += push;
    (Vec3&)record->contact = hit;
    record->count = 1;
    g_CollisionScratchPoints[0] = tri->normal;
    g_CollisionScratchCount = 1;
    return 1;
}
