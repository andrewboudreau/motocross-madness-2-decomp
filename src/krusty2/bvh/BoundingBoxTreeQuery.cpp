// BoundingBoxTreeQuery.cpp -- run-time queries against the bounding-box trees.
//
// Extent 0x00424690..0x0042ad2f (strong inference).  The file name is ours: the unit has
// no __FILE__ string and no RTTI.
// - Before it: bmpfile.cpp, whose last function ends at 0x00424689.  After it:
//   BoundingBoxTreeBuild.cpp (0x0042ad30, its partition helpers; own __FILE__ xrefs from
//   0x0042b75e).
// - .CRT$XCU lists, between bikerace.cpp's set and BoundingBoxTreeBuild.cpp's
//   (0x0042d250), this unit's kVec3 set (0x00429400..0x0042953b, Math3D.h's four
//   constants) followed by its five empty initializers (0x00424690..0x0042472f).  The
//   empty ones are the file statics with empty user constructors below (three Matrix4,
//   two Vec3 arrays); they are indistinguishable from each other in the image.
// - .bss 0x00578ea0..0x00579070 is read only by this code (the kVec3 set is interleaved
//   with the statics), except g_CollisionScratchPoints/Count and g_CollisionBoxResult,
//   which the collision code also uses.
// - Every function is called from here or from the collision code (CollisionObject.cpp,
//   0x004335cb..0x00439cb8); MatrixMultiply and the Vec3Transform* helpers also from the
//   scene graph and LightEmitter.
// - 0x00428060 and 0x00428090 (Vec3 operator+= / operator*(float, const Vec3&)) sit in the
//   range but are COMDAT copies of Math3D.h inlines shared with other units
//   (samples/physics/helpers/VectorOps.cpp).
//
// The functions sit in retail order.  Five are near misses, each with a status comment
// (docs/BOUNDINGBOXTREEQUERY.md has the forms tried): BoxCapsuleOverlap 0x00425900,
// BoxTriangleQuery 0x00426be0, SegmentTriangleQuery 0x00427c10, TreeTreeQueryNodes
// 0x004280e0 and SegmentSegmentDistance 0x0042a640.
#include "bvh/BoundingBoxTreeQuery.h"
#include "collision/CollisionPoint.h"

// Complete 0x00404e60 constructor behavior for this provisional call view.
Vec3Call::Vec3Call(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

void operator delete(void* p);   // 0x004a30c0

// 0x00578f4c: vertex array of the triangle tree being queried; the entry points store it
// before they call the recursive workers.
static Vec3* s_vertices;

// 0x00578ef0 / 0x00578eb0: the relative frames of a tree-against-tree query (B in A's frame
// and A in B's frame); 0x00579018: the optional motion matrix; 0x0057905c: the query mode.
static Matrix4 s_relativeAB;
static Matrix4 s_relativeBA;
static Matrix4 s_motion;
static int s_mode;

// 0x00578f50: the 15 separating-axis candidates of the box/box test (BoxOverlap 0x00424ab0).
static Vec3 s_axes[15];

// 0x00579068 / 0x00579014: contact points of the last tree/tree query and their count (also
// read by the collision code: collision/CollisionShapeTests.h).
Vec3 g_CollisionScratchPoints[128];
int g_CollisionScratchCount;

// 0x00579058: the hit record the leaf tests update (set by the collision code, 0x00439e00).
QueryHit* g_CollisionBoxResult;

// 0x00424730.  Overlap of box B (bCenter, bHalf, in A's frame through rel) with box A swept by
// the motion xfA: A's half extents rotated by xfA give the moved box's extents, the union of
// the box before and after the move (relative to aCenter) gives the swept centre offset and
// half extents, then BoxOverlap.
// Source shape (VC6 SP3, tier 2): the moved centre and the swept centre live in sibling
// blocks, which share one frame slot and rank it below the delta and extent vectors as in
// retail; the centre is read through a pointer for the delta and added back component by
// component (operator+= or a direct read loads the swept centre first).
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
    Vec3 delta;
    {
        Vec3 moved;
        const Vec3* origin = &aCenter;
        TransformPointPtr(&moved, origin, xfA);
        delta.x = moved.x - origin->x;
        delta.y = moved.y - origin->y;
        delta.z = moved.z - origin->z;
    }
    {
        Vec3 center;
        for (int i = 0; i < 3; i++) {
            float lo = delta[i] - ext[i];
            float hi = ext[i] + delta[i];
            lo = QueryMin(-half[i], lo);
            hi = QueryMax(half[i], hi);
            center[i] = (hi + lo) * 0.5f;
            half[i] = (hi - lo) * 0.5f;
        }
        aHalf = half;
        aCenter.x += center.x;
        aCenter.y += center.y;
        aCenter.z += center.z;
    }
    return BoxOverlap(&aCenter, &aHalf, *bCenter, *bHalf, rel);
}

