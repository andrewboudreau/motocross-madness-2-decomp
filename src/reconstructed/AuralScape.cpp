#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AuralScape.h"

#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "VehicleCamera.h"

#define SoundSystem() ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)

// TU extent (strong inference): 0x00401a30..0x00403d4b. Arrow.cpp's
// initializer is the .CRT$XCU entry 0x00566004; the next five entries
// (0x00403c10, 0x00403c60, 0x00403cb0, 0x00403d00 and 0x00402050) are this
// file's, and 0x0056601c is Bike.cpp's. The pooled ContainerList.h literal
// (0x005666e4) sits between Arrow.cpp's data and this file's __FILE__ and
// is shared by the SoundGroup code and AuralScape's; the type descriptors
// in .data and the vtables in .rdata run SoundGroup, SoundInterface,
// SoundEmitter, SoultreeSoundEmitter, AuralScape, and .bss 0x005776d8..
// 0x00577737 lies between Arrow.cpp's and BackgroundImage.cpp's globals.

// 0x005776e8, 0x00577710, 0x00577720 and 0x005776d8 (initializers
// 0x00403c10..0x00403d4b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00401a30
SoundGroup::SoundGroup(int flags) : GameObject(flags) {
    field_0x2c_bit0 = 1;
    field_0x30 = 0;
    field_0x34.Init(8, 16);
}

// 0x00401af0
SoundGroup::~SoundGroup() {
    UnknownFunction401c30();
}

// 0x00401b50
int SoundGroup::UnknownFunction401b50(Sound* sound) {
    return field_0x34.Add(sound);
}

// 0x00401be0
int SoundGroup::UnknownFunction401be0(Sound* sound) {
    return field_0x34.Remove(sound);
}

// 0x00401c30: releases the members through a copy of the list (a release
// removes the sound from field_0x34).
void SoundGroup::UnknownFunction401c30() {
    ContainerList<Sound*> sounds;
    sounds.Init(field_0x34.m_count, 16);
    int i;
    for (i = 0; i < field_0x34.m_count; i++)
        sounds.Add(field_0x34.Get(i));
    for (i = 0; i < sounds.m_count; i++)
        sounds.Get(i)->Release();
    field_0x34.Clear();
}

// 0x00401dd0
void SoundGroup::UnknownFunction401dd0(long volume) {
    field_0x30 = volume;
    for (int i = 0; i < field_0x34.m_count; i++)
        field_0x34.Get(i)->UnknownFunction4bcbe0(volume, 0);
}

// 0x00401e20
void SoundGroup::UnknownVirtualSlot4() {
    GameObject::UnknownVirtualSlot4();
    UnknownVirtualSlot16(1);
}

// 0x00401e40
void SoundGroup::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
    UnknownVirtualSlot16(0);
}

// 0x00401e60
void SoundGroup::UnknownVirtualSlot6() {
    GameObject::UnknownVirtualSlot6();
    UnknownVirtualSlot16(1);
}

// 0x00401e80
void SoundGroup::UnknownVirtualSlot7() {
    GameObject::UnknownVirtualSlot7();
    UnknownVirtualSlot16(0);
}

// 0x00401ea0
int SoundGroup::UnknownVirtualSlot18() {
    for (int i = 0; i < field_0x34.m_count; i++)
        field_0x34.Get(i)->UnknownFunction4bcdc0();
    return 1;
}

// 0x00401ee0
int SoundGroup::UnknownVirtualSlot16(int value) {
    GameObject::UnknownVirtualSlot16(value);
    for (int i = 0; i < field_0x34.m_count; i++)
        field_0x34.Get(i)->UnknownFunction4bce20(value);
    return 1;
}

