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
    KrustyBike();
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
    virtual int UnknownVirtualSlot28(int a);
    virtual int UnknownVirtualSlot33(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5);
    virtual int UnknownVirtualSlot39(float a);
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot48();
    virtual int UnknownVirtualSlot52();
    virtual int UnknownVirtualSlot63(float a);
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
    virtual void UnknownVirtualSlot96();
    virtual int UnknownVirtualSlot99(float a, float b, int c, float d);
    virtual void UnknownVirtualSlot101();

    int Fn_00414370(float dt);   // slot 63 returns its result (0x004924c0)
    void Fn_0048D8B0();
    void Fn_004925A0(Vehicle* who, int flag);
    void Fn_00496DA0();

public:
    char field_0x734; // 0x734  first KrustyBike field (Bike's own data ends at 0x734)
    char field_0x735; // 0x735
    char field_0x736; // 0x736
    char pad_0x0737[0x9];
    KbRace* field_0x740; // 0x740
    char pad_0x0744[0x14];
    float field_0x758; // 0x758
    char pad_0x075C[0x10];
    float field_0x76c; // 0x76C
    char pad_0x0770[0x1C];
    int field_0x78c; // 0x78C
    char pad_0x0790[0x14];
    char field_0x7a4; // 0x7A4
    char pad_0x07A5[0x3];
    float field_0x7a8; // 0x7A8
    float field_0x7ac; // 0x7AC
    float field_0x7b0; // 0x7B0
    char pad_0x07B4[0x4];
    int field_0x7b8; // 0x7B8
    char pad_0x07BC[0x48];
    float field_0x804; // 0x804
    float field_0x808; // 0x808
    char pad_0x080C[0x8];
    int field_0x814; // 0x814
    char pad_0x0818[0xBFC];
    KbObj128* field_0x1414; // 0x1414
    KbObj128* field_0x1418; // 0x1418
    float field_0x141c; // 0x141C
    char pad_0x1420[0x100];
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
    char pad_0x154C[0x1];
    char field_0x154d; // 0x154D
    char field_0x154e; // 0x154E
    char pad_0x154F[0xD];
    int field_0x155c; // 0x155C
    char pad_0x1560[0x85];
    char field_0x15e5; // 0x15E5
    char pad_0x15E6[0x22];   // own data ends at 0x1608; the compiler places the vtordisp there
};

typedef char kb_assert_sizeof[(sizeof(KrustyBike) == 0x160c + 0x2c) ? 1 : -1];

#endif
