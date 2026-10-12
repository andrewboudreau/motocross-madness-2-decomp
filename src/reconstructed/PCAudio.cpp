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

    events[0] = sound->notifier->firstHalfEvent;
    events[1] = sound->notifier->secondHalfEvent;
    events[2] = sound->notifier->stopEvent;
    while ((result = WaitForMultipleObjects(3, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0:
        case 1:
            if (sound && sound->dsBuffer) {
                if (!sound->notifier->skipNotification) {
                    unsigned long play;
                    unsigned long write;
                    EnterCriticalSection(&sound->bufferLock);
                    if (sound->dsBuffer->GetCurrentPosition(&play, &write) >= 0) {
                        unsigned long half = sound->bufferBytes / 2;
                        unsigned long offset = half < play ? 0 : half;
                        if (offset != sound->filledHalf) {
                            sound->FillBufferFromStream(&sound->dsBuffer, sound->sourceStream, offset, half);
                            sound->filledHalf = offset;
                        }
                    }
                    LeaveCriticalSection(&sound->bufferLock);
                } else {
                    sound->notifier->skipNotification = 0;
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
    ownerSound = 0;
    notify = 0;
    firstHalfEvent = 0;
    secondHalfEvent = 0;
    stopEvent = 0;
    refillThread = 0;
    skipNotification = 1;
}

// 0x004bb740: creates the events and thread and asks for notifications at
// the start of each half of the buffer.
int UnknownSoundNotifier::StartRefillThread(Sound* sound) {
    UnknownNotifyPosition positions[2];
    unsigned threadId;

    ownerSound = sound;
    firstHalfEvent = CreateEventA(0, 0, 0, 0);
    secondHalfEvent = CreateEventA(0, 0, 0, 0);
    stopEvent = CreateEventA(0, 0, 0, 0);
    refillThread = (HANDLE)_beginthreadex(0, 0, UnknownThreadProc, sound, 0, &threadId);
    if (!firstHalfEvent || !secondHalfEvent || !stopEvent || !refillThread)
        return 0;
    if (sound->dsBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&notify) < 0)
        return 0;
    positions[0].offset = 0;
    positions[0].event = firstHalfEvent;
    positions[1].offset = sound->bufferBytes / 2;
    positions[1].event = secondHalfEvent;
    notify->SetNotificationPositions(2, positions);
    return 1;
}

// 0x004bb810: stops the thread and closes the handles.
UnknownSoundNotifier::~UnknownSoundNotifier() {
    if (ownerSound) {
        if (refillThread) {
            SetEvent(stopEvent);
            WaitForSingleObject(refillThread, INFINITE);
            if (refillThread) {
                CloseHandle(refillThread);
                refillThread = 0;
            }
        }
        if (stopEvent) {
            CloseHandle(stopEvent);
            stopEvent = 0;
        }
        if (firstHalfEvent) {
            CloseHandle(firstHalfEvent);
            firstHalfEvent = 0;
        }
        if (secondHalfEvent) {
            CloseHandle(secondHalfEvent);
            secondHalfEvent = 0;
        }
        if (notify) {
            notify->Release();
            notify = 0;
        }
    }
}

// 0x004bba10: an empty sound of `type` in `group`.
Sound::Sound(SoundGroup* group, int type) {
    soundGroup = group;
    group->UnknownFunction401b50(this);
    dsBuffer = 0;
    dsBuffer3D = 0;
    pendingBuffer = 0;
    pendingBuffer3D = 0;
    propertySet = 0;
    duplicateBuffers = 0;
    duplicateCount = 0;
    duplicateSource = 0;
    memset(&fade, 0, sizeof(fade));
    fadeVolume = 0;
    notifier = 0;
    sourceStream = 0;
    filledHalf = 0;
    InitializeCriticalSection(&bufferLock);
    strcpy(soundName, "");
    soundType = type;
    lastPlayTime = 0;
    memset(&waveHeader, 0, sizeof(waveHeader));
    bufferBytes = 0;
    currentFrequency = 0;
    currentVolume = 0;
    currentPan = 0;
    memset(&parameters3D, 0, sizeof(parameters3D));
    bufferReady = 0;
    isStopped = 0;
    isFilling = 0;
    isPaused = 0;
    soundFlags = 0;
    controlsSet = 0;
    lastPlayFlags = 0;
    isPacked = 0;
    isDuplicate = 0;
    field_0x1f4_bit2 = 0;
    isLooping = 0;
    pausedWhilePlaying = 0;
    isFading = 0;
    field_0x1f4_bit6 = 0;
    field_0x1f4_bit7 = 0;
}

// 0x004bbb60: unloads the sound and unregisters it everywhere.
Sound::~Sound() {
    if (notifier)
        delete notifier;
    if (sourceStream && !sourceStream->field_0x1c)
        delete sourceStream;
    soundGroup->UnknownFunction401be0(this);
    if (soundFlags & 4) {
        if (bufferReady)
            SoundSystem()->soundMemoryManager->UnloadSound(this);
    } else {
        ReleaseBuffers();
    }
    DeleteCriticalSection(&bufferLock);
    if (duplicateSource) {
        duplicateSource->Release();
        duplicateSource = 0;
    }
    void* entry = g_UnknownResourceManager572b44->UnknownFunction4e93f0(this);
    if (entry)
        g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, 0);
    if (dsBuffer3D)
        SoundSystem()->field_0x18.Remove(this);
    SoundSystem()->field_0x04.Remove(this);
}

// 0x004bbcd0: plays the sound (with its play flags; flag 2 asks for a
// hardware voice, retried in software).
int Sound::Play() {
    if (SoundSystem()->audioStarted && dsBuffer) {
        lastPlayTime = UnknownFunction4bfa80();
        ApplyCachedSettings();
        isLooping = lastPlayFlags & 1;
        if ((soundFlags & 0x20) && dsBuffer->Play(0, 0, lastPlayFlags) < 0)
            goto failed;
        if (lastPlayFlags & 2) {
            if (dsBuffer->Play(0, soundType, lastPlayFlags) < 0) {
                lastPlayFlags = (lastPlayFlags & ~2) | 4;
                if (dsBuffer->Play(0, soundType, lastPlayFlags) < 0)
                    goto failed;
            }
        } else if (dsBuffer->Play(0, 0, lastPlayFlags) < 0) {
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
    if ((controlsSet & 0x20) && !SetFrequency(currentFrequency, 1))
        return 0;
    if ((controlsSet & 0x80) && !SetVolume(currentVolume, 1))
        return 0;
    if ((controlsSet & 0x40) && !SetPan(currentPan, 1))
        return 0;
    if (soundFlags & 8) {
        Set3DPosition(parameters3D.position, 1);
        Set3DVelocity(parameters3D.velocity, 1);
        Set3DDistanceRange(parameters3D.minDistance, parameters3D.maxDistance, 1);
        if (soundFlags & 0x10) {
            Set3DConeAngles(parameters3D.insideConeAngle, parameters3D.outsideConeAngle, 1);
            Set3DConeOrientation(parameters3D.coneOrientation, 1);
            Set3DConeOutsideVolume(parameters3D.coneOutsideVolume, 1);
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
    if (!stream || !SoundSystem() || !SoundSystem()->audioStarted)
        return 0;
    int is3D = (flags >> 3) & 1;
    controlsSet = GetBufferControlFlags(controls);
    soundFlags = flags;
    bufferReady = 0;
    ReleaseBuffers();
    strcpy(soundName, "");
    if (stream->field_0x1c)
        stream->UnknownFunction461340(stream->field_0x130, 0, 1);
    if (stream->UnknownFunction461640(&waveHeader, 0x2c, 1) != 1)
        return 0;
    int stereo = waveHeader.channels > 1;
    bufferBytes = waveHeader.dataSize;
    if (controlsSet & 0x80)
        SetVolume(soundGroup->field_0x30, 0);
    if (soundFlags & 2) {
        if (streamBytes == -1)
            bufferBytes = 0x1f400;
        else
            bufferBytes = streamBytes;
        filledHalf = bufferBytes / 2;
    } else if (soundFlags & 4) {
        return 1;
    }
    int isStatic = soundFlags & 1;
    if (soundFlags & 0x20) {
        if (!CreateBuffer(&dsBuffer, bufferBytes, waveHeader.samplesPerSec, waveHeader.bitsPerSample,
                                   waveHeader.blockAlign, stereo, is3D, isStatic, controlsSet, 1) &&
            !CreateBuffer(&dsBuffer, bufferBytes, waveHeader.samplesPerSec, waveHeader.bitsPerSample,
                                   waveHeader.blockAlign, stereo, is3D, isStatic, controlsSet, 0))
            goto failed;
    } else if (!CreateBuffer(&dsBuffer, bufferBytes, waveHeader.samplesPerSec,
                                      waveHeader.bitsPerSample, waveHeader.blockAlign, stereo, is3D, isStatic,
                                      controlsSet, 0)) {
        goto failed;
    }
    if (is3D) {
        if (!Query3DBuffer(dsBuffer, &dsBuffer3D))
            goto failed;
        SoundSystem()->field_0x18.Add(this);
    }
    SoundSystem()->field_0x04.Add(this);
    if (!is3D)
        Set3DMode(2);
    if (soundFlags & 3) {
        if (!FillBufferFromStream(&dsBuffer, stream, 0, bufferBytes))
            goto failed;
        bufferReady = 1;
    }
    if ((soundFlags & 1) && duplicates > 0) {
        duplicateBuffers = new (__FILE__, 496) UnknownSoundBuffer*[duplicates];
        duplicateCount = duplicates;
        for (int i = 0; i < duplicateCount; i++) {
            if (SoundSystem()->directSound->DuplicateSoundBuffer(dsBuffer, &duplicateBuffers[i]) != 0)
                goto failed;
        }
    }
    return 1;
failed:
    for (int j = 0; j < duplicateCount; j++) {
        if (duplicateBuffers[j]) {
            duplicateBuffers[j]->Release();
            duplicateBuffers[j] = 0;
        }
    }
    if (dsBuffer) {
        dsBuffer->Release();
        dsBuffer = 0;
    }
    if (dsBuffer3D) {
        dsBuffer3D->Release();
        dsBuffer3D = 0;
    }
    return 0;
}

// 0x004bc320: loads the sound from `stream`, or from the file `name`.
// Static sounds (flags 1 and 4) are read completely; streamed sounds
// (flag 2) keep the stream.
int Sound::UnknownFunction4bc320(const char* name, UnknownTextureStream* stream, int flags, int a, int b, int c) {
    UnknownTextureStream* file;

    if (!SoundSystem() || !SoundSystem()->audioStarted)
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
        sourceStream = file;
        if (!LoadWave(file, flags, a, b, c))
            goto failed;
    }
    CopySoundName(soundName, name);
    return 1;
failed:
    if (!stream)
        delete file;
    sourceStream = 0;
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
    if (!source || !SoundSystem() || !SoundSystem()->audioStarted)
        return 0;
    ReleaseBuffers();
    strcpy(soundName, "");
    UnknownSoundBuffer* duplicate = 0;
    if (!DuplicateBuffer(&duplicate, source))
        return 0;
    dsBuffer = duplicate;
    if (source->dsBuffer3D) {
        if (dsBuffer3D) {
            dsBuffer3D->Release();
            dsBuffer3D = 0;
        }
        if (!Query3DBuffer(dsBuffer, &dsBuffer3D))
            return 0;
    }
    duplicateSource = source;
    isDuplicate = 1;
    source->AddRef();
    CopySoundName(soundName, duplicateSource->soundName);
    if (controlsSet & 0x80)
        SetVolume(soundGroup->field_0x30, 0);
    return 1;
}

// 0x004bc5f0: whether the buffer's property set supports `support` for
// property `id` of `set`.
int Sound::SupportsProperty(const UnknownGuid* set, unsigned long id, unsigned long support) {
    unsigned long supported;
    if ((propertySet || QueryPropertySet()) && propertySet->QuerySupport(set, id, &supported) >= 0 &&
        (supported & support) == support)
        return 1;
    return 0;
}

// 0x004bc640: sets a property through the buffer's property set.
int Sound::SetProperty(const UnknownGuid* set, unsigned long id, void* instance,
    unsigned long instanceSize, void* data, unsigned long dataSize) {
    if (!SoundSystem() || !SoundSystem()->audioStarted)
        return 0;
    if (!propertySet && !QueryPropertySet())
        return 0;
    return propertySet->Set(set, id, instance, instanceSize, data, dataSize) >= 0;
}

// 0x004bc940: stops a playing sound (a streamed one is refilled from its
// start); `rewind` also moves the play position back to 0.
int Sound::Stop(int rewind) {
    if (!SoundSystem()->audioStarted)
        return 1;
    if (dsBuffer && bufferReady) {
        unsigned long status;
        if (dsBuffer->GetStatus(&status) < 0)
            goto failed;
        if ((status & 1) == 1) {
            if (notifier)
                delete notifier;
            notifier = 0;
            if (dsBuffer->Stop() < 0)
                goto failed;
            if (sourceStream && rewind) {
                sourceStream->UnknownFunction461340(sourceStream->field_0x130 + 0x2c, 0, 1);
                EnterCriticalSection(&bufferLock);
                FillBufferFromStream(&dsBuffer, sourceStream, 0, bufferBytes);
                LeaveCriticalSection(&bufferLock);
                filledHalf = bufferBytes / 2;
                SetBufferPosition(0);
            }
            if (rewind && !SetBufferPosition(0))
                goto failed;
        }
    }
    if (soundFlags & 4)
        EnterCriticalSection(&bufferLock);
    isStopped = 1;
    if (soundFlags & 4)
        LeaveCriticalSection(&bufferLock);
    return 1;
failed:
    return 0;
}

// 0x004bca80: whether the sound is playing (or, streamed, waiting to).
int Sound::IsPlaying() {
    if (!SoundSystem()->audioStarted)
        return 0;
    if (soundFlags & 4) {
        EnterCriticalSection(&bufferLock);
        if (!bufferReady && field_0x1f4_bit7) {
            LeaveCriticalSection(&bufferLock);
            return 1;
        }
    }
    if (dsBuffer && bufferReady) {
        unsigned long status;
        if (dsBuffer->GetStatus(&status) >= 0 && (status & 1) == 1) {
            if (soundFlags & 4)
                LeaveCriticalSection(&bufferLock);
            return 1;
        }
    }
    if (soundFlags & 4)
        LeaveCriticalSection(&bufferLock);
    return 0;
}

// 0x004bcb30: sets the frequency (100..100000 Hz) of the sound and its
// duplicates; unless `force`, only while playing and when it changes.
int Sound::SetFrequency(unsigned long frequency, int force) {
    if (!SoundSystem()->audioStarted)
        return 1;
    if (dsBuffer && bufferReady &&
        (force || (currentFrequency != frequency && IsPlaying()))) {
        if (frequency < 100 || frequency > 100000)
            goto failed;
        if (dsBuffer->SetFrequency(frequency) < 0)
            goto failed;
        int i = 0;
        if (duplicateCount > 0) {
            do {
                if (duplicateBuffers[i] && duplicateBuffers[i]->SetFrequency(frequency) < 0)
                    goto failed;
                i++;
            } while (i < duplicateCount);
        }
    }
    currentFrequency = frequency;
    return 1;
failed:
    return 0;
}

// 0x004bcbe0: sets the volume (-10000..0) of a 2D sound and its duplicates.
int Sound::SetVolume(long volume, int force) {
    if (!SoundSystem()->audioStarted)
        return 1;
    if (dsBuffer && !dsBuffer3D && bufferReady &&
        (force || (currentVolume != volume && IsPlaying()))) {
        if (volume >= -10000) {
            if (volume > 0)
                volume = 0;
        } else {
            volume = -10000;
        }
        if (dsBuffer->SetVolume(volume) < 0)
            goto failed;
        int i = 0;
        if (duplicateCount > 0) {
            do {
                if (duplicateBuffers[i] && duplicateBuffers[i]->SetVolume(volume) < 0)
                    goto failed;
                i++;
            } while (i < duplicateCount);
        }
    }
    currentVolume = volume;
    return 1;
failed:
    return 0;
}

// 0x004bcca0: sets the pan of a 2D sound and its duplicates.
int Sound::SetPan(long pan, int force) {
    if (!SoundSystem()->audioStarted)
        return 1;
    if (dsBuffer && !dsBuffer3D && bufferReady &&
        (force || (currentPan != pan && IsPlaying()))) {
        if (dsBuffer->SetPan(pan) < 0)
            goto failed;
        int i = 0;
        if (duplicateCount > 0) {
            do {
                if (duplicateBuffers[i] && duplicateBuffers[i]->SetPan(pan) < 0)
                    goto failed;
                i++;
            } while (i < duplicateCount);
        }
    }
    currentPan = pan;
    return 1;
failed:
    return 0;
}

// 0x004bcd40
int Sound::GetBufferPosition(unsigned long* play, unsigned long* write) {
    if (!SoundSystem()->audioStarted || !dsBuffer)
        return 1;
    return dsBuffer->GetCurrentPosition(play, write) >= 0;
}

// 0x004bcd80
int Sound::SetBufferPosition(unsigned long position) {
    if (!SoundSystem()->audioStarted || !dsBuffer)
        return 1;
    return dsBuffer->SetCurrentPosition(position) >= 0;
}

// 0x004bcdc0: restores a lost buffer and refills a static or archived one.
int Sound::RestoreBuffer() {
    unsigned long status;
    if (dsBuffer) {
        dsBuffer->GetStatus(&status);
        if (status == 2) {
            if (dsBuffer->Restore() < 0)
                return 0;
            if ((soundFlags & 5) && dsBuffer)
                FillBufferFromFile(&dsBuffer);
        }
    }
    return 1;
}

// 0x004bce20: pauses (stopping a playing sound) or resumes it.
int Sound::SetPaused(int paused) {
    if (paused) {
        if (!isPaused) {
            pausedWhilePlaying = IsPlaying();
            if (pausedWhilePlaying)
                Stop(0);
        }
    } else if (isPaused && pausedWhilePlaying) {
        Play();
        pausedWhilePlaying = 0;
    }
    isPaused = paused;
    return 1;
}

// 0x004bcea0: advances a fade by `elapsed` seconds; a fade-out stops the
// sound at its target volume, a fade-in ends at full volume.
int Sound::UpdateFade(float elapsed) {
    if (isFading) {
        int step = 0;
        if (fade.length)
            step = (int)(elapsed * 1000.0f / fade.length * 10000.0f);
        if (fade.flags & 2) {
            fadeVolume -= step;
            if (fadeVolume < fade.target) {
                Stop(fade.flags & 1);
                fadeVolume = 0;
                isFading = 0;
            }
        } else {
            fadeVolume += step;
            if (fadeVolume > 0) {
                fadeVolume = 0;
                isFading = 0;
            }
        }
        SetVolume(fadeVolume, 0);
    }
    return 1;
}

// 0x004bcf50: releases the buffers. A streamed sound being filled spins
// until the fill ends (retail reads the flag once, so it never ends).
void Sound::ReleaseBuffers() {
    EnterCriticalSection(&bufferLock);
    if (soundFlags & 4) {
        while (isFilling) {
        }
    }
    for (int i = 0; i < duplicateCount; i++) {
        if (duplicateBuffers[i]) {
            duplicateBuffers[i]->Release();
            duplicateBuffers[i] = 0;
        }
    }
    if (duplicateBuffers) {
        delete duplicateBuffers;
        duplicateBuffers = 0;
    }
    if (propertySet) {
        propertySet->Release();
        propertySet = 0;
    }
    if (dsBuffer3D) {
        dsBuffer3D->Release();
        dsBuffer3D = 0;
    }
    if (dsBuffer) {
        dsBuffer->Release();
        dsBuffer = 0;
    }
    g_MemTagStack->UnknownFunction4a2e00(bufferBytes);
    bufferReady = 0;
    field_0x1f4_bit6 = 0;
    LeaveCriticalSection(&bufferLock);
}

// 0x004bd020: locks part of the buffer.
int Sound::LockBuffer(unsigned long offset, unsigned long bytes, void** first,
    unsigned long* firstBytes, void** second, unsigned long* secondBytes,
    unsigned long flags) {
    if (!dsBuffer) {
        *first = 0;
        *second = 0;
        return 0;
    }
    return dsBuffer->Lock(offset, bytes, first, firstBytes, second, secondBytes, flags) >= 0;
}

// 0x004bd080
int Sound::UnlockBuffer(void* first, unsigned long firstBytes, void* second, unsigned long secondBytes) {
    if (dsBuffer && first)
        return dsBuffer->Unlock(first, firstBytes, second, secondBytes) >= 0;
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
    isFilling = 1;
    if ((*buffer)->Lock(offset, bytes, &first, &firstBytes, &second, &secondBytes, 0) != 0)
        return 0;
    if (firstBytes > 0) {
        if (stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461340(stream->field_0x130 + 0x2c, 0, 1);
            if (!isLooping)
                goto ended;
        }
        stream->UnknownFunction461640(first, firstBytes, 1);
    }
    if (secondBytes > 0) {
        if (stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461340(stream->field_0x130 + 0x2c, 0, 1);
            if (!isLooping) {
            ended:
                memset(first, 0, firstBytes);
                memset(second, 0, secondBytes);
                filledHalf = 0;
                (*buffer)->SetCurrentPosition(0);
                (*buffer)->Stop();
            }
            (*buffer)->Unlock(first, firstBytes, second, secondBytes);
            isFilling = 0;
            return 1;
        }
        stream->UnknownFunction461640(second, secondBytes, 1);
    }
    long result = (*buffer)->Unlock(first, firstBytes, second, secondBytes);
    isFilling = 0;
    if (result != 0)
        return 0;
    bufferReady = 1;
    return 1;
}

// 0x004bd4b0: creates the buffer a streamed sound is loaded into.
int Sound::CreatePendingBuffer() {
    ReleaseBuffers();
    int stereo = waveHeader.channels > 1;
    int is3D = (soundFlags >> 3) & 1;
    if (!CreateBuffer(&pendingBuffer, bufferBytes, waveHeader.samplesPerSec, waveHeader.bitsPerSample,
                               waveHeader.blockAlign, stereo, is3D, 1, controlsSet, 0))
        goto failed;
    if (is3D && !Query3DBuffer(pendingBuffer, &pendingBuffer3D))
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
    currentFrequency = rate;
    format.bitsPerSample = bits;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (soundFlags & 2)
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
    bufferBytes = bytes;
    SoundSystem()->RefreshSoundCaps();
    return 1;
}

// 0x004bd6a0
int Sound::DuplicateBuffer(UnknownSoundBuffer** duplicate, Sound* source) {
    if (!source || !source->dsBuffer)
        return 0;
    return SoundSystem()->directSound->DuplicateSoundBuffer(source->dsBuffer, duplicate) >= 0;
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
    if (!dsBuffer)
        return 0;
    if (dsBuffer->QueryInterface(IID_IKsPropertySet, (void**)&propertySet) < 0)
        return 0;
    return propertySet != 0;
}

// 0x004bd740
int Sound::Set3DParameters(UnknownSound3DParameters* parameters) {
    if (SoundSystem()->audioStarted && dsBuffer && dsBuffer3D && bufferReady) {
        parameters->size = sizeof(*parameters);
        if (dsBuffer3D->SetAllParameters(parameters, 1) < 0)
            return 0;
    }
    return 1;
}

// 0x004bd790
int Sound::Set3DMode(unsigned long mode) {
    if (SoundSystem()->audioStarted && dsBuffer && dsBuffer3D && bufferReady) {
        if (dsBuffer3D->SetMode(mode, 1) < 0)
            return 0;
    }
    return 1;
}

// 0x004bd7e0-0x004bdb60: cache a 3D setting and, when forced or changed
// while playing, apply it (deferred).
int Sound::Set3DPosition(Vector3 position, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || ((parameters3D.position.x != position.x || parameters3D.position.y != position.y ||
                        parameters3D.position.z != position.z) &&
                       IsPlaying()))) {
            if (dsBuffer3D->SetPosition(position.x, position.y, position.z, 1) < 0)
                return 0;
        }
        parameters3D.position = position;
    }
    return 1;
}

int Sound::Set3DVelocity(Vector3 velocity, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || ((parameters3D.velocity.x != velocity.x || parameters3D.velocity.y != velocity.y ||
                        parameters3D.velocity.z != velocity.z) &&
                       IsPlaying()))) {
            if (dsBuffer3D->SetVelocity(velocity.x, velocity.y, velocity.z, 1) < 0)
                return 0;
        }
        parameters3D.velocity = velocity;
    }
    return 1;
}

