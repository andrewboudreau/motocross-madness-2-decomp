// TerrainShadow.h -- TerrainShadow, a ShadowReceiver linked right after Terrain.cpp (TU unproven).
//
// Evidence (tier 1 unless noted):
//  * RTTI TerrainShadow vtable 0x005582d4, base ShadowReceiver (vtable 0x005515ec) -> GameObject.
//  * Ctor 0x00508ae0 (ret 4, one argument forwarded to the GameObject ctor 0x00468ca0), then vptr
//    0x005582d4; the deleting dtor 0x00446890 is shared with D3DIMSoultreeShadow by identical
//    code folding and calls ~ShadowReceiver 0x00508b70.
//  * Attach 0x00508b80 mirrors D3DIMSoultreeShadow::Attach 0x004468b0 (stores +0x2c/+0x30, calls
//    ProjectedShadow::AddReceiver 0x004daba0).
// Member names are field_0xNN or camelCase with the evidence (tier 3).
#ifndef BROADPHASE_TERRAINSHADOW_H
#define BROADPHASE_TERRAINSHADOW_H

#include "shadow/ProjectedShadow.h"
#include "broadphase/Terrain.h"

class TerrainShadow : public ShadowReceiver {
public:
    explicit TerrainShadow(int flags);                                                  // 0x00508ae0
    TerrainShadow* Attach(int host, Terrain* terrain, ProjectedShadow* shadow);     // 0x00508b80
    virtual int GameObjectVirtualSlot14();                                              // 0x0050a1a0
    virtual int UnknownVirtualSlot27();                                                 // 0x00508bc0
    virtual int UnknownVirtualSlot28();                                                 // 0x005097d0
    virtual int UnknownVirtualSlot29();                                                 // 0x005099c0
    virtual void UnknownVirtualSlot30();                                                // 0x00509aa0

    Terrain* caster;               // +0x2c ctor 0; Attach stores its second argument.  Tier 2: slot 28 calls
                                   // 0x00484d70 on caster+0x44, which is Terrain::heightField
    ProjectedShadow* shadow;       // +0x30 Attach stores its third argument and registers this
    char field_0x34[0x6054 - 0x34]; // +0x34 slot 14 draws it as FVF 0x1e2 vertices (32 bytes each)
    short indexTable[0x300];       // +0x6054 ctor fills with 0..0x2ff (`mov [ecx],ax; inc eax; cmp eax,0x300`)
    int field_0x6654;              // +0x6654 ctor 0; slot 29 clears it when its list fills
    int minX;                      // +0x6658 ctor 0x7fffffff (running minimum, tier 3 role)
    int maxX;                      // +0x665c ctor 0x80000000
    int minZ;                      // +0x6660 ctor 0x7fffffff
    int maxZ;                      // +0x6664 ctor 0x80000000
    float field_0x6668;            // +0x6668 ctor FLT_MAX
    float field_0x666c;            // +0x666c ctor -FLT_MAX
    float field_0x6670;            // +0x6670 ctor FLT_MAX
    float field_0x6674;            // +0x6674 ctor -FLT_MAX
    int cellCount;                 // +0x6678 ctor 0; slot 29 count of keys in cellKeys
    void* cellKeys[0x40];          // +0x667c slot 29 de-duplicated cell keys (0x40 entries to +0x677c)
    float field_0x677c;            // +0x677c ctor 0.0078125 (1/128)
};

#endif
