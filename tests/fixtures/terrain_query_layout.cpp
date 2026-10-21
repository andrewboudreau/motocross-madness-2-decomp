#include "../../src/krusty2/broadphase/Terrain.h"
#include <stddef.h>

// Compile the two unrelated consumer views separately: their visibility
// globals still use distinct provisional declarations.
#ifdef CHECK_ECO
#include "../../src/reconstructed/EcoSystem.h"
#else
#include "../../src/reconstructed/FollowCamera.h"
typedef char CameraTerrainCell[offsetof(UnknownFollowCameraSubject, gridCellSize) == offsetof(Terrain, gridCellSize) ? 1 : -1];
typedef char CameraTerrainFlag[offsetof(UnknownFollowCameraSubject, field_0xbe8) == offsetof(Terrain, field_0xbe8) ? 1 : -1];
typedef char VectorSize[sizeof(Vector3) == 12 && sizeof(TerrainVec3) == 12 ? 1 : -1];
typedef char VectorX[offsetof(Vector3, x) == 0 && offsetof(TerrainVec3, x) == 0 ? 1 : -1];
typedef char VectorY[offsetof(Vector3, y) == 4 && offsetof(TerrainVec3, y) == 4 ? 1 : -1];
typedef char VectorZ[offsetof(Vector3, z) == 8 && offsetof(TerrainVec3, z) == 8 ? 1 : -1];
#endif
typedef char GroundFogSize[sizeof(GroundFogableObject) == 4 ? 1 : -1];
typedef char TerrainSize[sizeof(Terrain) == 0xcc4 ? 1 : -1];
