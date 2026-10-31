// Group3_TerrainPrepareGeometry.cpp -- bring-up body for Terrain's GameObject slot 12
// (retail 0x00507610, 783 bytes).  Written from the decoded retail instruction flow so
// the source-built game prepares its terrain; without it the base
// GameObject::PrepareGeometry runs, TestVisibility never marks any grid node and nothing
// is drawn.  Statistics globals without a retail binding are stood in by file-local
// storage.
//
// Flow (tier 1, decoded direct behaviour): skip while the 0x00574718 toggle is 0; push
// the "Terrain" memory tag; trim the texture and mesh AgeManagers (1 MiB / 0.5 MiB
// stale limits); select the visibility camera; clear the per-frame counters; publish the
// viewer position (camera +0x170, divided by gridCellSize) and the focal length (camera
// +0x198, converted with FISTP under the current rounding mode); ComputeRatios; run the
// height field's visibility, evaluate, mesh walk passes with ReadClock timings; pop the
// tag.  Tier 3: 0x00481170 is WalkSlot4FromRoot (Griddraw.h) and runs last.
//
// Matching status (VC6 SP3, vc6_o2_mt plus /Oy- /QIfist, bindings for every call and
// global): 214 of retail's 219 instructions align; the five that differ are retail's
// float-to-int block `lea eax,[esi+0x70]; mov [ebp-8],eax; fld; fstp [ebp-0x10]; fld
// [ebp-0x10]; mov eax,[ebp-8]; fistp dword ptr [eax]`, i.e. an inlined `__asm`
// helper whose operands are forced to the frame.  Inline assembly is out of bounds,
// so FloatToInt below is an ordinary conversion: with /QIfist it is `fistp qword` plus a
// dword move (8 bytes shorter, one frame slot lower for the shared t/stale/dirty slot),
// without it `__ftol` (truncation).  The `push ebp; mov ebp,esp` frame is the asm
// helper's doing as well: every other Terrain.cpp body matches without /Oy-.
// The rest of the body is exact: start/last clock locals (`last` first), block-scoped
// stale/t locals, dirty flag before the texture peak-hold, member re-reads of
// visibilityCamera and heightField at each use, the 12-byte viewer copy as a struct
// assignment, and y/z divided through the member address rather than a pointer local.

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

// View of the visibility camera (a Camera, src/reconstructed/Camera.h: screenMatrix[0]
// at +0xec, eyePosition at +0x170, focalLength at +0x198).  Terrain.h keeps the member
// as an int, so the view stays local to this body.
struct TerrainVisibilityCamera {
    char pad_0x00[0xec];
    char screenMatrix[0x170 - 0xec];
    TerrainVec3 eyePosition;
    char pad_0x17c[0x198 - 0x17c];
    float focalLength;
};

// The height field's root is a DrawableGridNode (Griddraw.h); retail re-reads the member
// at every call, so this is a macro rather than a cached local.
#define GridRoot() (reinterpret_cast<DrawableGridNode*>(heightField))

// Stand-in for retail's inline-asm FLD/FISTP helper (see the header comment).  The
// pointer copy keeps the surrounding store order (focal length before the draw-distance
// copy) that retail has.
static inline void FloatToInt(int* destination, float value)
{
    int* p = destination;
    *p = (int)value;
}

int Terrain::PrepareGeometry()
{
    if (!g_terrainToggle718)
        return 1;

    int previousTag = g_MemTagStack->Push("Terrain");
    g_terrainAgeTime1 = 0;
    unsigned int last = ReadClock();
    unsigned int start = last;

    if (textureAgeManager) {
        int stale;
        int total = textureAgeManager->TotalSize(&stale);
        g_terrainCacheBytes = total;
        g_terrainCacheBytesPeak.Record(total);
        if (stale > 0x100000) {
            unsigned int t = ReadClock();
            textureAgeManager->EvictStale(total - stale + 0x100000);
            g_terrainAgeTime1 = ReadClock() - t;
        }
        textureAgeManager->AdvanceAge();
    }
    if (meshAgeManager) {
        int stale;
        int total = meshAgeManager->TotalSize(&stale);
        if (stale > 0x100000)
            meshAgeManager->EvictStale(total - stale + 0x80000);
        meshAgeManager->AdvanceAge();
    }
    unsigned int now = ReadClock();
    g_terrainAgeTime0 = now - last;
    last = now;

    visibilityCamera = g_terrainToggle314 ? (int)&g_terrainSharedState
                                          : (int)((TerrainHost*)renderContext)->sharedState;

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
        estimate += g_gridTextureSizes[i] * g_gridTextureSizes[i] * 4 / 3;
    if (estimate > s_gridTextureEstimatePeak)
        s_gridTextureEstimatePeak = estimate;
    g_gridTextureCount = 0;

    if (drawDistance != appliedDrawDistance)
        dirty = 1;

    TerrainVisibilityCamera* camera = (TerrainVisibilityCamera*)visibilityCamera;
    *(TerrainVec3*)viewerPosition = camera->eyePosition;
    ((TerrainVec3*)viewerPosition)->x /= gridCellSize;
    ((TerrainVec3*)viewerPosition)->y /= gridCellSize;
    ((TerrainVec3*)viewerPosition)->z /= gridCellSize;
    FloatToInt(&cameraFocalLength, camera->focalLength);
    appliedDrawDistance = drawDistance;
    ComputeRatios();

    GridRoot()->TestVisibility(stream, ((TerrainVisibilityCamera*)visibilityCamera)->screenMatrix);
    frameCount++;
    now = ReadClock();
    profileSetup = now - last;
    last = now;

    if (dirty)
        GridRoot()->PropagateFullMask();
    else
        GridRoot()->RetestChangedVertices();
    now = ReadClock();
    profileEval = now - last;
    last = now;

    if (field_0x84) {
        GridRoot()->TestDroppableVertices();      // 0x00481580
        GridRoot()->CountEdgeDifferences();       // 0x004813e0
    }
    g_gridDrawFrameCount = 0;
    GridRoot()->WalkVisibleFromRoot();
    now = ReadClock();
    profileMesh = now - last;
    last = now;

    GridRoot()->WalkSlot4FromRoot();
    now = ReadClock();
    g_terrainCacheCount = now - last;
    g_terrainPrepareGeometryTicks = now - start;
    drawDistanceDirty = 0;

    g_MemTagStack->Pop(previousTag);
    return 1;
}
