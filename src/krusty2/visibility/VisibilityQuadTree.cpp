// VisibilityQuadTree.cpp -- reconstruction of D:\aardvark\VC\krusty2\VisibilityQuadTree.cpp.
#include "visibility/VisibilityQuadTree.h"

// The unit's file statics, in .CRT$XCU order (entries 340-347): an empty
// static (0x0052d2c0), the frozen camera (0x0052d2e0) and the query timer
// (0x0052d320), then Math3D.h's four vectors (0x0052fdc0..) and a second
// empty static (0x0052f080). The empty statics' addresses never appear in the
// code, so their types are unknown; an empty user constructor reproduces them.
struct UnknownVisibilityStatic {
    UnknownVisibilityStatic() {}
};
static UnknownVisibilityStatic s_visibilityUnknownStatic0;
VisibilityCamera g_frozenCamera(1);                // 0x0068a968
VisibilityStat g_visibilityTickStat(5000);        // 0x0068ab90

#include "math/Math3D.h"

// Installed for CollisionObject broad-phase queries; g_pQuadTree is the
// separate active traversal context at 0x00689b78. Names are provisional.
QuadTree* g_collisionQuadTree;  // 0x0068aba4

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
// g_frozenCamera (0x0068a968): the 0x220-byte copy of the camera record (defined above).
extern int g_visibilityQueryTicks;         // 0x0068ab88 clock delta around Query in slot 12
// g_visibilityTickStat (0x0068ab90): accumulator fed with the query ticks (defined above).

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
    g_collisionQuadTree = this;
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
    g_collisionQuadTree = 0;
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
        g_visibilityTickStat.AddSample(g_visibilityQueryTicks);
        SetLineName(g_profiler->log, field_0x858, "QuadTree");
        LogLine(g_profiler->log, field_0x858, "Memory %d", g_memoryStats->GetBytes("QuadTree"));
        LogLine(g_profiler->log, field_0x858, "PrepareGeometry %d %d", g_visibilityQueryTicks,
                g_visibilityTickStat.GetValue());
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
    DrawBox(camera->renderer, &center->x, &extent->x);
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

// 0x0052f080 / 0x0052f090: the second empty file static (see the top).
static UnknownVisibilityStatic s_visibilityUnknownStatic1;

// ==============================================================================================
// VisibilityClipper (the object at 0x00575a98; see VisibilityQuadTree.h).  None of these
// reads `this`.  Names are tier 3; the arithmetic is decoded.
// ==============================================================================================

// 0x0052f0a0 (ret 0x10).  Rotates `count` vectors by the upper 3x3 of the matrix (row-vector
// convention: x' = x*m[0][0] + y*m[1][0] + z*m[2][0]); the translation row is not applied.
// Accumulating `t += ...` statements keep retail's x, z, y term order; a single sum expression
// is re-ordered by VC6 (the term order of a + b + c does not follow the source).
void VisibilityClipper::TransformVectors(const VisibilityBoxVec* src, VisibilityBoxVec* dst,
                                         const VisibilityMatrix* m, int count)
{
    for (int i = 0; i < count; i++) {
        VisibilityBoxVec v = src[i];
        float t = v.x * m->m[0][0];
        t += v.z * m->m[2][0];
        t += v.y * m->m[1][0];
        dst[i].x = t;
        t = v.x * m->m[0][1];
        t += v.z * m->m[2][1];
        t += v.y * m->m[1][1];
        dst[i].y = t;
        t = v.x * m->m[0][2];
        t += v.z * m->m[2][2];
        t += v.y * m->m[1][2];
        dst[i].z = t;
    }
}

// 0x0052f140 (ret 0xc).  Compares a dot product against a threshold derived from the
// camera record's float at +0x16c (10.0f at 0x00550780 and 0.00461538f at 0x00558e14).
int VisibilityClipper::TestDot(const VisibilityCamera* camera, const VisibilityBoxVec* a,
                               const VisibilityBoxVec* b)
{
    float dot = a->y * b->y;
    dot += a->x * b->x;
    dot += a->z * b->z;
    if (dot > (camera->fieldOfView - 10.0f) * 0.00461538f)
        return 0;
    return 1;
}

// 0x0052f340 (ret 0x14).  Projects a point with the matrix, writes the outcode (x: 1 / 2 for
// x' < 0 / x' > w, y: 4 / 8, z: 0x10 / 0x20) and, when `screen` is given, the coordinates
// scaled by the camera's viewport size (converted as unsigned values).  Returns 1 when the
// point is inside.
int VisibilityClipper::ProjectPoint(const VisibilityCamera* camera, const VisibilityMatrix* m,
                                    const VisibilityBoxVec* p, VisibilityBoxVec* screen,
                                    unsigned int* outCode)
{
    unsigned int code = 0;
    VisibilityClipPoint c;
    c.w = p->z * m->m[2][3];
    c.w += p->y * m->m[1][3];
    c.w += p->x * m->m[0][3];
    c.w += m->m[3][3];
    c.x = p->z * m->m[2][0];
    c.x += p->y * m->m[1][0];
    c.x += p->x * m->m[0][0];
    c.x += m->m[3][0];
    if (c.x < 0.0)
        code = 1;
    else if (c.w - c.x < 0.0)
        code = 2;
    c.y = p->z * m->m[2][1];
    c.y += p->y * m->m[1][1];
    c.y += p->x * m->m[0][1];
    c.y += m->m[3][1];
    if (c.y < 0.0)
        code |= 4;
    else if (c.w - c.y < 0.0)
        code |= 8;
    c.z = p->z * m->m[2][2];
    c.z += p->y * m->m[1][2];
    c.z += p->x * m->m[0][2];
    c.z += m->m[3][2];
    if (c.z < 0.0)
        code |= 0x10;
    else if (c.w - c.z < 0.0)
        code |= 0x20;
    if (screen) {
        float inv = 1.0f / c.w;
        screen->x = camera->viewportWidth * inv * c.x;
        screen->y = camera->viewportHeight * inv * c.y;
        screen->z = camera->viewportHeight * inv * c.z;
    }
    if (outCode)
        *outCode = code;
    return code == 0;
}

