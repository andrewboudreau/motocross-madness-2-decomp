#include <stdio.h>
#include <string.h>

#include "PCAudio.h"
#include "SoundInterface.h"

#include <process.h>

#include "DebugAlloc.h"
#include "MemTag.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "UnknownResourceManager.h"

// PCAudio.cpp: the DirectSound sound interface. Names are provisional; the
// literal option names, the listener IID and the caps report's format string
// are the evidence for the roles described here.

// 0x00689938: devices recorded by the enumeration callback. A file-level
// global rather than a class static: declaring a static data member in
// SoundInterface.h shifts the compiler-generated `$E` names in every file
// that includes it, which breaks Game.cpp's initializer matches.
int g_UnknownGlobal689938;

// The started sound interface (Game+0x04). A macro: as an inline function
// VC6 allocates the two loads to different registers than retail.
#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

// Copies at most 0x103 characters of `name` to `to` and terminates them.
#define CopySoundName(to, name)                          \
    {                                                    \
        int length = strlen(name);                       \
        int count = length > 0x103 ? 0x103 : length;     \
        strncpy(to, name, count);                        \
        to[count] = 0;                                   \
    }

// 0x004bb630: waits for the half-played events and refills the half that
// just finished playing, until the stop event.
unsigned __stdcall UnknownSoundNotifier::UnknownThreadProc(void* context) {
    Sound* sound = (Sound*)context;
    HANDLE events[3];
    unsigned long result;

    events[0] = sound->field_0x3c->field_0x08;
    events[1] = sound->field_0x3c->field_0x0c;
    events[2] = sound->field_0x3c->field_0x10;
    while ((result = WaitForMultipleObjects(3, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0:
        case 1:
            if (sound && sound->field_0x0c) {
                if (!sound->field_0x3c->field_0x18_bit0) {
                    unsigned long play;
                    unsigned long write;
                    EnterCriticalSection(&sound->field_0x48);
                    if (sound->field_0x0c->GetCurrentPosition(&play, &write) >= 0) {
                        unsigned long half = sound->field_0x198 / 2;
                        unsigned long offset = half < play ? 0 : half;
                        if (offset != sound->field_0x44) {
                            sound->FillBufferFromStream(&sound->field_0x0c, sound->field_0x40, offset, half);
                            sound->field_0x44 = offset;
                        }
                    }
                    LeaveCriticalSection(&sound->field_0x48);
                } else {
                    sound->field_0x3c->field_0x18_bit0 = 0;
                }
            }
            break;
        case 2:
            goto stopped;
        }
    }
stopped:
    _endthreadex(0);
    return 0;
}

// 0x004bb720
UnknownSoundNotifier::UnknownSoundNotifier() {
    field_0x00 = 0;
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x14 = 0;
    field_0x18_bit0 = 1;
}

// 0x004bb740: creates the events and thread and asks for notifications at
// the start of each half of the buffer.
int UnknownSoundNotifier::StartRefillThread(Sound* sound) {
    UnknownNotifyPosition positions[2];
    unsigned threadId;

    field_0x00 = sound;
    field_0x08 = CreateEventA(0, 0, 0, 0);
    field_0x0c = CreateEventA(0, 0, 0, 0);
    field_0x10 = CreateEventA(0, 0, 0, 0);
    field_0x14 = (HANDLE)_beginthreadex(0, 0, UnknownThreadProc, sound, 0, &threadId);
    if (!field_0x08 || !field_0x0c || !field_0x10 || !field_0x14)
        return 0;
    if (sound->field_0x0c->QueryInterface(IID_IDirectSoundNotify, (void**)&field_0x04) < 0)
        return 0;
    positions[0].offset = 0;
    positions[0].event = field_0x08;
    positions[1].offset = sound->field_0x198 / 2;
    positions[1].event = field_0x0c;
    field_0x04->SetNotificationPositions(2, positions);
    return 1;
}

// 0x004bb810: stops the thread and closes the handles.
UnknownSoundNotifier::~UnknownSoundNotifier() {
    if (field_0x00) {
        if (field_0x14) {
            SetEvent(field_0x10);
            WaitForSingleObject(field_0x14, INFINITE);
            if (field_0x14) {
                CloseHandle(field_0x14);
                field_0x14 = 0;
            }
        }
        if (field_0x10) {
            CloseHandle(field_0x10);
            field_0x10 = 0;
        }
        if (field_0x08) {
            CloseHandle(field_0x08);
            field_0x08 = 0;
        }
        if (field_0x0c) {
            CloseHandle(field_0x0c);
            field_0x0c = 0;
        }
        if (field_0x04) {
            field_0x04->Release();
            field_0x04 = 0;
        }
    }
}

// 0x004bba10: an empty sound of `type` in `group`.
Sound::Sound(SoundGroup* group, int type) {
    field_0x08 = group;
    group->UnknownFunction401b50(this);
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0x14 = 0;
    field_0x20 = 0;
    field_0x24 = 0;
    field_0x28 = 0;
    memset(&field_0x2c, 0, sizeof(field_0x2c));
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    InitializeCriticalSection(&field_0x48);
    strcpy(field_0x60, "");
    field_0x168 = type;
    field_0x164 = 0;
    memset(&field_0x16a, 0, sizeof(field_0x16a));
    field_0x198 = 0;
    field_0x19c = 0;
    field_0x1a0 = 0;
    field_0x1a4 = 0;
    memset(&field_0x1a8, 0, sizeof(field_0x1a8));
    field_0x1f5_bit0 = 0;
    field_0x1f5_bit1 = 0;
    field_0x1f5_bit2 = 0;
    field_0x1f5_bit3 = 0;
    field_0x1e8 = 0;
    field_0x1ec = 0;
    field_0x1f0 = 0;
    field_0x1f4_bit0 = 0;
    field_0x1f4_bit1 = 0;
    field_0x1f4_bit2 = 0;
    field_0x1f4_bit3 = 0;
    field_0x1f4_bit4 = 0;
    field_0x1f4_bit5 = 0;
    field_0x1f4_bit6 = 0;
    field_0x1f4_bit7 = 0;
}

// 0x004bbb60: unloads the sound and unregisters it everywhere.
Sound::~Sound() {
    if (field_0x3c)
        delete field_0x3c;
    if (field_0x40 && !field_0x40->field_0x1c)
        delete field_0x40;
    field_0x08->UnknownFunction401be0(this);
    if (field_0x1e8 & 4) {
        if (field_0x1f5_bit0)
            SoundSystem()->soundMemoryManager->UnloadSound(this);
    } else {
        ReleaseBuffers();
    }
    DeleteCriticalSection(&field_0x48);
    if (field_0x28) {
        field_0x28->Release();
        field_0x28 = 0;
    }
    void* entry = g_UnknownResourceManager572b44->UnknownFunction4e93f0(this);
    if (entry)
        g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, 0);
    if (field_0x10)
        SoundSystem()->field_0x18.Remove(this);
    SoundSystem()->field_0x04.Remove(this);
}

