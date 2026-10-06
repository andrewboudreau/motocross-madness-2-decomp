#include "BikeRace.h"

#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"

// A truncating copy into a `size`-byte buffer (as DlgProcs.cpp); retail
// evaluates `source` once for the length and again for the copy.
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The four vector constants of src/krusty2/math/Math3D.h: 0x00578e60,
// 0x00578e70, 0x00578e80 and 0x00578e50, initialised by
// 0x0041cdf0..0x0041cf2b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x00417b00
void UnknownFunction417b00(CollisionObject* self, CollisionObject* other) {
    BikeRace* race = (BikeRace*)g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    if (race != 0 && race->field_0x0b4 != 0) {
        race->field_0x0b4->field_0x2c = (int)other;
    }
}

// 0x00417b30
void UnknownFunction417b30(int* keys, int* values, int count) {
    int swapped = 1;
    for (int pass = 0; pass <= count && swapped; pass++) {
        swapped = 0;
        for (int i = 0; i < count - 1 - pass; i++) {
            if (keys[i] < keys[i + 1]) {
                int key = keys[i];
                keys[i] = keys[i + 1];
                keys[i + 1] = key;
                int value = values[i];
                values[i] = values[i + 1];
                swapped = 1;
                values[i + 1] = value;
            }
        }
    }
}

// 0x00417bc0
void BikeRace::UnknownFunction417bc0(int* order) {
    int points[11];
    for (int i = 0; i < field_0x158; i++) {
        points[i] = g_UnknownGlobal56e26c->eventManager->field_0x50[i].field_0x28;
        order[i] = i + 1;
    }
    UnknownFunction417b30(points, order, field_0x158);
}

// 0x00417c30
BikeRace::BikeRace(int flags) : GraphicsTest(flags) {
    field_0x0cc.field_0x38 = 0;
    field_0x108.field_0x38 = 0;
    field_0x1e8 = 0;
    field_0x078 = 0;
    field_0x038 = 0;
    field_0x03c = 0;
    field_0x040 = 0;
    field_0x044 = 0;
    field_0x04c = 0;
    field_0x050 = 0;
    field_0x054 = 0;
    field_0x058 = 0;
    field_0x05c = 0;
    field_0x060 = 0;
    field_0x0bc = 0;
    field_0x0c0 = 0;
    field_0x048 = 0;
    field_0x0c8 = 0;
    field_0x14c = 0;
    field_0x150 = 0;
    field_0x154 = 0;
    field_0x15c = 0;
    field_0x18a = false;
    field_0x18b = false;
    field_0x18d = false;
    field_0x158 = 0;
    field_0x188 = false;
    field_0x18f = false;
    if (g_UnknownGlobal56e26c->field_0x18 > 1) {
        field_0x18e = true;
    } else {
        field_0x18e = false;
    }
    field_0x190 = false;
    field_0x194 = 0;
    field_0x198 = g_UnknownGlobal56e26c->mode.field_0x6b4;
    field_0x064 = 0;
    field_0x06c = 0;
    field_0x074 = 0;
    field_0x068 = -1;
    field_0x070 = -1;
    field_0x189 = 0;
    field_0x160 = 0;
    field_0x1ac = 0;
    field_0x1b0 = 0;
    field_0x1b4 = 0;
    field_0x1bc = 0;
    field_0x1b8 = 0;
    field_0x1d8 = 0;
    field_0x1d4 = 0;
    field_0x1c8 = FLT_MAX;
    field_0x1a0 = 0;
    field_0x1a4 = 0;
    field_0x1a8 = 0;
    g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
    field_0x3f9 = false;
    field_0x1cc = (float)g_UnknownGlobal56e26c->UnknownVirtualSlot20("VCRGhostTimeLimit", 300);
    field_0x1d0 = (float)g_UnknownGlobal56e26c->UnknownVirtualSlot20("VCRRecordTimeLimit", 3600);
    field_0x1c0 = 0;
    field_0x3f8 = false;
    field_0x3f9 = false;
    field_0x1dc = 4;
    field_0x1e0 = 4;
    field_0x1e4 = 0;
    field_0x1c4 = -1.0f;
    field_0x3fa = false;
    field_0x3fb = false;
    field_0x3fc = 0;
    field_0x1f0[0] = 0;
    field_0x2f4[0] = 0;
    g_UnknownGlobal56e26c->field_0x1c4 = 0;
    field_0x144 = 0;
    field_0x148 = 0;
    field_0x19c = 0;
    field_0x18c = g_UnknownGlobal56e26c->UnknownVirtualSlot22("ForceHighLOD", 0);
    field_0x3fd = 1;
    field_0x0a8 = -1;
    field_0x0b0 = 0xff;
    field_0x0ac = 0;
    field_0x0b4 = 0;
    field_0x034 = 0;
}

