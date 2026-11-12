// Bring-up glue (see Group3_QuarryLoadLinks.h): the krusty2 quadtree-renderer call made
// by BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "effects/ParticleManager.h"
#include "shadow/ProjectedShadow.h"
#include "broadphase/Terrain.h"
#include "soultree/SoultreeQuadTreeRenderer.h"
#include "Group3_QuarryLoadLinks.h"

void G3Q_QuadTreeRendererInsertNodes(void* renderer)
{
    ((SoultreeQuadTreeRenderer*)renderer)->InsertNodesInQuadtree();
}
