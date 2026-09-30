// KrustyBike.h -- provisional class shape for KrustyBike (RTTI '.?AVKrustyBike@@').
//
// Confirmed (tier 1): RTTI base Bike; three vtables: +0 0x00554548 (103 slots),
// +540 0x00554514, +5644 0x005544a4 (rtti_classes vtable_records / COLs).
// Provisional (tier 3): Bike is a stand-in (see KrustyBikeTypes.h); the secondary
// vtables at +540 and +5644 are not modelled here, only the primary vtable slots
// that this translation unit overrides (analysis vtable_overrides.json).
#ifndef KRUSTYBIKE_H
#define KRUSTYBIKE_H

#include "KrustyBikeTypes.h"

class KrustyBike : public Bike {
public:
    virtual void UnknownVirtualSlot1(int a);
    virtual void UnknownVirtualSlot3(int a0, int a1, int a2, int a3, int a4, int a5, int* a6);
    virtual void UnknownVirtualSlot4(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int* a13, int a14);
    virtual void UnknownVirtualSlot8();
    virtual int UnknownVirtualSlot12(int a);
    virtual void UnknownVirtualSlot14(KbVec3* a, KbVec3* b, KbVec3* c);
    virtual void UnknownVirtualSlot15(KbVec3* v, float* out);
    virtual KbVec3 UnknownVirtualSlot16(KbVec3* v);
    virtual int UnknownVirtualSlot22();
    virtual int UnknownVirtualSlot23();
    virtual int UnknownVirtualSlot24();
    virtual int UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot27();
    virtual int UnknownVirtualSlot33(int a0, int a1, int a2, int a3, int a4, int a5);
    virtual int UnknownVirtualSlot39(int a);
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot48();
    virtual int UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot63(int a);
    virtual void UnknownVirtualSlot64(int a);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float a);
    virtual void UnknownVirtualSlot71(int a);
    virtual KbVec3 UnknownVirtualSlot76(int a, int b);
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(KbInput* input);
    virtual int UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot96();
    virtual void UnknownVirtualSlot101();

    void Fn_00414370(int a);
    void Fn_0048D8B0();
    void Fn_00496DA0();

public:
    KbRace* field_0x740; // 0x740
    char pad_0x0744[0x14];
    float field_0x758; // 0x758
    char pad_0x075C[0x10];
    float field_0x76c; // 0x76C
    char pad_0x0770[0x34];
    char field_0x7a4; // 0x7A4
    char pad_0x07A5[0x3];
    float field_0x7a8; // 0x7A8
    char pad_0x07AC[0x58];
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
    KbVec3 field_0x1540; // 0x1540
    char pad_0x154C[0x1];
    char field_0x154d; // 0x154D
    char field_0x154e; // 0x154E
    char pad_0x154F[0xD];
    int field_0x155c; // 0x155C
    char pad_0x1560[0x85];
    char field_0x15e5; // 0x15E5
};

#endif