// 0x00424ab0.  Box A (aCenter, aHalfExtents) against box B (bCenter, bHalfExtents, given in
// A's frame through bToA): the separating-axis test on A's three axes, B's three axes and
// their nine cross products.  The seven dot products call retail's dot (0x0040ae30, summed
// (y + x) + z) through Vec3DotCall: Math3D.h's DotProduct, left out of line by the
// inline budget, linked a COMDAT copy summing (z + y) + x (same bytes here, different
// result at 24-bit precision).  The inline cross products need Math3D.h's parenthesised
// CrossProduct (docs/VC6_OPERAND_ORDER.md section 3).
int BoxOverlap(const Vec3* aCenter, const Vec3* aHalfExtents, Vec3 bCenter, Vec3 bHalfExtents,
               const Matrix4* bToA)
{
    TransformPointInline(&bCenter, bCenter, bToA);
    Vec3 a[3];
    a[0] = Vec3(aHalfExtents->x, 0.0f, 0.0f);
    a[1] = Vec3(0.0f, aHalfExtents->y, 0.0f);
    a[2] = Vec3(0.0f, 0.0f, aHalfExtents->z);
    Vec3 b[3];
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
        float dist = QueryAbs(Vec3DotCall(&t, &s_axes[i]));
        float ra = QueryAbs(Vec3DotCall(&a[0], &s_axes[i])) + QueryAbs(Vec3DotCall(&a[1], &s_axes[i])) + QueryAbs(Vec3DotCall(&a[2], &s_axes[i]));
        if (dist > ra + QueryAbs(Vec3DotCall(&b[0], &s_axes[i])) + QueryAbs(Vec3DotCall(&b[1], &s_axes[i])) + QueryAbs(Vec3DotCall(&b[2], &s_axes[i])))
            return 0;
    }
    return 1;
}

// 0x004253b0.  Segment p0-p1 (moved by m) against the box (center, halfExtents): the
// separating-axis test on the three box axes and the three cross products with the segment.
int SegmentBoxOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 p0, Vec3 p1,
                      const Matrix4* m)
{
    TransformPointInline(&p0, p0, m);
    TransformPointInline(&p1, p1, m);
    p0 -= *center;
    p1 -= *center;
    Vec3 mid = ScaleCtorCall(AddCtorCall(p0, p1), 0.5f);
    Vec3 halfSpan = SubtractCtorCall(mid, p0);
    Vec3 ad;
    ad.x = QueryAbs(halfSpan.x);
    if (QueryAbs(mid.x) > ad.x + halfExtents->x)
        return 0;
    ad.y = QueryAbs(halfSpan.y);
    if (QueryAbs(mid.y) > ad.y + halfExtents->y)
        return 0;
    ad.z = QueryAbs(halfSpan.z);
    if (QueryAbs(mid.z) > ad.z + halfExtents->z)
        return 0;
    if (QueryAbs(mid.y * halfSpan.z - mid.z * halfSpan.y) > halfExtents->y * ad.z + halfExtents->z * ad.y)
        return 0;
    if (QueryAbs(mid.x * halfSpan.z - mid.z * halfSpan.x) > halfExtents->x * ad.z + halfExtents->z * ad.x)
        return 0;
    if (QueryAbs(mid.x * halfSpan.y - mid.y * halfSpan.x) > halfExtents->x * ad.y + halfExtents->y * ad.x)
        return 0;
    return 1;
}

// 0x00425750.  Overlap of a sphere (world centre, radius) with the box (centre, half extents)
// given in xf's frame.
int BoxSphereOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 sphereCenter,
                     float radius, float radiusSq, const Matrix4* xf)
{
    InverseTransformPointInline(&sphereCenter, sphereCenter, xf);
    sphereCenter -= *center;
    if (sphereCenter.x < 0.0f)
        sphereCenter.x = -sphereCenter.x;
    if (sphereCenter.x - halfExtents->x > radius)
        return 0;
    if (sphereCenter.y < 0.0f)
        sphereCenter.y = -sphereCenter.y;
    if (sphereCenter.y - halfExtents->y > radius)
        return 0;
    if (sphereCenter.z < 0.0f)
        sphereCenter.z = -sphereCenter.z;
    if (sphereCenter.z - halfExtents->z > radius)
        return 0;
    Vec3 d;
    d.x = sphereCenter.x - QueryMin(sphereCenter.x, halfExtents->x);
    d.y = sphereCenter.y - QueryMin(sphereCenter.y, halfExtents->y);
    d.z = sphereCenter.z - QueryMin(sphereCenter.z, halfExtents->z);
    if (radiusSq > QuerySquareMagnitude(d))
        return 1;
    return 0;
}

// v / |v| (returns v when |v|^2 == 1): the result of Math3D.h's Vec3Normalize
// (0x005087b0), written with a named result (BoxCapsuleOverlap is closer to retail with
// it; docs/BOUNDINGBOXTREEQUERY.md).  |v|^2 calls retail's dot through Vec3Dot.
inline Vec3 Vec3NormalizeInline(const Vec3& v)
{
    Vec3 r;
    float sq = Vec3Dot(v, v);
    if (sq == 1.0f) {
        r = v;
    } else {
        float inv = FastInvSqrt(sq);
        r = v * inv;
    }
    return r;
}

// The same with the inline DotProduct, for BoxCapsuleOverlap's last axis (x == 0, so the
// expanded sum equals retail's call; docs/BOUNDINGBOXTREEQUERY.md).
inline Vec3 Vec3NormalizeDotInline(const Vec3& v)
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

inline float QuerySigned(float s, float p) { if (p < 0.0f) return -s; return s; }
// Component-wise squares of the corner distances (BoxCapsuleOverlap).
inline Vec3 QuerySquares(float x, float y, float z)
{
    return Vec3(x * x, y * y, z * z);
}

