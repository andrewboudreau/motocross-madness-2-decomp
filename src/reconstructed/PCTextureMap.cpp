#include "PCTextureMap.h"

#include <string.h>

#include "DebugAlloc.h"
#include "PCRenderTarget.h"
#include "TrackGame.h"

// 0x004c5f00
PCTextureMap::PCTextureMap(TextureMapManager* manager, int value) : TextureMap(manager, value) {
    field_0x70 = 0;
    field_0x74 = 0;
    field_0x78 = 0;
    field_0x7c = 0;
}

// 0x004c5f50: frees the decoder, takes the system-memory surface's size
// (bytes per pixel x height x width, 4/3 more with mip levels) off the
// DirectX memory count and releases it, then releases the texture surface.
PCTextureMap::~PCTextureMap() {
    if (field_0x7c) {
        delete field_0x7c;
        field_0x7c = 0;
    }
    if (field_0x70) {
        float scale = 1.0f;
        if (field_0x24 != 1)
            scale = 4.0f / 3.0f;
        g_MemTagStack->UnknownFunction4a2e00(
            (int)(UnknownFunction511970(field_0x20) * field_0x18 * field_0x14 * scale));
        field_0x70->UnknownMethod2();
        field_0x70 = 0;
    }
    if (field_0x74) {
        field_0x74->UnknownMethod2();
        field_0x74 = 0;
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// 0x004c6040
void PCTextureMap::UnknownVirtualSlot12() {
    if (field_0x74 && field_0x74->UnknownMethod24()) {
        UnknownVirtualSlot10();
        UnknownVirtualSlot8(1, 0, 0);
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// 0x004c7470
int PCTextureMap::UnknownVirtualSlot7() {
    return field_0x74 != 0;
}

// 0x004c7610
void PCTextureMap::UnknownVirtualSlot10() {
    if (field_0x74) {
        field_0x74->UnknownMethod2();
        field_0x74 = 0;
        if (g_UnknownGlobal689964)
            field_0x10->UnknownFunction511580();
    }
}

// 0x004c7970
int PCTextureMap::UnknownVirtualSlot11() {
    if (field_0x74 &&
        !((PCRenderTarget*)g_UnknownGlobal56e26c->field_0x10)->field_0x50->UnknownMethod35(0, field_0x74))
        return 1;
    return 0;
}

// 0x004c79a0
void PCTextureMap::UnknownVirtualSlot19() {
    UnknownVirtualSlot11();
    for (int i = 0; i < field_0x44; i++)
        g_UnknownGlobal56e26c->field_0x10->UnknownVirtualSlot8(field_0x48[i].state, field_0x48[i].value, 0);
}

// 0x004c79e0
void* PCTextureMap::UnknownVirtualSlot13(void* rect, long* pitch, int flags) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    if (field_0x70->UnknownMethod25(rect, &desc, flags, 0))
        return 0;
    if (pitch)
        *pitch = desc.pitch;
    return desc.surface;
}

// 0x004c7a50
int PCTextureMap::UnknownVirtualSlot14(void* rect) {
    return !field_0x70->UnknownMethod32(rect);
}

// 0x004c7a70
void* PCTextureMap::UnknownVirtualSlot16(int level) {
    UnknownSurfaceInterface* surface = UnknownFunction4c83a0(level);
    if (surface) {
        UnknownSurfaceDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        if (!surface->UnknownMethod25(0, &desc, 0x801, 0))
            return desc.surface;
    }
    return 0;
}

// 0x004c7ad0
int PCTextureMap::UnknownVirtualSlot17(int level) {
    UnknownSurfaceInterface* surface = UnknownFunction4c83a0(level);
    if (surface && !surface->UnknownMethod32(0))
        return 1;
    return 0;
}

// 0x004c7420
int PCTextureMap::UnknownFunction4c7420() {
    if (field_0x74 && field_0x74->UnknownMethod24()) {
        field_0x74->UnknownMethod2();
        if (field_0x70 == field_0x74)
            field_0x70 = 0;
        field_0x74 = 0;
        return UnknownVirtualSlot8(1, 0, 0);
    }
    return 1;
}

// 0x004c7b00
int PCTextureMap::UnknownFunction4c7b00(void* destinationRect, UnknownSurfaceInterface* destination,
                                        void* sourceRect, int flags, int skip) {
    if (!skip)
        return !destination->UnknownMethod5(destinationRect, field_0x70, sourceRect, flags, 0);
    return 0;
}

// 0x004c84e0
int PCTextureMap::UnknownFunction4c84e0(UnknownSurfaceInterface* surface, int value) {
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    surface->UnknownMethod25(0, &desc, 0x811, 0);
    UnknownFunction4c8550(&desc, value);
    surface->UnknownMethod32(0);
    return 1;
}
