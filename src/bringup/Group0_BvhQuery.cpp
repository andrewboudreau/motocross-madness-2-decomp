// Group0_BvhQuery.cpp -- bring-up bodies for the box-tree query functions that
// src/krusty2/bvh/BoundingBoxTreeQuery.cpp declares but does not define yet:
//   BoxCapsuleOverlap      0x00425900 (Vec3 and CollisionVec3 views of one function)
//   BoxTriangleQuery       0x00426be0
//   SegmentTriangleQuery   0x00427c10
//   TreeTreeQueryNodes     0x004280e0
//   SegmentSegmentDistance 0x0042a640
// The arithmetic follows samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp
// (decoded structure, retail helper calls and constants), rewritten with plain
// inline vector math. Byte matching was not attempted.
//
// best guess / bring-up limitation: in retail these functions sit in the same
// unit as the query state (0x00578ea0..0x00579070): the vertex array, the two
// relative frames and the motion matrix. In this tree that state is file
// static in BoundingBoxTreeQuery.cpp, which this file cannot name. So:
// - TreeTreeQueryNodes gets the node/node box test from BoundingBoxTreeQuery.cpp
//   itself: LeafNodeQuery (0x004269e0) moves a box node by the motion matrix
//   when useMotion is set (MoveBox, the same sweep SweptBoxOverlap does) and
//   hands the box and &s_relativeAB to BoxTriangleQuery. A probe call through
//   LeafNodeQuery captures them, then BoxOverlap (0x00424ab0) runs on them,
//   which is what retail's BoxOverlap/SweptBoxOverlap test computes.
// - BoxTriangleQuery cannot read the vertex array, so it tests only the face
//   axis (exact: the plane offset gives the vertex projection) and reports
//   overlap for the other 12 axes. It only prunes the recursion; the exact
//   leaf tests (LeafPairQuery, PointInTriangle) still decide every hit, so
//   results are unchanged and only pruning is weaker.
// Moving these bodies into BoundingBoxTreeQuery.cpp (beside its statics)
// removes both limitations.
#include <string.h>

#include "bvh/BoundingBoxTreeQuery.h"

struct CollisionVec3;

// ---------------------------------------------------------------------------
// Local vector helpers (the header's out-of-line call views are not all
// defined in the link, so plain inline math is used here).
// ---------------------------------------------------------------------------

