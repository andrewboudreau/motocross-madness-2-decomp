// Group0_BvhQuery.cpp -- the game build's copy of the run-time box-tree query unit
// (retail 0x00424690..0x0042ad2f): the canonical src/krusty2/bvh/BoundingBoxTreeQuery.cpp,
// included whole (config/game_build.json included_sources), followed by the five functions
// it does not define yet:
//   BoxCapsuleOverlap      0x00425900
//   BoxTriangleQuery       0x00426be0
//   SegmentTriangleQuery   0x00427c10
//   TreeTreeQueryNodes     0x004280e0
//   SegmentSegmentDistance 0x0042a640
// Status: best-guess-unreviewed.  The bodies below are the near misses of
// samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp, unchanged; that file keeps the
// per-function notes and remains the calibration source.  Retail places them in the same
// unit as the query state (the vertex array, the relative frames, the motion matrix and
// the axis scratch), so they are compiled here beside the canonical file statics instead of
// beside the sample's own copies.  The out-of-line call views they declare (Vec3Dot, Vec3Sub,
// Vec3Add, Vec3Scale, Vec3LengthRef, Vec3Equal) resolve through config/game_build.json
// link_aliases to the existing providers of the same retail addresses.
#include "../krusty2/bvh/BoundingBoxTreeQuery.cpp"

// 0x00579668: read by SegmentTriangleQuery and never written anywhere in retail (.bss, so
// always 0); outside this unit's .bss range, owner unknown.  Defined here for the link.
int g_unknown579668;

// v / |v| (returns v when |v|^2 == 1): the result of Math3D.h's Vec3Normalize
// (0x005087b0), written with a named result (BoxCapsuleOverlap is closer to retail with
// it; see the file header).
inline Vec3 Vec3NormalizeInline(const Vec3& v)
{
    Vec3 r;
    float sq = DotProduct(v, v);
    if (sq == 1.0f) {
        r = v;
    } else {
        float inv = FastInvSqrt(sq);
        r = v * inv;
    }
    return r;
}

inline float QuerySigned(float s, float p) { return p < 0.0f ? -s : s; }
// Component-wise squares of the corner distances (BoxCapsuleOverlap).
inline Vec3 QuerySquares(float x, float y, float z)
{
    return Vec3(x * x, y * y, z * z);
}

