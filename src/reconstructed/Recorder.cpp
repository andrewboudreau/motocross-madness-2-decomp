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

// The four per-TU vector constants (see src/krusty2/math/Math3D.h):
// 0x00689c10, 0x00689c20, 0x00689c60 and 0x00689c00, initialised by
// 0x004e6e40..0x004e6f7b.
static const Vector3 s_UnknownVector689c10 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689c20 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689c60 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector689c00 = Vector3(0.0f, 0.0f, 1.0f);

unsigned __stdcall UnknownRecorderThread(void* parameters); // 0x004e6f80

// 0x004e77f0
VCRInterface::VCRInterface() : GameObject(1) {
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x44 = 0;
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0xb0 = 0;
    field_0x7c = 0;
    field_0x88 = 0;
    field_0x84 = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xb8 = 0;
    field_0xbc = 0;
    field_0xc0 = 0;
    field_0xc4 = 0;
    field_0xcc = 0;
    field_0xd0 = 0;
    field_0xb4 = 1;
    field_0xc8 = 1;
}

// 0x004e78b0
VCRInterface::~VCRInterface() {
    if (field_0x2c) {
        SetEvent(field_0x58);
        WaitForSingleObject(field_0x2c, INFINITE);
        CLOSE_HANDLE(field_0x2c);
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
    CLOSE_HANDLE(field_0x58);
    DeleteCriticalSection(&field_0x60);
    if (field_0xb0)
        delete field_0xb0;
    if (field_0x7c)
        operator delete(field_0x7c, __FILE__, 442);
}

// 0x004e7a00
void VCRInterface::UnknownFunction4e7a00(HANDLE event, DWORD interval) {
    int waiting;

    EnterCriticalSection(&field_0x60);
    field_0xc8 = 0;
    LeaveCriticalSection(&field_0x60);
    SetEvent(event);
    Sleep(interval);
    EnterCriticalSection(&field_0x60);
    waiting = !field_0xc8;
    LeaveCriticalSection(&field_0x60);
    while (waiting) {
        Sleep(interval);
        EnterCriticalSection(&field_0x60);
        waiting = !field_0xc8;
        LeaveCriticalSection(&field_0x60);
    }
}

// 0x004e7a90
int VCRInterface::UnknownFunction4e7a90(int a1, int a2, int a3, int mode, int a5, unsigned int size,
                                        UnknownRecorderOwner* owner) {
    DWORD interval;

    field_0xb0 = new (__FILE__, 477) UnknownVcr(size);
    if (!field_0xb0)
        return 0;
    field_0xac = owner;
    field_0x80 = a1;
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
    field_0x58 = CreateEventA(0, 0, 0, 0);
    if (!field_0x30 || !field_0x58 || !field_0x34 || !field_0x38 || !field_0x3c || !field_0x40 ||
        !field_0x44 || !field_0x48 || !field_0x4c || !field_0x50 || !field_0x54)
        return 0;
    field_0x78 = mode;
    field_0x90.field_0x00 = a2;
    field_0x90.field_0x04 = a3;
    field_0x90.field_0x08 = a5;
    field_0x90.owner = this;
    InitializeCriticalSection(&field_0x60);
    field_0x2c = (HANDLE)_beginthreadex(0, 0, UnknownRecorderThread, &field_0x90, 0, &field_0x5c);
    if (!field_0x2c)
        return 0;
    interval = field_0xac->field_0x23c ? 10 : 200;
    if (mode == 0) {
        UnknownFunction4e7a00(field_0x34, interval);
        return 1;
    }
    if (mode == 2) {
        UnknownFunction4e7a00(field_0x38, interval);
        return 1;
    }
    if (mode == 1) {
        field_0x7c = DebugMalloc(0x400, __FILE__, 535);
        if (!field_0x7c)
            return 0;
        UnknownFunction4e7a00(field_0x3c, interval);
        field_0xb0->UnknownFunction524590(-1, 0);
        field_0xb0->UnknownFunction524590(-1, 0);
    }
    return 1;
}

// 0x004e86d0
void VCRInterface::UnknownFunction4e86d0(int a, int b, int wait) {
    int fast = field_0xac->field_0x23c;

    field_0x90.field_0x00 = a;
    field_0x90.field_0x04 = b;
    if (wait)
        UnknownFunction4e7a00(field_0x54, fast ? 10 : 200);
    else
        SetEvent(field_0x54);
}

// 0x004e8720
int VCRInterface::UnknownFunction4e8720(int a, int b, const void* data, unsigned int size) {
    void* block;
    int* record;
    int index;

    block = 0;
    if (!field_0xb0)
        return 0;
    EnterCriticalSection(&field_0x60);
    if (field_0xb4) {
        LeaveCriticalSection(&field_0x60);
        return 0;
    }
    if (!field_0xb0->UnknownFunction524220(size + 12, &index, &block)) {
        LeaveCriticalSection(&field_0x60);
        return 0;
    }
    LeaveCriticalSection(&field_0x60);
    record = (int*)block;
    *record++ = a;
    *record++ = b;
    *record++ = size;
    block = record;
    memcpy(block, data, size);
    EnterCriticalSection(&field_0x60);
    field_0xb0->UnknownFunction5242e0(index);
    LeaveCriticalSection(&field_0x60);
    SetEvent(field_0x30);
    return 1;
}

// 0x004e8810
int VCRInterface::UnknownFunction4e8810(int* a, int* b, int* value, void* data, unsigned int* size, int signal) {
    void* block;
    int* record;
    int index;
    unsigned int bytes;
    int result;
    int extra;

    block = 0;
    bytes = 0;
    if (!field_0xb0)
        return 4;
    EnterCriticalSection(&field_0x60);
    result = field_0xb0->UnknownFunction524460(&block, &bytes, &extra, &index);
    if (result == 0) {
        LeaveCriticalSection(&field_0x60);
        if (*(unsigned char*)block == 0xff && extra == -1) {
            field_0xb0->UnknownFunction524540(index);
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
        EnterCriticalSection(&field_0x60);
        field_0xb0->UnknownFunction524540(index);
        LeaveCriticalSection(&field_0x60);
        if (signal)
            SetEvent(field_0x40);
        else
            SetEvent(field_0x44);
        return 0;
    }
    if (result == 1) {
        LeaveCriticalSection(&field_0x60);
        UnknownFunction4e8990();
        return 1;
    }
    if (result == 2) {
        LeaveCriticalSection(&field_0x60);
        UnknownFunction4e8990();
        return 2;
    }
    LeaveCriticalSection(&field_0x60);
    return 4;
}

// 0x004e8990
void VCRInterface::UnknownFunction4e8990() {
    EnterCriticalSection(&field_0x60);
    field_0xc0 = 0;
    field_0xbc = 0;
    LeaveCriticalSection(&field_0x60);
    SetEvent(field_0x50);
}

// 0x004e89c0
void VCRInterface::UnknownFunction4e89c0() {
    if (field_0xb0) {
        EnterCriticalSection(&field_0x60);
        field_0xb8 = 1;
        LeaveCriticalSection(&field_0x60);
        EnterCriticalSection(&field_0x60);
        field_0xb0->UnknownFunction524910();
        field_0xb8 = 0;
        LeaveCriticalSection(&field_0x60);
        SetEvent(field_0x44);
    }
}

// 0x004e8a20
void VCRInterface::UnknownFunction4e8a20(unsigned int milliseconds) {
    if (field_0xb0) {
        SetEvent(field_0x48);
        field_0xcc = 1;
        field_0xd0 = milliseconds * 0.001f;
    }
}

// 0x004e8a70
void VCRInterface::UnknownFunction4e8a70() {
    if (field_0xb0) {
        EnterCriticalSection(&field_0x60);
        field_0xcc = 1;
        field_0xd0 = 0;
        field_0xbc = 0;
        field_0xb0->UnknownFunction524640();
        SetEvent(field_0x4c);
        LeaveCriticalSection(&field_0x60);
    }
}
