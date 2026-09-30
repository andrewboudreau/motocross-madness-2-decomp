// SoulTreePhysics.cpp -- SoultreePhysicsBaseObject (D:\aardvark\VC\krusty2\SoulTreePhysics.cpp,
// confirmed by __FILE__ xrefs at 0x00500d6c/0x00500dbd/0x00500fe2/0x0050117c).
// Translation-unit ownership of the neighbouring functions is tier 2 (proximity
// plus the same __FILE__ string).
#include <math.h>
#include "SoultreePhysicsBaseObject.h"
#include "SoultreePhysicsCallees.h"
#include "SoultreePhysicsContact.h"

#define g_Zero g_SoultreeZeroVec3

// slot 5 (0x004aa150): shared with Vehicle/Character, compares its argument with zero.
int SoultreePhysicsBaseObject::UnknownVirtualSlot5(int value)
{
    return value == 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot10()
{
    if (field_0x154 == field_0x150)
        return 1;
    return 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot12(int value)
{
    return 0;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot19(SoultreeAttachment* a)
{
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot20(SoultreeAttachment* a)
{
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot22()
{
    return field_0x208 == field_0x204;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot23()
{
    return !field_0x108 && field_0xbc >= 1.0f;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot24()
{
    return 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot25()
{
    return 0;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot26()
{
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot27()
{
}

float SoultreePhysicsBaseObject::UnknownVirtualSlot32()
{
    if (UnknownVirtualSlot10())
        return 0.0f;
    return field_0x15c;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot34()
{
    return field_0x08->Fn_4fc540(0, field_0x88, field_0x94);
}

// ==== chunks ====

// Length with the same shape the retail code uses: 1.0f is returned for a unit
// squared length, otherwise the square root.
static inline float SquareMagnitudeAcc(const SoultreeVec3& v)
{
    float s = v.x * v.x;
    s += v.y * v.y;
    s += v.z * v.z;
    return s;
}

static inline float VecLength(const SoultreeVec3& v)
{
    float s = SquareMagnitudeAcc(v);
    if (s == 1.0f)
        return 1.0f;
    return (float)sqrt(s);
}

// Cross product in the operand order the retail code uses: r.x = b.z * a.y - b.y * a.z
// (the product with b comes first in every fmul; see slots 4 and 13).
static inline SoultreeVec3 SoultreeCross(const SoultreeVec3& a, const SoultreeVec3& b)
{
    SoultreeVec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}

// slot 3 (0x00501310): runs the TU-local solver helper, then refreshes cached values.
void SoultreePhysicsBaseObject::UnknownVirtualSlot3(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    Fn_500220(field_0x14c, field_0x24, field_0x08, a1, a2, a3, &field_0xe4, a4, &field_0xd8,
              &field_0x64, a7, a6);
    field_0xbc = VecLength(field_0x64);
    field_0xcc = field_0x08->Fn_4fd5c0(&field_0xd8);
}

// slot 4 (0x005013d0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot4(int a1, SoultreeVec3* a2, const SoultreeVec3* a3,
                                                    const SoultreeVec3* a4, int a5, int a6, int a7,
                                                    const SoultreeVec3* a8, const SoultreeVec3* a9,
                                                    const SoultreeVec3* a10, int a11, int a12, int a13,
                                                    int a14, float a15)
{
    SoultreeVec3 l1 = (SoultreeCross(*a3, *a4) + *a2) * a15;
    SoultreeVec3 l2 = (SoultreeCross(*a9, *a10) + *a8) * a15;
    Fn_5004a0(field_0x14c, a1, field_0x24, field_0x08, &l1, a4, &field_0xe4, &field_0xd8, a2, a6,
              a7, &l2, a10, a11, a12, a13, a14);
    field_0xbc = VecLength(*a2);
    field_0xcc = field_0x08->Fn_4fd5c0(&field_0xd8);
}

// slot 35 (0x004aa1e0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot35(int a, int b)
{
    return field_0x08->Fn_4fc050(0, &field_0x88, &field_0x94, a, b);
}
// slot 0 (0x005008d0): stores value + field_0x150 and derived quantities.
void SoultreePhysicsBaseObject::UnknownVirtualSlot0(float value)
{
    field_0x154 = value + field_0x150;
    field_0x188.y = -field_0x154;
    field_0x158 = field_0x154 * 0.0310559f;
    field_0x24 = 1.0f / field_0x158;
}

// slot 1 (0x00500910): reset dynamic state.
void SoultreePhysicsBaseObject::UnknownVirtualSlot1(float value)
{
    UnknownVirtualSlot0(value);
    field_0x64 = g_Zero;
    field_0x7c = g_Zero;
    field_0xb8 = 0.0f;
    field_0xbc = 0.0f;
    field_0x70 = g_Zero;
    field_0xc0 = g_Zero;
    field_0xcc = g_Zero;
    field_0xd8 = g_Zero;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    field_0x38 = 0.0f;
    field_0x3c = 1.0f;
    field_0x40 = 0.0f;
    field_0x44 = 1.0f;
    field_0x194 = g_Zero;
    field_0xa0 = field_0x88;
    field_0xac = field_0x94;
    field_0x54 = 0.0f;
    field_0x58 = 1.0f;
    field_0x50 = field_0x34;
    field_0x4c = field_0x30;
    field_0x48 = field_0x2c;
    field_0x60 = 1.0f;
    field_0x5c = 0.0f;
    field_0x108 = 0;
    field_0x214 = 0;
    field_0x138 = 0;
}

// slot 8 (0x00501bd0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot8()
{
    SoultreeVec3 t = field_0x08->Fn_4fd7f0(&field_0x18);
    field_0x1a0 = t;
    field_0x194 = t;
}

// slot 9 (0x00501ce0): fixed-timestep accumulator.
void SoultreePhysicsBaseObject::UnknownVirtualSlot9(float dt, int* steps)
{
    field_0x144 = dt + field_0x1e0;
    int n = (int)(field_0x144 / field_0x1e4);
    *steps = n;
    if (n) {
        field_0x1e0 = field_0x144 - n * field_0x1e4;
        if (*steps > field_0x1ec)
            *steps = field_0x1ec;
        field_0x13c = field_0x1e4;
    } else {
        field_0x1e0 = 0.0f;
        if (dt != 0.0f) {
            field_0x13c = field_0x144;
        } else {
            field_0x13c = field_0x144 = field_0x1e4;
        }
        *steps = 1;
    }
    field_0x140 = 1.0f / field_0x13c;
}

// slot 11 (0x00501da0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot11(int a1, int a2, int a3, int a4, int a5)
{
    return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, 0, 0x7fffffff,
                     0x7fffffff, 0, 3.0f, field_0x20f, &field_0xa0, 0, !field_0x109,
                     a2, a3, a4, a5);
}

// slot 15 (0x00502200)
void SoultreePhysicsBaseObject::UnknownVirtualSlot15(const SoultreeVec3* a, SoultreeVec3* b)
{
    SoultreeVec3 v = field_0x08->Fn_4fd710(a);
    SoultreeVec3 w;
    if (UnknownVirtualSlot10()) {
        w.x = v.x * field_0xf0.x;
        w.y = v.y * field_0xf0.y;
        w.z = v.z * field_0xf0.z;
    } else {
        w.x = v.x * field_0xe4.x;
        w.y = v.y * field_0xe4.y;
        w.z = v.z * field_0xe4.z;
    }
    Fn_4cb6e0(&field_0xd8, b, &w, field_0x13c, 0);
}

// slot 16 (0x005022b0)
SoultreeVec3 SoultreePhysicsBaseObject::UnknownVirtualSlot16(const SoultreeVec3* in)
{
    SoultreeVec3 t;
    if (UnknownVirtualSlot10()) {
        t.x = field_0xf0.x * in->x;
        t.y = field_0xf0.y * in->y;
        t.z = field_0xf0.z * in->z;
    } else {
        t.x = field_0xe4.x * in->x;
        t.y = field_0xe4.y * in->y;
        t.z = field_0xe4.z * in->z;
    }
    return t;
}

// slot 17 (0x00502330): average of the target positions of the non-type-4 attachments.
SoultreeVec3 SoultreePhysicsBaseObject::UnknownVirtualSlot17()
{
    SoultreeVec3 sum;
    int n = 0;
    int first = 1;
    for (int i = 0; i < field_0x1dc; i++) {
        SoultreeAttachment* a = &field_0x1d4[i];
        if (a->type == 4)
            continue;
        if (first) {
            sum = a->field_0x10->field_0x20;
            first = 0;
        } else {
            sum += a->field_0x10->field_0x20;
        }
        n++;
    }
    if (n) {
        float inv = 1.0f / n;
        return sum * inv;
    }
    return g_Zero;
}

// slot 18 (0x00502420)
void SoultreePhysicsBaseObject::UnknownVirtualSlot18(SoultreeAttachment* a)
{
    for (int i = 0; i < field_0x130; i++) {
        SoultreeContact* c = field_0x12c[i];
        if (c->field_0xa4 && !c->field_0xa8 &&
            field_0x1f0->field_0xa4->field_0x400[(unsigned char)c->field_0xbc] != 0) {
            c->field_0xa8 = 1;
            a->field_0x04->Fn_4b8d90(c->field_0x20, 0);
            if (a->field_0x24)
                a->field_0x04->field_0x50 = a->field_0x04->field_0x44;
            a->field_0x04->field_0x60 = 1;
            a->field_0x24 = 0;
            return;
        }
    }
}

// slot 28 (0x00502950)
int SoultreePhysicsBaseObject::UnknownVirtualSlot28(int a)
{
    field_0x128->Fn_435fb0();
    field_0x128->Fn_438e70();
    if (field_0x128->field_0x58) {
        field_0x0c -= *field_0x128->field_0x5c;
        field_0x08->Fn_4fc660(&field_0x0c);
        field_0x128->Fn_435fb0();
    }
    if (field_0x138) {
        UnknownVirtualSlot27();
        field_0x138 = 0;
    }
    return a;
}

// slot 29 (0x005029d0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot29(int a)
{
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

// slot 30 (0x00502f10)
void SoultreePhysicsBaseObject::UnknownVirtualSlot30()
{
    for (int i = 0; i < field_0x130; i++) {
        field_0x12c[i]->field_0xa8 = 0;
        field_0x12c[i]->field_0xac = 0;
        field_0x12c[i]->field_0xb0 = 0;
    }
}

// slot 31 (0x00501e10)
void SoultreePhysicsBaseObject::UnknownVirtualSlot31()
{
    for (int i = 0; i < field_0x130; i++) {
        SoultreeContact* c = field_0x12c[i];
        if (c->field_0x04 && c->field_0x98 >= 0.0f) {
            if (c->field_0xb4 != 2)
                c->field_0xb4 = 1;
            c->Fn_43a640(&field_0x18, &field_0xcc, &field_0x64, &c->field_0x38, field_0xbc);
        } else {
            c->field_0xb4 = 0;
        }
    }
}

// slot 33 (0x00501c20)
int SoultreePhysicsBaseObject::UnknownVirtualSlot33(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                                    const SoultreeVec3* a3, int a4, int a5, float a6)
{
    UnknownVirtualSlot1(a6);
    field_0x88 = *a2;
    field_0x94 = *a3;
    field_0x08->Fn_4fc630(a1->x, a1->y, a1->z);
    field_0x08->Fn_4fc970(&field_0x0c);
    UnknownVirtualSlot36();
    if (field_0x218)
        field_0x218->Fn_4fc9a0(0, &field_0x18);
    else
        field_0x08->Fn_4fc9a0(0, &field_0x18);
    if (field_0x128)
        field_0x128->Fn_435fe0();
    field_0x109 = 0;
    field_0x20d = 1;
    field_0x20c = 1;
    return 0;
}

// slot 37 (0x00502a40): append an attachment record.
SoultreeAttachment* SoultreePhysicsBaseObject::UnknownVirtualSlot37(int type, void* a2,
                                                                   SoultreeAttachTarget* a3,
                                                                   const SoultreeVec3* v)
{
    if (field_0x1dc >= field_0x1d8 || type == 0)
        return 0;
    SoultreeAttachment* a = &field_0x1d4[field_0x1dc];
    a->type = type;
    a->field_0x10 = a3;
    a->field_0x04 = 0;
    a->field_0x08 = 0;
    a->field_0x0c = 0;
    a->field_0x14 = 0;
    a->field_0x18 = g_Zero;
    a->field_0x24 = 0;
    switch (type) {
    case 1:
        a->field_0x04 = (SoultreeAttachedObject*)a2;
        break;
    case 2:
        a->field_0x08 = a2;
        break;
    case 3:
        a->field_0x0c = a2;
        break;
    case 4:
        a->field_0x14 = a2;
        a->field_0x18 = *v;
        a->field_0x10 = 0;
        break;
    }
    field_0x1dc++;
    return a;
}

// slot 39 (0x005019a0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot39(int a)
{
    if (field_0x109) {
        field_0x0c = field_0x10c;
        field_0x08->Fn_4fc660(&field_0x0c);
        return 1;
    }
    return 0;
}

// slot 6 (0x0040b410): shared with Vehicle/Character (same address).  Applies a drag-like
// correction along field_0x64 limited by the amount in *b.  Semantic reading is tier 3;
// note that the retail code compares and divides by the SQUARED length (see below).
void SoultreePhysicsBaseObject::UnknownVirtualSlot6(SoultreeVec3* a, float* b)
{
    if (field_0xb8 <= 0.001f)
        return;
    float k = -(field_0x148 * field_0xbc);
    field_0x1ac = field_0x64 * k;
    float lenSq = SquareMagnitudeAcc(field_0x1ac);
    if (lenSq == 1.0f) {
        lenSq = 1.0f;
    } else {
        float root = FastSqrt(lenSq);
        if (root <= 0.0f)
            return;
    }
    if (field_0xbc == 0.0f)
        return;
    float num = field_0x158;
    float den = field_0x13c;
    float t = field_0xbc;
    if (!(t <= *b))
        t = *b;
    if (t <= 0.0f)
        t = 0.0f;
    float q = num / den;
    float r = q * t;
    SoultreeVec3 v;
    if (lenSq > r) {
        v = field_0x1ac * (r / lenSq);
        *b -= t;
    } else {
        v = field_0x1ac;
        *b -= lenSq / q;
    }
    field_0x1b8 = v;
    *a += field_0x1b8;
}

// Length as computed by the retail inline helper: 1.0f for a unit squared length,
// otherwise the table-driven square root at 0x00460b50.
static inline float VecLengthFast(const SoultreeVec3& v)
{
    float s = SquareMagnitudeAcc(v);
    if (s == 1.0f)
        return 1.0f;
    return FastSqrt(s);
}

// slot 7 (0x005019e0): distributes the vector *a over the contacts, weighting each
// active contact by 1 - (its distance / summed distance).  Tier 3 reading.
void SoultreePhysicsBaseObject::UnknownVirtualSlot7(SoultreeVec3* a)
{
    float dist[124];
    int i;
    if (field_0x1d0) {
        float sum = 0.0f;
        int n = 0;
        int last = 0;
        for (i = 0; i < field_0x130; i++) {
            SoultreeContact* c = field_0x12c[i];
            if (c->field_0xa4) {
                SoultreeVec3 d = field_0x18 - c->field_0x14;
                float len = VecLengthFast(d);
                dist[i] = len;
                sum += len;
                n++;
                last = i;
            } else {
                c->field_0xa0 = 0.0f;
                c->field_0x44 = g_Zero;
            }
        }
        if (n == 1) {
            field_0x12c[last]->field_0xa0 = 1.0f;
            field_0x12c[last]->field_0x44 = *a;
            return;
        }
        for (i = 0; n > 0; i++) {
            SoultreeContact* c = field_0x12c[i];
            if (c->field_0xa4) {
                c->field_0xa0 = 1.0f - dist[i] / sum;
                c->field_0x44 = *a * c->field_0xa0;
                n--;
            }
        }
    } else {
        for (i = 0; i < field_0x130; i++) {
            field_0x12c[i]->field_0xa0 = 0.0f;
            field_0x12c[i]->field_0x44 = g_Zero;
        }
    }
}

// slot 13 (0x00501e90): per-contact response; accumulates into *a1 / *a2.
void SoultreePhysicsBaseObject::UnknownVirtualSlot13(SoultreeVec3* a1, SoultreeVec3* a2, float a3)
{
    if (!field_0x1cc)
        return;
    for (int i = 0; i < field_0x130; i++) {
        SoultreeContact* c = field_0x12c[i];
        if (!c->field_0xb4)
            continue;
        SoultreeVec3 t;
        float w = c->field_0xa0;
        t.x = w * a1->x;
        t.y = w * a1->y;
        t.z = w * a1->z;
        float d = DotProduct(t, c->field_0x2c);
        c->field_0x74 = d;
        if (d >= 0.0f) {
            c->field_0x74 = 0.0f;
            c->field_0x68 = SoultreeVec3(0.0f, 0.0f, 0.0f);
        } else {
            c->field_0x74 = -d;
            c->field_0x68 = *SoultreeScaleVec3(&t, &c->field_0x2c, -d);
        }
        c->UnknownVirtualSlot1();
        *a1 += c->field_0x78;
        SoultreeVec3 cr = SoultreeCross(c->field_0x38, c->field_0x78);
        *a2 += field_0x08->Fn_4fd710(&cr);
        if (c->field_0xb4 == 2) {
            *a2 *= a3;
            c->field_0xb4 = 0;
        }
    }
}

// slot 14 (0x00502080): integrates the accumulated force/torque estimates.
void SoultreePhysicsBaseObject::UnknownVirtualSlot14(const SoultreeVec3* a1, const SoultreeVec3* a2, int a3)
{
    field_0xc0 = UnknownVirtualSlot16(a1);
    UnknownVirtualSlot15(a2, &field_0xc0);
    field_0xd8 += field_0xc0 * field_0x13c;
    field_0xd8 *= 0.999f;
    field_0x70 = *a1 * field_0x24;
    field_0x64 += field_0x70 * field_0x13c;
    field_0xbc = VecLength(field_0x64);
}

// 0x005015b0: kept out of line on purpose (defined after its callers).
SoultreeVec3* SoultreeScaleVec3(SoultreeVec3* out, const SoultreeVec3* v, float s)
{
    out->x = s * v->x;
    out->y = s * v->y;
    out->z = s * v->z;
    return out;
}
// slot 36 (0x004aa210): resolve the initial orientation vectors and reset the pose state.
void SoultreePhysicsBaseObject::UnknownVirtualSlot36()
{
    if (field_0x88.x == 0.0f && field_0x88.z == 0.0f) {
        float s = (field_0x88.y < 0.0f) ? 1.0f : -1.0f;
        field_0x88.x = s * field_0x94.x;
        field_0x88.y = s * field_0x94.y;
        field_0x88.z = s * field_0x94.z;
    }
    field_0x94 = g_SoultreeVec3_685190;
    field_0x88.y = 0.0f;
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    field_0xa0 = field_0x88;
    field_0x2c = 0.0f;
    field_0x30 = 0.0f;
    field_0x38 = 0.0f;
    field_0x3c = 1.0f;
    field_0x40 = 0.0f;
    field_0x44 = 1.0f;
    field_0x4c = 0.0f;
    field_0x48 = 0.0f;
    field_0x54 = 0.0f;
    field_0x58 = 1.0f;
    field_0x60 = 1.0f;
    field_0x5c = 0.0f;
    field_0xac = field_0x94;
    field_0x50 = field_0x34;
}

