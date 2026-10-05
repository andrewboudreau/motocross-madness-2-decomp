#include <stdio.h>
#include <string.h>

#include "PCAudio.h"
#include "SoundInterface.h"

#include "DebugAlloc.h"
#include "TrackGame.h"

// PCAudio.cpp: the DirectSound sound interface. Names are provisional; the
// literal option names, the listener IID and the caps report's format string
// are the evidence for the roles described here.

// 0x00689938: devices recorded by the enumeration callback. A file-level
// global rather than a class static: declaring a static data member in
// SoundInterface.h shifts the compiler-generated `$E` names in every file
// that includes it, which breaks Game.cpp's initializer matches.
int g_UnknownGlobal689938;

// 0x004bddf0
UnknownPCAudioObject::UnknownPCAudioObject() {
    field_0x00 = 0;
    field_0x04 = 0;
    field_0x08 = 0;
    InitializeCriticalSection(&field_0x0c);
    field_0x28 = 0;
    field_0x24 = 0;
}

// 0x004bde40: signals the thread to stop and waits for it.
UnknownPCAudioObject::~UnknownPCAudioObject() {
    if (field_0x00) {
        SetEvent(field_0x04);
        WaitForSingleObject(field_0x00, INFINITE);
        if (field_0x00) {
            CloseHandle(field_0x00);
            field_0x00 = 0;
        }
    }
    if (field_0x04) {
        CloseHandle(field_0x04);
        field_0x04 = 0;
    }
    if (field_0x08) {
        CloseHandle(field_0x08);
        field_0x08 = 0;
    }
    DeleteCriticalSection(&field_0x0c);
}

// 0x004be280: an empty device record.
UnknownSoundDevice::UnknownSoundDevice() {
    memset(&guid, 0, sizeof(guid));
    strcpy(description, "");
    memset(&caps, 0, sizeof(caps));
}

// 0x004be2d0: records the device and its caps. The probe object is only
// released on success (retail behaviour).
int UnknownSoundDevice::UnknownFunction4be2d0(UnknownGuid* deviceGuid, const char* text) {
    if (!deviceGuid)
        return 0;
    guid = *deviceGuid;
    strncpy(description, text, 0x7f);
    UnknownDirectSound* sound = 0;
    if (DirectSoundCreate(deviceGuid, &sound, 0) < 0)
        goto failed;
    caps.size = sizeof(caps);
    if (sound->GetCaps(&caps) < 0)
        goto failed;
    if (sound)
        sound->Release();
    return 1;
failed:
    return 0;
}

// 0x004be370: reads the sound options; the bitfield stores are VC6's
// bitfield code shape.
PCSoundInterface::PCSoundInterface() {
    field_0x45c_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("AllowSoundHardware", 1);
    field_0x45c_bit1 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("AllowSoundEnumeration", 1);
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x460 = 0;
    field_0x464 = 0;
    field_0x468 = 0;
    field_0x46c = 0;
    field_0x3f8 = 0;
    memset(&field_0x3fc, 0, sizeof(field_0x3fc));
    field_0x45c_bit2 = 0;
    field_0x45c_bit3 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("AllowEAXExtension", 0);
    field_0x474 = 0;
    field_0x470 = 0;
}

// 0x004be4b0: releases the helpers and DirectSound objects, restoring the
// primary buffer's start-up volume first.
PCSoundInterface::~PCSoundInterface() {
    if (field_0x46c)
        delete field_0x46c;
    if (field_0x474) {
        field_0x474->Release();
        field_0x474 = 0;
    }
    if (field_0x470) {
        field_0x470->Release();
        field_0x470 = 0;
    }
    if (field_0x464) {
        field_0x464->Release();
        field_0x464 = 0;
    }
    if (field_0x468)
        field_0x468->SetVolume(field_0x3f8);
    if (field_0x468) {
        field_0x468->Release();
        field_0x468 = 0;
    }
    if (field_0x460) {
        field_0x460->Release();
        field_0x460 = 0;
    }
}

