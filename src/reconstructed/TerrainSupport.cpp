// Verified support functions from retail Terrain.cpp; remaining Terrain methods
// are in src/krusty2/broadphase/Terrain.cpp. This filename is ours.
#include "../krusty2/broadphase/Terrain.h"

// File-scope statics.  retail's first code in this TU is their dynamic initialisers
// (0x00505490.. : the camera ctor + atexit, then ten 5000-tick PeakHolds in 0x20-byte
// thunk/body pairs).  owner: Terrain.cpp bracket start.
TerrainSharedState g_terrainSharedState(1);   // 0x0068a090 (PCCamera, tier 2)
TerrainPeakHold g_terrainAgeTime0Peak(5000);   // 0x0068a008
TerrainPeakHold g_terrainAgeTime1Peak(5000);   // 0x0068a038
TerrainPeakHold g_terrainSetupPeak(5000);   // 0x0068a2e0
TerrainPeakHold g_terrainEvalPeak(5000);   // 0x0068a2c0
TerrainPeakHold g_terrainMeshPeak(5000);   // 0x0068a048
TerrainPeakHold g_terrainCacheCountPeak(5000);   // 0x0068a2b0
TerrainPeakHold g_terrainCacheBytesPeak(5000);   // 0x0068a018
TerrainPeakHold g_terrainPrepareGeometryPeak(5000);   // 0x0068a2d0
TerrainPeakHold g_terrainRender3DPeak(5000);   // 0x0068a068
TerrainPeakHold g_terrainTotalPeak(5000);   // 0x0068a078

// 0x00505600: takes the next pooled object (ownedObjectArray[ownedObjectArrayCount], parked behind
// the live prefix by RetireOwnedObject) or, when the slot is empty, creates a new surface and
// appends it.  owner: Terrain.cpp (__FILE__ 0x0050567c, 0x0050570b).
BaseObject* Terrain::AcquireOwnedObject()
{
    if (ownedObjectArrayCount > 0x3c) {
        textureAgeManager->age--;
        textureAgeManager->Purge(0);
        textureAgeManager->age++;
    }
    BaseObject* object = ownedObjectArray[ownedObjectArrayCount];
    if (object) {
        ownedObjectArrayCount++;
        return object;
    }
    TerrainSurfaceBase* surface;
    if (surfaceRegistry) {
        TerrainSurface* s = new(__FILE__, 0x67) TerrainSurface(surfaceDesc.manager);
        surface = s;
        surface->CreateSurface(0, 0x100, 0x100, 0x100, 1, 0, 0, colorMapper, 2, directDrawPalette, 0, 0, 2, 1,
                               &surfaceDesc, 0x80, 0xff00ff);
        surfaceRegistry->Register(s);
    } else {
        surface = new(__FILE__, 0x78) TerrainSurfaceBase(surfaceDesc.manager, 1);
        surface->CreateSurface(0, 0x100, 0x100, 0x100, 1, 0, 0, colorMapper, 2, directDrawPalette, 0, 0, 2, 1,
                               &surfaceDesc, 0x80, 0xff00ff);
    }
    createdSurfaceCount++;
    ownedObjectArray[ownedObjectArrayCount] = surface;
    ownedObjectArrayCount++;
    return surface;
}


// 0x00508970 (ret 8): world-space height range of the grid, the heightField's heightMin/heightMax
// (+0x18/+0x1c) scaled by gridCellSize (+0x40).  Either out pointer may be null.
// owner: bracket only (no __FILE__ reference; it follows Terrain slot 23 0x00508850).
void Terrain::GetHeightRange(float* outMin, float* outMax)
{
    if (outMin)
        *outMin = heightField->heightMin * gridCellSize;
    if (outMax)
        *outMax = heightField->heightMax * gridCellSize;
}
