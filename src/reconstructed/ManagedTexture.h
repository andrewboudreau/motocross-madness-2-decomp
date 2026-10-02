#pragma once

#include "PCTextureMap.h"

class ManagedTexture;

// RTTI: CacheTexture : PCTextureMap (vtable 0x005583d8; deleting wrapper
// 0x0050f810, destructor 0x0050f830), 0x190 bytes. A page that a
// ManagedTextureGroup packs its ManagedTextures onto; only what those
// classes use is declared.
class CacheTexture : public PCTextureMap {
public:
    // 0x0050f6a0
    CacheTexture(TextureMapManager* manager, ManagedTextureGroup* group, int value);
    // 0x0050f890: lists the textures placed on the page in `textures`.
    void UnknownFunction50f890(ContainerList<ManagedTexture*>* textures);
    void UnknownFunction50f9b0(int* levels, int count); // 0x0050f9b0
    // 0x0050fdb0: places textures from `textures` on the page.
    int UnknownFunction50fdb0(ContainerList<ManagedTexture*>* textures, int value);
    void UnknownFunction50fc40();             // 0x0050fc40 (after restoring a lost page)
    void UnknownFunction5102b0(ManagedTexture* texture); // 0x005102b0: takes `texture` off the page

    unsigned char field_0x80[0x184 - 0x80];
    int field_0x184;                          // memory
    unsigned char field_0x188[0x18c - 0x188];
    int field_0x18c;                          // texels wanted on the page less its size (repacking)
};

// The part of a CacheTexture page (ManagedTexture+0x94) a texture occupies.
struct UnknownTextureRegion {
    unsigned char field_0x00[0x18];
    int field_0x18;
    float field_0x1c;                         // left (u)
    float field_0x20;                         // top (v)
    float field_0x24;                         // right (u)
};

// RTTI: ManagedTexture : PCTextureMap (vtable 0x00558430, 0xbc bytes; it
// overrides slots 0, 7, 8, 11 and 19). Its code sits between
// TextureMapManager.cpp's literals. A ManagedTextureGroup (+0x90) owns it;
// while placed on a CacheTexture page (+0x80) it draws from the page and
// maps its texture coordinates into the page's region.
class ManagedTexture : public PCTextureMap {
public:
    explicit ManagedTexture(TextureMapManager* manager); // 0x00510500: sets TextureMap+0x68 bit 0
    virtual ~ManagedTexture();                // 0x005105a0 (deleting wrapper 0x00510580)
    virtual int UnknownVirtualSlot7();        // 0x00510980: whether it is placed (+0x94)
    virtual int UnknownVirtualSlot8(int a, int b, int c); // 0x00510750: 0
    virtual int UnknownVirtualSlot11();       // 0x005108f0: binds the page or itself
    virtual void UnknownVirtualSlot19();      // 0x00510870

    void UnknownFunction510610(int level);    // 0x00510610: takes the scale and offset of its region
    void UnknownFunction510670();             // 0x00510670: resets and re-adds itself to its group
    void UnknownFunction510700();             // 0x00510700: leaves its page
    int UnknownFunction510760(int a, int b, int c); // 0x00510760: PCTextureMap slot 8, called directly
    // 0x00510780: maps `count` (u, v) pairs, `stride` bytes apart, from the
    // given scale/offset to this texture's, and returns its scale/offset.
    void UnknownFunction510780(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                               unsigned int stride);
    void UnknownFunction510820(float value);  // 0x00510820: records a use (value at most 9)
    // 0x00510910: 0x00510780 unless the scale/offset are already this texture's.
    int UnknownFunction510910(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                              unsigned int stride);
    int UnknownFunction510990();              // 0x00510990: log2 of the width
    float UnknownFunction5109b0();            // 0x005109b0: +0xa4 less +0xac

    CacheTexture* field_0x80;                 // page it is placed on
    float field_0x84;                         // scale within the page
    float field_0x88;                         // u offset within the page
    float field_0x8c;                         // v offset within the page
    ManagedTextureGroup* field_0x90;
    UnknownTextureRegion* field_0x94;
    int field_0x98;
    int field_0x9c;
    int field_0xa0;                           // uses (0x00510820)
    float field_0xa4;                         // largest recorded value (0x00510820)
    int field_0xa8;                           // region +0x18
    int field_0xac;                           // planned level (repacking)
    int field_0xb0;                           // wanted level
    int field_0xb4;
    int field_0xb8;                           // manager frame of the last use
};
