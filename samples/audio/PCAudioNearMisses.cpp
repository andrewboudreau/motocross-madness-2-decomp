// Near-miss PCAudio.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PCAUDIO.md. Each differs only in branch layout or register
// choice:
//
// UnknownFunction4be910 (0x004be910, 158 bytes): sets the primary buffer's
// PCM format. The 0x14-byte frame (a WAVEFORMATEX-sized local with only the
// first 16 bytes cleared), the arithmetic and the call match (108 of 158
// bytes); only the scheduling of the zeroing stores against the argument
// loads differs. Retail clears all 16 bytes first and keeps `rate` in esi;
// every order of the four field assignments, a channel temporary, `?:` and
// an if/else body leave VC6 loading `stereo` into esi first.
//
// Sound 0x004bcb30 / 0x004bcbe0 / 0x004bcca0 (frequency, volume, pan): retail
// places the shared `return 0` between the duplicate loop and the final
// store (the loop exits with `jge store; jmp top`); here it lands after the
// store. Early returns, `goto failed`, a while loop, an HRESULT-carrying loop
// and a combined condition all keep VC6's layout.
//
// Sound 0x004bd4b0 (creates a streamed sound's buffer): the arguments of
// 0x004bd540 are scheduled into different registers.
//
// Sound 0x004bc4c0 (duplicates a sound) and 0x004bc320 (loads from a stream
// or file): `this` and the source pointer swap ebx/ebp (ebp/edi).
//
// Sound 0x004bd0c0 (refills from the archive or file) and 0x004bb890 (the
// sound factory): retail keeps one `return 1` block shared by both branches
// and inlines the factory's early `return 0` epilogues; VC6 here folds the
// archive branch's result into `neg/sbb` and shares the factory's failures.
//
// Sound 0x004bc6b0 (starts a sound): the same loop shape as the setters:
// retail exits the duplicate search with `jge failed; jmp top` and jumps to
// one shared failure epilogue, where VC6 here copies the epilogue after the
// loop (goto, break-then-test and a found flag all give that). The prologue
// also stores the bitfield and play flags in the other order.
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

// Copies at most 0x103 characters of `name` and terminates them.
static inline void CopySoundName(char* to, const char* name) {
    int length = strlen(name);
    if (length > 0x103)
        length = 0x103;
    strncpy(to, name, length);
    to[length] = 0;
}

// 0x004be910: sets the primary buffer's PCM format.
int PCSoundInterface::UnknownFunction4be910(int rate, int stereo, int bits) {
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
    return field_0x468->SetFormat(&format) >= 0;
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
            if (sound->UnknownFunction4bbef0(entry->field_0x14, flags, a, b, c))
                goto named;
        } else if (sound->UnknownFunction4bc4c0(loaded)) {
            goto named;
        }
    } else {
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
        if (sound->UnknownFunction4bbef0(entry->field_0x14, flags, a, b, c)) {
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
        if (!UnknownFunction4bbef0(file, flags, a, b, -1))
            goto failed;
        if (!stream)
            delete file;
    } else if (flags & 2) {
        field_0x40 = file;
        if (!UnknownFunction4bbef0(file, flags, a, b, c))
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

// 0x004bc4c0: shares `source`'s buffer.
int Sound::UnknownFunction4bc4c0(Sound* source) {
    if (!source || !SoundSystem() || !SoundSystem()->field_0x2c_bit0)
        return 0;
    UnknownFunction4bcf50();
    strcpy(field_0x60, "");
    UnknownSoundBuffer* duplicate = 0;
    if (!UnknownFunction4bd6a0(&duplicate, source))
        return 0;
    field_0x0c = duplicate;
    if (source->field_0x10) {
        if (field_0x10) {
            field_0x10->Release();
            field_0x10 = 0;
        }
        if (!UnknownFunction4bd6e0(field_0x0c, &field_0x10))
            return 0;
    }
    field_0x28 = source;
    field_0x1f4_bit1 = 1;
    source->AddRef();
    CopySoundName(field_0x60, field_0x28->field_0x60);
    if (field_0x1ec & 0x80)
        UnknownFunction4bcbe0(field_0x08->field_0x30, 0);
    return 1;
}

// 0x004bcb30: sets the frequency (100..100000 Hz) of the sound and its
// duplicates; unless `force`, only while playing and when it changes.
int Sound::UnknownFunction4bcb30(unsigned long frequency, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && field_0x1f5_bit0 &&
        (force || (field_0x19c != frequency && UnknownFunction4bca80()))) {
        if (frequency < 100 || frequency > 100000)
            goto failed;
        if (field_0x0c->SetFrequency(frequency) < 0)
            goto failed;
        for (int i = 0; i < field_0x24; i++) {
            if (field_0x20[i] && field_0x20[i]->SetFrequency(frequency) < 0)
                goto failed;
        }
    }
    field_0x19c = frequency;
    return 1;
failed:
    return 0;
}

// 0x004bcbe0: sets the volume (-10000..0) of a 2D sound and its duplicates.
int Sound::UnknownFunction4bcbe0(long volume, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && !field_0x10 && field_0x1f5_bit0 &&
        (force || (field_0x1a0 != volume && UnknownFunction4bca80()))) {
        if (volume < -10000)
            volume = -10000;
        else if (volume > 0)
            volume = 0;
        if (field_0x0c->SetVolume(volume) < 0)
            goto failed;
        for (int i = 0; i < field_0x24; i++) {
            if (field_0x20[i] && field_0x20[i]->SetVolume(volume) < 0)
                goto failed;
        }
    }
    field_0x1a0 = volume;
    return 1;
failed:
    return 0;
}