// 0x0052f4d0 (ret 0x18).  Gathers four points by index (12-byte stride) into a 4-point
// polygon and culls it with CullPolygon.
int VisibilityClipper::CullQuad(const VisibilityCamera* camera, const VisibilityBoxVec* points,
                                int i0, int i1, int i2, int i3)
{
    VisibilityCullPoint quad[4];
    quad[0].x = points[i0].x;
    quad[0].y = points[i0].y;
    quad[0].z = points[i0].z;
    quad[1].x = points[i1].x;
    quad[1].y = points[i1].y;
    quad[1].z = points[i1].z;
    quad[2].x = points[i2].x;
    quad[2].y = points[i2].y;
    quad[2].z = points[i2].z;
    quad[3].x = points[i3].x;
    quad[3].y = points[i3].y;
    quad[3].z = points[i3].z;
    return CullPolygon(camera, quad, 4);
}

// 0x0052fac0 (ret 0xc).  View-space polygon cull.  Returns 0 when every point has z below the
// camera's +0x1bc, or when the points' outcodes (|y| against z * +0x1b8, |x| against z) share
// a bit, i.e. the polygon is entirely outside one frustum side; the second test is skipped
// when the camera's +0x16c (a field of view in degrees, compared with 90.0) is above 90.
int VisibilityClipper::CullPolygon(const VisibilityCamera* camera, VisibilityCullPoint* points,
                                   int count)
{
    int i;
    int allBehind = 1;
    for (i = 0; i < count; i++) {
        if (points[i].z < camera->nearPlane && allBehind)
            allBehind = 1;
        else
            allBehind = 0;
    }
    if (allBehind)
        return 0;
    if (camera->fieldOfView <= 90.0) {
        for (i = 0; i < count; i++) {
            unsigned int code = 0;
            float slope = points[i].z * camera->aspectRatio;
            if (slope < points[i].y)
                code = 4;
            else if (-slope > points[i].y)
                code = 8;
            if (points[i].x > points[i].z)
                code |= 2;
            else if (-points[i].z > points[i].x)
                code |= 1;
            points[i].code = code;
        }
        unsigned int common = 0xffffffff;
        for (i = 0; i < count; i++)
            common &= points[i].code;
        if (common)
            return 0;
    }
    return 1;
}

// 0x0052fbb0 (ret 0x14).  Sphere against the view frustum.  The camera supplies the depth
// axis (+0xb4/+0xc4/+0xd4/+0xe4, a column of matrixB), the near/far limits (+0x1bc / +0x1c0)
// and the side-plane scale (`side`: matrixC column 0); `m` gives x', y' and w.  Returns 0
// when the sphere is outside; `fullyInside`, when given, receives 1 only if the sphere crosses
// no plane.  `side` is taken before the depth is computed: VC6 homes `depth` and `nearDepth`
// in the dead camera/center argument slots in the order of the arguments' first use, and the
// address of the side-plane column is the camera's first use (docs/VC6_FRAME_LAYOUT.md).
int VisibilityClipper::SphereInFrustum(const VisibilityCamera* camera, const VisibilityMatrix* m,
                                       const VisibilityBoxVec* center, float radius,
                                       int* fullyInside)
{
    int crosses = 0;
    float depth, nearDepth, spread, xHigh, xLow, yHigh, yLow;
    VisibilityClipPoint c;
    const float* side = &camera->matrixC[0][0];
    depth = center->y * camera->matrixB[1][0];
    depth += center->z * camera->matrixB[2][0];
    depth += center->x * camera->matrixB[0][0];
    depth += camera->matrixB[3][0];
    nearDepth = depth - radius;
    if (nearDepth > camera->farPlane || depth + radius < camera->nearPlane) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (depth + radius > camera->farPlane || nearDepth < camera->nearPlane)
        crosses = 1;
    spread = (side[4] + side[0]) * radius;
    spread += depth * side[8];
    spread += side[12];
    c.w = center->y * m->m[1][3];
    c.w += center->z * m->m[2][3];
    c.w += center->x * m->m[0][3];
    c.w += m->m[3][3];
    c.x = center->y * m->m[1][0];
    c.x += center->z * m->m[2][0];
    c.x += center->x * m->m[0][0];
    c.x += m->m[3][0];
    xHigh = c.x + spread;
    if (xHigh < 0.0f || (xLow = c.x - spread) > c.w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    c.y = center->y * m->m[1][1];
    c.y += center->z * m->m[2][1];
    c.y += center->x * m->m[0][1];
    c.y += m->m[3][1];
    yHigh = c.y + spread;
    if (yHigh < 0.0f || (yLow = c.y - spread) > c.w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (!fullyInside)
        return 1;
    if (!crosses && !(xLow < 0.0f) && !(xHigh > c.w) && !(yLow < 0.0f) && !(yHigh > c.w))
        *fullyInside = 1;
    else
        *fullyInside = 0;
    return 1;
}
