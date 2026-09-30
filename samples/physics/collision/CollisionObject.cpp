// Collision pipeline as understood (tier 3, from call graph + decoded bodies):
//   SoultreePhysics contacts -> CollisionPoint::Fn_0043a640 (0x43a640): builds a
//     contact from position delta, separating direction and tangent frame via
//     cross products; normalizes (0x5087b0), computes penetration/length, and
//     registers points with AddCollisionPoint (0x43a330).
//   0x43aa30 reduces the contact manifold; 0x43ad80 finalizes contacts.
//   Broadphase helpers: SphereContainsPointXZ (0x43a1e0, point-in-circle in XZ),
//     BoundingSpheresOverlap (0x43a270). Vector helpers: CollisionRejectFrom
//     (0x43b190, a - proj_b a), CollisionLength (0x435ec0), CollisionDivide (0x43c890).
//   0x436100-0x436e50 and 0x4376f0-0x4389b0 are narrow-phase shape tests; not yet decoded.
//   CollisionObject owns shapes (ctor 0x431e70, FreeShape 0x432430).

#include "CollisionObject.h"

// Retail helpers reached by direct call (addresses are the call targets).
void Fn_0042a160(void* p);                       // shape sub-object destructor (cdecl, 1 arg)
void CollisionHullShape_Free(CollisionHullShape* s);   // 0x00431da0
void CollisionModelShape_Free(void* s);                // 0x00431df0
void CollisionMeshShape_Free(void* s);                 // 0x00431e50

void CollisionHullShape_Free(CollisionHullShape* s) {
    if (s->field_0x188)
        Fn_0042a160(s->field_0x188);
    if (s->field_0x18c)
        Fn_0042a160(s->field_0x18c);
    if (s->field_0x190)
        operator delete(s->field_0x190, __FILE__, 36);
}

void CollisionObject::FreeShape() {
    if (field_0x54 != 0) {
        switch (field_0x50) {
        case 0:
            CollisionHullShape_Free((CollisionHullShape*)field_0x54);
            delete field_0x54;
            break;
        case 1:
            CollisionModelShape_Free(field_0x54);
            break;
        case 2:
            CollisionMeshShape_Free(field_0x54);
            break;
        case 3:
        case 4:
            delete field_0x54;
            break;
        }
    }
    if (field_0x5c != 0)
        delete field_0x5c;
    field_0x54 = 0;
    field_0x5c = 0;
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
    field_0x08 = g_TypeRegistry->FindTypeId("CollisionObject");
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x5c = 0;
    field_0x58 = 0;
    field_0x88 = 0;
    field_0x8c = 0;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0x84 = 15;
    field_0x68 = 0;
    field_0x6c = 1;
    field_0x70 = 1;
    field_0x74 = 1;
    field_0x78 = 0;
    field_0x7c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0xa0 = g_CollisionVec3_5797b0;
    field_0xac = g_CollisionVec3_5797b0;
}
