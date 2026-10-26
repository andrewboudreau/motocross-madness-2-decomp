#include <process.h>
#include <string.h>
#include <windows.h>

#include "Recorder.h"

#include "DebugAlloc.h"
#include "MatrixUtil.h"
#include "VCR.h"

// Closes a handle member once.
#define CLOSE_HANDLE(handle)          \
    if (handle) {                     \
        CloseHandle(handle);          \
        handle = 0;                   \
    }

unsigned __stdcall UnknownRecorderThread(void* parameters); // 0x004e6f80 (samples/race/RecorderNearMisses.cpp)

// 0x004e77f0
VCRInterface::VCRInterface() : GameObject(1) {
    workerThread = 0;
    writeEvent = 0;
    createFileEvent = 0;
    reopenFileEvent = 0;
    openPlaybackEvent = 0;
    readReverseEvent = 0;
    seekEvent = 0;
    rewindEvent = 0;
    stopReadingEvent = 0;
    closeFileEvent = 0;
    stopEvent = 0;
    workerThreadId = 0;
    frameRing = 0;
    recordBuffer = 0;
    readingReverse = 0;
    recordPending = 0;
    recordType = 0;
    recordId = 0;
    recordPosition = 0;
    isBusy = 0;
    fileReady = 0;
    readsReset = 0;
    reverseReadRestarted = 0;
    sceneTimePending = 0;
    pendingSceneTime = 0;
    isStopped = 1;
    workerAcknowledged = 1;
}

// 0x004e78b0
VCRInterface::~VCRInterface() {
    if (workerThread) {
        SetEvent(stopEvent);
        WaitForSingleObject(workerThread, INFINITE);
        CLOSE_HANDLE(workerThread);
    }
    CLOSE_HANDLE(writeEvent);
    CLOSE_HANDLE(createFileEvent);
    CLOSE_HANDLE(reopenFileEvent);
    CLOSE_HANDLE(readForwardEvent);
    CLOSE_HANDLE(openPlaybackEvent);
    CLOSE_HANDLE(readReverseEvent);
    CLOSE_HANDLE(seekEvent);
    CLOSE_HANDLE(rewindEvent);
    CLOSE_HANDLE(stopReadingEvent);
    CLOSE_HANDLE(closeFileEvent);
    CLOSE_HANDLE(stopEvent);
    DeleteCriticalSection(&lock);
    if (frameRing)
        delete frameRing;
    if (recordBuffer)
        DebugFree(recordBuffer, __FILE__, 442);
}

// 0x004e7a00
void VCRInterface::SignalAndWait(HANDLE event, DWORD interval) {
    int waiting;

    EnterCriticalSection(&lock);
    workerAcknowledged = 0;
    LeaveCriticalSection(&lock);
    SetEvent(event);
    Sleep(interval);
    EnterCriticalSection(&lock);
    waiting = !workerAcknowledged;
    LeaveCriticalSection(&lock);
    while (waiting) {
        Sleep(interval);
        EnterCriticalSection(&lock);
        waiting = !workerAcknowledged;
        LeaveCriticalSection(&lock);
    }
}

// 0x004e7a90
int VCRInterface::Start(int callback, int header, int headerSize, int mode, int fileName,
                        unsigned int size, UnknownRecorderOwner* owner) {
    DWORD interval;

    frameRing = new (__FILE__, 477) UnknownVcr(size);
    if (!frameRing)
        return 0;
    vcrFile = owner;
    recordCallback = (UnknownRecorderCallback)callback;
    writeEvent = CreateEventA(0, 0, 0, 0);
    createFileEvent = CreateEventA(0, 0, 0, 0);
    reopenFileEvent = CreateEventA(0, 0, 0, 0);
    openPlaybackEvent = CreateEventA(0, 0, 0, 0);
    readForwardEvent = CreateEventA(0, 0, 0, 0);
    readReverseEvent = CreateEventA(0, 0, 0, 0);
    seekEvent = CreateEventA(0, 0, 0, 0);
    rewindEvent = CreateEventA(0, 0, 0, 0);
    stopReadingEvent = CreateEventA(0, 0, 0, 0);
    closeFileEvent = CreateEventA(0, 0, 0, 0);
    stopEvent = CreateEventA(0, 0, 0, 0);
    if (!writeEvent || !stopEvent || !createFileEvent || !reopenFileEvent || !openPlaybackEvent || !readForwardEvent ||
        !readReverseEvent || !seekEvent || !rewindEvent || !stopReadingEvent || !closeFileEvent)
        return 0;
    recordMode = mode;
    threadParameters.header = header;
    threadParameters.headerSize = headerSize;
    threadParameters.fileName = fileName;
    threadParameters.owner = this;
    InitializeCriticalSection(&lock);
    workerThread = (HANDLE)_beginthreadex(0, 0, UnknownRecorderThread, &threadParameters, 0, &workerThreadId);
    if (!workerThread)
        return 0;
    interval = vcrFile->memory ? 10 : 200;
    if (mode == 0) {
        SignalAndWait(createFileEvent, interval);
        return 1;
    }
    if (mode == 2) {
        SignalAndWait(reopenFileEvent, interval);
        return 1;
    }
    if (mode == 1) {
        recordBuffer = DebugMalloc(0x400, __FILE__, 535);
        if (!recordBuffer)
            return 0;
        SignalAndWait(openPlaybackEvent, interval);
        frameRing->AppendPosition(-1, 0);
        frameRing->AppendPosition(-1, 0);
    }
    return 1;
}

