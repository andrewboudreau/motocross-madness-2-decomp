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

#define TARGET() ((PCRenderTarget*)field_0x18)
#define VIEWPORT(i) ((unsigned int)TARGET()->field_0x08->field_0x1a0[i])

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

// A D3DFVF_TLVERTEX (screen-space backdrop corner).
struct UnknownFogBackdropVertex {
    float x;
    float y;
    float z;
    float rhw;
    unsigned int color;
    unsigned int specular;
    float tu;
    float tv;
};

// 0x00462850: projects with the fog's far plane, draws the backdrop quad in
// the fog colour when FogOn has not, then sets the device fog states. Both
// failures leave through one `return 0` (retail shares a single block that
// reloads eax, which separate `return 0` statements do not give), and each
// corner stores rhw after its colours, as retail schedules it.
int Fog::UnknownVirtualSlot14()
{
    if (field_0x25_bit0) {
        TARGET()->field_0x08->UnknownFunction42e960(1.0f, fogEnd);
        TARGET()->field_0x08->UnknownVirtualSlot28();
        if (!TARGET()->field_0x08->UnknownVirtualSlot32(&TARGET()->field_0x08->projectionMatrix)) {
            goto failed;
        }
        if (!drawnByFogOn && TARGET()->field_0x04->field_0xb74_bit2
            && (TARGET()->field_0x34 || TARGET()->fillMode == D3DFILL_WIREFRAME)) {
            UnknownFogBackdropVertex vertices[4];
            vertices[0].x = (float)VIEWPORT(0);
            vertices[0].y = (float)VIEWPORT(1);
            vertices[0].z = 0.999999f;
            vertices[0].color = field_0x2c;
            vertices[0].specular = 0;
            vertices[0].rhw = 1.0f;
            vertices[1].x = (float)VIEWPORT(2) + (float)VIEWPORT(0);
            vertices[1].y = (float)VIEWPORT(1);
            vertices[1].z = 0.999999f;
            vertices[1].color = field_0x2c;
            vertices[1].specular = 0;
            vertices[1].rhw = 1.0f;
            vertices[2].x = (float)VIEWPORT(2) + (float)VIEWPORT(0);
            vertices[2].y = (float)VIEWPORT(3) + (float)VIEWPORT(1);
            vertices[2].z = 0.999999f;
            vertices[2].color = field_0x2c;
            vertices[2].specular = 0;
            vertices[2].rhw = 1.0f;
            vertices[3].x = (float)VIEWPORT(0);
            vertices[3].y = (float)VIEWPORT(3) + (float)VIEWPORT(1);
            vertices[3].z = 0.999999f;
            vertices[3].color = field_0x2c;
            vertices[3].specular = 0;
            vertices[3].rhw = 1.0f;
            TARGET()->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
            TARGET()->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            if (TARGET()->fillMode != D3DFILL_SOLID) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID, 0);
            }
            if (!TARGET()->UnknownVirtualSlot16(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, (int)vertices, 4, 0)) {
                goto failed;
            }
            if (TARGET()->fillMode != D3DFILL_SOLID) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, TARGET()->fillMode, 0);
            }
        }
        if (renderFog) {
            float density = 1.0f;
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 1, 0);
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGCOLOR, field_0x2c, 0);
            float start = fogStart;
            float end = fogEnd;
            if (field_0x44 == D3DPRASTERCAPS_FOGTABLE) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGSTART, *(int*)&start, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGEND, *(int*)&end, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                density = 0.22f;
                if (TARGET()->field_0x04->field_0xb74_bit2) {
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_EXP, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                } else {
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR, 0);
                }
                return 1;
            }
            if (field_0x44 == D3DPRASTERCAPS_FOGVERTEX) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGSTART, *(int*)&fogStart, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGEND, *(int*)&fogEnd, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR, 0);
                return 1;
            }
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE, 0);
        }
    }
    return 1;
failed:
    return 0;
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
