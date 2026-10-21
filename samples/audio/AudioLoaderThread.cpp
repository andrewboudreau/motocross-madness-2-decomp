// Complete loader-thread candidate; original translation-unit ownership is unproven.
// See docs/AUDIO_LOADER_THREAD.md for retail and native behavioral evidence.
#include <process.h>
#include "../../src/reconstructed/PCAudio.h"

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
