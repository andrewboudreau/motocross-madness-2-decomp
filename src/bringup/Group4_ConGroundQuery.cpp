// Bring-up definition (best-guess-unreviewed): ConGroundQuery::QueryPoint,
// the constraint code's view (src/krusty2/constraint/ConstraintTypes.h) of the
// object at ConstraintMethodCollisionModel+0xe4. Its call target 0x00507c10 is
// Terrain::QueryGround (thiscall, ret 0x10), already provided by
// src/krusty2/broadphase/Terrain.cpp, so the object is the Terrain itself
// (GameObject is Terrain's primary base: no pointer adjustment). The fourth
// argument is QueryGround's optional surface-byte output; the only caller
// passes 0. ConVec3 and TerrainVec3 share the 12-byte {x, y, z} float layout.
#include "../krusty2/constraint/ConstraintTypes.h"
#include "../krusty2/broadphase/Terrain.h"

void ConGroundQuery::QueryPoint(ConVec3* point, ConVec3* normalOut, int a, int b)
{
    reinterpret_cast<Terrain*>(this)->QueryGround(reinterpret_cast<TerrainVec3*>(point),
                                                  reinterpret_cast<TerrainVec3*>(normalOut), a,
                                                  reinterpret_cast<unsigned char*>(b));
}
