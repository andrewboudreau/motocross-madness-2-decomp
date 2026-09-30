// KrustyBike.cpp -- reconstruction of the KrustyBike overrides.
// Translation unit: KrustyBike.cpp (literal __FILE__ xrefs near 0x0048FE58; tier 2).
// Member and helper names are provisional (tier 3); see KrustyBikeTypes.h.
#include <math.h>
#include "KrustyBike.h"

void KrustyBike::UnknownVirtualSlot27()
{
    if (field_0x460 == 9 || field_0x430) {
        field_0x604->Fn_00532310();
    } else {
        field_0x604->Fn_00532220(field_0x448);
    }
    UnknownVirtualSlot101();
}

void KrustyBike::UnknownVirtualSlot1(int a)
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

void KrustyBike::UnknownVirtualSlot3(int a0, int a1, int a2, int a3, int a4, int a5, int* a6)
{
    if (a4 == 1000) {
        field_0x740->handler->Fn_004DE580(this, 0, a4);
        return;
    }
    Bike::UnknownVirtualSlot3(a0, a1, a2, a3, a4, a5, a6);
    if (a4 != 0x67)
        field_0x740->handler->Fn_004DE580(this, *a6, a4);
}

void KrustyBike::UnknownVirtualSlot4(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int* a13, int a14)
{
    Bike::UnknownVirtualSlot4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    field_0x740->handler->Fn_004DE580(this, *a13, a5);
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
        if (position.x < lo)
            return 1;
        KbTrackB* t = field_0x1f0->field_0xa4;
        if ((t->field_0x394 * 256.0f - 105.0f) * field_0x1f8 < position.x)
            return 2;
        if (position.z < lo)
            return 3;
        if ((t->field_0x398 * 256.0f - 105.0f) * field_0x1f8 < position.z)
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
        if (field_0xb8 < speed && !field_0x444 && speed < 22.0f)
            return 1;
    } else {
        if (field_0x5b0 < *field_0x480 && !field_0x444 && speed < 22.0f)
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot25()
{
    if (field_0x735) {
        if (field_0xb8 > speed && !field_0x444 && speed < 22.0f)
            return 1;
    } else {
        if (field_0x5b0 < *field_0x480 && !field_0x444 && speed < 22.0f)
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot33(int a0, int a1, int a2, int a3, int a4, int a5)
{
    int result = Bike::UnknownVirtualSlot33(a0, a1, a2, a3, a4, a5);
    field_0x1540 = position;
    field_0x604->field_0x52c = 1;
    return result;
}

int KrustyBike::UnknownVirtualSlot39(int a)
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
    if (!field_0x444 && field_0x433 >= 0 && field_0x430 && field_0x5c4->field_0xc) {
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

void KrustyBike::UnknownVirtualSlot63(int a)
{
    if (field_0x734) {
        Fn_00414370(a);
        return;
    }
    field_0x804 = field_0x808 = 0;
    Bike::UnknownVirtualSlot63(a);
}

void KrustyBike::UnknownVirtualSlot64(int a)
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
        Bike::UnknownVirtualSlot71(a);
        return;
    }
    if (field_0x108 && !a && !field_0x59c && field_0x431 && field_0x141c <= 0.0f)
        field_0x141c = 0.3f;
    Bike::UnknownVirtualSlot71(a);
}

KbVec3 KrustyBike::UnknownVirtualSlot76(int a, int b)
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
        (UnknownVirtualSlot84(4, 0x3f) && !UnknownVirtualSlot77(*field_0x480 == 0.0f)))
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
            if (!UnknownVirtualSlot77(*field_0x480 == 0.0f))
                return 1;
        }
        field_0x664 = 0;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot83(KbInput* input)
{
    if (field_0x808 > 0.0f ||
        (UnknownVirtualSlot84(5, 0x3f) && !UnknownVirtualSlot77(input->field_0x29c == 0.0f)))
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
        KbChild* c = field_0x12c[i];
        if (c->field_0x4)
            c->field_0xa4 = 0;
    }
    field_0x444 = 0;
    field_0x454 = 0;
    field_0x5b4 = 0;
    field_0x608 = 0;
    field_0x604->Fn_005327C0();
    field_0x61c = g_kbZeroVec;
}

