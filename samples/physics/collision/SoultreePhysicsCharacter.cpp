// SoultreePhysicsCharacter small virtuals (assigned targets).  TU ownership tier 2:
// SoulTreePhysics.cpp (__FILE__ at 0x00503b13/0x00503f87).
#include <string.h>
#include "../hierarchy/SoultreePhysicsCharacter.h"
#include "../soultree_base/SoultreePhysicsCallees.h"
#include "SoultreePhysicsObject.h"

extern void Fn_4b5a60(SoultreeVec3 a, SoultreeVec3 b, float* c, float* d, float* e, float* f,
                      float* g, float* h, float* i);

// SoultreePhysicsBaseObject::field_0x128 is the CollisionObject that slot 2 creates
// (`new` of 0xb8 bytes, ctor 0x00431e70, tier 1).  The SoultreePhysicsBaseObject header only
// forward-declares it as SoultreeBody, so this TU defines that name as a thin CollisionObject.
class SoultreeBody : public CollisionObject {
};

// The scene node at field_0x08 is the D3DIM object (SoultreePhysicsCharacter copies
// d3d_field_0x1a0 into it).  0x004444c0 (thiscall, `ret 4`) is one of that object's methods.
// SoultreeNode (soultree_base/SoultreePhysicsCallees.h) does not declare it, so it is
// reached through this view (tier 3 name).
class SoultreeD3DNode {
public:
    void Fn_4444c0(int a);
};

// 0x134-byte file object: ctor 0x00460d10 takes one global, the destructor 0x00460d60
// fcloses the FILE* at +0x14 (tier 2).  Names are tier 3.
class SoultreeFile {
public:
    SoultreeFile(int a);            // 0x00460d10
    ~SoultreeFile();                // 0x00460d60
    char field_0x00[0x134];
};

// Object at +0x574 of the global at 0x0056e26c.  0x004e9cd0 (thiscall, ret 0x10) sits
// between the ResourceManager.cpp and SceneManager.cpp __FILE__ references.  It gets the
// file object, the name, the mode "rb" and an output buffer; on a zero return the caller
// clears the output path (tier 2 data flow, tier 3 names).
class SoultreeFileFinder {
public:
    int Fn_4e9cd0(SoultreeFile* file, const char* name, const char* mode, char* outPath);
};
struct SoultreeGlobals {
    char field_0x000[0x574];
    SoultreeFileFinder* field_0x574;
};
extern SoultreeGlobals* g_0056e26c;
extern int g_00572b44;          // passed by value to the SoultreeFile constructor

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
                                                   const SoultreeVec3* a3, const SoultreeVec3* a4,
                                                   int a5, float a6)
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
    Method_0x004a8b00();
    field_0x128->Fn_00435fe0();
    field_0x109 = 0;
    field_0x20d = 1;
    field_0x20c = 1;
    return 0;
}

// slot 41 (0x00504360)
void SoultreePhysicsCharacter::UnknownVirtualSlot41()
{
    field_0x42c->Fn_4fc540(0, field_0xa0, field_0xac);
    d3d_field_0x1a0->Fn_4fc050(0, &field_0xa0, &field_0xac, 1, 0);
    Method_0x004a8b00();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    Fn_4b5a60(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c, &field_0x38,
              &field_0x3c, &field_0x44, &field_0x40);
    field_0xa0 = field_0x88;
    field_0xac = field_0x94;
    field_0x50 = field_0x34;
    field_0x4c = field_0x30;
    field_0x48 = field_0x2c;
    field_0x54 = field_0x38;
    field_0x58 = field_0x3c;
    field_0x60 = field_0x44;
    field_0x5c = field_0x40;
}

