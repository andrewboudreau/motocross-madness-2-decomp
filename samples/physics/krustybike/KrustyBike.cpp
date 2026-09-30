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

int KrustyBike::UnknownVirtualSlot63(float a)
{
    if (field_0x734)
        return Fn_00414370(a);
    field_0x804 = 0;
    field_0x808 = 0;
    return Bike::UnknownVirtualSlot63(a);
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
        int mode = g_kbGame->field_0x2d74;
        if ((field_0xbc - field_0xb8) * b[mode != 3][g_kbGame->field_0x60c - 1] < arg * field_0x450)
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