// 0x00401f30
int SoundGroup::UnknownVirtualSlot10(float frameTime) {
    for (int i = 0; i < field_0x34.m_count; i++)
        field_0x34.Get(i)->UnknownFunction4bcea0(frameTime);
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00401f80
SoundInterface::SoundInterface() {
    field_0x2c_bit0 = 0;
}

// 0x00401fe0
SoundInterface::~SoundInterface() {
}

// 0x005776f8 / 0x0057772c: each listener's position and its distance to
// the emitter being placed (0x00403150). The array's initializer is the
// empty 0x00402050/0x00402060.
static Vector3 s_UnknownListenerPositions5776f8[2];
static float s_UnknownListenerDistances57772c[2];

// 0x00577734: the next SoundEmitter serial number.
static int s_UnknownEmitterCount577734;

// 0x00402070
void UnknownFunction402070(Vector3* listener, Vector3* position, float distance, float minDistance,
                           float maxDistance, long* volume, long* pan) {
    *volume = -2500;
    *pan = 0;
    if (distance < maxDistance) {
        if (distance <= minDistance) {
            *volume = 0;
            *pan = 0;
            return;
        }
        *volume = (long)((distance - minDistance) / (maxDistance - minDistance) * -2500.0f);
        *pan = 0;
    }
}

// 0x004020e0
SoundEmitter::SoundEmitter(AuralScape* scape, SoundGroup* group, int type, int flags) : GameObject(flags) {
    field_0x2c = scape;
    field_0x30 = group;
    field_0x34 = 0;
    memset(&field_0x150, 0, sizeof(field_0x150));
    field_0x190 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    field_0x14c = 0;
    field_0x38 = 0;
    field_0x3c = 100.0f;
    field_0x150.position = field_0x190;
    strcpy(field_0x48, "");
    field_0x40 = (short)type;
    field_0x1a0_bit0 = 0;
    field_0x1a0_bit1 = 0;
    field_0x1a0_bit2 = 0;
    field_0x1a0_bit3 = 0;
    field_0x1a0_bit4 = 0;
    field_0x44 = s_UnknownEmitterCount577734++;
    field_0x19c = 0;
}

// 0x00402200
SoundEmitter::~SoundEmitter() {
    if (field_0x2c) {
        field_0x2c->UnknownFunction403bc0(this);
        field_0x2c->Release();
    }
}

// 0x00402260
SoundEmitter* SoundEmitter::UnknownFunction402260(void* target, const char* name, UnknownSound3DParameters params,
                                                  unsigned long flags, float oneShotDistance,
                                                  float randomTriggerPercent, int is2D, int force2D) {
    char path[0x104];
    UnknownTextureStream* stream = new (__FILE__, 130) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int found = 0;

    if (GameObject::UnknownVirtualSlot8(target) && field_0x2c) {
        field_0x2c->AddRef();
        if (!(flags & 2) || (flags & 4) || is2D)
            field_0x1a0_bit2 = 1;
        field_0x1a0_bit3 = force2D;
        params.size = sizeof(params);
        field_0x14c = flags;
        field_0x150 = params;
        field_0x38 = oneShotDistance;
        field_0x3c = randomTriggerPercent;
        strncpy(field_0x48, name, sizeof(field_0x48) - 1);
        field_0x48[sizeof(field_0x48) - 1] = 0;
        sprintf(path, "%s\\%s", "Res", field_0x48);
        found = g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)field_0x48);
    }
    if (!found) {
        if (stream)
            delete stream;
        Release();
        return 0;
    }
    if (stream)
        delete stream;
    field_0x2c->UnknownFunction403b20(this);
    return this;
}

// 0x00402420
void SoundEmitter::UnknownFunction402420(UnknownSound3DParameters params) {
    field_0x190 = field_0x150.position;
    field_0x150 = params;
    if (field_0x34)
        field_0x34->UnknownFunction4bd740(&params);
}

// 0x00402470
void SoundEmitter::UnknownFunction402470(Vector3* position, Vector3* velocity) {
    if (position) {
        field_0x190 = field_0x150.position;
        field_0x150.position = *position;
    }
    if (velocity)
        field_0x150.velocity = *velocity;
    if (field_0x34) {
        field_0x34->UnknownFunction4bd7e0(field_0x150.position, 1);
        field_0x34->UnknownFunction4bd8a0(field_0x150.velocity, 1);
    }
}

// 0x00402530
void SoundEmitter::UnknownFunction402530(int count, AuralScapeListener** listeners) {
    field_0x19c = FLT_MAX;
    for (int i = 0; i < count; i++) {
        Vector3 position;
        float dx;
        float dz;
        listeners[i]->UnknownFunction402cb0(&position);
        float x = field_0x150.position.x;
        if (field_0x14c & 2) {
            dx = x - position.x;
            if (dx < 0.0f)
                dx = -dx;
            dz = field_0x150.position.z - position.z;
            if (dz < 0.0f)
                dz = -dz;
        } else {
            Vector3 ahead;
            ahead.x = x + field_0x150.velocity.x;
            float z = field_0x150.position.z;
            ahead.z = z + field_0x150.velocity.z;
            dx = ahead.x - position.x;
            if (dx < 0.0f)
                dx = -dx;
            dz = ahead.z - position.z;
            if (dz < 0.0f)
                dz = -dz;
        }
        float distance = UnknownFunction460b50(dx * dx + dz * dz);
        if (distance < field_0x19c)
            field_0x19c = distance;
    }
    field_0x1a0_bit1 = 0;
    if (field_0x14c & 1) {
        if (!field_0x1a0_bit0 && field_0x19c <= field_0x38)
            field_0x1a0_bit1 = 1;
    } else if (field_0x150.maxDistance * 1.2f > field_0x19c) {
        field_0x1a0_bit1 = 1;
    }
}