// 0x004e86d0
void VCRInterface::CloseFile(int header, int headerSize, int wait) {
    int fast = vcrFile->memory;

    threadParameters.header = header;
    threadParameters.headerSize = headerSize;
    if (wait)
        SignalAndWait(closeFileEvent, fast ? 10 : 200);
    else
        SetEvent(closeFileEvent);
}

// 0x004e8720
int VCRInterface::QueueRecord(int type, int id, const void* data, unsigned int size) {
    void* block;
    int* record;
    int index;

    block = 0;
    if (!frameRing)
        return 0;
    EnterCriticalSection(&lock);
    if (isStopped) {
        LeaveCriticalSection(&lock);
        return 0;
    }
    if (!frameRing->ClaimRecordSlot(size + 12, &index, &block)) {
        LeaveCriticalSection(&lock);
        return 0;
    }
    LeaveCriticalSection(&lock);
    record = (int*)block;
    *record++ = type;
    *record++ = id;
    *record++ = size;
    block = record;
    memcpy(block, data, size);
    EnterCriticalSection(&lock);
    frameRing->CommitRecordSlot(index);
    LeaveCriticalSection(&lock);
    SetEvent(writeEvent);
    return 1;
}

// 0x004e8810
int VCRInterface::TakeRecord(int* type, int* id, int* position, void* data, unsigned int* size, int signal) {
    void* block;
    int* record;
    int index;
    unsigned int bytes;
    int result;
    int extra;

    block = 0;
    bytes = 0;
    if (!frameRing)
        return 4;
    EnterCriticalSection(&lock);
    result = frameRing->TakePlaybackSlot(&block, &bytes, &extra, &index);
    if (result == 0) {
        LeaveCriticalSection(&lock);
        if (*(unsigned char*)block == 0xff && extra == -1) {
            frameRing->ReleaseSlot(index);
            SetEvent(readReverseEvent);
            return 3;
        }
        record = (int*)block;
        *type = *record++;
        *id = *record++;
        *size = *record++;
        block = record;
        memcpy(data, block, *size);
        *position = extra;
        EnterCriticalSection(&lock);
        frameRing->ReleaseSlot(index);
        LeaveCriticalSection(&lock);
        if (signal)
            SetEvent(readForwardEvent);
        else
            SetEvent(readReverseEvent);
        return 0;
    }
    if (result == 1) {
        LeaveCriticalSection(&lock);
        StopReading();
        return 1;
    }
    if (result == 2) {
        LeaveCriticalSection(&lock);
        StopReading();
        return 2;
    }
    LeaveCriticalSection(&lock);
    return 4;
}

// 0x004e8990
void VCRInterface::StopReading() {
    EnterCriticalSection(&lock);
    readsReset = 0;
    fileReady = 0;
    LeaveCriticalSection(&lock);
    SetEvent(stopReadingEvent);
}

// 0x004e89c0
void VCRInterface::RestartReverseRead() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        isBusy = 1;
        LeaveCriticalSection(&lock);
        EnterCriticalSection(&lock);
        frameRing->ResetSlots();
        isBusy = 0;
        LeaveCriticalSection(&lock);
        SetEvent(readReverseEvent);
    }
}

// 0x004e8a20
void VCRInterface::Seek(unsigned int milliseconds) {
    if (frameRing) {
        SetEvent(seekEvent);
        sceneTimePending = 1;
        pendingSceneTime = milliseconds * 0.001f;
    }
}

// 0x004e8a70
void VCRInterface::Rewind() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        sceneTimePending = 1;
        pendingSceneTime = 0;
        fileReady = 0;
        frameRing->ClearPositions();
        SetEvent(rewindEvent);
        LeaveCriticalSection(&lock);
    }
}
