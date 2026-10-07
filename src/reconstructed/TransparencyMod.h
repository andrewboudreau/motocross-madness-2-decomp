#pragma once

// TransparencyMod.h -- RTTI .?AVTransparencyMod@@ (COL 0x0055f890, vtable
// 0x005588c4, 28 slots): TransparencyMod : D3DIMSoultreeModifier :
// GraphicsTest : GameObject : BaseObject, all single non-virtual at mdisp 0
// (tier 1). It overrides slot 0 (deleting destructor 0x005206d0) and slot 27
// (0x005206f0). The constructor 0x005206a0 (ArcadeObject.cpp line 104, new
// 0x4c) and the destructor 0x00520810 write the vptr.
//
// Code 0x005206a0..0x0052081a, after the TrackRecord code and before
// trkgame.cpp (first xref 0x00521086). No __FILE__ literal, so the file
// name 'TransparencyMod.cpp' is ours (tier 3), as are the member names.
//
// What slot 27 does (tier 3): while field_0x44 differs from field_0x40 it
// counts the surfaces drawn (field_0x49); after a whole level of detail it
// latches field_0x44 into field_0x40. Each surface gets the alpha byte
// field_0x48 in its vertex colours when field_0x44 is set, opaque colours
// otherwise, and the object's materials get the matching alpha flag.

#include "D3DIMSoultreeModifier.h"
#include "MorphBastardModifier.h"   // UnknownSoultreeMesh

class TransparencyMod : public D3DIMSoultreeModifier {
public:
    explicit TransparencyMod(int flags);       // 0x005206a0 (ret 4)
    virtual ~TransparencyMod();                // 0x00520810 (deleting wrapper 0x005206d0)
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out); // 0x005206f0

    int appliedState;                          // state last applied to a whole level of detail
    int isTransparent;                         // 1: transparent
    unsigned char field_0x48;                  // alpha, 0x80 by default
    unsigned char surfacesDrawn;               // surfaces drawn since field_0x44 changed
};