// slot 40 (0x00503de0, `ret 0x6c`).  Loads the character through the D3DIM base and runs the
// physics setup (slot 2).  Then it builds "<name without extension>.col"; if that file is
// found it becomes the collision shape, otherwise the shape comes from the node.
GameObject* SoultreePhysicsCharacter::UnknownVirtualSlot40(int a1, int a2, const char* a3,
                                                           const SoultreeLoadDesc* a4, int a5,
                                                           SoultreeVec3 a6, SoultreeVec3 a7,
                                                           SoultreeVec3 a8, void* a9, void* a10,
                                                           float a11, int a12, int a13,
                                                           SoultreeSlot1f0* a14, float a15,
                                                           int a16, float a17, float a18,
                                                           int a19, unsigned char a20, int a21)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeCharacter::D3DIMVirtualSlot11(a1, a3, a4, a5, 1, 1);
    field_0x42c = field_0x08 = d3d_field_0x1a0;
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(a1, a2, a6, a7, a8, a9, a10, a11, a12, a13,
                                                   a14, a15, a16, a17, a18, 0.0f, 0, a19, a20,
                                                   a21);
    if (a4 && (a4->field_0x25 & 1)) {
        ((SoultreeD3DNode*)field_0x08)->Fn_4444c0(1);
    }

    int len = strlen(a3);
    int n = len > 0x103 ? 0x103 : len;
    strncpy(name, a3, n);
    name[n] = 0;
    strcpy(strrchr(name, '.'), ".col");

    SoultreeFile* file = new(__FILE__, 0x8cd) SoultreeFile(g_00572b44);
    if (!g_0056e26c->field_0x574->Fn_4e9cd0(file, name, "rb", colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (field_0x128) {
        if (colPath[0]) {
            field_0x128->Fn_00432800(field_0x08, colPath);
        } else {
            field_0x128->Fn_004324b0(field_0x08, 1, 0, 0, 0);
        }
        field_0x128->field_0x64 = 0;
        field_0x128->field_0x60 = this;
        field_0x128->Fn_00435fe0();
        GameObject* child = field_0x128;
        GameObject::Method_0x00469190(child, -1);
    }
    return this;
}

// SoultreePhysicsObject slot 40 (0x00503970, `ret 0x70`).  The same loader for the D3DIM
// object base.  The differences are slot 9 instead of slot 11, line 0x84e instead of 0x8cd,
// and shape setup 0x00432720 instead of 0x004324b0.  SoultreePhysicsObject has two
// GameObjects, so field_0x08 and the GameObject call are qualified.
GameObject* SoultreePhysicsObject::UnknownVirtualSlot40(int a1, const char* a2,
                                                        const SoultreeLoadDesc* a3, int a4,
                                                        SoultreeVec3 a5, SoultreeVec3 a6,
                                                        SoultreeVec3 a7, void* a8, void* a9,
                                                        float a10, int a11, int a12,
                                                        SoultreeSlot1f0* a13, float a14,
                                                        int a15, float a16, float a17,
                                                        float a18, int a19, int a20,
                                                        unsigned char a21, int a22)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeObject::D3DIMObjectVirtualSlot9(a1, a2, a3, a4, 1);
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(a1, 1, a5, a6, a7, a8, a9, a10, a11, a12,
                                                   a13, a14, a15, a16, a17, a18, a19, a20,
                                                   a21, a22);
    if (a3 && (a3->field_0x25 & 1)) {
        ((SoultreeD3DNode*)SoultreePhysicsBaseObject::field_0x08)->Fn_4444c0(1);
    }

    int len = strlen(a2);
    int n = len > 0x103 ? 0x103 : len;
    strncpy(name, a2, n);
    name[n] = 0;
    strcpy(strrchr(name, '.'), ".col");

    SoultreeFile* file = new(__FILE__, 0x84e) SoultreeFile(g_00572b44);
    if (!g_0056e26c->field_0x574->Fn_4e9cd0(file, name, "rb", colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (field_0x128) {
        if (colPath[0]) {
            field_0x128->Fn_00432800(SoultreePhysicsBaseObject::field_0x08, colPath);
        } else {
            field_0x128->Fn_00432720(SoultreePhysicsBaseObject::field_0x08, 1, 0, 0, 0);
        }
        field_0x128->field_0x64 = 0;
        field_0x128->field_0x60 = this;
        field_0x128->Fn_00435fe0();
        GameObject* child = field_0x128;
        D3DIMSoultreeObject::Method_0x00469190(child, -1);
    }
    return static_cast<D3DIMSoultreeObject*>(this);
}