// 0x004bbcd0: plays the sound (with its play flags; flag 2 asks for a
// hardware voice, retried in software).
int Sound::Play() {
    if (SoundSystem()->field_0x2c_bit0 && field_0x0c) {
        field_0x164 = UnknownFunction4bfa80();
        ApplyCachedSettings();
        field_0x1f4_bit3 = field_0x1f0 & 1;
        if ((field_0x1e8 & 0x20) && field_0x0c->Play(0, 0, field_0x1f0) < 0)
            goto failed;
        if (field_0x1f0 & 2) {
            if (field_0x0c->Play(0, field_0x168, field_0x1f0) < 0) {
                field_0x1f0 = (field_0x1f0 & ~2) | 4;
                if (field_0x0c->Play(0, field_0x168, field_0x1f0) < 0)
                    goto failed;
            }
        } else if (field_0x0c->Play(0, 0, field_0x1f0) < 0) {
            goto failed;
        }
        SoundSystem()->RefreshSoundCaps();
    }
    return 1;
failed:
    return 0;
}

// 0x004bbdc0: applies the cached frequency, volume, pan and 3D settings.
int Sound::ApplyCachedSettings() {
    if ((field_0x1ec & 0x20) && !SetFrequency(field_0x19c, 1))
        return 0;
    if ((field_0x1ec & 0x80) && !SetVolume(field_0x1a0, 1))
        return 0;
    if ((field_0x1ec & 0x40) && !SetPan(field_0x1a4, 1))
        return 0;
    if (field_0x1e8 & 8) {
        Set3DPosition(field_0x1a8.position, 1);
        Set3DVelocity(field_0x1a8.velocity, 1);
        Set3DDistanceRange(field_0x1a8.minDistance, field_0x1a8.maxDistance, 1);
        if (field_0x1e8 & 0x10) {
            Set3DConeAngles(field_0x1a8.insideConeAngle, field_0x1a8.outsideConeAngle, 1);
            Set3DConeOrientation(field_0x1a8.coneOrientation, 1);
            Set3DConeOutsideVolume(field_0x1a8.coneOutsideVolume, 1);
        }
    }
    return 1;
}

// 0x004bbef0: loads a PCM .wav from `stream`: reads the 44-byte header,
// creates the buffer (looping sounds try hardware first), registers the
// sound with the interface, fills static ones and makes `duplicates`
// copies of a one-shot static sound. Streamed sounds (flag 2) get a buffer
// of `streamBytes` (default 128000); deferred ones (flag 4) stop after the
// header.
int Sound::LoadWave(UnknownTextureStream* stream, unsigned long flags, unsigned long controls,
    int duplicates, int streamBytes) {
    if (!stream || !SoundSystem() || !SoundSystem()->field_0x2c_bit0)
        return 0;
    int is3D = (flags >> 3) & 1;
    field_0x1ec = GetBufferControlFlags(controls);
    field_0x1e8 = flags;
    field_0x1f5_bit0 = 0;
    ReleaseBuffers();
    strcpy(field_0x60, "");
    if (stream->field_0x1c)
        stream->UnknownFunction461340(stream->field_0x130, 0, 1);
    if (stream->UnknownFunction461640(&field_0x16a, 0x2c, 1) != 1)
        return 0;
    int stereo = field_0x16a.channels > 1;
    field_0x198 = field_0x16a.dataSize;
    if (field_0x1ec & 0x80)
        SetVolume(field_0x08->field_0x30, 0);
    if (field_0x1e8 & 2) {
        if (streamBytes == -1)
            field_0x198 = 0x1f400;
        else
            field_0x198 = streamBytes;
        field_0x44 = field_0x198 / 2;
    } else if (field_0x1e8 & 4) {
        return 1;
    }
    int isStatic = field_0x1e8 & 1;
    if (field_0x1e8 & 0x20) {
        if (!CreateBuffer(&field_0x0c, field_0x198, field_0x16a.samplesPerSec, field_0x16a.bitsPerSample,
                                   field_0x16a.blockAlign, stereo, is3D, isStatic, field_0x1ec, 1) &&
            !CreateBuffer(&field_0x0c, field_0x198, field_0x16a.samplesPerSec, field_0x16a.bitsPerSample,
                                   field_0x16a.blockAlign, stereo, is3D, isStatic, field_0x1ec, 0))
            goto failed;
    } else if (!CreateBuffer(&field_0x0c, field_0x198, field_0x16a.samplesPerSec,
                                      field_0x16a.bitsPerSample, field_0x16a.blockAlign, stereo, is3D, isStatic,
                                      field_0x1ec, 0)) {
        goto failed;
    }
    if (is3D) {
        if (!Query3DBuffer(field_0x0c, &field_0x10))
            goto failed;
        SoundSystem()->field_0x18.Add(this);
    }
    SoundSystem()->field_0x04.Add(this);
    if (!is3D)
        Set3DMode(2);
    if (field_0x1e8 & 3) {
        if (!FillBufferFromStream(&field_0x0c, stream, 0, field_0x198))
            goto failed;
        field_0x1f5_bit0 = 1;
    }
    if ((field_0x1e8 & 1) && duplicates > 0) {
        field_0x20 = new (__FILE__, 496) UnknownSoundBuffer*[duplicates];
        field_0x24 = duplicates;
        for (int i = 0; i < field_0x24; i++) {
            if (SoundSystem()->directSound->DuplicateSoundBuffer(field_0x0c, &field_0x20[i]) != 0)
                goto failed;
        }
    }
    return 1;
failed:
    for (int j = 0; j < field_0x24; j++) {
        if (field_0x20[j]) {
            field_0x20[j]->Release();
            field_0x20[j] = 0;
        }
    }
    if (field_0x0c) {
        field_0x0c->Release();
        field_0x0c = 0;
    }
    if (field_0x10) {
        field_0x10->Release();
        field_0x10 = 0;
    }
    return 0;
}

