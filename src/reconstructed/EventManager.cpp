#include "EventManager.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Camera.h"
#include "DebugAlloc.h"
#include "DirectPlayMessages.h"
#include "ControlInterface.h"
#include "QuarryEvent.h"
#include "TrackGame.h"

extern "C" __declspec(dllimport) int __stdcall GetDateFormatA(unsigned long locale, unsigned long flags,
                                                              const void* date, const char* format,
                                                              char* buffer, int size);
extern "C" __declspec(dllimport) int __stdcall GetTimeFormatA(unsigned long locale, unsigned long flags,
                                                              const void* time, const char* format,
                                                              char* buffer, int size);

// EventManager.cpp's per-file vector set (0x0059af28, 0x0059af38,
// 0x0059af48 and 0x0059af18), initialised by 0x0045fe40..0x0045ff7b; the
// podium scene 0x0045d480 reads the y and z axes. Not const: the podium
// passes the y axis to D3DRMVectorRotate (LPD3DVECTOR).
static Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Characters kept in recording file names.
#define FILE_NAME_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-()$#"

// Network messages handled by slot 24 (the layout depends on the type).
// For DPSYS_DESTROYPLAYERORGROUP the fields are DPMSG_DESTROYPLAYERORGROUP's
// dwType, dwPlayerType and dpId.
struct UnknownEventPlayerMessage {
    int field_0x00;
    int field_0x04;                                // player (types 0xcc and 0x8e)
    int field_0x08;                                // dpId (DPSYS_DESTROYPLAYERORGROUP)
};
struct UnknownEventRacerMessage {                  // type 0x86
    int field_0x00;
    char field_0x04;                               // the sender's racer index
    char field_0x05;                               // finished
    float field_0x08;
    float field_0x0c;
    float field_0x10;                              // racer +0x750
    int field_0x14;
    int field_0x18;
    float field_0x1c;
    float field_0x20;
};

// 0x0045c830
UnknownEventEntry::UnknownEventEntry() {
    Reset();
}

// 0x0045c840: an empty entry in position 1.
void UnknownEventEntry::Reset() {
    field_0x00 = 0;
    field_0x04 = 1;
    field_0x08 = 0;
    field_0x14 = 0;
    field_0x28 = 0;
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0x20 = 0;
    strcpy(field_0x40, "");
    field_0x24 = 0;
    field_0x34[0] = 0;
    field_0x34[1] = 0;
    field_0x34[2] = 0;
}

// 0x0045c8b0: the racer's results. A racer with +0x4a0 set is listed in
// position 99 with nothing else; in modes 0 and 4 every racer counts as
// finished.
void UnknownEventEntry::CopyFromRacer(UnknownEventRacer* racer) {
    field_0x00 = racer->field_0x11bc;
    int length = strlen(racer->field_0x5e0);
    int count = length > 15 ? 15 : length;
    strncpy(field_0x40, racer->field_0x5e0, count);
    field_0x40[count] = 0;
    field_0x24 = racer->field_0x4a0;
    if (!field_0x24) {
        field_0x2c = racer->field_0x768;
        field_0x04 = racer->field_0x784;
        field_0x08 = racer->field_0x750;
        field_0x14 = racer->field_0x754;
        field_0x0c = racer->field_0x788;
        field_0x10 = racer->field_0x758;
        field_0x18 = racer->field_0x760;
        field_0x1c = racer->field_0x764;
        field_0x30 = racer->field_0x11c0;
        field_0x34[0] = racer->field_0x7ac[0];
        field_0x34[1] = racer->field_0x7ac[1];
        field_0x34[2] = racer->field_0x7ac[2];
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 0 || g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 4)
            field_0x20 = 1;
        else
            field_0x20 = racer->field_0x7a4;
    } else {
        field_0x2c = 0;
        field_0x04 = 99;
        field_0x08 = 0;
        field_0x14 = 0;
        field_0x0c = 0;
        field_0x10 = 0;
        field_0x18 = 0;
        field_0x1c = 0;
        field_0x30 = 0;
        field_0x34[0] = 0;
        field_0x34[1] = 0;
        field_0x34[2] = 0;
        field_0x20 = 0;
    }
}

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
    keepAliveTimeout = 20.0f;
    keepAliveInterval = 1.0f;
    field_0x440 = 0;
    podiumCamera = 0;
    podiumObject = 0;
    field_0x3c4 = Vector3(-1000.0f, -1000.0f, -1000.0f);
}

