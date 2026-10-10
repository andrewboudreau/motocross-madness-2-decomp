// BoundingBoxTreeQueryNearMisses.cpp -- near misses of the run-time box-tree query unit
// (src/krusty2/bvh/BoundingBoxTreeQuery.cpp, 0x00424690..0x0042ad2f).  Names are tier 3.
//
// Status (VC6 SP3, vc6_o2_ml; tools/run_physics_samples.py scores):
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
// SweptBoxOverlap 0x00424730, BoxOverlap 0x00424ab0 and PointInTriangle 0x004278d0 are exact
// in the src file as well (the last two with Math3D.h's parenthesised CrossProduct and
// QueryDot's parenthesised y term; docs/VC6_OPERAND_ORDER.md section 3).
//
// The SAT functions below are written with the natural Vec3 operators and inline helpers.
// VC6's inline budget (docs/VC6_INLINE_BUDGET.md) then reproduces retail's mix of expanded
// and out-of-line helper calls exactly; the out-of-line instances are bound to retail's
// COMDAT copies (TransformPointInline 0x0042a510, RotateVectorInline 0x0042a450,
// Vec3NormalizeInline 0x005087b0, CrossProduct 0x00515600, DotProduct 0x0040ae30, the
// Vec3 constructor and operators).
// - BoxTriangleQuery 0x00426be0 (2576 bytes, 2572 here, 1673 strict): the 13-axis
//   box/triangle SAT.  The inline/out-of-line pattern, the jump table and (with the
//   parenthesised CrossProduct) the cross products of cases 4-12 match.  Left: one operand
//   pair of the second vertex transform's y row, the frame (0x164 vs 0x160; retail's locals
//   sit 0x18 higher) and the min/max of the projections: retail keeps p2 on the x87 stack
//   (`fst` to a spare slot), stores p1 twice (the variable and the first QueryMin's
//   parameter, which shares the dead d1 slot) and selects with `fcomp st(1); fstp st(0);
//   fld` instead of loading either operand after the branch.  Tried for the min/max: the
//   helpers by value, by const reference, as if/return bodies, with swapped comparisons, as
//   macros, as three-argument inlines (VC6 calls them out of line); QueryMin(p0, ...) and
//   (p2, p1) argument orders; the min/max split into statements; the two tests split; p1
//   declared before p2, p1 written as p0 - DotProduct(...), both dots named first.
// - TreeTreeQueryNodes 0x004280e0 (2160 bytes, 2228 here): expands the SAT of BoxOverlap
//   inline with every helper called out of line except QueryAbs, which this source
//   reproduces through an inline copy of the BoxOverlap body (BoxOverlapInline; BoxOverlap
//   itself is not expanded in SweptBoxOverlap or TreeBoxOverlap, so it cannot be the inline
//   function).  Differences: VC6 here still expands the first a[] constructor, and the
//   recursion tails are cross-jumped differently (retail's leaf branches jump into the
//   b-children tail of the box branch).
// - SegmentSegmentDistance 0x0042a640 (1471 bytes, 0.89 instruction ratio): written with
//   explicit call views.  Retail homes the plane parameters, clamps and the first length in
//   the dead d0/d1 argument slots (frame 0x60); VC6 here keeps them in a 0x80 frame, and the
//   inline cross products load a's components first in the x term.  The natural operator
//   form expands more helpers than retail (see the notes at the function).
//
// - CapsuleTreeQuery 0x004298c0 (1418 vs 1436 bytes, 532 match): the frame (0x54), the
//   inline transforms and the out-of-line Vec3 operator calls match through the |dist| test
//   (the point branch needs its own block so its locals share the triangle branch's slots);
//   from there VC6 here assigns the operator result temporaries other slots and shares the
//   `return 1` of the PointInTriangle test, where retail returns inline.  Calls
//   SegmentSegmentDistance 0x0042a640 (near miss below) for the three edges.  Retail calls
//   every Vec3 operator out of line here; the natural operator form expands most of them.
//
// - SegmentTriangleQuery 0x00427c10 (1101 bytes, 1110 here): the natural operator form gives
//   retail's exact sequence of inline and out-of-line helpers (five constructor calls,
//   operator*(float, Vec3), three Vec3::operator+= calls).  The segment is moved into the
//   triangle's frame relative to the sweep record's offset, clipped against the face plane,
//   and on a hit inside the triangle the record's offset, contact, scale, count and the
//   contact normal list (0x00579068/0x00579014, deduplicated through 0x005299e0 unless
//   0x00579668 is set) are updated; the two flag branches are literal (the set-flag branch
//   adds the push vector a second time and replaces the contact).  Differences: retail keeps
//   the constant 1 in ebp (count, scratch count and result), so its register plan and the
//   copy of segment[1] at entry differ, and the dot product of the set-flag branch loads z
//   first.
//
// - BoxCapsuleOverlap 0x00425900 (4314 bytes, 4237 here): first draft with the decoded
//   structure (box axes, axes crossed with the segment direction, three corner directions).
//   The segment transform, the midpoint/half-segment constructor calls and the box-axis tests
//   match; the inline budget differs from retail from the fifth normalisation on (retail
//   calls DotProduct in all seven, and both squared-corner constructors), and the sums of the
//   projected radii are accumulated in another x87 order.
#include "bvh/BoundingBoxTreeQuery.h"

// The unit's file statics (bound to the same addresses as in the src file).
static Vec3* s_vertices;            // 0x00578f4c
static Matrix4 s_relativeAB;        // 0x00578ef0
static Matrix4 s_relativeBA;        // 0x00578eb0
static Matrix4 s_motion;            // 0x00579018
static int s_mode;                  // 0x0057905c
extern QueryHit* g_CollisionBoxResult;   // 0x00579058

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