// 0x004bc320: loads the sound from `stream`, or from the file `name`.
// Static sounds (flags 1 and 4) are read completely; streamed sounds
// (flag 2) keep the stream.
int Sound::UnknownFunction4bc320(const char* name, UnknownTextureStream* stream, int flags, int a, int b, int c) {
    UnknownTextureStream* file;

    if (!SoundSystem() || !SoundSystem()->field_0x2c_bit0)
        return 0;
    if (!stream) {
        file = new (__FILE__, 544) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (!file->UnknownFunction460f50(name, "rb", 0)) {
            delete file;
            return 0;
        }
    } else {
        file = stream;
    }
    if (flags & 5) {
        if (!LoadWave(file, flags, a, b, -1))
            goto failed;
        if (!stream)
            delete file;
    } else if (flags & 2) {
        field_0x40 = file;
        if (!LoadWave(file, flags, a, b, c))
            goto failed;
    }
    CopySoundName(field_0x60, name);
    return 1;
failed:
    if (!stream)
        delete file;
    field_0x40 = 0;
    return 0;
}

// 0x004bc490: buffer control flags for sound flags 1 (volume), 2 (pan) and
// 4 (frequency).
unsigned long Sound::GetBufferControlFlags(unsigned long flags) {
    unsigned long controls = 0;
    if (flags & 1)
        controls = 0x80;
    if (flags & 2)
        controls |= 0x40;
    if (flags & 4)
        controls |= 0x20;
    return controls;
}

// 0x004bc4c0: shares `source`'s buffer.
int Sound::DuplicateFrom(Sound* source) {
    if (!source || !SoundSystem() || !SoundSystem()->field_0x2c_bit0)
        return 0;
    ReleaseBuffers();
    strcpy(field_0x60, "");
    UnknownSoundBuffer* duplicate = 0;
    if (!DuplicateBuffer(&duplicate, source))
        return 0;
    field_0x0c = duplicate;
    if (source->field_0x10) {
        if (field_0x10) {
            field_0x10->Release();
            field_0x10 = 0;
        }
        if (!Query3DBuffer(field_0x0c, &field_0x10))
            return 0;
    }
    field_0x28 = source;
    field_0x1f4_bit1 = 1;
    source->AddRef();
    CopySoundName(field_0x60, field_0x28->field_0x60);
    if (field_0x1ec & 0x80)
        SetVolume(field_0x08->field_0x30, 0);
    return 1;
}

// 0x004bc5f0: whether the buffer's property set supports `support` for
// property `id` of `set`.
int Sound::SupportsProperty(const UnknownGuid* set, unsigned long id, unsigned long support) {
    unsigned long supported;
    if ((field_0x14 || QueryPropertySet()) && field_0x14->QuerySupport(set, id, &supported) >= 0 &&
        (supported & support) == support)
        return 1;
    return 0;
}

// 0x004bc640: sets a property through the buffer's property set.
int Sound::SetProperty(const UnknownGuid* set, unsigned long id, void* instance,
    unsigned long instanceSize, void* data, unsigned long dataSize) {
    if (!SoundSystem() || !SoundSystem()->field_0x2c_bit0)
        return 0;
    if (!field_0x14 && !QueryPropertySet())
        return 0;
    return field_0x14->Set(set, id, instance, instanceSize, data, dataSize) >= 0;
}

// 0x004bc940: stops a playing sound (a streamed one is refilled from its
// start); `rewind` also moves the play position back to 0.
int Sound::Stop(int rewind) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && field_0x1f5_bit0) {
        unsigned long status;
        if (field_0x0c->GetStatus(&status) < 0)
            goto failed;
        if ((status & 1) == 1) {
            if (field_0x3c)
                delete field_0x3c;
            field_0x3c = 0;
            if (field_0x0c->Stop() < 0)
                goto failed;
            if (field_0x40 && rewind) {
                field_0x40->UnknownFunction461340(field_0x40->field_0x130 + 0x2c, 0, 1);
                EnterCriticalSection(&field_0x48);
                FillBufferFromStream(&field_0x0c, field_0x40, 0, field_0x198);
                LeaveCriticalSection(&field_0x48);
                field_0x44 = field_0x198 / 2;
                SetBufferPosition(0);
            }
            if (rewind && !SetBufferPosition(0))
                goto failed;
        }
    }
    if (field_0x1e8 & 4)
        EnterCriticalSection(&field_0x48);
    field_0x1f5_bit1 = 1;
    if (field_0x1e8 & 4)
        LeaveCriticalSection(&field_0x48);
    return 1;