// 0x00402680
int SoundEmitter::UnknownFunction402680() {
    if (!field_0x34)
        return 0;
    return field_0x34->UnknownFunction4bca80();
}

// 0x00402690
int SoundEmitter::UnknownFunction402690(long volume) {
    if (!field_0x34)
        return 0;
    return field_0x34->UnknownFunction4bcbe0(volume, 0);
}

// 0x004026b0
int SoundEmitter::UnknownFunction4026b0(long pan) {
    if (!field_0x34)
        return 0;
    return field_0x34->UnknownFunction4bcca0(pan, 0);
}

// 0x004026d0
int SoundEmitter::UnknownFunction4026d0() {
    field_0x1a0_bit4 = 0;
    if (!field_0x34)
        return 0;
    return field_0x34->UnknownFunction4bc940(0);
}

// 0x00402700
int SoundEmitter::UnknownFunction402700(unsigned long flags) {
    if (field_0x34 || field_0x1a0_bit0)
        return 0;
    field_0x14c &= ~0x30;
    if (field_0x1a0_bit3)
        field_0x14c |= 0x20;
    else
        field_0x14c |= flags;
    int buffer = 0;
    if (field_0x14c & 0x10)
        buffer = 8;
    field_0x34 = UnknownFunction4bb890(field_0x30, field_0x48, buffer | 4, 3, 0, -1);
    if (!field_0x34)
        return 0;
    if (field_0x14c & 4) {
        UnknownFunction402420(field_0x150);
        return 1;
    }
    field_0x34->UnknownFunction4bd960(field_0x150.minDistance, field_0x150.maxDistance, 1);
    UnknownFunction402470(&field_0x150.position, &field_0x150.velocity);
    return 1;
}

// 0x004027f0
void SoundEmitter::UnknownFunction4027f0() {
    if (field_0x34) {
        field_0x34->Release();
        field_0x34 = 0;
    }
}

// 0x00402810
int SoundEmitter::UnknownFunction402810() {
    if (!field_0x34)
        return 0;
    if ((field_0x14c & 1) && field_0x1a0_bit0)
        return 0;
    if (field_0x1a0_bit4)
        return 1;
    field_0x1a0_bit4 = 1;
    if ((rand() * (1.0f / 32768.0f)) * 100.0f > field_0x3c)
        return 1;
    int loop = (field_0x14c >> 3) & 1;
    if (field_0x14c & 1)
        field_0x1a0_bit0 = 1;
    int hardware = 0;
    if ((int)SoundSystem()->field_0x3fc.freeHw3DAllBuffers > 0 && (field_0x14c & 0x10))
        hardware = 1;
    return field_0x34->UnknownFunction4bc6b0(0, loop, hardware);
}

