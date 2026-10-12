#pragma once

#include "CacheTexture.h"

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

    void UpdatePageCoords(int level);    // 0x00510610: takes the scale and offset of its region
    void Reregister();             // 0x00510670: resets and re-adds itself to its group
    void ReleasePage();             // 0x00510700: leaves its page
    int UnknownFunction510760(int a, int b, int c); // 0x00510760: PCTextureMap slot 8, called directly
    // 0x00510780: maps `count` (u, v) pairs, `stride` bytes apart, from the
    // given scale/offset to this texture's, and returns its scale/offset.
    void RemapTexCoords(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                               unsigned int stride);
    void RecordUse(float value);  // 0x00510820: records a use (value at most 9)
    // 0x00510910: 0x00510780 unless the scale/offset are already this texture's.
    int RemapTexCoordsIfMoved(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                              unsigned int stride);
    int SizeLevel();              // 0x00510990: log2 of the width
    float UnknownFunction5109b0();            // 0x005109b0: +0xa4 less +0xac

    CacheTexture* page;                       // +0x80 page it is placed on
    float pageScale;                          // +0x84 scale within the page
    float pageOffsetU;                        // +0x88 u offset within the page
    float pageOffsetV;                        // +0x8c v offset within the page
    ManagedTextureGroup* group;               // +0x90
    UnknownTextureRegion* region;             // +0x94
    int field_0x98;
    int field_0x9c;
    int useCount;                             // +0xa0 uses (0x00510820)
    float largestUse;                         // +0xa4 largest recorded value (0x00510820)
    int field_0xa8;                           // region +0x18
    int plannedLevel;                         // +0xac planned level (repacking)
    int wantedLevel;                          // +0xb0 wanted level
    int field_0xb4;
    int lastUseFrame;                         // +0xb8 manager frame of the last use
};
