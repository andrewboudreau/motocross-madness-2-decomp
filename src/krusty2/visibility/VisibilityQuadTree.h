// VisibilityQuadTree.h -- VisibilityQuadTree / VisibilityQuadTreeNode (VisibilityQuadTree.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI .?AVVisibilityQuadTree@@ : QuadTree (mdisp 0), GameObject (mdisp 2164 = 0x874),
//    plain inheritance.  Primary vtable 0x00558dfc @0 (QuadTree's 2 slots), secondary vtable
//    0x00558d8c @0x874 (GameObject shape, 27 slots).  QuadTree is 0x874 bytes
//    (broadphase/Quadtree.h), so GameObject sits right behind it.
//  * RTTI .?AVVisibilityQuadTreeNode@@ : QuadTreeNode, vtable 0x00558e08.
//  * The constructor 0x0052d340 inlines QuadTree's (no out-of-line QuadTree ctor exists) and
//    calls GameObject(int) 0x00468ca0 on this+0x874.
#ifndef VISIBILITY_QUADTREE_H
#define VISIBILITY_QUADTREE_H

#include "broadphase/Quadtree.h"
#include "core/GameObject.h"

// The view/camera object stored at GameObject::field_0x18 (set by GameObject slot 8, which
// Setup() calls with its first argument).  Tier 3 stand-in: only the pieces the slots below
// touch are declared.  Slot 7 (+0x1c) is called with (0, 1, 1) and (0, 4, 1) by slot 14 and
// field +0x08 is read as the camera record.
class VisibilityRenderer;

struct VisibilityCamera {
    // The frozen copy g_frozenCamera (0x0068a968) is built and destroyed by the
    // PCCamera constructor/destructor (its initializers 0x0052d2f0/0x0052d310),
    // so the record is a PCCamera (strong inference); the 0x220-byte copy in
    // slot 23 includes its vtable pointer.
    explicit VisibilityCamera(int flags);  // 0x004bed80
    ~VisibilityCamera();                   // 0x004624d0
    char field_0x00[0x18];
    VisibilityRenderer* renderer; // +0x18 loaded by the verified debug walk
    char field_0x1c[0x24];
    char field_0x40[0x74];
    float matrixB[4][4];          // +0xb4 second 4x4 (0x0052fbb0 reads +0xb4/+0xc4/+0xd4/+0xe4 as the columns of x/y/z/translation)
    char field_0xf4[0x38];
    float matrixC[4][4];          // +0x12c third 4x4 (0x0052fbb0)
    float field_0x16c;  // +0x16c compared with a dot product after (field - 10.0f) * 0.00461538f in 0x0052f140
    float worldX;       // +0x170 passed first to VisibilityQuadTree::Query (made tree-relative there)
    float field_0x174;
    float worldZ;       // +0x178 passed second to Query (multiplied by the tree's quantScale)
    char field_0x17c[0x2c];
    unsigned int viewportWidth;   // +0x1a8 converted as an unsigned value (fild qword) in 0x0052f340 / 0x0052f190
    unsigned int viewportHeight;  // +0x1ac
    char field_0x1b0[8];
    float field_0x1b8;  // +0x1b8 multiplied by a point's z and compared with its y in 0x0052fac0 (y slope)
    float field_0x1bc;  // +0x1bc compared with a point's z in 0x0052fac0
    float field_0x1c0;  // +0x1c0 compared with a bounding sphere's near depth in 0x0052fbb0 (far limit)
    char field_0x1c4[0x5c];
};   // 0x220 bytes: slot 23 copies this much (0x88 dwords) into g_frozenCamera

class VisibilityView {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(int a, int b, int c);   // +0x1c
    char field_0x04[4];
    VisibilityCamera* camera;   // +0x08
};

struct VisibilityResultList {
    QuadTreeResultLink* head;
    QuadTreeResultLink* field_0x04;
};

// Accumulator object at 0x0068ab90: 0x004cb6b0 adds a sample, 0x004cb690 reads a value (tier 3).
class VisibilityStat {
public:
    explicit VisibilityStat(int hold);   // 0x004cb670 (5000 for g_visibilityTickStat)
    void AddSample(int value);   // 0x004cb6b0
    int GetValue();              // 0x004cb690
};
// Text log at VisibilityProfiler::log (+0x38) and the profiler object at 0x0056e26c.
struct VisibilityLog {
    char field_0x00[0x26c0];
    int nextLineIndex;           // +0x26c0 post-incremented to hand out a line index (slot 12)
};
struct VisibilityProfiler {
    char field_0x00[0x38];
    VisibilityLog* log;          // +0x38 first argument of the 0x00447fa0 / 0x00447f40 line writers
};
class VisibilityMemoryStats {
public:
    int GetBytes(const char* name);   // 0x004a2d20, called with "QuadTree" (tier 3)
};

struct VisibilityBoxVec;
struct VisibilityMatrix {
    float m[4][4];
};
// Clip-space point of the projection helpers.  Retail reserves 0x20 bytes for it in
// ProjectPoint, ProjectVertices and SphereInFrustum (the last four floats are never used).
struct VisibilityClipPoint {
    float x, y, z, w;
    float field_0x10[4];
};
// 0x10-byte view-space point of CullPolygon: position and its outcode.
struct VisibilityCullPoint {
    float x, y, z;
    unsigned int code;
};
// A projected vertex record (0x20 bytes, position first) as ProjectVertices reads it.
struct VisibilityVertex {
    float x, y, z;
    char field_0x0c[0x14];
};