// 0x00425900.  Box (center, halfExtents) against a capsule (segment ends[0]..ends[1] moved
// by the inverse of xf, radius): the box axes, the three axes crossed with the normalised
// segment direction, then for the xy, xz and yz planes the direction towards the corner
// nearer to one of the segment's ends (squared distances, signed by that end's position).
// Not exact (2450 of 4314 bytes, 4321 compiled: the seventh normalisation's DotProduct
// stays inline where retail calls it, two fxch differ and the frame is permuted;
// docs/BOUNDINGBOXTREEQUERY.md lists the forms tried).
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
    float extent = QueryAbs(halfSegment.x);
    if (QueryAbs(mid.x) > extent + halfExtents->x + radius)
        return 0;
    float extentY = QueryAbs(halfSegment.y);
    if (QueryAbs(mid.y) > extentY + halfExtents->y + radius)
        return 0;
    float extentZ = QueryAbs(halfSegment.z);
    if (QueryAbs(mid.z) > extentZ + radius + halfExtents->z)
        return 0;
    Vec3 boxAxes[3];
    boxAxes[0] = Vec3(halfExtents->x, 0.0f, 0.0f);
    boxAxes[1] = Vec3(0.0f, halfExtents->y, 0.0f);
    boxAxes[2] = Vec3(0.0f, 0.0f, halfExtents->z);
    Vec3 dir = Vec3NormalizeInline(p1 - p0);
    Vec3 axis;
    float dist, ra;
    axis.x = -dir.z;
    axis.y = 0.0f;
    axis.z = dir.x;
    axis = Vec3NormalizeInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    axis.x = dir.y;
    axis.y = -dir.x;
    axis.z = 0.0f;
    axis = Vec3NormalizeInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    axis.x = 0.0f;
    axis.y = dir.z;
    axis.z = -dir.y;
    axis = Vec3NormalizeInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    float c0x = QueryAbs(halfExtents->x) - QueryAbs(p0.x);
    float c0y = QueryAbs(halfExtents->y) - QueryAbs(p0.y);
    float c0z = QueryAbs(halfExtents->z) - QueryAbs(p0.z);
    float c1x = QueryAbs(halfExtents->x) - QueryAbs(p1.x);
    float c1y = QueryAbs(halfExtents->y) - QueryAbs(p1.y);
    float c1z = QueryAbs(halfExtents->z) - QueryAbs(p1.z);
    Vec3 s0 = QuerySquares(c0x, c0y, c0z);
    Vec3 s1 = QuerySquares(c1x, c1y, c1z);
    if (s0.x + s0.y < s1.x + s1.y) {
        axis.x = QuerySigned(s0.x, p0.x);
        axis.y = QuerySigned(s0.y, p0.y);
    } else {
        axis.x = QuerySigned(s1.x, p1.x);
        axis.y = QuerySigned(s1.y, p1.y);
    }
    axis.z = 0.0f;
    axis = Vec3NormalizeInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    if (s0.x + s0.z < s1.x + s1.z) {
        axis.x = QuerySigned(s0.x, p0.x);
        axis.z = QuerySigned(s0.z, p0.z);
    } else {
        axis.x = QuerySigned(s1.x, p1.x);
        axis.z = QuerySigned(s1.z, p1.z);
    }
    axis.y = 0.0f;
    axis = Vec3NormalizeInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    if (s0.y + s0.z < s1.y + s1.z) {
        axis.y = QuerySigned(s0.y, p0.y);
        axis.z = QuerySigned(s0.z, p0.z);
    } else {
        axis.y = QuerySigned(s1.y, p1.y);
        axis.z = QuerySigned(s1.z, p1.z);
    }
    axis.x = 0.0f;
    axis = Vec3NormalizeDotInline(axis);
    dist = QueryAbs(QueryDot(axis, mid));
    ra = QueryAbs(QueryDot(axis, boxAxes[0])) + QueryAbs(QueryDot(axis, boxAxes[1]))
            + QueryAbs(QueryDot(axis, boxAxes[2]));
    if (dist > ra + QueryAbs(axis.z * halfSegment.z + (axis.x * halfSegment.x + axis.y * halfSegment.y)) + radius)
        return 0;
    return 1;
}

// 0x004269e0.  A leaf against an interior node: a segment leaf of A against a box of B, or a
// box of A (moved by the motion matrix when useMotion is set) against a triangle leaf of B.
int LeafNodeQuery(QueryTreeNode* a, QueryTreeNode* b, int useMotion)
{
    if (a->volume < 0.0f) {
        QuerySegmentLeaf* seg = (QuerySegmentLeaf*)a;
        switch (s_mode) {
        case 0:
            return 0;
        case 1: {
            const Vec3* ends = seg->end;
            return SegmentBoxOverlap(&b->center, &b->halfExtents, ends[0], ends[1], &s_relativeBA);
        }
        case 2: {
            Vec3 segment[2];
            segment[0] = seg->end[0];
            TransformPointInline(&segment[1], seg->end[0], &s_motion);
            return SegmentBoxOverlap(&b->center, &b->halfExtents, seg->end[0], segment[1], &s_relativeBA);
        }
        }
        return 0;
    }
    if (useMotion) {
        Vec3 halfExtents = a->halfExtents;
        Vec3 center = a->center;
        MoveBox(&center, &halfExtents, &s_motion);
        return BoxTriangleQuery(&center, &halfExtents, &((QueryTriangleLeaf*)b)->tri, &s_relativeAB);
    }
    return BoxTriangleQuery(&a->center, &a->halfExtents, &((QueryTriangleLeaf*)b)->tri, &s_relativeAB);
}