// 0x0041cf30
BikeRace::~BikeRace() {
    if (field_0x03c != 0) {
        delete field_0x03c;
        field_0x03c = 0;
    }
    if (field_0x040 != 0) {
        if (g_UnknownGlobal56e26c->UnknownFunction521cd0()) {
            operator delete(field_0x040, __FILE__, 0x809);
        } else {
            delete field_0x040;
        }
        field_0x040 = 0;
    }
    Track* track = field_0x048;
    if (track != 0) {
        if (track->field_0x00 != 0) {
            track->UnknownFunction516870(&track->field_0x00);
        }
        delete track;
    }
    UnknownFunctionFreeNodes();
    if (field_0x0c4 != 0) {
        UnknownFunction4e5f10(field_0x0c4);
        field_0x0c4 = 0;
    }
    if (field_0x1a8 != 0) {
        delete field_0x1a8;
        field_0x1a8 = 0;
        if (g_UnknownGlobal56e26c->field_0x3428) {
            UnknownFunctionCameraView()->field_0x244 = field_0x1ec;
            field_0x050->UnknownVirtualSlot61();
        }
        g_UnknownGlobal56e26c->field_0x3428 = 0;
        g_UnknownGlobal56e26c->field_0x342c = 0;
    }
    while (field_0x0c8 != 0) {
        UnknownBikeRaceNode* node = field_0x0c8;
        field_0x0c8 = node->field_0x38;
        operator delete(node, __FILE__, 0x82e);
    }
}

// 0x0041d260
int BikeRace::UnknownVirtualSlot16(int value) {
    GameObject::UnknownVirtualSlot16(value);
    JoystickDevice* joystick = g_UnknownGlobal56e26c->field_0x14->activeJoystick;
    if (joystick != 0 && g_UnknownGlobal56e26c->field_0x1000) {
        joystick->UnknownVirtualSlot18(value);
    }
    return 1;
}

// 0x0041d0d0
int BikeRace::UnknownFunction41d0d0(int index, UnknownBikeRaceNodeSource* source,
                                    UnknownBikeRaceNode*** tail, float scale, int value) {
    UnknownBikeRaceNode* node =
        (UnknownBikeRaceNode*)DebugCalloc(1, sizeof(UnknownBikeRaceNode), __FILE__, 0x840);
    if (node == 0) {
        return 0;
    }
    UnknownBikeRaceNodeSource* record = &source[index];
    node->field_0x00.x = record->field_0x00.x;
    node->field_0x00.y = record->field_0x00.y;
    node->field_0x00.z = record->field_0x00.z;
    node->field_0x0c.x = record->field_0x0c.x;
    node->field_0x0c.y = record->field_0x0c.y;
    node->field_0x0c.z = record->field_0x0c.z;
    node->field_0x24 = scale;
    node->field_0x28 = value;
    node->field_0x1c = 0;
    node->field_0x18 = scale * node->field_0x0c.z * 0.5f;
    node->field_0x20 = scale * node->field_0x0c.x * -0.5f;
    **tail = node;
    *tail = &node->field_0x38;
    return 1;
}

// 0x0041d170
int BikeRace::UnknownFunction41d170(UnknownBikeRaceNodeOwner* owner) {
    int count = owner->field_0x0ac;
    UnknownBikeRaceNodeSource* sources = owner->field_0x0d8;
    UnknownBikeRaceNode** tail = &field_0x0c8;
    for (int i = 0; i < count; i++) {
        UnknownBikeRaceNodeEntry* entry = owner->field_0x420[i];
        if (!UnknownFunction41d0d0(i, sources, &tail, entry->field_0x50,
                                   entry->field_0x54)) {
            return 0;
        }
    }
    return 1;
}