failed:
    return 0;
}

// 0x004bca80: whether the sound is playing (or, streamed, waiting to).
int Sound::IsPlaying() {
    if (!SoundSystem()->field_0x2c_bit0)
        return 0;
    if (field_0x1e8 & 4) {
        EnterCriticalSection(&field_0x48);
        if (!field_0x1f5_bit0 && field_0x1f4_bit7) {
            LeaveCriticalSection(&field_0x48);
            return 1;
        }
    }
    if (field_0x0c && field_0x1f5_bit0) {
        unsigned long status;
        if (field_0x0c->GetStatus(&status) >= 0 && (status & 1) == 1) {
            if (field_0x1e8 & 4)
                LeaveCriticalSection(&field_0x48);
            return 1;
        }
    }
    if (field_0x1e8 & 4)
        LeaveCriticalSection(&field_0x48);
    return 0;
}

// 0x004bcb30: sets the frequency (100..100000 Hz) of the sound and its
// duplicates; unless `force`, only while playing and when it changes.
int Sound::SetFrequency(unsigned long frequency, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && field_0x1f5_bit0 &&
        (force || (field_0x19c != frequency && IsPlaying()))) {
        if (frequency < 100 || frequency > 100000)
            goto failed;
        if (field_0x0c->SetFrequency(frequency) < 0)
            goto failed;
        int i = 0;
        if (field_0x24 > 0) {
            do {
                if (field_0x20[i] && field_0x20[i]->SetFrequency(frequency) < 0)
                    goto failed;
                i++;
            } while (i < field_0x24);
        }
    }
    field_0x19c = frequency;
    return 1;
failed:
    return 0;
}

// 0x004bcbe0: sets the volume (-10000..0) of a 2D sound and its duplicates.
int Sound::SetVolume(long volume, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && !field_0x10 && field_0x1f5_bit0 &&
        (force || (field_0x1a0 != volume && IsPlaying()))) {
        if (volume >= -10000) {
            if (volume > 0)
                volume = 0;
        } else {
            volume = -10000;
        }
        if (field_0x0c->SetVolume(volume) < 0)
            goto failed;
        int i = 0;
        if (field_0x24 > 0) {
            do {
                if (field_0x20[i] && field_0x20[i]->SetVolume(volume) < 0)
                    goto failed;
                i++;
            } while (i < field_0x24);
        }
    }
    field_0x1a0 = volume;
    return 1;
failed:
    return 0;
}

// 0x004bcca0: sets the pan of a 2D sound and its duplicates.
int Sound::SetPan(long pan, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && !field_0x10 && field_0x1f5_bit0 &&
        (force || (field_0x1a4 != pan && IsPlaying()))) {
        if (field_0x0c->SetPan(pan) < 0)
            goto failed;
        int i = 0;
        if (field_0x24 > 0) {
            do {
                if (field_0x20[i] && field_0x20[i]->SetPan(pan) < 0)
                    goto failed;
                i++;
            } while (i < field_0x24);
        }
    }
    field_0x1a4 = pan;
    return 1;
failed:
    return 0;
}

// 0x004bcd40
int Sound::GetBufferPosition(unsigned long* play, unsigned long* write) {
    if (!SoundSystem()->field_0x2c_bit0 || !field_0x0c)
        return 1;
    return field_0x0c->GetCurrentPosition(play, write) >= 0;
}

// 0x004bcd80
int Sound::SetBufferPosition(unsigned long position) {
    if (!SoundSystem()->field_0x2c_bit0 || !field_0x0c)
        return 1;
    return field_0x0c->SetCurrentPosition(position) >= 0;
}

// 0x004bcdc0: restores a lost buffer and refills a static or archived one.
int Sound::RestoreBuffer() {
    unsigned long status;
    if (field_0x0c) {
        field_0x0c->GetStatus(&status);
        if (status == 2) {
            if (field_0x0c->Restore() < 0)
                return 0;
            if ((field_0x1e8 & 5) && field_0x0c)
                FillBufferFromFile(&field_0x0c);
        }
    }
    return 1;
}

// 0x004bce20: pauses (stopping a playing sound) or resumes it.
int Sound::SetPaused(int paused) {
    if (paused) {
        if (!field_0x1f5_bit3) {
            field_0x1f4_bit4 = IsPlaying();
            if (field_0x1f4_bit4)
                Stop(0);
        }
    } else if (field_0x1f5_bit3 && field_0x1f4_bit4) {
        Play();
        field_0x1f4_bit4 = 0;
    }
    field_0x1f5_bit3 = paused;
    return 1;
}

// 0x004bcea0: advances a fade by `elapsed` seconds; a fade-out stops the
// sound at its target volume, a fade-in ends at full volume.
int Sound::UpdateFade(float elapsed) {
    if (field_0x1f4_bit5) {
        int step = 0;
        if (field_0x2c.length)
            step = (int)(elapsed * 1000.0f / field_0x2c.length * 10000.0f);
        if (field_0x2c.flags & 2) {
            field_0x38 -= step;
            if (field_0x38 < field_0x2c.target) {
                Stop(field_0x2c.flags & 1);
                field_0x38 = 0;
                field_0x1f4_bit5 = 0;
            }
        } else {
            field_0x38 += step;
            if (field_0x38 > 0) {
                field_0x38 = 0;
                field_0x1f4_bit5 = 0;
            }
        }
        SetVolume(field_0x38, 0);
    }
    return 1;
}

