// KrustyBike.cpp -- reconstruction of the KrustyBike overrides.
// Translation unit: KrustyBike.cpp (literal __FILE__ xrefs near 0x0048FE58; tier 2).
// Member and helper names are provisional (tier 3); see KrustyBikeTypes.h.
#include <math.h>
#include <stdlib.h>
#include "KrustyBike.h"

// KbFloat: identity inline standing in for the original inline float getters (tier 3).
// Passing a member straight to a float parameter makes VC6 push the raw dword; retail
// instead loads the value on the FPU (fld / fstp [esp]), which is what a float-returning
// accessor produces.  Used at two call sites only.
static inline float KbFloat(float v) { return v; }

// Provisional: rand() scaled to [0,1) (1/32768); kept as a float-returning inline so the
// later scale factor is not constant-folded into it (a plain return folds; the local does not).
static inline float KbRandUnit() { float r = rand() * (1.0f / 32768.0f); return r; }

// Provisional: square root with an exact-one shortcut (slot 14 tail; retail returns the
// pooled 1.0f through the FPU, which a plain member store of 1.0f does not reproduce).
static inline float KbLength(const Vec3& v)
{
    float d = v.x * v.x;   // accumulated term by term: a one-line sum loads in a different order
    d += v.y * v.y;
    d += v.z * v.z;
    if (d == 1.0f)
        return 1.0f;
    return (float)sqrt(d);
}

// Destructor body (0x00491540, reached through the vbase-adjusted scalar deleting
// destructor 0x00497c40): releases two debug-heap buffers (__FILE__ lines 0x8bb, 0x8be).
KrustyBike::~KrustyBike()
{
    if (field_0x77c)
        operator delete(field_0x77c, __FILE__, 0x8bb);
    if (field_0x780)
        operator delete(field_0x780, __FILE__, 0x8be);
}

void KrustyBike::UnknownVirtualSlot27()
{
    if (field_0x460 == 9 || field_0x430) {
        field_0x604->Method_0x00532310();
    } else {
        field_0x604->Method_0x00532220(field_0x448);
    }
    UnknownVirtualSlot101();
}

void KrustyBike::UnknownVirtualSlot1(float a)
{
    Bike::UnknownVirtualSlot1(a);
    Fn_0048D8B0();
    field_0x152c = 0;
    field_0x154d = 0;
    field_0x154e = 0;
    field_0x155c = 0;
    field_0x141c = 0;
    field_0x76c = 0;
    field_0x153e = 0;
    field_0x814 = 0;
}

void KrustyBike::UnknownVirtualSlot3(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, int a5, float* a6)
{
    if (a4 == 1000) {
        field_0x740->handler->Fn_004DE580(this, 0, a4);
        return;
    }
    Bike::UnknownVirtualSlot3(a0, a1, a2, a3, a4, a5, a6);
    if (a4 != 0x67)
        field_0x740->handler->Fn_004DE580(this, *a6, a4);
}

void KrustyBike::UnknownVirtualSlot4(const Vec3* a0, Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5, int a6,
                                     const Vec3* a7, const Vec3* a8, const Vec3* a9,
                                     Vec3* a10, Vec3* a11, int a12, float* a13, float a14)
{
    Bike::UnknownVirtualSlot4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    field_0x740->handler->Fn_004DE580(this, *a13, a4);
}

void KrustyBike::UnknownVirtualSlot8()
{
    Bike::UnknownVirtualSlot8();
    if (field_0x734 && field_0x700)
        field_0x700 = 0;
}

