#pragma once

// TerrainVec3.h -- the three-float vector of the terrain code: Terrain (Terrain.h)
// and its height-field quadtree (Griddraw.h, GridNode.cpp) pass it between
// each other.
//
// PROVISIONAL stand-in for the shared Vec3 (Math3D.h is not included: its static const
// objects would add $E initializers that Terrain.cpp does not have).  The out-of-line
// copies 0x00404e60 (ctor), 0x00421cb0 (+), 0x0043c890 (/ scalar), 0x005015b0 (* scalar)
// exist in retail as COMDATs; VC6 inlines them at most call sites of QueryGround.
struct TerrainVec3 {
    float x, y, z;
    TerrainVec3() {}
    TerrainVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    TerrainVec3& operator*=(float s);                            // 0x0040ae00 (out of line at its one call site)
};
