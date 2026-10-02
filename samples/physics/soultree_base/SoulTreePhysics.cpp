// SoulTreePhysics.cpp -- SoultreePhysicsBaseObject (D:\aardvark\VC\krusty2\SoulTreePhysics.cpp,
// confirmed by __FILE__ xrefs at 0x00500d6c/0x00500dbd/0x00500fe2/0x0050117c).
// Translation-unit ownership of the neighbouring functions is tier 2 (proximity
// plus the same __FILE__ string).
#include <math.h>
#include <string.h>
#include "SoultreePhysicsBaseObject.h"
#include "SoultreePhysicsCallees.h"
#include "core/DebugAlloc.h"
#include "core/MemTag.h"
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

void SoultreePhysicsBaseObject::UnknownVirtualSlot34()
{
    field_0x08->GetAxesIn(0, &field_0x88, &field_0x94);
}

// ==== chunks ====

// Length with the same shape the retail code uses: 1.0f is returned for a unit
// squared length, otherwise the square root.
static inline float SquareMagnitudeAcc(const Vec3& v)
{
    float s = v.x * v.x;
    s += v.y * v.y;
    s += v.z * v.z;
    return s;
}

static inline float VecLength(const Vec3& v)
{
    float s = SquareMagnitudeAcc(v);
    if (s == 1.0f)
        return 1.0f;
    return (float)sqrt(s);
}

// Cross product in the operand order the retail code uses: r.x = b.z * a.y - b.y * a.z
// (the product with b comes first in every fmul; see slots 4 and 13).
static inline Vec3 SoultreeCross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}

// Dot product with the z term first, the shape the retail code uses.
static inline float SoultreeDot(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

// slot 3 (0x00501310): runs the TU-local solver helper, then refreshes cached values.
void SoultreePhysicsBaseObject::UnknownVirtualSlot3(const Vec3* a1, const Vec3* a2,
                                                    const Vec3* a3, const Vec3* a4,
                                                    int a5, int a6, float* a7)
{
    Fn_500220(field_0x14c, field_0x24, field_0x08, a1, a2, a3, &field_0xe4, a4, &field_0xd8,
              &field_0x64, a7, a6);
    field_0xbc = VecLength(field_0x64);
    field_0xcc = field_0x08->LocalToWorldDirection(field_0xd8);
}

// Cross product built with the three-float constructor.  Slot 4 matches better with this
// form than with SoultreeCross (87.5% vs 85.1%); some fmul operand loads still differ.
static inline Vec3 SoultreeCrossCtor(const Vec3& a, const Vec3& b)
{
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// slot 4 (0x005013d0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot4(const Vec3* a1, Vec3* a2, const Vec3* a3,
                                                    const Vec3* a4, int a5, float a6, int a7,
                                                    const Vec3* a8, const Vec3* a9,
                                                    const Vec3* a10, Vec3* a11,
                                                    Vec3* a12, int a13, float* a14, float a15)
{
    Vec3 l1 = (SoultreeCrossCtor(*a3, *a4) + *a2) * a15;
    Vec3 l2 = (SoultreeCrossCtor(*a9, *a10) + *a8) * a15;
    ContactSolveImpulse(field_0x14c, a1, field_0x24, field_0x08, &l1, a4, &field_0xe4, &field_0xd8,
                        a2, a6, (SoultreeObject*)a7, &l2, a10, a11, a12, (Vec3*)a13, a14);
    float s = SoultreeDot(*a2, *a2);
    field_0xbc = (s == 1.0f) ? 1.0f : (float)sqrt(s);
    field_0xcc = field_0x08->LocalToWorldDirection(field_0xd8);
}

// slot 35 (0x004aa1e0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot35(int a, int b)
{
    field_0x08->SetAxesIn(0, &field_0x88, &field_0x94, a, b);
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
    field_0x194 = field_0x1a0 = field_0x08->WorldToLocalPoint(field_0x18);
}

// slot 9 (0x00501ce0): fixed-timestep accumulator (tier 3 reading; the arithmetic is decoded).
// field_0x1e4 is the fixed step, field_0x1e0 the carried remainder, field_0x1ec the cap on
// steps per frame.  steps = floor((dt + remainder) / step), remainder' = leftover time; when
// less than one step has elapsed the frame runs as a single variable-length step of dt (or
// of one nominal step when dt is 0).  field_0x13c = step length used, field_0x140 = 1/step.
void SoultreePhysicsBaseObject::UnknownVirtualSlot9(float dt, int* steps)
{
    field_0x144 = dt + field_0x1e0;
    int n = (int)(field_0x144 / field_0x1e4);
    *steps = n;
    if (n) {
        field_0x1e0 = field_0x144 - n * field_0x1e4;
        *steps = (*steps > field_0x1ec) ? field_0x1ec : *steps;
        field_0x13c = field_0x1e4;
    } else {
        field_0x1e0 = 0.0f;
        if (dt != 0.0f) {
            field_0x13c = field_0x144;
        } else {
            field_0x13c = field_0x1e4;
            field_0x144 = field_0x1e4;
        }
        *steps = 1;
    }
    field_0x140 = 1.0f / field_0x13c;
}

// slot 11 (0x00501da0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3,
                                                    Vec3* a4, int* a5)
{
    return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, 0, 0x7fffffff,
                     0x7fffffff, 0, 3.0f, field_0x20f, &field_0xa0, 0, !field_0x109,
                     a2, a3, a4, a5);
}