// 0x004bcf50: releases the buffers. A streamed sound being filled spins
// until the fill ends (retail reads the flag once, so it never ends).
void Sound::ReleaseBuffers() {
    EnterCriticalSection(&field_0x48);
    if (field_0x1e8 & 4) {
        while (field_0x1f5_bit2) {
        }
    }
    for (int i = 0; i < field_0x24; i++) {
        if (field_0x20[i]) {
            field_0x20[i]->Release();
            field_0x20[i] = 0;
        }
    }
    if (field_0x20) {
        delete field_0x20;
        field_0x20 = 0;
    }
    if (field_0x14) {
        field_0x14->Release();
        field_0x14 = 0;
    }
    if (field_0x10) {
        field_0x10->Release();
        field_0x10 = 0;
    }
    if (field_0x0c) {
        field_0x0c->Release();
        field_0x0c = 0;
    }
    g_MemTagStack->UnknownFunction4a2e00(field_0x198);
    field_0x1f5_bit0 = 0;
    field_0x1f4_bit6 = 0;
    LeaveCriticalSection(&field_0x48);
}

// 0x004bd020: locks part of the buffer.
int Sound::LockBuffer(unsigned long offset, unsigned long bytes, void** first,
    unsigned long* firstBytes, void** second, unsigned long* secondBytes,
    unsigned long flags) {
    if (!field_0x0c) {
        *first = 0;
        *second = 0;
        return 0;
    }
    return field_0x0c->Lock(offset, bytes, first, firstBytes, second, secondBytes, flags) >= 0;
}

// 0x004bd080
int Sound::UnlockBuffer(void* first, unsigned long firstBytes, void* second, unsigned long secondBytes) {
    if (field_0x0c && first)
        return field_0x0c->Unlock(first, firstBytes, second, secondBytes) >= 0;
    return 0;
}

// 0x004bd260: copies `bytes` of the stream into `buffer` at `offset`. At the
// end of the data a looping sound rewinds to the data start; any other is
// silenced and stopped.
int Sound::FillBufferFromStream(UnknownSoundBuffer** buffer, UnknownTextureStream* stream, unsigned long offset,
    unsigned long bytes) {
    void* first;
    unsigned long firstBytes;
    void* second;
    unsigned long secondBytes;

    if (!stream || !buffer)
        return 0;
    field_0x1f5_bit2 = 1;
    if ((*buffer)->Lock(offset, bytes, &first, &firstBytes, &second, &secondBytes, 0) != 0)
        return 0;
    if (firstBytes > 0) {
        if (stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461340(stream->field_0x130 + 0x2c, 0, 1);
            if (!field_0x1f4_bit3)
                goto ended;
        }
        stream->UnknownFunction461640(first, firstBytes, 1);
    }
    if (secondBytes > 0) {
        if (stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461340(stream->field_0x130 + 0x2c, 0, 1);
            if (!field_0x1f4_bit3) {
            ended:
                memset(first, 0, firstBytes);
                memset(second, 0, secondBytes);
                field_0x44 = 0;
                (*buffer)->SetCurrentPosition(0);
                (*buffer)->Stop();
            }
            (*buffer)->Unlock(first, firstBytes, second, secondBytes);
            field_0x1f5_bit2 = 0;
            return 1;
        }
        stream->UnknownFunction461640(second, secondBytes, 1);
    }
    long result = (*buffer)->Unlock(first, firstBytes, second, secondBytes);
    field_0x1f5_bit2 = 0;
    if (result != 0)
        return 0;
    field_0x1f5_bit0 = 1;
    return 1;
}

// 0x004bd4b0: creates the buffer a streamed sound is loaded into.
int Sound::CreatePendingBuffer() {
    ReleaseBuffers();
    int stereo = field_0x16a.channels > 1;
    int is3D = (field_0x1e8 >> 3) & 1;
    if (!CreateBuffer(&field_0x18, field_0x198, field_0x16a.samplesPerSec, field_0x16a.bitsPerSample,
                               field_0x16a.blockAlign, stereo, is3D, 1, field_0x1ec, 0))
        goto failed;
    if (is3D && !Query3DBuffer(field_0x18, &field_0x1c))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004bd540: creates a PCM buffer: static ones in software, others in
// hardware or deferred, 3D ones with the 3D algorithm.
int Sound::CreateBuffer(UnknownSoundBuffer** buffer, unsigned long bytes, unsigned long rate,
    int bits, int blockAlign, int stereo, int is3D,
    int isStatic, unsigned long flags, int hardware) {
    UnknownPcmFormat format;
    UnknownSoundBufferDesc desc;

    memset(&format, 0, sizeof(format));
    format.formatTag = 1;
    format.channels = (stereo != 0) + 1;
    format.samplesPerSec = rate;
    format.blockAlign = blockAlign;
    format.avgBytesPerSec = format.blockAlign * rate;
    field_0x19c = rate;
    format.bitsPerSample = bits;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x1e8 & 2)
        desc.flags = flags | 0x10108;
    else if (hardware)
        desc.flags = flags | 0x10004;
    else
        desc.flags = flags | 0x50000;
    if (is3D) {
        desc.flags |= 0x20010;
        desc.algorithm3D = g_UnknownSound3DAlgorithm;
    }
    if (isStatic)
        desc.flags |= 2;
    desc.bufferBytes = bytes;
    desc.format = (UnknownWaveFormat*)&format;
    if (SoundSystem()->directSound->CreateSoundBuffer(&desc, buffer, 0) < 0)
        return 0;
    g_MemTagStack->UnknownFunction4a2de0(bytes);
    field_0x198 = bytes;
    SoundSystem()->RefreshSoundCaps();
    return 1;
}

// 0x004bd6a0
int Sound::DuplicateBuffer(UnknownSoundBuffer** duplicate, Sound* source) {
    if (!source || !source->field_0x0c)
        return 0;
    return SoundSystem()->directSound->DuplicateSoundBuffer(source->field_0x0c, duplicate) >= 0;
}