int KrustyBike::UnknownVirtualSlot12(int a)
{
    if (a && g_kbGame->field_0x2d74 != 3) {
        float lo = field_0x1f8 * 105.0f;
        if (field_0x0c.x < lo)
            return 1;
        KbTrackB* t = ((KbTrackA*)field_0x1f0)->field_0xa4;
        if ((t->field_0x394 * 256.0f - 105.0f) * field_0x1f8 < field_0x0c.x)
            return 2;
        if (field_0x0c.z < lo)
            return 3;
        if ((t->field_0x398 * 256.0f - 105.0f) * field_0x1f8 < field_0x0c.z)
            return 4;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot22()
{
    if (field_0x208 == field_0x204 || field_0x740->field_0x50->field_0x3b4 == this)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot23()
{
    if (!field_0x108 && field_0x434 >= 0.2f && field_0x740->field_0x50->field_0x244 != 3)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot24()
{
    if (field_0x735) {
        if (field_0xb8 < field_0xbc && !field_0x444 && field_0xbc < 22.0f)
            return 1;
        return 0;
    }
    if (field_0x5b0 < field_0x480->field_0x00 && !field_0x444 && field_0xbc < 22.0f)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot25()
{
    if (field_0x735) {
        if (field_0xb8 > field_0xbc && !field_0x444 && field_0xbc < 22.0f)
            return 1;
    } else {
        if (field_0x5b0 < field_0x480->field_0x00 && !field_0x444 && field_0xbc < 22.0f)
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot33(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5)
{
    int result = Bike::UnknownVirtualSlot33(a0, a1, a2, a3, a4, a5);
    field_0x1540 = field_0x0c;
    field_0x604->a_0x52c = 1;
    return result;
}

int KrustyBike::UnknownVirtualSlot39(float a)
{
    if (!field_0x109 && g_kbGame->field_0x2d70 && field_0x76c > 6.0f) {
        if (field_0x444)
            UnknownVirtualSlot67();
        return 1;
    }
    return Bike::UnknownVirtualSlot39(a);
}

int KrustyBike::UnknownVirtualSlot42()
{
    if (!field_0x444 && field_0x433 >= 0 && field_0x430 && field_0x5c4->c_0xc) {
        float sum = field_0x1528 + field_0x1520;
        field_0x1520 = 0;
        field_0x153d++;
        field_0x1528 = 0;
        sum += field_0x1524;
        field_0x1524 = 0;
        field_0x1530 += sum * 2500.0f;
        field_0x1538 = (float)field_0x153d;
        field_0x758 += sum;
        return 1;
    }
    return 0;
}

void KrustyBike::UnknownVirtualSlot44()
{
    Bike::UnknownVirtualSlot44();
}

float KrustyBike::UnknownVirtualSlot45()
{
    KbRacer* r = g_kbGame->field_0x568;
    if (r && r->field_0xa8 == this)
        return 0.0f;
    return 3.0f;
}

int KrustyBike::UnknownVirtualSlot52()
{
    if (field_0x444 || field_0x735)
        return 0;
    return 1;
}

void KrustyBike::UnknownVirtualSlot63(float a)
{
    if (field_0x734) {
        Fn_00414370(a);
        return;
    }
    field_0x804 = 0;
    field_0x808 = 0;
    Bike::UnknownVirtualSlot63(a);
}

void KrustyBike::UnknownVirtualSlot64(float a)
{
    if (field_0x444) {
        field_0x804 = 0;
        field_0x808 = 0;
    }
    Bike::UnknownVirtualSlot64(a);
}

int KrustyBike::UnknownVirtualSlot66()
{
    if (field_0x444 || field_0x430)
        return 1;
    return field_0x7a4 != 0 && field_0x7a4 < 3;
}

int KrustyBike::UnknownVirtualSlot68(int* out)
{
    *out = 15;
    if (g_kbGame->field_0x2d70 && !field_0x7a4)
        return 0;
    return 1;
}

void KrustyBike::UnknownVirtualSlot69()
{
    if (field_0x448 == 2) {
        field_0x454 = field_0x7a8;
        return;
    }
    float t = field_0x458;
    field_0x454 = t;
    if (!g_kbGame->field_0x2d70 && !g_kbGame->field_0x2d74)
        field_0x454 = t + 1.0f;
}

void KrustyBike::UnknownVirtualSlot71(int a)
{
    if (field_0x444) {
        field_0x141c = 0;
    } else if (field_0x108 && !a && !field_0x59c && field_0x431 && field_0x141c <= 0.0f) {
        field_0x141c = 0.3f;
    }
    Bike::UnknownVirtualSlot71(a);
}

Vec3 KrustyBike::UnknownVirtualSlot76(const Vec3* a, const Vec3* b)
{
    if (field_0x734)
        return g_kbZeroVec;
    return Bike::UnknownVirtualSlot76(a, b);
}

int KrustyBike::UnknownVirtualSlot80()
{
    if (!field_0x740->field_0x18a && !field_0x479)
        return 0;
    if (field_0x804 > 0.0f || (field_0x46c && field_0x470 > 0.33f) ||
        (UnknownVirtualSlot84(4, 0x3f) && !UnknownVirtualSlot77(field_0x480->field_0x00 == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot81()
{
    if (!field_0x734 && !field_0x735 && !field_0x736 && UnknownVirtualSlot84(12, 0x3f)) {
        if (!UnknownVirtualSlot77(!field_0x479))
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot82()
{
    if (!field_0x734 && !field_0x735 && !field_0x736) {
        if (field_0x434 < 6.0f && UnknownVirtualSlot84(13, 0x3f)) {
            if (!UnknownVirtualSlot77(field_0x480->field_0x00 == 0.0f))
                return 1;
        }
        field_0x664 = 0;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot83(VehicleWheel* wheel)
{
    BikeWheel* input = (BikeWheel*)wheel;
    if (field_0x808 > 0.0f ||
        (UnknownVirtualSlot84(5, 0x3f) && !UnknownVirtualSlot77(input->w_0x29c == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot84(int a, int b)
{
    if (!field_0x734 && !field_0x735 && !field_0x736) {
        KbSensor* s = field_0x740->field_0x19c;
        if (!s || !s->field_0x3dc) {
            Bike::UnknownVirtualSlot84(a, b);
            return field_0x468->UnknownVirtualSlot2(a, b);
        }
    }
    return 0;
}

// 0x004925A0: tell the other peers (and the recorder) that this bike was hit by `who`.
void KrustyBike::Fn_004925A0(Vehicle* who, bool flag)
{
    if (g_kbGame->field_0x8 && !field_0x735) {
        KbNetPacket pkt;
        pkt.field_0x8 = who ? ((KbNetBike*)who)->field_0x11bc : 0;
        pkt.field_0x4 = flag;
        g_kbGame->field_0x8->Fn_004AC830((unsigned char)(flag ? 0x13 : 0x87), &pkt, 12, ((KbNetBike*)this)->field_0x11bc, 0);
        if (!flag && field_0x13fc && g_kbGame->field_0x3334 && !g_kbGame->field_0x3428)
            field_0x13fc->Fn_004E8720(0x87, who ? ((KbNetBike*)who)->field_0x11bc : 0, &pkt, 1);
    }
}

// 0x00495C00: true when the two cells referenced at +0x11c8/+0x11cc (both fully set) differ in
// their integer x or z (tier 3 semantics).
int KrustyBike::Fn_00495C00()
{
    if (field_0x735) {
        KbCell* a = ((KbNetBike*)this)->field_0x11c8;
        if (a->field_0x8 != 0 && a->field_0x10 != 0) {
            KbCell* b = ((KbNetBike*)this)->field_0x11cc;
            if (b->field_0x8 != 0 && b->field_0x10 != 0) {
                if ((int)a->field_0x8 != (int)b->field_0x8 || (int)a->field_0x10 != (int)b->field_0x10)
                    return 1;
            }
        }
    }
    return 0;
}

void KrustyBike::Fn_00496E20(KbRecorder* a)
{
    field_0x13fc = a;
}

// ---- non-virtual state reset (0x0048D8B0) ----
void KrustyBike::Fn_0048D8B0()
{
    field_0x1538 = 1.0f;
    field_0x1530 = 0;
    field_0x1520 = 0;
    field_0x1528 = 0;
    field_0x1524 = 0;
    field_0x59c = 0;
    field_0x431 = 0;
    field_0x141c = 0;
    field_0x153d = 0;
    field_0x1534 = 0;
    field_0x153c = 0;
    field_0x153f = 0;
    field_0x15e5 = 0;
}

void KrustyBike::UnknownVirtualSlot67()
{
    Fn_00496DA0();
    for (int i = 0; i < field_0x130; i++) {
        KbChild* c = ((KbChild**)field_0x12c)[i];
        if (c->field_0x4)
            c->field_0xa4 = 0;
    }
    field_0x444 = 0;
    field_0x454 = 0;
    field_0x5b4 = 0;
    field_0x608 = 0;
    field_0x604->Method_0x005327c0();
    field_0x61c = g_kbZeroVec;
}

// Back-camera swap: like slot 101 with the two scene objects exchanged.
void KrustyBike::Fn_00496DA0()
{
    field_0x1418->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Fn_004DCF20(field_0x1418, field_0x1418->field_0x84);
    field_0x128 = (CollisionObject*)field_0x1414;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)field_0x128)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)field_0x128)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)field_0x128)->Fn_00435FE0();
}

// Retail begins with a 16-byte 'jmp +11' followed by 11 nops before the real prologue;
// that patch-point padding is not reproducible from C++ (no inline asm), so this stays partial.
void KrustyBike::UnknownVirtualSlot101()
{
    field_0x1414->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Fn_004DCF20(field_0x1414, field_0x1414->field_0x84);
    field_0x128 = (CollisionObject*)field_0x1418;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)field_0x128)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)field_0x128)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)field_0x128)->Fn_00435FE0();
}

// ---- physics ----

// Slot 16: per-axis scale of v by one of two Vec3 tables (0xe4 or 0xf0), chosen by field_0x444.
Vec3 KrustyBike::UnknownVirtualSlot16(const Vec3* v)
{
    Vec3 r;
    if (!field_0x444) {
        r.x = field_0xe4.x * v->x;
        r.y = field_0xe4.y * v->y;
        r.z = field_0xe4.z * v->z;
    } else {
        r.x = field_0xf0.x * v->x;
        r.y = field_0xf0.y * v->y;
        r.z = field_0xf0.z * v->z;
    }
    return r;
}

// Slot 15: transform v by the matrix at +0x3bc, scale per axis as slot 16, then hand to 0x4CB6E0
// (which looks like a clamp/settle step on the accumulator at +0xd8).
void KrustyBike::UnknownVirtualSlot15(const Vec3* v, Vec3* out)
{
    Vec3 s;
    Vec3 t = ((KbXform*)d3d_field_0x1a0)->Fn_004FD710(v);
    if (!field_0x444) {
        s.x = t.x * field_0xe4.x;
        s.y = t.y * field_0xe4.y;
        s.z = t.z * field_0xe4.z;
    } else {
        s.x = t.x * field_0xf0.x;
        s.y = t.y * field_0xf0.y;
        s.z = t.z * field_0xf0.z;
    }
    UnknownAxisSettle_4cb6e0(&field_0xd8.x, &out->x, &s.x, field_0x13c, 0);
}

// Slot 14: apply an impulse. field_0x13c behaves as an inverse mass (dv = k * impulse).
void KrustyBike::UnknownVirtualSlot14(Vec3* a, const Vec3* b, const Vec3* c)
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot14(a, b, c);
        return;
    }
    field_0xc0 = UnknownVirtualSlot16(b);
    UnknownVirtualSlot15(c, &field_0xc0);
    field_0xd8 += field_0x13c * field_0xc0;
    field_0xd8 *= 0.999f;
    a->x = 0;
    a->z = 0;
    field_0x70 = Vec3(0.0f, field_0x24 * a->y, 0.0f);
    field_0x64 += field_0x13c * field_0x70;
    field_0xbc = KbLength(field_0x64);
}

// Slot 48: field_0x0c update from the (field_0x64*3 - field_0x7c) * 0.5 * dt step.
void KrustyBike::UnknownVirtualSlot48()
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot48();
        return;
    }
    Vec3 v = field_0x64 * 3.0f;
    Vec3 d = (v - field_0x7c) * 0.5f;
    field_0x0c += d * field_0x13c;
    field_0x5f4->w_0x248 = g_kbZeroVec;
    field_0x5f4->w_0x280 = 0;
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
int KrustyBike::UnknownVirtualSlot70(float arg)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (field_0x734) {
        float speedGain = field_0xbc - field_0xb8;
        if (speedGain * b[g_kbGame->field_0x2d74 != 3][g_kbGame->field_0x60c - 1] < arg * field_0x450)
            return 1;
        return 0;
    }
    if (field_0xbc - field_0xb8 < a[g_kbGame->field_0x60c - 1] * field_0x450 * arg)
        return 1;
    return 0;
}

// Slot 86: per-frame wheel-contact update (provisional semantics).  When the race context
// flag (field_0x740+0x18a) is set, a lookup ramp derived from field_0x664 (0.12..0.4762,
// scaled by 1010.668) feeds the wheel's helper object, and in one game mode the wheel's
// two force accumulators are halved; the wheel is then always updated.
void KrustyBike::UnknownVirtualSlot86()
{
    BikeWheel* w = field_0x5f4;
    if (!w->w_0x260)
        return;
    if (field_0x740->field_0x18a) {
        float ramp;
        if (field_0x47a && field_0x664 >= 0.12f && field_0x664 < 0.4762f)
            ramp = (0.4762f - field_0x664) * 1010.668f;
        else
            ramp = 0;
        if (w->w_0x2a8)
            w->w_0x2a8->Fn_004D31B0(KbFloat(field_0xbc), &w->w_0x230, KbFloat(field_0x4a4), field_0x47a, ramp,
                                    &w->w_0x248, &w->w_0x280);
        if (g_kbGame->field_0x2d70 && !field_0x78c) {
            field_0x5f4->w_0x248 *= 0.5f;
            field_0x5f4->w_0x280 *= 0.5f;
        }
    }
    field_0x5f4->Fn_00513F90(this);
}

// Slot 96: tuning constants derived from the game-settings integers (0xfe0..0xff8).
void KrustyBike::UnknownVirtualSlot96()
{
    if (field_0x734)
        field_0x524[0] = 0.675f;
    else
        {
            float pct = g_kbGame->field_0xfe0 * 0.01f;
            field_0x524[0] = pct * 0.2f + 0.575f;
        }
    if (field_0x734)
        field_0x63c = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfe4 * 0.01f;
            field_0x63c = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x524[1] = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfe8 * 0.01f;
            field_0x524[1] = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x524[2] = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfec * 0.01f;
            field_0x524[2] = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x524[3] = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff4 * 0.01f;
            field_0x524[3] = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x524[4] = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff0 * 0.01f;
            field_0x524[4] = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x524[5] = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff8 * 0.01f;
            field_0x524[5] = pct * 0.4f + 0.8f;
        }
}

// Slot 28 (0x0048da50): per-frame hook.  In mode 4 (tier 3: a spectator/replay mode) it
// re-targets the session's player record onto this bike, then runs the base update and,
// while airborne (field_0x444 set, field_0x5b4 clear), jitters the two 0x7ac/0x7b0
// accumulators with random noise (0.15 scale for the field_0x734 variant, 0.667 otherwise).
int KrustyBike::UnknownVirtualSlot28(int a)
{
    field_0x134 = 0;
    if (field_0x736) return a;
    if (g_kbGame->field_0x2d74 == 4) {
        KbPlayer* p = g_kbGame->field_0x570->Fn_0045D2B0();
        if (this != p->field_0xa8 && this == field_0x740->field_0x38) {
            Vehicle* v = p->field_0xa8;
            if (v) {
                if (((KbBody*)field_0x128)->Fn_004392C0((KbBody*)v->field_0x128))
                    Fn_004925A0(this, 1);
            } else if (p->field_0xdc) {
                if (((KbBody*)field_0x128)->Fn_004392C0((KbBody*)p->field_0xdc->field_0x128)) {
                    if (g_kbGame->field_0x8->field_0x10) p->Fn_004A9E80(this, 0, 1);
                    else Fn_004925A0(this, 1);
                }
            }
        }
        if (field_0x735) return a;
    }
    int r = Vehicle::UnknownVirtualSlot28(a);
    if (field_0x444 && !field_0x5b4) {
        if (field_0x734) {
            field_0x7ac += KbRandUnit() * 0.15f;
            field_0x7b0 += KbRandUnit() * 0.15f;
        } else {
            field_0x7ac += KbRandUnit() * 0.667f;
            field_0x7b0 += KbRandUnit() * 0.667f;
        }
    }
    return r;
}

// Slot 99 (0x004916a0): tier 3 -- out-of-bounds / off-track recovery.  When slot 12 reports
// the bike outside the track box it aims field_0x1ac at the track centre (20 units up),
// sets velocity to a 0.055 fraction of the offset, and gives a small random kick to
// field_0xd8; otherwise it may enter recovery states 2 (both wheels loaded past -3.5)
// or 1/3/4/5/9, and falls through to the base handler when nothing applies.
int KrustyBike::UnknownVirtualSlot99(float a, float b, int c, float d)
{
    if (field_0x444) return field_0x444;
    if (UnknownVirtualSlot12(field_0x5a4)) {
        KbTrackB* t = ((KbTrackA*)field_0x1f0)->field_0xa4;
        field_0x1ac = Vec3(t->field_0x394 * field_0x1f8 * 128.0f, 20.0f,
                             t->field_0x398 * field_0x1f8 * 128.0f);
        Vec3 off = field_0x1ac - field_0x0c;
        field_0x64 = off * 0.055f;
        field_0x64.y = 120.0f;
        field_0xb8 = field_0xbc = KbLength(field_0x64);
        float rx = rand() * (1.0f / 32768.0f);
        float ry = rand() * (1.0f / 32768.0f);
        field_0xd8 = Vec3(rx, ry, 0.02f);
        field_0x448 = 1;
        field_0x460 = 12;
        return 1;
    }
    if (!field_0x430 && field_0x468 && field_0x468->UnknownVirtualSlot2(8, 0x3f)) {
        if (field_0x108 && !field_0x444 && field_0x5f0->w_0x150 < -3.5f && field_0x5f4->w_0x150 < -3.5f) {
            field_0x448 = 2;
            field_0x460 = 1;
            return 1;
        }
    }
    if (!field_0x108 && field_0x430) {
        if (field_0x5f0->w_0x260) {
            if (field_0xbc > 30.0f) field_0x448 = 1;
            else if (field_0xd8.z > 0.0f) field_0x448 = 4;
            else field_0x448 = 5;
        } else field_0x448 = 3;
        field_0x460 = 9;
        Fn_0048D8B0();
        return 1;
    }
    if (UnknownVirtualSlot51()) return 0;
    return Bike::UnknownVirtualSlot99(a, b, c, d);
}

// Slot 11 (0x00491d10): tier 3 -- camera / target ray query.  Fills a2 (origin) and a3
// (unit direction) according to the game mode (g->0x2d74: 0..5), then issues the same
// collision query as the base class with per-mode ranges.  The two "copy the stored
// vectors" paths fall off the end without setting a result (retail leaves eax as is).
int KrustyBike::UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3, Vec3* a4, int* a5)
{
    int flag = 0;
    if (g_kbGame->field_0x578.Fn_00524100() == 3 || g_kbGame->field_0x578.Fn_00524100() == 4)
        flag = 1;
    int lim = 0x66;
    if (g_kbGame->field_0x578.Fn_00524100() == 5 || g_kbGame->field_0x578.Fn_00524100() == 4)
        lim = 0x7fffffff;
    unsigned char hit = 0;
    if (g_kbGame->field_0x578.Fn_00524100() == 5)
        hit = Kb_004B0AC0(&field_0x0c, 2.0f, 2.0f, 2.0f, field_0x1f8, 0, 0, 0, 0);
    KbGame* g = g_kbGame;
    switch (g->field_0x2d74) {
    case 2:
    case 3:
        if (g->field_0x2d70 || field_0x78c || field_0x734 || field_0x736 || a1 == 2 ||
            g->field_0xc50) {
            *a2 = field_0x10c;
            *a3 = field_0x118;
            field_0x0c = field_0x10c;
        } else {
            Vec3 v = field_0x10c;
            *a2 = field_0x0c;
            *a3 = Vec3(v.x - a2->x, -a2->y, v.z - a2->z);
            float d = a3->x * a3->x;
            d += a3->y * a3->y;
            d += a3->z * a3->z;
            if (d == 0.0f) *a3 = g_kbZeroVec;
            else *a3 *= FastInvSqrt(d);
            return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, flag, 0x64,
                               0x65, hit, 3.0f, 8, 0, &field_0x10c, !field_0x109, a2, a3, a4, a5);
        }
    case 0: {
        const Vec3* p = field_0x109 ? &field_0x118 : &field_0x88;
        return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, flag, lim, lim,
                           hit, 6.0f, 8, p, 0, !field_0x109, a2, a3, a4, a5);
    }
    case 4: {
        KbPlayer* r = (KbPlayer*)g->field_0x568;
        const Vec3* l = 0;
        const Vec3* k = 0;
        if (r->field_0xdc) l = &r->field_0xdc->field_0x0c;
        else if (r->field_0xa8 == this) { *a3 = field_0x88; k = &field_0x88; }
        else l = &r->field_0xa8->field_0x0c;
        float i = 9.0f;
        if (((KbPlayer*)g->field_0x568)->field_0xa8 != this) i = 3.0f;
        return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, flag, lim, lim,
                           hit, i, 8, k, l, !field_0x109, a2, a3, a4, a5);
    }
    case 1:
    case 5: {
        if (field_0x109) field_0x1ac = field_0x118 + field_0x10c;
        else field_0x1ac = g->field_0x560[field_0x7b8 + 9].field_0x00;
        return Fn_4b0df0(field_0x128, field_0x1f4, &field_0x0c, field_0x1f8, flag, lim,
                           field_0x109 ? 0x64 : 0x7fffffff, field_0x109 ? 0x65 : 0x7fffffff,
                           3.0f, 8, 0, &field_0x1ac, !field_0x109, a2, a3, a4, a5);
    }
    default:
        *a2 = field_0x0c;
        *a3 = field_0x88;
    }
}

