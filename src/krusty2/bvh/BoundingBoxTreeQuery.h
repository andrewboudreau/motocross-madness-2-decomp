// BoundingBoxTreeQuery.h -- the run-time box-tree query unit (0x00424690..0x0042ad2f).
//
// The unit has no __FILE__ string and no RTTI, so its file name is not attested; the
// header and source names are ours (tier 3).  Extent and evidence: see the top of
// BoundingBoxTreeQuery.cpp.
//
// The node layouts are the ones BoundingBoxTreeBuild.h documents for the builder (the same
// 0x24-byte interior node, 0x1c-byte triangle and segment ("box") leaves and 0x10-byte point
// leaf).  This unit is written against the Math3D Vec3/Matrix4 and has its own per-unit
// kVec3 set (Math3D.h), so it cannot include BoundingBoxTreeBuild.h, which declares the
// builder's kVec3 constants extern.  All names are tier 3.
#ifndef BOUNDING_BOX_TREE_QUERY_H
#define BOUNDING_BOX_TREE_QUERY_H

#include "math/Math3D.h"

// Interior node, 0x24 bytes.  volume >= 0 marks an interior node; a leaf stores -1.0f there.
struct QueryTreeNode {
    float volume;               // +0x00
    Vec3 center;                // +0x04 box centre
    Vec3 halfExtents;           // +0x10 box half size
    QueryTreeNode* child[2];    // +0x1c, +0x20
};

// Triangle of a triangle leaf (the leaf from +0x04): what the triangle helpers receive.
struct QueryTriangle {
    unsigned short vertex[3];   // +0x00 indices into the query's vertex array (0x00578f4c)
    Vec3 normal;                // +0x08 face normal
    float planeOffset;          // +0x14
};

// Triangle leaf, 0x1c bytes.
struct QueryTriangleLeaf {
    float marker;               // +0x00 -1.0f
    QueryTriangle tri;          // +0x04
};

// Point leaf, 0x10 bytes.
struct QueryPointLeaf {
    float marker;               // +0x00 -1.0f
    Vec3 point;                 // +0x04
};

// Segment leaf of a model tree, 0x1c bytes (BoundingBoxTreeBuild.h's box leaf: two corners,
// used by the leaf tests as the two ends of a segment).
struct QuerySegmentLeaf {
    float marker;               // +0x00 -1.0f
    Vec3 end[2];                // +0x04, +0x10
};

// Hit record the leaf tests update through 0x00579058 (the 0x14-byte mesh record
// CollisionObject allocates): the hit fraction along the segment, the segment leaf's first
// point, the face normal.
struct QueryHit {
    float fraction;             // +0x00
    const Vec3* point;          // +0x04
    Vec3 normal;                // +0x08
};

// ---------------------------------------------------------------------------------------
// Inline helpers as this unit's code expands them.
// ---------------------------------------------------------------------------------------

// Dot product and squared length: VC6 emits (y + x) + z for this grouping (tier 2:
// PointSegmentDistance 0x0042ac00 and SegmentBoxOverlap 0x004253b0 are exact only with it;
// the out-of-line DotProduct copy 0x0040ae30 sums in the same order, and so does
// Vec3Normalize 0x005087b0, see math/Math3D.h).  The parentheses around a.y * b.y make
// PointInTriangle 0x004278d0 store a copied component after the inner faddp, as retail does
// (docs/VC6_OPERAND_ORDER.md section 3); the other users keep their bytes.
inline float QueryDot(const Vec3& a, const Vec3& b) { return a.z * b.z + (a.x * b.x + (a.y * b.y)); }
inline float QuerySquareMagnitude(const Vec3& v) { return v.z * v.z + (v.x * v.x + v.y * v.y); }

// fabs as compare-and-negate (constant 0x00550484), and the min/max the box tests use.
inline float QueryAbs(float v) { return v < 0.0f ? -v : v; }
inline float QueryMin(float a, float b) { return a < b ? a : b; }
inline float QueryMax(float a, float b) { return a > b ? a : b; }

