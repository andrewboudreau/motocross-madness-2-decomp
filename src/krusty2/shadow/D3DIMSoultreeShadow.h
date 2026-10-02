// D3DIMSoultreeShadow.h -- D3DIMSoultreeShadow (D:\aardvark\VC\krusty2\D3DIMSoultreeShadow.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI .?AVD3DIMSoultreeShadow@@ vtable 0x0055156c (31 slots, object_offset 0); bases
//    ShadowReceiver (vtable 0x005515ec) -> GameObject.
//  * Ctor 0x00446840 (ret 4, one argument forwarded to the GameObject ctor 0x00468ca0), then
//    vptr 0x0055156c and seven zero dwords at +0x2c..+0x44 (tier 2 extent).
//  * Overrides (vtable_overrides.json): slot 0 0x00446890 (deleting dtor; the class has no
//    user dtor, so it calls ~ShadowReceiver 0x00508b70), 14 0x00447540, 27 0x004468f0,
//    28 0x00446bf0, 29 0x00446c30, 30 0x00446f40.  Slot 8 is ShadowReceiver's 0x00447780.
//  * ProjectedShadow::AddReceiver (0x004daba0) is called from Attach 0x004468b0 with this.
// Member names are tier 3 with the evidence on each declaration.
#ifndef SHADOW_D3DIMSOULTREESHADOW_H
#define SHADOW_D3DIMSOULTREESHADOW_H

#include "ProjectedShadow.h"

class D3DIMSoultreeShadow : public ShadowReceiver {
public:
    explicit D3DIMSoultreeShadow(int flags);                       // 0x00446840
    D3DIMSoultreeShadow* Attach(int host, ShadowCaster* caster, ProjectedShadow* shadow);  // 0x004468b0
    virtual int GameObjectVirtualSlot14();                         // 0x00447540 (render)
    virtual int UnknownVirtualSlot27();                            // 0x004468f0
    virtual int UnknownVirtualSlot28();                            // 0x00446bf0
    virtual int UnknownVirtualSlot29();                            // 0x00446c30
    virtual void UnknownVirtualSlot30();                           // 0x00446f40 (src/krusty2/motion)

    ShadowCaster* caster;          // +0x2c ctor 0; Attach stores its second argument (slot 27 reads +0x14c / calls 0x004fe850 on it)
    ProjectedShadow* shadow;       // +0x30 ctor 0; Attach stores its third argument and registers this with it
    int field_0x34;                // +0x34 ctor 0; slot 27 result flag (also its return value)
    int field_0x38;                // +0x38 ctor 0; slot 27 clears it, slot 14 tests it
    int field_0x3c;                // +0x3c ctor 0
    int field_0x40;                // +0x40 ctor 0
    int field_0x44;                // +0x44 ctor 0; slot 14 tests it
};

#endif