// Slot 93: when the pad device (input map +0x0c) is in mode 3, reset one effect channel and
// re-arm another with a time-scaled value, then derive field_0x51c from the same game value.
void KrustyBike::UnknownVirtualSlot93()
{
    KbPad* pad = (KbPad*)field_0x468->field_0x0c;
    if (pad && pad->field_0xc == 3) {
        pad->UnknownVirtualSlot6(1, 10000, -1);
        pad = (KbPad*)field_0x468->field_0x0c;
        pad->UnknownVirtualSlot6(0, (int)(g_kbGame->field_0xffc * 83.333336f), 70000);
        field_0x51c = g_kbGame->field_0xffc * 0.04f;
    }
}

// Slot 94: pad effect update; the force term is field_0x590 * field_0x51c * 180 clamped to
// [-10000, 10000] while not in state 1, then the effect is toggled on slot 80's result.
void KrustyBike::UnknownVirtualSlot94()
{
    KbPad* pad = (KbPad*)field_0x468->field_0x0c;
    if (pad && pad->field_0xc == 3) {
        if (field_0x444 != 1) {
            KbEffectParams params = { 0x4650, 0 };
            int force = (int)(field_0x590 * field_0x51c * 180.0f);
            if (force <= -10000)
                force = -10000;
            else if (force >= 10000)
                force = 10000;
            pad = (KbPad*)field_0x468->field_0x0c;
            pad->UnknownVirtualSlot7(1, &params, force);
            if (UnknownVirtualSlot80())
                ((KbPad*)field_0x468->field_0x0c)->UnknownVirtualSlot12(1, 1, 0);
            else
                ((KbPad*)field_0x468->field_0x0c)->UnknownVirtualSlot11(1);
        }
        if (field_0x108) {
            g_kbPadActive = 1;
            return;
        }
        if (g_kbPadActive) {
            ((KbPad*)field_0x468->field_0x0c)->UnknownVirtualSlot12(0, 1, 0);
            g_kbPadActive = 0;
        }
    }
}

