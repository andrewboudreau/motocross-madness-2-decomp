// Complete out-of-line scale operators used by Bike and Terrain.
// Both call views target 0x0040ae00, 33 bytes. Their signatures preserve the
// thiscall receiver, four-byte argument and returned receiver reference.
// Names and this source grouping are provisional, not original TU ownership.
#include "VectorScaleAssign.h"
#include "../krusty2/broadphase/Terrain.h"

BikeVec3ScaleAssign& BikeVec3ScaleAssign::operator*=(float scale)
{
    x *= scale;
    y *= scale;
    z *= scale;
    return *this;
}

TerrainVec3& TerrainVec3::operator*=(float scale)
{
    x *= scale;
    y *= scale;
    z *= scale;
    return *this;
}
