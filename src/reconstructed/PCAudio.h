#pragma once

#include <windows.h>

#include "ContainerList.h"

// 0x54-byte helper PCSoundInterface owns at +0x46c (not RTTI-typed): a
// worker thread with two events, a critical section and two pointer lists.
// Defined in PCAudio.cpp (its constructor and destructor precede
// PCSoundInterface there).
class UnknownPCAudioObject {
public:
    UnknownPCAudioObject();  // 0x004bddf0
    ~UnknownPCAudioObject(); // 0x004bde40: stops the thread, closes the handles
    int UnknownFunction4bdef0(int value); // 0x004bdef0

    HANDLE field_0x00; // thread
    HANDLE field_0x04; // event signalled to stop the thread
    HANDLE field_0x08;
    CRITICAL_SECTION field_0x0c;
    int field_0x24;
    int field_0x28;
    ContainerList<void*> field_0x2c;
    ContainerList<void*> field_0x40;
};