// 0x004bd6e0
int Sound::Query3DBuffer(UnknownSoundBuffer* buffer, UnknownSound3DBuffer** buffer3D) {
    if (!buffer)
        goto failed;
    if (buffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)buffer3D) < 0)
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004bd710
int Sound::QueryPropertySet() {
    if (!field_0x0c)
        return 0;
    if (field_0x0c->QueryInterface(IID_IKsPropertySet, (void**)&field_0x14) < 0)
        return 0;
    return field_0x14 != 0;
}

// 0x004bd740
int Sound::Set3DParameters(UnknownSound3DParameters* parameters) {
    if (SoundSystem()->field_0x2c_bit0 && field_0x0c && field_0x10 && field_0x1f5_bit0) {
        parameters->size = sizeof(*parameters);
        if (field_0x10->SetAllParameters(parameters, 1) < 0)
            return 0;
    }
    return 1;
}

// 0x004bd790
int Sound::Set3DMode(unsigned long mode) {
    if (SoundSystem()->field_0x2c_bit0 && field_0x0c && field_0x10 && field_0x1f5_bit0) {
        if (field_0x10->SetMode(mode, 1) < 0)
            return 0;
    }
    return 1;
}

// 0x004bd7e0-0x004bdb60: cache a 3D setting and, when forced or changed
// while playing, apply it (deferred).
int Sound::Set3DPosition(Vector3 position, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || ((field_0x1a8.position.x != position.x || field_0x1a8.position.y != position.y ||
                        field_0x1a8.position.z != position.z) &&
                       IsPlaying()))) {
            if (field_0x10->SetPosition(position.x, position.y, position.z, 1) < 0)
                return 0;
        }
        field_0x1a8.position = position;
    }
    return 1;
}

int Sound::Set3DVelocity(Vector3 velocity, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || ((field_0x1a8.velocity.x != velocity.x || field_0x1a8.velocity.y != velocity.y ||
                        field_0x1a8.velocity.z != velocity.z) &&
                       IsPlaying()))) {
            if (field_0x10->SetVelocity(velocity.x, velocity.y, velocity.z, 1) < 0)
                return 0;
        }
        field_0x1a8.velocity = velocity;
    }
    return 1;
}

int Sound::Set3DDistanceRange(float minDistance, float maxDistance, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || ((field_0x1a8.minDistance != minDistance || field_0x1a8.maxDistance != maxDistance) &&
                       IsPlaying()))) {
            if (field_0x10->SetMinDistance(minDistance, 1) < 0 || field_0x10->SetMaxDistance(maxDistance, 1) < 0)
                return 0;
        }
        field_0x1a8.minDistance = minDistance;
        field_0x1a8.maxDistance = maxDistance;
    }
    return 1;
}

int Sound::Set3DConeAngles(unsigned long insideConeAngle, unsigned long outsideConeAngle, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || ((field_0x1a8.insideConeAngle != insideConeAngle ||
                        field_0x1a8.outsideConeAngle != outsideConeAngle) &&
                       IsPlaying()))) {
            if (field_0x10->SetConeAngles(insideConeAngle, outsideConeAngle, 1) < 0)
                return 0;
        }
        field_0x1a8.insideConeAngle = insideConeAngle;
        field_0x1a8.outsideConeAngle = outsideConeAngle;
    }
    return 1;
}

int Sound::Set3DConeOrientation(Vector3 orientation, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || ((field_0x1a8.coneOrientation.x != orientation.x ||
                        field_0x1a8.coneOrientation.y != orientation.y ||
                        field_0x1a8.coneOrientation.z != orientation.z) &&
                       IsPlaying()))) {
            if (field_0x10->SetConeOrientation(orientation.x, orientation.y, orientation.z, 1) < 0)
                return 0;
        }
        field_0x1a8.coneOrientation = orientation;
    }
    return 1;
}

int Sound::Set3DConeOutsideVolume(long volume, int force) {
    if (SoundSystem()->field_0x2c_bit0) {
        if (field_0x0c && field_0x10 && field_0x1f5_bit0 &&
            (force || (field_0x1a8.coneOutsideVolume != volume && IsPlaying()))) {
            if (field_0x10->SetConeOutsideVolume(volume, 1) < 0)
                return 0;
        }
        field_0x1a8.coneOutsideVolume = volume;
    }
    return 1;
}

// 0x004bdbd0: qsort order of sounds, least recently played first.
static int __cdecl CompareSounds(const void* a, const void* b) {
    Sound* first = *(Sound**)a;
    Sound* second = *(Sound**)b;
    if (first->field_0x164 < second->field_0x164)
        return -1;
    return first->field_0x164 != second->field_0x164;
}

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

// 0x004bdef0: starts the loader thread with a budget of `budget` bytes.
int UnknownPCAudioObject::StartLoaderThread(int budget) {
    unsigned threadId;

    field_0x24 = budget;
    field_0x08 = CreateEventA(0, 0, 0, 0);
    field_0x04 = CreateEventA(0, 0, 0, 0);
    field_0x00 = (HANDLE)_beginthreadex(0, 0, UnknownThreadProc, this, 0, &threadId);
    if (!field_0x08 || !field_0x04 || !field_0x00)
        return 0;
    SetThreadPriority(field_0x00, THREAD_PRIORITY_BELOW_NORMAL);
    if (!field_0x2c.Init(8, 16))
        return 0;
    return field_0x40.Init(8, 16);
}

