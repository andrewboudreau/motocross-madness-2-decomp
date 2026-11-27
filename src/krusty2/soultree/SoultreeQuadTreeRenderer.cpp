// SoultreeQuadTreeRenderer.cpp -- SoultreeQuadTreeRenderer : GraphicsTest.
// owner: SoultreeQuadTreeRenderer.cpp (retail __FILE__ 0x005743f4; bracket 0x504490..0x5048cf,
// between SoulTreePhysics.cpp and the next file).  Every function below is either a direct
// referrer of that string (InsertNodesInQuadtree, RemoveNodesFromQuadtree) or a method of the same class laid out
// contiguously with them, so the promotion rule is met.
#include "../../reconstructed/TrackGameGlobal.h"
#include "../../reconstructed/ControlInterface.h"
#include "SoultreeQuadTreeRenderer.h"

#ifdef MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H
#error "SoultreeQuadTreeRenderer.cpp declares its own polymorphic SoultreeObject view"
#endif
#include "collision/CollisionObject.h"   // QuadTreeObject (objectTypeId at +8)

// SoultreeObject as the renderer sees it: RTTI .?AVSoultreeObject@@ has the direct bases
// QuadTreeObject (+0) and GameObject (+12).  Only the fields the renderer reads are named; the
// flat core/SoultreeObject.h lists the same offsets (tier 1).  The class is polymorphic and
// carries the RTTI names so that dynamic_cast compiles to the same __RTDynamicCast call as
// retail and references the type descriptors .?AVSoultreeObject@@ (0x00568950) and
// .?AVD3DIMSoultreeObject@@ (0x00568970).  This TU-local view and the flat physics layout in
// core/SoultreeObject.h are alternative declarations of one retail class: never include both.
// The views stay in this file so SoultreeQuadTreeRenderer.h can share a unit with the
// canonical D3DIMSoulTree.h and GameObjectIterator.h (QuarryStuntEvent.cpp includes it).
class SoultreeObject : public QuadTreeObject, public GameObject {
public:
    char pad_0x38[0x14c - 0x38];
    int hidden;                       // +0x14c tested by slot 12
    char pad_0x150[0x18c - 0x150];
    int subtreeDirty;                 // +0x18c (common/SoultreeObject.h); set to 1 by 0x5046e0
    int drawsItself;                  // +0x190 cleared by InsertNodesInQuadtree, set to 1 by RemoveNodesFromQuadtree (core/SoultreeObject.h). Tier 3
    int inQuadtree;                   // +0x194 set to 1 by InsertNodesInQuadtree; tested by slot 10
    int quadtreeCell;                 // +0x198 quadtree cell code (core/SoultreeObject.h)
    void UpdateQuadtreeCell();                 // thiscall, plain ret
    void RemoveFromQuadtree();                 // thiscall, plain ret: releases the inQuadtree entry
};

// RTTI .?AVD3DIMSoultreeObject@@ : SoultreeObject (plain inheritance).
class D3DIMSoultreeObject : public SoultreeObject {
public:
    char pad_0x19c[0x28c - 0x19c];
    int lodTable;                     // +0x28c D3DIMSoulTree.h's SoultreeLod* at this offset; nonzero gates the work in 0x5046e0. Tier 2
    char pad_0x290[0x2d4 - 0x290];
    int visible;                      // +0x2d4 D3DIMSoulTree.h's name; set to 1 by slot 12 before SelectLod. Tier 2
    void TransformVertexGroups(int lod);  // 0x00440d40, ret 4 (src/reconstructed D3DIMSoulTree.h)
    void SelectLod(int center, int extents);  // 0x00443de0, ret 8: null bounds pick the node's own
};

// Debug allocator iterator over a GameObject tree: heap allocated, 0x94 bytes
// (operator new(0x94, __FILE__, line) at 0x504719 and 0x504829).
#include "../../reconstructed/GameObjectIterator.h"   // gameobj.cpp's iterator (0x94 bytes)

// TrackGame's Game base stores its DebugOverlay pointer at +0x38.
#include "../../reconstructed/DebugOverlay.h"
struct SoultreeStats {
    char pad_0x00[0x38];
    DebugOverlay* log;            // +0x38
    char pad_0x3c[0x2d4 - 0x3c];
    unsigned char debugFlags;         // +0x2d4 bit 2 enables the debug keys (slot 23)
};

#include "core/DebugAlloc.h"
#include "broadphase/Quadtree.h"

#include "../../reconstructed/TypeRegistry.h"   // g_TypeRegistry 0x00575744

extern QuadTree* g_collisionQuadTree;                     // 0x0068aba4
extern SoultreeObject** g_soultreeNodes;     // 0x00689ec4 node array (RegisterNode 0x4fedb0)
extern int g_soultreeNodeCount;                  // 0x00689ec8


