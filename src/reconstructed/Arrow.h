#pragma once

// Arrow.h -- the reconstructed D:\aardvark\VC\krusty2\Arrow.cpp (literal
// __FILE__ at 0x00566684, single xref 0x00401979 in the static initializer).
// Code 0x00401950..0x00401a2a: the pre-main thunk 0x00401950, the
// initializer 0x00401960, then the ArrowManager constructor, deleting
// destructor and destructor. SoundGroup (0x00401a30, vtable 0x00550500)
// follows; it is not claimed for this file.
//
// Confirmed (tier 1): RTTI .?AVArrowManager@@ (COL 0x0055a7c8, vtable
// 0x00550490, 27 slots), single base GameObject at mdisp 0. Overrides: slot
// 0 (deleting destructor 0x00401a00) and slot 8, the shared base-forwarding
// body 0x00462e30 (also NullManager's). Allocation size 0x41c (line 4).
// Member names are provisional (tier 3).

#include "GameObject.h"

class ArrowManager : public GameObject {
public:
    explicit ArrowManager(int flags);          // 0x004019d0
    virtual ~ArrowManager();                   // 0x00401a20 (deleting wrapper 0x00401a00)
    virtual GameObject* UnknownVirtualSlot8(void* value); // shared body 0x00462e30

    unsigned char field_0x2c[0x414 - 0x2c];
    int field_0x414;
    float field_0x418;                         // 1 by default
};

// 0x005776d0: written by the initializer only (0 when the allocation fails).
extern ArrowManager* g_UnknownArrowManager5776d0;
