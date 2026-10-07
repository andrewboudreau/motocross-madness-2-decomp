#include "ManagedTexture.h"

#include "PCRenderTarget.h"
#include "TrackGame.h"

// 0x00510500
ManagedTexture::ManagedTexture(TextureMapManager* manager) : PCTextureMap(manager, 0) {
    field_0x80 = 0;
    field_0xac = -1;
    field_0xa8 = -1;
    field_0x94 = 0;
    field_0x8c = 0;
    field_0x88 = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xb8 = 0;
    field_0x90 = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0x68 |= 1;
    field_0x84 = 1.0f;
}

// 0x005105a0
ManagedTexture::~ManagedTexture() {
    if (field_0x90)
        field_0x90->field_0x44.Remove(this);
    if (field_0x80)
        UnknownFunction510700();
}

// 0x00510610
void ManagedTexture::UnknownFunction510610(int level) {
    UnknownTextureRegion* region = field_0x94;
    if (region && field_0x80) {
        field_0x84 = (region->field_0x24 - region->field_0x1c) / (1 << level);
        field_0x88 = region->field_0x1c;
        field_0x8c = region->field_0x20;
        field_0xa8 = region->field_0x18;
    }
}

// 0x00510670
void ManagedTexture::UnknownFunction510670() {
    if (field_0x90)
        field_0x90->field_0x44.Remove(this);
    if (field_0x80)
        UnknownFunction510700();
    field_0x80 = 0;
    field_0x94 = 0;
    field_0x8c = 0;
    field_0x88 = 0;
    field_0x84 = 1.0f;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xb8 = 0;
    field_0xac = -1;
    field_0xa8 = -1;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0x90->UnknownFunction50c6c0(this);
}

// 0x00510700
void ManagedTexture::UnknownFunction510700() {
    if (field_0x80) {
        field_0x80->UnknownFunction5102b0(this);
        field_0x80 = 0;
        field_0x94 = 0;
        field_0x8c = 0;
        field_0x88 = 0;
        field_0x84 = 0;
        field_0xa8 = -1;
    }
}

// 0x00510750
int ManagedTexture::UnknownVirtualSlot8(int a, int b, int c) {
    return 0;
}

// 0x00510760
int ManagedTexture::UnknownFunction510760(int a, int b, int c) {
    return PCTextureMap::UnknownVirtualSlot8(a, b, c);
}

// 0x00510780
void ManagedTexture::UnknownFunction510780(float* scale, float* offsetU, float* offsetV, float* u, float* v,
                                           int count, unsigned int stride) {
    unsigned int step = stride / sizeof(float);
    float ratio = field_0x84 / *scale;
    float du = field_0x88 - ratio * *offsetU;
    float dv = field_0x8c - ratio * *offsetV;
    while (count--) {
        *u = ratio * *u + du;
        *v = ratio * *v + dv;
        u += step;
        v += step;
    }
    *scale = field_0x84;
    *offsetU = field_0x88;
    *offsetV = field_0x8c;
}

// 0x00510820
void ManagedTexture::UnknownFunction510820(float value) {
    if (value > 9.0f)
        value = 9.0f;
    if (value > field_0xa4)
        field_0xa4 = value;
    field_0xa0++;
    field_0xb8 = field_0x10->field_0x74;
}

// 0x00510870: with a page, applies the render states, the colour key and
// the page; otherwise the PCTextureMap behaviour.
void ManagedTexture::UnknownVirtualSlot19() {
    if (field_0x80) {
        for (int i = 0; i < field_0x44; i++)
            g_TrackGame->renderTarget->UnknownVirtualSlot8(field_0x48[i].state, field_0x48[i].value, 0);
        if (field_0x30 && field_0x80->textureSurface && field_0x80->textureSurface->SetColorKey(8, &field_0x34))
            return;
        field_0x80->UnknownVirtualSlot11();
    } else if (textureSurface) {
        PCTextureMap::UnknownVirtualSlot19();
    }
}

// 0x005108f0
int ManagedTexture::UnknownVirtualSlot11() {
    if (field_0x80)
        return field_0x80->UnknownVirtualSlot11();
    if (textureSurface)
        return PCTextureMap::UnknownVirtualSlot11();
    return 0;
}

// 0x00510910
int ManagedTexture::UnknownFunction510910(float* scale, float* offsetU, float* offsetV, float* u, float* v,
                                          int count, unsigned int stride) {
    if (*scale == field_0x84 && *offsetU == field_0x88 && *offsetV == field_0x8c)
        return 0;
    UnknownFunction510780(scale, offsetU, offsetV, u, v, count, stride);
    return 1;
}

// 0x00510980
int ManagedTexture::UnknownVirtualSlot7() {
    return field_0x94 != 0;
}

// 0x00510990
int ManagedTexture::UnknownFunction510990() {
    int size = field_0x14;
    int level = 0;
    if (size == 1)
        return 0;
    size >>= 1;
    while (size) {
        size >>= 1;
        level++;
    }
    return level;
}

// 0x005109b0
float ManagedTexture::UnknownFunction5109b0() {
    return field_0xa4 - field_0xac;
}