int Sound::Set3DDistanceRange(float minDistance, float maxDistance, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || ((parameters3D.minDistance != minDistance || parameters3D.maxDistance != maxDistance) &&
                       IsPlaying()))) {
            if (dsBuffer3D->SetMinDistance(minDistance, 1) < 0 || dsBuffer3D->SetMaxDistance(maxDistance, 1) < 0)
                return 0;
        }
        parameters3D.minDistance = minDistance;
        parameters3D.maxDistance = maxDistance;
    }
    return 1;
}

int Sound::Set3DConeAngles(unsigned long insideConeAngle, unsigned long outsideConeAngle, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || ((parameters3D.insideConeAngle != insideConeAngle ||
                        parameters3D.outsideConeAngle != outsideConeAngle) &&
                       IsPlaying()))) {
            if (dsBuffer3D->SetConeAngles(insideConeAngle, outsideConeAngle, 1) < 0)
                return 0;
        }
        parameters3D.insideConeAngle = insideConeAngle;
        parameters3D.outsideConeAngle = outsideConeAngle;
    }
    return 1;
}

int Sound::Set3DConeOrientation(Vector3 orientation, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || ((parameters3D.coneOrientation.x != orientation.x ||
                        parameters3D.coneOrientation.y != orientation.y ||
                        parameters3D.coneOrientation.z != orientation.z) &&
                       IsPlaying()))) {
            if (dsBuffer3D->SetConeOrientation(orientation.x, orientation.y, orientation.z, 1) < 0)
                return 0;
        }
        parameters3D.coneOrientation = orientation;
    }
    return 1;
}