// The min/max of the two edge projections in BoxTriangleQuery: the test reads the second
// operand first.  VC6 expands an inline's arguments left to right, so with (p0 - d2, p0 - d1)
// the first difference stays on the x87 stack (stored to its CSE slot with fst) while the
// second is stored and compared against it, unrounded at 53-bit precision, as retail does.
inline float QueryMinBFirst(float a, float b) { return b < a ? b : a; }
inline float QueryMaxBFirst(float a, float b) { return b > a ? b : a; }

// 0x00426be0.  Box (center, halfExtents) against a triangle moved by m: the 13-axis
// separating-axis test (face normal, the box axes, the box axes crossed with two edges and
// with the normalised third edge).  The third edge is v[2] - v[0] (the same direction as the
// second one, negated): retail's choice, kept.
// Not exact (1542 of 2576 bytes: every instruction and the 0x164 frame match, but the
// transformed vertices and the case temporaries take other stack slots;
// docs/BOUNDINGBOXTREEQUERY.md lists the forms tried).
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
        // The seven dots call retail's 0x0040ae30 through Vec3Dot (see BoxOverlap).
        float boxRadius = QueryAbs(Vec3Dot(boxAxes[0], s_axes[i]))
                          + QueryAbs(Vec3Dot(boxAxes[1], s_axes[i]))
                          + QueryAbs(Vec3Dot(boxAxes[2], s_axes[i]));
        if (i == 0) {
            if (QueryAbs(Vec3Dot(offset, s_axes[0])) > boxRadius)
                return 0;
            e0 = v[0] - v[1];
            e1 = v[0] - v[2];
        } else {
            float p0 = Vec3Dot(offset, s_axes[i]);
            float d1 = Vec3Dot(e0, s_axes[i]);
            float d2 = Vec3Dot(e1, s_axes[i]);
            float mn = QueryMin(QueryMinBFirst(p0 - d2, p0 - d1), p0);
            float mx = QueryMax(QueryMaxBFirst(p0 - d2, p0 - d1), p0);
            if (-boxRadius > mx || boxRadius < mn)
                return 0;
        }
    }
    return 1;
}

// 0x004275f0.  Leaf against leaf: a segment leaf of tree A (mode 1: its two points; mode 2:
// its first point and where the motion matrix moves it) against a triangle leaf of tree B.
int LeafPairQuery(QuerySegmentLeaf* a, QueryTriangleLeaf* b)
{
    QueryTriangle* tri = &b->tri;
    Vec3 segment[2];   // case 2's; retail gives it its own frame slots, as at function scope
    switch (s_mode) {
    case 1: {
        const Vec3* ends = a->end;
        Vec3 p = ends[0];
        Vec3 start;
        TransformPointInline(&start, p, &s_relativeBA);
        Vec3 dir;
        Vec3 delta;
        Vec3TransformNormal(&dir, *Vec3SubtractCall(&delta, &ends[1], &ends[0]), &s_relativeBA);
        if (Vec3DotCall(&dir, &tri->normal) >= 0.0f)
            return 0;
        float t = -((Vec3DotCall(&tri->normal, &start) + tri->planeOffset) / Vec3DotCall(&tri->normal, &dir));
        if (t < 0.0f || t > 1.0f)
            return 0;
        Vec3 scaled;
        Vec3AddAssignCall hit;
        (Vec3&)hit = *Vec3ScaleCall(&scaled, &dir, t);
        hit += start;
        if (!PointInTriangle(&hit, tri))
            return 0;
        if (t > g_CollisionBoxResult->fraction)
            return 0;
        g_CollisionBoxResult->fraction = t;
        g_CollisionBoxResult->point = &a->end[0];
        g_CollisionBoxResult->normal = tri->normal;
        return 1;
    }
    case 2: {
        segment[0] = a->end[0];
        Vec3 p = a->end[0];
        RotateVectorInline(&segment[1], p, &s_motion);
        segment[1].x = s_motion._41 + segment[1].x;
        segment[1].y = s_motion._42 + segment[1].y;
        segment[1].z = s_motion._43 + segment[1].z;
        return SegmentTriangleQuery(tri, segment, &s_relativeBA);
    }
    }
    return 0;
}

// 0x004278d0.  Nonzero when p (already in the triangle's plane) lies inside the triangle.
int PointInTriangle(const Vec3* p, const QueryTriangle* tri)
{
    Vec3 e0 = s_vertices[tri->vertex[2]] - s_vertices[tri->vertex[1]];
    Vec3 e1 = s_vertices[tri->vertex[0]] - s_vertices[tri->vertex[1]];
    Vec3 toPoint = *p - s_vertices[tri->vertex[1]];
    Vec3 cross = CrossProduct(e0, toPoint);
    if (QueryDot(cross, tri->normal) < 0.0f)
        return 0;
    cross = CrossProduct(toPoint, e1);
    if (QueryDot(cross, tri->normal) < 0.0f)
        return 0;
    e0 = SubtractCtorCall(s_vertices[tri->vertex[0]], s_vertices[tri->vertex[2]]);
    e1 = SubtractCtorCall(s_vertices[tri->vertex[1]], s_vertices[tri->vertex[2]]);
    toPoint = SubtractCtorCall(*p, s_vertices[tri->vertex[2]]);
    cross = CrossProductCall(e0, toPoint);
    if (Vec3DotCall(&tri->normal, &cross) < 0.0f)
        return 0;
    cross = CrossProductCall(toPoint, e1);
    if (Vec3DotCall(&tri->normal, &cross) < 0.0f)
        return 0;
    return 1;
}

