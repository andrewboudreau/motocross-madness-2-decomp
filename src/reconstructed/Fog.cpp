// Fog.cpp -- Fog, FogOff and FogOn; see Fog.h for the evidence.

#include <stdio.h>

#include "Fog.h"

#include "Camera.h"
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
    field_0x4c = 0;
    field_0x48 = 1;
}

// 0x00462680
Fog* Fog::UnknownFunction462680(void* target, unsigned int color, float visibility, float haziness,
                                float farScale, float nearScale, float minimum)
{
    char name[256];

    GameObject::UnknownVirtualSlot8(target);
    field_0x50 = farScale;
    field_0x54 = nearScale;
    field_0x3c = 0;
    field_0x58 = minimum;
    if (g_UnknownGlobal56e26c->field_0x2d0) {
        field_0x48 = 0;
    } else {
        sprintf(name, "DriverInfo\\%s\\RenderFog", TARGET()->field_0x04->field_0x4bc);
        field_0x48 = g_UnknownGlobal56e26c->UnknownVirtualSlot22(name, 1);
        if (TARGET()->field_0x1a8 & 0x80) {
            field_0x44 = 0x80;
        } else if (TARGET()->field_0x04->field_0xb74_bit2 || (TARGET()->field_0x1a8 & 0x100)) {
            field_0x44 = 0x100;
        } else if (TARGET()->field_0x1a8 & 0x10000) {
            field_0x44 = 0x10000;
        }
        if (g_UnknownGlobal56e26c->field_0x424.platformId != 2 && !(TARGET()->field_0x04->field_0x1b8 & 0x400)
            && (TARGET()->field_0x1a8 & 0x100)) {
            field_0x44 = 0x100;
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
    field_0x34 = (visibility - field_0x3c) * (field_0x50 - field_0x54) + field_0x54;
    field_0x30 = (field_0x34 - field_0x58) * (1.0f - haziness);
    TARGET()->field_0x30 = color;
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
        TARGET()->field_0x08->UnknownFunction42e960(1.0f, field_0x34);
        TARGET()->field_0x08->UnknownVirtualSlot28();
        TARGET()->field_0x08->UnknownVirtualSlot32(&TARGET()->field_0x08->projectionMatrix);
    }
    return 1;
}

// 0x00462c20
int Fog::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    char name[256];

    if (UnknownFunction43caa0(0x57, 0, event, 3) && !g_UnknownGlobal56e26c->field_0x2d0) {
        field_0x48 = 1 - field_0x48;
        sprintf(name, "DriverInfo\\%s\\RenderFog", TARGET()->field_0x04->field_0x4bc);
        g_UnknownGlobal56e26c->UnknownVirtualSlot27(name, field_0x48);
        return 1;
    }
    if (UnknownFunction43caa0(0x21, 0, event, 0x80)) {
        field_0x3c += 0.1f;
        if (field_0x3c > 1.0f) {
            field_0x3c = 1.0f;
        }
        float visibility = field_0x38;
        if (visibility - field_0x3c < 0.0f) {
            field_0x3c = visibility = field_0x38;
        }
        UnknownFunction4627a0(field_0x2c, visibility, field_0x40);
        return 1;
    }
    if (UnknownFunction43caa0(0x22, 0, event, 0x80)) {
        field_0x3c -= 0.1f;
        if (field_0x3c < 0.0f) {
            field_0x3c = 0;
        }
        float visibility = field_0x38;
        if (visibility - field_0x3c > 1.0f) {
            field_0x3c = 0;
        }
        UnknownFunction4627a0(field_0x2c, visibility, field_0x40);
        return 1;
    }
    return 0;
}

// 0x00462db0
void Fog::UnknownFunction462db0(int level)
{
    field_0x3c = (9 - level) * 0.1f;
    float visibility = field_0x38;
    if (visibility - field_0x3c < 0.0f) {
        field_0x3c = visibility = field_0x38;
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
    TARGET()->UnknownVirtualSlot8(0x1c, 0, 0);
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
    field_0x2c->field_0x4c = 1;
    field_0x2c->UnknownVirtualSlot14();
    field_0x2c->field_0x4c = 0;
    return 1;
}
