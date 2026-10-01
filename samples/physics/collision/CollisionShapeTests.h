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

#include "CollisionObject.h"
#include "CollisionPoint.h"
#include "../common/Math3D.h"

// Box bounds object at CollisionHullShape::field_0x188: two vec3s read as
// (+4, +0x10) by every caller of the broad-phase test 0x00424730 (center, half extents;
// the extent role is from 0x00436100 where |M| * extents gives the world half size).
struct CollisionBoxBounds {
    int field_0x00;
    CollisionVec3 center;          // +0x04
    CollisionVec3 halfExtents;     // +0x10
};

// View of a type-0 hull payload (0x198 bytes, the same payload as CollisionHullShape,
// which only names part of it).  Five 4x4 matrices sit at +0x08, +0x48, +0x88, +0xc8,
// +0x108, +0x148 (six 0x40-byte slots); the tests use +0x48 as the current transform and
// +0xc8 / +0x108 as history.  field_0x00 is a mode flag selecting the swept variant.
struct CollisionHullBody {
    int field_0x00;                // nonzero: use the swept/matrix-history test
    void* field_0x04;              // scene node; 0x004fc9a0 reads its world position
    Matrix4 field_0x08;
    Matrix4 field_0x48;
    Matrix4 field_0x88;
    Matrix4 field_0xc8;
    Matrix4 field_0x108;
    Matrix4 field_0x148;
    CollisionBoxBounds* field_0x188;
    void* field_0x18c;             // convex geometry handed to the box test 0x00428950
    void* field_0x190;
    int field_0x194;               // not read here; the element stride is 0x198 (add edi,0x198 at 0x0043898a)
};

// View of a type-1 "model" payload: an array of hulls plus its own bounds.
struct CollisionModelBody {
    int elementCount;              // +0x00
    int* elementEnabled;           // +0x04, one flag per element
    int* field_0x08;               // +0x08, one flag per element (0x00432b30 draws a set
                                   // element green)
    int swept;                     // +0x0c, nonzero selects the swept element test
    int field_0x10;
    CollisionHullBody* elements;   // +0x14, elements are 0x198 bytes apart
    CollisionVec3 center;          // +0x18 model bounds center
    CollisionVec3 halfExtents;     // +0x24 model bounds half extents
    CollisionVec3 field_0x30;
    CollisionVec3 field_0x3c;
    char field_0x48[0x40];
    Matrix4 field_0x88;
    Matrix4 field_0xc8;
    CollisionVec3 field_0x108;     // written by the swept variants (0x00437ef0)
    CollisionVec3 field_0x114;
};

// View of a type-2 static-mesh payload (only the parts 0x00436100 / 0x00438c90 read).
struct CollisionMeshBody {
    void* field_0x00;
    CollisionBoxBounds* field_0x04;   // bounds object (center at +4, half extents at +0x10)
    Matrix4 field_0x08;               // mesh -> world transform
};

// ---------------------------------------------------------------------------
// Callees outside this file (all cdecl unless noted; arguments are tier 3 guesses from
// the push order at the call sites).
// ---------------------------------------------------------------------------

// 0x00428950: oriented-box (hull) vs oriented-box test.  Copies both transforms into
// file-scope scratch (0x00578ef0...), leaves contact points in g_CollisionScratchPoints
// and their count in g_CollisionScratchCount.
int Fn_00428950(void* geomA, void* geomB, const Matrix4* xfA, const Matrix4* xfB,
                int mode, void* aux, void* out);
// 0x00429570: hull vs sphere (world center, r, r*r).  0x00429890: hull vs capsule (two
// world endpoints, r, r*r).  Same trailing arguments as the box test.  The second
// argument is the radius: every caller stores r into a dead argument slot and pushes that
// dword (0x004379c0 +0xaf..+0xc9), so it is a float, not the shape pointer.
int Fn_00429570(CollisionVec3* worldCenter, float radius, float radiusSq, void* geom,
                const Matrix4* xf, int mode);
