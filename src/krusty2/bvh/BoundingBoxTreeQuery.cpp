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
// Near misses and the functions not yet matched are in
// samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp.
#include "bvh/BoundingBoxTreeQuery.h"

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
        const Vec3* c = &aCenter;
        TransformPointPtr(&moved, c, xfA);
        delta.x = moved.x - c->x;
        delta.y = moved.y - c->y;
        delta.z = moved.z - c->z;
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
// their nine cross products.  Naming `dist` and `ra` but not `rb` sets the inline budget
// that calls all seven dot products out of line, as retail does; the inline cross products
// need Math3D.h's parenthesised CrossProduct (docs/VC6_OPERAND_ORDER.md section 3).
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
        float dist = QueryAbs(DotProduct(t, s_axes[i]));
        float ra = QueryAbs(DotProduct(a[0], s_axes[i])) + QueryAbs(DotProduct(a[1], s_axes[i])) + QueryAbs(DotProduct(a[2], s_axes[i]));
        if (dist > ra + QueryAbs(DotProduct(b[0], s_axes[i])) + QueryAbs(DotProduct(b[1], s_axes[i])) + QueryAbs(DotProduct(b[2], s_axes[i])))
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
    Vec3 d = SubtractCtorCall(mid, p0);
    Vec3 ad;
    ad.x = QueryAbs(d.x);
    if (QueryAbs(mid.x) > ad.x + halfExtents->x)
        return 0;
    ad.y = QueryAbs(d.y);
    if (QueryAbs(mid.y) > ad.y + halfExtents->y)
        return 0;
    ad.z = QueryAbs(d.z);
    if (QueryAbs(mid.z) > ad.z + halfExtents->z)
        return 0;
    if (QueryAbs(mid.y * d.z - mid.z * d.y) > halfExtents->y * ad.z + halfExtents->z * ad.y)
        return 0;
    if (QueryAbs(mid.x * d.z - mid.z * d.x) > halfExtents->x * ad.z + halfExtents->z * ad.x)
        return 0;
    if (QueryAbs(mid.x * d.y - mid.y * d.x) > halfExtents->x * ad.y + halfExtents->y * ad.x)
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

// 0x00429540.
int SphereTreeQueryWithVertices(const Vec3* center, float radius, float radiusSq,
                                QueryTreeNode* node, const Matrix4* xf, int triangles,
                                Vec3* vertices)
{
    s_vertices = vertices;
    return SphereTreeQuery(center, radius, radiusSq, node, xf, triangles);
}

// 0x00429890.
int CapsuleTreeQueryWithVertices(const Vec3* ends, float radius, float radiusSq,
                                 QueryTreeNode* node, const Matrix4* xf, int triangles,
                                 Vec3* vertices)
{
    s_vertices = vertices;
    return CapsuleTreeQuery(ends, radius, radiusSq, node, xf, triangles);
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
    Vec3 q = *a + ab * t;
    Vec3 d = *p - q;
    float distSq = QuerySquareMagnitude(d);
    if (distSq == 1.0f)
        return 1.0f;
    return FastSqrt(distSq);
}