// Slot 29: refresh the previous/current orientation angles (0x48..0x60 from 0x2c..0x44).
// Normally they are recomputed from the node's world axes; while neither field_0x6fc nor
// field_0x430 is set the old values are just copied.  Then the bike's matrix is mirrored
// onto the 0x5c4 node when field_0x604's mode is 0 or 1, and the 0x141c timer is advanced.
void KrustyBike::UnknownVirtualSlot29(int a)
{
    if (!field_0x6fc && !field_0x430) {
        field_0xa0 = field_0x88;
        field_0xac = field_0x94;
        field_0x50 = field_0x34;
        field_0x4c = field_0x30;
        field_0x48 = field_0x2c;
        field_0x54 = field_0x38;
        field_0x58 = field_0x3c;
        field_0x60 = field_0x44;
        field_0x5c = field_0x40;
    } else {
        field_0x42c->GetAxesIn(0, &field_0xa0, &field_0xac);
        OrientationAnglesFromVectors(field_0xa0, field_0xac, &field_0x50, &field_0x4c, &field_0x48,
                                     &field_0x54, &field_0x58, &field_0x60, &field_0x5c);
    }
    int mode = field_0x604->a_0x44;
    if (!mode || mode == 1) {
        Matrix4 m;
        d3d_field_0x1a0->GetMatrixIn(0, &m);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, &m);
    }
    if (field_0x431 && !field_0x7a4 && field_0x59c) {
        if (field_0x141c < 0.0f) {
            field_0x768 += Fn_00495FF0();
            return;
        }
        field_0x141c -= field_0x13c;
    }
}

