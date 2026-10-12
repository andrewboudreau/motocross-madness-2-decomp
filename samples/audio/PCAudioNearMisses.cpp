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
// an if/else body leave VC6 loading `stereo` into esi first; so do all 24
// assignment orders x five channel-count spellings, memset sizes 16 and
// sizeof - 4, a derived struct clearing in its constructor, unsigned/short
// parameter types (bool or unsigned char `stereo`: 38) and blockAlign
// first; avgBytesPerSec from `rate` or as a product scores lower.
// Session 10, no gain: `1 + (stereo != 0)`, `!!stereo + 1` and
// `(stereo ? 1 : 0) + 1` give the same 108; `channels = 1; if (stereo)
// channels = 2;` (34), an inline SetPcmFormat(format*, channels, rate, bits)
// helper (51) and rate first with (unsigned short) casts (68) are worse.
//
// Sound 0x004bc6b0 (starts a sound): compiled inside PCAudio.cpp (where the
// UnknownSoundNotifier constructor is defined, so VC6 drops the EH frame
// like retail) with the guarded do-while duplicate search and playFlags
// used before preferHardware, 237 of 629 bytes match: the search loop, the
// prologue and the slot of `status` agree. Left: VC6 cross-jumps the two
// `Play()` failure tails (streamed `return 0` and the `& 5` branch's
// `goto failed`) where retail keeps them apart (the streamed one returns
// Play's zero without `xor eax, eax`). Declaring `queue` after the
// EnterCriticalSection gives retail's late `xor ebp, ebp` (238 of 629
// inside PCAudio.cpp; that cross-jump is the only difference left). Tried
// against the cross-jump: `return played` of a Play result local, `if
// (Play()) goto done; return 0;` in either branch, `return 0` instead of
// `goto failed` in the `& 5` branch, the `failed:` label inside that
// branch, and the `& 5` branch first (VC6 then moves `done` after it).
// Here (outside PCAudio.cpp) VC6 also adds the EH frame.
//
// UnknownPCAudioObject 0x004bdc00 (the loader thread; retail extent is 483
// bytes, 0x4bdc00..0x4bdde3): spelling the inner queue loop `while (1)`
// instead of `for (;;)` keeps it top-tested like retail (instruction count
// 155 = retail, candidate 478 bytes; `for (;;)` is rotated, 162
// instructions and 491 bytes, though it scores 95 strict bytes against 91
// here). Left: register allocation. Retail keeps ebp = audio,
// ebx = &audio->field_0x0c then `loaded`, edi = &sound->field_0x48, and
// spills `ready`/`bytes` to one frame slot (frame 0xc); VC6 here caches the
// EnterCriticalSection pointer in ebx, spills `loaded` (frame 8). Tried
// without gain: context macro instead of the `audio` local, `loaded = 0`
// before the lock, function-scope ready/bytes/loaded, one shared variable
// for ready and bytes, a CRITICAL_SECTION* local, inline Lock/Unlock
// helpers, `loaded = 1` after Play (same strict score), swapped bitfield
// stores, `if (!loaded) continue;`, if-wrapping instead of `continue`, the
// ready test inline in the if.

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
    if (!audioStarted)
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


