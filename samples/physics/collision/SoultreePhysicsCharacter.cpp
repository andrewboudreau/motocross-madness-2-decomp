// SoultreePhysicsCharacter small virtuals (assigned targets).  TU ownership tier 2:
// SoulTreePhysics.cpp (__FILE__ at 0x00503b13/0x00503f87).
#include "SoultreePhysicsCharacter.h"

extern void Fn_4b5a60(SoultreeVec3 a, SoultreeVec3 b, float* c, float* d, float* e, float* f,
                      float* g, float* h, float* i);
// Body/collision object at field_0x128; 0x00435fe0 is a thiscall refresh with no arguments.
class SoultreeBody { public:
    void Fn_435fe0();
};

// slot 1 (0x005040c0): base slot 1 then clears four flag bytes.
void SoultreePhysicsCharacter::UnknownVirtualSlot1(float value)
{
    SoultreePhysicsBaseObject::UnknownVirtualSlot1(value);
    field_0x430 = 0;
    field_0x431 = 0;
    field_0x432 = 0;
    field_0x433 = 0;
}

// slot 8 (0x005041c0)
void SoultreePhysicsCharacter::UnknownVirtualSlot8()
{
    field_0x194 = field_0x1a0 = field_0x42c->Fn_4fd7f0(&field_0x18);
}

// slot 42 (0x00504470)
int SoultreePhysicsCharacter::UnknownVirtualSlot42()
{
    if (field_0x433 >= 0 && field_0x430) {
        return 1;
    }
    return 0;
}

// slot 33 (0x005040f0), retail `ret 0x18`
int SoultreePhysicsCharacter::UnknownVirtualSlot33(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                                   const SoultreeVec3* a3, int a4, int a5, float a6)
{
    UnknownVirtualSlot1(a6);
    field_0x88 = *a2;
    field_0x94 = *a3;
    field_0x08->Fn_4fc630(a1->x, a1->y, a1->z);
    field_0x08->Fn_4fc970(&field_0x0c);
    UnknownVirtualSlot36();
    if (field_0x218) {
        field_0x218->Fn_4fc9a0(0, &field_0x18);
    } else {
        field_0x08->Fn_4fc9a0(0, &field_0x18);
    }
    field_0x21c.Fn_4a8b00();
    field_0x128->Fn_435fe0();
    field_0x109 = 0;
    field_0x20d = 1;
    field_0x20c = 1;
    return 0;
}

// slot 41 (0x00504360)
void SoultreePhysicsCharacter::UnknownVirtualSlot41()
{
    field_0x42c->Fn_4fc540(0, field_0xa0, field_0xac);
    field_0x3bc->Fn_4fc050(0, &field_0xa0, &field_0xac, 1, 0);
    field_0x21c.Fn_4a8b00();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    Fn_4b5a60(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c, &field_0x38,
              &field_0x3c, &field_0x44, &field_0x40);
    field_0xa0 = field_0x88;
    field_0xac = field_0x94;
    field_0x48 = field_0x2c;
    field_0x4c = field_0x30;
    field_0x50 = field_0x34;
    field_0x54 = field_0x38;
    field_0x58 = field_0x3c;
    field_0x5c = field_0x40;
    field_0x60 = field_0x44;
}