static inline float Group0Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 Group0Cross(const Vec3& a, const Vec3& b)
{
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// v / |v|; v unchanged when |v|^2 == 1 (Vec3Normalize 0x005087b0, no zero check).
static inline Vec3 Group0Normalize(const Vec3& v)
{
    float sq = Group0Dot(v, v);
    if (sq == 1.0f)
        return v;
    return v * FastInvSqrt(sq);
}

// |v|; 1 when |v|^2 == 1 (0x00435ec0).
static inline float Group0Length(const Vec3& v)
{
    float sq = Group0Dot(v, v);
    if (sq == 1.0f)
        return 1.0f;
    return FastSqrt(sq);
}

static inline float Group0Signed(float s, float p) { return p < 0.0f ? -s : s; }

// One separating-axis test of BoxCapsuleOverlap: nonzero when `axis`
// separates the box (boxAxes) from the capsule (mid +- halfSegment, radius).
static inline int Group0CapsuleAxisSeparates(const Vec3& axis, const Vec3* boxAxes,
                                             const Vec3& mid, const Vec3& halfSegment,
                                             float radius)
{
    float dist = QueryAbs(Group0Dot(axis, mid));
    float reach = QueryAbs(Group0Dot(axis, boxAxes[0])) + QueryAbs(Group0Dot(axis, boxAxes[1]))
                  + QueryAbs(Group0Dot(axis, boxAxes[2]))
                  + QueryAbs(Group0Dot(axis, halfSegment)) + radius;
    return dist > reach;
}

// ---------------------------------------------------------------------------
// 0x00425900. Box (center, halfExtents) against a capsule (segment ends[0]..
// ends[1] moved by the inverse of xf, radius): the three box axes, the
// segment direction crossed with them, then for the xy, xz and yz planes the
// direction towards the nearer box corner. radiusSq is not used.
// ---------------------------------------------------------------------------
int BoxCapsuleOverlap(const Vec3* center, const Vec3* halfExtents, const Vec3* ends,
                      float radius, float radiusSq, const Matrix4* xf)
{
    Vec3 p0, p1;
    InverseTransformPointInline(&p0, ends[0], xf);
    InverseTransformPointInline(&p1, ends[1], xf);
    p0 = p0 - *center;
    p1 = p1 - *center;
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
    Vec3 dir = Group0Normalize(p1 - p0);

    // The segment direction crossed with the y, z and x box axes.
    if (Group0CapsuleAxisSeparates(Group0Normalize(Vec3(-dir.z, 0.0f, dir.x)), boxAxes, mid, halfSegment, radius))
        return 0;
    if (Group0CapsuleAxisSeparates(Group0Normalize(Vec3(dir.y, -dir.x, 0.0f)), boxAxes, mid, halfSegment, radius))
        return 0;
    if (Group0CapsuleAxisSeparates(Group0Normalize(Vec3(0.0f, dir.z, -dir.y)), boxAxes, mid, halfSegment, radius))
        return 0;

    // Squared distances of each end to the box faces, used to pick the end
    // nearer to a corner in each coordinate plane.
    float c0x = QueryAbs(halfExtents->x) - QueryAbs(p0.x);
    float c0y = QueryAbs(halfExtents->y) - QueryAbs(p0.y);
    float c0z = QueryAbs(halfExtents->z) - QueryAbs(p0.z);
    float c1x = QueryAbs(halfExtents->x) - QueryAbs(p1.x);
    float c1y = QueryAbs(halfExtents->y) - QueryAbs(p1.y);
    float c1z = QueryAbs(halfExtents->z) - QueryAbs(p1.z);
    Vec3 s0(c0x * c0x, c0y * c0y, c0z * c0z);
    Vec3 s1(c1x * c1x, c1y * c1y, c1z * c1z);

    Vec3 cornerDir;
    if (s0.x + s0.y < s1.x + s1.y) {
        cornerDir.x = Group0Signed(s0.x, p0.x);
        cornerDir.y = Group0Signed(s0.y, p0.y);
    } else {
        cornerDir.x = Group0Signed(s1.x, p1.x);
        cornerDir.y = Group0Signed(s1.y, p1.y);
    }
    cornerDir.z = 0.0f;
    if (Group0CapsuleAxisSeparates(Group0Normalize(cornerDir), boxAxes, mid, halfSegment, radius))
        return 0;

    if (s0.x + s0.z < s1.x + s1.z) {
        cornerDir.x = Group0Signed(s0.x, p0.x);
        cornerDir.z = Group0Signed(s0.z, p0.z);
    } else {
        cornerDir.x = Group0Signed(s1.x, p1.x);
        cornerDir.z = Group0Signed(s1.z, p1.z);
    }
    cornerDir.y = 0.0f;
    if (Group0CapsuleAxisSeparates(Group0Normalize(cornerDir), boxAxes, mid, halfSegment, radius))
        return 0;

    if (s0.y + s0.z < s1.y + s1.z) {
        cornerDir.y = Group0Signed(s0.y, p0.y);
        cornerDir.z = Group0Signed(s0.z, p0.z);
    } else {
        cornerDir.y = Group0Signed(s1.y, p1.y);
        cornerDir.z = Group0Signed(s1.z, p1.z);
    }
    cornerDir.x = 0.0f;
    if (Group0CapsuleAxisSeparates(Group0Normalize(cornerDir), boxAxes, mid, halfSegment, radius))
        return 0;
    return 1;
}

// The collision code's view of the same function (CollisionShapeTests.h);
// CollisionVec3 has Vec3's layout (three floats).
int BoxCapsuleOverlap(const CollisionVec3* boundsCenter, const CollisionVec3* boundsHalf,
                      const CollisionVec3* worldEnds, float radius, float radiusSq,
                      const Matrix4* xf)
{
    return BoxCapsuleOverlap((const Vec3*)boundsCenter, (const Vec3*)boundsHalf,
                             (const Vec3*)worldEnds, radius, radiusSq, xf);
}

// ---------------------------------------------------------------------------
// TreeTreeQueryNodes' probe of LeafNodeQuery (see the file header).
// ---------------------------------------------------------------------------

static int s_probeActive;
static Vec3 s_probeCenter;
static Vec3 s_probeHalfExtents;
static const Matrix4* s_probeFrame;

// ---------------------------------------------------------------------------
// 0x00426be0. Box (center, halfExtents) against a triangle moved by m.
// Retail runs the 13-axis separating-axis test (face normal, box axes, box
// axes crossed with the three edges).
// best guess: only the face axis is tested here (the vertex array is not
// reachable from this file, see the header); every other axis is treated as
// overlapping, which keeps the tree walk correct but prunes less.
// ---------------------------------------------------------------------------
int BoxTriangleQuery(const Vec3* center, const Vec3* halfExtents, const QueryTriangle* tri,
                     const Matrix4* m)
{
    if (s_probeActive) {
        s_probeCenter = *center;
        s_probeHalfExtents = *halfExtents;
        s_probeFrame = m;
        return 0;
    }

    Vec3 faceNormal;
    RotateVectorInline(&faceNormal, tri->normal, m);
    float boxRadius = QueryAbs(faceNormal.x * halfExtents->x)
                      + QueryAbs(faceNormal.y * halfExtents->y)
                      + QueryAbs(faceNormal.z * halfExtents->z);
    // dot(v0 * m, n * m) = dot(v0, n) + dot(translation, n * m) for a rigid m,
    // and dot(v0, n) = -planeOffset (LeafPairQuery's plane: n.p + offset = 0).
    Vec3 translation(m->_41, m->_42, m->_43);
    float vertexProjection = -tri->planeOffset + Group0Dot(translation, faceNormal);
    if (QueryAbs(vertexProjection - Group0Dot(*center, faceNormal)) > boxRadius)
        return 0;
    return 1;
}

// The node/node box test of TreeTreeQueryNodes: retail calls
// SweptBoxOverlap(a, b, &s_relativeAB, &s_motion) when useMotion is set and
// the inline BoxOverlap(a, b, &s_relativeAB) otherwise.
static int Group0NodeBoxesOverlap(QueryTreeNode* a, QueryTreeNode* b, int useMotion)
{
    QueryTriangleLeaf probeLeaf;
    memset(&probeLeaf, 0, sizeof(probeLeaf));
    probeLeaf.marker = -1.0f;
    s_probeActive = 1;
    LeafNodeQuery(a, (QueryTreeNode*)&probeLeaf, useMotion);
    s_probeActive = 0;
    return BoxOverlap(&s_probeCenter, &s_probeHalfExtents, b->center, b->halfExtents,
                      s_probeFrame);
}

// ---------------------------------------------------------------------------
// 0x004280e0. Walks two trees: leaf against leaf (LeafPairQuery), leaf against
// node (LeafNodeQuery, then the node's children), node against node (box
// overlap, swept by the motion matrix when useMotion is set, then the
// children of the larger node).
// ---------------------------------------------------------------------------
int TreeTreeQueryNodes(QueryTreeNode* a, QueryTreeNode* b, int useMotion)
{
    if (a->volume < 0.0f && b->volume < 0.0f)
        return LeafPairQuery((QuerySegmentLeaf*)a, (QueryTriangleLeaf*)b);

    int hit = 0;
    if (a->volume >= 0.0f && b->volume >= 0.0f) {
        if (!Group0NodeBoxesOverlap(a, b, useMotion))
            return 0;
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

    if (!LeafNodeQuery(a, b, useMotion))
        return 0;
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

// ---------------------------------------------------------------------------
// 0x00427c10. Segment (moved by m) against a triangle, accumulating into the
// sweep record g_CollisionBoxResult points at.
// ---------------------------------------------------------------------------

extern Vec3 g_CollisionScratchPoints[128];   // 0x00579068

// 0x00579668: read only here and never written anywhere in retail (.bss, so
// always 0). best guess: kept as a zero flag.
static int s_replaceContactFlag;

// The sweep record SegmentTriangleQuery accumulates into (the same pointer as
// g_CollisionBoxResult; CollisionShapeTests.h's CollisionSweepQuery).
struct Group0SweepRecord {
    Vec3 offset;                    // +0x00
    unsigned char field_0x0c[0x0c];
    Vec3 contact;                   // +0x18
    float scale;                    // +0x24
    int count;                      // +0x28
};

int SegmentTriangleQuery(const QueryTriangle* tri, const Vec3* segment, const Matrix4* m)
{
    Group0SweepRecord* record = (Group0SweepRecord*)g_CollisionBoxResult;
    Vec3 start;
    TransformPointInline(&start, segment[0], m);
    start = start - record->offset;
    Vec3 dir;
    TransformPointInline(&dir, segment[1], m);
    dir = dir - start;
    if (Group0Dot(dir, tri->normal) <= 0.0f)
        return 0;
    float t = -((Group0Dot(start, tri->normal) + tri->planeOffset) / Group0Dot(dir, tri->normal));
    if (t < 0.0f || t > 1.0f)
        return 0;
    record->scale *= t;
    Vec3 hit = dir * t + start;
    Vec3 push = -Group0Dot(dir * t, tri->normal) * tri->normal;
    if (!PointInTriangle(&hit, tri))
        return 0;
    record->offset = record->offset + push;
    record->contact = record->contact + hit;
    if (!s_replaceContactFlag) {
        // Adds the face normal to the contact normal list unless already there
        // (exact component equality, 0x005299e0).
        int i;
        for (i = 0; i < g_CollisionScratchCount; i++) {
            const Vec3& n = g_CollisionScratchPoints[i];
            if (n.x == tri->normal.x && n.y == tri->normal.y && n.z == tri->normal.z)
                break;
        }
        if (i >= g_CollisionScratchCount)
            g_CollisionScratchPoints[g_CollisionScratchCount++] = tri->normal;
        record->count++;
        return 1;
    }
    record->offset = record->offset + push;
    record->contact = hit;
    record->count = 1;
    g_CollisionScratchPoints[0] = tri->normal;
    g_CollisionScratchCount = 1;
    return 1;
}

// ---------------------------------------------------------------------------
// 0x0042a640. Distance between the segments p0 + s*d0 and p1 + t*d1 (0 when
// they are parallel): each line is cut with the plane through the other
// segment that contains the common normal; when both cuts lie on the
// segments, the distance between the two points, otherwise the larger of the
// two point-to-segment distances.
// ---------------------------------------------------------------------------

// Distance from p to the segment a + s*d, s = dot(p - a, d) clamped to [0, 1].
static inline float Group0PointSegmentDistance(const Vec3& p, const Vec3& a, const Vec3& d)
{
    float s = QueryMax(QueryMin(Group0Dot(p - a, d), 1.0f), 0.0f);
    return Group0Length(s * d + a - p);
}

float SegmentSegmentDistance(const Vec3* p0, const Vec3* d0, const Vec3* p1, const Vec3* d1)
{
    Vec3 n = Group0Cross(*d0, *d1);
    if (Group0Dot(n, n) == 0.0f)
        return 0.0f;
    Vec3 m0 = Group0Normalize(Group0Cross(*d0, n));
    float off = -Group0Dot(m0, *p1);
    float t0 = -((off + Group0Dot(m0, *p0)) / Group0Dot(m0, *d0));
    Vec3 m1 = Group0Normalize(Group0Cross(*d1, n));
    off = -Group0Dot(m1, *p0);
    float t1 = -((off + Group0Dot(m1, *p1)) / Group0Dot(m1, *d1));
    Vec3 c0 = t0 * *d0 + *p0;
    Vec3 c1 = t1 * *d1 + *p1;
    if (t0 >= 0.0f && t0 <= 1.0f && t1 >= 0.0f && t1 <= 1.0f)
        return Group0Length(c0 - c1);
    float l1 = Group0PointSegmentDistance(c0, *p1, *d1);
    float l2 = Group0PointSegmentDistance(c1, *p0, *d0);
    return QueryMax(l1, l2);
}
