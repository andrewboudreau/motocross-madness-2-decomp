// Group3_TerrainPrepareGeometry.cpp -- bring-up body for Terrain's GameObject slot 12
// (retail 0x00507610, 783 bytes).  NOT a byte-match claim: written from the decoded
// retail instruction flow so the source-built game prepares its terrain; without it the
// base GameObject::PrepareGeometry runs, TestVisibility never marks any grid node and
// nothing is drawn.  Statistics globals without a retail binding are stood in by
// file-local storage.
//
// Flow (tier 1, decoded direct behaviour): skip while the 0x00574718 toggle is 0; push
// the "Terrain" memory tag; trim the texture and mesh AgeManagers (1 MiB / 0.5 MiB
// stale limits); select the visibility camera; clear the per-frame counters; publish the
// viewer position (camera +0x170, divided by gridCellSize) and the focal length (camera
// +0x198, truncated); ComputeRatios; run the height field's visibility, evaluate, mesh
// walk passes with ReadClock timings; pop the tag.
// Tier 3: 0x00481170 is WalkSlot4FromRoot (Griddraw.h) and runs last.

#include "broadphase/Terrain.h"
#include "AgeManager.h"
#include "Griddraw.h"
#include "Grid1.h"

unsigned int ReadClock();   // 0x004bfa80

extern int g_gridDrawFrameCount;        // 0x0068a304
extern int g_gridBoxTests;              // 0x0068a084
extern int g_terrainCacheBytes;         // 0x00689ff4
extern int g_terrainAgeTime0;           // 0x0068a308
extern int g_terrainAgeTime1;           // 0x0068a30c
extern int g_terrainCacheCount;         // 0x0068a310
extern int g_terrainPrepareGeometryTicks;   // 0x0068a318
extern TerrainPeakHold g_terrainCacheBytesPeak;   // 0x0068a018

// Unbound in retail bindings: 0x0066529c, 0x0068a088 (cleared per frame), 0x0068a2f0 /
// 0x0068a2f4 (peak texture count / peak texture byte estimate).
static int s_gridCounter66529c;
static int s_gridCounter68a088;
static int s_gridTextureCountPeak;
static int s_gridTextureEstimatePeak;

int Terrain::PrepareGeometry()
{
    if (!g_terrainToggle718)
        return 1;

    int previousTag = g_MemTagStack->Push("Terrain");
    g_terrainAgeTime1 = 0;
    int start = (int)ReadClock();
    int ageStart = start;
    int stale;

    if (textureAgeManager) {
        int total = textureAgeManager->TotalSize(&stale);
        g_terrainCacheBytes = total;
        g_terrainCacheBytesPeak.Record(total);
        if (stale > 0x100000) {
            int t = (int)ReadClock();
            textureAgeManager->EvictStale(total - stale + 0x100000);
            g_terrainAgeTime1 = (int)ReadClock() - t;
        }
        textureAgeManager->AdvanceAge();
    }
    if (meshAgeManager) {
        int total = meshAgeManager->TotalSize(&stale);
        if (stale > 0x100000)
            meshAgeManager->EvictStale(total - stale + 0x80000);
        meshAgeManager->AdvanceAge();
    }
    int now = (int)ReadClock();
    g_terrainAgeTime0 = now - ageStart;

    char* camera = g_terrainToggle314 ? (char*)&g_terrainSharedState
                                      : *(char**)((char*)renderContext + 8);
    visibilityCamera = (int)camera;

    s_gridCounter66529c = 0;
    g_gridBoxTests = 0;
    s_gridCounter68a088 = 0;
    vertexErrorTests = 0;
    field_0x94 = 0;
    detailUpdates = 0;
    vertexDirectionSets = 0;
    vertexChangePropagations = 0;
    bufferRebuilds = 0;
    verticesPinned = 0;
    verticesReleased = 0;
    vertexTests = 0;

    // Peak-hold the grid texture usage, then reset the use table.
    int dirty = 0;
    if (g_gridTextureCount > s_gridTextureCountPeak)
        s_gridTextureCountPeak = g_gridTextureCount;
    int estimate = 0;
    for (int i = 0; i < g_gridTextureCount; i++)
        estimate += (g_gridTextureSizes[i] * g_gridTextureSizes[i] * 4) / 3;
    if (estimate > s_gridTextureEstimatePeak)
        s_gridTextureEstimatePeak = estimate;
    g_gridTextureCount = 0;

    if (drawDistance != appliedDrawDistance)
        dirty = 1;

    float* viewer = (float*)viewerPosition;
    float* cameraPosition = (float*)(camera + 0x170);
    viewer[0] = cameraPosition[0];
    viewer[1] = cameraPosition[1];
    viewer[2] = cameraPosition[2];
    viewer[0] /= gridCellSize;
    viewer[1] /= gridCellSize;
    viewer[2] /= gridCellSize;
    cameraFocalLength = (int)*(float*)(camera + 0x198);
    appliedDrawDistance = drawDistance;
    ComputeRatios();

    DrawableGridNode* node = reinterpret_cast<DrawableGridNode*>(heightField);
    node->TestVisibility(stream, camera + 0xec);
    frameCount++;
    int visibilityDone = (int)ReadClock();
    profileSetup = visibilityDone - now;

    if (dirty)
        node->PropagateFullMask();
    else
        node->RetestChangedVertices();
    int evalDone = (int)ReadClock();
    profileEval = evalDone - visibilityDone;

    if (field_0x84) {
        node->TestDroppableVertices();      // 0x00481580
        node->CountEdgeDifferences();       // 0x004813e0
    }
    g_gridDrawFrameCount = 0;
    node->WalkVisibleFromRoot();
    int walkDone = (int)ReadClock();
    profileMesh = walkDone - evalDone;

    node->WalkSlot4FromRoot();
    int finish = (int)ReadClock();
    g_terrainCacheCount = finish - walkDone;
    g_terrainPrepareGeometryTicks = finish - start;
    drawDistanceDirty = 0;

    g_MemTagStack->Pop(previousTag);
    return 1;
}