// 0x0045cae0
EventManager::~EventManager() {}

// 0x0045caf0
GameObject* EventManager::UnknownVirtualSlot8(void* value) {
    GameObject::UnknownVirtualSlot8(value);
    keepAliveTimeout = (float)g_UnknownGlobal56e26c->UnknownVirtualSlot20("KeepAliveTimeout", 20);
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
    TextQueueOverlay* target = UnknownFunction45d340();
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
TextQueueOverlay* EventManager::UnknownFunction45d340() {
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
int EventManager::HasRaceMode() {
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
// the block timer, ticks the podium characters and pans the camera; after 7
// seconds it lifts the block.
int EventManager::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x3c) {
        EndNetworkGame();
        field_0x3c = 0;
    }
    if (field_0x34)
        WaitForRemoteRacers(frameTime);
    else if (!g_UnknownGlobal56e26c->field_0x3428 && !g_UnknownGlobal56e26c->uiInteractionBlocked &&
             HasRaceMode())
        UnknownFunction45eef0(frameTime);
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        g_UnknownGlobal56e26c->field_0x3434 += frameTime;
        for (int i = 0; i < podiumCharacterCount; i++)
            podiumCharacters[i]->CharacterVirtualSlot7(frameTime, 0, 0);
        podiumCameraPosition += frameTime * podiumPanSpeed * (1.0f / 7);
        podiumCamera->UnknownFunction42e9b0(&podiumCameraPosition, 0, 0, 0, 0);
        podiumCamera->UnknownVirtualSlot29(field_0x3d8);
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
        field_0x50[i].Reset();
    field_0x48 = 0;
    g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c = 0;
}

// 0x0045e550
void EventManager::WaitForRemoteRacers(float) {
    if (!field_0x34)
        return;
    int ready = 1;
    int local = g_UnknownGlobal56e26c->field_0x08->localPlayer;
    for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
        int player = g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4;
        if (player != local) {
            NetPlayer* connected = g_UnknownGlobal56e26c->field_0x08->FindPlayer(player);
            if (!g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc && connected)
                ready = 0;
        }
    }
    if (ready) {
        g_UnknownGlobal56e26c->field_0x08->StopKeepAlive();
        field_0x34 = 0;
        UnknownFunction45e600();
        if (g_UnknownGlobal56e26c->networkGameObject)
            g_UnknownGlobal56e26c->networkGameObject->UnknownFunction49c770();
    }
}

