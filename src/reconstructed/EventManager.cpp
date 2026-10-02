#include "EventManager.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Camera.h"
#include "DebugAlloc.h"
#include "ControlInterface.h"
#include "QuarryEvent.h"
#include "TrackGame.h"

// Network messages handled by slot 24 (the layout depends on the type).
struct UnknownEventPlayerMessage {
    int field_0x00;
    int field_0x04;                                // player (types 0xcc and 0x8e)
    int field_0x08;                                // player (type 5)
};
struct UnknownEventRacerMessage {                  // type 0x86
    int field_0x00;
    char field_0x04;                               // the sender's racer index
    char field_0x05;                               // finished
    int field_0x08;
    float field_0x0c;
    int field_0x10;
    int field_0x14;
    int field_0x18;
    float field_0x1c;
    float field_0x20;
};

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
        int player = g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4;
        if (player != local) {
            int connected = g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac800(player);
            if (!g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc && connected)
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

// 0x0045f490: network messages. Type 5 and 0x89 mark a player ready (and
// in mode 2 without a race-mode object start it), 0x86 updates a remote
// racer, 0xcc reports a player leaving and 0x8e the host ending the event.
int EventManager::UnknownVirtualSlot24(int type, void* data, int player, int d, int e) {
    if (GameObject::UnknownVirtualSlot24(type, data, player, d, e))
        return 1;
    UnknownEventPlayerMessage* message = (UnknownEventPlayerMessage*)data;
    char name[16];
    char text[128];
    char line[260];
    if (type == 5) {
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
            if (g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4 == message->field_0x08)
                g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc = 1;
        }
        if (g_UnknownGlobal56e26c->field_0x2d70 == 2 && !UnknownFunction45d2b0())
            UnknownFunction45fbd0(message->field_0x08);
    } else if (player) {
        if (type == 0x89) {
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
                if (g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4 == player)
                    g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc = 1;
            }
            if (g_UnknownGlobal56e26c->field_0x2d70 == 2 && !UnknownFunction45d2b0())
                UnknownFunction45fbd0(player);
        } else if (type == 0x86) {
            UnknownEventRacerMessage* update = (UnknownEventRacerMessage*)data;
            UnknownKrustyBikeView* view = UnknownFunction45d2f0();
            if (!view || g_UnknownGlobal56e26c->uiInteractionBlocked)
                return 0;
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
                if (g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4 == player &&
                    g_UnknownGlobal56e26c->field_0x215c[i].field_0xd8 == update->field_0x04) {
                    g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc = 1;
                    if (view->field_0x3c[i]) {
                        view->field_0x3c[i]->field_0x768 = update->field_0x08;
                        view->field_0x3c[i]->field_0x750 = update->field_0x10;
                        view->field_0x3c[i]->field_0x7a4 = update->field_0x05;
                        view->field_0x3c[i]->field_0x788 = update->field_0x14;
                        view->field_0x3c[i]->field_0x758 = update->field_0x18;
                        view->field_0x3c[i]->field_0x760 = update->field_0x1c;
                        view->field_0x3c[i]->field_0x764 = update->field_0x20;
                        if (view->field_0x3c[i]->field_0x7a4) {
                            view->field_0x3c[i]->field_0x754 = update->field_0x0c;
                            // Always false here; retail still tests it.
                            if (!view->field_0x3c[i]->field_0x7a4)
                                view->field_0x3c[i]->field_0x748 = UnknownFunction4bfa80();
                        } else {
                            view->field_0x3c[i]->field_0x754 = FLT_MAX;
                            view->field_0x3c[i]->field_0x748 = 0x7ffffffe;
                        }
                    }
                }
            }
            if (field_0x38)
                return 0;
            if (g_UnknownGlobal56e26c->field_0x2d74 == 4) {
                TrackGameViewOwner* owner = UnknownFunction45d2b0();
                if (owner->field_0xa8 == owner->field_0x34->field_0x38) {
                    owner->field_0xa8->field_0x764 += owner->field_0xa8->field_0x75c;
                    // Compared through locals, kept as the best of +0x75c.
                    float best = owner->field_0xa8->field_0x760;
                    float lap = owner->field_0xa8->field_0x75c;
                    owner->field_0xa8->field_0x760 = best > lap ? owner->field_0xa8->field_0x760
                                                                : owner->field_0xa8->field_0x75c;
                    owner->field_0xa8->field_0x75c = 0;
                }
            }
            UnknownFunction45f9a0();
            UnknownFunction45e600();
        } else if (type == 0xcc) {
            UnknownMessageTarget* target = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d340();
            if (target) {
                g_UnknownGlobal56e26c->UnknownFunction521970(0x13d7, text, sizeof(text));
                if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac720(message->field_0x04, name)) {
                    sprintf(line, "%s %s", name, text);
                    UnknownMessage notice(line, 3.25f);
                    target->UnknownFunction51b540(&notice);
                }
            }
            if (message->field_0x04 == g_UnknownGlobal56e26c->field_0x08->field_0x0c) {
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac950();
                field_0x3c = 1;
            } else {
                for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
                    if (message->field_0x04 == g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4) {
                        g_UnknownGlobal56e26c->field_0x215c[i].field_0xc8 = 1;
                        g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc = 1;
                    }
                }
            }
            UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
            if (view)
                view->UnknownFunction420590(message->field_0x04);
        } else if (type == 0x8e) {
            if (message->field_0x04 == g_UnknownGlobal56e26c->field_0x08->field_0x0c) {
                if (UnknownFunction45d2b0()) {
                    UnknownMessageTarget* target = UnknownFunction45d340();
                    if (target) {
                        g_UnknownGlobal56e26c->UnknownFunction521970(0x13d1, line, 128); // capped like `text`
                        UnknownMessage notice(line, 3.25f);
                        target->UnknownFunction51b540(&notice);
                    }
                } else {
                    UnknownFunction45e520();
                }
                field_0x3c = 1;
            }
        }
    }
    return 0;
}