// 0x00579668: read here and never written anywhere in retail (.bss, so always 0); outside
// the unit's .bss range, owner unknown.  Defined here for the link.
int g_unknown579668;

// The sweep record SegmentTriangleQuery accumulates into (the same pointer as
// g_CollisionBoxResult; CollisionShapeTests.h's CollisionSweepQuery).
struct QuerySweepRecord {
    Vec3 offset;                    // +0x00
    unsigned char contactNormal[0x0c]; // +0x0c CollisionSweepQuery contactNormal (bytes kept)
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

// The set-flag arm's push dot: retail computes the z product first, keeps it on the x87
// stack and adds it to x + y last (0x00427f0d..0x00427f23, popped at 0x00427f2b).  The
// flat DotProduct gave (z + y) + x, an ulp apart at 24-bit precision.
static inline float QueryDotZKept(const Vec3& a, const Vec3& b)
{
    float z = a.z * b.z;
    return (a.y * b.y + a.x * b.x) + z;
}

// The segment's second point is transformed with its y row summed (y + z) + x, as retail
// does (retail copies segment[1] at entry; the natural TransformPointInline gives (x + z) + y
// here, which rounds differently at 24-bit precision: tools/emu_cases/segment_triangle_query.py).
inline void TransformPointYZX(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = (v.y * m->_22 + v.z * m->_32) + v.x * m->_12 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// 0x00427c10.  Segment (moved by m) against a triangle, accumulating into the sweep record.
// Not exact (424 of 1101 bytes, 1112 compiled: retail keeps the constant 1 in ebp, so the
// register plan and the copy of segment[1] at entry differ;
// docs/BOUNDINGBOXTREEQUERY.md lists the forms tried).
int SegmentTriangleQuery(const QueryTriangle* tri, const Vec3* segment, const Matrix4* m)
{
    Vec3 start;
    TransformPointInline(&start, segment[0], m);
    start -= g_SweepRecord->offset;
    Vec3 dir;
    TransformPointYZX(&dir, segment[1], m);
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
        push = -QueryDotZKept(dir * t, tri->normal) * tri->normal;
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

// The box/box test as TreeTreeQueryNodes expands it: the BoxOverlap body (with the arrays
// declared first, which keeps their constructors expanded at the low budget left there).
// The dots call retail's 0x0040ae30 through Vec3Dot (see BoxOverlap).  A's radius is
// grouped |a0| + (|a1| + |a2|): retail adds |a2| + |a1| first (0x004287c1..0x004287cb),
// where the flat sum here came out (|a2| + |a0|) + |a1| (2 of 371 runs at 24-bit, seed 7).
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
        float dist = QueryAbs(Vec3Dot(t, s_axes[i]));
        float ra = QueryAbs(Vec3Dot(a[0], s_axes[i])) + (QueryAbs(Vec3Dot(a[1], s_axes[i])) + QueryAbs(Vec3Dot(a[2], s_axes[i])));
        if (dist > ra + QueryAbs(Vec3Dot(b[0], s_axes[i])) + QueryAbs(Vec3Dot(b[1], s_axes[i])) + QueryAbs(Vec3Dot(b[2], s_axes[i])))
            return 0;
    }
    return 1;
}

// 0x004280e0.  Walks two trees: leaf against leaf (LeafPairQuery), leaf against node
// (LeafNodeQuery, then the node's children), node against node (box overlap, swept by the
// motion matrix when useMotion is set, then the children of the larger node).
// Not exact (450 of 2160 bytes, 2228 compiled: VC6 still expands the first a[] constructor
// that retail calls, and the recursion tails are cross-jumped differently;
// docs/BOUNDINGBOXTREEQUERY.md lists the forms tried).
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

// 0x00428950.  Tree against tree: sets up both relative frames (and the motion matrix when
// given), then walks the two trees.
int TreeTreeQuery(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA, const Matrix4* xfB,
                  int mode, Vec3* vertices, const Matrix4* motion)
{
    s_vertices = vertices;
    s_mode = mode;
    s_relativeAB = *xfA;
    InvertRigid(&s_relativeAB);
    MatrixMultiply(&s_relativeAB, s_relativeAB, *xfB);
    s_relativeBA = *xfB;
    InvertRigid(&s_relativeBA);
    MatrixMultiply(&s_relativeBA, s_relativeBA, *xfA);
    if (motion) {
        s_motion = *motion;
        return TreeTreeQueryNodes(a, b, 1);
    }
    return TreeTreeQueryNodes(a, b, 0);
}

// 0x00428c20.  Overlap of the root boxes of two trees.
int TreeBoxOverlap(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA, const Matrix4* xfB)
{
    s_relativeAB = *xfA;
    InvertRigid(&s_relativeAB);
    MatrixMultiply(&s_relativeAB, s_relativeAB, *xfB);
    return BoxOverlap(&a->center, &a->halfExtents, b->center, b->halfExtents, &s_relativeAB);
}

// v * m with the translation row, as GrowBoxByTransformedBox expands it.
inline void TransformPointGB(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + (v.y * m->_21 + v.z * m->_31) + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + (v.y * m->_23 + v.z * m->_33) + m->_43;
}

// 0x004290d0.  Grows the box (outCenter, outHalfExtents) to contain the box (center,
// halfExtents) moved by m.  The x and z rows of the transform need the parenthesised
// grouping.  The loop writes a new local (`mid`), which VC6 homes in the dead `center`
// argument slot; the transformed centre's x is then only read by d.x and is not stored.
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
    Vec3 mid;
    for (int i = 0; i < 3; i++) {
        float lo = d[i] - half[i];
        float hi = half[i] + d[i];
        if (-grown[i] < lo)
            lo = -grown[i];
        hi = QueryMax(grown[i], hi);
        mid[i] = (hi + lo) * 0.5f;
        grown[i] = (hi - lo) * 0.5f;
    }
    *outCenter += mid;
    *outHalfExtents = grown;
}

