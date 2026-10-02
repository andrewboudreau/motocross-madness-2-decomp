// VisibilityQuadTree.cpp -- reconstruction of D:\aardvark\VC\krusty2\VisibilityQuadTree.cpp.
#include "visibility/VisibilityQuadTree.h"

struct VisibilityGlobalEntry {   // 0x20-byte entries at 0x0068a774, reset by the ctor
    int field_0x00;
    int field_0x04;
    int field_0x08;
    char field_0x0c[0x14];
};
extern VisibilityGlobalEntry g_visibilityEntries[16];

// Debug toggles and the frozen camera snapshot (slot 23).  Names tier 3.
extern int g_visibilityDebugDraw;          // 0x0068ab9c toggled by key 0x44 in slot 23; gates slot 14
extern int g_visibilityFreeze;             // 0x0068aba0 toggled by key 0x57 in slot 23; selects g_frozenCamera in slot 12
extern VisibilityCamera g_frozenCamera;    // 0x0068a968 0x220-byte copy of the camera record
extern int g_visibilityQueryTicks;         // 0x0068ab88 clock delta around Query in slot 12
extern VisibilityStat g_visibilityQueryStat; // 0x0068ab90 accumulator fed with the query ticks

int ReadClock();                                        // 0x004bfa80
int CheckKey(int key, int a, int b, int flags);         // 0x0043caa0 (cdecl, 4 args)
void SetLineName(VisibilityLog* log, int line, const char* name);                    // 0x00447fa0
void LogLine(VisibilityLog* log, int line, const char* format, ...);                 // 0x00447f40
extern VisibilityProfiler* g_profiler;     // 0x0056e26c
extern VisibilityMemoryStats* g_memoryStats; // 0x0056df04

VisibilityQuadTreeNode::VisibilityQuadTreeNode()
{
}

QuadTreeNode* VisibilityQuadTree::UnknownVirtualSlot1()
{
    return new(__FILE__, 0x4f) VisibilityQuadTreeNode;
}

VisibilityQuadTree::VisibilityQuadTree(int flags)
    : GameObject(flags)
{
    g_pQuadTree = this;
    liveNodeCount = 0;
    rootNode = 0;
    for (int i = 0; i < 16; i++) {
        g_visibilityEntries[i].field_0x04 = 0x2000ffff;
        g_visibilityEntries[i].field_0x08 = 0;
        g_visibilityEntries[i].field_0x00 = 0;
    }
}

VisibilityQuadTree::~VisibilityQuadTree()
{
    g_pQuadTree = 0;
}

GameObject* VisibilityQuadTree::Setup(int parentArg, float x0, float z0, float x1, float z1, float minCell)
{
    GameObject::GameObjectVirtualSlot8(parentArg);
    Reset();
    Init(x0, z0, x1, z1, minCell);
    return this;
}

int VisibilityQuadTree::Query(float x, float z, VisibilityCamera* camera)
{
    if (stateFlags & 1)
        return 0;
    queryStamp++;
    g_pQuadTree = this;
    resultArray = 0;
    stateFlags |= 1;
    VisibilityResultList list;
    list.head = 0;
    if (rootNode) {
        ((VisibilityQuadTreeNode*)rootNode)->Traverse((int)((x - worldMinX - worldMinX) * quantScale),
                                                      (int)(z * quantScale), (QuadTreeResultLink*)&list, 0, 0, 0x4000, camera);
    }
    queryListHead = queryCursor = list.head;
    sortedCount = 0;
    itemCursor = 0;
    stateFlags |= 1;
    return 1;
}

int VisibilityQuadTree::GameObjectVirtualSlot15()
{
    EndQuery();
    return 1;
}

int VisibilityQuadTree::GameObjectVirtualSlot23(int a, int b)
{
    if (CheckKey(0x44, 0, a, 0x80)) {
        g_visibilityDebugDraw = 1 - g_visibilityDebugDraw;
        return 1;
    }
    if (CheckKey(0x57, 0, a, 0x80)) {
        g_visibilityFreeze = 1 - g_visibilityFreeze;
        if (g_visibilityFreeze)
            g_frozenCamera = *View()->camera;
        return 1;
    }
    return 0;
}