// 0x0045e930
int CompareRankings(const void* a, const void* b) {
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
void EventManager::AddChampionshipPoints(UnknownEventRacer* racer, int* points) {
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
int CompareStandings(const void* a, const void* b) {
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
        if (CreatePodiumScene()) {
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
            (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 2 || field_0x48 > g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c))
            g_UnknownGlobal56e26c->field_0x08->SetSessionJoinable(1);
        field_0x34 = 0;
        UnknownFunction45cdc0(2);
        UnknownFunction45e710(g_UnknownGlobal56e26c->field_0x18 == 1 ? 0x88e : 0x868);
    }
}

// 0x0045f490: network messages. DPSYS_DESTROYPLAYERORGROUP and 0x89 mark a
// player done (and in mode 2 without a race-mode object remove it), 0x86
// updates a remote racer, 0xcc reports a player leaving (NetProcs sends it
// for a keep-alive timeout) and 0x8e the host ending the event.
int EventManager::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags))
        return 1;
    UnknownEventPlayerMessage* message = (UnknownEventPlayerMessage*)data;
    char name[16];
    char text[128];
    char line[260];
    if (type == DPSYS_DESTROYPLAYERORGROUP) {
        for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
            if (g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4 == message->field_0x08)
                g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc = 1;
        }
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !UnknownFunction45d2b0())
            UnknownFunction45fbd0(message->field_0x08);
    } else if (from) {
        if (type == 0x89) {
            for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
                if (g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4 == from)
                    g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc = 1;
            }
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !UnknownFunction45d2b0())
                UnknownFunction45fbd0(from);
        } else if (type == 0x86) {
            UnknownEventRacerMessage* update = (UnknownEventRacerMessage*)data;
            UnknownKrustyBikeView* view = UnknownFunction45d2f0();
            if (!view || g_UnknownGlobal56e26c->uiInteractionBlocked)
                return 0;
            for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
                if (g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4 == from &&
                    g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd8 == update->field_0x04) {
                    g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc = 1;
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
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 4) {
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
            TextQueueOverlay* target = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d340();
            if (target) {
                g_UnknownGlobal56e26c->UnknownFunction521970(0x13d7, text, sizeof(text));
                if (g_UnknownGlobal56e26c->field_0x08->GetPlayerName(message->field_0x04, name)) {
                    sprintf(line, "%s %s", name, text);
                    UnknownMessage notice(line, 3.25f);
                    target->UnknownFunction51b540(&notice);
                }
            }
            if (message->field_0x04 == g_UnknownGlobal56e26c->field_0x08->localPlayer) {
                g_UnknownGlobal56e26c->field_0x08->StopKeepAlive();
                field_0x3c = 1;
            } else {
                for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
                    if (message->field_0x04 == g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xd4) {
                        g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xc8 = 1;
                        g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc = 1;
                    }
                }
            }
            UnknownKrustyBikeView* view = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
            if (view)
                view->UnknownFunction420590(message->field_0x04);
        } else if (type == 0x8e) {
            if (message->field_0x04 == g_UnknownGlobal56e26c->field_0x08->localPlayer) {
                if (UnknownFunction45d2b0()) {
                    TextQueueOverlay* target = UnknownFunction45d340();
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
int CompareUnsigned(const void* a, const void* b) {
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
    for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x1be0; i++) {
        if (field_0x50[i].field_0x00 == player) {
            if (!field_0x50[i].field_0x30)
                g_UnknownGlobal56e26c->field_0x18--;
            else
                g_UnknownGlobal56e26c->field_0x3424--;
            field_0x50[i] = field_0x50[g_UnknownGlobal56e26c->mode.field_0x1be0 - 1];
            if (UnknownFunction45d2b0())
                g_UnknownGlobal56e26c->mode.field_0x1be4[i] =
                    g_UnknownGlobal56e26c->mode.field_0x1be4[g_UnknownGlobal56e26c->mode.field_0x1be0 - 1];
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35--;
        }
    }
    if (g_UnknownGlobal56e26c->mode.field_0x1be0)
        qsort(field_0x50, g_UnknownGlobal56e26c->mode.field_0x1be0 - 1, sizeof(UnknownEventEntry), CompareUnsigned);
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
    g_UnknownGlobal56e26c->field_0x08->Send(
        0x86, &message, sizeof(message), g_UnknownGlobal56e26c->field_0x08->localPlayer, 0);
    for (int i = 0; i < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24; i++) {
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
        g_UnknownGlobal56e26c->field_0x08->Send(
            0x86, &message, sizeof(message), g_UnknownGlobal56e26c->field_0x08->localPlayer, 0);
    }
    field_0x38 = 1;
    g_UnknownGlobal56e26c->field_0x08->StartKeepAlive(keepAliveTimeout, keepAliveInterval);
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
        g_UnknownGlobal56e26c->mode.field_0x1be4[i].field_0xcc = 0;
    podiumCharacterCount = 0;
    for (int j = 0; j < 3; j++)
        podiumCharacters[j] = 0;
    podiumCamera = 0;
    podiumObject = 0;
    g_UnknownGlobal56e26c->ui->UnknownFunction49b530();
    g_UnknownGlobal56e26c->mode.UnknownFunction523a60(g_UnknownGlobal56e26c->mode.field_0x6a0,
                                                      g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "env", path);
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(path);
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 0:
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "scn", 0);
        g_UnknownGlobal56e26c->field_0x55c = (TrackGameViewOwner*)(new(__FILE__, 252) BaseQuarryEvent(1))
            ->Create(g_UnknownGlobal56e26c->field_0x10, LoadProgressCallback);
        if (!g_UnknownGlobal56e26c->field_0x34->UnknownFunction469190(g_UnknownGlobal56e26c->field_0x55c, -1))
            return 0;
        break;
    case 2:
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9e30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "scn", 0);
        g_UnknownGlobal56e26c->field_0x564 = (TrackGameViewOwner*)(new(__FILE__, 270) NationalRace(1))
            ->Create(g_UnknownGlobal56e26c->field_0x10, LoadProgressCallback);
        if (!g_UnknownGlobal56e26c->field_0x34->UnknownFunction469190(g_UnknownGlobal56e26c->field_0x564, -1))
            return 0;
        break;
    }
    return UnknownFunction45d2b0() != 0;
}

