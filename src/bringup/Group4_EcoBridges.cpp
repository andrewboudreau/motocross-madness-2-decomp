// Bring-up bridges (best-guess-unreviewed) for src/bringup/Group4_EcoSystem.cpp.
//
// The EcoSystem near-miss bodies were written against the proxy types in
// src/reconstructed/EcoSystem.h (UnknownEcoTerrain, the proxy CollisionObject
// taking Vector3, NormalDistribution via g_SharedNormalDistribution). Those
// proxies mangle differently from the real providers (Terrain::QueryGround
// takes TerrainVec3*, CollisionObject::SetSphereShape takes CollisionVec3), so
// this unit includes the real headers and exposes uniquely named free
// functions the EcoSystem bring-up unit calls instead. All vector types here
// are the same 12-byte {x, y, z} float layout.
#include "../krusty2/collision/CollisionObject.h"
#include "../krusty2/broadphase/Terrain.h"

struct Group4EcoVec3 {
    float x, y, z;
};

static CollisionVec3 Group4ToCollision(const Group4EcoVec3* v) {
    CollisionVec3 r;
    r.x = v->x;
    r.y = v->y;
    r.z = v->z;
    return r;
}

// 0x004328b0 CollisionObject::SetTriangleMeshHullShape.
void Group4Eco_SetTriangleMeshHullShape(void* object, const void* vertices, const int* indices,
                                        int triangleCount, int vertexCount) {
    ((CollisionObject*)object)->SetTriangleMeshHullShape((const CollisionVec3*)vertices, indices,
                                                          triangleCount, vertexCount);
}

// 0x004329a0 CollisionObject::SetSphereShape.
void Group4Eco_SetSphereShape(void* object, const void* center, float radius) {
    ((CollisionObject*)object)->SetSphereShape(Group4ToCollision((const Group4EcoVec3*)center), radius);
}

// 0x00432a20 CollisionObject::SetCapsuleShape.
void Group4Eco_SetCapsuleShape(void* object, const void* p0, const void* p1, float radius) {
    ((CollisionObject*)object)->SetCapsuleShape(Group4ToCollision((const Group4EcoVec3*)p0),
                                                 Group4ToCollision((const Group4EcoVec3*)p1), radius);
}

// 0x00507c10 Terrain::QueryGround. The EcoSystem keeps the ground terrain as
// a Terrain* (GameObject is Terrain's primary base, so no adjustment).
void Group4Eco_QueryGround(void* terrain, void* position, void* normal, int flatShaded,
                           unsigned char* surface) {
    ((Terrain*)terrain)->QueryGround((TerrainVec3*)position, (TerrainVec3*)normal, flatShaded, surface);
}

// 0x00508970 Terrain::GetHeightRange.
void Group4Eco_GetHeightRange(void* terrain, float* outMin, float* outMax) {
    ((Terrain*)terrain)->GetHeightRange(outMin, outMax);
}

// 0x0056ece0: the shared bell-curve table pointer EcoSystem::GenerateObjects
// weighs slope and altitude with. In retail it points at NormalDistribution.cpp's
// static object (0x006886e8, resolution 100). That object is file-static in
// src/krusty2/effects/NormalDistribution.cpp, so
// best guess: point at an identical private instance built by the same
// out-of-line constructor (the table is a pure function of the resolution).
class NormalDistribution {
public:
    explicit NormalDistribution(int resolution);  // NormalDistribution.cpp
    ~NormalDistribution();
    float Lookup(float mean, float value, float sigma); // 0x004b0070
    float* table;
    int count;
};
static NormalDistribution s_group4NormalDistribution(100);
NormalDistribution* g_SharedNormalDistribution = &s_group4NormalDistribution;