int Sound::Set3DConeOutsideVolume(long volume, int force) {
    if (SoundSystem()->audioStarted) {
        if (dsBuffer && dsBuffer3D && bufferReady &&
            (force || (parameters3D.coneOutsideVolume != volume && IsPlaying()))) {
            if (dsBuffer3D->SetConeOutsideVolume(volume, 1) < 0)
                return 0;
        }
        parameters3D.coneOutsideVolume = volume;
    }
    return 1;
}

// 0x004bdbd0: qsort order of sounds, least recently played first.
static int __cdecl CompareSounds(const void* a, const void* b) {
    Sound* first = *(Sound**)a;
    Sound* second = *(Sound**)b;
    if (first->lastPlayTime < second->lastPlayTime)
        return -1;
    return first->lastPlayTime != second->lastPlayTime;
}

// 0x004bddf0
UnknownPCAudioObject::UnknownPCAudioObject() {
    loaderThread = 0;
    stopEvent = 0;
    queuedEvent = 0;
    InitializeCriticalSection(&queueLock);
    loadedBytes = 0;
    byteBudget = 0;
}

// 0x004bde40: signals the thread to stop and waits for it.
UnknownPCAudioObject::~UnknownPCAudioObject() {
    if (loaderThread) {
        SetEvent(stopEvent);
        WaitForSingleObject(loaderThread, INFINITE);
        if (loaderThread) {
            CloseHandle(loaderThread);
            loaderThread = 0;
        }
    }
    if (stopEvent) {
        CloseHandle(stopEvent);
        stopEvent = 0;
    }
    if (queuedEvent) {
        CloseHandle(queuedEvent);
        queuedEvent = 0;
    }
    DeleteCriticalSection(&queueLock);
}