// 0x004be5a0: creates DirectSound (on the enumerated device with the most
// hardware mixing buffers when enumeration is allowed; the start-up clears
// that option first, so it never is), the primary buffer and the helper,
// sets the format and queries the listener. 0x8878000a is returned when
// already started or without a primary buffer.
long PCSoundInterface::UnknownFunction4be5a0(int rate, int stereo, int bits, int value, int allowEax) {
    long result;

    if (field_0x2c_bit0)
        return 0x8878000a;
    field_0x2c_bit0 = 0;
    field_0x45c_bit1 = 0;
    field_0x45c_bit3 = allowEax;
    if (field_0x45c_bit1) {
        g_UnknownGlobal689938 = 0;
        DirectSoundEnumerateA(UnknownEnumCallback, this);
        field_0x30 = g_UnknownGlobal689938;
        unsigned long best = field_0x38[0].caps.maxHwMixingAllBuffers;
        field_0x34 = field_0x38;
        for (int i = 0; i < g_UnknownGlobal689938; i++) {
            if (field_0x38[i].caps.maxHwMixingAllBuffers > best) {
                best = field_0x38[i].caps.maxHwMixingAllBuffers;
                field_0x34 = &field_0x38[i];
            }
        }
        g_UnknownGlobal689938 = 0;
        if ((result = DirectSoundCreate(&field_0x34->guid, &field_0x460, 0)) != 0)
            goto done;
    } else if ((result = DirectSoundCreate(0, &field_0x460, 0)) != 0) {
        goto done;
    }
    if ((result = field_0x460->SetCooperativeLevel(g_UnknownGlobal56e26c->field_0x31c, 2)) != 0)
        goto done;
    field_0x468 = UnknownFunction4beb10();
    if (!field_0x468)
        return 0x8878000a;
    field_0x46c = new (__FILE__, 2367) UnknownPCAudioObject;
    if (!field_0x46c || !field_0x46c->UnknownFunction4bdef0(value))
        return 0;
    field_0x2c_bit0 = 1;
    UnknownFunction4be850();
    field_0x04.Init(8, 8);
    field_0x18.Init(8, 8);
    field_0x468->GetVolume(&field_0x3f8);
    UnknownFunction4be910(rate, stereo, bits);
    result = field_0x468->QueryInterface(IID_IDirectSound3DListener, (void**)&field_0x464);
    if (result == 0)
        UnknownFunction4be9e0();
done:
    return result;
}

// 0x004be7b0: DirectSoundEnumerate callback; `context` is the interface.
int __stdcall PCSoundInterface::UnknownEnumCallback(UnknownGuid* guid, const char* description,
                                                    const char* module, void* context) {
    if (g_UnknownGlobal689938 >= 4)
        return 0;
    PCSoundInterface* sound = (PCSoundInterface*)context;
    if (sound->field_0x38[g_UnknownGlobal689938].UnknownFunction4be2d0(guid, description))
        g_UnknownGlobal689938++;
    return 1;
}

// 0x004be800: a new listener interface from the primary buffer, or 0.
UnknownSoundListener* PCSoundInterface::UnknownFunction4be800() {
    if (!field_0x2c_bit0 && !field_0x468)
        return 0;
    UnknownSoundListener* listener = 0;
    long result = field_0x468->QueryInterface(IID_IDirectSound3DListener, (void**)&listener);
    return result < 0 ? 0 : listener;
}

// 0x004be850
int PCSoundInterface::UnknownFunction4be850() {
    return UnknownFunction4be860(&field_0x3fc);
}

// 0x004be860: fills `caps` from DirectSound.
int PCSoundInterface::UnknownFunction4be860(UnknownSoundCaps* caps) {
    if (!field_0x2c_bit0)
        return 0;
    if (caps && field_0x460) {
        caps->size = sizeof(*caps);
        if (field_0x460->GetCaps(caps) < 0)
            return 0;
    }
    return 1;
}

// 0x004be8b0: formats the free hardware resources into a local buffer that
// is never output.
void PCSoundInterface::UnknownFunction4be8b0() {
    UnknownSoundCaps caps;
    char text[512];

    if (field_0x2c_bit0) {
        UnknownFunction4be860(&caps);
        sprintf(text,
                "\nSound Card reports:\n"
                "\tdwFreeHwMixingStaticBuffers %d (%d Currently Allocated)\n"
                "\tdwFreeHw3DStaticBuffers %d (%d Currently Allocated)\n"
                "\tdwFreeHwMemBytes %d (%d Currently Allocated)\n\n",
                caps.freeHwMixingStaticBuffers, caps.maxHwMixingStaticBuffers - caps.freeHwMixingStaticBuffers,
                caps.freeHw3DStaticBuffers, caps.maxHw3DStaticBuffers - caps.freeHw3DStaticBuffers,
                caps.freeHwMemBytes, caps.totalHwMemBytes - caps.freeHwMemBytes);
    }
}

