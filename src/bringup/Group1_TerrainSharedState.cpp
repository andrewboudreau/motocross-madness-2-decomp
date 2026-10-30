// Group1_TerrainSharedState.cpp -- bring-up bodies for Terrain.h's TerrainSharedState
// stand-in (the 0x220-byte block behind g_terrainSharedState, 0x0068a090).
//
// Retail 0x004bed80 is PCCamera::PCCamera(int): it calls Camera::Camera(flags)
// (0x0042e340) and stores PCCamera's vptr 0x00555da4.  Retail 0x004624d0 is the
// compiler-generated ~PCCamera, a tail jump to Camera::~Camera (0x0042f020).
// Terrain.h keeps the block opaque (int words[0x88]); these bodies construct and
// destroy a real PCCamera in that storage, which is what retail does.

#include <new.h>

#include "broadphase/Terrain.h"
#include "PCCamera.h"

// 0x004bed80 (thiscall, ret 4)
TerrainSharedState::TerrainSharedState(int flags)
{
    // sizeof(PCCamera) == 0x220 == sizeof(words): the block is the camera.
    new (this) PCCamera(flags);
}

// 0x004624d0: jmp Camera::~Camera.  The qualified call is the non-virtual,
// non-deleting destructor body retail jumps to.
TerrainSharedState::~TerrainSharedState()
{
    reinterpret_cast<PCCamera*>(this)->Camera::~Camera();
}
