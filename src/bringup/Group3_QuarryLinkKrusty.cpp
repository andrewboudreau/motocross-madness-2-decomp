// Bring-up glue (see Group3_QuarryLoadLinks.h): the krusty2 particle, visibility, shadow,
// terrain, quadtree-renderer and collision calls made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "effects/ParticleManager.h"
#include "visibility/VisibilityQuadTree.h"
#include "shadow/ProjectedShadow.h"
#include "broadphase/Terrain.h"
#include "soultree/SoultreeQuadTreeRenderer.h"
#include "collision/CollisionObject.h"
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

// Retail pushes six arguments (target, 0, 0, width, breadth, 96.0f) and receives the
// GameObject part (+0x874), or 0.
void* G3Q_VisibilitySetup(void* tree, void* target, float x0, float z0, float x1, float z1,
                          float minCell)
{
    return ((VisibilityQuadTree*)tree)->Setup((int)target, x0, z0, x1, z1, minCell);
}

void* G3Q_TerrainShadowAttach(void* terrainShadow, void* target, void* terrain, void* shadow)
{
    return ((TerrainShadow*)terrainShadow)->Attach((int)target, (Terrain*)terrain, (ProjectedShadow*)shadow);
}

void* G3Q_TerrainLoad(void* terrain, void* target, void* stream, int unused, void* textures,
                      int quality, int field0xc40, const char* detailTexture, int halfFormat)
{
    return ((Terrain*)terrain)->Load((int)target, (TerrainStream*)stream, unused,
                                     (TerrainSurfaceDesc*)textures, quality, field0xc40,
                                     detailTexture, halfFormat);
}

void G3Q_QuadTreeRendererInsertNodes(void* renderer)
{
    ((SoultreeQuadTreeRenderer*)renderer)->InsertNodesInQuadtree();
}

void G3Q_CollisionSnapPlacement(void* collision)
{
    ((CollisionObject*)collision)->SnapPlacement();
}

void G3Q_CollisionSetUseBroadphase(void* collision, int enable)
{
    ((CollisionObject*)collision)->SetUseBroadphase(enable);
}

void G3Q_CollisionSetIgnoreListMode(void* collision, int mode)
{
    ((CollisionObject*)collision)->SetIgnoreListMode(mode);
}

void G3Q_CollisionAddIgnoredOwner(void* collision, void* owner)
{
    ((CollisionObject*)collision)->AddIgnoredOwner(owner);
}

void G3Q_CollisionSetOwner(void* collision, void* owner, int tag)
{
    ((CollisionObject*)collision)->SetOwner(owner, tag);
}
