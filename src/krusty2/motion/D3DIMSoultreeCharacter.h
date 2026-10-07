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

#include "core/GameObject.h"
#include "math/Math3D.h"

class SoultreeObject;
class SltFile;          // .slt section/key reader (0x5c4 bytes); declared by its users
struct Motion;          // 0xcc-byte motion record of the MotionManager (src/krusty2/motion/Motnctrl.cpp)
struct CharacterPose;   // 0x2c-byte pose record; defined by users that need the layout (src/krusty2/motion/MotionPose.h)

// {int, records, count} list of pose records: Character's rest pose list (+0x190) and each frame of a
// Motion (0xc-byte elements of Motion::frames).  field_0x00 is the frame number in motion frames
// (the advance helpers 0x004a56d0/0x004a5750 read it).  Names tier 3.
struct MotionPoseList {
    int field_0x00;              // +0x00 frame number
    CharacterPose* poses;        // +0x04 array of 0x2c-byte records
    int count;                   // +0x08 element count
};

// One 0x40-byte row of Character's per-node name table (written by FillNodeNames 0x00445460; names tier 3).
struct NodeNameEntry {
    char name[0x34];          // +0x00 upper-cased copy of the node name, at most 0x31 chars
    SoultreeObject* node;     // +0x34 the scene node this row describes
    int field_0x38;           // +0x38 cleared when the row is written; D3DIM slots 4/5 test it before writing the node position
    int field_0x3c;           // +0x3c cleared when the row is written; D3DIM slots 4/5 test it before writing the node axes
};

class Character : public virtual GameObject {
public:
    // ctor 0x004455b0's callee 0x004a6780 takes (a, most-derived flag): one source argument plus
    // the hidden flag (tier 2, from the D3DIM ctor's pushes). The argument's role is tier 3.
    explicit Character(int a);
    virtual ~Character();                       // core 0x004a69d0, deleting 0x004a68d0
    // --- vtable 0x00555318 (offset 0), slots 0..10, all introduced here ---
    virtual void CharacterVirtualSlot0();         // 0x00464e90 (ret; shared empty stub)
    virtual void CharacterVirtualSlot1();         // 0x004a6930
    virtual void CharacterVirtualSlot2();         // 0x00464e90 (ret)
    virtual void CharacterVirtualSlot3(int a);    // 0x00464e80 (ret 4; shared empty stub)
    virtual void CharacterVirtualSlot4(int a, int b);   // 0x004a6ba0 (ret 8; shared by 4..6)
    virtual void CharacterVirtualSlot5(int a, int b);   // 0x004a6ba0
    virtual void CharacterVirtualSlot6(int a, int b);   // 0x004a6ba0
    virtual int CharacterVirtualSlot7(float time, int a, int b);  // 0x004a70c0 (ret 0xc): fld of the first argument, added to +0x10
    virtual void CharacterVirtualSlot8(int a);    // 0x004a98b0 (ret 4)
    virtual void CharacterVirtualSlot9(int a);    // 0x00464e80 (ret 4)
    virtual void CharacterVirtualSlot10();        // 0x00464e90 (ret)

    // Non-virtual helpers.  Owner class is tier 3 (address proximity to Character's
    // code at 0x004a6930..0x004a98b0; the callers pass the 0x21c subobject pointer,
    // which is both the Character and the D3DIMSoultreeCharacter start).
    // 0x004a6b10, thiscall, ret 4: takes a pointer to {int, elements, count} and qsorts (0x534426)
    // the 0x2c-byte records (tier 1 decode); D3DIM slot 2 passes the address of its pose list.
    void SortPoseList(void* poseList);
    // 0x004a6910, thiscall, ret 8: calls GameObject slot 8's body (0x004692f0, stores its argument in
    // field_0x18) with the first argument; the second argument is unused (tier 1 decode).
    void Method_0x004a6910(int a, int b);
    // 0x004a62d0 and 0x004a6500, thiscall, no arguments (plain ret). Both reference Motnctrl.cpp's
    // __FILE__ (0x0056e034) with debug new (lines 0x26b, 0x29d) and use the name strings at +0x8c / +0xdc /
    // +0x12c, so they belong to a different TU than D3DIMSoultreeMotnctrl.cpp (tier 1 decode).
    void LoadVut();
    void LoadMir();
    void ApplyRestPose();                               // ret
    // Motion control (src/krusty2/motion/Motnctrl.cpp; names tier 3, ABI from each body's ret N).
    int LoadMotions();                                      // 0x004a66c0
    void CaptureMotion(Motion* motion);                     // 0x004a6a60, ret 4
    void SortMotion(Motion* motion);                        // 0x004a6ab0, ret 4
    Motion* FindMotion(const char* name, int a);            // 0x004a6b30, ret 8
    unsigned char FindNode(const char* name);               // 0x004a6b50, ret 4
    void ApplyPoseListSlot4(MotionPoseList* list, int mask);  // 0x004a8a80, ret 8
    void ApplyPoseListSlot6(MotionPoseList* list, int mask);  // 0x004a8ac0, ret 8
    void SetMotionByName(const char* name);                 // 0x004a8b10, ret 4
    void SetMotion(Motion* motion);                         // 0x004a8b40, ret 4
    void BlendToMotion(Motion* motion);                     // 0x004a8b60, ret 4
    void BlendToMotion(Motion* motion, float time);         // 0x004a8b80, ret 8
    void Method_0x004a8bf0(int a, float b);                 // ret 8
    void Method_0x004a8c50(int a, int b, float c, float d); // ret 0x10

