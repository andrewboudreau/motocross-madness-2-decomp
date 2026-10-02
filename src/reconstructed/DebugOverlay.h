#pragma once

#include "GameObject.h"

class UnknownGameOwned;
class TextureMapManager;

// RTTI: DebugOverlay : GameObject (0x36e0 bytes, the size Game's initialiser
// allocates). DebugOverlay.cpp is the nearest source literal. Only the
// members Game calls are declared; none is reconstructed.
class DebugOverlay : public GameObject {
public:
    explicit DebugOverlay(int flags);                                   // 0x00447920
    // 0x00447de0: sets the overlay up; returns it, or 0.
    DebugOverlay* UnknownFunction447de0(UnknownGameOwned* a, TextureMapManager* b,
                                        int c, int d, int e);
    int UnknownFunction4484f0();                                        // 0x004484f0
    // 0x00447f40 and 0x00447fa0: format a row onto page `page` when that page
    // is shown (+0x25 bit 0 and +0x26c4); cdecl, `this` on the stack.
    void UnknownFunction447f40(int page, const char* format, ...);
    void UnknownFunction447fa0(int page, const char* format, ...);
    int NewPage() { return field_0x26c0++; }

    char field_0x2c[0x80];                    // formatted row
    int field_0xac;                           // rows on the shown page
    unsigned char field_0xb0[0x26c0 - 0xb0];
    int field_0x26c0;                         // next page number
    int field_0x26c4;                         // page shown
    int field_0x26c8;                         // row limit
    unsigned char field_0x26cc[0x36dc - 0x26cc];
    int field_0x36dc;                         // Game slot 8: "[this overlay %d]"
};
