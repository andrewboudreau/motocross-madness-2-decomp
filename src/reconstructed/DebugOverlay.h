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

    unsigned char field_0x2c[0x36e0 - 0x2c];
};