// 0x00425900.  Box (center, halfExtents) against a capsule (segment ends[0]..ends[1] moved
// by the inverse of xf, radius): the box axes, the three axes crossed with the normalised
// segment direction, then for the xy, xz and yz planes the direction towards the corner
// nearer to one of the segment's ends (squared distances, signed by that end's position).
// First draft; see the file header.
int BoxCapsuleOverlap(const Vec3* center, const Vec3* halfExtents, const Vec3* ends,
                      float radius, float radiusSq, const Matrix4* xf)
{
    Vec3 p0, p1;
    InverseTransformPointInline(&p0, ends[0], xf);
    InverseTransformPointInline(&p1, ends[1], xf);
    p0 -= *center;
    p1 -= *center;
    Vec3 mid = (p0 + p1) * 0.5f;
    Vec3 halfSegment = mid - p0;
    if (QueryAbs(mid.x) > QueryAbs(halfSegment.x) + halfExtents->x + radius)
        return 0;
    if (QueryAbs(mid.y) > QueryAbs(halfSegment.y) + halfExtents->y + radius)
        return 0;
    if (QueryAbs(mid.z) > QueryAbs(halfSegment.z) + radius + halfExtents->z)
        return 0;
    Vec3 boxAxes[3];
    boxAxes[0] = Vec3(halfExtents->x, 0.0f, 0.0f);
    boxAxes[1] = Vec3(0.0f, halfExtents->y, 0.0f);
    boxAxes[2] = Vec3(0.0f, 0.0f, halfExtents->z);
    Vec3 dir = Vec3NormalizeInline(p1 - p0);
    Vec3 axis;
    axis = Vec3NormalizeInline(Vec3(-dir.z, 0.0f, dir.x));
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    axis = Vec3NormalizeInline(Vec3(dir.y, -dir.x, 0.0f));
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    axis = Vec3NormalizeInline(Vec3(0.0f, dir.z, -dir.y));
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    float c0x = QueryAbs(halfExtents->x) - QueryAbs(p0.x);
    float c0y = QueryAbs(halfExtents->y) - QueryAbs(p0.y);
    float c0z = QueryAbs(halfExtents->z) - QueryAbs(p0.z);
    float c1x = QueryAbs(halfExtents->x) - QueryAbs(p1.x);
    float c1y = QueryAbs(halfExtents->y) - QueryAbs(p1.y);
    float c1z = QueryAbs(halfExtents->z) - QueryAbs(p1.z);
    Vec3 s0 = QuerySquares(c0x, c0y, c0z);
    Vec3 s1 = QuerySquares(c1x, c1y, c1z);
    Vec3 cornerDir;
    if (s0.x + s0.y < s1.x + s1.y) {
        cornerDir.x = QuerySigned(s0.x, p0.x);
        cornerDir.y = QuerySigned(s0.y, p0.y);
    } else {
        cornerDir.x = QuerySigned(s1.x, p1.x);
        cornerDir.y = QuerySigned(s1.y, p1.y);
    }
    cornerDir.z = 0.0f;
    axis = Vec3NormalizeInline(cornerDir);
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    if (s0.x + s0.z < s1.x + s1.z) {
        cornerDir.x = QuerySigned(s0.x, p0.x);
        cornerDir.z = QuerySigned(s0.z, p0.z);
    } else {
        cornerDir.x = QuerySigned(s1.x, p1.x);
        cornerDir.z = QuerySigned(s1.z, p1.z);
    }
    cornerDir.y = 0.0f;
    axis = Vec3NormalizeInline(cornerDir);
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    if (s0.y + s0.z < s1.y + s1.z) {
        cornerDir.y = QuerySigned(s0.y, p0.y);
        cornerDir.z = QuerySigned(s0.z, p0.z);
    } else {
        cornerDir.y = QuerySigned(s1.y, p1.y);
        cornerDir.z = QuerySigned(s1.z, p1.z);
    }
    cornerDir.x = 0.0f;
    axis = Vec3NormalizeInline(cornerDir);
    if (QueryAbs(axis.z * mid.z + (axis.x * mid.x + axis.y * mid.y)) >
            QueryAbs(DotProduct(axis, boxAxes[0])) + QueryAbs(DotProduct(axis, boxAxes[1]))
            + QueryAbs(DotProduct(axis, boxAxes[2]))
            + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    return 1;
}

// 0x00426be0.  Box (center, halfExtents) against a triangle moved by m: the 13-axis
// separating-axis test (face normal, the box axes, the box axes crossed with two edges and
// with the normalised third edge).  The third edge is v[2] - v[0] (the same direction as the
// second one, negated): retail's choice, kept.
int BoxTriangleQuery(const Vec3* center, const Vec3* halfExtents, const QueryTriangle* tri,
                     const Matrix4* m)
{
    Vec3 v[3];
    TransformPointInline(&v[0], s_vertices[tri->vertex[0]], m);
    TransformPointInline(&v[1], s_vertices[tri->vertex[1]], m);
    TransformPointInline(&v[2], s_vertices[tri->vertex[2]], m);
    Vec3 faceNormal;
    RotateVectorInline(&faceNormal, tri->normal, m);
    Vec3 boxAxes[3];
    boxAxes[0] = Vec3(halfExtents->x, 0.0f, 0.0f);
    boxAxes[1] = Vec3(0.0f, halfExtents->y, 0.0f);
    boxAxes[2] = Vec3(0.0f, 0.0f, halfExtents->z);
    Vec3 offset = v[0] - *center;
    Vec3 e0, e1, e2;
    for (int i = 0; i < 13; i++) {
        switch (i) {
        case 0: s_axes[0] = faceNormal; break;
        case 1: s_axes[1] = Vec3(1.0f, 0.0f, 0.0f); break;
        case 2: s_axes[2] = Vec3(0.0f, 1.0f, 0.0f); break;
        case 3: s_axes[3] = Vec3(0.0f, 0.0f, 1.0f); break;
        case 4: s_axes[4] = CrossProduct(s_axes[1], e0); break;
        case 5: s_axes[5] = CrossProduct(s_axes[2], e0); break;
        case 6: s_axes[6] = CrossProduct(s_axes[3], e0); break;
        case 7: s_axes[7] = CrossProduct(s_axes[1], e1); break;
        case 8: s_axes[8] = CrossProduct(s_axes[2], e1); break;
        case 9: s_axes[9] = CrossProduct(s_axes[3], e1); break;
        case 10:
            e2 = Vec3NormalizeInline(v[2] - v[0]);
            s_axes[10] = CrossProduct(s_axes[1], e2);
            break;
        case 11: s_axes[11] = CrossProduct(s_axes[2], e2); break;
        case 12: s_axes[12] = CrossProduct(s_axes[3], e2); break;
        }
        float boxRadius = QueryAbs(DotProduct(boxAxes[0], s_axes[i]))
                          + QueryAbs(DotProduct(boxAxes[1], s_axes[i]))
                          + QueryAbs(DotProduct(boxAxes[2], s_axes[i]));
        if (i == 0) {
            if (QueryAbs(DotProduct(offset, s_axes[0])) > boxRadius)
                return 0;
            e0 = v[0] - v[1];
            e1 = v[0] - v[2];
        } else {
            float p0 = DotProduct(offset, s_axes[i]);
            float d1 = DotProduct(e0, s_axes[i]);
            float p2 = p0 - DotProduct(e1, s_axes[i]);
            float p1 = p0 - d1;
            float mn = QueryMin(QueryMin(p1, p2), p0);
            float mx = QueryMax(QueryMax(p1, p2), p0);
            if (-boxRadius > mx || boxRadius < mn)
                return 0;
        }
    }
    return 1;
}

// The box/box test as TreeTreeQueryNodes expands it: the BoxOverlap body (with the arrays
// declared first, which keeps their constructors expanded at the low budget left there).
inline int BoxOverlapInline(const Vec3* aCenter, const Vec3* aHalfExtents, Vec3 bCenter,
                            Vec3 bHalfExtents, const Matrix4* bToA)
{
    Vec3 a[3];
    Vec3 b[3];
    TransformPointInline(&bCenter, bCenter, bToA);
    a[0] = Vec3(aHalfExtents->x, 0.0f, 0.0f);
    a[1] = Vec3(0.0f, aHalfExtents->y, 0.0f);
    a[2] = Vec3(0.0f, 0.0f, aHalfExtents->z);
    b[0] = Vec3(bHalfExtents.x, 0.0f, 0.0f);
    b[1] = Vec3(0.0f, bHalfExtents.y, 0.0f);
    b[2] = Vec3(0.0f, 0.0f, bHalfExtents.z);
    RotateVectorInline(&b[0], b[0], bToA);
    RotateVectorInline(&b[1], b[1], bToA);
    RotateVectorInline(&b[2], b[2], bToA);
    Vec3 t = bCenter - *aCenter;
    for (int i = 0; i < 15; i++) {
        switch (i) {
        case 0: s_axes[0] = Vec3(1.0f, 0.0f, 0.0f); break;
        case 1: s_axes[1] = Vec3(0.0f, 1.0f, 0.0f); break;
        case 2: s_axes[2] = Vec3(0.0f, 0.0f, 1.0f); break;
        case 3: s_axes[3] = Vec3(bToA->_11, bToA->_12, bToA->_13); break;
        case 4: s_axes[4] = Vec3(bToA->_21, bToA->_22, bToA->_23); break;
        case 5: s_axes[5] = Vec3(bToA->_31, bToA->_32, bToA->_33); break;
        case 6: s_axes[6] = CrossProduct(s_axes[0], s_axes[3]); break;
        case 7: s_axes[7] = CrossProduct(s_axes[0], s_axes[4]); break;
        case 8: s_axes[8] = CrossProduct(s_axes[0], s_axes[5]); break;
        case 9: s_axes[9] = CrossProduct(s_axes[1], s_axes[3]); break;
        case 10: s_axes[10] = CrossProduct(s_axes[1], s_axes[4]); break;
        case 11: s_axes[11] = CrossProduct(s_axes[1], s_axes[5]); break;
        case 12: s_axes[12] = CrossProduct(s_axes[2], s_axes[3]); break;
        case 13: s_axes[13] = CrossProduct(s_axes[2], s_axes[4]); break;
        case 14: s_axes[14] = CrossProduct(s_axes[2], s_axes[5]); break;
        }
        float dist = QueryAbs(DotProduct(t, s_axes[i]));
        float ra = QueryAbs(DotProduct(a[0], s_axes[i])) + QueryAbs(DotProduct(a[1], s_axes[i])) + QueryAbs(DotProduct(a[2], s_axes[i]));
        if (dist > ra + QueryAbs(DotProduct(b[0], s_axes[i])) + QueryAbs(DotProduct(b[1], s_axes[i])) + QueryAbs(DotProduct(b[2], s_axes[i])))
            return 0;
    }
    return 1;
}

// 0x004280e0.  Walks two trees: leaf against leaf (LeafPairQuery), leaf against node
// (LeafNodeQuery, then the node's children), node against node (box overlap, swept by the
// motion matrix when useMotion is set, then the children of the larger node).
int TreeTreeQueryNodes(QueryTreeNode* a, QueryTreeNode* b, int useMotion)
{
    if (a->volume < 0.0f && b->volume < 0.0f)
        return LeafPairQuery((QuerySegmentLeaf*)a, (QueryTriangleLeaf*)b);
    if (a->volume >= 0.0f && b->volume >= 0.0f) {
        if (useMotion ? SweptBoxOverlap(a->center, a->halfExtents, &b->center, &b->halfExtents, &s_relativeAB, &s_motion)
                      : BoxOverlapInline(&a->center, &a->halfExtents, b->center, b->halfExtents, &s_relativeAB)) {
            int hit = 0;
            if (a->volume > b->volume) {
                if (TreeTreeQueryNodes(a->child[0], b, useMotion))
                    hit = 1;
                if (TreeTreeQueryNodes(a->child[1], b, useMotion))
                    hit = 1;
            } else {
                if (TreeTreeQueryNodes(a, b->child[0], useMotion))
                    hit = 1;
                if (TreeTreeQueryNodes(a, b->child[1], useMotion))
                    hit = 1;
            }
            return hit;
        }
    } else {
        if (LeafNodeQuery(a, b, useMotion)) {
            int hit = 0;
            if (a->volume < 0.0f) {
                if (TreeTreeQueryNodes(a, b->child[0], useMotion))
                    hit = 1;
                if (TreeTreeQueryNodes(a, b->child[1], useMotion))
                    hit = 1;
            } else {
                if (TreeTreeQueryNodes(a->child[0], b, useMotion))
                    hit = 1;
                if (TreeTreeQueryNodes(a->child[1], b, useMotion))
                    hit = 1;
            }
            return hit;
        }
    }
    return 0;
}

// Helpers of SegmentSegmentDistance, written against the call views (the natural operator
// form expands more of them than retail: VC6 here keeps DotProduct(n, n), the in-range
// operators and the second plane's denominator inline where retail calls them).
inline Vec3 SegCross(const Vec3& a, const Vec3& b)
{
    return Vec3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

inline Vec3 SegNormalize(const Vec3& v)
{
    float sq = Vec3Dot(v, v);
    if (sq == 1.0f)
        return v;
    float inv = FastInvSqrt(sq);
    return Vec3Scale(v, inv);
}

inline float SegLength(const Vec3& v)
{
    float sq = Vec3Dot(v, v);
    if (sq == 1.0f)
        return 1.0f;
    return FastSqrt(sq);
}

// Min, then max: retail loads 1.0 through the FPU in the upper arm and skips the zero test
// there (the `v < 1 ? max(v, 0) : 1` form stores the immediates; 76 -> 56 differing).
inline float SegClamp01(float v) { return QueryMax(QueryMin(v, 1.0f), 0.0f); }

float Vec3LengthRef(const Vec3& v);   // 0x00435ec0 (call view of the length helper)

// Distance from p to the segment a + s*d, s clamped to [0, 1].
inline float SegPointDistance(const Vec3& p, const Vec3& a, const Vec3& d)
{
    float s = Vec3Dot(Vec3Sub(p, a), d);
    s = SegClamp01(s);
    return Vec3LengthRef(Vec3Sub(Vec3Add(Vec3ScaleLeft(s, d), a), p));
}

// 0x0042a640.  Distance between the segments p0 + s*d0 and p1 + t*d1 (0 when they are
// parallel): each line is cut with the plane through the other segment that contains the
// common normal; when both cuts lie on the segments the distance between the two points,
// otherwise the larger (retail: QueryMax) of the two point-to-segment distances.
float SegmentSegmentDistance(const Vec3* p0, const Vec3* d0, const Vec3* p1, const Vec3* d1)
{
    Vec3 n = SegCross(*d0, *d1);
    if (Vec3Dot(n, n) == 0.0f)
        return 0.0f;
    Vec3 c = SegCross(*d0, n);
    Vec3 m0 = SegNormalize(c);
    float off = -DotProduct(m0, *p1);
    float t0 = -((off + (m0.x * p0->x + m0.y * p0->y) + m0.z * p0->z) / DotProduct(m0, *d0));
    c = SegCross(*d1, n);
    Vec3 m1 = SegNormalize(c);
    off = -DotProduct(m1, *p0);
    float t1 = -((off + (m1.x * p1->x + m1.y * p1->y) + m1.z * p1->z) / Vec3Dot(m1, *d1));
    if (t0 >= 0.0f && t0 <= 1.0f && t1 >= 0.0f && t1 <= 1.0f)
        return SegLength(Vec3Sub(Vec3Add(Vec3ScaleLeft(t0, *d0), *p0), Vec3Add(Vec3ScaleLeft(t1, *d1), *p1)));
    Vec3 c0 = Vec3Add(Vec3ScaleLeft(t0, *d0), *p0);
    Vec3 c1 = Vec3Add(Vec3ScaleLeft(t1, *d1), *p1);
    float l1 = SegPointDistance(c0, *p1, *d1);
    float l2 = SegPointDistance(c1, *p0, *d0);
    return QueryMax(l1, l2);
}

// 0x00427c10.  Segment (moved by m) against a triangle, accumulating into the sweep record.
extern Vec3 g_CollisionScratchPoints[128];   // 0x00579068
extern int g_CollisionScratchCount;          // 0x00579014
extern int g_unknown579668;                  // 0x00579668

// The sweep record SegmentTriangleQuery accumulates into (the same pointer as
// g_CollisionBoxResult; CollisionShapeTests.h's CollisionSweepQuery).
struct QuerySweepRecord {
    Vec3 offset;                    // +0x00
    unsigned char field_0x0c[0x0c];
    Vec3 contact;                   // +0x18
    float scale;                    // +0x24
    int count;                      // +0x28
};
// Retail reloads the record pointer at every use; a macro keeps that without spending inline
// budget.
#define g_SweepRecord ((QuerySweepRecord*)g_CollisionBoxResult)

int Vec3Equal(const Vec3* a, const Vec3* b);   // 0x005299e0

// The clear-flag arm's push dot accumulates z first (measured: this grouping moves the arm
// to retail's load order; the set-flag arm keeps DotProduct).  Provisional.
static inline float QueryDotZFirst(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

int SegmentTriangleQuery(const QueryTriangle* tri, const Vec3* segment, const Matrix4* m)
{
    Vec3 start;
    TransformPointInline(&start, segment[0], m);
    start -= g_SweepRecord->offset;
    Vec3 dir;
    TransformPointInline(&dir, segment[1], m);
    dir -= start;
    if (QueryDot(dir, tri->normal) <= 0.0f)
        return 0;
    float t = -((QueryDot(start, tri->normal) + tri->planeOffset) / QueryDot(dir, tri->normal));
    if (t < 0.0f || t > 1.0f)
        return 0;
    g_SweepRecord->scale *= t;
    Vec3 hit = dir * t + start;
    Vec3 push;
    if (!g_unknown579668)
        push = -QueryDotZFirst(dir * t, tri->normal) * tri->normal;
    else
        push = -DotProduct(dir * t, tri->normal) * tri->normal;
    if (!PointInTriangle(&hit, tri))
        return 0;
    g_SweepRecord->offset += push;
    g_SweepRecord->contact += hit;
    if (!g_unknown579668) {
        int i;
        for (i = 0; i < g_CollisionScratchCount; i++) {
            if (Vec3Equal(&g_CollisionScratchPoints[i], &tri->normal))
                break;
        }
        if (i >= g_CollisionScratchCount)
            g_CollisionScratchPoints[g_CollisionScratchCount++] = tri->normal;
        g_SweepRecord->count++;
        return 1;
    }
    g_SweepRecord->offset += push;
    g_SweepRecord->contact = hit;
    g_SweepRecord->count = 1;
    g_CollisionScratchPoints[0] = tri->normal;
    g_CollisionScratchCount = 1;
    return 1;
}