// slot 15 (0x00502200)
void SoultreePhysicsBaseObject::UnknownVirtualSlot15(const Vec3* a, Vec3* b)
{
    Vec3 v = field_0x08->WorldToLocalDirection(*a);
    Vec3 w;
    if (UnknownVirtualSlot10()) {
        w.x = v.x * field_0xf0.x;
        w.y = v.y * field_0xf0.y;
        w.z = v.z * field_0xf0.z;
    } else {
        w.x = v.x * field_0xe4.x;
        w.y = v.y * field_0xe4.y;
        w.z = v.z * field_0xe4.z;
    }
    UnknownAxisSettle_4cb6e0(&field_0xd8.x, &b->x, &w.x, field_0x13c, 0);
}

// slot 16 (0x005022b0)
Vec3 SoultreePhysicsBaseObject::UnknownVirtualSlot16(const Vec3* in)
{
    Vec3 t;
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
Vec3 SoultreePhysicsBaseObject::UnknownVirtualSlot17()
{
    Vec3 sum;
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
        return Vec3(sum.x * (1.0f / n), sum.y * (1.0f / n), sum.z * (1.0f / n));
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
    field_0x128->Fn_00435fb0();
    field_0x128->Fn_00438e70();
    if (field_0x128->field_0x58) {
        field_0x0c -= *(Vec3*)field_0x128->field_0x5c;
        field_0x08->SetPosition(field_0x0c);
        field_0x128->Fn_00435fb0();
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
int SoultreePhysicsBaseObject::UnknownVirtualSlot33(const Vec3* a1, const Vec3* a2,
                                                    const Vec3* a3, const Vec3* a4,
                                                    int a5, float a6)
{
    UnknownVirtualSlot1(a6);
    field_0x88 = *a2;
    field_0x94 = *a3;
    field_0x08->SetPosition(a1->x, a1->y, a1->z);
    field_0x08->GetPosition(&field_0x0c);
    UnknownVirtualSlot36();
    if (field_0x218)
        field_0x218->GetPositionIn(0, &field_0x18);
    else
        field_0x08->GetPositionIn(0, &field_0x18);
    if (field_0x128)
        field_0x128->Fn_00435fe0();
    field_0x109 = 0;
    field_0x20d = 1;
    field_0x20c = 1;
    return 0;
}

// slot 37 (0x00502a40): append an attachment record.
SoultreeAttachment* SoultreePhysicsBaseObject::UnknownVirtualSlot37(int type, void* a2,
                                                                   SoultreeAttachTarget* a3,
                                                                   const Vec3* v)
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
int SoultreePhysicsBaseObject::UnknownVirtualSlot39(float dt)
{
    if (field_0x109) {
        field_0x0c = field_0x10c;
        field_0x08->SetPosition(field_0x0c);
        return 1;
    }
    return 0;
}

// slot 6 (0x0040b410): shared with Vehicle/Character (same address).  Applies a drag-like
// correction along field_0x64 limited by the amount in *b.  Semantic reading is tier 3;
// note that the retail code compares and divides by the SQUARED length (see below).
// Reading: v = -(k * speed) * dir is a velocity-proportional (linear drag) vector.  The
// amount applied is clamped by a budget (num/den scaled by the available time t, num/den
// being a mass over the step) - if the drag exceeds the budget it is scaled down, otherwise it
// is applied whole and the budget is charged for what was used.  The result accumulates into *a.
void SoultreePhysicsBaseObject::UnknownVirtualSlot6(Vec3* a, float* b)
{
    if (field_0xb8 <= 0.001f)
        return;
    float k = -(field_0x148 * field_0xbc);
    field_0x1ac = field_0x64 * k;
    float lenSq = field_0x1ac.z * field_0x1ac.z + (field_0x1ac.x * field_0x1ac.x + field_0x1ac.y * field_0x1ac.y);
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
    Vec3 v;
    if (lenSq > r) {
        v = field_0x1ac * (r / lenSq);
        *b -= t;
    } else {
        *b -= lenSq / q;
        v = field_0x1ac;
    }
    field_0x1b8 = v;
    *a += field_0x1b8;
}

// Length as computed by the retail inline helper: 1.0f for a unit squared length,
// otherwise the table-driven square root at 0x00460b50.
static inline float VecLengthFast(const Vec3& v)
{
    float s = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (s == 1.0f)
        return 1.0f;
    return FastSqrt(s);
}

// slot 7 (0x005019e0): distributes the vector *a over the contacts, weighting each
// active contact by 1 - (its distance / summed distance).  Tier 3 reading.
// This is an inverse-distance weighting: contacts closer to the body position get the larger
// share (a lone active contact gets the whole vector, weight 1).  Vehicle slot 7 is a near copy.
void SoultreePhysicsBaseObject::UnknownVirtualSlot7(const Vec3* a)
{
    float dist[128];
    int i;
    if (field_0x1d0) {
        float sum = 0.0f;
        int n = 0;
        int last;
        for (i = 0; i < field_0x130; i++) {
            SoultreeContact* c = field_0x12c[i];
            if (c->field_0xa4) {
                Vec3 d = field_0x18 - c->field_0x14;
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
            SoultreeContact* c = field_0x12c[i];
            c->field_0xa0 = 0.0f;
            c->field_0x44 = g_Zero;
        }
    }
}

// slot 13 (0x00501e90): per-contact response; accumulates into *a1 / *a2.
void SoultreePhysicsBaseObject::UnknownVirtualSlot13(Vec3* a1, Vec3* a2, float a3)
{
    if (!field_0x1cc)
        return;
    for (int i = 0; i < field_0x130; i++) {
        SoultreeContact* c = field_0x12c[i];
        if (!c->field_0xb4)
            continue;
        Vec3 t;
        float w = c->field_0xa0;
        t.x = w * a1->x;
        t.y = w * a1->y;
        t.z = w * a1->z;
        float d = SoultreeDot(t, c->field_0x2c);
        c->field_0x74 = d;
        if (d >= 0.0f) {
            c->field_0x74 = 0.0f;
            c->field_0x68 = Vec3(0.0f, 0.0f, 0.0f);
        } else {
            c->field_0x74 = -d;
            Vec3 scaled;
            c->field_0x68 = *Vec3ScaleCall(&scaled, &c->field_0x2c, -d);
        }
        c->UnknownVirtualSlot1();
        *a1 += c->field_0x78;
        Vec3 cr = SoultreeCross(c->field_0x38, c->field_0x78);
        *a2 += field_0x08->WorldToLocalDirection(cr);
        if (c->field_0xb4 == 2) {
            *a2 *= a3;
            c->field_0xb4 = 0;
        }
    }
}

// slot 14 (0x00502080): integrates the accumulated force/torque estimates.
void SoultreePhysicsBaseObject::UnknownVirtualSlot14(Vec3* a1, const Vec3* a2,
                                                     const Vec3* a3)
{
    field_0xc0 = UnknownVirtualSlot16(a2);
    UnknownVirtualSlot15(a3, &field_0xc0);
    field_0xd8 += field_0xc0 * field_0x13c;
    field_0xd8 *= 0.999f;
    field_0x70 = *a1 * field_0x24;
    field_0x64 += field_0x70 * field_0x13c;
    field_0xbc = VecLength(field_0x64);
}

// slot 36 (0x004aa210): resolve the initial orientation vectors and reset the pose state.
void SoultreePhysicsBaseObject::UnknownVirtualSlot36()
{
    if (field_0x88.x == 0.0f && field_0x88.z == 0.0f) {
        float s = (field_0x88.y >= 0.0f) ? -1.0f : 1.0f;
        field_0x88 = s * field_0x94;
    }
    field_0x94 = g_SoultreeVec3_685190;
    field_0x88.y = 0.0f;
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    field_0x2c = 0.0f;
    field_0x30 = 0.0f;
    field_0x38 = 0.0f;
    field_0x3c = 1.0f;
    field_0xa0 = field_0x88;
    field_0xac = field_0x94;
    field_0x40 = 0.0f;
    field_0x44 = 1.0f;
    field_0x4c = 0.0f;
    field_0x48 = 0.0f;
    field_0x50 = field_0x34;
    field_0x54 = 0.0f;
    field_0x58 = 1.0f;
    field_0x60 = 1.0f;
    field_0x5c = 0.0f;
}


// Event record handed to slot 38 as a3; +0x60 is the other body (tier 1, decoded).
struct SoultreeCollisionEvent {
    char pad_0x00[0x60];
    SoultreePhysicsBaseObject* other;
};

static inline Vec3 Scale3(Vec3 v, const Vec3& s)
{
    Vec3 r;
    r.x = v.x * s.x;
    r.y = v.y * s.y;
    r.z = v.z * s.z;
    return r;
}

// slot 38 (0x00501600): collision response for one contact.  Tier 3 reading: a2 is the
// event/surface kind (0x68 = contact with another physics body, 0x66/0x6a/0x3e9 = static
// or world contact, anything else ignored).  For a body contact the relative contact
// positions and the other body's velocity/angular state are gathered and slot 4 (the impulse
// solver) is run; the other body's speed and world angular velocity are then refreshed.  For
// a static contact the lever arm r = contact - position gives torque-like field_0x1ac = r x
// (field_0xcc * s), and slot 3 (the static solver) is run with velocity + that term.  Finally
// a nearly resting body (small field_0xb8 and speed) is snapped to zero velocity.
// a2's address is passed on as a float* to slots 3/4 (retail does that).
void SoultreePhysicsBaseObject::UnknownVirtualSlot38(int a1, int a2, void* a3)
{
    Vec3 s;
    float l10;
    s = Vec3(1.0f, 1.0f, 1.0f);
    l10 = 1.0f;
    SoultreeObject* node;
    Vec3* otherVel = 0;
    SoultreePhysicsBaseObject* other = 0;
    Vec3 v48, v54, v60;
    int hasBody;

    switch (a2) {
    case 0x68:
        other = ((SoultreeCollisionEvent*)a3)->other;
        l10 = other->field_0x24;
        node = other->field_0x08;
        otherVel = &other->field_0x64;
        v54 = *(Vec3*)&field_0x128->field_0xa0 - other->field_0x18;
        v60 = other->field_0xcc;
        a3 = &other->field_0xd8;
        v48 = other->field_0xe4;
        hasBody = 1;
        break;
    case 0x66:
    case 0x6a:
    case 0x3e9:
        s = Vec3(1.0f, 1.0f, 1.0f);
        hasBody = 0;
        break;
    default:
        return;
    }

    Vec3 r = *(Vec3*)&field_0x128->field_0xa0 - field_0x18;
    if (hasBody) {
        UnknownVirtualSlot4((Vec3*)&field_0x128->field_0xac, &field_0x64, &field_0xcc, &r,
                            a2, l10, (int)node, otherVel, &v60, &v54, &v48,
                            (Vec3*)a3, (int)otherVel, (float*)&a2, 1.0f);
        if (other) {
            other->field_0x10a = 0;
            other->field_0xbc = VecLength(*otherVel);
            other->field_0xcc = other->field_0x08->LocalToWorldDirection(*(Vec3*)a3);
        }
    } else {
        Vec3 t = Scale3(field_0xcc, s);
        field_0x1ac = CrossProduct(t, r);
        Vec3 p = field_0x64 + field_0x1ac;
        UnknownVirtualSlot3((Vec3*)&field_0x128->field_0xac, &p, &r, &s, a2, 0, (float*)&a2);
    }
    if (field_0xb8 < 0.001f && field_0xbc < 0.1f) {
        field_0x64 = g_Zero;
        field_0xbc = 0.0f;
    }
}

// ---- slot 21 (0x005024f0) -------------------------------------------------------------
// Local views of objects reached through the attachment records / probe (tier 3 names,
// offsets decoded from slot 21 only).
struct SoultreeLightList {          // field_0x08->+0x1bc: count at +0x2c, items from +0x30
    char pad_0x00[0x2c];
    int count;
    struct SoultreeLight* items[1];
};
struct SoultreeLight {        // entries of the list above; tier 3: a light (kind 2 directional, 4 positional)
    char pad_0x00[0x2c];
    int kind;                      // 2 or 4 qualify as the tracked object (tier 3)
    char pad_0x30[0x34];
    Vec3 field_0x64;
    float field_0x70, field_0x74, field_0x78;
};
struct SoultreePadObject {         // attachment field_0x08 / field_0x0c targets: only +0x60 is written
    char pad_0x00[0x60];
    int field_0x60;
};
struct SoultreeSinkObject {        // attachment field_0x14 target (type 4)
    char pad_0x00[0x48];
    Vec3 field_0x48;
    Vec3 field_0x54;
    void Fn_4ba2c0(Vec3 v);    // thiscall, callee pops 0xc
    void Fn_4ba300();
    void Fn_4ba320();
    void Fn_4ba340();
    void Fn_4ba360(int a, int b, int c);
};

// The type-4 attachment target (field_0x14).  Slot 21 reloads it at every use; caching it
// in a local changes register allocation.
static inline SoultreeSinkObject* AttachmentSink(const SoultreeAttachment* a)
{
    return (SoultreeSinkObject*)a->field_0x14;
}

// Per-frame refresh of the attachment records (field_0x1d4, field_0x1dc of them).  Tier 3
// reading: (1) pick a tracked object from the node's list (kind 2 or 4); (2) on the
// slot 22 cadence, probe from the body position (+1.5 up) along the tracked object's
// velocity * -3000 (kind 2) or its own position, and when the probe result flips relative to
// field_0x1fc retint every type 1/4 attachment (0x40 vs 0xff) and toggle flag 0x800;
// (3) advance the field_0x208 cadence counter (wraps after 5); (4) reset each attachment,
// re-running the slot 18/19/20 updaters, or for type 4 moving the sink to the node-local
// point and calling its 4ba300/4ba320 depending on slots 24/25; (5) finally clear field_0x60
// of every attachment when field_0x20c was set.
void SoultreePhysicsBaseObject::UnknownVirtualSlot21()
{
    if (!field_0x1dc)
        return;
    if (!field_0x124 || !(field_0x124->field_0x25 & 1))
        return;
    if (!field_0x200) {
        SoultreeLightList* list = *(SoultreeLightList**)((char*)field_0x08 + 0x1bc);
        for (int i = 0; i < list->count; i++) {
            SoultreeLight* h = list->items[i];
            if (h->kind == 2 || h->kind == 4)
                field_0x200 = h;
        }
    }
    if (UnknownVirtualSlot22()) {
        int hit;
        Vec3 pos = UnknownVirtualSlot17();
        pos.y += 1.5f;
        SoultreeLight* h = field_0x200;
        Vec3 out;
        if (h->kind == 2) {
            Vec3 to = pos + Vec3(h->field_0x70 * -3000.0f, h->field_0x74 * -3000.0f,
                                 h->field_0x78 * -3000.0f);
            hit = field_0x1f4->Fn_506e90(&pos, &to, &out, 0, 0, 0);
        } else {
            hit = field_0x1f4->Fn_506e90(&pos, &h->field_0x64, &out, 0, 0, 0);
        }
        if (hit != (unsigned char)field_0x1fc) {
            for (int i = 0; i < field_0x1dc; i++) {
                switch (field_0x1d4[i].type) {
                case 1:
                    if (hit) {
                        field_0x1d4[i].field_0x04->Fn_4b8dd0(0x40, 0x40, 0x40);
                        field_0x1d4[i].field_0x04->field_0x74 |= 0x800;
                    } else {
                        field_0x1d4[i].field_0x04->Fn_4b8dd0(0xff, 0xff, 0xff);
                        field_0x1d4[i].field_0x04->field_0x74 &= ~0x800;
                    }
                    break;
                case 4:
                    if (hit)
                        AttachmentSink(&field_0x1d4[i])->Fn_4ba360(0x80, 0x80, 0x80);
                    else
                        AttachmentSink(&field_0x1d4[i])->Fn_4ba360(0xff, 0xff, 0xff);
                    break;
                }
            }
            field_0x1fc = (char)hit;
        }
    }
    if (++field_0x208 > 5)
        field_0x208 = 0;
    int moving = UnknownVirtualSlot23();
    int settled = UnknownVirtualSlot24();
    for (int i = 0; i < field_0x1dc; i++) {
        SoultreeAttachment* a = &field_0x1d4[i];
        a->field_0x24 = (field_0x20d || a->field_0x24) ? 1 : 0;
        switch (a->type) {
        case 1:
            a->field_0x04->field_0x60 = 0;
            if (moving)
                UnknownVirtualSlot18(a);
            break;
        case 2:
            ((SoultreePadObject*)a->field_0x08)->field_0x60 = 0;
            if (moving)
                UnknownVirtualSlot19(a);
            break;
        case 3:
            ((SoultreePadObject*)a->field_0x0c)->field_0x60 = 0;
            if (moving)
                UnknownVirtualSlot20(a);
            break;
        case 4:
            if (!field_0x20d) {
                field_0x1ac = field_0x08->LocalToWorldPoint(a->field_0x18);
                AttachmentSink(a)->Fn_4ba2c0(field_0x1ac);
                if (a->field_0x24) {
                    AttachmentSink(a)->field_0x54 = AttachmentSink(a)->field_0x48;
                    a->field_0x24 = 0;
                }
                if (settled && !field_0x20e)
                    AttachmentSink(a)->Fn_4ba300();
                else if (UnknownVirtualSlot25())
                    AttachmentSink(a)->Fn_4ba320();
            }
            break;
        }
    }
    field_0x20e = (char)settled;
    if (field_0x20c) {
        field_0x20c = 0;
        for (int i = 0; i < field_0x1dc; i++) {
            switch (field_0x1d4[i].type) {
            case 1:
                field_0x1d4[i].field_0x04->field_0x60 = 0;
                break;
            case 2:
                ((SoultreePadObject*)field_0x1d4[i].field_0x08)->field_0x60 = 0;
                break;
            case 3:
                ((SoultreePadObject*)field_0x1d4[i].field_0x0c)->field_0x60 = 0;
                break;
            case 4:
                AttachmentSink(&field_0x1d4[i])->Fn_4ba340();
                break;
            }
        }
    }
}

// ---- slot 2 (0x00500c50) --------------------------------------------------------------
// Debug allocator (core/DebugAlloc.h, 0x004a3010: size, __FILE__, line); the retail source is
// D:\aardvark\VC\krusty2\SoulTreePhysics.cpp, so the file argument is that literal.
#define SP_FILE "D:\\aardvark\\VC\\krusty2\\SoulTreePhysics.cpp"

extern int g_SoultreeInstanceCounter;      // 0x00689f14: cycles 0..5 (field_0x204 takes the old value)

// cdecl 0x004b5a60 is OrientationAnglesFromVectors (../common/Math3D.h).

// Initializer (tier 3 reading): stores the construction parameters (a3 position, a4 forward,
// a5 up, a8 mass, a9/a10 contact and attachment capacities, a16/a17 sphere radius or box
// inertia mode, ...), allocates the contact pointer array and attachment records, derives the
// inverse inertia diagonal (sphere: 1/(0.4 m r^2), otherwise a box from the node extents with
// m/12 (h^2 + d^2)), builds the collision body (a20) and orientation matrix, and optionally
// creates a child node (a2).  Returns the GameObject virtual base (`this ? vbase : 0`).
GameObject* SoultreePhysicsBaseObject::UnknownVirtualSlot2(int a1, int a2, Vec3 a3,
                                                           Vec3 a4, Vec3 a5,
                                                           void* a6, void* a7, float a8, int a9,
                                                           int a10, SoultreeSlot1f0* a11, float a12,
                                                           int a13, float a14, float a15, float a16,
                                                           int a17, int a18, unsigned char a19,
                                                           int a20)
{
    field_0x210 = a18;
    field_0x124 = (GameObject*)a7;
    field_0x1f4 = (SoultreeProbe*)a6;
    field_0x150 = a8;
    field_0x1f0 = a11;
    field_0x1e4 = a12;
    field_0x1ec = a13;
    field_0x148 = a14;
    field_0x14c = a15;
    field_0x1c4 = a17;
    field_0x160 = a16;
    field_0x10c = a3;
    field_0x118 = a4;
    field_0x20f = a19;
    if (a6)
        field_0x1f8 = ((SoultreeProbe*)a6)->field_0x40;
    field_0x1c8 = a9;
    if (a9 > 0) {
        field_0x12c = new(SP_FILE, 0x202) SoultreeContact*[a9];
        for (int i = 0; i < field_0x1c8; i++)
            field_0x12c[i] = 0;
    }
    field_0x1d8 = a10;
    if (a10 > 0)
        field_0x1d4 = new(SP_FILE, 0x20d) SoultreeAttachment[a10];
    field_0x204 = g_SoultreeInstanceCounter;
    field_0x208 = 0;
    if (++g_SoultreeInstanceCounter > 5)
        g_SoultreeInstanceCounter = 0;
    field_0x109 = 0;
    field_0x1e0 = 0.0f;
    UnknownVirtualSlot1(0.0f);
    field_0x08->SetPosition(0.0f, 0.0f, 0.0f);
    field_0x08->Fn_004fbd10(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
    field_0x1a0 = g_Zero;
    field_0x194 = g_Zero;

    Vec3 extents;
    Vec3 center;
    field_0x08->Fn_004fe850(&center, &extents);
    float mass = field_0x158;
    Vec3* inertia = &field_0xf0;   // always non-null; the test only steers codegen
    float ident[9];
    memset(ident, 0, sizeof(ident));
    ident[0] = ident[4] = ident[8] = 1.0f;
    // Inverse inertia diagonal (tier 3 names; the formulas are decoded).  Sphere mode
    // (a17 == 1): I = 0.4 * m * r^2 (solid sphere), stored as 1/I on all three axes.  Otherwise a
    // solid box of full sizes (sx,sy,sz) = 2 * extents: I_x = m/12 * (sy^2 + sz^2), etc.
    if (inertia) {
        if (a17 == 1) {
            float inv = 1.0f / ((mass * 0.4f) * (a16 * a16));
            inertia->x = inv;
            inertia->y = inv;
            inertia->z = inv;
        } else {
            float sx = extents.x + extents.x;
            float sy = extents.y + extents.y;
            float sz = extents.z + extents.z;
            float zz = sz * sz;
            float yy = sy * sy;
            float k = mass * (1.0f / 12.0f);
            inertia->x = 1.0f / ((yy + zz) * k);
            float xx = sx * sx;
            inertia->y = 1.0f / ((xx + zz) * k);
            inertia->z = 1.0f / ((xx + yy) * k);
        }
    }
    memcpy(field_0x164, ident, sizeof(ident));
    field_0xe4 = field_0xf0;

    int prevTag = g_MemTagStack->Push("Collision");
    if (a20) {
        field_0x128 = new(SP_FILE, 0x245) CollisionObject(1);
        Fn_501230();
        field_0x128->Fn_004320f0(a1, 0, 1, 1);
    } else {
        field_0x128 = 0;
    }
    g_MemTagStack->Pop(prevTag);

    field_0x88 = a4;
    field_0x94 = a5;
    field_0x08->SetPosition(a3.x, a3.y, a3.z);
    field_0x08->GetPosition(&field_0x0c);
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c, &field_0x38,
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
    if (a2) {
        field_0x218 = new(SP_FILE, 0x266) SoultreeObject(1);
        field_0x08->AddChild(field_0x218);
        field_0x218->SetPosition(center);
        field_0x218->GetPositionIn(0, &field_0x18);
    } else {
        field_0x08->GetPositionIn(0, &field_0x18);
    }
    field_0x20d = 1;
    field_0x109 = 0;
    field_0x20c = 1;
    return this;
}

// ==== wave 3 (fork B): constructor, destructor, collision callbacks, frame step ====

// ---- collision callbacks (0x00500c00, 0x00500c30) ----------------------------------------
// Installed into the CollisionObject at field_0x128 by Fn_501230 (+0x88 / +0x8c).  The
// collision object's field_0x60 is its owner (this body) and the other object's field_0x64
// its type tag (0x68 = another physics body).  The tag is recorded in field_0x134 and
// forwarded to slot 38 together with the other collision object (tier 1 data flow).
static void SoultreeCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    SoultreePhysicsBaseObject* body = (SoultreePhysicsBaseObject*)self->field_0x60;
    int kind = other->field_0x64;
    body->field_0x134 = kind;
    body->UnknownVirtualSlot38(0, kind, other);
}

// Second callback: the same response, except for body-to-body contacts (tag 0x68).
static void SoultreeStaticCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    if (other->field_0x64 != 0x68)
        SoultreeCollisionCallback(self, other);
}

// 0x00501230
void SoultreePhysicsBaseObject::Fn_501230()
{
    field_0x128->field_0x88 = SoultreeCollisionCallback;
    field_0x128->field_0x8c = SoultreeStaticCollisionCallback;
}

// ---- constructor (0x00500aa0) / destructor (0x00501260) ---------------------------------
// Defining them here also makes VC6 emit the vbase deleting destructor 0x00504290, its
// vtordisp thunk 0x00504280 and the GameObject slot 10 vtordisp thunk 0x005042d0.
SoultreePhysicsBaseObject::SoultreePhysicsBaseObject(int flags)
    : GameObject(flags)
{
    field_0x210 = 1;
    field_0x10a = 0;
    field_0x28 = 1.0f;
    field_0x15c = 0.0f;
    field_0x188 = g_Zero;
    field_0x1f8 = 1.0f;
    field_0x1f4 = 0;
    field_0x124 = 0;
    field_0x1fc = 0;
    field_0x200 = 0;
    field_0x20c = 0;
    field_0x218 = 0;
    field_0x1d4 = 0;
    field_0x1dc = 0;
    field_0x1d8 = 0;
    field_0x13c = 0.025f;          // default step 1/40 s ...
    field_0x140 = 40.0f;           // ... and its reciprocal
    field_0x144 = 0.0f;
    field_0x1e8 = 0.025f;
    field_0x1f0 = 0;
    field_0x12c = 0;
    field_0x1c8 = 0;
    field_0x130 = 0;
    field_0x1cc = 0;
    field_0x20e = 0;
    field_0x20d = 0;
    field_0x20f = 0;
    UnknownVirtualSlot1(0.0f);
}

SoultreePhysicsBaseObject::~SoultreePhysicsBaseObject()
{
    if (field_0x12c) {
        for (int i = 0; i < field_0x1c8; i++) {
            if (field_0x12c[i] && field_0x12c[i]->field_0x04)
                delete field_0x12c[i];
        }
        delete field_0x12c;
        field_0x12c = 0;
    }
    if (field_0x1d4) {
        delete field_0x1d4;
        field_0x1d4 = 0;
    }
}

// ---- rest / settle check (0x00502c40) ------------------------------------------------------
// Called once per frame by Fn_502f60.  Tier 3 reading: when the body is slow (speed
// field_0xbc < 2) and its angular velocity field_0xd8 is small on every axis, with at least
// field_0x210 - 1 active contacts (field_0x1cc), the timer field_0x214 runs; after 0.5 s,
// on more than one contact or on a single near-flat one (normal y > 0.95), the body is put
// to sleep: velocities zeroed and field_0x10a set (GameObject slot 10 then skips the step).
// A body slower than 5 with enough contacts is damped instead (angular 0.95 per frame, and
// after 3 s angular 0.5 and linear 0.95).
static inline float AbsF(float v)
{
    if (v < 0.0f)
        v = -v;
    return v;
}

void SoultreePhysicsBaseObject::Fn_502c40()
{
    int spinSlow;
    if (field_0xbc < 5.0f && AbsF(field_0xd8.y) < 0.98f && AbsF(field_0xd8.x) < 0.98f &&
        AbsF(field_0xd8.z) < 0.98f)
        spinSlow = 1;
    else
        spinSlow = 0;
    if (field_0xbc < 2.0f && spinSlow && !field_0x108) {
        if (field_0x1cc < field_0x210 && field_0x214 <= 0.5f) {
            if (field_0x1cc >= field_0x210 - 1)
                field_0x214 += field_0x13c;
            return;
        }
        if (field_0x130 > 1 || field_0x12c[0]->field_0x2c.y > 0.95f) {
            field_0x64 = g_Zero;
            field_0xbc = 0.0f;
            field_0xb8 = 0.0f;
            field_0xd8 = g_Zero;
            field_0xcc = g_Zero;
            field_0x214 = 0.0f;
            field_0x10a = 1;
        }
        return;
    }
    if (field_0xbc > 5.0f) {
        field_0x214 = 0.0f;
        return;
    }
    if (field_0x1cc < field_0x210 - 1)
        return;
    field_0x214 += field_0x13c;
    if (field_0x214 > 3.0f) {
        field_0xd8 *= 0.5f;
        field_0x64 *= 0.95f;
    } else {
        field_0xd8 *= 0.95f;
    }
}

// ---- GameObject slot 10 override (0x005036f0, via vtordisp thunk 0x005042d0) ---------------
// 0x0043ad80 (cdecl, collision/CollisionContactUpdate.cpp): refreshes every contact point
// from its owner node, counts the penetrating ones into *activeCount and returns nonzero
// when it ran (first argument zero: nothing done).  Parameter names tier 3.
int SoultreeRefreshContacts(int enabled, int* activeCount, int count, SoultreeContact** contacts,
                            SoultreeProbe* terrain, Vec3* center, int mode, float radius);

// Per-frame update: skipped while asleep (field_0x10a); otherwise slot 9 splits dt into
// fixed steps, slot 30 clears the contact flags, slot 39 handles a pending reset, the
// contacts are refreshed (field_0x1d0 = any contact active), Fn_502f60 runs the steps and
// slot 21 refreshes the attachments.  Tier 2 for the call structure.
int SoultreePhysicsBaseObject::GameObjectVirtualSlot10(float dt)
{
    int steps;
    int refreshed = 0;
    field_0x20d = 0;
    if (field_0x10a)
        return 1;
    UnknownVirtualSlot9(dt, &steps);
    UnknownVirtualSlot30();
    field_0x1d0 = 0;
    int held = UnknownVirtualSlot39(dt);
    if (!held) {
        refreshed = SoultreeRefreshContacts(UnknownVirtualSlot5(1), &field_0x1cc, field_0x130,
                                            field_0x12c, field_0x1f4, &field_0x18, field_0x1c4,
                                            field_0x160);
        if (refreshed)
            field_0x1d0 = field_0x1cc > 0;
    }
    Fn_502f60(steps, held, refreshed);
    UnknownVirtualSlot21();
    return 1;
}

// 0x00500220 (cdecl, called by slot 3): impulse response of one contact.  Tier 3 reading
// (decoded arithmetic, tier 1): normal = contact normal, relVel = relative contact
// velocity, arm = contact point relative to the centre of mass.  When the contact is
// approaching (relVel . normal < 0) the impulse j = -(1 + restitution) * approach /
// (invMass + n . ((I^-1 (arm x n)) x arm)) is written to *impulseOut, the linear velocity
// gains n * j * invMass (j scaled first when the scale argument is non-zero), and the
// body-space angular velocity gains I^-1 (arm x n) * j, scaled per axis by *angScale.
// Slot 3 passes field_0x14c, field_0x24, field_0x08, &field_0xe4 (body-space inverse
// inertia diagonal), &field_0xd8 and &field_0x64.
void Fn_500220(float restitution, float invMass, SoultreeObject* node, const Vec3* normal,
               const Vec3* relVel, const Vec3* arm, Vec3* invInertia, const Vec3* angScale,
               Vec3* angVel, Vec3* velocity, float* impulseOut, int scaleBits)
{
    // The last argument holds a float (fcomp/fmul as a single); slot 3 forwards it as
    // its int a6, so it is reinterpreted here.
    float scale = *(float*)&scaleBits;
    float approach = SoultreeDot(*relVel, *normal);
    if (approach < 0.0) {
        Vec3 axis = CrossProduct(*arm, *normal);
        Vec3 local = node->WorldToLocalDirection(axis);
        Vec3 scaled(local.x * invInertia->x, local.y * invInertia->y, local.z * invInertia->z);
        Vec3 world = node->LocalToWorldDirection(scaled);
        Vec3 c = CrossProduct(world, *arm);
        float j = ((restitution + 1.0f) * -approach) / (DotProduct(c, *normal) + invMass);
        *impulseOut = j;
        Vec3 impulse;
        if (scale != 0.0f)
            impulse = *normal * (j * scale);
        else
            impulse = *normal * j;
        *velocity += impulse * invMass;
        Vec3 spin = *impulseOut * scaled;
        spin.x *= angScale->x;
        spin.y *= angScale->y;
        spin.z *= angScale->z;
        *angVel += spin;
    }
}