// 0x0045fbb0
int UnknownFunction45fbb0(const void* a, const void* b) {
    unsigned int first = *(const unsigned int*)a;
    unsigned int second = *(const unsigned int*)b;
    if (first < second)
        return -1;
    return first != second;
}

// 0x0045fbd0: removes `player`, moving the last entry and record into its
// place, then re-sorts the entries.
void EventManager::UnknownFunction45fbd0(int player) {
    if (g_UnknownGlobal56e26c->field_0x18 < 2)
        return;
    for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
        if (field_0x50[i].field_0x00 == player) {
            if (!field_0x50[i].field_0x30)
                g_UnknownGlobal56e26c->field_0x18--;
            else
                g_UnknownGlobal56e26c->field_0x3424--;
            field_0x50[i] = field_0x50[g_UnknownGlobal56e26c->field_0x2158 - 1];
            if (UnknownFunction45d2b0())
                g_UnknownGlobal56e26c->field_0x215c[i] =
                    g_UnknownGlobal56e26c->field_0x215c[g_UnknownGlobal56e26c->field_0x2158 - 1];
            g_UnknownGlobal56e26c->field_0x2da5--;
        }
    }
    if (g_UnknownGlobal56e26c->field_0x2158)
        qsort(field_0x50, g_UnknownGlobal56e26c->field_0x2158 - 1, sizeof(UnknownEventEntry), UnknownFunction45fbb0);
}