// 0x00504490
SoultreeQuadTreeRenderer::SoultreeQuadTreeRenderer(int flags)
    : GraphicsTest(flags)
{
    sceneObjectCount = 0;
    statsId = -1;
    field_0x40 = 0;
    nodeTypeId = g_TypeRegistry->FindTypeId("SoultreeObject");
}

// slot 0: scalar deleting 0x00504500 is compiler generated; this is the core 0x00504520.
SoultreeQuadTreeRenderer::~SoultreeQuadTreeRenderer()
{
    RemoveNodesFromQuadtree();
}

// slot 14, 0x00504570: publishes the node counters on the statistics overlay.
int SoultreeQuadTreeRenderer::Render3D()
{
    DebugOverlay* log = reinterpret_cast<SoultreeStats*>(g_TrackGame)->log;
    if (log) {
        if (statsId < 0) {
            int line = log->nextPage++;
            statsId = line;
        }
        reinterpret_cast<SoultreeStats*>(g_TrackGame)->log->Title(statsId, "SoultreeQuadTreeRenderer");
        reinterpret_cast<SoultreeStats*>(g_TrackGame)->log->Print(statsId, "Total Objects in scene : %i\n", sceneObjectCount);
        reinterpret_cast<SoultreeStats*>(g_TrackGame)->log->Print(statsId, "Objects Visible        : %i\n", visibleCount);
        reinterpret_cast<SoultreeStats*>(g_TrackGame)->log->Print(statsId, "Objects in list        : %i\n", g_soultreeNodeCount);
    }
    return GraphicsTest::Render3D();
}

// slot 12, 0x00504620: flags every quadtree object of the renderer's type that has no
// +0x14c yet and counts them.
int SoultreeQuadTreeRenderer::PrepareGeometry()
{
    g_collisionQuadTree->RestartQuery();
    visibleCount = 0;
    while (QuadTreeObject* obj = g_collisionQuadTree->NextObject()) {
        if (obj->objectTypeId == nodeTypeId) {
            D3DIMSoultreeObject* node = (D3DIMSoultreeObject*)obj;
            if (node->hidden == 0) {
                node->visible = 1;
                node->SelectLod(0, 0);
                visibleCount++;
            }
        }
    }
    return GraphicsTest::PrepareGeometry();
}

// slot 10, 0x00504690.
int SoultreeQuadTreeRenderer::Tick(float dt)
{
    for (int i = 0; i < g_soultreeNodeCount; i++) {
        SoultreeObject* node = g_soultreeNodes[i];
        if (node->subtreeDirty && node->inQuadtree)
            node->UpdateQuadtreeCell();
    }
    return GraphicsTest::Tick(dt);
}

// 0x005046e0: walks every D3DIMSoultreeObject below the tree root and switches it on.
void SoultreeQuadTreeRenderer::InsertNodesInQuadtree()
{
    GameObject* root = this;
    while (root->parent)
        root = root->parent;
    GameObjectIterator* it = new(__FILE__, 0x63) GameObjectIterator(root, 1, "");
    while (GameObject* obj = it->Next()) {
        D3DIMSoultreeObject* node = dynamic_cast<D3DIMSoultreeObject*>(obj);
        if (node && node->lodTable) {
            sceneObjectCount++;
            node->drawsItself = 0;
            node->subtreeDirty = 1;
            node->inQuadtree = 1;
            node->UpdateQuadtreeCell();
            node->TransformVertexGroups(-1);
        }
    }
    delete it;
}

// 0x005047f0: the inverse walk; resets sceneObjectCount.
void SoultreeQuadTreeRenderer::RemoveNodesFromQuadtree()
{
    GameObject* root = this;
    while (root->parent)
        root = root->parent;
    GameObjectIterator* it = new(__FILE__, 0x7f) GameObjectIterator(root, 1, "");
    while (GameObject* obj = it->Next()) {
        SoultreeObject* node = dynamic_cast<SoultreeObject*>(obj);
        if (node) {
            node->RemoveFromQuadtree();
            node->drawsItself = 1;
        }
    }
    sceneObjectCount = 0;
    delete it;
}

// slot 23, 0x005048d0: with the debug keys enabled, key 0x2d switches every
// node on (InsertNodesInQuadtree) or, when some are on, off again (RemoveNodesFromQuadtree).
int SoultreeQuadTreeRenderer::OnControlPress(ControlEvent* event, InputButtonState* entry)
{
    if ((reinterpret_cast<SoultreeStats*>(g_TrackGame)->debugFlags & 4) && TestInputEvent(0x2d, 0, event, 0x80)) {
        if (!sceneObjectCount)
            InsertNodesInQuadtree();
        else
            RemoveNodesFromQuadtree();
    }
    return GraphicsTest::OnControlPress(event, entry);
}

// slot 8, 0x004452e0 (shared body): forwards to the base.
GameObject* SoultreeQuadTreeRenderer::SetRenderContext(void* context)
{
    return GameObject::SetRenderContext(context);
}
