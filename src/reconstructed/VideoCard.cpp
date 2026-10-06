#include <stdlib.h>

#include "VideoCard.h"

#include "DebugAlloc.h"
#include "Display.h"
#include "Game.h"

// VideoCard.cpp (literal __FILE__ at 0x0052d22a). Retail code
// 0x0052d0d0..0x0052d2bc; the vfwdeco.cpp code comes before it and the
// next TU's initializers (0x0052d2e0..) after it. Names are provisional.

// 0x0052d0d0
void UnknownFunction52d0d0() {
    for (int i = 0; i < g_UnknownDisplayCount68a764; i++) {
        if (g_UnknownDisplays68a754[i]) {
            delete g_UnknownDisplays68a754[i];
            g_UnknownDisplays68a754[i] = 0;
        }
    }
    g_UnknownDisplayCount68a764 = 0;
}

// 0x0052d120
int UnknownCompare52d120(const void* first, const void* second) {
    const UnknownDisplayMode* a = (const UnknownDisplayMode*)first;
    const UnknownDisplayMode* b = (const UnknownDisplayMode*)second;
    if (a->bitDepth != b->bitDepth)
        return a->bitDepth - b->bitDepth;
    if (a->height != b->height)
        return a->height - b->height;
    if (a->width != b->width)
        return a->width - b->width;
    if (a->field_0x10 != b->field_0x10)
        return a->field_0x10 - b->field_0x10;
    if (a->refreshRate != b->refreshRate)
        return a->refreshRate - b->refreshRate;
    return 0;
}

// 0x0052d240
VideoCard::~VideoCard() {
    UnknownVirtualSlot1();
}

// 0x0052d220
void VideoCard::UnknownVirtualSlot1() {
    if (field_0x10)
        operator delete(field_0x10, __FILE__, 63);
}

// 0x0052d250
int VideoCard::UnknownFunction52d250(int width, int height, int bitDepth, int a, int b) {
    qsort(field_0x10, field_0x08, sizeof(UnknownDisplayMode), UnknownCompare52d120);
    for (int i = 0; i < field_0x08; i++) {
        UnknownDisplayMode* mode = &field_0x10[i];
        if (mode->bitDepth == bitDepth && mode->height == height && mode->width == width &&
            (!a || mode->field_0x10 == a) && (!b || mode->refreshRate == b))
            return i;
    }
    return -1;
}
