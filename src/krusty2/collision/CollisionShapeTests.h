// CollisionShapeTests.h -- narrow-phase shape tests (0x00436100-0x004389b0 and friends).
//
// Shape payload views (all tier 3, offsets are confirmed by the decoded accesses):
//
//   CollisionObject::field_0x50 selects the payload hung off field_0x54:
//     0 = oriented-box "hull" (0x198 bytes)   1 = "model", an array of hulls
//     2 = static mesh                         3 = capsule      4 = sphere
//   The dispatch tables at 0x00438bf0 (hull) and 0x00438c70 (model) map the OTHER
//   object's type 0..4 to a handler; type 2 is not handled there (returns 0), so the
//   mesh (type 2) is only ever the "this" side (0x00438c90).
#ifndef COLLISION_SHAPE_TESTS_H
#define COLLISION_SHAPE_TESTS_H

#include "collision/CollisionObject.h"
#include "CollisionPoint.h"
#include "math/Math3D.h"
#include "bvh/BoundingBoxTreeQueryFunctions.h"

// Box bounds object at CollisionHullShape::field_0x188: two vec3s read as
// (+4, +0x10) by every caller of the broad-phase test 0x00424730 (center, half extents;
// the extent role is from 0x00436100 where |M| * extents gives the world half size).
struct CollisionBoxBounds {
    int field_0x00;
    CollisionVec3 center;          // +0x04
    CollisionVec3 halfExtents;     // +0x10
    void* child[2];                // +0x1c, +0x20: the root is a 0x24-byte tree node; 0x004392c0's
                                   // two model-bounds locals take 0x24 bytes each
};

// View of a type-0 hull payload (0x198 bytes, the same payload as CollisionHullShape,
// which only names part of it).  Five 4x4 matrices sit at +0x08, +0x48, +0x88, +0xc8,
// +0x108, +0x148 (six 0x40-byte slots); the tests use +0x48 as the current transform and
// +0xc8 / +0x108 as history.  field_0x00 is a mode flag selecting the swept variant.
struct CollisionHullBody {
    int swept;                // +0x00 nonzero: use the swept/matrix-history test
    void* sceneNode;              // +0x04 scene node; 0x004fc9a0 reads its world position
    Matrix4 motionTransform;  // +0x08 SetTransform 0x00435830 builds it with CollisionRelativeTransform 0x00432180 from the current and previous body matrices; DrawTreeMotion receives it as the motion matrix
    Matrix4 worldTransform;  // +0x48 SetTransform: 3x3 of localTransform (+0x88) times the body matrix plus translation; used as the frame by every hull test (TreeTreeQuery, 0x00429570/890)
    Matrix4 localTransform;  // +0x88 SetTransform multiplies it by the new body matrix to produce +0x48
    Matrix4 bodyTransform;  // +0xc8 SetTransform copies the argument matrix here after moving the old one to +0x108; the swept tests use it as the current frame
    Matrix4 prevBodyTransform;  // +0x108 SetTransform: rep movsd +0xc8 -> +0x108 before the new matrix is stored in +0xc8
    Matrix4 relativeFrame;  // +0x148 written by HullVsHullSwept (0x00432260 output), passed to DrawPointTree as the motion frame
    CollisionBoxBounds* triangleTree;  // +0x188 DrawHull -> DrawTree(+0x188, ..., verts +0x190); root node carries the bounds read through CollisionBoxBounds
    void* pointTree;             // +0x18c convex geometry handed to the box test 0x00428950
    void* vertices;  // +0x190 vertex array passed to DrawTree / 0x00429890
    int field_0x194;               // not read here; the element stride is 0x198 (add edi,0x198 at 0x0043898a)
};

// View of a type-1 "model" payload: an array of hulls plus its own bounds.
struct CollisionModelBody {
    int elementCount;              // +0x00
    int* elementEnabled;           // +0x04, one flag per element
    int* elementHighlight;               // +0x08, one flag per element (0x00432b30 draws a set
                                   // element green)
    int swept;                     // +0x0c, nonzero selects the swept element test
    int field_0x10;
    CollisionHullBody* elements;   // +0x14, elements are 0x198 bytes apart
    CollisionVec3 center;          // +0x18 model bounds center
    CollisionVec3 halfExtents;     // +0x24 model bounds half extents
    CollisionVec3 field_0x30;
    CollisionVec3 field_0x3c;
    Matrix4 field_0x48;            // +0x48 SetTransform keeps the previous field_0x88 here
    Matrix4 field_0x88;
    Matrix4 field_0xc8;
    CollisionVec3 sweptCenter;     // +0x108 written by the swept variants (0x00437ef0)
    CollisionVec3 sweptHalfExtents;  // +0x114 second output of GrowBoxByTransformedBox, passed as A half extents to 0x00424730
};