// 0x0045eef0: decides when the race is over (the frame time argument is
// unused; Game+0x2f0 is read instead). After 315 seconds without an open
// menu item it always ends; otherwise by mode (TrackGame+0x2d74).
void EventManager::UnknownFunction45eef0(float frameTime) {
    if (!g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485df0())
        g_UnknownGlobal59af54 += g_UnknownGlobal56e26c->field_0x2f0;
    if (g_UnknownGlobal59af54 >= 315.0f) {
        UnknownFunction45f9a0();
        UnknownFunction45e600();
    }
    TrackGameViewOwner* owner = UnknownFunction45d2b0();
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 0 || g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4 || !owner ||
        !owner->field_0x25_bit0)
        return;
    UnknownKrustyBikeView* view = UnknownFunction45d2f0();
    if (!view || !view->field_0x18a)
        return;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 1:
    case 2:
    case 3:
    case 5:
        if (g_UnknownGlobal56e26c->field_0x08) {
            field_0x40 = 0;
            field_0x44 = 1;
            int iterator = 0;
            UnknownEventRacer* racer;
            while ((racer = view->UnknownFunction4204e0(&iterator)) != 0) {
                if (racer->field_0x7a4)
                    field_0x40 = 1;
                else if (!racer->field_0x4a0)
                    field_0x44 = 0;
            }
            if (field_0x40)
                field_0x3c0 += g_UnknownGlobal56e26c->field_0x2f0;
            // VC6 merges this call pair with the one after the switch.
            float limit = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x18 ? 30.0f : 120.0f;
            if (field_0x44 || limit < field_0x3c0) {
                UnknownFunction45f9a0();
                UnknownFunction45e600();
            }
            return;
        } else {
            UnknownEventRacer* racer = view->field_0x38;
            if (!racer->field_0x7a4)
                return;
            if (racer->field_0x784 == 1) {
                if (racer->field_0x7a4 >= 3)
                    UnknownFunction45e600();
                return;
            }
            if (racer->field_0x7a4 >= 3 && UnknownFunction4bfa80() - racer->field_0x748 >= 3000)
                UnknownFunction45e600();
            return;
        }
        break;
    case 0:
        if (g_UnknownGlobal56e26c->field_0x55c->field_0x70 < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140)
            return;
        break;
    case 4:
        if (g_UnknownGlobal56e26c->field_0x568->field_0x70 < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140)
            return;
        if (owner->field_0xa8 == owner->field_0x34->field_0x38) {
            owner->field_0xa8->field_0x764 += owner->field_0xa8->field_0x75c;
            // Compared through locals, kept as the best of +0x75c.
            float best = owner->field_0xa8->field_0x760;
            float lap = owner->field_0xa8->field_0x75c;
            owner->field_0xa8->field_0x760 = best > lap ? owner->field_0xa8->field_0x760
                                                        : owner->field_0xa8->field_0x75c;
            owner->field_0xa8->field_0x75c = 0;
        }
        break;
    default:
        return;
    }
    UnknownFunction45f9a0();
    UnknownFunction45e600();
}