// 0x004028d0
int SoundEmitter::UnknownVirtualSlot10(float frameTime) {
    if (field_0x34 && !(field_0x14c & 2)) {
        Vector3 move;
        move.x = field_0x150.position.x - field_0x190.x;
        move.y = field_0x150.position.y - field_0x190.y;
        move.z = field_0x150.position.z - field_0x190.z;
        field_0x150.velocity = move * frameTime;
        field_0x34->UnknownFunction4bd8a0(field_0x150.velocity, 0);
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00402980
SoultreeSoundEmitter::SoultreeSoundEmitter(AuralScape* scape, SoundGroup* group, int type, int flags)
    : SoundEmitter(scape, group, type, flags) {
    field_0x1a4 = 0;
    field_0x1a8 = 0;
}

// 0x004029f0
SoultreeSoundEmitter* SoultreeSoundEmitter::UnknownFunction4029f0(void* target, const char* name,
                                                                  const char* partName, UnknownVehiclePart* part,
                                                                  UnknownSound3DParameters params,
                                                                  unsigned long flags, float oneShotDistance,
                                                                  float randomTriggerPercent, int is2D,
                                                                  int force2D) {
    if (!UnknownFunction402260(target, name, params, flags, oneShotDistance, randomTriggerPercent, is2D, force2D))
        return 0;
    field_0x1a4 = part;
    if (!part) {
        Release();
        return 0;
    }
    field_0x1a8 = part->UnknownFunction4fdae0(partName);
    return this;
}

// 0x00402a70
int SoultreeSoundEmitter::UnknownVirtualSlot10(float frameTime) {
    Vector3 position = kVec3Zero;
    if (field_0x1a8)
        field_0x1a8->UnknownFunction4fc9a0(0, &position);
    else
        field_0x1a4->UnknownFunction4fc9a0(0, &position);
    field_0x190 = field_0x150.position;
    field_0x150.position = position;
    if (field_0x34)
        field_0x34->UnknownFunction4bd7e0(position, 1);
    return SoundEmitter::UnknownVirtualSlot10(frameTime);
}

AuralScapeListener::AuralScapeListener() {
    field_0x00 = 0;
    field_0x04 = 0;
}

// 0x00402b20
AuralScapeListener::~AuralScapeListener() {
    if (field_0x04) {
        field_0x04->Release();
        field_0x04 = 0;
    }
}

// 0x00402b40
int AuralScapeListener::UnknownFunction402b40(AuralScape* owner, float doppler, float rolloff) {
    if (!owner)
        return 0;
    field_0x00 = owner;
    field_0x04 = SoundSystem()->UnknownFunction4be800();
    if (!field_0x04)
        return 0;
    if (!UnknownFunction402c10(doppler))
        return 0;
    if (!UnknownFunction402c40(rolloff))
        return 0;
    if (!UnknownFunction402be0(0.3048f))
        return 0;
    return UnknownFunction402bc0() != 0;
}

// 0x00402bc0
int AuralScapeListener::UnknownFunction402bc0() {
    if (!field_0x04)
        return 0;
    return field_0x04->CommitDeferredSettings() >= 0;
}

// 0x00402be0
int AuralScapeListener::UnknownFunction402be0(float factor) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetDistanceFactor(factor, 1) >= 0;
}

// 0x00402c10
int AuralScapeListener::UnknownFunction402c10(float factor) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetDopplerFactor(factor, 1) >= 0;
}

// 0x00402c40
int AuralScapeListener::UnknownFunction402c40(float factor) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetRolloffFactor(factor, 1) >= 0;
}

// 0x00402c70
int AuralScapeListener::UnknownFunction402c70(Vector3 front, Vector3 top) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetOrientation(front.x, front.y, front.z, top.x, top.y, top.z, 1) >= 0;
}

// 0x00402cb0
int AuralScapeListener::UnknownFunction402cb0(Vector3* position) {
    if (!field_0x04)
        return 0;
    return field_0x04->GetPosition(position) >= 0;
}

// 0x00402ce0
int AuralScapeListener::UnknownFunction402ce0(Vector3 position) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetPosition(position.x, position.y, position.z, 1) >= 0;
}

// 0x00402d10
int AuralScapeListener::UnknownFunction402d10(Vector3 velocity) {
    if (!field_0x04)
        return 0;
    return field_0x04->SetVelocity(velocity.x, velocity.y, velocity.z, 1) >= 0;
}

// 0x00402d40: qsort order with bit 2 set first.
static int UnknownCompare402d40(const void* a, const void* b) {
    SoundEmitter* first = *(SoundEmitter**)a;
    SoundEmitter* second = *(SoundEmitter**)b;
    if (second->field_0x1a0_bit2 < first->field_0x1a0_bit2)
        return -1;
    return 1;
}

// 0x00402d70: qsort order, farthest first.
static int UnknownCompare402d70(const void* a, const void* b) {
    SoundEmitter* first = *(SoundEmitter**)a;
    SoundEmitter* second = *(SoundEmitter**)b;
    if (first->field_0x19c > second->field_0x19c)
        return -1;
    return 1;
}

// 0x00402da0
AuralScape::AuralScape(int flags) : GameObject(flags) {
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x2c = 0;
    for (int i = 0; i < 2; i++)
        field_0x3c[i] = 0;
    field_0x94 = 0;
    field_0x98 = 0;
    field_0x9c = -1;
    field_0xa0_bit0 = 1;
}

// 0x00402f00
AuralScape* AuralScape::UnknownFunction402f00(void* target, int sounds) {
    if (!GameObject::UnknownVirtualSlot8(target))
        return 0;
    field_0x30 = sounds;
    int ok = field_0x44.Init(8, 16) && field_0x58.Init(8, 16) && field_0x6c.Init(8, 16) && field_0x80.Init(8, 16);
    if (!ok) {
        Release();
        return 0;
    }
    return this;
}