// 0x0041d1e0
int BikeRace::UnknownFunction41d1e0() {
    if (field_0x048 != 0 && field_0x048->UnknownFunction517da0(field_0x108.field_0x2c, field_0x0cc.field_0x2c) > 0.0f) {
        field_0x189 = 0;
    } else {
        field_0x189 = 1;
    }
    return field_0x189;
}

// 0x0041d2a0
void BikeRace::UnknownFunction41d2a0(float time) {
    field_0x058->UnknownFunction4eb000(time);
}

// 0x0041ea10
int BikeRace::UnknownFunction41ea10() {
    if (g_UnknownGlobal56e26c->field_0x18 > 1) {
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
            if (field_0x03c[i]->UnknownFunction495c00()) {
                return 1;
            }
        }
    }
    return 0;
}

// 0x0041ea60
void BikeRace::UnknownFunction41ea60(UnknownBikeRaceCharacter* character, int from, int to) {
    for (int i = 0; i < character->field_0x1a0->field_0x274; i++) {
        for (int j = 0; j < character->field_0x1a0->field_0x28c[i].field_0x00; j++) {
            for (int k = 0; k < character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x20; k++) {
                if (character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x24[k] == from) {
                    character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x24[k] = to;
                }
            }
        }
    }
}

// 0x0041f550
void BikeRace::UnknownFunction41f550(int value) {
    if (value) {
        if (field_0x3f8) {
            return;
        }
        field_0x3f8 = true;
    } else {
        field_0x3f8 = !field_0x3f8;
    }
    g_UnknownGlobal56e26c->field_0x1c4 = field_0x3f8;
}

// 0x0041f590
int BikeRace::UnknownVirtualSlot20(int value) {
    if (field_0x190 && field_0x19c != 0 && field_0x19c->UnknownFunction51da30(value, &value)) {
        if (value == 0) {
            field_0x194 = 10.0f;
        }
        return 1;
    }
    return 0;
}