// 0x0045e9d0: ranks the racers by mode, copies them into the entries and
// awards points; in mode 2 it also orders the standings and copies each
// racer's place to its player's entry.
void EventManager::UnknownFunction45e9d0() {
    UnknownKrustyBikeView* view = UnknownFunction45d2f0();
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00) { // a one-case switch: `sub eax, 2; jne`
    case 2:
        field_0x48++;
    }
    int count = 0;
    int iterator = 0;
    UnknownEventRacer* racer;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 1:
    case 2:
    case 3:
    case 5: {
        if (g_UnknownGlobal56e26c->field_0x18 > 1) {
            UnknownEventRanking rankings[11];
            int ranked = 0;
            int next = 0;
            while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
                if (!racer->field_0x4a0) {
                    rankings[ranked].value = racer->field_0x754;
                    rankings[ranked].racer = racer;
                    ranked++;
                }
            }
            qsort(rankings, ranked, sizeof(rankings[0]), CompareRankings);
            for (int i = 0; i < ranked; i++)
                rankings[i].racer->field_0x784 = i + 1;
        }
        iterator = 0;
        for (racer = view->UnknownFunction4204e0(&iterator); racer; racer = view->UnknownFunction4204e0(&iterator)) {
            field_0x50[count].CopyFromRacer(racer);
            if ((g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 || g_UnknownGlobal56e26c->field_0x3444) &&
                !racer->field_0x4a0)
                AddChampionshipPoints(racer, &field_0x50[count].field_0x28);
            count++;
        }
        break;
    }
    case 0: {
        UnknownEventScore scores[11];
        int scored = 0;
        int next = 0;
        while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
            if (!racer->field_0x4a0) {
                scores[scored].value = racer->field_0x768;
                scores[scored].racer = racer;
                scored++;
            }
        }
        qsort(scores, scored, sizeof(scores[0]), UnknownFunction5199f0);
        for (int i = 0; i < scored; i++)
            scores[i].racer->field_0x784 = i + 1;
        next = 0;
        for (racer = view->UnknownFunction4204e0(&next); racer; racer = view->UnknownFunction4204e0(&next)) {
            field_0x50[count].CopyFromRacer(racer);
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !racer->field_0x4a0)
                AddChampionshipPoints(racer, &field_0x50[count].field_0x28);
            count++;
        }
        break;
    }
    case 4:
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x148) {
            UnknownEventScore scores[11];
            int scored = 0;
            int next = 0;
            while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
                if (!racer->field_0x4a0) {
                    scores[scored].value = racer->field_0x768;
                    scores[scored].racer = racer;
                    scored++;
                }
            }
            qsort(scores, scored, sizeof(scores[0]), UnknownFunction5199f0);
            for (int i = 0; i < scored; i++)
                scores[i].racer->field_0x784 = i + 1;
            next = 0;
            for (racer = view->UnknownFunction4204e0(&next); racer; racer = view->UnknownFunction4204e0(&next)) {
                field_0x50[count].CopyFromRacer(racer);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !racer->field_0x4a0)
                    AddChampionshipPoints(racer, &field_0x50[count].field_0x28);
                count++;
            }
        } else {
            UnknownEventScore scores[11];
            int scored = 0;
            int next = 0;
            while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
                if (!racer->field_0x4a0) {
                    scores[scored].value = racer->field_0x764;
                    scores[scored].racer = racer;
                    scored++;
                }
            }
            qsort(scores, scored, sizeof(scores[0]), UnknownFunction5199f0);
            for (int i = 0; i < scored; i++)
                scores[i].racer->field_0x784 = i + 1;
            next = 0;
            for (racer = view->UnknownFunction4204e0(&next); racer; racer = view->UnknownFunction4204e0(&next)) {
                field_0x50[count].CopyFromRacer(racer);
                if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && !racer->field_0x4a0)
                    AddChampionshipPoints(racer, &field_0x50[count].field_0x28);
                count++;
            }
        }
        break;
    }
    field_0x4c = count;
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2) {
        UnknownEventStanding standings[11];
        int next = 0;
        int standing = 0;
        while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
            if (!racer->field_0x4a0) {
                standings[standing].field_0x00 = field_0x50[standing].field_0x28;
                standings[standing].racer = racer;
                standing++;
            }
        }
        qsort(standings, standing, sizeof(standings[0]), CompareStandings);
        next = 0;
        while ((racer = view->UnknownFunction4204e0(&next)) != 0) {
            int i;
            for (i = 0; i < standing; i++) {
                if (standings[i].racer == racer) {
                    racer->field_0x784 = i + 1;
                    break;
                }
            }
            for (i = 0; i < standing; i++) {
                if (field_0x50[i].field_0x00 == racer->field_0x11bc && field_0x50[i].field_0x30 == racer->field_0x11c0) {
                    field_0x50[i].field_0x04 = racer->field_0x784;
                    break;
                }
            }
        }
    }
}

