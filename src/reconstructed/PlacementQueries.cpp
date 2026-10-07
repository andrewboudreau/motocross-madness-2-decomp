// Placement area queries (0x004b08f0..0x004b0df0). The code sits after
// ObjectPicker.cpp's last function and before ObjectPlacement.cpp's first
// __FILE__ reference, and ObjectPlacement (0x004b0df0) is the main caller.
// No literal ties it to either file, so the file name is ours (tier 3).

// The quadtree of collision objects (krusty2 broadphase/Quadtree.h).
class QuadTreeObject;
class QuadTree {
public:
    int BeginQuery(float x0, float z0, float x1, float z1);  // 0x004dd270
    QuadTreeObject* NextObject();                            // 0x004dd540
    void EndQuery();                                         // 0x004dd770
};
extern QuadTree* g_collisionQuadTree;                        // 0x0068aba4

class TypeRegistry {
public:
    char FindTypeId(const char* name);                       // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;                         // 0x00575744

struct CollisionVec3 {
    float x;
    float y;
    float z;
};

inline CollisionVec3 operator-(const CollisionVec3& a, const CollisionVec3& b)
{
    CollisionVec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

// The members of CollisionObject (krusty2 collision/CollisionObject.h) read
// here. A quadtree entry is the CollisionObject itself (QuadTreeObject at +0).
class CollisionObject {
public:
    char field_0x00[8];
    unsigned char objectTypeId;       // +0x08 QuadTreeObject type id
    char field_0x09[0x64 - 0x09];
    int ownerType;                    // +0x64
    char field_0x68[0x74 - 0x68];
    int ignoreListMode;               // +0x74 0: the list includes, else it excludes
    CollisionObject** ignoreList;     // +0x78
    int ignoreCount;                  // +0x7c
};

// 0x004394f0 (CollisionObject.cpp): whether the sphere touches the object.
int SphereTouchesObject(const CollisionVec3* center, float radius, float radiusSq, CollisionObject* object);

// The ground probe (0x00507c10, thiscall, ret 0x10).
class PlacementProbe {
public:
    void QueryGround(const CollisionVec3* point, CollisionVec3* normal, int one, unsigned char* flag);
};

// The type ids are compared as unsigned bytes: retail loads 0xff with
// `mov bl, 0xff`, where a signed char -1 gives `or bl, 0xff`.
static char s_typeIdsReady;                                  // 0x00688774
static unsigned char s_collisionObjectTypeId = 0xff;         // 0x0056eeb0
static unsigned char s_vegetationTypeId = 0xff;              // 0x0056eeb1

// 0x004b08f0: whether a sphere of `radius` around `point` touches a
// collision object (other than `self` and the two owner types) or a
// vegetation cell. Returns `testObjects` or `testVegetation` for the first
// hit, else 0. Raises point->y by half the radius when the quadtree query
// starts.
int PlacementSphereBlocked(CollisionVec3* point, float radius, CollisionObject* self, int ownerA, int ownerB,
                           int testObjects, int testVegetation)
{
    if (!s_typeIdsReady) {
        if (s_collisionObjectTypeId == 0xff)
            s_collisionObjectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
        if (s_vegetationTypeId == 0xff)
            s_vegetationTypeId = g_TypeRegistry->FindTypeId("Vegetation");
        s_typeIdsReady = 1;
    }
    float radiusSq = radius * radius;
    if (g_collisionQuadTree && (testObjects || testVegetation)) {
        if (g_collisionQuadTree->BeginQuery(point->x - radius, point->z - radius, point->x + radius,
                                            point->z + radius)) {
            point->y += radius * 0.5f;
            for (CollisionObject* object = (CollisionObject*)g_collisionQuadTree->NextObject(); object;
                 object = (CollisionObject*)g_collisionQuadTree->NextObject()) {
                if (testObjects && object->objectTypeId == s_collisionObjectTypeId) {
                    if (object != self && object->ownerType != ownerA && object->ownerType != ownerB) {
                        int test = 1;
                        if (object->ignoreCount && self) {
                            int i;
                            if (object->ignoreListMode == 0) {
                                test = 0;
                                for (i = 0; i < object->ignoreCount; i++) {
                                    if (object->ignoreList[i] == self) {
                                        test = 1;
                                        break;
                                    }
                                }
                            } else {
                                for (i = 0; i < object->ignoreCount; i++) {
                                    if (object->ignoreList[i] == self) {
                                        test = 0;
                                        break;
                                    }
                                }
                            }
                        }
                        if (test && SphereTouchesObject(point, radius, radiusSq, object)) {
                            g_collisionQuadTree->EndQuery();
                            return testObjects;
                        }
                    }
                } else if (testVegetation && object->objectTypeId == s_vegetationTypeId) {
                    g_collisionQuadTree->EndQuery();
                    return testVegetation;
                }
            }
            g_collisionQuadTree->EndQuery();
        }
    }
    return 0;
}

// 0x004b0ac0: the square of `size` at (x, z), all scaled by `scale` * 256,
// and whether `point` lies strictly inside it in x and z. The bounds go to
// the optional outputs.
int PointInPlacementSquare(const CollisionVec3* point, float x, float z, float size, float scale, float* minX,
                           float* maxX, float* minZ, float* maxZ)
{
    float x0 = x * scale * 256.0f;
    float x1 = x0 + size * scale * 256.0f;
    float z0 = z * scale * 256.0f;
    float z1 = z0 + size * scale * 256.0f;
    if (minX)
        *minX = x0;
    if (maxX)
        *maxX = x1;
    if (minZ)
        *minZ = z0;
    if (maxZ)
        *maxZ = z1;
    if (x0 < point->x && point->x < x1 && point->z > z0 && point->z < z1)
        return 1;
    return 0;
}

// 0x004b0b80: does nothing unless `clamp` is set and `position` lies outside
// the square PointInPlacementSquare(position, 1, 1, 1, scale) reports. Then
// each of x and z is set to `scale` * 1.2333 inside the square's edge when
// `center` lies closer to that edge than that margin. Within `scale` * 10 of
// an edge the ground is probed, and where its normal's y is below 0.707
// (steeper than 45 degrees) the coordinate is pushed by `scale` * 10 away
// from that edge. `offset` gets `*target - *position`. `object` and the
// sixth and seventh arguments are unused.
void ClampToPlacementSquare(CollisionObject* object, PlacementProbe* probe, const CollisionVec3* center,
                            float scale, int clamp, int unused6, int unused7, const CollisionVec3* target,
                            CollisionVec3* position, CollisionVec3* offset)
{
    float minX;
    float maxX;
    float minZ;
    float maxZ;
    CollisionVec3 normal;

    if (!clamp)
        return;
    if (PointInPlacementSquare(position, 1.0f, 1.0f, 1.0f, scale, &minX, &maxX, &minZ, &maxZ))
        return;

    float margin = scale * 1.2333f;

    if (center->x < margin + minX)
        position->x = margin + minX;
    else if (maxX - margin < center->x)
        position->x = maxX - margin;
    float step = scale * 10.0f;
    if (step + minX > position->x) {
        CollisionVec3 probePoint = *position;
        probe->QueryGround(&probePoint, &normal, 1, 0);
        if (normal.y < 0.707f)
            position->x = step + position->x;
    } else if (maxX - step < position->x) {
        CollisionVec3 probePoint = *position;
        probe->QueryGround(&probePoint, &normal, 1, 0);
        if (normal.y < 0.707f)
            position->x = position->x - step;
    }

    if (center->z < margin + minZ)
        position->z = margin + minZ;
    else if (maxZ - margin < center->z)
        position->z = maxZ - margin;
    if (step + minZ > position->z) {
        CollisionVec3 probePoint = *position;
        probe->QueryGround(&probePoint, &normal, 1, 0);
        if (normal.y < 0.707f)
            position->z = step + position->z;
    } else if (maxZ - step < position->z) {
        CollisionVec3 probePoint = *position;
        probe->QueryGround(&probePoint, &normal, 1, 0);
        if (normal.y < 0.707f)
            position->z = position->z - step;
    }

    if (offset && target)
        *offset = *target - *position;
}