// Slot 50: switch the 0x4f0 state (and the attached 0x15e8 object) on or off.  The base slot
// only runs for the forced (c != 0) case; otherwise the state is latched here with b stored
// as the countdown at 0x4f4.
void KrustyBike::UnknownVirtualSlot50(int a, float b, int c)
{
    if (field_0x740->field_0x18e)
        return;
    if (c) {
        if (a) {
            if (!field_0x4f0) {
                if (field_0x736) {
                    ((KbXform*)d3d_field_0x1a0)->Fn_00444D80(field_0x15e8);
                    ((KbXform*)field_0x5c4->c_0x1a0)->Fn_00444D80(field_0x15e8);
                    field_0x15e8->Fn_0047BBF0(0);
                }
                Fn_00496E30(1);
            }
            field_0x15e6 = 1;
            field_0x4f0 = 1;
            field_0x4f4 = 0;
        } else {
            field_0x15e6 = 0;
            if (field_0x4f0)
                field_0x7a5 = 1;
        }
        Vehicle::UnknownVirtualSlot50(a, 0, 0);
        return;
    }
    if (field_0x15e6)
        return;
    if (a) {
        if (!field_0x4f0) {
            if (field_0x736) {
                ((KbXform*)d3d_field_0x1a0)->Fn_00444D80(field_0x15e8);
                ((KbXform*)field_0x5c4->c_0x1a0)->Fn_00444D80(field_0x15e8);
                field_0x15e8->Fn_0047BBF0(b);
            }
            Fn_00496E30(1);
        }
        field_0x4f0 = 1;
        field_0x4f4 = b;
    } else if (field_0x4f0) {
        field_0x7a5 = 1;
    }
}

