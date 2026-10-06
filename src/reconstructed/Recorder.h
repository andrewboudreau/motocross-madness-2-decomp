#pragma once

#include <windows.h>

#include "GameObject.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\recorder.cpp (literal
// __FILE__ at 0x00572b04). Evidence and the per-function table are in
// docs/RECORDER.md.
//
// RTTI: VCRInterface : GameObject (vtable 0x0055778c, 27 slots; overrides
// slot 0, deleting destructor 0x004e7890, and slot 10 0x004e7d60) and
// KrustyVCR : VCRInterface (vtable 0x00550e9c). The recorder runs a worker
// thread (0x004e6f80) that the game drives through eleven events and a
// critical section; recorded frames pass through an UnknownVcr ring
// (VCR.h). Names other than the RTTI classes are provisional.

class UnknownVcr;

// The record buffer at VCRInterface+0x7c (0x400 bytes): slot 10 reads a time
// in seconds and a count from its head.
struct UnknownRecorderRecord {
    float time;                           // seconds; stored at +0xd4 in ms
    int field_0x04;
    int count;
};

// The function pointer at VCRInterface+0x80 (cdecl, seven arguments). Slot 10
// passes -2 and -3 as `a` for control calls; the result selects its next step.
typedef int (*UnknownRecorderCallback)(int a, void* data, int flag, int b, int* keep, int time,
                                       int* milliseconds);

// What 0x004e7a90 passes to the worker thread (VCRInterface+0x90).
struct UnknownRecorderThreadParameters {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    class VCRInterface* owner;
};

// The object at VCRInterface+0xac: its +0x23c selects a 10 ms (else 200 ms)
// poll interval. The worker thread calls UnknownVcrFile (VCRfile.h) methods on
// it and +0x23c is that class's memory flag, so it is the VCR file object; the
// type is kept for the 0x004e7a90 symbol.
struct UnknownRecorderOwner {
    unsigned char field_0x000[0x23c];
    int field_0x23c;
};

class VCRInterface : public GameObject {
public:
    VCRInterface();                       // 0x004e77f0
    virtual ~VCRInterface();              // 0x004e78b0 (deleting wrapper 0x004e7890)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004e7d60

    // 0x004e7a00: signals `event` and polls every `interval` ms until the
    // worker acknowledges (+0xc8).
    void UnknownFunction4e7a00(HANDLE event, DWORD interval);
    // 0x004e7a90: creates the ring (`size`), the events and the worker, then
    // starts `mode` (0, 1 or 2); 0 on failure.
    int UnknownFunction4e7a90(int a1, int a2, int a3, int mode, int a5, unsigned int size,
                              UnknownRecorderOwner* owner);
    void UnknownFunction4e86d0(int a, int b, int wait);  // 0x004e86d0
    // 0x004e8720: queues `size` bytes of `data` with two header values; 0 when
    // full or stopped.
    int UnknownFunction4e8720(int a, int b, const void* data, unsigned int size);
    // 0x004e8810: takes the next queued record; 0..4.
    int UnknownFunction4e8810(int* a, int* b, int* value, void* data, unsigned int* size, int signal);
    void UnknownFunction4e8990();         // 0x004e8990
    void UnknownFunction4e89c0();         // 0x004e89c0
    void UnknownFunction4e8a20(unsigned int milliseconds); // 0x004e8a20
    void UnknownFunction4e8a70();         // 0x004e8a70

    HANDLE field_0x2c;                    // worker thread
    HANDLE field_0x30;                    // events (CreateEventA, auto-reset)
    HANDLE field_0x34;
    HANDLE field_0x38;
    HANDLE field_0x3c;
    HANDLE field_0x40;
    HANDLE field_0x44;
    HANDLE field_0x48;
    HANDLE field_0x4c;
    HANDLE field_0x50;
    HANDLE field_0x54;
    HANDLE field_0x58;                    // stops the worker
    unsigned int field_0x5c;              // worker thread id
    CRITICAL_SECTION field_0x60;
    int field_0x78;                       // mode
    void* field_0x7c;                     // 0x400-byte buffer (mode 1)
    UnknownRecorderCallback field_0x80;
    int field_0x84;
    int field_0x88;
    unsigned char field_0x8c[0x90 - 0x8c];
    UnknownRecorderThreadParameters field_0x90;
    int field_0xa0;
    int field_0xa4;
    int field_0xa8;
    UnknownRecorderOwner* field_0xac;
    UnknownVcr* field_0xb0;
    int field_0xb4;                       // stopped
    int field_0xb8;                       // busy
    int field_0xbc;
    int field_0xc0;
    int field_0xc4;
    int field_0xc8;                       // acknowledged by the worker
    int field_0xcc;
    float field_0xd0;                     // seconds
    int field_0xd4;                       // record time in ms (worker event 5, slot 10)
};
