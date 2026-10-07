#pragma once

// RTTI: SkyCube : DrawableCube : GameObject (COL 0x0055f1d8, vtable
// 0x00557ba8, slots 0 and 10 overridden), 0x48 bytes: a DrawableCube kept
// centred on the render target's camera. Its code sits at
// 0x004fb1e0..0x004fb2ab, after the Shock family and before soultree.cpp
// (first xref 0x004fdb7d); the TU name is unattested and the member names
// are provisional. QuarryStuntEvent.cpp's loader creates it (line 744).

#include "CubeDraw.h"

class SkyCube : public DrawableCube {
public:
    explicit SkyCube(int flags);              // 0x004fb1e0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004fb260: recentres the cube
    // 0x004fb230: DrawableCube's loader, then the height above the camera.
    SkyCube* UnknownFunction4fb230(void* value, UnknownTextureStream* stream, UnknownCubeTextureContext* context,
                                   float height);

    float field_0x44;                         // height of the cube's centre above the camera
};