// 0x00420040
int BikeRace::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    int count;
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags)) {
        return 1;
    }
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        return 1;
    }
    if (from == 0) {
        return 0;
    }
    if (type == 1) {
        UnknownBikeRaceNetState* state = (UnknownBikeRaceNetState*)data;
        int self = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
        count = g_UnknownGlobal56e26c->field_0x2158;
        for (int i = 0; i < count; i++) {
            int id = g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = field_0x03c[i];
                if (racer != 0 && racer->field_0x734 == state->field_0x54) {
                    field_0x03c[i]->field_0x11b8 = 1;
                    UnknownBikeRaceRacerPart* oldest = field_0x03c[i]->field_0x11c8[2];
                    field_0x03c[i]->field_0x11c8[2] = field_0x03c[i]->field_0x11c8[1];
                    field_0x03c[i]->field_0x11c8[1] = field_0x03c[i]->field_0x11c8[0];
                    field_0x03c[i]->field_0x11c8[0] = oldest;
                    field_0x03c[i]->field_0x11c8[0]->field_0x58 = flags;
                    field_0x03c[i]->field_0x11c8[0]->field_0x00 = *state;
                    field_0x03c[i]->field_0x11c8[0]->field_0x5c = 1;
                    field_0x03c[i]->field_0x1358 = state->field_0x3c;
                    field_0x03c[i]->field_0x1364 = state->field_0x08;
                    field_0x03c[i]->field_0x1370 = state->field_0x30;
                    field_0x03c[i]->field_0x137c = state->field_0x14;
                    field_0x03c[i]->field_0x1380 = state->field_0x18;
                    field_0x03c[i]->field_0x1384 = state->field_0x1c;
                    field_0x03c[i]->field_0x1388 = field_0x1dc == 11 ? flags : state->field_0x4c;
                    return 1;
                }
            }
        }
    } else if (type == 13) {
        UnknownBikeRaceNetMessage13* message = (UnknownBikeRaceNetMessage13*)data;
        int self = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
            int id = g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4;
            if (id != self && id == from && field_0x03c[i] != 0 &&
                field_0x03c[i]->field_0x734 == message->field_0x16) {
                if (!field_0x03c[i]->field_0x11b8) {
                    return 1;
                }
                UnknownBikeRaceRacerPart* oldest = field_0x03c[i]->field_0x11c8[2];
                field_0x03c[i]->field_0x11c8[2] = field_0x03c[i]->field_0x11c8[1];
                field_0x03c[i]->field_0x11c8[1] = field_0x03c[i]->field_0x11c8[0];
                field_0x03c[i]->field_0x11c8[0] = oldest;
                field_0x03c[i]->field_0x11c8[0]->field_0x58 = flags;
                field_0x03c[i]->UnknownFunction4933e0(message, field_0x03c[i]->field_0x11c8[0]);
                field_0x03c[i]->field_0x11c8[0]->field_0x5c = 0;
                return 1;
            }
        }
    } else if (type == 10) {
        UnknownBikeRaceNetScore* score = (UnknownBikeRaceNetScore*)data;
        count = g_UnknownGlobal56e26c->field_0x2158;
        int self = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
        for (int i = 0; i < count; i++) {
            int id = g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = field_0x03c[i];
                if (racer != 0 && racer->field_0x734 == score->field_0x01) {
                    field_0x03c[i]->field_0x768 = score->field_0x04;
                    return 1;
                }
            }
        }
    } else if (type == 0x85) {
        char name[0x14];
        if (g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac720(from, name)) {
            if (g_UnknownGlobal56e26c->mode.field_0x6b4 != 2) {
                if (field_0x19c != 0) {
                    field_0x19c->UnknownFunction51dd10();
                }
                field_0x194 = 10.0f;
                field_0x190 = true;
            }
            if (field_0x19c != 0) {
                field_0x19c->UnknownFunction51dd70(name, &((UnknownBikeRaceNetStamp*)data)->field_0x04, 1);
            }
        }
        return 1;
    } else if (type == 0x84) {
        UnknownBikeRaceNetStamp* stamp = (UnknownBikeRaceNetStamp*)data;
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
            if (g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4 == from) {
                float delay = (float)(unsigned int)flags - (float)stamp->field_0x04;
                if (delay < g_UnknownGlobal56e26c->field_0x215c[i].field_0xd0) {
                    g_UnknownGlobal56e26c->field_0x215c[i].field_0xd0 = delay;
                }
                g_UnknownGlobal56e26c->field_0x215c[i].field_0xc8 = 1;
            }
        }
    }
    return 0;
}

// 0x004204e0
UnknownBikeRaceRacer* BikeRace::UnknownFunction4204e0(int* iterator) {
    UnknownBikeRaceRacer* racer = 0;
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        return 0;
    }
    int players = g_UnknownGlobal56e26c->field_0x18;
    if (players == 1 && *iterator == 0) {
        racer = field_0x038;
    } else {
        int index = *iterator;
        if (players > 1) {
            racer = field_0x03c[index];
        } else if (index >= players && field_0x040 != 0) {
            racer = field_0x040[index - players];
        }
    }
    (*iterator)++;
    if (g_UnknownGlobal56e26c->field_0x18 > 1) {
        if (*iterator > g_UnknownGlobal56e26c->field_0x2d98 + g_UnknownGlobal56e26c->field_0x18) {
            *iterator = 0;
            return 0;
        }
    } else {
        int count = g_UnknownGlobal56e26c->field_0x2d70 == 4 ? 1 : g_UnknownGlobal56e26c->field_0x2d94;
        if (*iterator > count + 1) {
            *iterator = 0;
            return 0;
        }
    }
    return racer;
}

// 0x00420590
void BikeRace::UnknownFunction420590(int player) {
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        return;
    }
    int self = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
    for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2158; i++) {
        int id = g_UnknownGlobal56e26c->field_0x215c[i].field_0xd4;
        if (id != self && id == player && field_0x03c[i] != 0) {
            field_0x03c[i]->UnknownRacerVirtualSlot44();
            TrackGameViewOwner* owner = g_UnknownGlobal56e26c->field_0x568;
            if (owner != 0 && owner->field_0xa8 == (UnknownEventRacer*)field_0x03c[i]) {
                if (g_UnknownGlobal56e26c->field_0x2eb4) {
                    owner->field_0xa8 = 0;
                } else if (g_UnknownGlobal56e26c->field_0x08->isHost) {
                    owner->UnknownFunction4a9d20();
                }
            }
        }
    }
}

