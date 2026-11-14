// Group1_GridVertexLighter.cpp -- bring-up body for Griddraw.h's GridVertexLighter view.
//
// Retail 0x00484f10 (thiscall, ret 8) is the ground-fog vertex shader reconstructed as
// GroundFogCacheOwner::FogVertices in src/reconstructed/GUIManager.cpp: Terrain+0x2c
// holds the cached GroundFog pointer (or 1) and the 0x20-byte GridVertex has the same
// layout as GroundFogVertex (x, y, z, reserved, colour, specular, u, v).  The view's
// method forwards to that body.  GUIManager.h is not included (it pulls in the whole
// UI); the declarations below reproduce the canonical mangled name
// (?FogVertices@GroundFogCacheOwner@@QAEXPAUGroundFogVertex@@H@Z).

#include "Griddraw.h"

struct GroundFogVertex;
struct GroundFogCacheOwner {
    void FogVertices(GroundFogVertex* vertices, int count);
};

// 0x00484f10
void GridVertexLighter::FogVertices(GridVertex* vertices, int count)
{
    reinterpret_cast<GroundFogCacheOwner*>(this)->FogVertices(
        reinterpret_cast<GroundFogVertex*>(vertices), count);
}
