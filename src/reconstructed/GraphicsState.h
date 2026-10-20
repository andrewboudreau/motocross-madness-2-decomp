#pragma once

#include "Guid.h"

struct UnknownDisplay;
struct UnknownSurfaceInterface;

// Shared storage used by camera, display enumeration and texture management.
// These declarations join confirmed addresses; the original owning TUs are
// not established by this grouping. See docs/GRAPHICS_STATE.md.
extern const float g_DefaultNearPlane;             // 0x00550f6c
extern const float g_DefaultFarPlane;              // 0x00550f70
extern const UnknownGuid g_BladeDeviceGuid;         // 0x00556040
extern unsigned short g_GammaRamp[3][256];         // 0x00570570
extern int g_DebugCountTextureMemory;              // 0x00689964
extern UnknownSurfaceInterface* g_SharedMipTextureSurfaces[10]; // 0x0068a36c
extern UnknownSurfaceInterface* g_SharedTextureSurfaces[10];    // 0x0068a394
extern UnknownDisplay* g_Displays[4];              // 0x0068a754
extern int g_DisplayCount;                         // 0x0068a764