// 0x0045f9a0: once, sends the local racer's state and then each AI racer's
// (type 0x86), and starts the network wait.
void EventManager::UnknownFunction45f9a0() {
    if (!g_UnknownGlobal56e26c->field_0x08 || field_0x38)
        return;
    UnknownKrustyBikeView* view = UnknownFunction45d2f0();
    if (!view->field_0x38->field_0x7a4) {
        view->field_0x38->field_0x754 = FLT_MAX;
        view->field_0x38->field_0x748 = 0x7ffffffe;
    }
    UnknownEventRacerMessage message;
    message.field_0x08 = view->field_0x38->field_0x768;
    message.field_0x0c = view->field_0x38->field_0x754;
    message.field_0x10 = view->field_0x38->field_0x750;
    message.field_0x05 = view->field_0x38->field_0x7a4;
    message.field_0x14 = view->field_0x38->field_0x788;
    message.field_0x18 = view->field_0x38->field_0x758;
    message.field_0x1c = view->field_0x38->field_0x760;
    message.field_0x20 = view->field_0x38->field_0x764;
    message.field_0x04 = 0;
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(
        0x86, &message, sizeof(message), g_UnknownGlobal56e26c->field_0x08->field_0x0c, 0);
    for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2d94; i++) {
        if (!view->field_0x40[i]->field_0x7a4) {
            view->field_0x40[i]->field_0x754 = FLT_MAX;
            view->field_0x40[i]->field_0x748 = 0x7ffffffe;
        }
        message.field_0x08 = view->field_0x40[i]->field_0x768;
        message.field_0x0c = view->field_0x40[i]->field_0x754;
        message.field_0x10 = view->field_0x40[i]->field_0x750;
        message.field_0x05 = view->field_0x40[i]->field_0x7a4;
        message.field_0x14 = view->field_0x40[i]->field_0x788;
        message.field_0x18 = view->field_0x40[i]->field_0x758;
        message.field_0x1c = view->field_0x40[i]->field_0x760;
        message.field_0x20 = view->field_0x40[i]->field_0x764;
        message.field_0x04 = view->field_0x40[i]->field_0x11c0;
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(
            0x86, &message, sizeof(message), g_UnknownGlobal56e26c->field_0x08->field_0x0c, 0);
    }
    field_0x38 = 1;
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac8d0(field_0x2c, field_0x30);
    view->UnknownVirtualSlot4();
    field_0x34 = 1;
}

// Global at 0x0059af54; 0x0045cb70 clears it first.
float g_UnknownGlobal59af54;

// 0x0045cb70: resets the event, loads the track's environment and scene
// and, for mode 0 (BaseQuarryEvent) or 2 (NationalRace), creates the race
// object and adds it to the second root; 1 when a race-mode object exists.
int EventManager::UnknownFunction45cb70() {
    char path[260];
    g_UnknownGlobal59af54 = 0;
    field_0x3c0 = 0;
    field_0x38 = 0;
    field_0x34 = 0;
    for (int i = 0; i < 8; i++)
        g_UnknownGlobal56e26c->field_0x215c[i].field_0xcc = 0;
    field_0x420 = 0;
    for (int j = 0; j < 3; j++)
        field_0x424[j] = 0;
    field_0x3d4 = 0;
    field_0x3d0 = 0;
    g_UnknownGlobal56e26c->ui->UnknownFunction49b530();
    g_UnknownGlobal56e26c->mode.UnknownFunction523a60(g_UnknownGlobal56e26c->mode.field_0x6a0,
                                                      g_UnknownGlobal56e26c->field_0x2da6, "env", path);
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(path);
    switch (g_UnknownGlobal56e26c->field_0x2d74) {
    case 0:
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(g_UnknownGlobal56e26c->field_0x2da6, "scn", 0);
        g_UnknownGlobal56e26c->field_0x55c = (TrackGameViewOwner*)(new(__FILE__, 252) BaseQuarryEvent(1))
            ->UnknownFunction4de3b0(g_UnknownGlobal56e26c->field_0x10, UnknownFunction45cb20);
        if (!g_UnknownGlobal56e26c->field_0x34->UnknownFunction469190(g_UnknownGlobal56e26c->field_0x55c, -1))
            return 0;
        break;
    case 2:
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(g_UnknownGlobal56e26c->field_0x2da6, "scn", 0);
        g_UnknownGlobal56e26c->field_0x564 = (TrackGameViewOwner*)(new(__FILE__, 270) NationalRace(1))
            ->UnknownFunction4aa850(g_UnknownGlobal56e26c->field_0x10, UnknownFunction45cb20);
        if (!g_UnknownGlobal56e26c->field_0x34->UnknownFunction469190(g_UnknownGlobal56e26c->field_0x564, -1))
            return 0;
        break;
    }
    return UnknownFunction45d2b0() != 0;
}