// 0x00420650
void BikeRace::UnknownFunction420650(int mode, char* path, char* description) {
    if (g_UnknownGlobal56e26c->field_0x08 != 0) {
        return;
    }
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
        return;
    }
    field_0x18b = true;
    field_0x188 = false;
    UnknownFunction421050();
    int iterator = 0;
    Vector3 position;
    Vector3 direction;
    if (field_0x064 != 0 && g_UnknownGlobal56e26c->field_0x2d70 != 0 &&
        g_UnknownGlobal56e26c->field_0x2d70 != 4) {
        field_0x064->UnknownVirtualSlot5();
    }
    int positions[11];
    int order[11];
    if (g_UnknownGlobal56e26c->field_0x2d70 == 2) {
        UnknownFunction417bc0(order);
        for (int i = 0; i < field_0x158; i++) {
            for (int j = 0; j < field_0x158; j++) {
                if (order[j] == i + 1) {
                    positions[i] = j + 1;
                    break;
                }
            }
        }
    } else {
        for (int i = 0; i < field_0x158; i++) {
            positions[i] = i + 1;
        }
    }
    if (g_UnknownGlobal56e26c->field_0x342c) {
        UnknownFunction420b00(path, description);
        field_0x1b8 = 0;
        field_0x1d4 = 0;
        field_0x1bc = 0;
        field_0x1d8 = 0;
        field_0x1dc = 4;
        field_0x3f8 = false;
        field_0x3f9 = false;
        field_0x3fc = mode;
        g_UnknownGlobal56e26c->field_0x2e0 = 1.0f;
        g_UnknownGlobal56e26c->field_0x1c4 = 0;
        field_0x044->field_0x3c->UnknownVirtualSlot16(0);
        if (field_0x044->field_0x40 != 0) {
            field_0x044->field_0x40->UnknownVirtualSlot16(0);
        }
        field_0x044->field_0x44->UnknownVirtualSlot16(0);
        field_0x1a0 = new (__FILE__, 0x1068) KrustyVCR;
        if (field_0x1a0 != 0) {
            if (mode == 0) {
                g_UnknownGlobal56e26c->field_0x3428 = 1;
                if (!field_0x1a0->UnknownFunction49bf10(UnknownFunction4230e0, 1, "VCRtape.dat",
                                                        field_0x1a8)) {
                    delete field_0x1a0;
                    field_0x1a0 = 0;
                }
                g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(1);
                g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(
                    new (__FILE__, 0x1073) VCRDlg(1, "VCR.dtm"), 0, 1, 0, 0, 0, 0, 1);
            } else {
                g_UnknownGlobal56e26c->field_0x3428 = 0;
                if (!field_0x1a0->UnknownFunction49bf10(UnknownFunction4230e0, 0, "VCRtape.dat",
                                                        field_0x1a8)) {
                    delete field_0x1a0;
                    field_0x1a0 = 0;
                }
            }
            if (field_0x1a0 != 0) {
                UnknownFunction469190(field_0x1a0, -1);
            }
        }
    }
    if (field_0x074 != 0) {
        UnknownFunction41ea60(field_0x074, 1, 2);
        field_0x074->UnknownFunction4a8b10("Stand");
        field_0x078 = 0;
    }
    UnknownFunction41d2a0(0.0001f);
    field_0x058->field_0x7c0 = 1;
    for (UnknownBikeRaceRacer* racer = UnknownFunction4204e0(&iterator); racer != 0;
         racer = UnknownFunction4204e0(&iterator)) {
        racer->UnknownRacerVirtualSlot43();
        racer->field_0x109 = true;
        UnknownFunction4210f0(&position, &direction, g_UnknownGlobal56e26c->field_0x560,
                              positions[iterator - 1]);
        racer->field_0x10c = position;
        racer->field_0x118 = direction;
        racer->UnknownFunction496e20(field_0x1a0);
        field_0x3f8 = false;
    }
    if (g_UnknownGlobal56e26c->field_0x568 != 0 &&
        g_UnknownGlobal56e26c->field_0x568->field_0xdc != 0) {
        g_UnknownGlobal56e26c->field_0x568->field_0xdc->UnknownFunction4a9d10(field_0x1a0);
    }
    if (g_UnknownGlobal56e26c->field_0x55c != 0) {
        g_UnknownGlobal56e26c->field_0x55c->UnknownFunction4e1f00();
    }
    if (g_UnknownGlobal56e26c->field_0x568 != 0) {
        g_UnknownGlobal56e26c->field_0x568->UnknownFunction4e1f00();
    }
    if (g_UnknownGlobal56e26c->field_0x560 != 0) {
        g_UnknownGlobal56e26c->field_0x560->UnknownFunction404df0(0, 1, UnknownFunctionCameraView()->field_0x3b4);
    }
}

