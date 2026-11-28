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

// GameObject::renderContext (set by GameObject slot 8, which Setup() calls with its first
// argument) is the RenderTarget (reconstructed/RenderTarget.h): slot 14 calls its slot 7
// SetTextureStageState (+0x1c) with (0, 1, 1) and (0, 4, 1) and reads its +0x08 currentCamera.
// The walks and the clipper take that Camera (reconstructed/Camera.h): fieldOfView (+0x16c),
// eyePosition (+0x170; x and z start the query), the view matrix's z column (+0xb4, +0xc4,
// +0xd4, +0xe4: SphereInFrustum's depth), screenMatrix[0] (+0xec, the box test's matrix) and
// [1] (+0x12c), the viewport size (+0x1a8/+0x1ac, converted as unsigned values),
// heightWidthRatio (+0x1b8) and the near and far planes (+0x1bc/+0x1c0).  The frozen copy
// g_frozenCamera (0x0068a968) is a PCCamera built and destroyed by its constructor and
// implicit destructor (initializers 0x0052d2f0/0x0052d310); slot 23 copies the whole
// 0x220-byte object over it, vtable pointer included (0x88 dwords).
class Camera;
class RenderTarget;

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
                                 int size, Camera* camera);

    __forceinline QuadTreeResultLink* VisitChild(int i, int x, int z, QuadTreeResultLink* tail,
                                                 int nodeX, int nodeZ, int midX, int midZ, int half,
                                                 float centerX, float centerZ, float halfExtent,
                                                 Camera* camera,
                                                 VisibilityBoxVec* center, VisibilityBoxVec* extent, int* cornersInside);

    // Tier 3 name.  0x0052e450 (ret 0x18): the same walk as a debug draw (no result list).
    void DebugDraw(int x, int z, int nodeX, int nodeZ, int size, Camera* camera);

    __forceinline void DrawChild(int i, int x, int z, int nodeX, int nodeZ, int midX, int midZ,
                                 int half, float centerX, float centerZ, float halfExtent,
                                 Camera* camera, VisibilityBoxVec* center,
                                 VisibilityBoxVec* extent, int* cornersInside);

    // Tier 3 name.  0x0052e1f0 (ret 0xc): draws the box (center, half extents) of a node
    // through the camera's render target (camera +0x18, loaded with mov ecx,[edi+0x18] in
    // 0x0052e450): RenderTarget slot 16 (+0x40) DrawPrimitive with (3, 0x1e2, vertices,
    // 16, 0), a 16-vertex line strip of 0x20-byte vertices.
    void DrawBox(RenderTarget* renderer, const float* center, const float* extent);
    // No declared destructor: deleting 0x0052d530, core 0x0052d550 (a jmp to 0x004dd0c0).
};

class VisibilityQuadTree : public QuadTree, public GameObject {
public:
    explicit VisibilityQuadTree(int flags);   // 0x0052d340 (ret 4)
    virtual ~VisibilityQuadTree();            // deleting 0x0052d3d0, core 0x0052d3f0
    virtual QuadTreeNode* CreateNode();   // 0x0052d4b0 node factory

    // 0x0052d460 (ret 0x18).
    GameObject* Setup(int parentArg, float x0, float z0, float x1, float z1, float minCell);

    // Tier 3 name.  Retail 0x0052d560 (ret 0xc): starts a visibility query from the
    // camera ground position (x, z) with the frustum camera record.
    int Query(float x, float z, Camera* camera);

    RenderTarget* View() const { return (RenderTarget*)renderContext; }

    virtual int PrepareGeometry();            // 0x0052ef50 (secondary vtable)
    virtual int Render3D();    // 0x0052e3b0
    virtual int RenderPost3D();               // 0x0052f050
    virtual int OnControlPress(ControlEvent* a, InputButtonState* b);            // 0x0052eec0
};

// The debug box strip DrawBox fills and draws (FVF 0x1e2, the D3DLVERTEX layout); the ctor
// resets every vertex's colour so the lines come out 0x2000ffff.
struct VisibilityBoxVertex {   // 0x20 bytes, 16 of them at 0x0068a768
    float x, y, z;
    int reserved;
    int color;
    int specular;
    float tu, tv;
};
extern VisibilityBoxVertex g_visibilityBoxVertices[16];   // 0x0068a768 (ends at the frozen camera, 0x0068a968)

#endif