// 0x004bdef0: starts the loader thread with a budget of `budget` bytes.
int UnknownPCAudioObject::StartLoaderThread(int budget) {
    unsigned threadId;

    byteBudget = budget;
    queuedEvent = CreateEventA(0, 0, 0, 0);
    stopEvent = CreateEventA(0, 0, 0, 0);
    loaderThread = (HANDLE)_beginthreadex(0, 0, UnknownThreadProc, this, 0, &threadId);
    if (!queuedEvent || !stopEvent || !loaderThread)
        return 0;
    SetThreadPriority(loaderThread, THREAD_PRIORITY_BELOW_NORMAL);
    if (!queuedSounds.Init(8, 16))
        return 0;
    return loadedSounds.Init(8, 16);
}

// 0x004bdfc0: queues a streamed sound whose buffer is not ready.
void UnknownPCAudioObject::QueueSoundLoad(Sound* sound) {
    if (sound && (sound->soundFlags & 4)) {
        EnterCriticalSection(&sound->bufferLock);
        int ready = sound->bufferReady;
        LeaveCriticalSection(&sound->bufferLock);
        if (!ready) {
            EnterCriticalSection(&queueLock);
            queuedSounds.Add(sound);
            LeaveCriticalSection(&queueLock);
            SetEvent(queuedEvent);
        }
    }
}

