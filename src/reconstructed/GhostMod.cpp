// GhostMod.cpp -- GhostMod1, see GhostMod.h for the evidence.

#include "GhostMod.h"

#include "DebugAlloc.h"
#include "TrackGame.h"

#define GHOST_MOD_NONE 3.402823466e+38F       // FLT_MAX

// 0x0047bb20
GhostMod1::GhostMod1(int flags)
    : D3DIMSoultreeModifier(flags)
{
    field_0x40 = 0;
    field_0x44 = 0;
}

// 0x0047bb70
void GhostMod1::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                     UnknownSoultreeMesh** out)
{
    *out = mesh;
    if (field_0x40 > 0.0f) {
        unsigned int alpha = (unsigned char)(int)(200.0f - field_0x40 / field_0x44 * 100.0f) << 24;
        for (int i = 0; i < mesh->field_0x08; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].field_0x10 &= 0xffffff;
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].field_0x10 |= alpha;
        }
    }
}

// 0x0047bbf0
void GhostMod1::UnknownFunction47bbf0(float duration)
{
    if (duration == 0.0f) {
        field_0x40 = GHOST_MOD_NONE;
        field_0x44 = GHOST_MOD_NONE;
    } else {
        field_0x40 = duration;
        field_0x44 = duration;
    }
}

// 0x0047bc20
int GhostMod1::UnknownVirtualSlot10(float frameTime)
{
    if (field_0x40 != GHOST_MOD_NONE) {
        float remaining = field_0x40 - g_UnknownGlobal56e26c->field_0x2f0;
        field_0x40 = remaining > 0.0f ? remaining : 0.0f;
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x0047bc60
GhostMod1::~GhostMod1()
{
}
