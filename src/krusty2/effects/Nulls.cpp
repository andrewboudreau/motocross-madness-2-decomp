// Nulls.cpp -- reconstruction of D:\aardvark\VC\krusty2\Nulls.cpp (0x0056ed14).
//
// Retail layout: the file's single own __FILE__ reference is the `new` at line 5 in the static
// initialiser 0x004b00f0 (entry 0x004b00e0 is the jmp thunk the pre-main table points at).
// The NullManager methods 0x004b0160..0x004b01bb follow contiguously, then ObjectPicker.cpp.
#include "core/GameObject.h"
#include "core/DebugAlloc.h"

// RTTI .?AVNullManager@@ COL 0x0055da80, vtable 0x0055549c (27 slots, object_offset 0), single
// base GameObject.  Overrides: slot 0 (deleting dtor 0x004b0190) and slot 8 0x00462e30, which
// is a stub shared with ArrowManager/FogOff/SphereManager (forwards to GameObject slot 8).
// Allocation size 0x418 (the `new` at 0x004b0112), last field +0x414 zeroed by the ctor.
class NullManager : public GameObject {
public:
    explicit NullManager(int flags);
    virtual ~NullManager();
    virtual GameObject* GameObjectVirtualSlot8(int parentArg);

    char field_0x2c[0x414 - 0x2c];
    int field_0x414;    // +0x414 zeroed by the ctor 0x004b0160; no other access seen in the bracket
};

NullManager::NullManager(int flags) : GameObject(flags)
{
    field_0x414 = 0;
}

NullManager::~NullManager()
{
}

// Global at 0x006886f0 (written by the initialiser at 0x004b00f0; null when the allocation fails).
NullManager* g_nullManager = new(__FILE__, 5) NullManager(1);

// RTTI slot 8 points to the shared body at 0x00462e30.
GameObject* NullManager::GameObjectVirtualSlot8(int parentArg)
{
    GameObject::GameObjectVirtualSlot8(parentArg);
    return this;
}
