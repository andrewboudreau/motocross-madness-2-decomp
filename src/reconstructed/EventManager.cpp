#include "EventManager.h"

#include "TrackGame.h"

// 0x0045c9e0
EventManager::EventManager(int flags) : GameObject(flags) {
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x3c0 = 0;
    field_0x2c = 20.0f;
    field_0x30 = 1.0f;
    field_0x440 = 0;
    field_0x3d4 = 0;
    field_0x3d0 = 0;
    field_0x3c4 = Vector3(-1000.0f, -1000.0f, -1000.0f);
}

// 0x0045cae0
EventManager::~EventManager() {}

// 0x0045caf0
GameObject* EventManager::UnknownVirtualSlot8(void* value) {
    GameObject::UnknownVirtualSlot8(value);
    field_0x2c = (float)g_UnknownGlobal56e26c->UnknownVirtualSlot20("KeepAliveTimeout", 20);
    return this;
}

// 0x0045d270
void EventManager::UnknownFunction45d270() {
    TrackGameViewOwner* owner = UnknownFunction45d2b0();
    if (!owner)
        return;
    owner->UnknownVirtualSlot5();
    UnknownKrustyBikeView* view = UnknownFunction45d2f0();
    if (view)
        view->UnknownVirtualSlot5();
    UnknownMessageTarget* target = UnknownFunction45d340();
    if (target)
        target->UnknownVirtualSlot5();
}

// 0x0045d2b0
TrackGameViewOwner* EventManager::UnknownFunction45d2b0() {
    if (g_UnknownGlobal56e26c->field_0x558)
        return g_UnknownGlobal56e26c->field_0x558;
    if (g_UnknownGlobal56e26c->field_0x55c)
        return g_UnknownGlobal56e26c->field_0x55c;
    if (g_UnknownGlobal56e26c->field_0x560)
        return g_UnknownGlobal56e26c->field_0x560;
    if (g_UnknownGlobal56e26c->field_0x564)
        return g_UnknownGlobal56e26c->field_0x564;
    return g_UnknownGlobal56e26c->field_0x568;
}

// 0x0045d2f0
UnknownKrustyBikeView* EventManager::UnknownFunction45d2f0() {
    if (g_UnknownGlobal56e26c->field_0x558)
        return g_UnknownGlobal56e26c->field_0x558->field_0x34;
    if (g_UnknownGlobal56e26c->field_0x55c)
        return g_UnknownGlobal56e26c->field_0x55c->field_0x34;
    if (g_UnknownGlobal56e26c->field_0x560)
        return g_UnknownGlobal56e26c->field_0x560->field_0x34;
    if (g_UnknownGlobal56e26c->field_0x564)
        return g_UnknownGlobal56e26c->field_0x564->field_0x34;
    if (g_UnknownGlobal56e26c->field_0x568)
        return g_UnknownGlobal56e26c->field_0x568->field_0x34;
    return 0;
}

// 0x0045d340
UnknownMessageTarget* EventManager::UnknownFunction45d340() {
    if (g_UnknownGlobal56e26c->field_0x558)
        return g_UnknownGlobal56e26c->field_0x558->field_0x6c;
    if (g_UnknownGlobal56e26c->field_0x55c)
        return g_UnknownGlobal56e26c->field_0x55c->field_0x6c;
    if (g_UnknownGlobal56e26c->field_0x560)
        return g_UnknownGlobal56e26c->field_0x560->field_0x6c;
    if (g_UnknownGlobal56e26c->field_0x564)
        return g_UnknownGlobal56e26c->field_0x564->field_0x6c;
    if (g_UnknownGlobal56e26c->field_0x568)
        return g_UnknownGlobal56e26c->field_0x568->field_0x6c;
    return 0;
}

// 0x0045d390
int EventManager::UnknownFunction45d390() {
    if (g_UnknownGlobal56e26c->field_0x558 || g_UnknownGlobal56e26c->field_0x55c ||
        g_UnknownGlobal56e26c->field_0x560 || g_UnknownGlobal56e26c->field_0x564 ||
        g_UnknownGlobal56e26c->field_0x568)
        return 1;
    return 0;
}
