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

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

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
    memberVolume = 0;
    members.Init(8, 16);
}

// 0x00401af0
SoundGroup::~SoundGroup() {
    ReleaseMembers();
}

// 0x00401b50
int SoundGroup::AddMember(Sound* sound) {
    return members.Add(sound);
}

// 0x00401be0
int SoundGroup::RemoveMember(Sound* sound) {
    return members.Remove(sound);
}

// 0x00401c30: releases the members through a copy of the list (a release
// removes the sound from members).
void SoundGroup::ReleaseMembers() {
    ContainerList<Sound*> sounds;
    sounds.Init(members.m_count, 16);
    int i;
    for (i = 0; i < members.m_count; i++)
        sounds.Add(members.Get(i));
    for (i = 0; i < sounds.m_count; i++)
        sounds.Get(i)->Release();
    members.Clear();
}

// 0x00401dd0
void SoundGroup::SetMemberVolume(long volume) {
    memberVolume = volume;
    for (int i = 0; i < members.m_count; i++)
        members.Get(i)->SetVolume(volume, 0);
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
    for (int i = 0; i < members.m_count; i++)
        members.Get(i)->RestoreBuffer();
    return 1;
}

// 0x00401ee0
int SoundGroup::UnknownVirtualSlot16(int value) {
    GameObject::UnknownVirtualSlot16(value);
    for (int i = 0; i < members.m_count; i++)
        members.Get(i)->SetPaused(value);
    return 1;
}

// 0x00401f30
int SoundGroup::UnknownVirtualSlot10(float frameTime) {
    for (int i = 0; i < members.m_count; i++)
        members.Get(i)->UpdateFade(frameTime);
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00401f80
SoundInterface::SoundInterface() {
    audioStarted = 0;
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
void VolumeForDistance(Vector3* listener, Vector3* position, float distance, float minDistance,
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
    ownerScape = scape;
    emitterGroup = group;
    loadedSound = 0;
    memset(&soundParameters, 0, sizeof(soundParameters));
    previousPosition = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    emitterFlags = 0;
    oneShotRange = 0;
    triggerPercent = 100.0f;
    soundParameters.position = previousPosition;
    strcpy(fileName, "");
    emitterType = (short)type;
    oneShotFired = 0;
    audible = 0;
    field_0x1a0_bit2 = 0;
    field_0x1a0_bit3 = 0;
    started = 0;
    serialNumber = s_UnknownEmitterCount577734++;
    listenerDistance = 0;
}

// 0x00402200
SoundEmitter::~SoundEmitter() {
    if (ownerScape) {
        ownerScape->RemoveEmitter(this);
        ownerScape->Release();
    }
}

// 0x00402260
SoundEmitter* SoundEmitter::Create(void* target, const char* name, UnknownSound3DParameters params,
                                                  unsigned long flags, float oneShotDistance,
                                                  float randomTriggerPercent, int is2D, int force2D) {
    char path[0x104];
    UnknownTextureStream* stream = new (__FILE__, 130) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int found = 0;

    if (GameObject::UnknownVirtualSlot8(target) && ownerScape) {
        ownerScape->AddRef();
        if (!(flags & 2) || (flags & 4) || is2D)
            field_0x1a0_bit2 = 1;
        field_0x1a0_bit3 = force2D;
        params.size = sizeof(params);
        emitterFlags = flags;
        soundParameters = params;
        oneShotRange = oneShotDistance;
        triggerPercent = randomTriggerPercent;
        strncpy(fileName, name, sizeof(fileName) - 1);
        fileName[sizeof(fileName) - 1] = 0;
        sprintf(path, "%s\\%s", "Res", fileName);
        found = g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)fileName);
    }
    if (!found) {
        if (stream)
            delete stream;
        Release();
        return 0;
    }
    if (stream)
        delete stream;
    ownerScape->AddEmitter(this);
    return this;
}

// 0x00402420
void SoundEmitter::SetSoundParameters(UnknownSound3DParameters params) {
    previousPosition = soundParameters.position;
    soundParameters = params;
    if (loadedSound)
        loadedSound->Set3DParameters(&params);
}

// 0x00402470
void SoundEmitter::SetPositionAndVelocity(Vector3* position, Vector3* velocity) {
    if (position) {
        previousPosition = soundParameters.position;
        soundParameters.position = *position;
    }
    if (velocity)
        soundParameters.velocity = *velocity;
    if (loadedSound) {
        loadedSound->Set3DPosition(soundParameters.position, 1);
        loadedSound->Set3DVelocity(soundParameters.velocity, 1);
    }
}