// 0x004bdfc0: queues a streamed sound whose buffer is not ready.
void UnknownPCAudioObject::QueueSoundLoad(Sound* sound) {
    if (sound && (sound->field_0x1e8 & 4)) {
        EnterCriticalSection(&sound->field_0x48);
        int ready = sound->field_0x1f5_bit0;
        LeaveCriticalSection(&sound->field_0x48);
        if (!ready) {
            EnterCriticalSection(&field_0x0c);
            field_0x2c.Add(sound);
            LeaveCriticalSection(&field_0x0c);
            SetEvent(field_0x08);
        }
    }
}

// 0x004be0a0
void UnknownPCAudioObject::RecordLoadedSound(Sound* sound, int bytes) {
    field_0x28 += bytes;
    field_0x40.Add(sound);
}

// 0x004be130: unloads the least recently played idle sounds until `bytes`
// more fit in the budget.
void UnknownPCAudioObject::MakeRoomForSound(int bytes) {
    unsigned long total = field_0x28 + bytes;
    if (total < field_0x24)
        return;
    qsort(field_0x40.m_data, field_0x40.m_count, sizeof(Sound*), CompareSounds);
    for (int i = 0; i < field_0x40.m_count; i++) {
        Sound* sound = field_0x40.Get(i);
        if (sound) {
            int queued = 0;
            for (int j = 0; j < field_0x2c.m_count; j++) {
                if (field_0x40.Get(j) == sound)
                    queued = 1;
            }
            if (!sound->IsPlaying() && !queued) {
                UnloadSound(sound);
                total -= sound->field_0x198;
                i--;
            }
            if (total < field_0x24)
                return;
        }
    }
}

// 0x004be220: releases a sound's buffers and forgets it.
void UnknownPCAudioObject::UnloadSound(Sound* sound) {
    sound->IsPlaying();
    sound->ReleaseBuffers();
    field_0x28 -= sound->field_0x198;
    field_0x40.Remove(sound);
}

// 0x004be280: an empty device record.
UnknownSoundDevice::UnknownSoundDevice() {
    memset(&guid, 0, sizeof(guid));
    strcpy(description, "");
    memset(&caps, 0, sizeof(caps));
}

// 0x004be2d0: records the device and its caps. The probe object is only
// released on success (retail behaviour).
int UnknownSoundDevice::ProbeSoundDevice(UnknownGuid* deviceGuid, const char* text) {
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
    allowSoundHardware = g_TrackGame->GetRegistryFlag("AllowSoundHardware", 1);
    allowSoundEnumeration = g_TrackGame->GetRegistryFlag("AllowSoundEnumeration", 1);
    soundDeviceCount = 0;
    selectedSoundDevice = 0;
    directSound = 0;
    listener = 0;
    primaryBuffer = 0;
    soundMemoryManager = 0;
    startupPrimaryVolume = 0;
    memset(&soundCaps, 0, sizeof(soundCaps));
    eaxAvailable = 0;
    allowEAXExtension = g_TrackGame->GetRegistryFlag("AllowEAXExtension", 0);
    eaxProbeSound = 0;
    eaxProbeGroup = 0;
}

// 0x004be4b0: releases the helpers and DirectSound objects, restoring the
// primary buffer's start-up volume first.
PCSoundInterface::~PCSoundInterface() {
    if (soundMemoryManager)
        delete soundMemoryManager;
    if (eaxProbeSound) {
        eaxProbeSound->Release();
        eaxProbeSound = 0;
    }
    if (eaxProbeGroup) {
        eaxProbeGroup->Release();
        eaxProbeGroup = 0;
    }
    if (listener) {
        listener->Release();
        listener = 0;
    }
    if (primaryBuffer)
        primaryBuffer->SetVolume(startupPrimaryVolume);
    if (primaryBuffer) {
        primaryBuffer->Release();
        primaryBuffer = 0;
    }
    if (directSound) {
        directSound->Release();
        directSound = 0;
    }
}

// 0x004be5a0: creates DirectSound (on the enumerated device with the most
// hardware mixing buffers when enumeration is allowed; the start-up clears
// that option first, so it never is), the primary buffer and the helper,
// sets the format and queries the listener. 0x8878000a is returned when
// already started or without a primary buffer.
long PCSoundInterface::InitializeSound(int rate, int stereo, int bits, int value, int allowEax) {
    long result;

    if (field_0x2c_bit0)
        return 0x8878000a;
    field_0x2c_bit0 = 0;
    allowSoundEnumeration = 0;
    allowEAXExtension = allowEax;
    if (allowSoundEnumeration) {
        g_UnknownGlobal689938 = 0;
        DirectSoundEnumerateA(EnumSoundDeviceCallback, this);
        soundDeviceCount = g_UnknownGlobal689938;
        unsigned long best = soundDevices[0].caps.maxHwMixingAllBuffers;
        selectedSoundDevice = soundDevices;
        for (int i = 0; i < g_UnknownGlobal689938; i++) {
            if (soundDevices[i].caps.maxHwMixingAllBuffers > best) {
                best = soundDevices[i].caps.maxHwMixingAllBuffers;
                selectedSoundDevice = &soundDevices[i];
            }
        }
        g_UnknownGlobal689938 = 0;
        if ((result = DirectSoundCreate(&selectedSoundDevice->guid, &directSound, 0)) != 0)
            goto done;
    } else if ((result = DirectSoundCreate(0, &directSound, 0)) != 0) {
        goto done;
    }
    if ((result = directSound->SetCooperativeLevel(g_TrackGame->windowHandle, 2)) != 0)
        goto done;
    primaryBuffer = CreatePrimaryBuffer();
    if (!primaryBuffer)
        return 0x8878000a;
    soundMemoryManager = new (__FILE__, 2367) UnknownPCAudioObject;
    if (!soundMemoryManager || !soundMemoryManager->StartLoaderThread(value))
        return 0;
    field_0x2c_bit0 = 1;
    RefreshSoundCaps();
    field_0x04.Init(8, 8);
    field_0x18.Init(8, 8);
    primaryBuffer->GetVolume(&startupPrimaryVolume);
    SetPrimaryFormat(rate, stereo, bits);
    result = primaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void**)&listener);
    if (result == 0)
        ProbeEAXSupport();
