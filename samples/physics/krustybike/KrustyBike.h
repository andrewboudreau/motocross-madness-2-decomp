// KrustyBike.h -- provisional class shape for KrustyBike (RTTI '.?AVKrustyBike@@').
//
// Confirmed (tier 1): RTTI base Bike; three vtables: +0 0x00554548 (103 slots),
// +540 0x00554514, +5644 0x005544a4 (rtti_classes vtable_records / COLs).
// Bases are canonical (bike/Bike.h -> vehicle/Vehicle.h -> hierarchy/...), so the
// +540 (D3DIMSoultreeCharacter) and +5644 (virtual GameObject, vtordisp at +0x1608)
// subobjects are real.  KrustyBike's own data is 0x734..0x1608.  Overrides use the
// canonical slot signatures.
#ifndef KRUSTYBIKE_H
#define KRUSTYBIKE_H

#include "KrustyBikeTypes.h"

class KrustyBike : public Bike {
public:
    explicit KrustyBike(int flags);   // 0x0048fa60, ret 8 = flags + hidden most-derived flag
    virtual ~KrustyBike();
    virtual int GameObjectVirtualSlot10(float dt);   // 0x004977a0 (vbase vtable slot 10)

    virtual void UnknownVirtualSlot1(float a);
    virtual void UnknownVirtualSlot3(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, int a5, float* a6);
    virtual void UnknownVirtualSlot4(const Vec3* a0, Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5, int a6,
                                     const Vec3* a7, const Vec3* a8, const Vec3* a9,
                                     Vec3* a10, Vec3* a11, int a12, float* a13, float a14);
    virtual void UnknownVirtualSlot8();
    virtual int UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3, Vec3* a4, int* a5);
    virtual int UnknownVirtualSlot12(int a);
    virtual void UnknownVirtualSlot14(Vec3* a, const Vec3* b, const Vec3* c);
    virtual void UnknownVirtualSlot15(const Vec3* v, Vec3* out);
    virtual Vec3 UnknownVirtualSlot16(const Vec3* v);
    virtual int UnknownVirtualSlot22();
    virtual int UnknownVirtualSlot23();
    virtual int UnknownVirtualSlot24();
    virtual int UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot29(int a);
    virtual void UnknownVirtualSlot49(float dt);
    virtual void UnknownVirtualSlot50(int a, float b, int c);
    virtual int UnknownVirtualSlot28(int a);
    virtual int UnknownVirtualSlot33(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5);
    virtual int UnknownVirtualSlot39(float a);
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot48();
    virtual int UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot63(float a);
    virtual void UnknownVirtualSlot64(float a);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float a);
    virtual void UnknownVirtualSlot71(int a);
    virtual Vec3 UnknownVirtualSlot76(const Vec3* a, const Vec3* b);
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(VehicleWheel* wheel);
    virtual void UnknownVirtualSlot86();
    virtual int UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot97();
    virtual void UnknownVirtualSlot94();
    virtual void UnknownVirtualSlot96();
    virtual int UnknownVirtualSlot99(float a, float b, int c, float d);
    virtual void UnknownVirtualSlot101();

    int Fn_00414370(float dt);   // called by slot 63 (0x004924c0); the result is unused
    void Fn_0048D8B0();
    void Fn_0048D910(int idx);
    void Fn_0048D990(int idx);
    KrustyBike* FindNearestRival(float* outDistance);
    void Fn_0048E280();
    void Fn_004925A0(Vehicle* who, bool flag);
    void Fn_00496E20(KbRecorder* a);
    int Fn_00495C00();
    void Fn_00496DA0();
    void Fn_00496D20();
    void Fn_00496F90(KrustyBike* other);
    void Fn_00497370(KrustyBike* other);
    float Fn_00495FF0();
    // 0x004933e0: adds message 13's deltas to the decoded network state and copies it out.
    void Fn_004933E0(const KbNetDelta* delta, KbNetState* state);
    void Fn_00496E30(int a);
    void Fn_00413200(float dt);
    // 0x00493660 (8.5 KB) and 0x00492ad0: per-frame steps GameObject slot 10 runs (ret 8;
    // tier 3 roles: the local/replayed update and the network update).
    void Fn_00493660(float dt, int a);
    void Fn_00492AD0(float dt, int a);
    // 0x00492670 (ret 0xc; called three times by 0x00492ad0): sends (or records) message 1
    // with this bike's state and keeps a copy in `state`; `dt` is unused.
    void Fn_00492670(KbBikeState* state, float dt, int record);