// 0x00403000
int AuralScape::UnknownFunction403000() {
    return ++field_0x2c - 1;
}

// 0x00403010
int AuralScape::UnknownFunction403010(int index, float doppler, float rolloff) {
    field_0x3c[index] = new (__FILE__, 717) AuralScapeListener();
    if (field_0x3c[index] && field_0x3c[index]->UnknownFunction402b40(this, doppler, rolloff)) {
        field_0x38++;
        return 1;
    }
    delete field_0x3c[index];
    field_0x3c[index] = 0;
    return 0;
}

// 0x004030a0
void AuralScape::UnknownFunction4030a0(int index, Vector3* position, Vector3* front, Vector3* top,
                                       Vector3* velocity) {
    if (field_0x3c[index]) {
        if (position)
            field_0x3c[index]->UnknownFunction402ce0(*position);
        if (front && top)
            field_0x3c[index]->UnknownFunction402c70(*front, *top);
        if (velocity)
            field_0x3c[index]->UnknownFunction402d10(*velocity);
    }
}

// The volume of `emitter` from its nearest listener (pan is always 0).
static inline void UnknownPlaceEmitter(const int& count, AuralScapeListener** listeners, SoundEmitter* emitter) {
    int j;
    for (j = 0; j < count; j++) {
        listeners[j]->UnknownFunction402cb0(&s_UnknownListenerPositions5776f8[j]);
        float dx = s_UnknownListenerPositions5776f8[j].x - emitter->field_0x150.position.x;
        if (dx < 0.0f)
            dx = -dx;
        float dz = s_UnknownListenerPositions5776f8[j].z - emitter->field_0x150.position.z;
        if (dz < 0.0f)
            dz = -dz;
        s_UnknownListenerDistances57772c[j] = UnknownFunction460b50(dx * dx + dz * dz);
    }
    float nearest = s_UnknownListenerDistances57772c[0];
    Vector3 listener = s_UnknownListenerPositions5776f8[0];
    for (j = 0; j < count; j++) {
        if (s_UnknownListenerDistances57772c[j] < nearest) {
            nearest = s_UnknownListenerDistances57772c[j];
            listener = s_UnknownListenerPositions5776f8[j];
        }
    }
    long volume;
    long pan;
    UnknownFunction402070(&listener, &emitter->field_0x150.position, nearest,
                          emitter->field_0x150.minDistance, emitter->field_0x150.maxDistance, &volume,
                          &pan);
    emitter->UnknownFunction402690(volume);
    emitter->UnknownFunction4026b0(pan);
}

// Inline in 0x00403150. With this helper and UnknownPlaceEmitter VC6's
// inline budget for 0x00403150 covers ContainerList::Reserve only in the
// last Add, as in retail.
inline void AuralScape::UnknownUpdateDistances() {
    for (int i = 0; i < field_0x44.m_count; i++)
        field_0x44.Get(i)->UnknownFunction402530(field_0x38, field_0x3c);
}

// 0x00403150
void AuralScape::UnknownFunction403150() {
    int i;
    SoundEmitter* emitter;
    int waiting3D = 0;

    UnknownUpdateDistances();

    field_0x58.Clear();
    for (i = 0; i < field_0x44.m_count; i++) {
        emitter = field_0x44.Get(i);
        if (emitter->field_0x1a0_bit1)
            field_0x58.Add(emitter);
    }

    field_0x6c.Clear();
    for (i = 0; i < field_0x44.m_count; i++) {
        emitter = field_0x44.Get(i);
        if (emitter->UnknownFunction402680())
            field_0x6c.Add(emitter);
    }

    for (i = 0; i < field_0x44.m_count; i++) {
        emitter = field_0x44.Get(i);
        if (!emitter->field_0x1a0_bit1) {
            emitter->UnknownFunction4026d0();
            field_0x6c.Remove(emitter);
        }
    }


    field_0x80.Clear();
    for (i = 0; i < field_0x58.m_count; i++) {
        emitter = field_0x58.Get(i);
        if (!emitter->UnknownFunction402680()) {
            if (emitter->field_0x34) {
                emitter->UnknownFunction402810();
            } else {
                field_0x80.Add(emitter);
                if (emitter->field_0x1a0_bit2)
                    waiting3D++;
            }
        }
    }
    qsort(field_0x80.m_data, field_0x80.m_count, sizeof(SoundEmitter*), UnknownCompare402d40);

    int available = SoundSystem()->field_0x3fc.freeHw3DAllBuffers + field_0x30 - field_0x34;
    if (waiting3D > available) {
        qsort(field_0x44.m_data, field_0x44.m_count, sizeof(SoundEmitter*), UnknownCompare402d70);
        for (i = 0; i < field_0x44.m_count; i++) {
            emitter = field_0x44.Get(i);
            if (!emitter->UnknownFunction402680() && (emitter->field_0x14c & 0x10)) {
                available++;
                emitter->UnknownFunction4027f0();
                field_0x34--;
                if (waiting3D >= available)
                    break;
            }
        }
    }

    for (i = 0; i < field_0x80.m_count; i++) {
        emitter = field_0x80.Get(i);
        if (available > 0) {
            if (emitter->UnknownFunction402700(0x10)) {
                field_0x34++;
                available--;
            }
        } else {
            emitter->UnknownFunction402700(0x20);
        }
    }


    for (i = 0; i < field_0x80.m_count; i++) {
        emitter = field_0x80.Get(i);
        emitter->UnknownFunction402810();
        field_0x6c.Add(emitter);
    }

    for (i = 0; i < field_0x6c.m_count; i++) {
        emitter = field_0x6c.Get(i);
        if (emitter->field_0x14c & 0x20) {
            UnknownPlaceEmitter(field_0x38, field_0x3c, emitter);
        }
    }
}

