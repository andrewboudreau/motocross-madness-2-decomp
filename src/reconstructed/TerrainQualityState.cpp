// Two writable quality tables selected by Terrain::Load. Original TU ownership
// and hardware/software names remain provisional; all 320 initial bytes are
// literal retail .data, not inferred defaults. No runtime initialization.
#include "../krusty2/broadphase/Terrain.h"

TerrainQualityEntry g_terrainQualityHardware[10] = {
    { 7, 4096, 512, 0 },
    { 6, 2048, 512, 0 },
    { 5, 1024, 512, 0 },
    { 5, 1024, 512, 0 },
    { 4, 1024, 512, 0 },
    { 4, 512, 256, 0 },
    { 4, 512, 256, 0 },
    { 3, 512, 256, 0 },
    { 3, 256, 256, 0 },
    { 2, 256, 0, 1 },
};

TerrainQualityEntry g_terrainQualitySoftware[10] = {
    { 7, 4096, 0, 1 },
    { 6, 2048, 0, 1 },
    { 5, 1024, 0, 1 },
    { 4, 1024, 0, 1 },
    { 4, 1024, 0, 1 },
    { 3, 512, 0, 1 },
    { 3, 512, 0, 1 },
    { 2, 512, 0, 1 },
    { 2, 256, 0, 1 },
    { 1, 256, 0, 1 },
};