// 0x0045cdc0: ends the race. With `mode` 1 (0x0045d480, before the podium
// scene) it only calls slot 4 on the race objects and racers. Otherwise it saves the replay (Record\\<scene>_<racer>_
// <date>_<time>.vcr, filtered to file-name characters) and, in mode 4, the
// ghost (.gho), then releases the first race-mode object.
void EventManager::UnknownFunction45cdc0(int mode) {
    char date[32];
    char scene[64];
    char name[260];
    char filtered[260];
    g_UnknownGlobal56e26c->ui->field_0x44 = 0;
    UnknownKrustyBikeView* view = UnknownFunction45d2f0();
    TrackGameViewOwner* owner = UnknownFunction45d2b0();
    g_UnknownGlobal56e26c->mode.UnknownFunction523580();
    if (mode == 1) {
        if (owner) {
            if (owner->field_0x60)
                owner->field_0x60->UnknownVirtualSlot4();
            if (owner->field_0x5c)
                owner->field_0x5c->UnknownVirtualSlot4();
            if (owner->field_0x68)
                owner->field_0x68->UnknownVirtualSlot4();
            if (owner->field_0x64)
                owner->field_0x64->UnknownVirtualSlot4();
        }
        view->field_0x50->UnknownVirtualSlot4();
        if (view->field_0x44)
            view->field_0x44->UnknownVirtualSlot4();
        if (view->field_0x60)
            view->field_0x60->UnknownVirtualSlot4();
        int iterator = 0;
        UnknownEventRacer* racer;
        while ((racer = view->UnknownFunction4204e0(&iterator)) != 0)
            racer->UnknownVirtualSlot4();
        return;
    }
    if (view) {
        if (g_UnknownGlobal56e26c->mode.field_0x2dbc && !g_UnknownGlobal56e26c->field_0x3428 &&
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4) {
            char description[32];
            char time[32];
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea390(
                scene, g_UnknownGlobal56e26c->sceneObject->field_0x24c, 0);
            GetDateFormatA(0, 0, 0, "yyyyMMdd", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "HHmm", time, sizeof(time));
            sprintf(name, "%s_%s_%s_%s", scene, view->field_0x38->field_0x5e0, date, time);
            int length = strlen(name);
            int count = 0;
            filtered[0] = 0;
            for (int i = 0; i < length; i++) {
                if (strchr(FILE_NAME_CHARACTERS, name[i]))
                    filtered[count++] = name[i];
            }
            filtered[count] = 0;
            sprintf(name, "Record\\%s.vcr", filtered);
            GetDateFormatA(0, 0, 0, "M/d/yyyy", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "h:mm tt", time, sizeof(time));
            sprintf(description, "%s %s", date, time);
            view->UnknownFunction420b00(name, description);
        }
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4 && g_UnknownGlobal56e26c->mode.field_0x26f0) {
            char time[32];
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea390(
                scene, g_UnknownGlobal56e26c->sceneObject->field_0x24c, 0);
            GetDateFormatA(0, 0, 0, "yyyyMMdd", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "HHmm", time, sizeof(time));
            sprintf(name, "%s_%s_%s_%s", scene, view->field_0x38->field_0x5e0, date, time);
            int length = strlen(name);
            int count = 0;
            filtered[0] = 0;
            for (int i = 0; i < length; i++) {
                if (strchr(FILE_NAME_CHARACTERS, name[i]))
                    filtered[count++] = name[i];
            }
            filtered[count] = 0;
            sprintf(name, "Record\\%s.gho", filtered);
            if (view->field_0x1a8)
                view->field_0x1a8->Save(name);
        }
    }
    if (g_UnknownGlobal56e26c->field_0x558) {
        g_UnknownGlobal56e26c->field_0x558->UnknownFunction4691f0();
        g_UnknownGlobal56e26c->field_0x558->Release();
        g_UnknownGlobal56e26c->field_0x558 = 0;
    } else if (g_UnknownGlobal56e26c->field_0x55c) {
        g_UnknownGlobal56e26c->field_0x55c->UnknownFunction4691f0();
        g_UnknownGlobal56e26c->field_0x55c->Release();
        g_UnknownGlobal56e26c->field_0x55c = 0;
    } else if (g_UnknownGlobal56e26c->field_0x560) {
        g_UnknownGlobal56e26c->field_0x560->UnknownFunction4691f0();
        g_UnknownGlobal56e26c->field_0x560->Release();
        g_UnknownGlobal56e26c->field_0x560 = 0;
    } else if (g_UnknownGlobal56e26c->field_0x564) {
        g_UnknownGlobal56e26c->field_0x564->UnknownFunction4691f0();
        g_UnknownGlobal56e26c->field_0x564->Release();
        g_UnknownGlobal56e26c->field_0x564 = 0;
    } else if (g_UnknownGlobal56e26c->field_0x568) {
        g_UnknownGlobal56e26c->field_0x568->UnknownFunction4691f0();
        g_UnknownGlobal56e26c->field_0x568->Release();
        g_UnknownGlobal56e26c->field_0x568 = 0;
    }
}