// 0x004be9b0: sets the primary buffer's volume.
int PCSoundInterface::UnknownFunction4be9b0(long volume) {
    if (field_0x2c_bit0 && field_0x468)
        return field_0x468->SetVolume(volume) >= 0;
    return 0;
}

// 0x004be9e0: creates a sound group and a probe sound; with hardware sound
// allowed and mixing buffers available, a small 3D buffer that supports the
// EAX listener properties enables them with value 9.
void PCSoundInterface::UnknownFunction4be9e0() {
    if (!field_0x2c_bit0)
        return;
    field_0x470 = new (__FILE__, 2574) SoundGroup(1);
    field_0x474 = new (__FILE__, 2575) Sound(field_0x470, 1);
    if (field_0x470 && field_0x474 && field_0x45c_bit0 && field_0x3fc.maxHwMixingAllBuffers >= 1 &&
        field_0x474->UnknownFunction4bd540(field_0x474->field_0x0c, 0x400, 11025, 16, 2, 0, 1, 1, 16, 1) &&
        field_0x474->UnknownFunction4bc5f0(&DSPROPSETID_EAX_ListenerProperties, 0, 3)) {
        field_0x45c_bit2 = 1;
        UnknownFunction4bed00(9);
    }
}

// 0x004beb10: creates the primary buffer (3D and volume control), or 0.
UnknownSoundBuffer* PCSoundInterface::UnknownFunction4beb10() {
    UnknownSoundBuffer* buffer = 0;
    UnknownSoundBufferDesc desc = {0};

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x91;
    desc.bufferBytes = 0;
    long result = field_0x460->CreateSoundBuffer(&desc, &buffer, 0);
    return result < 0 ? 0 : buffer;
}

// 0x004beb80-0x004becd0: listener settings, deferred (flag 1) until
// 0x004beb80 commits them.
int PCSoundInterface::UnknownFunction4beb80() {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->CommitDeferredSettings() >= 0;
}

int PCSoundInterface::UnknownFunction4beba0(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetDistanceFactor(factor, 1) >= 0;
}

int PCSoundInterface::UnknownFunction4bebd0(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetDopplerFactor(factor, 1) >= 0;
}

int PCSoundInterface::UnknownFunction4bec00(float frontX, float frontY, float frontZ, float topX, float topY,
                                            float topZ) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetOrientation(frontX, frontY, frontZ, topX, topY, topZ, 1) >= 0;
}

int PCSoundInterface::UnknownFunction4bec50(float x, float y, float z) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetPosition(x, y, z, 1) >= 0;
}

int PCSoundInterface::UnknownFunction4bec90(float x, float y, float z) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetVelocity(x, y, z, 1) >= 0;
}

int PCSoundInterface::UnknownFunction4becd0(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return field_0x464->SetRolloffFactor(factor, 1) >= 0;
}

// 0x004bed00: sets EAX listener property 1 when EAX is available and allowed.
int PCSoundInterface::UnknownFunction4bed00(unsigned long value) {
    if (field_0x2c_bit0 && field_0x45c_bit2 && field_0x474 && field_0x45c_bit3)
        return field_0x474->UnknownFunction4bc640(&DSPROPSETID_EAX_ListenerProperties, 1, 0, 0, &value, 4);
    return 0;
}

// 0x004bed40: sets all 16 bytes of EAX listener property 0.
int PCSoundInterface::UnknownFunction4bed40(void* parameters) {
    if (field_0x2c_bit0 && field_0x45c_bit2 && field_0x474 && field_0x45c_bit3)
        return field_0x474->UnknownFunction4bc640(&DSPROPSETID_EAX_ListenerProperties, 0, 0, 0, parameters,
                                                  16);
    return 0;
}