// Slot 43: reset of the per-bike state (runs the Vehicle reset, then clears the 0x74c..0x7c0
// block, the field_0x744 record and a few late fields).  The record's 0x44 vector is taken from
// the race context (0xf8..0x100, or from the object at +0x48 when no 0x144 object exists).
void KrustyBike::UnknownVirtualSlot43()
{
    Vehicle::UnknownVirtualSlot43();
    field_0x78c = 1;
    field_0x76c = 0;
    field_0x7a0 = 0;
    field_0x7a2 = 0;
    field_0x790 = 0;
    field_0x74c = 0;
    field_0x774 = 0;
    field_0x770 = 0;
    field_0x784 = 0;
    field_0x7a4 = 0;
    field_0x750 = 0;
    field_0x754 = 0;
    field_0x788 = 0;
    field_0x758 = 0;
    field_0x760 = 0;
    field_0x764 = 0;
    field_0x794 = 0;
    field_0x798 = 0;
    field_0x768 = 0;
    field_0x1400 = 0;
    field_0x1404 = 0;
    field_0x1408 = 0;
    field_0x140c = 0;
    field_0x804 = 0;
    field_0x808 = 0;
    field_0x7b8 = 0;
    field_0x7c0 = 0;
    field_0x7bc = 1;
    if (field_0x744) {
        KbRaceSub* p = field_0x740->field_0x48;
        if (p) {
            if (field_0x740->field_0x144) {
                field_0x744->field_0x44.a = field_0x740->field_0xf8;
                field_0x744->field_0x44.b = field_0x740->field_0xfc;
                field_0x744->field_0x44.c = field_0x740->field_0x100;
            } else {
                field_0x744->field_0x44.a = (int)p->field_0x0;
                field_0x744->field_0x44.b = field_0x740->field_0x48->field_0x0[2];
                field_0x744->field_0x44.c = 0;
            }
            field_0x744->field_0x38 = field_0x744->field_0x44;
        }
        field_0x744->field_0x34 = field_0x740->field_0xc8;
        field_0x744->field_0x08 = 0;
        field_0x744->field_0x0c = 0;
        field_0x744->field_0x10 = 0;
        field_0x744->field_0x14 = 0;
        field_0x744->field_0x18 = 0;
        field_0x744->field_0x1c = Vec3(0.0f, 0.0f, 0.0f);
        field_0x744->field_0x28 = Vec3(0.0f, 0.0f, 0.0f);
    }
    field_0x15c8 = g_kbGame->field_0x3414;
    field_0x15d0 = g_kbGame->field_0x3418;
    field_0x138c = 0;
    field_0x13f8 = 0x7effffff;
}

