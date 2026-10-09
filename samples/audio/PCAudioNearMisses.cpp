// Near-miss PCAudio.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PCAUDIO.md. Each differs only in branch layout or register
// choice:
//
// SetPrimaryFormat (0x004be910, 158 bytes): sets the primary buffer's
// PCM format. The 0x14-byte frame (a WAVEFORMATEX-sized local with only the
// first 16 bytes cleared), the arithmetic and the call match (108 of 158
// bytes); only the scheduling of the zeroing stores against the argument
// loads differs. Retail clears all 16 bytes first and keeps `rate` in esi;
// every order of the four field assignments, a channel temporary, `?:` and
// an if/else body leave VC6 loading `stereo` into esi first.
//
// Sound 0x004bd0c0 (refills from the archive or file) and 0x004bb890 (the
// sound factory): retail keeps one `return 1` block shared by both branches
// and inlines the factory's early `return 0` epilogues (the first one
// without `xor eax, eax`, eax being the null entry); VC6 here folds the
// archive branch's result into `neg/sbb` and shares the factory's failures.
// An if/else with one trailing `return 1`, gotos to shared labels, a
// result variable and an ok flag were tried for 0x004bd0c0.
//
// Sound 0x004bc6b0 (starts a sound): compiled inside PCAudio.cpp (where the
// UnknownSoundNotifier constructor is defined, so VC6 drops the EH frame
// like retail) with the guarded do-while duplicate search and playFlags
// used before preferHardware, 237 of 629 bytes match: the search loop, the
// prologue and the slot of `status` agree. Left: VC6 cross-jumps the two
// `Play()` failure tails (streamed `return 0` and the `& 5` branch's
// `goto failed`) where retail keeps them apart (the streamed one returns
// Play's zero without `xor eax, eax`), and retail initialises `queue` after
// the EnterCriticalSection. Here (outside PCAudio.cpp) VC6 also adds the EH
// frame.
//
// UnknownPCAudioObject 0x004bdc00 (the loader thread): retail's queue loop is
// not rotated and spills one local (frame 0xc); here VC6 rotates the loop
// and keeps everything in registers.

#include <process.h>
#include <string.h>

#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

// 0x004be910: sets the primary buffer's PCM format.
int PCSoundInterface::SetPrimaryFormat(int rate, int stereo, int bits) {
    if (!field_0x2c_bit0)
        return 0;
    UnknownWaveFormat format;
    memset(&format, 0, 16); // the PCM part only; extraSize is left as is
    format.formatTag = 1;
    format.bitsPerSample = bits;
    format.channels = (stereo != 0) + 1;
    format.samplesPerSec = rate;
    format.avgBytesPerSec = format.channels * format.bitsPerSample * format.samplesPerSec / 8;
    format.blockAlign = format.channels * format.bitsPerSample / 8;
    return primaryBuffer->SetFormat(&format) >= 0;
}


// 0x004bb890: a new sound for the archive entry `name`: a duplicate of the
// sound already loaded from it when there is one (unless that one is
// streamed), otherwise loaded from the entry's stream and recorded there.
Sound* UnknownFunction4bb890(SoundGroup* group, const char* name, int flags, int a, int b, int c) {
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    if (!entry)
        return 0;
    Sound* sound = new (__FILE__, 186) Sound(group, 1);
    if (!sound)
        return 0;
    Sound* loaded = (Sound*)entry->field_0x10;
    if (loaded) {
        if (loaded->field_0x1e8 & 4) {
            entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
            if (sound->LoadWave(entry->field_0x14, flags, a, b, c))
                goto named;
        } else if (sound->DuplicateFrom(loaded)) {
            goto named;
        }
    } else {
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
        if (sound->LoadWave(entry->field_0x14, flags, a, b, c)) {
            g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, sound);
            goto named;
        }
    }
    delete sound;
    return 0;
named:
    strncpy(sound->field_0x60, name, 0x103);
    sound->field_0x1f4_bit0 = 1;
    return sound;
}

// 0x004bd0c0: refills `buffer` from the sound's archive entry or file,
// skipping the 44-byte header.
int Sound::FillBufferFromFile(UnknownSoundBuffer** buffer) {
    if (!buffer)
        return 0;
    if (field_0x1f4_bit0) {
        UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(field_0x60, 0);
        if (!entry)
            return 0;
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
        UnknownWaveHeader header;
        if (entry->field_0x14->UnknownFunction461640(&header, 0x2c, 1) != 1)
            return 0;
        if (!FillBufferFromStream(buffer, entry->field_0x14, 0, field_0x198))
            return 0;
        return 1;
    }
    UnknownTextureStream* file = new (__FILE__, 1368) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (file->UnknownFunction460f50(field_0x60, "rb", 0)) {
        if (file->field_0x1c)
            file->UnknownFunction461340(file->field_0x130, 0, 0);
        UnknownWaveHeader header;
        if (file->UnknownFunction461640(&header, 0x2c, 1) == 1 &&
            FillBufferFromStream(buffer, file, 0, field_0x198)) {
            delete file;
            return 1;
        }
    }
    delete file;
    return 0;
}