public:
    char field_0x734; // 0x734  first KrustyBike field (Bike's own data ends at 0x734)
    char field_0x735; // 0x735
    char field_0x736; // 0x736
    char pad_0x0737[0x5];
    int field_0x73c; // 0x73C  0x65 from the constructor
    KbRace* field_0x740; // 0x740
    KbGhost* field_0x744; // 0x744
    int field_0x748; // 0x748
    int field_0x74c; // 0x74C
    int field_0x750; // 0x750
    float field_0x754; // 0x754
    float field_0x758; // 0x758
    int field_0x75c; // 0x75C
    int field_0x760; // 0x760
    int field_0x764; // 0x764
    float field_0x768; // 0x768
    float field_0x76c; // 0x76C
    int field_0x770; // 0x770
    int field_0x774; // 0x774
    char pad_0x0778[0x4];
    void* heapBufferA; // 0x77C  heap buffer released in the destructor (tier 3: type unknown)
    void* heapBufferB; // 0x780  heap buffer released in the destructor
    int field_0x784; // 0x784
    int field_0x788; // 0x788
    int field_0x78c; // 0x78C
    int field_0x790; // 0x790
    int field_0x794; // 0x794
    int field_0x798; // 0x798
    char pad_0x079C[0x4];
    short field_0x7a0; // 0x7A0
    short field_0x7a2; // 0x7A2
    char field_0x7a4; // 0x7A4
    char field_0x7a5; // 0x7A5
    char pad_0x07A6[0x2];
    float field_0x7a8; // 0x7A8
    float field_0x7ac; // 0x7AC
    float field_0x7b0; // 0x7B0
    float field_0x7b4; // 0x7B4  bonus accumulated by 0x00495ff0
    int field_0x7b8; // 0x7B8
    int field_0x7bc; // 0x7BC
    int field_0x7c0; // 0x7C0
    char pad_0x07C4[0x40];
    float field_0x804; // 0x804
    float field_0x808; // 0x808
    char pad_0x080C[0x8];
    int field_0x814; // 0x814
    char pad_0x0818[0x11b8 - 0x818];
    char field_0x11b8; // 0x11B8
    char pad_0x11B9[0x3];
    int field_0x11bc; // 0x11BC  network id (KbNetBike)
    unsigned char field_0x11c0; // 0x11C0
    char pad_0x11C1[0x1358 - 0x11c1];
    Vec3 field_0x1358; // 0x1358
    Vec3 field_0x1364; // 0x1364
    Vec3 field_0x1370; // 0x1370
    float field_0x137c; // 0x137C
    float field_0x1380; // 0x1380
    float field_0x1384; // 0x1384
    int field_0x1388; // 0x1388
    int field_0x138c; // 0x138C
    KbBikeState recordState; // 0x1390  last state recorded (0x00492ad0 with record set)
    KbRecorder* netRecorder; // 0x13FC
    int field_0x1400; // 0x1400
    int field_0x1404; // 0x1404
    int field_0x1408; // 0x1408
    int field_0x140c; // 0x140C
    int field_0x1410; // 0x1410  -1 from the constructor
    KbObj128* altBodyA; // 0x1414
    KbObj128* altBodyB; // 0x1418
    float field_0x141c; // 0x141C
    int animSetA[16]; // 0x1420  Fn_0048D910 indexes it with the argument (rider handle for riderCharacter); tier 3 name
    int animSetB[16]; // 0x1460  Fn_0048D910: same index, handle for the D3DIMSoultreeCharacter base
    int animSetC[16]; // 0x14A0  Fn_0048D990: handle for riderCharacter
    int animSetD[16]; // 0x14E0  Fn_0048D990: handle for the base character
    float field_0x1520; // 0x1520
    float field_0x1524; // 0x1524
    float field_0x1528; // 0x1528
    float field_0x152c; // 0x152C
    float field_0x1530; // 0x1530
    float field_0x1534; // 0x1534
    float field_0x1538; // 0x1538
    char field_0x153c; // 0x153C
    unsigned char field_0x153d; // 0x153D
    char field_0x153e; // 0x153E
    char field_0x153f; // 0x153F
    Vec3 field_0x1540; // 0x1540
    char field_0x154c; // 0x154C
    char field_0x154d; // 0x154D
    char field_0x154e; // 0x154E
    char pad_0x154F[0x1];
    float field_0x1550; // 0x1550  Fn_0048E280 stores a clamped, scaled bearing to the nearest rival
    int field_0x1554; // 0x1554  Fn_0048E280 clears it
    float field_0x1558; // 0x1558  Fn_0048E280: wrapped bearing to the rival relative to heading +0x50, clamped to +-2.7
    KrustyBike* nearestRival; // 0x155C  FindNearestRival result (closest other bike of the race; tier 3 name)
    KbBikeState netState; // 0x1560  last state sent to the peers (0x00492ad0)
    char pad_0x15CC[0x4];
    float field_0x15d0; // 0x15D0  seconds since the last short message (0x00492ad0)
    int field_0x15d4; // 0x15D4
    char pad_0x15D8[0xd];
    char field_0x15e5; // 0x15E5
    char field_0x15e6; // 0x15E6
    char pad_0x15E7[0x1];
    KbObj128* field_0x15e8; // 0x15E8
    char pad_0x15EC[0x18];
    float field_0x1604; // 0x1604  seconds to the next message 10 (0x00492670)
    // own data ends at 0x1608; the compiler places the vtordisp there
};

typedef char kb_assert_sizeof[(sizeof(KrustyBike) == 0x160c + 0x2c) ? 1 : -1];

#endif