// 0x00420b00
void BikeRace::UnknownFunction420b00(char* path, char* description) {
    g_UnknownGlobal56e26c->ui->field_0x40 = 0;
    if (field_0x1a0 != 0) {
        char text[0x80];
        g_UnknownGlobal56e26c->UnknownFunction521970(0x1437, text, sizeof(text));
        if (field_0x3fc) {
            field_0x1a0->UnknownFunction49c010(field_0x1bc, description ? description : text,
                                               path != 0);
        }
        if (path != 0) {
            CopyFileA("VCRtape.dat", path, 1);
        }
        field_0x1a0->UnknownFunction4691f0();
        field_0x1a0->Release();
        field_0x1a0 = 0;
        g_UnknownGlobal56e26c->ui->field_0x40 = 1;
    }
}

// 0x00420bd0
void BikeRace::UnknownFunction420bd0() {
    if (field_0x1a4 != 0) {
        field_0x1a4->UnknownFunction4691f0();
        field_0x1a4->Release();
        field_0x1a4 = 0;
    }
    if (field_0x1a0 == 0) {
        return;
    }
    if (field_0x038->field_0x7a0 == 1 && field_0x189) {
        field_0x1a0->UnknownFunction4691f0();
        field_0x1a0->Release();
        field_0x1bc = 0;
    } else {
        char date[0x20];
        char time[0x20];
        char description[0x20];
        GetDateFormatA(0, 0, 0, "M/d/yyyy", date, sizeof(date));
        GetTimeFormatA(0, 0, 0, "h:mm tt", time, sizeof(time));
        sprintf(description, "%s %s", date, time);
        field_0x1a0->UnknownFunction49c010(field_0x038->field_0x74c, description, 0);
        field_0x1a0->UnknownFunction4691f0();
        field_0x1a0->Release();
        if (field_0x1bc <= field_0x1c8) {
            field_0x1c8 = field_0x1bc;
            COPY_TEXT(field_0x1f0, field_0x2f4, 0x104);
            if (strcmp(field_0x2f4, "VCRghost.dat") == 0) {
                COPY_TEXT(field_0x2f4, "VCRgtemp.dat", 0x104);
            } else {
                COPY_TEXT(field_0x2f4, "VCRghost.dat", 0x104);
            }
        }
    }
    field_0x1a0 = new (__FILE__, 0x10e7) KrustyVCR;
    if (field_0x1a0 == 0) {
        return;
    }
    if (!field_0x1a0->UnknownFunction49bf10(UnknownFunction4230e0, 0, field_0x2f4, field_0x1a8)) {
        return;
    }
    char model[0x104];
    char rider[0x104];
    char riderName[0x40];
    char bikeName[0x40];
    sprintf(model, "%s\\%s", "Res", "Ghost.mcf");
    sprintf(rider, "%s\\GhostRider.mcf", "Res");
    g_UnknownGlobal56e26c->ui->UnknownFunction49b560(&g_UnknownGlobal56e26c->field_0x1eec[0x40],
                                                     riderName, 0x3f);
    g_UnknownGlobal56e26c->ui->UnknownFunction49b7f0(&g_UnknownGlobal56e26c->field_0x1eec[0x80],
                                                     bikeName, 0x3f);
    field_0x1a0->UnknownFunction49c070(0, 0, 0, &g_UnknownGlobal56e26c->mode, model, rider,
                                       riderName, bikeName, field_0x040[0]->field_0x738,
                                       (unsigned char)field_0x040[0]->field_0x737,
                                       g_UnknownGlobal56e26c->field_0x2144);
    UnknownFunction469190(field_0x1a0, -1);
    field_0x038->UnknownFunction496e20(field_0x1a0);
    if (field_0x189 && field_0x038->field_0x7a0 == 1 && field_0x1f0[0] == 0) {
        return;
    }
    field_0x1a4 = new (__FILE__, 0x1103) KrustyVCR;
    if (field_0x1a4 == 0) {
        return;
    }
    if (!field_0x1a4->UnknownFunction49bf10(UnknownFunction4230e0, 1, field_0x1f0, field_0x1a8)) {
        return;
    }
    field_0x038->field_0x750 = field_0x1c8 = field_0x1a4->UnknownFunction49c000();
    field_0x040[0]->UnknownFunction496e20(field_0x1a4);
    field_0x040[0]->UnknownVirtualSlot5();
    field_0x040[0]->field_0x11b8 = 0;
    UnknownFunction423040(0);
    UnknownFunction469190(field_0x1a4, -1);
}