// 0x00429540.
int SphereTreeQueryWithVertices(const Vec3* center, float radius, float radiusSq,
                                QueryTreeNode* node, const Matrix4* xf, int triangles,
                                Vec3* vertices)
{
    s_vertices = vertices;
    return SphereTreeQuery(center, radius, radiusSq, node, xf, triangles);
}

// 0x00429570.  Sphere (world centre, radius) against a tree: a triangle tree when
// triangles != 0 (the vertices are in s_vertices), else a point tree.  A triangle hits when
// the centre is within radius of its plane and either projects inside it or lies within
// radius of an edge; a point hits within radiusSq (squared).  The misses share one
// `return 0` tail with the inner node's box test, the first child's nonzero result is
// returned as is, and the second child's call is not in tail position (VC6 turns
// `return SphereTreeQuery(...)` into a loop).  The point branch's own block lets its
// locals share the triangle branch's slots.
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
            Vec3 u, q;
            if (dist <= radius) {
                // d is dead here and holds the scaled normal (retail's slot for both).
                q = *Vec3SubtractCall(&u, &p, Vec3ScaleCall(&d, &tri->normal, dist));
                if (PointInTriangle(&q, tri))
                    return 1;
                if (PointSegmentDistance(&p, &s_vertices[tri->vertex[0]], &s_vertices[tri->vertex[1]]) < radius)
                    return 1;
                if (PointSegmentDistance(&p, &s_vertices[tri->vertex[1]], &s_vertices[tri->vertex[2]]) < radius)
                    return 1;
                if (PointSegmentDistance(&p, &s_vertices[tri->vertex[2]], &s_vertices[tri->vertex[0]]) < radius)
                    return 1;
            }
            return 0;
        }
        {
            Vec3 p = ((QueryPointLeaf*)node)->point;
            Vec3 world;
            world.x = p.x * xf->_11 + p.y * xf->_21 + p.z * xf->_31 + xf->_41;
            world.y = p.x * xf->_12 + p.y * xf->_22 + p.z * xf->_32 + xf->_42;
            world.z = p.x * xf->_13 + p.y * xf->_23 + p.z * xf->_33 + xf->_43;
            Vec3 offset = world - *center;
            if (QueryDot(offset, offset) < radiusSq)
                return 1;
        }
    } else if (BoxSphereOverlap(&node->center, &node->halfExtents, *center, radius, radiusSq, xf)) {
        int hit = SphereTreeQuery(center, radius, radiusSq, node->child[0], xf, triangles);
        if (!hit)
            hit = SphereTreeQuery(center, radius, radiusSq, node->child[1], xf, triangles);
        return hit;
    }
    return 0;
}


// 0x00429890.
int CapsuleTreeQueryWithVertices(const Vec3* ends, float radius, float radiusSq,
                                 QueryTreeNode* node, const Matrix4* xf, int triangles,
                                 Vec3* vertices)
{
    s_vertices = vertices;
    return CapsuleTreeQuery(ends, radius, radiusSq, node, xf, triangles);
}