int VisibilityQuadTree::GameObjectVirtualSlot12()
{
    int start = ReadClock();
    VisibilityCamera* cam;
    if (g_visibilityFreeze)
        cam = &g_frozenCamera;
    else
        cam = View()->camera;
    Query(cam->worldX, cam->worldZ, cam);
    g_visibilityQueryTicks = ReadClock() - start;
    if (g_profiler->log) {
        if (field_0x858 < 0) {
            VisibilityLog* log = g_profiler->log;
            int line = log->nextLineIndex++;
            field_0x858 = line;
        }
        g_visibilityQueryStat.AddSample(g_visibilityQueryTicks);
        SetLineName(g_profiler->log, field_0x858, "QuadTree");
        LogLine(g_profiler->log, field_0x858, "Memory %d", g_memoryStats->GetBytes("QuadTree"));
        LogLine(g_profiler->log, field_0x858, "PrepareGeometry %d %d", g_visibilityQueryTicks,
                g_visibilityQueryStat.GetValue());
    }
    return 1;
}

int VisibilityQuadTree::GameObjectVirtualSlot14()
{
    if (!g_visibilityDebugDraw)
        return 1;
    VisibilityView* view = View();
    VisibilityCamera* cam = view->camera;
    int x = (int)((cam->worldX - worldMinX - worldMinX) * quantScale);
    int z = (int)(cam->worldZ * quantScale);
    View()->UnknownVirtualSlot7(0, 1, 1);
    View()->UnknownVirtualSlot7(0, 4, 1);
    if (rootNode)
        ((VisibilityQuadTreeNode*)rootNode)->DebugDraw(x, z, 0, 0, 0x4000, View()->camera);
    return 1;
}

// One child of the walk: skipped when absent or outside the frustum, gathered whole when all 8
// box corners are inside and recursed into otherwise.
__forceinline QuadTreeResultLink* VisibilityQuadTreeNode::VisitChild(
    int i, int x, int z, QuadTreeResultLink* tail, int nodeX, int nodeZ, int midX, int midZ,
    int half, float centerX, float centerZ, float halfExtent, VisibilityCamera* camera,
    VisibilityBoxVec* center, VisibilityBoxVec* extent, int* cornersInside)
{
    if (!children[i])
        return tail;
    center->y = children[i]->yCenter;
    extent->y = children[i]->yHalfExtent;
    center->x = (i & 2) ? centerX + halfExtent : centerX - halfExtent;
    center->z = (i & 1) ? centerZ + halfExtent : centerZ - halfExtent;
    if (!g_visibilityClipper->TestBox(camera, (const float*)((char*)camera + 0xec), &center->x,
                                      &extent->x, 0, cornersInside, 0))
        return tail;
    int childX = (i & 2) ? midX : nodeX;
    int childZ = (i & 1) ? midZ : nodeZ;
    if (*cornersInside == 8)
        return children[i]->UnknownVirtualSlot2(x, z, tail, childX, childZ, half);
    return ((VisibilityQuadTreeNode*)children[i])->Traverse(x, z, tail, childX, childZ, half, camera);
}

// Back-to-front walk: the quadrant containing the query point is visited last.
QuadTreeResultLink* VisibilityQuadTreeNode::Traverse(int x, int z, QuadTreeResultLink* tail,
                                                     int nodeX, int nodeZ, int size,
                                                     VisibilityCamera* camera)
{
    int midX = nodeX + size;
    int midZ = nodeZ + size;
    float centerX = (float)midX * g_quadTreeInvScale;
    float centerZ = (float)midZ * g_quadTreeInvScale;
    size >>= 1;
    float halfExtent = (float)size * g_quadTreeInvScale;
    VisibilityBoxVec center;
    VisibilityBoxVec extent;
    int cornersInside;
    extent.x = halfExtent;
    extent.z = halfExtent;
    if (children) {
        if (x >= midX) {
            if (z >= midZ) {
                tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            } else {
                tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            }
        } else if (z >= midZ) {
            tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        } else {
            tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        }
    }
    if (itemList) {
        QuadTreeResultLink* cell = (QuadTreeResultLink*)g_pQuadTree->resultPool->Alloc();
        if (tail)
            tail->next = cell;
        cell->item = this;
        cell->next = 0;
        return cell;
    }
    return tail;
}

struct VisibilityBoxVertex {   // 0x20 bytes, 16 of them at 0x0068a768
    float x, y, z;
    char field_0x0c[0x14];
};
extern VisibilityBoxVertex g_visibilityBoxVertices[16];   // 0x0068a768 (ends at the frozen camera, 0x0068a968)

