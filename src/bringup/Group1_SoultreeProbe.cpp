// Group1_SoultreeProbe.cpp -- bring-up body for SoultreePhysicsBaseObjectCore.h's
// SoultreeProbe view.
//
// Retail 0x00506e90 (thiscall, ret 0x18) is Terrain::CastSegment, reconstructed in
// src/krusty2/broadphase/Terrain.cpp: the physics objects' +0x1f4 probe is the
// terrain.  The view's method forwards to the canonical body.  Terrain.h is not
// included (its own Vec3 stand-ins collide with math/Vec3.h); the declaration below
// reproduces Terrain::CastSegment's mangled name exactly
// (?CastSegment@Terrain@@QAEHPBUTerrainVec3@@0PAU2@HHH@Z).  TerrainVec3 and Vec3 are
// both three packed floats.

#include "soultree/SoultreePhysicsBaseObjectCore.h"

struct TerrainVec3;
class Terrain {
public:
    int CastSegment(const TerrainVec3* from, const TerrainVec3* to, TerrainVec3* out,
                    int a, int b, int c);
};

// 0x00506e90
int SoultreeProbe::CastSegment(const Vec3* from, const Vec3* to, Vec3* out, int a, int b, int c)
{
    return reinterpret_cast<Terrain*>(this)->CastSegment(
        reinterpret_cast<const TerrainVec3*>(from), reinterpret_cast<const TerrainVec3*>(to),
        reinterpret_cast<TerrainVec3*>(out), a, b, c);
}