// 0x004bdc00: loads queued streamed sounds until the stop event: makes room
// within the budget, creates and fills the buffer, and plays the sound
// unless it was stopped meanwhile.
unsigned __stdcall UnknownPCAudioObject::UnknownThreadProc(void* context) {
    UnknownPCAudioObject* audio = (UnknownPCAudioObject*)context;
    HANDLE events[2];
    unsigned long result;

    events[0] = audio->queuedEvent;
    events[1] = audio->stopEvent;
    while ((result = WaitForMultipleObjects(2, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0:
            while (1) {
                EnterCriticalSection(&audio->queueLock);
                if (audio->queuedSounds.m_count <= 0) {
                    LeaveCriticalSection(&audio->queueLock);
                    break;
                }
                Sound* sound = audio->queuedSounds.Get(0);
                audio->queuedSounds.Remove(sound);
                LeaveCriticalSection(&audio->queueLock);
                if (!(sound->soundFlags & 4))
                    continue;
                EnterCriticalSection(&sound->bufferLock);
                int ready = sound->bufferReady;
                LeaveCriticalSection(&sound->bufferLock);
                if (ready)
                    continue;
                EnterCriticalSection(&sound->bufferLock);
                unsigned long bytes = sound->bufferBytes;
                LeaveCriticalSection(&sound->bufferLock);
                EnterCriticalSection(&audio->queueLock);
                audio->MakeRoomForSound(bytes);
                LeaveCriticalSection(&audio->queueLock);
                EnterCriticalSection(&sound->bufferLock);
                int loaded = 0;
                if (sound->CreatePendingBuffer()) {
                    sound->FillBufferFromFile(&sound->pendingBuffer);
                    sound->field_0x1f4_bit7 = 0;
                    sound->bufferReady = 1;
                    sound->dsBuffer = sound->pendingBuffer;
                    sound->dsBuffer3D = sound->pendingBuffer3D;
                    sound->pendingBuffer = 0;
                    sound->pendingBuffer3D = 0;
                    loaded = 1;
                    if (!sound->isStopped)
                        sound->Play();
                }
                LeaveCriticalSection(&sound->bufferLock);
                if (loaded) {
                    EnterCriticalSection(&audio->queueLock);
                    audio->RecordLoadedSound(sound, bytes);
                    LeaveCriticalSection(&audio->queueLock);
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
    if (!SoundSystem()->audioStarted || !soundGroup || !soundGroup->field_0x2c_bit0 ||
        soundGroup->isPaused)
        return 1;
    int looping = playFlags & 1;
    lastPlayFlags = playFlags;
    field_0x1f4_bit6 = preferHardware;
    if (soundFlags & 2) {
        lastPlayFlags = 1;
    } else if (!(soundFlags & 0x20)) {
        if (field_0x1f4_bit6 && SoundSystem()->allowSoundHardware &&
            (long)SoundSystem()->soundCaps.freeHw3DAllBuffers > 0)
            lastPlayFlags = playFlags | 2;
        else
            lastPlayFlags = playFlags | 4;
    }
    if (soundFlags & 4)
        EnterCriticalSection(&bufferLock);
    if (dsBuffer) {
        unsigned long status;
        int i = 0;
        if (dsBuffer->GetStatus(&status) < 0)
            goto failed;
        if ((status & 1) == 1 && looping == isLooping) {
            if (!restart)
                goto done;
            if (duplicateCount > 0) {
                do {
                    if (duplicateBuffers[i]) {
                        if (duplicateBuffers[i]->GetStatus(&status) < 0)
                            goto failed;
                        if ((status & 1) != 1) {
                            if (duplicateBuffers[i]->Play(0, 0, playFlags) < 0)
                                goto failed;
                            return 1;
                        }
                    }
                    i++;
                } while (i < duplicateCount);
            }
            goto failed;
        }
        if (!(soundFlags & 5)) {
            if (soundFlags & 2) {
                if (notifier)
                    delete notifier;
                notifier = new (__FILE__, 816) UnknownSoundNotifier;
                if (!notifier || !notifier->StartRefillThread(this))
                    goto failed;
                filledHalf = bufferBytes / 2;
                if (!Play())
                    return 0;
            }
        } else if (!Play()) {
            goto failed;
        }
    }
done:
    if (soundFlags & 4)
        LeaveCriticalSection(&bufferLock);
    if (soundFlags & 4) {
        EnterCriticalSection(&bufferLock);
        int queue = 0;
        if (!bufferReady && !field_0x1f4_bit7) {
            field_0x1f4_bit7 = 1;
            queue = 1;
        }
        LeaveCriticalSection(&bufferLock);
        if (queue)
            SoundSystem()->soundMemoryManager->QueueSoundLoad(this);
        EnterCriticalSection(&bufferLock);
        isStopped = 0;
        LeaveCriticalSection(&bufferLock);
    }
    return 1;
failed:
    return 0;
}