// p * inverse(m) for a rigid m: subtract the translation row, then dot with each rotation row.
// The by-value point is modified in place: that is what makes VC6 copy it at the call site
// (BoxSphereOverlap 0x00425750).  Same arithmetic as the out-of-line InverseTransformPoint
// 0x0042a5e0.
inline void InverseTransformPointInline(Vec3* out, Vec3 p, const Matrix4* m)
{
    p.x -= m->_41;
    p.y -= m->_42;
    p.z -= m->_43;
    out->x = p.x * m->_11 + p.y * m->_12 + p.z * m->_13;
    out->y = p.x * m->_21 + p.y * m->_22 + p.z * m->_23;
    out->z = p.x * m->_31 + p.y * m->_32 + p.z * m->_33;
}

// out = v * m without / with the translation row (row-vector convention).
inline void RotateVectorInline(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

inline void TransformPointInline(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// In-place inverse of a rigid transform (BoundingBoxTreeBuild.h's InvertRigid, on Matrix4).
inline void InvertRigid(Matrix4* m)
{
    float swap;
    swap = m->_12; m->_12 = m->_21; m->_21 = swap;
    swap = m->_13; m->_13 = m->_31; m->_31 = swap;
    swap = m->_23; m->_23 = m->_32; m->_32 = swap;
    Vec3 t;
    t.x = -(m->_41 * m->_11 + m->_42 * m->_21 + m->_43 * m->_31);
    t.y = -(m->_41 * m->_12 + m->_42 * m->_22 + m->_43 * m->_32);
    t.z = -(m->_41 * m->_13 + m->_42 * m->_23 + m->_43 * m->_33);
    m->_41 = t.x;
    m->_42 = t.y;
    m->_43 = t.z;
}

// ---------------------------------------------------------------------------------------
// Out-of-line call views (see math/Math3D.h): sites where retail calls the non-inlined
// COMDAT copy of a Vec3 helper instead of expanding it (VC6 inline budget).  Declared only;
// the bindings map them to the retail copies.
// ---------------------------------------------------------------------------------------

// Vec3::Vec3(float, float, float), 0x00404e60 (thiscall, ret 0xc).
struct Vec3Call : Vec3 {
    Vec3Call(float x_, float y_, float z_);
};

// Vec3::operator+=, 0x00428060 (thiscall, ret 4).
struct Vec3AddAssignCall : Vec3 {
    Vec3AddAssignCall() {}
    Vec3AddAssignCall& operator+=(const Vec3& v);
};

// By-value views of the out-of-line operators (same ABI: hidden result pointer first).
Vec3 Vec3Sub(const Vec3& a, const Vec3& b);     // 0x00421d00 operator-
Vec3 Vec3Add(const Vec3& a, const Vec3& b);     // 0x00421cb0 operator+
Vec3 Vec3Scale(const Vec3& v, float s);         // 0x005015b0 operator*(const Vec3&, float)
float Vec3Dot(const Vec3& a, const Vec3& b);    // 0x0040ae30 DotProduct
Vec3 Vec3ScaleLeft(float s, const Vec3& v);     // 0x00428090 operator*(float, const Vec3&)
float Vec3LengthCall(const Vec3* v);            // 0x00435ec0 length (CollisionLength)

// The operators expanded inline while their constructor is called out of line (the nested
// expansion is the first one VC6 gives up when the budget runs low).
inline Vec3 SubtractCtorCall(const Vec3& a, const Vec3& b)
{
    return Vec3Call(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec3 AddCtorCall(const Vec3& a, const Vec3& b)
{
    return Vec3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3 ScaleCtorCall(const Vec3& v, float s)
{
    return Vec3Call(v.x * s, v.y * s, v.z * s);
}

// ---------------------------------------------------------------------------------------
// The unit's functions (cdecl).
// ---------------------------------------------------------------------------------------

int SweptBoxOverlap(Vec3 aCenter, Vec3 aHalf, const Vec3* bCenter, const Vec3* bHalf,
                    const Matrix4* rel, const Matrix4* xfA);                       // 0x00424730
int BoxOverlap(const Vec3* aCenter, const Vec3* aHalfExtents, Vec3 bCenter, Vec3 bHalfExtents,
               const Matrix4* bToA);                                               // 0x00424ab0
int SegmentBoxOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 p0, Vec3 p1,
                      const Matrix4* m);                                           // 0x004253b0
int BoxSphereOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 sphereCenter,
                     float radius, float radiusSq, const Matrix4* xf);             // 0x00425750
int BoxCapsuleOverlap(const Vec3* center, const Vec3* halfExtents, const Vec3* ends,
                      float radius, float radiusSq, const Matrix4* xf);            // 0x00425900
int LeafNodeQuery(QueryTreeNode* a, QueryTreeNode* b, int useMotion);              // 0x004269e0
int BoxTriangleQuery(const Vec3* center, const Vec3* halfExtents, const QueryTriangle* tri,
                     const Matrix4* m);                                            // 0x00426be0
int LeafPairQuery(QuerySegmentLeaf* a, QueryTriangleLeaf* b);                      // 0x004275f0
int PointInTriangle(const Vec3* p, const QueryTriangle* tri);                      // 0x004278d0
int SegmentTriangleQuery(const QueryTriangle* tri, const Vec3* segment, const Matrix4* m);
                                                                                   // 0x00427c10
int TreeTreeQueryNodes(QueryTreeNode* a, QueryTreeNode* b, int useMotion);         // 0x004280e0
int TreeTreeQuery(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA, const Matrix4* xfB,
                  int mode, Vec3* vertices, const Matrix4* motion);                // 0x00428950
int TreeBoxOverlap(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA,
                   const Matrix4* xfB);                                            // 0x00428c20
void MoveBox(Vec3* center, Vec3* halfExtents, const Matrix4* m);                   // 0x00428db0
void GrowBoxByTransformedBox(Vec3* outCenter, Vec3* outHalfExtents, Vec3 center,
                             Vec3 halfExtents, const Matrix4* m);                  // 0x004290d0
int SphereTreeQueryWithVertices(const Vec3* center, float radius, float radiusSq,
                                QueryTreeNode* node, const Matrix4* xf, int triangles,
                                Vec3* vertices);                                   // 0x00429540
int SphereTreeQuery(const Vec3* center, float radius, float radiusSq, QueryTreeNode* node,
                    const Matrix4* xf, int triangles);                             // 0x00429570
int CapsuleTreeQueryWithVertices(const Vec3* ends, float radius, float radiusSq,
                                 QueryTreeNode* node, const Matrix4* xf, int triangles,
                                 Vec3* vertices);                                  // 0x00429890
int CapsuleTreeQuery(const Vec3* ends, float radius, float radiusSq, QueryTreeNode* node,
                     const Matrix4* xf, int triangles);                            // 0x004298c0
int SegmentTreeQueryWithVertices(const Vec3* ends, QueryTreeNode* node, const Matrix4* xf,
                                 Vec3* vertices);                                  // 0x00429e60
int SegmentTreeQuery(const Vec3* ends, QueryTreeNode* node, const Matrix4* xf);    // 0x00429e90
void FreeBoxTree(QueryTreeNode* node);                                             // 0x0042a160
void Vec3TransformNormal(Vec3* out, Vec3 v, const Matrix4* m);                     // 0x0042a450
void Vec3TransformNormalTranspose(Vec3* out, Vec3 v, const Matrix4* m);            // 0x0042a4b0
void Vec3TransformPoint(Vec3* out, Vec3 v, const Matrix4* m);                      // 0x0042a510
void TransformPointPtr(Vec3* out, const Vec3* v, const Matrix4* m);                // 0x0042a580
void InverseTransformPoint(Vec3* out, Vec3 p, const Matrix4* m);                   // 0x0042a5e0
// 0x0042a640: a distance between the segments p0 + s*d0 and p1 + t*d1 (0 when d0 x d1 is
// zero); CapsuleTreeQuery reports a hit when it exceeds the radius.  Near miss in
// samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp.
float SegmentSegmentDistance(const Vec3* p0, const Vec3* d0, const Vec3* p1, const Vec3* d1);
float PointSegmentDistance(const Vec3* p, const Vec3* a, const Vec3* b);           // 0x0042ac00

#endif
