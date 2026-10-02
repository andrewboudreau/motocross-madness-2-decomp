// Character and D3DIMSoultreeCharacter -- canonical class shapes for the Soultree
// physics hierarchy (secondary base of SoultreePhysicsCharacter at object offset 540).
//
// Evidence (tier 1 unless noted):
//  * RTTI: Character : virtual GameObject; D3DIMSoultreeCharacter : Character.
//    COL/vtable records (analysis/vtables.json):
//      Character              0x00555318 @0 (11 slots), 0x005552a8 @416 (27, GameObject vbase)
//      D3DIMSoultreeCharacter 0x00551530 @0 (12 slots), 0x005514c0 @532 (27, GameObject vbase)
//  * Character vbase deleting dtor 0x004a68d0 uses the static `lea esi,[ecx-0x1a0]` and
//    is referenced directly from vbase slot 0 (no vtordisp thunk): Character has NO
//    vtordisp, its non-virtual size is 0x1a0 and GameObject sits at 0x1a0 (416).
//    Character overrides only the GameObject destructor.
//  * D3DIMSoultreeCharacter vbase slot 0 is the vtordisp thunk 0x00446670
//    (`sub ecx,[ecx-4]`) -> 0x00446680 (`lea esi,[ecx-0x214]`), and slots 4/5 are vtordisp
//    thunks 0x004466c0 -> 0x00446640 and 0x004466d0 -> 0x00446620 (both plain `ret`).
//    So D3DIM overrides GameObject slots 0, 4 and 5, carries a vtordisp at 0x210 and
//    its non-virtual size is 0x210 (528); GameObject sits at 0x214 (532).
//  * Primary-vtable diff (Character -> D3DIM): D3DIM overrides slots 0, 2..6, 8, 9, 10
//    and introduces slot 11; slots 1 (0x004a6930) and 7 (0x004a70c0) are inherited.
//  * Argument byte counts below come from each implementation's `ret N` (tier 2);
//    types are provisional (int placeholders).  Slots are named CharacterVirtualSlotN /
//    D3DIMVirtualSlotN (not UnknownVirtualSlotN) so they cannot collide with
//    SoultreePhysicsBaseObject::UnknownVirtualSlotN in SoultreePhysicsCharacter, where an
//    equal parameter list would silently override both bases and any unqualified call
//    would be an ambiguous lookup.
//
// Field naming: offsets are relative to the start of each class, but the member
// names carry a chr_/d3d_ prefix because SoultreePhysicsCharacter inherits from both
// SoultreePhysicsBaseObject and this class, and identical field_0xNN names (e.g.
// field_0x1a0, which both classes have) would make unqualified lookups ambiguous.
#ifndef SOULTREE_D3DIM_SOULTREE_CHARACTER_H
#define SOULTREE_D3DIM_SOULTREE_CHARACTER_H

#include "../soultree_base/SoultreePhysicsBaseObject.h"

class SoultreeObject;

class Character : public virtual GameObject {
public:
    Character();
    virtual ~Character();                       // core 0x004a69d0, deleting 0x004a68d0
    // --- vtable 0x00555318 (offset 0), slots 0..10, all introduced here ---
    virtual void CharacterVirtualSlot0();         // 0x00464e90 (ret; shared empty stub)
    virtual void CharacterVirtualSlot1();         // 0x004a6930
    virtual void CharacterVirtualSlot2();         // 0x00464e90 (ret)
    virtual void CharacterVirtualSlot3(int a);    // 0x00464e80 (ret 4; shared empty stub)
    virtual void CharacterVirtualSlot4(int a, int b);   // 0x004a6ba0 (ret 8; shared by 4..6)
    virtual void CharacterVirtualSlot5(int a, int b);   // 0x004a6ba0
    virtual void CharacterVirtualSlot6(int a, int b);   // 0x004a6ba0
    virtual void CharacterVirtualSlot7(int a, int b, int c);  // 0x004a70c0 (ret 0xc)
    virtual void CharacterVirtualSlot8(int a);    // 0x004a98b0 (ret 4)
    virtual void CharacterVirtualSlot9(int a);    // 0x00464e80 (ret 4)
    virtual void CharacterVirtualSlot10();        // 0x00464e90 (ret)

    // Non-virtual helpers.  Owner class is tier 3 (address proximity to Character's
    // code at 0x004a6930..0x004a98b0; the callers pass the 0x21c subobject pointer,
    // which is both the Character and the D3DIMSoultreeCharacter start).
    void Method_0x004a8b00();                               // ret
    void Method_0x004a8bf0(int a, float b);                 // ret 8
    void Method_0x004a8c50(int a, int b, float c, float d); // ret 0x10

    // vfptr +0, vbptr +4 (compiler generated); Character's own data up to 0x1a0.
    char chr_field_0x08[0x198];
};

// Descriptor passed to the D3DIM loaders (D3DIMSoultreeCharacter slot 11, D3DIMSoultreeObject
// slot 9) and tested by both slot 40 loaders: `test byte [p+0x25],1` selects a
// field_0x08->Fn_4444c0(1) call (tier 1 offset; the type and its meaning are tier 3).
struct SoultreeLoadDesc {
    char field_0x00[0x25];
    unsigned char field_0x25;
};

class D3DIMSoultreeCharacter : public Character {
public:
    // ctor 0x004455b0 takes one argument plus the hidden most-derived flag (callers push
    // `a, 1` or `a, 0` before the call, e.g. 0x00418b52, 0x00503cec).  Type is tier 3.
    D3DIMSoultreeCharacter(int a);
    virtual ~D3DIMSoultreeCharacter();          // core 0x004459a0, deleting 0x00446680
    virtual void GameObjectVirtualSlot4();      // 0x00446640 via vtordisp thunk 0x004466c0
    virtual void GameObjectVirtualSlot5();      // 0x00446620 via vtordisp thunk 0x004466d0
    // --- vtable 0x00551530 (offset 0): overrides of Character slots ---
    virtual void CharacterVirtualSlot0();         // 0x00445a70
    virtual void CharacterVirtualSlot2();         // 0x00445fc0
    virtual void CharacterVirtualSlot3(int a);    // 0x00446110
    virtual void CharacterVirtualSlot4(int a, int b);   // 0x00446210
    virtual void CharacterVirtualSlot5(int a, int b);   // 0x00446300
    virtual void CharacterVirtualSlot6(int a, int b);   // 0x00446480
    virtual void CharacterVirtualSlot8(int a);    // 0x00445e70
    virtual void CharacterVirtualSlot9(int a);    // 0x00445ec0
    virtual void CharacterVirtualSlot10();        // 0x00446520
    // --- slot 11, introduced here ---
    // a2/a3 types follow SoultreePhysicsCharacter slot 40, which passes its name string and
    // descriptor here (tier 2).
    virtual void D3DIMVirtualSlot11(int a1, const char* a2, const SoultreeLoadDesc* a3, int a4,
                                    int a5, int a6); // 0x00445680 (ret 0x18)

    // D3DIM's own data 0x1a0..0x210 (then vtordisp at 0x210, GameObject at 0x214).
    // modelNode is the pointer that SoultreePhysicsCharacter code reads at the
    // absolute offset 0x3bc (0x21c + 0x1a0); its type is shared with
    // SoultreePhysicsBaseObject::field_0x08 in the collision area (tier 3).
    SoultreeObject* modelNode;
    char d3d_field_0x1a4[0x6c];
};

#endif