// 0x004298c0.  Capsule (segment ends[0]..ends[1], radius) against the tree; the triangle
// branch clips the segment against the face plane, the point branch measures the distance
// from the transformed point to the segment.  The misses share one `return 0` tail with the
// inner node's box test and the second child's call stays out of tail position, as in
// SphereTreeQuery.  q and e are declared with the branch: the operator temporaries then take
// the two slots retail gives them instead of q's and e's.  The point is copied and transformed
// in place, and its distance goes to the length helper by reference.
int CapsuleTreeQuery(const Vec3* ends, float radius, float radiusSq, QueryTreeNode* node,
                     const Matrix4* xf, int triangles)
{
    if (node->volume < 0.0f) {
        if (triangles) {
            QueryTriangle* tri = &((QueryTriangleLeaf*)node)->tri;
            Vec3 q, e;
            Vec3 p;
            InverseTransformPointInline(&p, *ends, xf);
            Vec3 d;
            {
                Vec3 delta;
                Vec3TransformNormalTranspose(&d, *Vec3SubtractCall(&delta, &ends[1], &ends[0]), xf);
            }
            float t = -(Vec3DotCall(&tri->normal, &p) + tri->planeOffset) / Vec3DotCall(&tri->normal, &d);
            if (!(t < 1.0f))
                t = 1.0f;
            else if (t <= 0.0f)
                t = 0.0f;
            Vec3 c;
            {
                Vec3 scaled, sum;
                c = *Vec3AddCall(&sum, Vec3ScaleCall(&scaled, &d, t), &p);
            }
            // Retain result storage for the original full call expression.
            float dist;
            {
                Vec3 delta;
                dist = Vec3DotCall(Vec3SubtractCall(&delta, &c, &s_vertices[tri->vertex[0]]), &tri->normal);
            }
            if (QueryAbs(dist) <= radius) {
                {
                    Vec3 delta;
                    q = *Vec3SubtractCall(&delta, &c, &static_cast<const Vec3&>(Vec3ScaleLeft(dist, tri->normal)));
                }
                if (PointInTriangle(&q, tri))
                    return 1;
                {
                    Vec3 delta;
                    e = *Vec3SubtractCall(&delta, &s_vertices[tri->vertex[1]], &s_vertices[tri->vertex[0]]);
                }
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[0]], &e) > radius)
                    return 1;
                {
                    Vec3 delta;
                    e = *Vec3SubtractCall(&delta, &s_vertices[tri->vertex[2]], &s_vertices[tri->vertex[1]]);
                }
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[1]], &e) > radius)
                    return 1;
                {
                    Vec3 delta;
                    e = *Vec3SubtractCall(&delta, &s_vertices[tri->vertex[0]], &s_vertices[tri->vertex[2]]);
                }
                if (SegmentSegmentDistance(&p, &d, &s_vertices[tri->vertex[2]], &e) > radius)
                    return 1;
            }
            return 0;
        } else {
            Vec3 p = ((QueryPointLeaf*)node)->point;
            TransformPointInline(&p, p, xf);
            Vec3 dir = Vec3Call(ends[1].x - ends[0].x, ends[1].y - ends[0].y, ends[1].z - ends[0].z);
            float t;
            {
                Vec3 delta;
                t = Vec3DotCall(&dir, Vec3SubtractCall(&delta, &p, ends)) / Vec3DotCall(&dir, &dir);
            }
            if (!(t < 1.0f))
                t = 1.0f;
            else if (t <= 0.0f)
                t = 0.0f;
            {
                Vec3 sum, delta;
                if (CollisionLength(reinterpret_cast<const CollisionVec3*>(Vec3SubtractCall(&delta, &p,
                        Vec3AddCall(&sum, &static_cast<const Vec3&>(Vec3ScaleLeft(t, dir)), ends)))) < radius)
                    return 1;
            }
        }
    } else if (BoxCapsuleOverlap(&node->center, &node->halfExtents, ends, radius, radiusSq, xf)) {
        int hit = CapsuleTreeQuery(ends, radius, radiusSq, node->child[0], xf, triangles);
        if (!hit)
            hit = CapsuleTreeQuery(ends, radius, radiusSq, node->child[1], xf, triangles);
        return hit;
    }
    return 0;
}

// 0x00429e60.
int SegmentTreeQueryWithVertices(const Vec3* ends, QueryTreeNode* node, const Matrix4* xf,
                                 Vec3* vertices)
{
    s_vertices = vertices;
    return SegmentTreeQuery(ends, node, xf);
}

// 0x00429e90.  Segment (ends[0] -> ends[1], moved by xf) against a tree: box nodes are
// culled with SegmentBoxOverlap; a triangle leaf is hit where the segment crosses its plane
// inside the triangle, and the nearest hit so far is kept in g_CollisionBoxResult.
int SegmentTreeQuery(const Vec3* ends, QueryTreeNode* node, const Matrix4* xf)
{
    if (node->volume < 0.0f) {
        const QueryTriangle* tri = &((QueryTriangleLeaf*)node)->tri;
        Vec3 p;
        TransformPointInline(&p, ends[0], xf);
        Vec3 d;
        RotateVectorInline(&d, ends[1] - ends[0], xf);
        if (QueryDot(d, tri->normal) < 0.0f) {
            float t = -((QueryDot(p, tri->normal) + tri->planeOffset) / QueryDot(d, tri->normal));
            if (t >= 0.0f && t <= 1.0f) {
                Vec3 hit = ScaleCtorCall(d, t);
                hit += p;
                if (PointInTriangle(&hit, tri) && t <= g_CollisionBoxResult->fraction) {
                    g_CollisionBoxResult->fraction = t;
                    g_CollisionBoxResult->point = ends;
                    g_CollisionBoxResult->normal = tri->normal;
                    return 1;
                }
            }
        }
    } else if (SegmentBoxOverlap(&node->center, &node->halfExtents, ends[0], ends[1], xf)) {
        int hit = SegmentTreeQuery(ends, node->child[0], xf);
        if (SegmentTreeQuery(ends, node->child[1], xf))
            hit = 1;
        return hit;
    }
    return 0;
}