// 0x00402530
void SoundEmitter::UpdateListenerDistance(int count, AuralScapeListener** listeners) {
    listenerDistance = FLT_MAX;
    for (int i = 0; i < count; i++) {
        Vector3 position;
        float dx;
        float dz;
        listeners[i]->GetPosition(&position);
        float x = soundParameters.position.x;
        if (emitterFlags & 2) {
            dx = x - position.x;
            if (dx < 0.0f)
                dx = -dx;
            dz = soundParameters.position.z - position.z;
            if (dz < 0.0f)
                dz = -dz;
        } else {
            Vector3 ahead;
            ahead.x = x + soundParameters.velocity.x;
            float z = soundParameters.position.z;
            ahead.z = z + soundParameters.velocity.z;
            dx = ahead.x - position.x;
            if (dx < 0.0f)
                dx = -dx;
            dz = ahead.z - position.z;
            if (dz < 0.0f)
                dz = -dz;
        }
        float distance = FastSqrt(dx * dx + dz * dz);
        if (distance < listenerDistance)
            listenerDistance = distance;
    }
    audible = 0;
    if (emitterFlags & 1) {
        if (!oneShotFired && listenerDistance <= oneShotRange)
            audible = 1;
    } else if (soundParameters.maxDistance * 1.2f > listenerDistance) {
        audible = 1;
    }
}

// 0x00402680
int SoundEmitter::IsPlaying() {
    if (!loadedSound)
        return 0;
    return loadedSound->IsPlaying();
}

// 0x00402690
int SoundEmitter::SetVolume(long volume) {
    if (!loadedSound)
        return 0;
    return loadedSound->SetVolume(volume, 0);
}

// 0x004026b0
int SoundEmitter::SetPan(long pan) {
    if (!loadedSound)
        return 0;
    return loadedSound->SetPan(pan, 0);
}

// 0x004026d0
int SoundEmitter::Stop() {
    started = 0;
    if (!loadedSound)
        return 0;
    return loadedSound->Stop(0);
}

// 0x00402700
int SoundEmitter::LoadSound(unsigned long flags) {
    if (loadedSound || oneShotFired)
        return 0;
    emitterFlags &= ~0x30;
    if (field_0x1a0_bit3)
        emitterFlags |= 0x20;
    else
        emitterFlags |= flags;
    int buffer = 0;
    if (emitterFlags & 0x10)
        buffer = 8;
    loadedSound = LoadEntrySound(emitterGroup, fileName, buffer | 4, 3, 0, -1);
    if (!loadedSound)
        return 0;
    if (emitterFlags & 4) {
        SetSoundParameters(soundParameters);
        return 1;
    }
    loadedSound->Set3DDistanceRange(soundParameters.minDistance, soundParameters.maxDistance, 1);
    SetPositionAndVelocity(&soundParameters.position, &soundParameters.velocity);
    return 1;
}

// 0x004027f0
void SoundEmitter::ReleaseSound() {
    if (loadedSound) {
        loadedSound->Release();
        loadedSound = 0;
    }
}

// 0x00402810
int SoundEmitter::Play() {
    if (!loadedSound)
        return 0;
    if ((emitterFlags & 1) && oneShotFired)
        return 0;
    if (started)
        return 1;
    started = 1;
    if ((rand() * (1.0f / 32768.0f)) * 100.0f > triggerPercent)
        return 1;
    int loop = (emitterFlags >> 3) & 1;
    if (emitterFlags & 1)
        oneShotFired = 1;
    int hardware = 0;
    if ((int)SoundSystem()->soundCaps.freeHw3DAllBuffers > 0 && (emitterFlags & 0x10))
        hardware = 1;
    return loadedSound->PlayWithOptions(0, loop, hardware);
}

