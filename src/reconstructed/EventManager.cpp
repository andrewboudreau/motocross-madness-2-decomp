#include "EventManager.h"

#include "Camera.h"
#include "ControlInterface.h"
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

// 0x0045f3a0
int EventManager::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (GameObject::UnknownVirtualSlot22(event, entry))
        return 1;
    if (g_UnknownGlobal56e26c->uiInteractionBlocked && !g_UnknownGlobal56e26c->field_0x08 &&
        (event->kind == 2 || event->kind == 0 && event->control == 1 ||
         event->kind == 0 && event->control == 0x1c || event->kind == 0 && event->control == 0x39) &&
        field_0x440) {
        field_0x440 = 0;
        g_UnknownGlobal56e26c->uiInteractionBlocked = 0;
        g_UnknownGlobal56e26c->field_0x3438 = 1;
        UnknownFunction45e600();
        return 1;
    }
    return 0;
}

// 0x0045f440
int EventManager::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    if (g_UnknownGlobal56e26c->uiInteractionBlocked)
        field_0x440 = 1;
    return 0;
}

// 0x0045f200: per-frame update. While UI interaction is blocked it advances
// the block timer, ticks the listeners and pans the camera; after 7 seconds
// it lifts the block.
int EventManager::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x3c) {
        UnknownFunction4aef40();
        field_0x3c = 0;
    }
    if (field_0x34)
        UnknownFunction45e550(frameTime);
    else if (!g_UnknownGlobal56e26c->field_0x3428 && !g_UnknownGlobal56e26c->uiInteractionBlocked &&
             UnknownFunction45d390())
        UnknownFunction45eef0(frameTime);
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        g_UnknownGlobal56e26c->field_0x3434 += frameTime;
        for (int i = 0; i < field_0x420; i++)
            field_0x424[i]->UnknownVirtualSlot7(frameTime, 0, 0);
        field_0x3e4 += frameTime * field_0x414 * (1.0f / 7);
        field_0x3d4->UnknownFunction42e9b0(&field_0x3e4, 0, 0, 0, 0);
        field_0x3d4->UnknownVirtualSlot29(field_0x3d8);
        if (!g_UnknownGlobal56e26c->field_0x3438 && g_UnknownGlobal56e26c->field_0x3434 > 7.0f) {
            g_UnknownGlobal56e26c->uiInteractionBlocked = 0;
            g_UnknownGlobal56e26c->field_0x3438 = 1;
            UnknownFunction45e600();
        }
    }
    return 1;
}