// 0x0042a160.  Frees a box tree: interior nodes (volume >= 0) free both subtrees first.
void FreeBoxTree(QueryTreeNode* node)
{
    if (node->volume >= 0.0f) {
        FreeBoxTree(node->child[0]);
        FreeBoxTree(node->child[1]);
    }
    operator delete(node);
}

// 0x0042a1a0. Full 4x4 product; both matrices are passed by value (retail pushes 2 x 64
// bytes, callers do "add esp,0x84"). Decoded: out[i][j] = sum_k a[k][j] * b[i][k], i.e. in the
// row-vector convention out = b * a (b is applied first). Index pattern is tier 1; the
// parameter names are tier 3.
void MatrixMultiply(Matrix4* out, Matrix4 a, Matrix4 b)
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

// 0x0042a450. out = v * R(m): row-vector times the upper 3x3, no translation. cdecl, the Vec3
// is passed by value (3 floats on the stack, the caller copies it), 5 stack dwords.
void Vec3TransformNormal(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// 0x0042a4b0. out = R(m) * v: column-vector product, i.e. v * transpose(R).
void Vec3TransformNormalTranspose(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_12 + v.z * m->_13;
    out->y = v.x * m->_21 + v.y * m->_22 + v.z * m->_23;
    out->z = v.x * m->_31 + v.y * m->_32 + v.z * m->_33;
}

// 0x0042a510. out = v * m with the translation row (_41.._43) added.
void Vec3TransformPoint(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// 0x0042a580.  out = *v * m with the translation row added (the point taken by pointer;
// compare Vec3TransformPoint 0x0042a510, which takes it by value).
void TransformPointPtr(Vec3* out, const Vec3* v, const Matrix4* m)
{
    out->x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31 + m->_41;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42;
    out->z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33 + m->_43;
}

// 0x0042a5e0.  The inverse of a rigid transform applied to p: subtract the translation row,
// then multiply by the transposed rotation (dot with each row).
void InverseTransformPoint(Vec3* out, Vec3 p, const Matrix4* m)
{
    float dx = p.x - m->_41;
    float dy = p.y - m->_42;
    float dz = p.z - m->_43;

    out->x = dx * m->_11 + dy * m->_12 + dz * m->_13;
    out->y = dx * m->_21 + dy * m->_22 + dz * m->_23;
    out->z = dx * m->_31 + dy * m->_32 + dz * m->_33;
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
// Not exact (73 of 1471 bytes, 1458 compiled: retail homes the plane parameters, clamps
// and the first length in the dead d0/d1 argument slots (frame 0x60 against 0x80), and the
// inline cross products load a's components first; docs/BOUNDINGBOXTREEQUERY.md lists
// the forms tried).
float SegmentSegmentDistance(const Vec3* p0, const Vec3* d0, const Vec3* p1, const Vec3* d1)
{
    Vec3 n = SegCross(*d0, *d1);
    if (Vec3Dot(n, n) == 0.0f)
        return 0.0f;
    Vec3 c = SegCross(*d0, n);
    Vec3 m0 = SegNormalize(c);
    float off = -DotProduct(m0, *p1);
    // The denominator is spelled out: with DotProduct VC6 sums it (y + z) + x once t1 below
    // takes retail's shape; retail sums (x + y) + z (0x0042a7db..0x0042a7f2).
    float t0 = -((off + (m0.x * p0->x + m0.y * p0->y) + m0.z * p0->z)
                 / ((m0.x * d0->x + m0.y * d0->y) + m0.z * d0->z));
    c = SegCross(*d1, n);
    Vec3 m1 = SegNormalize(c);
    // Retail adds the plane offset to x + y and the z product last ((off + xy) + z,
    // 0x0042a8ed..0x0042a914); without the z temporary VC6 moves off to the end of the sum
    // ((x + y + z) + off), which rounds differently at 24-bit precision.
    float az = m1.z * p1->z;
    off = -DotProduct(m1, *p0);
    float t1 = -(((off + (m1.x * p1->x + m1.y * p1->y)) + az) / Vec3Dot(m1, *d1));
    if (t0 >= 0.0f && t0 <= 1.0f && t1 >= 0.0f && t1 <= 1.0f)
        return SegLength(Vec3Sub(Vec3Add(Vec3ScaleLeft(t0, *d0), *p0), Vec3Add(Vec3ScaleLeft(t1, *d1), *p1)));
    Vec3 c0 = Vec3Add(Vec3ScaleLeft(t0, *d0), *p0);
    Vec3 c1 = Vec3Add(Vec3ScaleLeft(t1, *d1), *p1);
    float l1 = SegPointDistance(c0, *p1, *d1);
    float l2 = SegPointDistance(c1, *p0, *d0);
    return QueryMax(l1, l2);
}

// 0x0042ac00.  Distance from p to the segment ab.
float PointSegmentDistance(const Vec3* p, const Vec3* a, const Vec3* b)
{
    Vec3 ab = *b - *a;
    Vec3 ap = *p - *a;
    float t = QueryDot(ap, ab) / QuerySquareMagnitude(ab);
    if (t >= 1.0f)
        t = 1.0f;
    else if (t <= 0.0f)
        t = 0.0f;
    Vec3 closest = *a + ab * t;
    Vec3 offset = *p - closest;
    float distSq = QuerySquareMagnitude(offset);
    if (distSq == 1.0f)
        return 1.0f;
    return FastSqrt(distSq);
}