// The 15 separating-axis candidates of the box/box test, filled lazily inside the loop.
static Vec3 s_axes[15];             // 0x00578f50

// v / |v| (returns v when |v|^2 == 1); its out-of-line copy is 0x005087b0 (Math3D.h's
// Vec3Normalize).
inline Vec3 Vec3NormalizeInline(const Vec3& v)
{
    float sq = DotProduct(v, v);
    if (sq == 1.0f)
        return v;
    float inv = FastInvSqrt(sq);
    return v * inv;
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
    Vec3 d = mid - p0;
    if (QueryAbs(mid.x) > QueryAbs(d.x) + halfExtents->x + radius)
        return 0;
    if (QueryAbs(mid.y) > QueryAbs(d.y) + halfExtents->y + radius)
        return 0;
    if (QueryAbs(mid.z) > QueryAbs(d.z) + radius + halfExtents->z)
        return 0;
    Vec3 a[3];
    a[0] = Vec3(halfExtents->x, 0.0f, 0.0f);
    a[1] = Vec3(0.0f, halfExtents->y, 0.0f);
    a[2] = Vec3(0.0f, 0.0f, halfExtents->z);
    Vec3 u = Vec3NormalizeInline(p1 - p0);
    Vec3 l;
    l = Vec3NormalizeInline(Vec3(-u.z, 0.0f, u.x));
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
        return 0;
    l = Vec3NormalizeInline(Vec3(u.y, -u.x, 0.0f));
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
        return 0;
    l = Vec3NormalizeInline(Vec3(0.0f, u.z, -u.y));
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
        return 0;
    Vec3 c0(QueryAbs(halfExtents->x) - QueryAbs(p0.x), QueryAbs(halfExtents->y) - QueryAbs(p0.y),
            QueryAbs(halfExtents->z) - QueryAbs(p0.z));
    Vec3 c1(QueryAbs(halfExtents->x) - QueryAbs(p1.x), QueryAbs(halfExtents->y) - QueryAbs(p1.y),
            QueryAbs(halfExtents->z) - QueryAbs(p1.z));
    Vec3 s0 = Vec3(c0.x * c0.x, c0.y * c0.y, c0.z * c0.z);
    Vec3 s1 = Vec3(c1.x * c1.x, c1.y * c1.y, c1.z * c1.z);
    Vec3 w;
    if (s0.x + s0.y < s1.x + s1.y) {
        w.x = p0.x < 0.0f ? -s0.x : s0.x;
        w.y = p0.y < 0.0f ? -s0.y : s0.y;
    } else {
        w.x = p1.x < 0.0f ? -s1.x : s1.x;
        w.y = p1.y < 0.0f ? -s1.y : s1.y;
    }
    w.z = 0.0f;
    l = Vec3NormalizeInline(w);
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
        return 0;
    if (s0.x + s0.z < s1.x + s1.z) {
        w.x = p0.x < 0.0f ? -s0.x : s0.x;
        w.z = p0.z < 0.0f ? -s0.z : s0.z;
    } else {
        w.x = p1.x < 0.0f ? -s1.x : s1.x;
        w.z = p1.z < 0.0f ? -s1.z : s1.z;
    }
    w.y = 0.0f;
    l = Vec3NormalizeInline(w);
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
        return 0;
    if (s0.y + s0.z < s1.y + s1.z) {
        w.y = p0.y < 0.0f ? -s0.y : s0.y;
        w.z = p0.z < 0.0f ? -s0.z : s0.z;
    } else {
        w.y = p1.y < 0.0f ? -s1.y : s1.y;
        w.z = p1.z < 0.0f ? -s1.z : s1.z;
    }
    w.x = 0.0f;
    l = Vec3NormalizeInline(w);
    if (QueryAbs(l.z * mid.z + (l.x * mid.x + l.y * mid.y)) > QueryAbs(DotProduct(l, a[0])) + QueryAbs(DotProduct(l, a[1])) + QueryAbs(DotProduct(l, a[2])) +
                                         QueryAbs(l.z * d.z + (l.x * d.x + l.y * d.y)) + radius)
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
    Vec3 n;
    RotateVectorInline(&n, tri->normal, m);
    Vec3 a[3];
    a[0] = Vec3(halfExtents->x, 0.0f, 0.0f);
    a[1] = Vec3(0.0f, halfExtents->y, 0.0f);
    a[2] = Vec3(0.0f, 0.0f, halfExtents->z);
    Vec3 c = v[0] - *center;
    Vec3 e0, e1, e2;
    for (int i = 0; i < 13; i++) {
        switch (i) {
        case 0: s_axes[0] = n; break;
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
        float r = QueryAbs(DotProduct(a[0], s_axes[i])) + QueryAbs(DotProduct(a[1], s_axes[i])) + QueryAbs(DotProduct(a[2], s_axes[i]));
        if (i == 0) {
            if (QueryAbs(DotProduct(c, s_axes[0])) > r)
                return 0;
            e0 = v[0] - v[1];
            e1 = v[0] - v[2];
        } else {
            float p0 = DotProduct(c, s_axes[i]);
            float d1 = DotProduct(e0, s_axes[i]);
            float p2 = p0 - DotProduct(e1, s_axes[i]);
            float p1 = p0 - d1;
            float mn = QueryMin(QueryMin(p1, p2), p0);
            float mx = QueryMax(QueryMax(p1, p2), p0);
            if (-r > mx || r < mn)
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

inline float SegClamp01(float v) { return v < 1.0f ? QueryMax(v, 0.0f) : 1.0f; }

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
        push = -DotProduct(dir * t, tri->normal) * tri->normal;
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