// 0x004be0a0
void UnknownPCAudioObject::RecordLoadedSound(Sound* sound, int bytes) {
    loadedBytes += bytes;
    loadedSounds.Add(sound);
}

// 0x004be130: unloads the least recently played idle sounds until `bytes`
// more fit in the budget.
void UnknownPCAudioObject::MakeRoomForSound(int bytes) {
    unsigned long total = loadedBytes + bytes;
    if (total < byteBudget)
        return;
    qsort(loadedSounds.m_data, loadedSounds.m_count, sizeof(Sound*), CompareSounds);
    for (int i = 0; i < loadedSounds.m_count; i++) {
        Sound* sound = loadedSounds.Get(i);
        if (sound) {
            int queued = 0;
            for (int j = 0; j < queuedSounds.m_count; j++) {
                if (loadedSounds.Get(j) == sound)
                    queued = 1;
            }
            if (!sound->IsPlaying() && !queued) {
                UnloadSound(sound);
                total -= sound->bufferBytes;
                i--;
            }
            if (total < byteBudget)
                return;
        }
    }
}

// 0x004be220: releases a sound's buffers and forgets it.
void UnknownPCAudioObject::UnloadSound(Sound* sound) {
    sound->IsPlaying();
    sound->ReleaseBuffers();
    loadedBytes -= sound->bufferBytes;
    loadedSounds.Remove(sound);
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

    if (audioStarted)
        return 0x8878000a;
    audioStarted = 0;
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
    audioStarted = 1;
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
    if (!audioStarted && !primaryBuffer)
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
    if (!audioStarted)
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

    if (audioStarted) {
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
    if (audioStarted && primaryBuffer)
        return primaryBuffer->SetVolume(volume) >= 0;
    return 0;
}

// 0x004be9e0: creates a sound group and a probe sound; with hardware sound
// allowed and mixing buffers available, a small 3D buffer that supports the
// EAX listener properties enables them with value 9.
void PCSoundInterface::ProbeEAXSupport() {
    if (!audioStarted)
        return;
    eaxProbeGroup = new (__FILE__, 2574) SoundGroup(1);
    eaxProbeSound = new (__FILE__, 2575) Sound(eaxProbeGroup, 1);
    if (eaxProbeGroup && eaxProbeSound && allowSoundHardware && soundCaps.maxHwMixingAllBuffers >= 1 &&
        eaxProbeSound->CreateBuffer(&eaxProbeSound->dsBuffer, 0x400, 11025, 16, 2, 0, 1, 1, 16, 1) &&
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
    if (!audioStarted)
        return 0;
    return listener->CommitDeferredSettings() >= 0;
}

int PCSoundInterface::SetListenerDistanceFactor(float factor) {
    if (!audioStarted)
        return 0;
    return listener->SetDistanceFactor(factor, 1) >= 0;
}

int PCSoundInterface::SetListenerDopplerFactor(float factor) {
    if (!audioStarted)
        return 0;
    return listener->SetDopplerFactor(factor, 1) >= 0;
}

int PCSoundInterface::SetListenerOrientation(Vector3 front, Vector3 top) {
    if (!audioStarted)
        return 0;
    return listener->SetOrientation(front.x, front.y, front.z, top.x, top.y, top.z, 1) >= 0;
}

int PCSoundInterface::SetListenerPosition(Vector3 position) {
    if (!audioStarted)
        return 0;
    return listener->SetPosition(position.x, position.y, position.z, 1) >= 0;
}

int PCSoundInterface::SetListenerVelocity(Vector3 velocity) {
    if (!audioStarted)
        return 0;
    return listener->SetVelocity(velocity.x, velocity.y, velocity.z, 1) >= 0;
}

int PCSoundInterface::SetListenerRolloffFactor(float factor) {
    if (!audioStarted)
        return 0;
    return listener->SetRolloffFactor(factor, 1) >= 0;
}

// 0x004bed00: sets EAX listener property 1 when EAX is available and allowed.
int PCSoundInterface::SetEAXEnvironment(unsigned long value) {
    if (audioStarted && eaxAvailable && eaxProbeSound && allowEAXExtension)
        return eaxProbeSound->SetProperty(&DSPROPSETID_EAX_ListenerProperties, 1, 0, 0, &value, 4);
    return 0;
}

// 0x004bed40: sets all 16 bytes of EAX listener property 0.
int PCSoundInterface::SetEAXListenerParameters(void* parameters) {
    if (audioStarted && eaxAvailable && eaxProbeSound && allowEAXExtension)
        return eaxProbeSound->SetProperty(&DSPROPSETID_EAX_ListenerProperties, 0, 0, 0, parameters,
                                                  16);
    return 0;
}