void KrustyBike::UnknownVirtualSlot101()
{
    field_0x1414->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Fn_004DCF20(field_0x1414, field_0x1414->field_0x84);
    field_0x128 = field_0x1418;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        field_0x128->field_0xc.UnknownVirtualSlot4();
    else
        field_0x128->field_0xc.UnknownVirtualSlot5();
    field_0x128->Fn_00435FE0();
}

// ---- physics ----

// Slot 16: per-axis scale of v by one of two Vec3 tables (0xe4 or 0xf0), chosen by field_0x444.
KbVec3 KrustyBike::UnknownVirtualSlot16(KbVec3* v)
{
    KbVec3 r;
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
void KrustyBike::UnknownVirtualSlot15(KbVec3* v, float* out)
{
    KbVec3 s;
    KbVec3 t = field_0x3bc->Fn_004FD710(v);
    if (!field_0x444) {
        s.x = t.x * field_0xe4.x;
        s.y = t.y * field_0xe4.y;
        s.z = t.z * field_0xe4.z;
    } else {
        s.x = t.x * field_0xf0.x;
        s.y = t.y * field_0xf0.y;
        s.z = t.z * field_0xf0.z;
    }
    Fn_004CB6E0(&field_0xd8.x, out, &s.x, field_0x13c, 0);
}

// Slot 14: apply an impulse. field_0x13c behaves as an inverse mass (dv = k * impulse).
void KrustyBike::UnknownVirtualSlot14(KbVec3* a, KbVec3* b, KbVec3* c)
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot14(a, b, c);
        return;
    }
    field_0xc0 = UnknownVirtualSlot16(b);
    UnknownVirtualSlot15(c, &field_0xc0.x);
    field_0xd8 += field_0x13c * field_0xc0;
    field_0xd8 *= 0.999f;
    a->x = 0;
    a->z = 0;
    field_0x70 = KbVec3(0.0f, field_0x24 * a->y, 0.0f);
    velocity += field_0x13c * field_0x70;
    float d = DotProduct(velocity, velocity);
    if (d == 1.0f)
        speed = 1.0f;
    else
        speed = (float)sqrt(d);
}

// Slot 48: position update from the (velocity*3 - field_0x7c) * 0.5 * dt step.
void KrustyBike::UnknownVirtualSlot48()
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot48();
        return;
    }
    float dt = field_0x13c;
    KbVec3 v = velocity * 3.0f;
    KbVec3 d = (v - field_0x7c) * 0.5f;
    position += d * dt;
    field_0x5f4->field_0x248 = g_kbZeroVec;
    field_0x5f4->field_0x280 = 0;
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
int KrustyBike::UnknownVirtualSlot70(float arg)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (field_0x734) {
        int mode = g_kbGame->field_0x2d74;
        int i = g_kbGame->field_0x60c - 1;
        return (speed - field_0xb8) * b[mode != 3][i] < arg * field_0x450;
    }
    return speed - field_0xb8 < a[g_kbGame->field_0x60c - 1] * field_0x450 * arg;
}

// Slot 96: tuning constants derived from the game-settings integers (0xfe0..0xff8).
void KrustyBike::UnknownVirtualSlot96()
{
    if (field_0x734)
        field_0x524 = 0.675f;
    else
        {
            float pct = g_kbGame->field_0xfe0 * 0.01f;
            field_0x524 = pct * 0.2f + 0.575f;
        }
    if (field_0x734)
        field_0x63c = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfe4 * 0.01f;
            field_0x63c = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x528 = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfe8 * 0.01f;
            field_0x528 = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x52c = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xfec * 0.01f;
            field_0x52c = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x530 = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff4 * 0.01f;
            field_0x530 = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x534 = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff0 * 0.01f;
            field_0x534 = pct * 0.4f + 0.8f;
        }
    if (field_0x734)
        field_0x538 = 1.0f;
    else
        {
            float pct = g_kbGame->field_0xff8 * 0.01f;
            field_0x538 = pct * 0.4f + 0.8f;
        }
}