done:
    return result;
}

// 0x004be7b0: DirectSoundEnumerate callback; `context` is the interface.
int __stdcall PCSoundInterface::EnumSoundDeviceCallback(UnknownGuid* guid, const char* description,
    const char* module, void* context) {
    if (g_UnknownGlobal689938 >= 4)
        return 0;
    PCSoundInterface* sound = (PCSoundInterface*)context;
    if (sound->soundDevices[g_UnknownGlobal689938].ProbeSoundDevice(guid, description))
        g_UnknownGlobal689938++;
    return 1;
}

// 0x004be800: a new listener interface from the primary buffer, or 0.
UnknownSoundListener* PCSoundInterface::QueryListener() {
    if (!field_0x2c_bit0 && !primaryBuffer)
        return 0;
    UnknownSoundListener* listener = 0;
    long result = primaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void**)&listener);
    return result < 0 ? 0 : listener;
}

// 0x004be850
int PCSoundInterface::RefreshSoundCaps() {
    return GetSoundCaps(&soundCaps);
}

// 0x004be860: fills `caps` from DirectSound.
int PCSoundInterface::GetSoundCaps(UnknownSoundCaps* caps) {
    if (!field_0x2c_bit0)
        return 0;
    if (caps && directSound) {
        caps->size = sizeof(*caps);
        if (directSound->GetCaps(caps) < 0)
            return 0;
    }
    return 1;
}

// 0x004be8b0: formats the free hardware resources into a local buffer that
// is never output.
void PCSoundInterface::FormatSoundCapsReport() {
    UnknownSoundCaps caps;
    char text[512];

    if (field_0x2c_bit0) {
        GetSoundCaps(&caps);
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
int PCSoundInterface::SetPrimaryVolume(long volume) {
    if (field_0x2c_bit0 && primaryBuffer)
        return primaryBuffer->SetVolume(volume) >= 0;
    return 0;
}

// 0x004be9e0: creates a sound group and a probe sound; with hardware sound
// allowed and mixing buffers available, a small 3D buffer that supports the
// EAX listener properties enables them with value 9.
void PCSoundInterface::ProbeEAXSupport() {
    if (!field_0x2c_bit0)
        return;
    eaxProbeGroup = new (__FILE__, 2574) SoundGroup(1);
    eaxProbeSound = new (__FILE__, 2575) Sound(eaxProbeGroup, 1);
    if (eaxProbeGroup && eaxProbeSound && allowSoundHardware && soundCaps.maxHwMixingAllBuffers >= 1 &&
        eaxProbeSound->CreateBuffer(&eaxProbeSound->field_0x0c, 0x400, 11025, 16, 2, 0, 1, 1, 16, 1) &&
        eaxProbeSound->SupportsProperty(&DSPROPSETID_EAX_ListenerProperties, 0, 3)) {
        eaxAvailable = 1;
        SetEAXEnvironment(9);
    }
}

// 0x004beb10: creates the primary buffer (3D and volume control), or 0.
UnknownSoundBuffer* PCSoundInterface::CreatePrimaryBuffer() {
    UnknownSoundBuffer* buffer = 0;
    UnknownSoundBufferDesc desc = {0};

    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = 0x91;
    desc.bufferBytes = 0;
    long result = directSound->CreateSoundBuffer(&desc, &buffer, 0);
    return result < 0 ? 0 : buffer;
}

// 0x004beb80-0x004becd0: listener settings, deferred (flag 1) until
// 0x004beb80 commits them.
int PCSoundInterface::CommitListenerSettings() {
    if (!field_0x2c_bit0)
        return 0;
    return listener->CommitDeferredSettings() >= 0;
}

int PCSoundInterface::SetListenerDistanceFactor(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetDistanceFactor(factor, 1) >= 0;
}

int PCSoundInterface::SetListenerDopplerFactor(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetDopplerFactor(factor, 1) >= 0;
}

int PCSoundInterface::SetListenerOrientation(Vector3 front, Vector3 top) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetOrientation(front.x, front.y, front.z, top.x, top.y, top.z, 1) >= 0;
}

int PCSoundInterface::SetListenerPosition(Vector3 position) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetPosition(position.x, position.y, position.z, 1) >= 0;
}

int PCSoundInterface::SetListenerVelocity(Vector3 velocity) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetVelocity(velocity.x, velocity.y, velocity.z, 1) >= 0;
}

int PCSoundInterface::SetListenerRolloffFactor(float factor) {
    if (!field_0x2c_bit0)
        return 0;
    return listener->SetRolloffFactor(factor, 1) >= 0;
}

// 0x004bed00: sets EAX listener property 1 when EAX is available and allowed.
int PCSoundInterface::SetEAXEnvironment(unsigned long value) {
    if (field_0x2c_bit0 && eaxAvailable && eaxProbeSound && allowEAXExtension)
        return eaxProbeSound->SetProperty(&DSPROPSETID_EAX_ListenerProperties, 1, 0, 0, &value, 4);
    return 0;
}

// 0x004bed40: sets all 16 bytes of EAX listener property 0.
int PCSoundInterface::SetEAXListenerParameters(void* parameters) {
    if (field_0x2c_bit0 && eaxAvailable && eaxProbeSound && allowEAXExtension)
        return eaxProbeSound->SetProperty(&DSPROPSETID_EAX_ListenerProperties, 0, 0, 0, parameters,
                                                  16);
    return 0;
}