// The quadtree members 0x0045fce0 and 0x0045fdc0 use. Quadtree.cpp and
// CollisionObject.cpp are declared in full under src/krusty2/broadphase and
// src/krusty2/collision, whose header tree cannot be mixed with this one.
// RTTI: Vegetation : QuadTreeObject (vtable 0x005524f8) and
// CollisionObject : QuadTreeObject (+0), GraphicsTest (+0xc) : GameObject.
class QuadTree;
class QuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();
    // 0x00456720 for Vegetation: the object's quadtree code (ret 4).
    virtual unsigned int UnknownVirtualSlot1(QuadTree* tree);
    short field_0x04;
    short field_0x06;
    char field_0x08;
};
class Vegetation : public QuadTreeObject {};
class UnknownEventGraphicsTest : public GameObject {};
class CollisionObject : public QuadTreeObject, public UnknownEventGraphicsTest {};

class QuadTree {
public:
    void Remove(QuadTreeObject* object, unsigned int code);     // 0x004dcf20
    int BeginQuery(float x0, float z0, float x1, float z1);     // 0x004dd270
    QuadTreeObject* NextObject();                               // 0x004dd540
    void EndQuery();                                            // 0x004dd770
};
extern QuadTree* g_collisionQuadTree;                           // 0x0068aba4

// 0x0045fce0
void EventManager::RemoveVegetationInRect(float x0, float z0, float x1, float z1) {
    struct {
        Vegetation* object;
        unsigned int code;
    } found[1000];
    int count = 0;
    if (g_collisionQuadTree && g_collisionQuadTree->BeginQuery(x0, z0, x1, z1)) {
        QuadTreeObject* object;
        while ((object = g_collisionQuadTree->NextObject()) != 0) {
            if (count < 1000) {
                found[count].object = dynamic_cast<Vegetation*>(object);
                if (found[count].object) {
                    found[count].code = found[count].object->UnknownVirtualSlot1(g_collisionQuadTree);
                    count++;
                }
            }
        }
        g_collisionQuadTree->EndQuery();
        for (int i = 0; i < count; i++)
            g_collisionQuadTree->Remove(found[i].object, found[i].code);
    }
}

// 0x0045fdc0
void EventManager::UnknownFunction45fdc0(float x0, float z0, float x1, float z1) {
    if (g_collisionQuadTree && g_collisionQuadTree->BeginQuery(x0, z0, x1, z1)) {
        QuadTreeObject* object;
        while ((object = g_collisionQuadTree->NextObject()) != 0) {
            GameObject* graphics = dynamic_cast<CollisionObject*>(object);
            if (graphics)
                graphics->UnknownVirtualSlot4();
        }
        g_collisionQuadTree->EndQuery();
    }
}
