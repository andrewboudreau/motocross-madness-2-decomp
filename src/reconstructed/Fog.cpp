// Fog.cpp -- Fog, FogOff and FogOn; see Fog.h for the evidence.

#include <stdio.h>

#include "Fog.h"

#include "Camera.h"
#include "D3DConstants.h"
#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "Display.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// Slot 14 (0x00462850) is a near miss: samples/render/FogNearMisses.cpp.

#define TARGET() ((PCRenderTarget*)field_0x18)

// 0x00462620
Fog::Fog(int flags)
    : GameObject(flags)
{
    field_0x44 = 0;
    drawnByFogOn = 0;
    renderFog = 1;
}

// 0x00462680
Fog* Fog::UnknownFunction462680(void* target, unsigned int color, float visibility, float haziness,
                                float farScale, float nearScale, float minimum)
{
    char name[256];

    GameObject::UnknownVirtualSlot8(target);
    this->farScale = farScale;
    this->nearScale = nearScale;
    visibilityOffset = 0;
    minimumDistance = minimum;
    if (g_TrackGame->field_0x2d0) {
        renderFog = 0;
    } else {
        sprintf(name, "DriverInfo\\%s\\RenderFog", TARGET()->field_0x04->field_0x4bc);
        renderFog = g_TrackGame->GetRegistryFlag(name, 1);
        if (TARGET()->triRasterCaps & D3DPRASTERCAPS_FOGVERTEX) {
            field_0x44 = D3DPRASTERCAPS_FOGVERTEX;
        } else if (TARGET()->field_0x04->field_0xb74_bit2 || (TARGET()->triRasterCaps & D3DPRASTERCAPS_FOGTABLE)) {
            field_0x44 = D3DPRASTERCAPS_FOGTABLE;
        } else if (TARGET()->triRasterCaps & D3DPRASTERCAPS_FOGRANGE) {
            field_0x44 = D3DPRASTERCAPS_FOGRANGE;
        }
        // Not Windows NT (VER_PLATFORM_WIN32_NT is 2).
        if (g_TrackGame->field_0x424.platformId != 2 && !(TARGET()->field_0x04->field_0x1b8 & 0x400)
            && (TARGET()->triRasterCaps & D3DPRASTERCAPS_FOGTABLE)) {
            field_0x44 = D3DPRASTERCAPS_FOGTABLE;
        }
    }
    UnknownFunction4627a0(color, visibility, haziness);
    return this;
}

// 0x004627a0
void Fog::UnknownFunction4627a0(unsigned int color, float visibility, float haziness)
{
    field_0x2c = color;
    field_0x38 = visibility;
    field_0x40 = haziness;
    fogEnd = (visibility - visibilityOffset) * (farScale - nearScale) + nearScale;
    fogStart = (fogEnd - minimumDistance) * (1.0f - haziness);
    TARGET()->field_0x30 = color; // the clear colour
}

// 0x004627f0
int Fog::UnknownVirtualSlot10(float frameTime)
{
    return 1;
}

// 0x00462800
int Fog::UnknownVirtualSlot12()
{
    if (field_0x25_bit0) {
        TARGET()->field_0x08->UnknownFunction42e960(1.0f, fogEnd);
        TARGET()->field_0x08->UnknownVirtualSlot28();
        TARGET()->field_0x08->UnknownVirtualSlot32(&TARGET()->field_0x08->projectionMatrix);
    }
    return 1;
}

// 0x00462c20
int Fog::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    char name[256];

    if (UnknownFunction43caa0(0x57, 0, event, 3) && !g_TrackGame->field_0x2d0) {
        renderFog = 1 - renderFog;
        sprintf(name, "DriverInfo\\%s\\RenderFog", TARGET()->field_0x04->field_0x4bc);
        g_TrackGame->SetRegistryFlag(name, renderFog);
        return 1;
    }
    if (UnknownFunction43caa0(0x21, 0, event, 0x80)) {
        visibilityOffset += 0.1f;
        if (visibilityOffset > 1.0f) {
            visibilityOffset = 1.0f;
        }
        float visibility = field_0x38;
        if (visibility - visibilityOffset < 0.0f) {
            visibilityOffset = visibility = field_0x38;
        }
        UnknownFunction4627a0(field_0x2c, visibility, field_0x40);
        return 1;
    }
    if (UnknownFunction43caa0(0x22, 0, event, 0x80)) {
        visibilityOffset -= 0.1f;
        if (visibilityOffset < 0.0f) {
            visibilityOffset = 0;
        }
        float visibility = field_0x38;
        if (visibility - visibilityOffset > 1.0f) {
            visibilityOffset = 0;
        }
        UnknownFunction4627a0(field_0x2c, visibility, field_0x40);
        return 1;
    }
    return 0;
}

// 0x00462db0
void Fog::UnknownFunction462db0(int level)
{
    visibilityOffset = (9 - level) * 0.1f;
    float visibility = field_0x38;
    if (visibility - visibilityOffset < 0.0f) {
        visibilityOffset = visibility = field_0x38;
    }
    UnknownFunction4627a0(field_0x2c, visibility, field_0x40);
}

// 0x00462670
Fog::~Fog()
{
}

// 0x00462e10
FogOff::FogOff(int flags)
    : GameObject(flags)
{
}

// 0x00462e30 (the linker keeps one copy of this body for several classes)
GameObject* FogOff::UnknownVirtualSlot8(void* value)
{
    GameObject::UnknownVirtualSlot8(value);
    return this;
}

// 0x00462e50
int FogOff::UnknownVirtualSlot14()
{
    TARGET()->field_0x08->UnknownFunction42e8e0();
    TARGET()->field_0x08->UnknownVirtualSlot32(&TARGET()->field_0x08->projectionMatrix);
    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 0, 0);
    return 1;
}

// 0x00462e90
FogOn::FogOn(int flags)
    : GameObject(flags)
{
}

// 0x00462eb0
int FogOn::UnknownVirtualSlot14()
{
    fog->drawnByFogOn = 1;
    fog->UnknownVirtualSlot14();
    fog->drawnByFogOn = 0;
    return 1;
}
