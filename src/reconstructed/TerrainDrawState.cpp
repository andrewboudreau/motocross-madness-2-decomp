// Writable grid/terrain counters and feature state confirmed from retail.
// Data grouping only: original translation-unit ownership is not established.
// Existing scalar declarations are in Griddraw.cpp and broadphase/Terrain.cpp.

int g_terrainDebugPage = -1;        // 0x00574714, file-backed sentinel

// Retail .data virtual tail: loader-zeroed, with ordinary runtime writers.
int g_terrainCacheBytes;           // 0x00689ff4
int g_gridDrawMemoryPeak;          // 0x0068a2fc
int g_gridDrawMemory;              // 0x0068a300
int g_gridDrawFrameCount;          // 0x0068a304
int g_terrainAgeTime0;             // 0x0068a308
int g_terrainAgeTime1;             // 0x0068a30c
int g_terrainCacheCount;           // 0x0068a310
int g_terrainPrepareGeometryTicks; // 0x0068a318
int g_terrainRender3DTicks;        // 0x0068a31c
int g_terrainTotalTicks;           // 0x0068a320
int g_terrainDualTextureProbed;    // 0x0068a328
