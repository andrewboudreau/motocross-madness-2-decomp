#include "Arrow.h"
#include "DebugAlloc.h"

ArrowManager* g_UnknownArrowManager5776d0 = new (__FILE__, 4) ArrowManager(1);

// 0x004019d0
ArrowManager::ArrowManager(int flags) : GameObject(flags) {
    field_0x414 = 0;
    field_0x418 = 1.0f;
}

// 0x00401a20
ArrowManager::~ArrowManager() {
}

// Folded with the identical overrides of other managers into 0x00462e30.
GameObject* ArrowManager::UnknownVirtualSlot8(void* value) {
    GameObject::UnknownVirtualSlot8(value);
    return this;
}
