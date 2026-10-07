// MediaControl.cpp -- see MediaControl.h for the evidence.

#include <windows.h>

#include "MediaControl.h"

#include "DebugAlloc.h"

extern "C" const UnknownGuid CLSID_FilterGraph;           // 0x00558ee0
extern "C" const UnknownGuid IID_IGraphBuilder;           // 0x00558ef0

// 0x004a2560 (open) and 0x004a2900 (restart) are near misses:
// samples/gameui/MediaControlNearMisses.cpp.

// 0x004a23d0
int MediaControl::UnknownFunction4a23d0()
{
    UnknownSurfaceInterface* graph;
    if (CoCreateInstance(*(const GUID*)&CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
                         *(const GUID*)&IID_IGraphBuilder, (void**)&graph) >= 0) {
        graph->UnknownMethod2();
        return 1;
    }
    return 0;
}

// 0x004a2410
MediaControl::MediaControl(int a)
    : GameObject(a)
{
    field_0x68_low = 0;
    field_0x78_bit0 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x2c = 0;
    field_0x38 = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x54 = 0;
    field_0x58 = 0;
    field_0x70 = 0;
    field_0x74 = 0;
    field_0x68_high = 0;
}

// 0x004a2490
MediaControl::~MediaControl()
{
    UnknownFunction4a2940();
    if (field_0x2c) {
        field_0x2c->UnknownMethod2();
        field_0x2c = 0;
    }
    if (field_0x38) {
        field_0x38->UnknownMethod2();
        field_0x38 = 0;
    }
    if (field_0x64) {
        field_0x64->UnknownMethod2();
        field_0x64 = 0;
    }
    if (field_0x60) {
        field_0x60->UnknownMethod2();
        field_0x60 = 0;
    }
    if (field_0x5c) {
        field_0x5c->UnknownMethod2();
        field_0x5c = 0;
    }
    if (field_0x50) {
        field_0x50->UnknownMethod2();
        field_0x50 = 0;
    }
    if (field_0x4c) {
        field_0x4c->UnknownMethod2();
        field_0x4c = 0;
    }
    if (field_0x54) {
        field_0x54->UnknownMethod2();
        field_0x54 = 0;
    }
}

// 0x004a27f0
int MediaControl::UnknownVirtualSlot10(float frameTime)
{
    __int64 time;
    __int64 duration;

    GameObject::UnknownVirtualSlot10(frameTime);
    if (!field_0x64) {
        return 0;
    }
    if (UnknownFunction4a2a20()) {
        UnknownFunction4a2a50(&time);
        UnknownFunction4a2a70(&duration);
        if (time >= duration) {
            UnknownFunction4a2940();
            if (field_0x70) {
                field_0x70(field_0x74);
            }
        } else if (field_0x64->UnknownMethod7(2, 25) == 0) {
            UnknownFunction4a2a90(field_0x2c, field_0x3c, 0, 0x1000000);
            if (field_0x64->UnknownMethod6(1, 0, 0, 0) < 0) {
                return 0;
            }
        }
    }
    return 1;
}

// 0x004a28c0
int MediaControl::UnknownVirtualSlot17()
{
    UnknownVirtualSlot16(1);
    return GameObject::UnknownVirtualSlot17();
}

// 0x004a28e0
int MediaControl::UnknownVirtualSlot18()
{
    UnknownVirtualSlot16(0);
    return GameObject::UnknownVirtualSlot18();
}

// 0x004a2940
void MediaControl::UnknownFunction4a2940()
{
    if (UnknownFunction4a2a20() && field_0x50->UnknownMethod7(0) >= 0) {
        UnknownFunction4a2a10();
    }
}

// 0x004a2970
int MediaControl::UnknownVirtualSlot16(int value)
{
    if (value) {
        if (!field_0x25_bit2) {
            field_0x78_bit0 = UnknownFunction4a2a20();
            UnknownFunction4a2a50(&field_0x68);
            field_0x50->UnknownMethod7(0);
        }
    } else if (field_0x78_bit0) {
        UnknownFunction4a29f0(field_0x68);
        field_0x50->UnknownMethod7(1);
    }
    return GameObject::UnknownVirtualSlot16(value);
}

// 0x004a29f0
int MediaControl::UnknownFunction4a29f0(__int64 time)
{
    return field_0x50->UnknownMethod10(time) >= 0;
}

// 0x004a2a10
int MediaControl::UnknownFunction4a2a10()
{
    return UnknownFunction4a29f0(0);
}

// 0x004a2a20
int MediaControl::UnknownFunction4a2a20()
{
    int state;
    if (field_0x50->UnknownMethod6(&state) >= 0 && state == 1) {
        return 1;
    }
    return 0;
}

// 0x004a2a50
int MediaControl::UnknownFunction4a2a50(__int64* time)
{
    return field_0x50->UnknownMethod8(time) >= 0;
}

// 0x004a2a70
int MediaControl::UnknownFunction4a2a70(__int64* duration)
{
    return field_0x50->UnknownMethod9(duration) >= 0;
}

// 0x004a2a90
long MediaControl::UnknownFunction4a2a90(UnknownSurfaceInterface* destination, void* destinationRect,
                                         void* sourceRect, int flags)
{
    if (!field_0x38) {
        return 0x80004005;
    }
    return destination->UnknownMethod5(destinationRect, field_0x38, sourceRect, flags, 0);
}
