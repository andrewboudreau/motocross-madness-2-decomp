// Bring-up glue (see Group3_QuarryLoadLinks.h): the krusty2 particle,
// terrain-shadow, terrain and quadtree-renderer calls made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "effects/ParticleManager.h"
#include "shadow/ProjectedShadow.h"
#include "broadphase/Terrain.h"
#include "soultree/SoultreeQuadTreeRenderer.h"
#include "Group3_QuarryLoadLinks.h"

// TerrainShadow (vtable 0x005582d4, 0x6780 bytes). A local view: these are the names
// src/reconstructed/TerrainShadow.h gives the constructor and Attach, so the exact
// src/reconstructed/TerrainShadow.cpp resolves them. The storage only fixes the size.
class TerrainShadow {
public:
    explicit TerrainShadow(int flags);                                       // 0x00508ae0 (ret 4)
    TerrainShadow* Attach(int target, Terrain* terrain, ProjectedShadow* shadow); // 0x00508b80
private:
    char storage[0x6780];
};

void* G3Q_TerrainShadowAttach(void* terrainShadow, void* target, void* terrain, void* shadow)
{
    return ((TerrainShadow*)terrainShadow)->Attach((int)target, (Terrain*)terrain, (ProjectedShadow*)shadow);
}

void G3Q_QuadTreeRendererInsertNodes(void* renderer)
{
    ((SoultreeQuadTreeRenderer*)renderer)->InsertNodesInQuadtree();
}