// 0x004bdc00: loads queued streamed sounds until the stop event: makes room
// within the budget, creates and fills the buffer, and plays the sound
// unless it was stopped meanwhile.
unsigned __stdcall UnknownPCAudioObject::UnknownThreadProc(void* context) {
    UnknownPCAudioObject* audio = (UnknownPCAudioObject*)context;
    HANDLE events[2];
    unsigned long result;

    events[0] = audio->field_0x08;
    events[1] = audio->field_0x04;
    while ((result = WaitForMultipleObjects(2, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0:
            for (;;) {
                EnterCriticalSection(&audio->field_0x0c);
                if (audio->field_0x2c.m_count <= 0) {
                    LeaveCriticalSection(&audio->field_0x0c);
                    break;
                }
                Sound* sound = audio->field_0x2c.Get(0);
                audio->field_0x2c.Remove(sound);
                LeaveCriticalSection(&audio->field_0x0c);
                if (!(sound->field_0x1e8 & 4))
                    continue;
                EnterCriticalSection(&sound->field_0x48);
                int ready = sound->field_0x1f5_bit0;
                LeaveCriticalSection(&sound->field_0x48);
                if (ready)
                    continue;
                EnterCriticalSection(&sound->field_0x48);
                unsigned long bytes = sound->field_0x198;
                LeaveCriticalSection(&sound->field_0x48);
                EnterCriticalSection(&audio->field_0x0c);
                audio->MakeRoomForSound(bytes);
                LeaveCriticalSection(&audio->field_0x0c);
                EnterCriticalSection(&sound->field_0x48);
                int loaded = 0;
                if (sound->CreatePendingBuffer()) {
                    sound->FillBufferFromFile(&sound->field_0x18);
                    sound->field_0x1f4_bit7 = 0;
                    sound->field_0x1f5_bit0 = 1;
                    sound->field_0x0c = sound->field_0x18;
                    sound->field_0x10 = sound->field_0x1c;
                    sound->field_0x18 = 0;
                    sound->field_0x1c = 0;
                    loaded = 1;
                    if (!sound->field_0x1f5_bit1)
                        sound->Play();
                }
                LeaveCriticalSection(&sound->field_0x48);
                if (loaded) {
                    EnterCriticalSection(&audio->field_0x0c);
                    audio->RecordLoadedSound(sound, bytes);
                    LeaveCriticalSection(&audio->field_0x0c);
                }
            }
            break;
        case 1:
            goto stopped;
        }
    }
stopped:
    _endthreadex(0);
    return 0;
}

// 0x004bc6b0: starts the sound. A sound already playing in the same loop
// mode starts a free duplicate instead (when `restart`); static sounds
// play directly, streamed ones get a new notifier first; deferred ones are
// queued for the loader. Failures after the lock leave the critical section
// held (retail behaviour).
int Sound::PlayWithOptions(int restart, unsigned long playFlags, int preferHardware) {
    if (!SoundSystem()->field_0x2c_bit0 || !field_0x08 || !field_0x08->field_0x2c_bit0 ||
        field_0x08->field_0x25_bit2)
        return 1;
    int looping = playFlags & 1;
    field_0x1f0 = playFlags;
    field_0x1f4_bit6 = preferHardware;
    if (field_0x1e8 & 2) {
        field_0x1f0 = 1;
    } else if (!(field_0x1e8 & 0x20)) {
        if (field_0x1f4_bit6 && SoundSystem()->allowSoundHardware &&
            (long)SoundSystem()->soundCaps.freeHw3DAllBuffers > 0)
            field_0x1f0 = playFlags | 2;
        else
            field_0x1f0 = playFlags | 4;
    }
    if (field_0x1e8 & 4)
        EnterCriticalSection(&field_0x48);
    if (field_0x0c) {
        unsigned long status;
        int i = 0;
        if (field_0x0c->GetStatus(&status) < 0)
            goto failed;
        if ((status & 1) == 1 && looping == field_0x1f4_bit3) {
            if (!restart)
                goto done;
            if (field_0x24 > 0) {
                do {
                    if (field_0x20[i]) {
                        if (field_0x20[i]->GetStatus(&status) < 0)
                            goto failed;
                        if ((status & 1) != 1) {
                            if (field_0x20[i]->Play(0, 0, playFlags) < 0)
                                goto failed;
                            return 1;
                        }
                    }
                    i++;
                } while (i < field_0x24);
            }
            goto failed;
        }
        if (!(field_0x1e8 & 5)) {
            if (field_0x1e8 & 2) {
                if (field_0x3c)
                    delete field_0x3c;
                field_0x3c = new (__FILE__, 816) UnknownSoundNotifier;
                if (!field_0x3c || !field_0x3c->StartRefillThread(this))
                    goto failed;
                field_0x44 = field_0x198 / 2;
                if (!Play())
                    return 0;
            }
        } else if (!Play()) {
            goto failed;
        }
    }
done:
    if (field_0x1e8 & 4)
        LeaveCriticalSection(&field_0x48);
    if (field_0x1e8 & 4) {
        int queue = 0;
        EnterCriticalSection(&field_0x48);
        if (!field_0x1f5_bit0 && !field_0x1f4_bit7) {
            field_0x1f4_bit7 = 1;
            queue = 1;
        }
        LeaveCriticalSection(&field_0x48);
        if (queue)
            SoundSystem()->soundMemoryManager->QueueSoundLoad(this);
        EnterCriticalSection(&field_0x48);
        field_0x1f5_bit1 = 0;
        LeaveCriticalSection(&field_0x48);
    }
    return 1;
failed:
    return 0;
}