// Slot 97: after the Bike set-up, registers the rider's animation clips (by name) with the
// field_0x604 object, binds both wheels and attaches the scene node (tier 3 semantics).
void KrustyBike::UnknownVirtualSlot97()
{
    Bike::UnknownVirtualSlot97();
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Fall01", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Fall02", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Fall03", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Fall04", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("LeftHit", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("RightHit", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("FeetHitL", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("FeetHitR", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)field_0x5c4)->Fn_004A6B30("HeadHit", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("BackOver", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Endo", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("Kahuna", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("BackOver", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("LeftOver", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)field_0x5c4)->Fn_004A6B30("RightOver", 1));
    ((KbA604*)field_0x604)->Fn_00530680(field_0x5f0);
    ((KbA604*)field_0x604)->Fn_00530680(field_0x5f4);
    ((KbA604*)field_0x604)->Fn_005328B0(d3d_field_0x1a0->firstChild, Vec3(0.0f, 2.0f, 0.0f), Vec3(0.5f, 1.5f, 3.0f));
}

// Slot 49: per-frame update.  Keeps the 0x154c "pinned" copy of the placement in sync, posts
// on-screen messages to the local player (string ids 0x1481.. / 0x14d6 / 0x14d7 / 0x14e1; tier 3
// semantics), advances the 0x7a4 state, and runs the Bike update unless the race context
// takes over.
void KrustyBike::UnknownVirtualSlot49(float dt)
{
    if (field_0x154c) {
        d3d_field_0x1a0->SetPosition(field_0x0c);
        ((SoultreeObject*)field_0x5c4->c_0x1a0)->SetPosition(field_0x0c);
    }
    if (field_0x740->field_0x38 == this && g_kbGame->field_0x2d70 == 4) {
        KbMsgSink* sink = g_kbGame->field_0x570->Fn_0045D340();
        KbMessage* msg = 0;
        if (sink) {
            char text[0x100];
            if (field_0x754 == 0) {
                if (field_0x740->field_0x189)
                    g_kbGame->Fn_00521970(0x14d6, text, 0x80);
                else
                    g_kbGame->Fn_00521970(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11c6) KbMessage(text, 3.25f);
                if (msg) {
                    sink->Fn_0051B540(msg);
                    delete msg;
                }
            } else if (field_0x7a0 != field_0x7a2 && field_0x7a0 == 1 && field_0x740->field_0x189) {
                g_kbGame->Fn_00521970(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11d0) KbMessage(text, 3.25f);
                if (msg) {
                    sink->Fn_0051B540(msg);
                    delete msg;
                }
            }
        }
    }
    if (field_0x7a4) {
        if (field_0x7a4 == 1) {
            KbMsgSink* sink = g_kbGame->field_0x570->Fn_0045D340();
            if (field_0x740->field_0x38 == this && sink) {
                char text[0x100];
                KbMessage* msg = 0;
                if (g_kbGame->field_0x18 == 1) {
                    switch (field_0x784) {
                    case 1: g_kbGame->Fn_00521970(0x1481, text, 0x80); break;
                    case 2: g_kbGame->Fn_00521970(0x1482, text, 0x80); break;
                    case 3: g_kbGame->Fn_00521970(0x1483, text, 0x80); break;
                    case 4: g_kbGame->Fn_00521970(0x1484, text, 0x80); break;
                    case 5: g_kbGame->Fn_00521970(0x1485, text, 0x80); break;
                    case 6: g_kbGame->Fn_00521970(0x1486, text, 0x80); break;
                    case 7: g_kbGame->Fn_00521970(0x1487, text, 0x80); break;
                    case 8: g_kbGame->Fn_00521970(0x1488, text, 0x80); break;
                    case 9: g_kbGame->Fn_00521970(0x1489, text, 0x80); break;
                    case 10: g_kbGame->Fn_00521970(0x148a, text, 0x80); break;
                    case 11: g_kbGame->Fn_00521970(0x148b, text, 0x80); break;
                    default: g_kbGame->Fn_00521970(0x148c, text, 0x80); break;
                    }
                    msg = new(__FILE__, 0x1206) KbMessage(text, 3.25f);
                    if (msg) {
                        sink->Fn_0051B540(msg);
                        delete msg;
                    }
                } else {
                    g_kbGame->Fn_00521970(0x14e1, text, 0x80);
                    msg = new(__FILE__, 0x120d) KbMessage(text, 3.25f);
                    if (msg) {
                        sink->Fn_0051B540(msg);
                        delete msg;
                    }
                }
            }
            if (field_0x784 == 1) {
                if (g_kbGame->field_0x2d74) {
                    UnknownVirtualSlot41();
                    field_0x431 = 0;
                    ((KbA5C4*)field_0x5c4)->Fn_004A8B40(field_0x66c[15]);
                    ((KbA5C4*)field_0x5c4)->field_0x10 = 0;
                    field_0x7a4 = 3;
                } else {
                    field_0x7a4 = 3;
                    UnknownVirtualSlot41();
                    field_0x431 = 0;
                }
            } else {
                field_0x7a4 = 3;
                UnknownVirtualSlot41();
                field_0x431 = 0;
            }
        }
    }
    if (!field_0x444 && (field_0x734 || field_0x740->field_0xb8)) {
        if (field_0x740->field_0x18a)
            Fn_00413200(dt);
    } else {
        Bike::UnknownVirtualSlot49(dt);
    }
    if (field_0x740->field_0x50->field_0x244 == 3 && field_0x740->field_0x50->field_0x3b0 == this) {
        d3d_field_0x1a0->SetPosition(field_0x1540);
        ((SoultreeObject*)field_0x5c4->c_0x1a0)->SetPosition(field_0x1540);
        field_0x154c = 1;
    } else {
        field_0x154c = 0;
    }
    field_0x1540 = field_0x0c;
    field_0x153e = field_0x153c;
    field_0x7a2 = field_0x7a0;
}

// 0x00496F90: both bikes' collision objects ignore each other (AddIgnoredOwner = 0x00439410).
void KrustyBike::Fn_00496F90(KrustyBike* other)
{
    if (field_0x1418) {
        ((KbCollider*)other->field_0x1418)->AddIgnoredOwner((void*)field_0x1418);
        ((KbCollider*)other->field_0x1418)->AddIgnoredOwner((void*)field_0x1414);
        ((KbCollider*)other->field_0x1418)->AddIgnoredOwner((void*)field_0x5f0);
        ((KbCollider*)other->field_0x1418)->AddIgnoredOwner((void*)field_0x5f4);
        ((KbCollider*)other->field_0x1418)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->field_0x1414)->AddIgnoredOwner((void*)field_0x1418);
        ((KbCollider*)other->field_0x1414)->AddIgnoredOwner((void*)field_0x1414);
        ((KbCollider*)other->field_0x1414)->AddIgnoredOwner((void*)field_0x5f0);
        ((KbCollider*)other->field_0x1414)->AddIgnoredOwner((void*)field_0x5f4);
        ((KbCollider*)other->field_0x1414)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->field_0x5f0)->AddIgnoredOwner((void*)field_0x1418);
        ((KbCollider*)other->field_0x5f0)->AddIgnoredOwner((void*)field_0x1414);
        ((KbCollider*)other->field_0x5f0)->AddIgnoredOwner((void*)field_0x5f0);
        ((KbCollider*)other->field_0x5f0)->AddIgnoredOwner((void*)field_0x5f4);
        ((KbCollider*)other->field_0x5f0)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->field_0x5f4)->AddIgnoredOwner((void*)field_0x1418);
        ((KbCollider*)other->field_0x5f4)->AddIgnoredOwner((void*)field_0x1414);
        ((KbCollider*)other->field_0x5f4)->AddIgnoredOwner((void*)field_0x5f0);
        ((KbCollider*)other->field_0x5f4)->AddIgnoredOwner((void*)field_0x5f4);
        ((KbCollider*)other->field_0x5f4)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x1418);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x1414);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x5f0);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x5f4);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)field_0x1418)->AddIgnoredOwner((void*)other->field_0x1418);
        ((KbCollider*)field_0x1418)->AddIgnoredOwner((void*)other->field_0x1414);
        ((KbCollider*)field_0x1418)->AddIgnoredOwner((void*)other->field_0x5f0);
        ((KbCollider*)field_0x1418)->AddIgnoredOwner((void*)other->field_0x5f4);
        ((KbCollider*)field_0x1418)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)field_0x1414)->AddIgnoredOwner((void*)other->field_0x1418);
        ((KbCollider*)field_0x1414)->AddIgnoredOwner((void*)other->field_0x1414);
        ((KbCollider*)field_0x1414)->AddIgnoredOwner((void*)other->field_0x5f0);
        ((KbCollider*)field_0x1414)->AddIgnoredOwner((void*)other->field_0x5f4);
        ((KbCollider*)field_0x1414)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)field_0x5f0)->AddIgnoredOwner((void*)other->field_0x1418);
        ((KbCollider*)field_0x5f0)->AddIgnoredOwner((void*)other->field_0x1414);
        ((KbCollider*)field_0x5f0)->AddIgnoredOwner((void*)other->field_0x5f0);
        ((KbCollider*)field_0x5f0)->AddIgnoredOwner((void*)other->field_0x5f4);
        ((KbCollider*)field_0x5f0)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)field_0x5f4)->AddIgnoredOwner((void*)other->field_0x1418);
        ((KbCollider*)field_0x5f4)->AddIgnoredOwner((void*)other->field_0x1414);
        ((KbCollider*)field_0x5f4)->AddIgnoredOwner((void*)other->field_0x5f0);
        ((KbCollider*)field_0x5f4)->AddIgnoredOwner((void*)other->field_0x5f4);
        ((KbCollider*)field_0x5f4)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->field_0x1418);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->field_0x1414);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->field_0x5f0);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->field_0x5f4);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
    }
}

