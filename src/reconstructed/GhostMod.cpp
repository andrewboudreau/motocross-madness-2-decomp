// GhostMod.cpp -- GhostMod1, see GhostMod.h for the evidence.

#include "GhostMod.h"

#include "DebugAlloc.h"
#include "TrackGame.h"

#define GHOST_MOD_NONE 3.402823466e+38F       // FLT_MAX

// 0x0047bb20
GhostMod1::GhostMod1(int flags)
    : D3DIMSoultreeModifier(flags)
{
    timeRemaining = 0;
    totalDuration = 0;
}

// 0x0047bb70
void GhostMod1::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                     UnknownSoultreeMesh** out)
{
    *out = mesh;
    if (timeRemaining > 0.0f) {
        unsigned int alpha = (unsigned char)(int)(200.0f - timeRemaining / totalDuration * 100.0f) << 24;
        for (int i = 0; i < mesh->vertexCount; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].diffuse &= 0xffffff;
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].diffuse |= alpha;
        }
    }
}

// 0x0047bbf0
void GhostMod1::UnknownFunction47bbf0(float duration)
{
    if (duration == 0.0f) {
        timeRemaining = GHOST_MOD_NONE;
        totalDuration = GHOST_MOD_NONE;
    } else {
        timeRemaining = duration;
        totalDuration = duration;
    }
}

// 0x0047bc20
int GhostMod1::UnknownVirtualSlot10(float frameTime)
{
    if (timeRemaining != GHOST_MOD_NONE) {
        float remaining = timeRemaining - g_TrackGame->frameTime;
        timeRemaining = remaining > 0.0f ? remaining : 0.0f;
    }
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x0047bc60
GhostMod1::~GhostMod1()
{
}