// Builds the 16-vertex line strip that traces the box (bottom square, one vertical edge, top
// square, the remaining vertical edges) and draws it.
void VisibilityQuadTreeNode::DrawBox(VisibilityRenderer* renderer, const float* center, const float* extent)
{
    VisibilityBoxVertex* v = g_visibilityBoxVertices;
    float x0 = center[0] - extent[0];
    float x1 = center[0] + extent[0];
    float y0 = center[1] - extent[1];
    float y1 = extent[1] + center[1];
    float z0 = center[2] - extent[2];
    float z1 = extent[2] + center[2];
    v[0].x = x0;  v[0].y = y0;  v[0].z = z0;
    v[1].x = x1;  v[1].y = y0;  v[1].z = z0;
    v[2].x = x1;  v[2].y = y1;  v[2].z = z0;
    v[3].x = x0;  v[3].y = y1;  v[3].z = z0;
    v[4].x = x0;  v[4].y = y0;  v[4].z = z0;
    v[5].x = x0;  v[5].y = y0;  v[5].z = z1;
    v[6].x = x1;  v[6].y = y0;  v[6].z = z1;
    v[7].x = x1;  v[7].y = y1;  v[7].z = z1;
    v[8].x = x0;  v[8].y = y1;  v[8].z = z1;
    v[9].x = x0;  v[9].y = y0;  v[9].z = z1;
    v[10].x = x1; v[10].y = y0; v[10].z = z1;
    v[11].x = x1; v[11].y = y0; v[11].z = z0;
    v[12].x = x1; v[12].y = y1; v[12].z = z0;
    v[13].x = x1; v[13].y = y1; v[13].z = z1;
    v[14].x = x0; v[14].y = y1; v[14].z = z1;
    v[15].x = x0; v[15].y = y1; v[15].z = z0;
    renderer->DrawLineStrip(3, 0x1e2, v, 16, 0);
}

// One child of the debug walk: draws the child's box when it is at least partly visible and
// recurses unless it is completely inside.
__forceinline void VisibilityQuadTreeNode::DrawChild(
    int i, int x, int z, int nodeX, int nodeZ, int midX, int midZ, int half, float centerX,
    float centerZ, float halfExtent, VisibilityCamera* camera, VisibilityBoxVec* center, VisibilityBoxVec* extent, int* cornersInside)
{
    if (!children[i])
        return;
    center->y = children[i]->yCenter;
    extent->y = children[i]->yHalfExtent;
    center->x = (i & 2) ? centerX + halfExtent : centerX - halfExtent;
    center->z = (i & 1) ? centerZ + halfExtent : centerZ - halfExtent;
    if (!g_visibilityClipper->TestBox(camera, (const float*)((char*)camera + 0xec), &center->x,
                                      &extent->x, 0, cornersInside, 0))
        return;
    DrawBox(*(VisibilityRenderer**)((char*)camera + 0x18), &center->x, &extent->x);
    if (*cornersInside != 8)
        ((VisibilityQuadTreeNode*)children[i])->DebugDraw(x, z, (i & 2) ? midX : nodeX,
                                                          (i & 1) ? midZ : nodeZ, half, camera);
}

// Same back-to-front walk as Traverse, drawing the box of every visible node.
void VisibilityQuadTreeNode::DebugDraw(int x, int z, int nodeX, int nodeZ, int size,
                                       VisibilityCamera* camera)
{
    int midX = nodeX + size;
    int midZ = nodeZ + size;
    float centerX = (float)midX * g_quadTreeInvScale;
    float centerZ = (float)midZ * g_quadTreeInvScale;
    size >>= 1;
    float halfExtent = (float)size * g_quadTreeInvScale;
    VisibilityBoxVec center;
    VisibilityBoxVec extent;
    int cornersInside;
    extent.x = halfExtent;
    extent.z = halfExtent;
    if (!children)
        return;
    if (x >= midX) {
        if (z >= midZ) {
            DrawChild(0, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(1, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(2, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(3, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        } else {
            DrawChild(1, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(0, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(3, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            DrawChild(2, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        }
    } else if (z >= midZ) {
        DrawChild(2, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(3, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(0, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(1, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
    } else {
        DrawChild(3, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(2, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(1, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        DrawChild(0, x, z, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
    }
}
