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
// What slot 27 does (tier 3): while isTransparent differs from appliedState
// it counts the surfaces drawn; after a whole level of detail it latches
// isTransparent into appliedState. Each surface gets the alpha byte in its
// vertex colours when isTransparent is set, opaque colours
// otherwise, and the object's materials get the matching alpha flag.

#include "D3DIMSoultreeModifier.h"
#include "MorphBastardModifier.h"   // UnknownSoultreeMesh

class TransparencyMod : public D3DIMSoultreeModifier {
public:
    explicit TransparencyMod(int flags);       // 0x005206a0 (ret 4)
    virtual ~TransparencyMod();                // 0x00520810 (deleting wrapper 0x005206d0)
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out); // 0x005206f0

    int appliedState;                          // +0x40 state last applied to a whole level of detail
    int isTransparent;                         // +0x44 1: transparent
    unsigned char alpha;                       // +0x48 0x80 by default; ArcadeObject 0x00401310 stores its `alpha`
    unsigned char surfacesDrawn;               // +0x49 surfaces drawn since isTransparent changed
};