// View of a type-2 static-mesh payload (only the parts 0x00436100 / 0x00438c90 read).
struct CollisionMeshBody {
    void* field_0x00;
    CollisionBoxBounds* field_0x04;   // bounds object (center at +4, half extents at +0x10)
    Matrix4 field_0x08;               // mesh -> world transform
};

// ---------------------------------------------------------------------------
// Remaining provisional call views. The connected query entry points are declared in
// BoundingBoxTreeQueryFunctions.h; CollisionObject casts only at the call boundary.
// Their pointer/value ABI is verified without equating whole provisional classes.
// ---------------------------------------------------------------------------

// 0x00425900: this capsule broad-phase provider is still missing from the game link.
int BoxCapsuleOverlap(const CollisionVec3* boundsCenter, const CollisionVec3* boundsHalf,
                const CollisionVec3* worldEnds, float radius, float radiusSq, const Matrix4* xf);

// 0x0042a4b0: rotate by the transpose; this provisional call view is not connected here.
void Vec3TransformNormalTranspose(CollisionVec3* out, CollisionVec3 v, const Matrix4* m);

class CollisionSceneNode {
public:
    void GetMatrixIn(CollisionSceneNode* frame, Matrix4* out);   // 0x004fca80
};

// Result record of the last box test (global pointer at 0x00579058); the mesh tests read a
// vec3 at +8 from it and rotate it into world space.  Tier 3 shape.
struct CollisionBoxResult {
    float fraction;                // +0x00 hit fraction along the segment (0x00439820 starts it at 1.0)
    const CollisionVec3* point;    // +0x04 segment leaf point (0x00439820 starts it at 0)
    CollisionVec3 field_0x08;      // +0x08 face normal
};
extern CollisionBoxResult* g_CollisionBoxResult;   // 0x00579058

// Scratch contact list filled by the box tests (globals, .bss).
extern int g_CollisionScratchCount;             // 0x00579014
extern CollisionVec3 g_CollisionScratchPoints[];  // 0x00579068, 12 bytes each

// Contact/sweep record passed to the hull-vs-hull family (tier 3 layout): two input
// vectors, an accumulated contact position and a contact counter.
struct CollisionSweepQuery {
    CollisionVec3 contactOffset;  // +0x00 rotated as a direction (CollisionRotateRows) into the other hull frame in HullVsHull 0x00436720; ConstraintContactCallback 0x0043b800 passes record+0 as ApplyContactImpulse's offset argument
    CollisionVec3 contactNormal;  // +0x0c rotated as a direction like +0x00; ConstraintContactCallback passes record+0xc as the normal argument of ApplyContactImpulse
    CollisionVec3 contact;         // +0x18
    int fraction;  // +0x24 ConstraintContactCallback 0x0043b800 passes record+0x24 as ApplyContactImpulse's interpolation factor t (prev velocity .. velocity)
    int contactCount;              // +0x28
};

// Inverse of a rigid transform in the row-vector convention: transpose the 3x3 block and
// t' = -t * R^T.  Retail inlines this (transposes in place on a stack copy, translation
// dot products negated) wherever a model is moved into another frame, e.g. 0x00437c20.
inline void CollisionInvertRigid(Matrix4* out, const Matrix4* src)
{
    *out = *src;
    float t;
    t = out->m[0][1]; out->m[0][1] = out->m[1][0]; out->m[1][0] = t;
    t = out->m[0][2]; out->m[0][2] = out->m[2][0]; out->m[2][0] = t;
    t = out->m[1][2]; out->m[1][2] = out->m[2][1]; out->m[2][1] = t;
    CollisionVec3 p;
    p.x = -((out->m[3][2] * out->m[2][0] + out->m[3][1] * out->m[1][0]) + out->m[3][0] * out->m[0][0]);
    p.y = -((out->m[3][2] * out->m[2][1] + out->m[3][1] * out->m[1][1]) + out->m[3][0] * out->m[0][1]);
    p.z = -((out->m[3][2] * out->m[2][2] + out->m[3][1] * out->m[1][2]) + out->m[3][0] * out->m[0][2]);
    out->m[3][0] = p.x;
    out->m[3][1] = p.y;
    out->m[3][2] = p.z;
}

