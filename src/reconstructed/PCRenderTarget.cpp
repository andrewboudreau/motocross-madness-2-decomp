#include <string.h>

#include "PCRenderTarget.h"
#include "DebugAlloc.h"

// 0x004c4ee0: every cached render state starts as {i, 0}.
PCRenderTarget::PCRenderTarget() {
    memset(&field_0x54, 0, sizeof(field_0x54));
    field_0x48 = 0;
    field_0x50 = 0;
    field_0x4c = 0;
    field_0x254 = 0;
    field_0x25c = 0;
    field_0x258 = 0;
    field_0x260 = 0;
    field_0x250 = 3;
    memset(field_0x264, 0, sizeof(field_0x264));
    for (int i = 0; i < 300; i++)
        field_0x264[i].state = i;
}

// 0x004c5320
PCRenderTarget::~PCRenderTarget() {
    if (field_0x258) {
        operator delete(field_0x258, __FILE__, 196);
        field_0x258 = 0;
        field_0x254 = 0;
    }
    if (field_0x50) {
        field_0x50->UnknownMethod2();
        field_0x50 = 0;
    }
    if (field_0x4c) {
        field_0x4c->UnknownMethod2();
        field_0x4c = 0;
    }
    if (field_0x260)
        operator delete(field_0x260, __FILE__, 210);
}

// 0x004c53d0
int PCRenderTarget::UnknownVirtualSlot1() {
    return field_0x50->UnknownMethod5() == 0;
}

// 0x004c53e0
int PCRenderTarget::UnknownVirtualSlot2() {
    return field_0x50->UnknownMethod6() == 0;
}

// 0x004c53f0
int PCRenderTarget::UnknownVirtualSlot3(void* destination, void* source, void* sourceRect, int flags) {
    return field_0x48->UnknownMethod5(destination, static_cast<UnknownBlitSource*>(source)->field_0x70,
                                      sourceRect, flags, 0) == 0;
}

// 0x004c5490
int PCRenderTarget::UnknownVirtualSlot5(void* rect) {
    return field_0x48->UnknownMethod32(rect) == 0;
}

// 0x004c54b0
long PCRenderTarget::UnknownVirtualSlot6(int stage, int type, int* value) {
    return field_0x50->UnknownMethod36(stage, type, value);
}

// 0x004c54d0
long PCRenderTarget::UnknownVirtualSlot7(int stage, int type, int value) {
    return field_0x50->UnknownMethod37(stage, type, value);
}

// 0x004c54f0
int PCRenderTarget::UnknownVirtualSlot14(void* viewport) {
    return field_0x50->UnknownMethod13(viewport) == 0;
}

// 0x004c56e0: skips states whose cached value already matches unless forced.
void PCRenderTarget::UnknownVirtualSlot8(int state, int value, int force) {
    if (force || field_0x264[state].value != value) {
        field_0x50->UnknownMethod20(state, value);
        field_0x264[state].value = value;
    }
}

// 0x004c5720
long PCRenderTarget::UnknownVirtualSlot9(int state, int* value) {
    return field_0x50->UnknownMethod21(state, value);
}

// 0x004c5930
long PCRenderTarget::UnknownVirtualSlot11(int stage) {
    return field_0x50->UnknownMethod35(stage, 0);
}

// 0x004c5ed0: three render states (0x19 = 5, 0x18 = 0, 0x0f = 0), unforced.
void PCRenderTarget::UnknownVirtualSlot19() {
    UnknownVirtualSlot8(0x19, 5, 0);
    UnknownVirtualSlot8(0x18, 0, 0);
    UnknownVirtualSlot8(0x0F, 0, 0);
}