// 0x00421050
void BikeRace::UnknownFunction421050() {
    if (g_UnknownGlobal56e26c->field_0x3428) {
        field_0x15c = 0.0f;
        return;
    }
    int mode = g_UnknownGlobal56e26c->field_0x2d70;
    if (mode != 0 && mode != 4) {
        if (field_0x06c != 0) {
            field_0x15c = 8.2f;
            field_0x06c->UnknownVirtualSlot5();
            field_0x058->UnknownFunction4eb040(field_0x070, 0.0f, 0, 0);
            if (field_0x06c->field_0x210 != 0) {
                field_0x06c->field_0x210->Release();
                field_0x06c->field_0x210 = 0;
            }
            field_0x058->UnknownFunction4eafd0(field_0x070);
        } else {
            field_0x15c = 5.2f;
        }
    }
}

// 0x00423040
void BikeRace::UnknownFunction423040(int index) {
    UnknownBikeRaceRacer* racer = field_0x040[index];
    if (racer != 0) {
        racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
    }
}

// Inline: the racers in the race, own and AI (a single player race) or
// own and remote.
static inline int UnknownFunctionRacerCount() {
    if (g_UnknownGlobal56e26c->field_0x18 == 1) {
        if (g_UnknownGlobal56e26c->field_0x2d70 == 4) {
            return g_UnknownGlobal56e26c->field_0x18;
        }
        return g_UnknownGlobal56e26c->field_0x2d94 + 1;
    }
    return g_UnknownGlobal56e26c->field_0x2d98 + g_UnknownGlobal56e26c->field_0x18;
}

// 0x00422ec0
void BikeRace::UnknownFunction422ec0() {
    UnknownBikeRaceRacer* racer = field_0x038;
    racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
    for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++) {
        racer = field_0x040[i];
        if (racer != 0) {
            racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
        }
    }
}

// 0x004230e0
int UnknownFunction4230e0(int a, void* data, int flag, int b, int* keep, int time,
                          int* milliseconds) {
    BikeRace* race = (BikeRace*)g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2f0();
    *milliseconds = (int)(race->field_0x1b8 * 1000.0f);
    return race->UnknownFunction421d50(a, data, flag, b, keep, time);
}

// 0x00423140
void BikeRace::UnknownFunction423140(Track* track) {
    TrackListItem* list = 0;
    TrackListItem* item =
        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x139d);
    if (item == 0) {
        return;
    }
    item->field_0x04 = track->field_0x00;
    item->field_0x0c = list;
    list = item;
    while (list != 0) {
        if (list->field_0x04->field_0x00 & 4) {
            item = list;
            list = list->field_0x0c;
            item->field_0x04->field_0x00 &= ~4;
            operator delete(item, __FILE__, 0x13a8);
        } else {
            TrackSegment* segment = list->field_0x04->field_0x08;
            while (segment != 0 && segment->field_0x2c != 0) {
                Vector3 a;
                Vector3 b;
                SetDrawColor(0, 0xff, 0);
                a = *(Vector3*)&segment->field_0x0c;
                b = *(Vector3*)&segment->field_0x2c->field_0x0c;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                a = *(Vector3*)&segment->field_0x18;
                b = *(Vector3*)&segment->field_0x2c->field_0x18;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                SetDrawColor(0, 0, 0xff);
                a = *(Vector3*)&segment->field_0x00;
                b = *(Vector3*)&segment->field_0x2c->field_0x00;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                segment = segment->field_0x2c;
            }
            item = list;
            item->field_0x04->field_0x00 |= 4;
            for (int i = 0; i < item->field_0x04->field_0x10; i++) {
                TrackNode* link = item->field_0x04->field_0x14[i];
                if (!(link->field_0x00 & 4)) {
                    TrackListItem* next =
                        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x13c9);
                    if (next == 0) {
                        track->UnknownFunction517930(&list, 0);
                        return;
                    }
                    next->field_0x04 = item->field_0x04->field_0x14[i];
                    next->field_0x0c = list;
                    list = next;
                }
            }
        }
    }
}