int Fn_00429890(CollisionVec3* worldEnds, float radius, float radiusSq, void* geom,
                const Matrix4* xf, int mode, void* aux);
// 0x00424730: broad-phase box overlap in a relative frame (center/half extents of box A by
// value, box B by pointer, relative transform, transform of A).
int Fn_00424730(CollisionVec3 aCenter, CollisionVec3 aHalf, const CollisionVec3* bCenter,
                const CollisionVec3* bHalf, const Matrix4* rel, const Matrix4* xfA);

// 0x00425750 / 0x00425900: broad-phase overlap of a model's bounds (center, half extents,
// in the frame of `xf`) with a sphere (world center by value) / capsule (pointer to the two
// world endpoints), given radius and radius squared.  Nonzero = may touch.
int Fn_00425750(const CollisionVec3* boundsCenter, const CollisionVec3* boundsHalf,
                CollisionVec3 worldCenter, float radius, float radiusSq, const Matrix4* xf);
int Fn_00425900(const CollisionVec3* boundsCenter, const CollisionVec3* boundsHalf,
                const CollisionVec3* worldEnds, float radius, float radiusSq, const Matrix4* xf);

// 0x0042a450: out = v * M3x3 (rotate by the matrix, no translation); 0x0042a4b0:
// out = M3x3 * v (rotate by the transpose); 0x0042a510: out = v * M + translation row.
void Fn_0042a450(CollisionVec3* out, CollisionVec3 v, const Matrix4* m);
void Fn_0042a4b0(CollisionVec3* out, CollisionVec3 v, const Matrix4* m);
void Fn_0042a510(CollisionVec3* out, CollisionVec3 v, const Matrix4* m);

// 0x004fc9a0 (thiscall on a scene node): position of the node relative to `parent`
// (parent == 0 gives the world position).
class CollisionSceneNode {
public:
    void GetPositionRelativeTo(CollisionSceneNode* parent, CollisionVec3* out);
};

// Result record of the last box test (global pointer at 0x00579058); the mesh tests read a
// vec3 at +8 from it and rotate it into world space.  Tier 3 shape.
struct CollisionBoxResult {
    char field_0x00[8];
    CollisionVec3 field_0x08;
};
extern CollisionBoxResult* g_CollisionBoxResult;   // 0x00579058

// Scratch contact list filled by the box tests (globals, .bss).
extern int g_CollisionScratchCount;             // 0x00579014
extern CollisionVec3 g_CollisionScratchPoints[];  // 0x00579068, 12 bytes each

// Contact/sweep record passed to the hull-vs-hull family (tier 3 layout): two input
// vectors, an accumulated contact position and a contact counter.
struct CollisionSweepQuery {
    CollisionVec3 field_0x00;
    CollisionVec3 field_0x0c;
    CollisionVec3 contact;         // +0x18
    int field_0x24;
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
    r.x = v.z * m->m[2][0] + v.y * m->m[1][0] + v.x * m->m[0][0] + m->m[3][0];
    r.y = v.z * m->m[2][1] + v.y * m->m[1][1] + v.x * m->m[0][1] + m->m[3][1];
    r.z = v.z * m->m[2][2] + v.y * m->m[1][2] + v.x * m->m[0][2] + m->m[3][2];
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

// 0x004290d0: bounds of a swept box in another frame (provisional signature: two output
// vec3s, two input vec3s by value, a transform).  Results land in CollisionModelBody
// field_0x108 / field_0x114 in 0x00437ef0 and 0x00438550.
void Fn_004290d0(CollisionVec3* outCenter, CollisionVec3* outHalf, CollisionVec3 center,
                 CollisionVec3 half, const Matrix4* xf);

// 0x00432180 (cdecl): out = to * inverse(from) for rigid transforms.
void CollisionRelativeTransform(Matrix4* out, const Matrix4* from, const Matrix4* to);

void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b);   // 0x00436500

#endif
