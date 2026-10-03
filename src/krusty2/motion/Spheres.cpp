// Spheres.cpp -- reconstruction of D:\aardvark\VC\krusty2\Spheres.cpp.
#include "core/GameObject.h"
#include "core/DebugAlloc.h"

// Per-TU vector constants (the four header $E initialisers, 0x00504a20..0x00504b20); see
// broadphase/Quadtree.cpp for the evidence.  Copy-initialisation gives the stack temporary.
struct SphereConstVec3 {
    float x, y, z;
    SphereConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const SphereConstVec3 kVec3Zero = SphereConstVec3(0.0f, 0.0f, 0.0f);
static const SphereConstVec3 kVec3XAxis = SphereConstVec3(1.0f, 0.0f, 0.0f);
static const SphereConstVec3 kVec3YAxis = SphereConstVec3(0.0f, 1.0f, 0.0f);
static const SphereConstVec3 kVec3ZAxis = SphereConstVec3(0.0f, 0.0f, 1.0f);

// RTTI .?AVSphereManager@@ (COL 0x0055f290), direct base GameObject (mdisp 0), vtable 0x005581d8
// with overrides at slot 0 (0x005049f0, deleting dtor) and slot 8 (0x00462e30).  Ctor 0x005049c0, object size 0x418
// (the allocation in the $E initialiser, 0x00504950).
class SphereManager : public GameObject {
public:
    explicit SphereManager(int a);
    virtual ~SphereManager();
    virtual GameObject* GameObjectVirtualSlot8(int parentArg);
    char field_0x2c[0x3e8];  // +0x2c..0x413 not accessed in this TU
    int field_0x414;  // +0x414 zeroed by the ctor, nothing else seen yet
};

SphereManager::SphereManager(int a)
    : GameObject(a)
{
    field_0x414 = 0;
}

SphereManager::~SphereManager()
{
}

// $E initialiser at 0x00504950 (jmp thunk at 0x00504940): the global manager is created at
// start-up, __FILE__ Spheres.cpp line 4; the pointer is the global at 0x00689f1c.
SphereManager* g_pSphereManager = new(__FILE__, 4) SphereManager(1);

// RTTI slot 8 points to the shared body at 0x00462e30.
GameObject* SphereManager::GameObjectVirtualSlot8(int parentArg)
{
    GameObject::GameObjectVirtualSlot8(parentArg);
    return this;
}