// Tier 3.  The object at 0x00575a98: every caller loads ecx from that pointer and the
// methods are thiscall (ret N), but none of them reads its fields; CullQuad keeps ecx intact
// to pass it on to CullPolygon.  The methods lie in this unit's code between the second empty
// static (0x0052f080) and the Math3D.h vector initializers (0x0052fdc0), so they are
// VisibilityQuadTree.cpp's (strong inference: the unit's code range).
//  * TestBox 0x0052f570 (ret 0x1c) takes the camera, a matrix (camera + 0xec), a box centre
//    and half extents (3 floats each), an optional screen rect (always 0 here) and returns
//    the number of box corners inside the frustum through its sixth argument.  The result is
//    0 when all 8 corners fail one plane.  It rounds with inline x87 code (excluded).
class VisibilityClipper {
public:
    int TestBox(VisibilityCamera* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int unused);
    void TransformVectors(const VisibilityBoxVec* src, VisibilityBoxVec* dst,
                          const VisibilityMatrix* m, int count);               // 0x0052f0a0
    int TestDot(const VisibilityCamera* camera, const VisibilityBoxVec* a,
                const VisibilityBoxVec* b);                                    // 0x0052f140
    void ProjectVertices(const VisibilityCamera* camera, const VisibilityMatrix* m, int count,
                         const VisibilityVertex* vertices, VisibilityBoxVec* out,
                         int* codes);                                          // 0x0052f190
    int ProjectPoint(const VisibilityCamera* camera, const VisibilityMatrix* m,
                     const VisibilityBoxVec* p, VisibilityBoxVec* screen,
                     unsigned int* outCode);                                   // 0x0052f340
    int CullQuad(const VisibilityCamera* camera, const VisibilityBoxVec* points, int i0,
                 int i1, int i2, int i3);                                      // 0x0052f4d0
    int CullPolygon(const VisibilityCamera* camera, VisibilityCullPoint* points,
                    int count);                                                // 0x0052fac0
    int SphereInFrustum(const VisibilityCamera* camera, const VisibilityMatrix* m,
                        const VisibilityBoxVec* center, float radius,
                        int* fullyInside);                                     // 0x0052fbb0
};
extern VisibilityClipper* g_visibilityClipper;   // 0x00575a98

// Tier 3.  The renderer reached through camera + 0x18 (0x0052e450 loads it with
// mov ecx,[edi+0x18]); DrawBox calls its virtual slot 16 (+0x40) with (3, 0x1e2, vertices,
// 16, 0) and no explicit this, i.e. a thiscall: a 16-vertex line strip (type 3) of 0x20-byte
// vertices whose position is the first three floats.
class VisibilityRenderer {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void DrawLineStrip(int primitiveType, int vertexFormat, void* vertices, int count, int flags);   // +0x40
};

struct VisibilityBoxVec {
    float x, y, z;
};

class VisibilityQuadTreeNode : public QuadTreeNode {
public:
    VisibilityQuadTreeNode();                 // 0x0052d510

    // Tier 3 name.  0x0052d610 (ret 0x1c): recursive frustum walk over the four children,
    // farthest quadrant first.  Arguments: query cell (x, z) in tree units, the tail link of
    // the result list (the new cell is stored through it), node origin, node size, camera.
    // Returns the new tail.  Not virtual: a child that is completely inside the frustum is
    // gathered whole through QuadTreeNode slot 2 instead.
    QuadTreeResultLink* Traverse(int x, int z, QuadTreeResultLink* tail, int nodeX, int nodeZ,
                                 int size, VisibilityCamera* camera);

    __forceinline QuadTreeResultLink* VisitChild(int i, int x, int z, QuadTreeResultLink* tail,
                                                 int nodeX, int nodeZ, int midX, int midZ, int half,
                                                 float centerX, float centerZ, float halfExtent,
                                                 VisibilityCamera* camera,
                                                 VisibilityBoxVec* center, VisibilityBoxVec* extent, int* cornersInside);

    // Tier 3 name.  0x0052e450 (ret 0x18): the same walk as a debug draw (no result list).
    void DebugDraw(int x, int z, int nodeX, int nodeZ, int size, VisibilityCamera* camera);

    __forceinline void DrawChild(int i, int x, int z, int nodeX, int nodeZ, int midX, int midZ,
                                 int half, float centerX, float centerZ, float halfExtent,
                                 VisibilityCamera* camera, VisibilityBoxVec* center,
                                 VisibilityBoxVec* extent, int* cornersInside);

    // Tier 3 name.  0x0052e1f0 (ret 0xc): draws the box (center, half extents) of a node.
    void DrawBox(VisibilityRenderer* renderer, const float* center, const float* extent);
    // No declared destructor: deleting 0x0052d530, core 0x0052d550 (a jmp to 0x004dd0c0).
};

class VisibilityQuadTree : public QuadTree, public GameObject {
public:
    explicit VisibilityQuadTree(int flags);   // 0x0052d340 (ret 4)
    virtual ~VisibilityQuadTree();            // deleting 0x0052d3d0, core 0x0052d3f0
    virtual QuadTreeNode* UnknownVirtualSlot1();   // 0x0052d4b0 node factory

    // 0x0052d460 (ret 0x18).
    GameObject* Setup(int parentArg, float x0, float z0, float x1, float z1, float minCell);

    // Tier 3 name.  Retail 0x0052d560 (ret 0xc): starts a visibility query from the
    // camera ground position (x, z) with the frustum camera record.
    int Query(float x, float z, VisibilityCamera* camera);

    VisibilityView* View() const { return (VisibilityView*)field_0x18; }

    virtual int GameObjectVirtualSlot12();    // 0x0052ef50 (secondary vtable)
    virtual int GameObjectVirtualSlot14();    // 0x0052e3b0
    virtual int GameObjectVirtualSlot15();    // 0x0052f050
    virtual int GameObjectVirtualSlot23(int a, int b);   // 0x0052eec0
};

#endif