// 0x00497370: undo the mutual ignore unless one of them is the camera-followed racer.
void KrustyBike::Fn_00497370(KrustyBike* other)
{
    if (g_kbGame->field_0x568) {
        void* current = g_kbGame->field_0x568->field_0xa8;
        if (other == current || this == current)
            return;
    }
    ((KbCollider*)other->field_0x128)->RemoveIgnoredOwner((void*)field_0x128);
    ((KbCollider*)other->field_0x128)->RemoveIgnoredOwner((void*)field_0x5f0);
    ((KbCollider*)other->field_0x128)->RemoveIgnoredOwner((void*)field_0x5f4);
    ((KbCollider*)other->field_0x128)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->field_0x5f0)->RemoveIgnoredOwner((void*)field_0x128);
    ((KbCollider*)other->field_0x5f0)->RemoveIgnoredOwner((void*)field_0x5f0);
    ((KbCollider*)other->field_0x5f0)->RemoveIgnoredOwner((void*)field_0x5f4);
    ((KbCollider*)other->field_0x5f0)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->field_0x5f4)->RemoveIgnoredOwner((void*)field_0x128);
    ((KbCollider*)other->field_0x5f4)->RemoveIgnoredOwner((void*)field_0x5f0);
    ((KbCollider*)other->field_0x5f4)->RemoveIgnoredOwner((void*)field_0x5f4);
    ((KbCollider*)other->field_0x5f4)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)field_0x128);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)field_0x5f0);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)field_0x5f4);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    if (other->UnknownVirtualSlot51())
        return;
    ((KbCollider*)field_0x128)->RemoveIgnoredOwner((void*)other->field_0x128);
    ((KbCollider*)field_0x128)->RemoveIgnoredOwner((void*)other->field_0x5f0);
    ((KbCollider*)field_0x128)->RemoveIgnoredOwner((void*)other->field_0x5f4);
    ((KbCollider*)field_0x128)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)field_0x5f0)->RemoveIgnoredOwner((void*)other->field_0x128);
    ((KbCollider*)field_0x5f0)->RemoveIgnoredOwner((void*)other->field_0x5f0);
    ((KbCollider*)field_0x5f0)->RemoveIgnoredOwner((void*)other->field_0x5f4);
    ((KbCollider*)field_0x5f0)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)field_0x5f4)->RemoveIgnoredOwner((void*)other->field_0x128);
    ((KbCollider*)field_0x5f4)->RemoveIgnoredOwner((void*)other->field_0x5f0);
    ((KbCollider*)field_0x5f4)->RemoveIgnoredOwner((void*)other->field_0x5f4);
    ((KbCollider*)field_0x5f4)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->field_0x128);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->field_0x5f0);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->field_0x5f4);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
}

// 0x00496E30: a != 0 makes this bike's collision objects ignore every other bike (0x00496F90)
// and stops the part collision objects ignoring its main body; a == 0 does the reverse
// (0x00497370) and makes the part collision objects ignore both of its scene bodies.
void KrustyBike::Fn_00496E30(int a)
{
    int cursor = 0;
    KrustyBike* other;
    if (a) {
        for (other = field_0x740->Fn_004204E0(&cursor); other; other = field_0x740->Fn_004204E0(&cursor)) {
            if (other != this)
                Fn_00496F90(other);
        }
        if (((KbTrackA*)field_0x1f0)->field_0xb4 && !field_0x735) {
            for (int i = 0; i < ((KbTrackA*)field_0x1f0)->field_0xb4->count; i++) {
                KbPart* part = ((KbTrackA*)field_0x1f0)->field_0xb4->items + i;
                if (!(part->flags & 8))
                    part->obj->collider->RemoveIgnoredOwner((void*)field_0x128);
            }
        }
    } else {
        for (other = field_0x740->Fn_004204E0(&cursor); other; other = field_0x740->Fn_004204E0(&cursor)) {
            if (other != this && (!field_0x735 || !other->field_0x735))
                Fn_00497370(other);
        }
        if (((KbTrackA*)field_0x1f0)->field_0xb4 && !field_0x735) {
            for (int i = 0; i < ((KbTrackA*)field_0x1f0)->field_0xb4->count; i++) {
                if (!(((KbTrackA*)field_0x1f0)->field_0xb4->items[i].flags & 8)) {
                    KbPart* part = ((KbTrackA*)field_0x1f0)->field_0xb4->items + i;
                    KbPartObj* obj = part->obj;
                    obj->collider->AddIgnoredOwner((void*)field_0x1418);
                    obj->collider->AddIgnoredOwner((void*)field_0x1414);
                }
            }
        }
    }
}
