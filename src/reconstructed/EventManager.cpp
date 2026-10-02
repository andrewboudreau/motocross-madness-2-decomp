#include "EventManager.h"

#include <string.h>

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

// 0x0045e520
void EventManager::UnknownFunction45e520() {
    for (int i = 0; i < 11; i++)
        field_0x50[i].UnknownFunction45c840();
    field_0x48 = 0;
    g_UnknownGlobal56e26c->field_0x2d7c = 0;
}

// 0x0045e550
void EventManager::UnknownFunction45e550(float) {
    if (!field_0x34)
        return;
    int ready = 1;
    int local = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
    for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
        int player = g_UnknownGlobal56e26c->field_0x2228[i].field_0x08;
        if (player != local) {
            int connected = g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac800(player);
            if (!g_UnknownGlobal56e26c->field_0x2228[i].field_0x00 && connected)
                ready = 0;
        }
    }
    if (ready) {
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac950();
        field_0x34 = 0;
        UnknownFunction45e600();
        if (g_UnknownGlobal56e26c->networkGameObject)
            g_UnknownGlobal56e26c->networkGameObject->UnknownFunction49c770();
    }
}

// 0x0045e930
int UnknownFunction45e930(const void* a, const void* b) {
    const UnknownEventRanking* first = (const UnknownEventRanking*)a;
    const UnknownEventRanking* second = (const UnknownEventRanking*)b;
    if (first->value < second->value)
        return -1;
    if (first->value == second->value) {
        if (first->racer->field_0x7a0 > second->racer->field_0x7a0)
            return -1;
        if (first->racer->field_0x7a0 == second->racer->field_0x7a0) {
            if (first->racer->field_0x790 > second->racer->field_0x790)
                return -1;
            if (first->racer->field_0x790 == second->racer->field_0x790) {
                if (first->racer->field_0x744->field_0x0c < second->racer->field_0x744->field_0x0c)
                    return -1;
                if (first->racer->field_0x744->field_0x0c == second->racer->field_0x744->field_0x0c)
                    return 0;
            }
        }
    }
    return 1;
}

// 0x0045f180
void EventManager::UnknownFunction45f180(UnknownEventRacer* racer, int* points) {
    int table[10];
    table[0] = 20;
    table[1] = 17;
    table[2] = 15;
    table[3] = 12;
    table[4] = 10;
    table[5] = 8;
    table[6] = 6;
    table[7] = 5;
    table[8] = 3;
    table[9] = 1;
    if (racer->field_0x784 >= 1 && racer->field_0x784 <= 10)
        *points += table[racer->field_0x784 - 1];
}

// 0x0045d3d0
int UnknownFunction45d3d0(const void* a, const void* b) {
    const UnknownEventStanding* first = (const UnknownEventStanding*)a;
    const UnknownEventStanding* second = (const UnknownEventStanding*)b;
    if (first->field_0x00 > second->field_0x00)
        return -1;
    if (first->field_0x00 == second->field_0x00) {
        if (first->field_0x04 < second->field_0x04)
            return -1;
        if (first->field_0x04 == second->field_0x04) {
            if (first->field_0x08 < second->field_0x08)
                return -1;
            if (first->field_0x08 == second->field_0x08)
                return strcmp(first->racer->field_0x5e0, second->racer->field_0x5e0);
        }
    }
    return 1;
}

// 0x0045e600
void EventManager::UnknownFunction45e600() {
    if (g_UnknownGlobal56e26c->field_0x08 && field_0x34)
        return;
    if (!g_UnknownGlobal56e26c->uiInteractionBlocked && !g_UnknownGlobal56e26c->field_0x3438) {
        UnknownFunction45e9d0();
        UnknownKrustyBikeView* view = UnknownFunction45d2f0();
        if (view)
            view->UnknownVirtualSlot5();
        if (UnknownFunction45d480()) {
            g_UnknownGlobal56e26c->uiInteractionBlocked = 1;
            g_UnknownGlobal56e26c->field_0x3434 = 0;
            return;
        }
        g_UnknownGlobal56e26c->field_0x3438 = 1;
        g_UnknownGlobal56e26c->uiInteractionBlocked = 0;
        return;
    }
    if (g_UnknownGlobal56e26c->uiInteractionBlocked && !g_UnknownGlobal56e26c->field_0x3438)
        return;
    if (!g_UnknownGlobal56e26c->uiInteractionBlocked && g_UnknownGlobal56e26c->field_0x3438) {
        g_UnknownGlobal56e26c->field_0x3438 = 0;
        if (g_UnknownGlobal56e26c->field_0x08 &&
            (g_UnknownGlobal56e26c->field_0x2d70 != 2 || field_0x48 > g_UnknownGlobal56e26c->field_0x2d7c))
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac510(1);
        field_0x34 = 0;
        UnknownFunction45cdc0(2);
        UnknownFunction45e710(g_UnknownGlobal56e26c->field_0x18 == 1 ? 0x88e : 0x868);
    }
}