// 0x004028d0
int SoundEmitter::UnknownVirtualSlot10(float frameTime) {
    if (loadedSound && !(emitterFlags & 2)) {
        Vector3 move;
        move.x = soundParameters.position.x - previousPosition.x;
        move.y = soundParameters.position.y - previousPosition.y;
        move.z = soundParameters.position.z - previousPosition.z;
        soundParameters.velocity = move * frameTime;
        loadedSound->Set3DVelocity(soundParameters.velocity, 0);
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00402980
SoultreeSoundEmitter::SoultreeSoundEmitter(AuralScape* scape, SoundGroup* group, int type, int flags)
    : SoundEmitter(scape, group, type, flags) {
    followedPart = 0;
    childPart = 0;
}

// 0x004029f0
SoultreeSoundEmitter* SoultreeSoundEmitter::CreateFollowing(void* target, const char* name,
                                                                  const char* partName, UnknownVehiclePart* part,
                                                                  UnknownSound3DParameters params,
                                                                  unsigned long flags, float oneShotDistance,
                                                                  float randomTriggerPercent, int is2D,
                                                                  int force2D) {
    if (!Create(target, name, params, flags, oneShotDistance, randomTriggerPercent, is2D, force2D))
        return 0;
    followedPart = part;
    if (!part) {
        Release();
        return 0;
    }
    childPart = part->FindByName(partName);
    return this;
}

// 0x00402a70
int SoultreeSoundEmitter::UnknownVirtualSlot10(float frameTime) {
    Vector3 position = kVec3Zero;
    if (childPart)
        childPart->GetPositionIn(0, &position);
    else
        followedPart->GetPositionIn(0, &position);
    previousPosition = soundParameters.position;
    soundParameters.position = position;
    if (loadedSound)
        loadedSound->Set3DPosition(position, 1);
    return SoundEmitter::UnknownVirtualSlot10(frameTime);
}

AuralScapeListener::AuralScapeListener() {
    ownerScape = 0;
    soundListener = 0;
}

// 0x00402b20
AuralScapeListener::~AuralScapeListener() {
    if (soundListener) {
        soundListener->Release();
        soundListener = 0;
    }
}

// 0x00402b40
int AuralScapeListener::Initialize(AuralScape* owner, float doppler, float rolloff) {
    if (!owner)
        return 0;
    ownerScape = owner;
    soundListener = SoundSystem()->QueryListener();
    if (!soundListener)
        return 0;
    if (!SetDopplerFactor(doppler))
        return 0;
    if (!SetRolloffFactor(rolloff))
        return 0;
    if (!SetDistanceFactor(0.3048f))
        return 0;
    return CommitDeferredSettings() != 0;
}

// 0x00402bc0
int AuralScapeListener::CommitDeferredSettings() {
    if (!soundListener)
        return 0;
    return soundListener->CommitDeferredSettings() >= 0;
}

// 0x00402be0
int AuralScapeListener::SetDistanceFactor(float factor) {
    if (!soundListener)
        return 0;
    return soundListener->SetDistanceFactor(factor, 1) >= 0;
}

// 0x00402c10
int AuralScapeListener::SetDopplerFactor(float factor) {
    if (!soundListener)
        return 0;
    return soundListener->SetDopplerFactor(factor, 1) >= 0;
}

// 0x00402c40
int AuralScapeListener::SetRolloffFactor(float factor) {
    if (!soundListener)
        return 0;
    return soundListener->SetRolloffFactor(factor, 1) >= 0;
}

// 0x00402c70
int AuralScapeListener::SetOrientation(Vector3 front, Vector3 top) {
    if (!soundListener)
        return 0;
    return soundListener->SetOrientation(front.x, front.y, front.z, top.x, top.y, top.z, 1) >= 0;
}

// 0x00402cb0
int AuralScapeListener::GetPosition(Vector3* position) {
    if (!soundListener)
        return 0;
    return soundListener->GetPosition(position) >= 0;
}

// 0x00402ce0
int AuralScapeListener::SetPosition(Vector3 position) {
    if (!soundListener)
        return 0;
    return soundListener->SetPosition(position.x, position.y, position.z, 1) >= 0;
}

// 0x00402d10
int AuralScapeListener::SetVelocity(Vector3 velocity) {
    if (!soundListener)
        return 0;
    return soundListener->SetVelocity(velocity.x, velocity.y, velocity.z, 1) >= 0;
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
    if (first->listenerDistance > second->listenerDistance)
        return -1;
    return 1;
}

// 0x00402da0
AuralScape::AuralScape(int flags) : GameObject(flags) {
    soundBudget = 0;
    soundsLoaded = 0;
    listenerCount = 0;
    listenerNumber = 0;
    for (int i = 0; i < 2; i++)
        listeners[i] = 0;
    reverbOwner = 0;
    groundSubject = 0;
    debugPage = -1;
    reverbEnabled = 1;
}

// 0x00402f00
AuralScape* AuralScape::UnknownFunction402f00(void* target, int sounds) {
    if (!GameObject::UnknownVirtualSlot8(target))
        return 0;
    soundBudget = sounds;
    int ok = allEmitters.Init(8, 16) && audibleEmitters.Init(8, 16) && playingEmitters.Init(8, 16) && waitingEmitters.Init(8, 16);
    if (!ok) {
        Release();
        return 0;
    }
    return this;
}

// 0x00403000
int AuralScape::NextListenerNumber() {
    return ++listenerNumber - 1;
}

// 0x00403010
int AuralScape::AddListener(int index, float doppler, float rolloff) {
    listeners[index] = new (__FILE__, 717) AuralScapeListener();
    if (listeners[index] && listeners[index]->Initialize(this, doppler, rolloff)) {
        listenerCount++;
        return 1;
    }
    delete listeners[index];
    listeners[index] = 0;
    return 0;
}

// 0x004030a0
void AuralScape::SetListener(int index, Vector3* position, Vector3* front, Vector3* top,
                                       Vector3* velocity) {
    if (listeners[index]) {
        if (position)
            listeners[index]->SetPosition(*position);
        if (front && top)
            listeners[index]->SetOrientation(*front, *top);
        if (velocity)
            listeners[index]->SetVelocity(*velocity);
    }
}

// The volume of `emitter` from its nearest listener (pan is always 0).
static inline void UnknownPlaceEmitter(const int& count, AuralScapeListener** listeners, SoundEmitter* emitter) {
    int j;
    for (j = 0; j < count; j++) {
        listeners[j]->GetPosition(&s_UnknownListenerPositions5776f8[j]);
        float dx = s_UnknownListenerPositions5776f8[j].x - emitter->soundParameters.position.x;
        if (dx < 0.0f)
            dx = -dx;
        float dz = s_UnknownListenerPositions5776f8[j].z - emitter->soundParameters.position.z;
        if (dz < 0.0f)
            dz = -dz;
        s_UnknownListenerDistances57772c[j] = FastSqrt(dx * dx + dz * dz);
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
    VolumeForDistance(&listener, &emitter->soundParameters.position, nearest,
                          emitter->soundParameters.minDistance, emitter->soundParameters.maxDistance, &volume,
                          &pan);
    emitter->SetVolume(volume);
    emitter->SetPan(pan);
}

// Inline in 0x00403150. With this helper and UnknownPlaceEmitter VC6's
// inline budget for 0x00403150 covers ContainerList::Reserve only in the
// last Add, as in retail.
inline void AuralScape::UnknownUpdateDistances() {
    for (int i = 0; i < allEmitters.m_count; i++)
        allEmitters.Get(i)->UpdateListenerDistance(listenerCount, listeners);
}

// 0x00403150
void AuralScape::PickEmittersToPlay() {
    int i;
    SoundEmitter* emitter;
    int waiting3D = 0;

    UnknownUpdateDistances();

    audibleEmitters.Clear();
    for (i = 0; i < allEmitters.m_count; i++) {
        emitter = allEmitters.Get(i);
        if (emitter->audible)
            audibleEmitters.Add(emitter);
    }

    playingEmitters.Clear();
    for (i = 0; i < allEmitters.m_count; i++) {
        emitter = allEmitters.Get(i);
        if (emitter->IsPlaying())
            playingEmitters.Add(emitter);
    }

    for (i = 0; i < allEmitters.m_count; i++) {
        emitter = allEmitters.Get(i);
        if (!emitter->audible) {
            emitter->Stop();
            playingEmitters.Remove(emitter);
        }
    }


    waitingEmitters.Clear();
    for (i = 0; i < audibleEmitters.m_count; i++) {
        emitter = audibleEmitters.Get(i);
        if (!emitter->IsPlaying()) {
            if (emitter->loadedSound) {
                emitter->Play();
            } else {
                waitingEmitters.Add(emitter);
                if (emitter->field_0x1a0_bit2)
                    waiting3D++;
            }
        }
    }
    qsort(waitingEmitters.m_data, waitingEmitters.m_count, sizeof(SoundEmitter*), UnknownCompare402d40);

    int available = SoundSystem()->soundCaps.freeHw3DAllBuffers + soundBudget - soundsLoaded;
    if (waiting3D > available) {
        qsort(allEmitters.m_data, allEmitters.m_count, sizeof(SoundEmitter*), UnknownCompare402d70);
        for (i = 0; i < allEmitters.m_count; i++) {
            emitter = allEmitters.Get(i);
            if (!emitter->IsPlaying() && (emitter->emitterFlags & 0x10)) {
                available++;
                emitter->ReleaseSound();
                soundsLoaded--;
                if (waiting3D >= available)
                    break;
            }
        }
    }

    for (i = 0; i < waitingEmitters.m_count; i++) {
        emitter = waitingEmitters.Get(i);
        if (available > 0) {
            if (emitter->LoadSound(0x10)) {
                soundsLoaded++;
                available--;
            }
        } else {
            emitter->LoadSound(0x20);
        }
    }


    for (i = 0; i < waitingEmitters.m_count; i++) {
        emitter = waitingEmitters.Get(i);
        emitter->Play();
        playingEmitters.Add(emitter);
    }

    for (i = 0; i < playingEmitters.m_count; i++) {
        emitter = playingEmitters.Get(i);
        if (emitter->emitterFlags & 0x20) {
            UnknownPlaceEmitter(listenerCount, listeners, emitter);
        }
    }
}

// 0x00403710
int AuralScape::UnknownVirtualSlot10(float frameTime) {
    PickEmittersToPlay();

    int zone = 0;
    unsigned long environment = 0;
    unsigned char material = 0;
    if (reverbEnabled && groundSubject && reverbOwner) {
        Vector3 position = ((UnknownAuralScapeTarget*)renderContext)->currentCamera->eyePosition;
        groundSubject->QueryGround(&position, 0, 0, (int)&material);
        zone = (material >> 4) & 3;
        environment = reverbOwner->field_0xa4->field_0x410[zone];
        SoundSystem()->SetEAXEnvironment(environment);
    }

    for (int i = 0; i < listenerCount; i++)
        listeners[i]->CommitDeferredSettings();

    DebugOverlay* overlay = g_TrackGame->debugOverlay;
    if (overlay) {
        if (debugPage < 0)
            debugPage = overlay->NewPage();
        g_TrackGame->debugOverlay->Title(debugPage, "Audio");
        g_TrackGame->debugOverlay->Print(debugPage, "Hardware");
        g_TrackGame->debugOverlay->Print(
            debugPage, "3D HW Buffers %d Free %d", SoundSystem()->soundCaps.maxHw3DAllBuffers,
            SoundSystem()->soundCaps.freeHw3DAllBuffers);
        g_TrackGame->debugOverlay->Print(
            debugPage, "2D HW Buffers %d Free %d", SoundSystem()->soundCaps.maxHwMixingAllBuffers,
            SoundSystem()->soundCaps.freeHwMixingAllBuffers);
        g_TrackGame->debugOverlay->Print(
            debugPage, "Bytes %d Free %d", SoundSystem()->soundCaps.totalHwMemBytes,
            SoundSystem()->soundCaps.freeHwMemBytes);
        g_TrackGame->debugOverlay->Print(debugPage, "");

        int sounds = 0;
        int j = 0;
        int playing = 0;
        int bytes = 0;
        for (; j < SoundSystem()->field_0x04.m_count; j++) {
            Sound* sound = (Sound*)SoundSystem()->field_0x04.Get(j);
            if (sound) {
                sounds++;
                if (sound->IsPlaying())
                    playing++;
                bytes += sound->bufferBytes;
            }
        }
        g_TrackGame->debugOverlay->Print(debugPage, "Sounds %d Playing %d", sounds,
                                                                 playing);
        g_TrackGame->debugOverlay->Print(debugPage, "");
        g_TrackGame->debugOverlay->Print(debugPage, "AuralScape");
        g_TrackGame->debugOverlay->Print(debugPage, "NumEmitters %d", allEmitters.m_count);
        g_TrackGame->debugOverlay->Print(debugPage, "Audible %d", audibleEmitters.m_count);
        int loaded = 0;
        for (int k = 0; k < allEmitters.m_count; k++) {
            if (allEmitters.Get(k)->loadedSound)
                loaded++;
        }
        g_TrackGame->debugOverlay->Print(debugPage, "Loaded %d", loaded);
        g_TrackGame->debugOverlay->Print(debugPage, "Playing %d", playingEmitters.m_count);
        g_TrackGame->debugOverlay->Print(debugPage, "");
        g_TrackGame->debugOverlay->Print(debugPage, "ReverbZone %d", zone);
        g_TrackGame->debugOverlay->Print(debugPage, "ReverbEnviro %d", environment);
        g_TrackGame->debugOverlay->Print(debugPage, "");
        g_TrackGame->debugOverlay->Print(debugPage, "Total Wave Memory %d", bytes);
        if (SoundSystem()->soundMemoryManager)
            g_TrackGame->debugOverlay->Print(debugPage, "SoundCacheBytes %d",
                                                                     SoundSystem()->soundMemoryManager->loadedBytes);
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00403b20
int AuralScape::AddEmitter(SoundEmitter* emitter) {
    if (!emitter)
        return 0;
    return allEmitters.Add(emitter);
}

// 0x00403bc0
int AuralScape::RemoveEmitter(SoundEmitter* emitter) {
    if (!emitter)
        return 0;
    return allEmitters.Remove(emitter);
}
