#include "../../src/krusty2/broadphase/Terrain.h"
#include "../../src/reconstructed/Grid1.h"
#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/FileStream.h"
#include <stddef.h>

// Only the selected call boundary is shared; partial views remain distinct.
typedef char TerrainSize[sizeof(Terrain) == 0xcc4 ? 1 : -1];
typedef char TerrainCellSize[offsetof(Terrain, gridCellSize) == 0x40 ? 1 : -1];
typedef char TerrainHeightField[offsetof(Terrain, heightField) == 0x44 ? 1 : -1];
typedef char TerrainPool[offsetof(Terrain, ownedObjectArray) == 0xcb8 ? 1 : -1];
typedef char TerrainPoolCount[offsetof(Terrain, ownedObjectArrayCount) == 0xcbc ? 1 : -1];
typedef char GridCameraField[offsetof(Grid1Terrain, cameraFocalLength) == offsetof(Terrain, cameraFocalLength) ? 1 : -1];
typedef char GridTextures[offsetof(Grid1Terrain, textures) == offsetof(Terrain, ownedObjects) ? 1 : -1];
typedef char VectorSize[sizeof(Vector3) == 12 && sizeof(TerrainVec3) == 12 ? 1 : -1];
typedef char VectorX[offsetof(Vector3, x) == 0 && offsetof(TerrainVec3, x) == 0 ? 1 : -1];
typedef char VectorY[offsetof(Vector3, y) == 4 && offsetof(TerrainVec3, y) == 4 ? 1 : -1];
typedef char VectorZ[offsetof(Vector3, z) == 8 && offsetof(TerrainVec3, z) == 8 ? 1 : -1];
typedef char StreamSize[sizeof(TerrainStream) == 0x134 && sizeof(UnknownTextureStream) == 0x134 ? 1 : -1];
typedef char StreamInner[offsetof(TerrainStream, inner) == offsetof(UnknownTextureStream, innerStream) ? 1 : -1];
typedef char StreamSliceOffset[offsetof(TerrainStream, innerStart) == offsetof(UnknownTextureStream, sliceOffset) ? 1 : -1];
