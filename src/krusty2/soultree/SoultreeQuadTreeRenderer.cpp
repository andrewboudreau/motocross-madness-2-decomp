// SoultreeQuadTreeRenderer.cpp -- SoultreeQuadTreeRenderer : GraphicsTest.
// owner: SoultreeQuadTreeRenderer.cpp (retail __FILE__ 0x005743f4; bracket 0x504490..0x5048cf,
// between SoulTreePhysics.cpp and the next file).  Every function below is either a direct
// referrer of that string (InsertNodesInQuadtree, RemoveNodesFromQuadtree) or a method of the same class laid out
// contiguously with them, so the promotion rule is met.
#include "SoultreeQuadTreeRenderer.h"
#include "core/DebugAlloc.h"
#include "broadphase/Quadtree.h"

// Type-id registry (0x00575744), same declaration as collision/CollisionObject.cpp.
class TypeRegistry {
public:
    char FindTypeId(const char* name);           // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;             // 0x00575744

extern SoultreeStats* g_soultreeStats;           // 0x0056e26c
extern QuadTree* g_quadTree;                     // 0x0068aba4
extern SoultreeObject** g_soultreeNodes;     // 0x00689ec4 node array (RegisterNode 0x4fedb0)
extern int g_soultreeNodeCount;                  // 0x00689ec8

int CheckKey(int key, int a, int b, int flags);  // 0x0043caa0 (cdecl)

// 0x00504490
SoultreeQuadTreeRenderer::SoultreeQuadTreeRenderer(int flags)
    : GraphicsTest(flags)
{
    field_0x34 = 0;
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
int SoultreeQuadTreeRenderer::GameObjectVirtualSlot14()
{
    SoultreeStatsLog* log = g_soultreeStats->log;
    if (log) {
        if (statsId < 0) {
            int line = log->nextLineIndex++;
            statsId = line;
        }
        SoultreeSetLineName(g_soultreeStats->log, statsId, "SoultreeQuadTreeRenderer");
        SoultreeLogLine(g_soultreeStats->log, statsId, "Total Objects in scene : %i\n", field_0x34);
        SoultreeLogLine(g_soultreeStats->log, statsId, "Objects Visible        : %i\n", visibleCount);
        SoultreeLogLine(g_soultreeStats->log, statsId, "Objects in list        : %i\n", g_soultreeNodeCount);
    }
    return GraphicsTest::GameObjectVirtualSlot14();
}

// slot 12, 0x00504620: flags every quadtree object of the renderer's type that has no
// +0x14c yet and counts them.
int SoultreeQuadTreeRenderer::GameObjectVirtualSlot12()
{
    g_quadTree->RestartQuery();
    visibleCount = 0;
    while (QuadTreeObject* obj = g_quadTree->NextObject()) {
        if (obj->objectTypeId == nodeTypeId) {
            D3DIMSoultreeObject* node = (D3DIMSoultreeObject*)obj;
            if (node->field_0x14c == 0) {
                node->field_0x2d4 = 1;
                node->Fn_443de0(0, 0);
                visibleCount++;
            }
        }
    }
    return GraphicsTest::GameObjectVirtualSlot12();
}

// slot 10, 0x00504690.
int SoultreeQuadTreeRenderer::GameObjectVirtualSlot10(float dt)
{
    for (int i = 0; i < g_soultreeNodeCount; i++) {
        SoultreeObject* node = g_soultreeNodes[i];
        if (node->subtreeDirty && node->inQuadtree)
            node->UpdateQuadtreeCell();
    }
    return GraphicsTest::GameObjectVirtualSlot10(dt);
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
        if (node && node->field_0x28c) {
            field_0x34++;
            node->field_0x190 = 0;
            node->subtreeDirty = 1;
            node->inQuadtree = 1;
            node->UpdateQuadtreeCell();
            node->SelectLod(-1);
        }
    }
    delete it;
}

// 0x005047f0: the inverse walk; resets the +0x34 count.
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
            node->field_0x190 = 1;
        }
    }
    field_0x34 = 0;
    delete it;
}

// slot 23, 0x005048d0: with the debug keys enabled, key 0x2d switches every
// node on (InsertNodesInQuadtree) or, when some are on, off again (RemoveNodesFromQuadtree).
int SoultreeQuadTreeRenderer::GameObjectVirtualSlot23(int a, int b)
{
    if ((g_soultreeStats->debugFlags & 4) && CheckKey(0x2d, 0, a, 0x80)) {
        if (!field_0x34)
            InsertNodesInQuadtree();
        else
            RemoveNodesFromQuadtree();
    }
    return GraphicsTest::GameObjectVirtualSlot23(a, b);
}