// Variant whose p.y sum order matches CollisionRelativeFrame (0x432260).
inline void CollisionInvertRigidRelFrame(Matrix4* out, const Matrix4* src)
{
    *out = *src;
    float t;
    t = out->m[0][1]; out->m[0][1] = out->m[1][0]; out->m[1][0] = t;
    t = out->m[0][2]; out->m[0][2] = out->m[2][0]; out->m[2][0] = t;
    t = out->m[1][2]; out->m[1][2] = out->m[2][1]; out->m[2][1] = t;
    CollisionVec3 p;
    p.x = -((out->m[3][2] * out->m[2][0] + out->m[3][1] * out->m[1][0]) + out->m[3][0] * out->m[0][0]);
    p.y = -((out->m[3][1] * out->m[1][1] + out->m[3][0] * out->m[0][1]) + out->m[3][2] * out->m[2][1]);
    p.z = -((out->m[3][2] * out->m[2][2] + out->m[3][1] * out->m[1][2]) + out->m[3][0] * out->m[0][2]);
    out->m[3][0] = p.x;
    out->m[3][1] = p.y;
    out->m[3][2] = p.z;
}

// Inline forms of the vector/matrix helpers the shape tests use in row-vector convention
// (component sums are ordered z, y, x as the retail x87 code evaluates them).
//   CollisionRotateRows:   M3x3 * v   (dot each ROW with v; same result as 0x0042a4b0)
//   CollisionRotateCols:   v * M3x3   (dot each COLUMN with v; same result as 0x0042a450)
//   CollisionTransformPoint: v * M + translation row (same result as 0x0042a510)
inline CollisionVec3 CollisionRotateRows(const CollisionVec3& v, const Matrix4* m)
{
    CollisionVec3 r;
    r.x = v.z * m->m[0][2] + v.y * m->m[0][1] + v.x * m->m[0][0];
    r.y = v.z * m->m[1][2] + v.y * m->m[1][1] + v.x * m->m[1][0];
    r.z = v.z * m->m[2][2] + v.y * m->m[2][1] + v.x * m->m[2][0];
    return r;
}
inline CollisionVec3 CollisionRotateCols(const CollisionVec3& v, const Matrix4* m)
{
    CollisionVec3 r;
    r.x = v.z * m->m[2][0] + v.y * m->m[1][0] + v.x * m->m[0][0];
    r.y = v.z * m->m[2][1] + v.y * m->m[1][1] + v.x * m->m[0][1];
    r.z = v.z * m->m[2][2] + v.y * m->m[1][2] + v.x * m->m[0][2];
    return r;
}
inline CollisionVec3 CollisionTransformPoint(const CollisionVec3& v, const Matrix4* m)
{
    CollisionVec3 r;
    r.x = v.x * m->m[0][0] + v.y * m->m[1][0] + v.z * m->m[2][0] + m->m[3][0];
    r.y = v.x * m->m[0][1] + v.y * m->m[1][1] + v.z * m->m[2][1] + m->m[3][1];
    r.z = v.x * m->m[0][2] + v.y * m->m[1][2] + v.z * m->m[2][2] + m->m[3][2];
    return r;
}

// 0x00432260 (see CollisionShapeTests.cpp): relative matrix between three frames.
void CollisionRelativeFrame(Matrix4* out, const Matrix4* m1, const Matrix4* m2, const Matrix4* m3);

// fabs as retail writes it: compare with 0 (constant 0x00550484) and negate (fchs) if below.
inline float CollisionAbs(float v) { return v < 0.0f ? -v : v; }

// Running sum used by HullVsModel (0x00436e50): three floats with the out-of-line
// accumulate 0x00428060 (thiscall, ret 4).  Tier 3 shape; only the call is confirmed.
struct CollisionContactSum {
    float x, y, z;
    CollisionContactSum() { x = 0.0f; y = 0.0f; z = 0.0f; }
    void Accumulate(const CollisionVec3* v);   // 0x00428060
};

// 0x00432180 (cdecl): out = to * inverse(from) for rigid transforms.
void CollisionRelativeTransform(Matrix4* out, const Matrix4* from, const Matrix4* to);

void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b);   // 0x00436500

#endif