// 0x004bcca0: sets the pan of a 2D sound and its duplicates.
int Sound::UnknownFunction4bcca0(long pan, int force) {
    if (!SoundSystem()->field_0x2c_bit0)
        return 1;
    if (field_0x0c && !field_0x10 && field_0x1f5_bit0 &&
        (force || (field_0x1a4 != pan && UnknownFunction4bca80()))) {
        if (field_0x0c->SetPan(pan) < 0)
            goto failed;
        for (int i = 0; i < field_0x24; i++) {
            if (field_0x20[i] && field_0x20[i]->SetPan(pan) < 0)
                goto failed;
        }
    }
    field_0x1a4 = pan;
    return 1;
failed:
    return 0;
}

// 0x004bd0c0: refills `buffer` from the sound's archive entry or file,
// skipping the 44-byte header.
int Sound::UnknownFunction4bd0c0(UnknownSoundBuffer** buffer) {
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
        if (!UnknownFunction4bd260(buffer, entry->field_0x14, 0, field_0x198))
            return 0;
        return 1;
    }
    UnknownTextureStream* file = new (__FILE__, 1368) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (file->UnknownFunction460f50(field_0x60, "rb", 0)) {
        if (file->field_0x1c)
            file->UnknownFunction461340(file->field_0x130, 0, 0);
        UnknownWaveHeader header;
        if (file->UnknownFunction461640(&header, 0x2c, 1) == 1 &&
            UnknownFunction4bd260(buffer, file, 0, field_0x198)) {
            delete file;
            return 1;
        }
    }
    delete file;
    return 0;
}

// 0x004bd4b0: creates the buffer a streamed sound is loaded into.
int Sound::UnknownFunction4bd4b0() {
    UnknownFunction4bcf50();
    int is3D = (field_0x1e8 >> 3) & 1;
    if (!UnknownFunction4bd540(&field_0x18, field_0x198, field_0x16a.samplesPerSec, field_0x16a.bitsPerSample,
                               field_0x16a.blockAlign, field_0x16a.channels > 1, is3D, 1, field_0x1ec, 0))
        goto failed;
    if (is3D && !UnknownFunction4bd6e0(field_0x18, &field_0x1c))
        goto failed;
    return 1;
failed:
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
                audio->UnknownFunction4be130(bytes);
                LeaveCriticalSection(&audio->field_0x0c);
                EnterCriticalSection(&sound->field_0x48);
                int loaded = 0;
                if (sound->UnknownFunction4bd4b0()) {
                    sound->UnknownFunction4bd0c0(&sound->field_0x18);
                    sound->field_0x1f4_bit7 = 0;
                    sound->field_0x1f5_bit0 = 1;
                    sound->field_0x0c = sound->field_0x18;
                    sound->field_0x10 = sound->field_0x1c;
                    sound->field_0x18 = 0;
                    sound->field_0x1c = 0;
                    loaded = 1;
                    if (!sound->field_0x1f5_bit1)
                        sound->UnknownFunction4bbcd0();
                }
                LeaveCriticalSection(&sound->field_0x48);
                if (loaded) {
                    EnterCriticalSection(&audio->field_0x0c);
                    audio->UnknownFunction4be0a0(sound, bytes);
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
int Sound::UnknownFunction4bc6b0(int restart, unsigned long playFlags, int preferHardware) {
    if (!SoundSystem()->field_0x2c_bit0 || !field_0x08 || !field_0x08->field_0x2c_bit0 ||
        field_0x08->field_0x25_bit2)
        return 1;
    field_0x1f4_bit6 = preferHardware;
    int looping = playFlags & 1;
    field_0x1f0 = playFlags;
    if (field_0x1e8 & 2) {
        field_0x1f0 = 1;
    } else if (!(field_0x1e8 & 0x20)) {
        if (field_0x1f4_bit6 && SoundSystem()->field_0x45c_bit0 &&
            (long)SoundSystem()->field_0x3fc.freeHw3DAllBuffers > 0)
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
            int found = 0;
            for (; i < field_0x24; i++) {
                if (field_0x20[i]) {
                    if (field_0x20[i]->GetStatus(&status) < 0)
                        goto failed;
                    if ((status & 1) != 1) {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found)
                goto failed;
            if (field_0x20[i]->Play(0, 0, playFlags) < 0)
                goto failed;
            return 1;
        }
        if (!(field_0x1e8 & 5)) {
            if (field_0x1e8 & 2) {
                if (field_0x3c)
                    delete field_0x3c;
                field_0x3c = new (__FILE__, 816) UnknownSoundNotifier;
                if (!field_0x3c || !field_0x3c->UnknownFunction4bb740(this))
                    goto failed;
                field_0x44 = field_0x198 / 2;
                if (!UnknownFunction4bbcd0())
                    return 0;
            }
        } else if (!UnknownFunction4bbcd0()) {
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
            SoundSystem()->field_0x46c->UnknownFunction4bdfc0(this);
        EnterCriticalSection(&field_0x48);
        field_0x1f5_bit1 = 0;
        LeaveCriticalSection(&field_0x48);
    }
    return 1;
failed:
    return 0;
}
