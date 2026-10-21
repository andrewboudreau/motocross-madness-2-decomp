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

// The view/camera object stored at GameObject::renderContext (set by GameObject slot 8, which
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
    float fieldOfView;  // +0x16c degrees (Camera.h: 77.0f; tested <= 90 in 0x0052fac0); compared with a dot product after (field - 10.0f) * 0.00461538f in 0x0052f140
    float worldX;       // +0x170 passed first to VisibilityQuadTree::Query (made tree-relative there)
    float worldY;       // +0x174 Camera eyePosition.y (eyePosition is +0x170..+0x178); not read here
    float worldZ;       // +0x178 passed second to Query (multiplied by the tree's quantScale)
    char field_0x17c[0x2c];
    unsigned int viewportWidth;   // +0x1a8 converted as an unsigned value (fild qword) in 0x0052f340 / 0x0052f190
    unsigned int viewportHeight;  // +0x1ac
    char field_0x1b0[8];
    float aspectRatio;  // +0x1b8 Camera.h: height/width ratio; multiplied by a point's z and compared with its y in 0x0052fac0 (y slope)
    float nearPlane;    // +0x1bc Camera.h: near clip (1.0f); compared with a point's z in 0x0052fac0
    float farPlane;     // +0x1c0 Camera.h: far clip (100000.0f); compared with a bounding sphere's near depth in 0x0052fbb0 (far limit)
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
    // +0x1c: RenderTarget slot 7; the debug draw (0x0052e3b0) passes (0, D3DTSS_COLOROP/ALPHAOP, D3DTOP_DISABLE).
    virtual void SetTextureStageState(int stage, int type, int value);
    char field_0x04[4];
    VisibilityCamera* camera;   // +0x08
};

struct VisibilityResultList {
    QuadTreeResultLink* head;
    QuadTreeResultLink* field_0x04;
};

// The query timer uses the same 12-byte peak holder as Game's profile page.
#include "../../reconstructed/PeakHold.h"
#include "../../reconstructed/DebugOverlay.h"

struct VisibilityProfiler {
    char field_0x00[0x38];
    DebugOverlay* log;          // +0x38 first argument of the 0x00447fa0 / 0x00447f40 line writers
};
#include "../core/MemTag.h"

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

#include "VisibilityClipper.h"
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

    VisibilityView* View() const { return (VisibilityView*)renderContext; }

    virtual int PrepareGeometry();            // 0x0052ef50 (secondary vtable)
    virtual int UnknownVirtualSlot14();    // 0x0052e3b0
    virtual int RenderPost3D();               // 0x0052f050
    virtual int OnControlPress(UnknownControlEvent* a, UnknownInputEntry* b);            // 0x0052eec0
};

#endif
