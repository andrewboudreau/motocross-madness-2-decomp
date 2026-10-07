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
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x44 = 0;
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    stopEvent = 0;
    workerThreadId = 0;
    frameRing = 0;
    recordBuffer = 0;
    field_0x88 = 0;
    field_0x84 = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    isBusy = 0;
    field_0xbc = 0;
    field_0xc0 = 0;
    field_0xc4 = 0;
    field_0xcc = 0;
    field_0xd0 = 0;
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
    CLOSE_HANDLE(field_0x30);
    CLOSE_HANDLE(field_0x34);
    CLOSE_HANDLE(field_0x38);
    CLOSE_HANDLE(field_0x40);
    CLOSE_HANDLE(field_0x3c);
    CLOSE_HANDLE(field_0x44);
    CLOSE_HANDLE(field_0x48);
    CLOSE_HANDLE(field_0x4c);
    CLOSE_HANDLE(field_0x50);
    CLOSE_HANDLE(field_0x54);
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
int VCRInterface::Start(int a1, int a2, int a3, int mode, int a5, unsigned int size,
                                        UnknownRecorderOwner* owner) {
    DWORD interval;

    frameRing = new (__FILE__, 477) UnknownVcr(size);
    if (!frameRing)
        return 0;
    vcrFile = owner;
    recordCallback = (UnknownRecorderCallback)a1;
    field_0x30 = CreateEventA(0, 0, 0, 0);
    field_0x34 = CreateEventA(0, 0, 0, 0);
    field_0x38 = CreateEventA(0, 0, 0, 0);
    field_0x3c = CreateEventA(0, 0, 0, 0);
    field_0x40 = CreateEventA(0, 0, 0, 0);
    field_0x44 = CreateEventA(0, 0, 0, 0);
    field_0x48 = CreateEventA(0, 0, 0, 0);
    field_0x4c = CreateEventA(0, 0, 0, 0);
    field_0x50 = CreateEventA(0, 0, 0, 0);
    field_0x54 = CreateEventA(0, 0, 0, 0);
    stopEvent = CreateEventA(0, 0, 0, 0);
    if (!field_0x30 || !stopEvent || !field_0x34 || !field_0x38 || !field_0x3c || !field_0x40 ||
        !field_0x44 || !field_0x48 || !field_0x4c || !field_0x50 || !field_0x54)
        return 0;
    recordMode = mode;
    threadParameters.field_0x00 = a2;
    threadParameters.field_0x04 = a3;
    threadParameters.field_0x08 = a5;
    threadParameters.owner = this;
    InitializeCriticalSection(&lock);
    workerThread = (HANDLE)_beginthreadex(0, 0, UnknownRecorderThread, &threadParameters, 0, &workerThreadId);
    if (!workerThread)
        return 0;
    interval = vcrFile->field_0x23c ? 10 : 200;
    if (mode == 0) {
        SignalAndWait(field_0x34, interval);
        return 1;
    }
    if (mode == 2) {
        SignalAndWait(field_0x38, interval);
        return 1;
    }
    if (mode == 1) {
        recordBuffer = DebugMalloc(0x400, __FILE__, 535);
        if (!recordBuffer)
            return 0;
        SignalAndWait(field_0x3c, interval);
        frameRing->UnknownFunction524590(-1, 0);
        frameRing->UnknownFunction524590(-1, 0);
    }
    return 1;
}

// 0x004e86d0
void VCRInterface::UnknownFunction4e86d0(int a, int b, int wait) {
    int fast = vcrFile->field_0x23c;

    threadParameters.field_0x00 = a;
    threadParameters.field_0x04 = b;
    if (wait)
        SignalAndWait(field_0x54, fast ? 10 : 200);
    else
        SetEvent(field_0x54);
}

// 0x004e8720
int VCRInterface::QueueRecord(int a, int b, const void* data, unsigned int size) {
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
    if (!frameRing->UnknownFunction524220(size + 12, &index, &block)) {
        LeaveCriticalSection(&lock);
        return 0;
    }
    LeaveCriticalSection(&lock);
    record = (int*)block;
    *record++ = a;
    *record++ = b;
    *record++ = size;
    block = record;
    memcpy(block, data, size);
    EnterCriticalSection(&lock);
    frameRing->UnknownFunction5242e0(index);
    LeaveCriticalSection(&lock);
    SetEvent(field_0x30);
    return 1;
}

// 0x004e8810
int VCRInterface::TakeRecord(int* a, int* b, int* value, void* data, unsigned int* size, int signal) {
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
    result = frameRing->UnknownFunction524460(&block, &bytes, &extra, &index);
    if (result == 0) {
        LeaveCriticalSection(&lock);
        if (*(unsigned char*)block == 0xff && extra == -1) {
            frameRing->UnknownFunction524540(index);
            SetEvent(field_0x44);
            return 3;
        }
        record = (int*)block;
        *a = *record++;
        *b = *record++;
        *size = *record++;
        block = record;
        memcpy(data, block, *size);
        *value = extra;
        EnterCriticalSection(&lock);
        frameRing->UnknownFunction524540(index);
        LeaveCriticalSection(&lock);
        if (signal)
            SetEvent(field_0x40);
        else
            SetEvent(field_0x44);
        return 0;
    }
    if (result == 1) {
        LeaveCriticalSection(&lock);
        UnknownFunction4e8990();
        return 1;
    }
    if (result == 2) {
        LeaveCriticalSection(&lock);
        UnknownFunction4e8990();
        return 2;
    }
    LeaveCriticalSection(&lock);
    return 4;
}

// 0x004e8990
void VCRInterface::UnknownFunction4e8990() {
    EnterCriticalSection(&lock);
    field_0xc0 = 0;
    field_0xbc = 0;
    LeaveCriticalSection(&lock);
    SetEvent(field_0x50);
}

// 0x004e89c0
void VCRInterface::UnknownFunction4e89c0() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        isBusy = 1;
        LeaveCriticalSection(&lock);
        EnterCriticalSection(&lock);
        frameRing->UnknownFunction524910();
        isBusy = 0;
        LeaveCriticalSection(&lock);
        SetEvent(field_0x44);
    }
}

// 0x004e8a20
void VCRInterface::UnknownFunction4e8a20(unsigned int milliseconds) {
    if (frameRing) {
        SetEvent(field_0x48);
        field_0xcc = 1;
        field_0xd0 = milliseconds * 0.001f;
    }
}

// 0x004e8a70
void VCRInterface::UnknownFunction4e8a70() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        field_0xcc = 1;
        field_0xd0 = 0;
        field_0xbc = 0;
        frameRing->UnknownFunction524640();
        SetEvent(field_0x4c);
        LeaveCriticalSection(&lock);
    }
}
