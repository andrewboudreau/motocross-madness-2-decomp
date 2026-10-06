#pragma once

#include "ContainerList.h"
#include "DebugAlloc.h"

// RTTI: OverlayIconService (root, vtable 0x00555a9c; 0x1c bytes). Retail
// cites D:\aardvark\VC\krusty2\OverlayIconService.h (0x0056f77c) from
// Overlay 0x004b5f50, which allocates one at line 90; its out-of-line
// constructor (0x004b6a70; whether it belongs to overlay.cpp or to a
// file of its own is not established) and destructor (0x004b6b10, scalar deleting
// wrapper 0x004b6af0) follow overlay.cpp's code. The element type of the
// list is not established (4 bytes); names are provisional.
class OverlayIconService {
public:
    // 0x004b6a70: a list with room for (and growing by) `capacity`. Retail
    // calls it out of line; Overlay.cpp defines it.
    OverlayIconService(int capacity);
    virtual ~OverlayIconService() {}

    int field_0x04;                           // not touched here
    ContainerList<void*> field_0x08;
};

// The allocation Overlay 0x004b5f50 inlines (OverlayIconService.h line 90).
inline OverlayIconService* UnknownCreateOverlayIconService() {
    return new(__FILE__, 90) OverlayIconService(10);
}
