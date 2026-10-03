// SoultreePhysicsCharacter -- canonical class shape (SoulTreePhysics.cpp, tier 2 TU:
// __FILE__ references at 0x00503b13/0x00503f87).
//
// Evidence (tier 1 unless noted):
//  * RTTI: SoultreePhysicsCharacter : SoultreePhysicsBaseObject (offset 0),
//    D3DIMSoultreeCharacter (offset 540), virtual GameObject.  vtables (analysis/vtables.json):
//      0x005580a8 @0    43 slots: SoultreePhysicsBaseObject's 40 + slots 40..42
//      0x00558074 @540  12 slots: identical to D3DIMSoultreeCharacter's primary vtable
//      0x00558004 @1080 27 slots: GameObject virtual base
//  * vbase deleting dtor 0x005042f0: `lea esi,[ecx-0x438]`, calls core 0x00503d40 then
//    ??1GameObject 0x00468d60 -> GameObject at 0x438 (1080).
//  * vbase slot 10 is the vtordisp thunk 0x00504350 (`sub ecx,[ecx-4]`) -> 0x00504210
//    (ret 4): this class overrides GameObject slot 10 and has a vtordisp at 0x434.
//    Vbase slots 4/5 are static adjustors `sub ecx,8` -> D3DIM's 0x00446640/0x00446620
//    (0x438 - (0x21c + 0x214) = 8), i.e. inherited, not overridden.
//  * Primary-vtable diff vs SoultreePhysicsBaseObject: slots 1, 8, 33 overridden,
//    40..42 introduced (0x00503de0, 0x00504360, 0x00504470).
//  * Layout: SoultreePhysicsBaseObject non-virtual part 0x000..0x21c,
//    D3DIMSoultreeCharacter 0x21c..0x42c, own fields 0x42c..0x434, vtordisp 0x434,
//    GameObject 0x438..0x464.  sizeof == 0x464.  Proven by hierarchy/LayoutProbe.cpp.
//  * Own fields: 0x42c is a node pointer (slot 8 calls 0x004fd7f0 on it); 0x430..0x433
//    are four byte flags cleared by slot 1 (0x005040c0).  The pointer the collision area
//    called field_0x3bc is D3DIMSoultreeCharacter::d3d_field_0x1a0.
#ifndef SOULTREE_PHYSICS_CHARACTER_CANONICAL_H
#define SOULTREE_PHYSICS_CHARACTER_CANONICAL_H

#include "../soultree_base/SoultreePhysicsBaseObject.h"
#include "motion/D3DIMSoultreeCharacter.h"

class SoultreePhysicsCharacter : public SoultreePhysicsBaseObject, public D3DIMSoultreeCharacter {
public:
    // 0x00503c70 (thiscall, ret 8 = flags + the compiler's hidden most-derived flag; tier 1):
    // GameObject(1) when most-derived, then SoultreePhysicsBaseObject(flags) (0x00500aa0) and
    // D3DIMSoultreeCharacter(flags) (0x004455b0), vptrs/vtordisp, poseNode = 0.
    // Vehicle's ctor 0x005257a0 calls it as (arg, 0), i.e. SoultreePhysicsCharacter(arg).
    explicit SoultreePhysicsCharacter(int flags);
    virtual ~SoultreePhysicsCharacter();        // core 0x00503d40, deleting 0x005042f0
    virtual int GameObjectVirtualSlot10(float dt);  // 0x00504210 via vtordisp thunk 0x00504350 (ret 4)
    // --- vtable 0x005580a8 (offset 0) ---
    virtual void UnknownVirtualSlot1(float value);                // 0x005040c0
    virtual void UnknownVirtualSlot8();                           // 0x005041c0
    virtual int UnknownVirtualSlot33(const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, const Vec3* a4, int a5,
                                     float a6);                  // 0x005040f0
    // Slot 40 (0x00503de0, `ret 0x6c` = 27 argument dwords, tier 1).  Grouping (tier 2):
    //  * a6..a8 (dwords 5..13, 0-based) are three Vec3 by value.  Both the body and the one direct caller
    //    (Vehicle.cpp 0x005261e3, `call 0x00503de0`) copy them with the `sub esp,0xc;
    //    mov [esp],..` struct-copy shape; the body forwards them unchanged to slot 2.
    //  * a3 is a NUL-terminated name (strlen/strncpy, then ".col" is appended).
    //  * a4 is a descriptor pointer (`test byte [a4+0x25],1`).
    //  * a1, a3, a4, a5 go to D3DIMVirtualSlot11 as (a1, a3, a4, a5, 1, 1).
    //  * Slot 2 (0x00500c50) stores the rest:
    //      a9 -> 0x1f4 (pointer; +0x40 read into field_0x1f8), a10 -> 0x124, a11 -> 0x150,
    //      a12 -> 0x1c8 (element count of the 0x12c array), a14 -> 0x1f0 (pointer),
    //      a15 -> 0x1e4, a16 -> 0x1ec, a17 -> 0x148, a18 -> 0x14c, a19 -> 0x210,
    //      a20 -> byte 0x20f.
    //  * Float types (a15, a17, a18) follow the Vehicle caller's float constants
    //    (0.02f, 0.001f, 0.1f); a20 is pushed zero-extended from a byte (tier 2).
    //  * a10, a11, a14, a20 take the types of the slot 2 parameters they are forwarded to.
    //  * Returns the GameObject virtual base (`this ? this+vbase : 0` at 0x0050408e).  The
    //    sibling SoultreePhysicsObject slot 40 (0x00503970, ret 0x70) has the same shape; its
    //    SceneManager caller 0x004ed3bf passes the result to GameObject::Method_0x00469190.
    virtual GameObject* UnknownVirtualSlot40(int a1, int a2, const char* a3,
                                             const SoultreeLoadDesc* a4, int a5,
                                             Vec3 a6, Vec3 a7, Vec3 a8,
                                             void* a9, void* a10, float a11, int a12, int a13,
                                             SoultreeSlot1f0* a14, float a15, int a16, float a17,
                                             float a18, int a19, unsigned char a20, int a21);
    virtual void UnknownVirtualSlot41();                          // 0x00504360
    virtual int UnknownVirtualSlot42();                           // 0x00504470

    SoultreeObject* poseNode;  // +0x42c slot 40 sets it to the model node (same as sceneNode); Bike reads the rider/pose axes from it
    char field_0x430;
    char field_0x431;
    char field_0x432;
    char field_0x433;
};

#endif