// 0x00423410
int BikeRace::UnknownVirtualSlot14() {
    Vector3 a;
    Vector3 b;
    if (field_0x3fd) {
        GameObject::UnknownVirtualSlot14();
    }
    if (field_0x034) {
        if (field_0x048 != 0) {
            UnknownFunction423140(field_0x048);
            if (field_0x144) {
                SetDrawColor(0xff, 0, 0xff);
                a = field_0x0cc.field_0x00 - field_0x0cc.field_0x18;
                b = field_0x0cc.field_0x00 + field_0x0cc.field_0x18;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine(&a, &b);
            }
            if (field_0x148) {
                SetDrawColor(0, 0xff, 0xff);
                a = field_0x108.field_0x00 - field_0x108.field_0x18;
                b = field_0x108.field_0x00 + field_0x108.field_0x18;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine(&a, &b);
            }
        }
        if (field_0x034) {
            if (g_UnknownGlobal577a00) {
                SetDrawColor(0xff, 0, 0);
            } else {
                SetDrawColor(0, 0xff, 0);
            }
            DrawLine(&g_UnknownGlobal578de0[0], &g_UnknownGlobal578de0[1]);
            if (g_UnknownGlobal578da8) {
                SetDrawColor(0xff, 0, 0x80);
            } else {
                SetDrawColor(0, 0xff, 0x80);
            }
            DrawLine(&g_UnknownGlobal577a30[0], &g_UnknownGlobal577a30[1]);
            if (g_UnknownGlobal578dac) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal577a48[0], &g_UnknownGlobal577a48[1]);
            if (g_UnknownGlobal578df8) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal5779d8[0], &g_UnknownGlobal5779d8[1]);
            if (g_UnknownGlobal578dd8) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal577a18[0], &g_UnknownGlobal577a18[1]);
            SetDrawColor(0, 0, 0xff);
            DrawLine(&g_UnknownGlobal689c30[0], &g_UnknownGlobal689c30[1]);
            SetDrawColor(0xff, 0, 0xff);
            DrawLine(&g_UnknownGlobal689c48[0], &g_UnknownGlobal689c48[1]);
            SetDrawColor(0xff, 0, 0);
            DrawLine(&g_UnknownGlobal578dc0[0], &g_UnknownGlobal578dc0[1]);
        }
    }
    return 1;
}

// 0x00423790
void BikeRace::UnknownFunction423790(int level) {
    g_UnknownGlobal689f18 = g_UnknownGlobal56e26c->field_0x2d0 ? g_UnknownGlobal5744c8 : g_UnknownGlobal574428;
    field_0x038->field_0x3bc->UnknownFunction4451e0(level);
    field_0x038->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
    if (g_UnknownGlobal56e26c->field_0x2d70 != 4) {
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x2d94; i++) {
            field_0x040[i]->field_0x3bc->UnknownFunction4451e0(level);
            field_0x040[i]->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
        }
    }
    for (int j = 0; j < g_UnknownGlobal56e26c->field_0x2da5; j++) {
        field_0x03c[j]->field_0x3bc->UnknownFunction4451e0(level);
        field_0x03c[j]->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
    }
}

// 0x00423880
int BikeRace::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (UnknownFunction43caa0(6, 0, event, 0x80)) {
        field_0x3fd = 1 - field_0x3fd;
        return 1;
    }
    return 0;
}