    // vfptr +0, vbptr +4 (compiler generated); Character's own data up to 0x1a0.
    // Offsets are tier 1 (ctor 0x004a6780 stores, slot 8 0x004a98b0 copies, Motnctrl.cpp users);
    // names are tier 3.  See src/krusty2/motion/Motnctrl.cpp.
    SltFile* sltFile;               // +0x008 set by D3DIM slot 11 around the slot 1 call; slot 1 reads the "General info" keys through it
    int chr_field_0x0c;             // +0x00c ctor 1; SetMotion (0x004a8b40) clears it
    float chr_field_0x10;           // +0x010 copied to blendFromTime by BlendToMotion (0x004a8b80)
    int motionCount;                // +0x014 "NumberOfMotions" (slot 1); loop bound of LoadMotions and the dtor
    Motion* currentMotion;          // +0x018 SetMotion stores the motion; cleared by LoadMotions and the dtor
    MotionPoseList* currentFrame;   // +0x01c SetMotion stores the motion's first frame; cleared by the dtor
    Motion* blendFromMotion;        // +0x020 BlendToMotion saves currentMotion here before switching
    int chr_field_0x24;             // +0x024 BlendToMotion only blends when it is nonzero
    int chr_field_0x28;             // +0x028 ctor 0; not copied by slot 8
    float blendFromTime;            // +0x02c BlendToMotion: chr_field_0x10 at the switch
    float blendDuration;            // +0x030 BlendToMotion: the transition time argument
    int blendActive;                // +0x034 BlendToMotion sets 1; SetMotion clears it
    int chr_field_0x38;             // +0x038 ctor 0; copied by slot 8
    char sltPath[0x50];             // +0x03c D3DIM slot 11 copies the resource name (at most 0x4f chars)
    char vutFilename[0x50];         // +0x08c "VUTFilename" (slot 1); read by LoadVUT (0x004a62d0)
    char mirFilename[0x50];         // +0x0dc "MIRFilename" (slot 1); read by LoadMIR (0x004a6500)
    char contentDirectory[0x50];    // +0x12c "ContentDirectory" (slot 1); path prefix of the motion and VUT/MIR files
    int keepMotions;                // +0x17c ctor 1; LoadMotions allocates and fills motions only when set
    Motion** motions;               // +0x180 motionCount entries (LoadMotions line 0x2d2); released by the dtor
    int nodeCount;                  // +0x184 rows in nodeNames; slot 10 loops to it, slot 2 sizes the pose array from it
    NodeNameEntry* nodeNames;       // +0x188 FillNodeNames table; index = pose nodeIndex (slots 2..6, 10)
    int* mirrorMap;                 // +0x18c slot 5 maps a pose's node index through it
    MotionPoseList poseList;        // +0x190 rest pose list: slot 2 allocates poses (line 0x12d) and sorts it via 0x004a6b10;
                                    // slot 8 copies it by value; 0x004a8b00 applies it through slot 6
    int vutLoaded;                  // +0x19c tested by LoadVUT; slot 8 sets 1
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
    // Returns the GameObject subobject (this converted to GameObject*) on success, 0 when the
    // resource could not be found and the object released itself (tier 1 decode of 0x00445680).
    virtual GameObject* D3DIMVirtualSlot11(int a1, const char* a2, const SoultreeLoadDesc* a3, int a4,
                                    int a5, int a6); // 0x00445680 (ret 0x18)

    // D3DIM's own data 0x1a0..0x210 (then vtordisp at 0x210, GameObject at 0x214).
    // modelNode is the pointer that SoultreePhysicsCharacter code reads at the
    // absolute offset 0x3bc (0x21c + 0x1a0); its type is shared with
    // SoultreePhysicsBaseObject::field_0x08 in the collision area (tier 3).
    SoultreeObject* modelNode;
    char d3d_field_0x1a4[0x6c];
};

#endif