// 0x00403710
int AuralScape::UnknownVirtualSlot10(float frameTime) {
    UnknownFunction403150();

    int zone = 0;
    unsigned long environment = 0;
    unsigned char material = 0;
    if (field_0xa0_bit0 && field_0x98 && field_0x94) {
        Vector3 position = ((UnknownAuralScapeTarget*)field_0x18)->field_0x08->field_0x170;
        field_0x98->UnknownFunction507c10(&position, 0, 0, (int)&material);
        zone = (material >> 4) & 3;
        environment = field_0x94->field_0xa4->field_0x410[zone];
        SoundSystem()->UnknownFunction4bed00(environment);
    }

    for (int i = 0; i < field_0x38; i++)
        field_0x3c[i]->UnknownFunction402bc0();

    DebugOverlay* overlay = g_UnknownGlobal56e26c->field_0x38;
    if (overlay) {
        if (field_0x9c < 0)
            field_0x9c = overlay->NewPage();
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447fa0(field_0x9c, "Audio");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Hardware");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x9c, "3D HW Buffers %d Free %d", SoundSystem()->field_0x3fc.maxHw3DAllBuffers,
            SoundSystem()->field_0x3fc.freeHw3DAllBuffers);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x9c, "2D HW Buffers %d Free %d", SoundSystem()->field_0x3fc.maxHwMixingAllBuffers,
            SoundSystem()->field_0x3fc.freeHwMixingAllBuffers);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
            field_0x9c, "Bytes %d Free %d", SoundSystem()->field_0x3fc.totalHwMemBytes,
            SoundSystem()->field_0x3fc.freeHwMemBytes);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "");

        int sounds = 0;
        int j = 0;
        int playing = 0;
        int bytes = 0;
        for (; j < SoundSystem()->field_0x04.m_count; j++) {
            Sound* sound = (Sound*)SoundSystem()->field_0x04.Get(j);
            if (sound) {
                sounds++;
                if (sound->UnknownFunction4bca80())
                    playing++;
                bytes += sound->field_0x198;
            }
        }
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Sounds %d Playing %d", sounds,
                                                                 playing);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "AuralScape");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "NumEmitters %d", field_0x44.m_count);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Audible %d", field_0x58.m_count);
        int loaded = 0;
        for (int k = 0; k < field_0x44.m_count; k++) {
            if (field_0x44.Get(k)->field_0x34)
                loaded++;
        }
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Loaded %d", loaded);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Playing %d", field_0x6c.m_count);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "ReverbZone %d", zone);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "ReverbEnviro %d", environment);
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "Total Wave Memory %d", bytes);
        if (SoundSystem()->field_0x46c)
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x9c, "SoundCacheBytes %d",
                                                                     SoundSystem()->field_0x46c->field_0x28);
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00403b20
int AuralScape::UnknownFunction403b20(SoundEmitter* emitter) {
    if (!emitter)
        return 0;
    return field_0x44.Add(emitter);
}

// 0x00403bc0
int AuralScape::UnknownFunction403bc0(SoundEmitter* emitter) {
    if (!emitter)
        return 0;
    return field_0x44.Remove(emitter);
}
