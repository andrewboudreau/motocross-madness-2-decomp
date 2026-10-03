// Collision pipeline as understood (tier 3, from call graph + decoded bodies):
//   SoultreePhysics contacts -> CollisionPoint::Fn_0043a640 (0x43a640): builds a
//     contact from position delta, separating direction and tangent frame via
//     cross products; normalizes (0x5087b0), computes penetration/length, and
//     registers points with AddCollisionPoint (0x43a330).
//   0x43aa30 merges the penetrating contacts (mean position, summed+normalised normal,
//     deepest penetration, response terms); 0x43ad80 refreshes contacts (CollisionContactUpdate.cpp).
//   Broadphase helpers: SphereContainsPointXZ (0x43a1e0, point-in-circle in XZ),
//     BoundingSpheresOverlap (0x43a270). Vector helpers: CollisionRejectFrom
//     (0x43b190, a - proj_b a), CollisionLength (0x435ec0), CollisionDivide (0x43c890).
//   Narrow phase (CollisionShapeTests.cpp, tier 3 names; shape type = field_0x50:
//     0 hull/oriented box, 1 model = array of hulls, 2 static mesh, 3 capsule, 4 sphere):
//     TestAgainst 0x436430 filters and picks TestHull/Model/MeshAgainst (0x438b90/0x438c10/0x438c90);
//     the first two jump-table into HullVs{Hull 0x436720, Model 0x436e50, Capsule 0x437aa0,
//     Sphere 0x4379c0} and ModelVs{Hull 0x437c20, Model 0x438280, Capsule 0x4389b0, Sphere 0x438860};
//     swept variants 0x436af0 / 0x4376f0 / 0x437ef0 / 0x438550 are chosen by the hull swept flag.
//     Hull tests bring the query into the other body's frame, call the box test 0x428950
//     (or 0x429570/0x429890 for sphere/capsule), average contacts and transform them back.
//     GetWorldBounds 0x436100 is an Arvo transformed AABB.
//   CollisionObject owns shapes (ctor 0x431e70, FreeShape 0x432430).

#include "collision/CollisionObject.h"
#include "broadphase/Quadtree.h"


// Retail helpers reached by direct call (addresses are the call targets).
void Fn_0042a160(void* p);                       // shape sub-object destructor (cdecl, 1 arg)
void CollisionHullShape_Free(CollisionHullShape* s);   // 0x00431da0
struct CollisionHullElement { char bytes[0x198]; };
struct CollisionModelShapeData {   // type 1: count + array of CollisionHullShape (stride 0x198)
    int hullCount;
    void* field_0x04;
    void* field_0x08;
    char field_0x0c[8];
    CollisionHullElement* hulls;   // hull elements, 0x198 bytes each
};
struct CollisionMeshShapeData {    // type 2
    int field_0x00;
    void* field_0x04;
};
void CollisionModelShape_Free(void* s);                // 0x00431df0
void CollisionMeshShape_Free(void* s);                 // 0x00431e50
void Fn_004a30c0(void* p);                             // free()

void CollisionHullShape_Free(CollisionHullShape* s) {
    if (s->triangleTree)
        Fn_0042a160(s->triangleTree);
    if (s->pointTree)
        Fn_0042a160(s->pointTree);
    if (s->vertices)
        operator delete(s->vertices, __FILE__, 36);
}

void CollisionModelShape_Free(void* shape) {
    CollisionModelShapeData* s = (CollisionModelShapeData*)shape;
    for (int i = 0; i < s->hullCount; i++)
        CollisionHullShape_Free((CollisionHullShape*)&s->hulls[i]);
    Fn_004a30c0(s->hulls);
    Fn_004a30c0(s->field_0x04);
    Fn_004a30c0(s->field_0x08);
    Fn_004a30c0(s);
}

void CollisionMeshShape_Free(void* shape) {
    CollisionMeshShapeData* s = (CollisionMeshShapeData*)shape;
    if (s->field_0x04)
        Fn_0042a160(s->field_0x04);
    Fn_004a30c0(s);
}

void CollisionObject::FreeShape() {
    if (shape != 0) {
        switch (shapeType) {
        case 0:
            CollisionHullShape_Free((CollisionHullShape*)shape);
            delete shape;
            break;
        case 1:
            CollisionModelShape_Free(shape);
            break;
        case 2:
            CollisionMeshShape_Free(shape);
            break;
        case 3:
        case 4:
            delete shape;
            break;
        }
    }
    if (contactRecord != 0)
        delete contactRecord;
    shape = 0;
    contactRecord = 0;
}

// Type-id registry (TypeRegistry.cpp): 0x00521ea0 looks a type name up and returns its id.
class TypeRegistry {
public:
    char FindTypeId(const char* name);           // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;             // 0x00575744

static char g_CollisionObjectTypeId = (char)0xff;   // 0x00568610
static char g_VegetationTypeId = (char)0xff;        // 0x00568611

// 0x00431e70
CollisionObject::CollisionObject(int a)
    : GraphicsTest(a)
{
    if (g_CollisionObjectTypeId == (char)0xff)
        g_CollisionObjectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
    if (g_VegetationTypeId == (char)0xff)
        g_VegetationTypeId = g_TypeRegistry->FindTypeId("Vegetation");
    Fn_00469ce0(this);
    QuadTreeObject::objectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
    shapeType = 0;
    shape = 0;
    contactRecord = 0;
    hasContact = 0;
    onHitCallback = 0;
    onHitByCallback = 0;
    debugTreeDepth = 0;
    debugDrawMode = 0;
    quadtreeCell = 15;
    ignoreVegetation = 0;
    collisionEnabled = 1;
    collidable = 1;
    ignoreListMode = 1;
    ignoreList = 0;
    ignoreCount = 0;
    ownerObject = 0;
    ownerType = 0;
    field_0x98 = 0;
    hitObject = 0;
    hitPoint = g_CollisionVec3_5797b0;
    hitNormal = g_CollisionVec3_5797b0;
}

// 0x00431fd0 (deleting) -> 0x00432000 (core)
CollisionObject::~CollisionObject()
{
    if (g_collisionQuadTree && useBroadphase)
        g_collisionQuadTree->Remove(this, quadtreeCell);
    FreeShape();
    if (ignoreList)
        operator delete((void*)ignoreList, __FILE__, 0x6c);
    if (shape) {
        switch (shapeType) {
        case 0:
            CollisionHullShape_Free((CollisionHullShape*)shape);
            delete shape;
            break;
        case 1:
            CollisionModelShape_Free(shape);
            break;
        case 2:
            CollisionMeshShape_Free(shape);
            break;
        case 3:
        case 4:
            delete shape;
            break;
        }
    }
}
