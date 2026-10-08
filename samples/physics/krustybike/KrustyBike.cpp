// KrustyBike.cpp -- reconstruction of the KrustyBike overrides.
// Translation unit: KrustyBike.cpp (literal __FILE__ xrefs near 0x0048FE58; tier 2).
// Member and helper names are provisional (tier 3); see KrustyBikeTypes.h.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "KrustyBike.h"
#include "math/FastMath.h"
#include "collision/CollisionObject.h"

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

// 0x0048fa60 (near miss, 472 of 532 bytes): the GameObject virtual base is built
// only for the most-derived object (GameObject(1)), then Bike(flags). Every store
// matches, but VC6 schedules the +0x7c0/+0x7bc stores into the kVec3Zero copy for
// +0x1540 where retail puts +0x11b8/+0x604 (the next two statements). Other
// orders of the first four statements and an inline helper for the
// +0x7b8/+0x7c0/+0x7bc triple (also written together by slot 97) are worse.
KrustyBike::KrustyBike(int flags) : GameObject(1), Bike(flags)
{
    field_0x73c = 0x65;
    field_0x1540 = kVec3Zero;
    field_0x11b8 = 0;
    field_0x604 = 0;
    field_0x734 = 0;
    field_0x735 = 0;
    field_0x768 = 0;
    field_0x1604 = 0;
    field_0x740 = 0;
    field_0x7b8 = 0;
    field_0x7c0 = 0;
    field_0x7bc = 1;
    field_0x748 = 0;
    field_0x11bc = 0;
    field_0x141c = 0;
    field_0x4a4 = 1.0f;
    field_0x788 = 0;
    field_0x758 = 0;
    field_0x760 = 0;
    field_0x75c = 0;
    field_0x764 = 0;
    field_0x15e8 = 0;
    field_0x1410 = -1;
    field_0x15e6 = 0;
    field_0x7a5 = 0;
    field_0x736 = 0;
    field_0x7ac = 0;
    field_0x7b0 = 0;
    field_0x7b4 = 0;
    field_0x1358 = kVec3Zero;
    field_0x1364 = kVec3Zero;
    field_0x1370 = kVec3Zero;
    field_0x137c = 0;
    field_0x1380 = 0;
    field_0x1384 = 0;
    field_0x1388 = 0;
    field_0x1558 = 0;
    field_0x15d4 = 0;
    lastCollisionType = 0;
    altBodyB = 0;
    altBodyA = 0;
}

// Destructor body (0x00491540, reached through the vbase-adjusted scalar deleting
// destructor 0x00497c40): releases two debug-heap buffers (__FILE__ lines 0x8bb, 0x8be).
KrustyBike::~KrustyBike()
{
    if (heapBufferA)
        DebugFree(heapBufferA, __FILE__, 0x8bb);
    if (heapBufferB)
        DebugFree(heapBufferB, __FILE__, 0x8be);
}

void KrustyBike::UnknownVirtualSlot27()
{
    if (crashReason == 9 || field_0x430) {
        field_0x604->Method_0x00532310();
    } else {
        field_0x604->Method_0x00532220(crashDirection);
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
    nearestRival = 0;
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
        float lo = terrainScale * 105.0f;
        if (position.x < lo)
            return 1;
        KbTrackB* t = ((KbTrackA*)track)->field_0xa4;
        if ((t->field_0x394 * 256.0f - 105.0f) * terrainScale < position.x)
            return 2;
        if (position.z < lo)
            return 3;
        if ((t->field_0x398 * 256.0f - 105.0f) * terrainScale < position.z)
            return 4;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot22()
{
    if (staggerCounter == staggerPhase || field_0x740->field_0x50->field_0x3b4 == this)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot23()
{
    if (!airborne && field_0x434 >= 0.2f && field_0x740->field_0x50->field_0x244 != 3)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot24()
{
    if (field_0x735) {
        if (prevSpeed < linearSpeed && !crashState && linearSpeed < 22.0f)
            return 1;
        return 0;
    }
    if (field_0x5b0 < engineState->field_0x00 && !crashState && linearSpeed < 22.0f)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot25()
{
    if (field_0x735) {
        if (prevSpeed > linearSpeed && !crashState && linearSpeed < 22.0f)
            return 1;
    } else {
        if (field_0x5b0 < engineState->field_0x00 && !crashState && linearSpeed < 22.0f)
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot33(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5)
{
    int result = Bike::UnknownVirtualSlot33(a0, a1, a2, a3, a4, a5);
    field_0x1540 = position;
    field_0x604->a_0x52c = 1;
    return result;
}

int KrustyBike::UnknownVirtualSlot39(float a)
{
    if (!respawnPending && g_kbGame->field_0x2d70 && field_0x76c > 6.0f) {
        if (crashState)
            UnknownVirtualSlot67();
        return 1;
    }
    return Bike::UnknownVirtualSlot39(a);
}

int KrustyBike::UnknownVirtualSlot42()
{
    if (!crashState && field_0x433 >= 0 && field_0x430 && riderCharacter->c_0xc) {
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
    if (crashState || field_0x735)
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
    if (crashState) {
        field_0x804 = 0;
        field_0x808 = 0;
    }
    Bike::UnknownVirtualSlot64(a);
}

int KrustyBike::UnknownVirtualSlot66()
{
    if (crashState || field_0x430)
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
    if (crashDirection == 2) {
        crashTimer = field_0x7a8;
        return;
    }
    float t = crashTimerReload;
    crashTimer = t;
    if (!g_kbGame->field_0x2d70 && !g_kbGame->field_0x2d74)
        crashTimer = t + 1.0f;
}

void KrustyBike::UnknownVirtualSlot71(int a)
{
    if (crashState) {
        field_0x141c = 0;
    } else if (airborne && !a && !landingLatched && field_0x431 && field_0x141c <= 0.0f) {
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
    if (field_0x804 > 0.0f || (inputDevice && throttleInput > 0.33f) ||
        (UnknownVirtualSlot84(4, 0x3f) && !UnknownVirtualSlot77(engineState->field_0x00 == 0.0f)))
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
            if (!UnknownVirtualSlot77(engineState->field_0x00 == 0.0f))
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
            return inputMap->UnknownVirtualSlot2(a, b);
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
        g_kbGame->field_0x8->Send((unsigned char)(flag ? 0x13 : 0x87), &pkt, 12, ((KbNetBike*)this)->field_0x11bc, 0);
        if (!flag && netRecorder && g_kbGame->field_0x3334 && !g_kbGame->field_0x3428)
            netRecorder->Fn_004E8720(0x87, who ? ((KbNetBike*)who)->field_0x11bc : 0, &pkt, 1);
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
    netRecorder = a;
}

// ---- non-virtual state reset (0x0048D8B0) ----
void KrustyBike::Fn_0048D8B0()
{
    field_0x1538 = 1.0f;
    field_0x1530 = 0;
    field_0x1520 = 0;
    field_0x1528 = 0;
    field_0x1524 = 0;
    landingLatched = 0;
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
    for (int i = 0; i < collisionPointCount; i++) {
        KbChild* c = ((KbChild**)collisionPoints)[i];
        if (c->field_0x4)
            c->field_0xa4 = 0;
    }
    crashState = 0;
    crashTimer = 0;
    prevCrashState = 0;
    linkedVehicle = 0;
    field_0x604->Method_0x005327c0();
    field_0x61c = g_kbZeroVec;
}

// Back-camera swap: like slot 101 with the two scene objects exchanged.
void KrustyBike::Fn_00496DA0()
{
    altBodyB->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Remove(altBodyB, altBodyB->field_0x84);
    collisionObject = (CollisionObject*)altBodyA;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)collisionObject)->Fn_00435FE0();
}

// owner: bracket only (0x496d20 sits between slot 101's stub and Fn_00496DA0)
// Real body of slot 101: switch the active scene object from altBodyA to altBodyB.  Retail's
// slot 101 entry (0x496d10) is a 16-byte stub, `jmp 0x496d20` plus 11 nops, in front of it.
void KrustyBike::Fn_00496D20()
{
    altBodyA->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Remove(altBodyA, altBodyA->field_0x84);
    collisionObject = (CollisionObject*)altBodyB;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)collisionObject)->Fn_00435FE0();
}

// The 5-byte `jmp` plus 11 nops of retail's stub cannot be reproduced from C++ (no inline asm
// or padding tricks), so this stays partial.
void KrustyBike::UnknownVirtualSlot101()
{
    Fn_00496D20();
}

// ---- physics ----

// Slot 16: per-axis scale of v by one of two Vec3 tables (0xe4 or 0xf0), chosen by field_0x444.
Vec3 KrustyBike::UnknownVirtualSlot16(const Vec3* v)
{
    Vec3 r;
    if (!crashState) {
        r.x = invInertia.x * v->x;
        r.y = invInertia.y * v->y;
        r.z = invInertia.z * v->z;
    } else {
        r.x = shapeInvInertia.x * v->x;
        r.y = shapeInvInertia.y * v->y;
        r.z = shapeInvInertia.z * v->z;
    }
    return r;
}

// Slot 15: transform v by the matrix at +0x3bc, scale per axis as slot 16, then hand to 0x4CB6E0
// (which looks like a clamp/settle step on the accumulator at +0xd8).
void KrustyBike::UnknownVirtualSlot15(const Vec3* v, Vec3* out)
{
    Vec3 s;
    Vec3 t = ((KbXform*)modelNode)->WorldToLocalDirection(v);
    if (!crashState) {
        s.x = t.x * invInertia.x;
        s.y = t.y * invInertia.y;
        s.z = t.z * invInertia.z;
    } else {
        s.x = t.x * shapeInvInertia.x;
        s.y = t.y * shapeInvInertia.y;
        s.z = t.z * shapeInvInertia.z;
    }
    UnknownAxisSettle_4cb6e0(&angularVelocity.x, &out->x, &s.x, stepTime, 0);
}

// Slot 14: apply an impulse. field_0x13c behaves as an inverse mass (dv = k * impulse).
void KrustyBike::UnknownVirtualSlot14(Vec3* a, const Vec3* b, const Vec3* c)
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot14(a, b, c);
        return;
    }
    angularAcceleration = UnknownVirtualSlot16(b);
    UnknownVirtualSlot15(c, &angularAcceleration);
    angularVelocity += stepTime * angularAcceleration;
    angularVelocity *= 0.999f;
    a->x = 0;
    a->z = 0;
    acceleration = Vec3(0.0f, invMass * a->y, 0.0f);
    velocity += stepTime * acceleration;
    linearSpeed = KbLength(velocity);
}

// Slot 48: field_0x0c update from the (field_0x64*3 - field_0x7c) * 0.5 * dt step.
void KrustyBike::UnknownVirtualSlot48()
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot48();
        return;
    }
    Vec3 v = velocity * 3.0f;
    Vec3 d = (v - prevVelocity) * 0.5f;
    position += d * stepTime;
    rearWheel->w_0x248 = g_kbZeroVec;
    rearWheel->w_0x280 = 0;
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
int KrustyBike::UnknownVirtualSlot70(float arg)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (field_0x734) {
        float speedGain = linearSpeed - prevSpeed;
        if (speedGain * b[g_kbGame->field_0x2d74 != 3][g_kbGame->field_0x60c - 1] < arg * speedGainLimit)
            return 1;
        return 0;
    }
    if (linearSpeed - prevSpeed < a[g_kbGame->field_0x60c - 1] * speedGainLimit * arg)
        return 1;
    return 0;
}

// Slot 86: per-frame wheel-contact update (provisional semantics).  When the race context
// flag (field_0x740+0x18a) is set, a lookup ramp derived from field_0x664 (0.12..0.4762,
// scaled by 1010.668) feeds the wheel's helper object, and in one game mode the wheel's
// two force accumulators are halved; the wheel is then always updated.
void KrustyBike::UnknownVirtualSlot86()
{
    BikeWheel* w = rearWheel;
    if (!w->inContact)
        return;
    if (field_0x740->field_0x18a) {
        float ramp;
        if (field_0x47a && field_0x664 >= 0.12f && field_0x664 < 0.4762f)
            ramp = (0.4762f - field_0x664) * 1010.668f;
        else
            ramp = 0;
        if (w->w_0x2a8)
            w->w_0x2a8->Fn_004D31B0(KbFloat(linearSpeed), &w->w_0x230, KbFloat(field_0x4a4), field_0x47a, ramp,
                                    &w->w_0x248, &w->w_0x280);
        if (g_kbGame->field_0x2d70 && !field_0x78c) {
            rearWheel->w_0x248 *= 0.5f;
            rearWheel->w_0x280 *= 0.5f;
        }
    }
    rearWheel->UpdateDriveShare(this);
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
    lastCollisionType = 0;
    if (field_0x736) return a;
    if (g_kbGame->field_0x2d74 == 4) {
        KbPlayer* p = g_kbGame->field_0x570->Fn_0045D2B0();
        if (this != p->field_0xa8 && this == field_0x740->field_0x38) {
            Vehicle* v = p->field_0xa8;
            if (v) {
                if (((KbBody*)collisionObject)->TestMeshBounds((KbBody*)v->collisionObject))
                    Fn_004925A0(this, 1);
            } else if (p->field_0xdc) {
                if (((KbBody*)collisionObject)->TestMeshBounds((KbBody*)p->field_0xdc->collisionObject)) {
                    if (g_kbGame->field_0x8->field_0x10) p->Fn_004A9E80(this, 0, 1);
                    else Fn_004925A0(this, 1);
                }
            }
        }
        if (field_0x735) return a;
    }
    int r = Vehicle::UnknownVirtualSlot28(a);
    if (crashState && !prevCrashState) {
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
    if (crashState) return crashState;
    if (UnknownVirtualSlot12(anyWheelInContact)) {
        KbTrackB* t = ((KbTrackA*)track)->field_0xa4;
        scratchVector = Vec3(t->field_0x394 * terrainScale * 128.0f, 20.0f,
                             t->field_0x398 * terrainScale * 128.0f);
        Vec3 off = scratchVector - position;
        velocity = off * 0.055f;
        velocity.y = 120.0f;
        prevSpeed = linearSpeed = KbLength(velocity);
        float rx = rand() * (1.0f / 32768.0f);
        float ry = rand() * (1.0f / 32768.0f);
        angularVelocity = Vec3(rx, ry, 0.02f);
        crashDirection = 1;
        crashReason = 12;
        return 1;
    }
    if (!field_0x430 && inputMap && inputMap->UnknownVirtualSlot2(8, 0x3f)) {
        if (airborne && !crashState && frontWheel->w_0x150 < -3.5f && rearWheel->w_0x150 < -3.5f) {
            crashDirection = 2;
            crashReason = 1;
            return 1;
        }
    }
    if (!airborne && field_0x430) {
        if (frontWheel->inContact) {
            if (linearSpeed > 30.0f) crashDirection = 1;
            else if (angularVelocity.z > 0.0f) crashDirection = 4;
            else crashDirection = 5;
        } else crashDirection = 3;
        crashReason = 9;
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
        hit = Kb_004B0AC0(&position, 2.0f, 2.0f, 2.0f, terrainScale, 0, 0, 0, 0);
    KbGame* g = g_kbGame;
    switch (g->field_0x2d74) {
    case 2:
    case 3:
        if (g->field_0x2d70 || field_0x78c || field_0x734 || field_0x736 || a1 == 2 ||
            g->field_0xc50) {
            *a2 = respawnPosition;
            *a3 = respawnHeading;
            position = respawnPosition;
        } else {
            Vec3 v = respawnPosition;
            *a2 = position;
            *a3 = Vec3(v.x - a2->x, -a2->y, v.z - a2->z);
            float d = a3->x * a3->x;
            d += a3->y * a3->y;
            d += a3->z * a3->z;
            if (d == 0.0f) *a3 = g_kbZeroVec;
            else *a3 *= FastInvSqrt(d);
            return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, 0x64,
                               0x65, hit, 3.0f, 8, 0, &respawnPosition, !respawnPending, a2, a3, a4, a5);
        }
    case 0: {
        const Vec3* p = respawnPending ? &respawnHeading : &bodyForward;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, 6.0f, 8, p, 0, !respawnPending, a2, a3, a4, a5);
    }
    case 4: {
        KbPlayer* r = (KbPlayer*)g->field_0x568;
        const Vec3* l = 0;
        const Vec3* k = 0;
        if (r->field_0xdc) l = &r->field_0xdc->position;
        else if (r->field_0xa8 == this) { *a3 = bodyForward; k = &bodyForward; }
        else l = &r->field_0xa8->position;
        float i = 9.0f;
        if (((KbPlayer*)g->field_0x568)->field_0xa8 != this) i = 3.0f;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, i, 8, k, l, !respawnPending, a2, a3, a4, a5);
    }
    case 1:
    case 5: {
        if (respawnPending) scratchVector = respawnHeading + respawnPosition;
        else scratchVector = g->field_0x560[field_0x7b8 + 9].field_0x00;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim,
                           respawnPending ? 0x64 : 0x7fffffff, respawnPending ? 0x65 : 0x7fffffff,
                           3.0f, 8, 0, &scratchVector, !respawnPending, a2, a3, a4, a5);
    }
    default:
        *a2 = position;
        *a3 = bodyForward;
    }
}

// Slot 93: when the pad device (input map +0x0c) is in mode 3, reset one effect channel and
// re-arm another with a time-scaled value, then derive field_0x51c from the same game value.
void KrustyBike::UnknownVirtualSlot93()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->field_0xc == 3) {
        pad->UnknownVirtualSlot6(1, 10000, -1);
        pad = (KbPad*)inputMap->valueSource;
        pad->UnknownVirtualSlot6(0, (int)(g_kbGame->field_0xffc * 83.333336f), 70000);
        field_0x51c = g_kbGame->field_0xffc * 0.04f;
    }
}

// Slot 94: pad effect update; the force term is field_0x590 * field_0x51c * 180 clamped to
// [-10000, 10000] while not in state 1, then the effect is toggled on slot 80's result.
void KrustyBike::UnknownVirtualSlot94()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->field_0xc == 3) {
        if (crashState != 1) {
            KbEffectParams params = { 0x4650, 0 };
            int force = (int)(smoothedForwardAccel * field_0x51c * 180.0f);
            if (force <= -10000)
                force = -10000;
            else if (force >= 10000)
                force = 10000;
            pad = (KbPad*)inputMap->valueSource;
            pad->UnknownVirtualSlot7(1, &params, force);
            if (UnknownVirtualSlot80())
                ((KbPad*)inputMap->valueSource)->UnknownVirtualSlot12(1, 1, 0);
            else
                ((KbPad*)inputMap->valueSource)->UnknownVirtualSlot11(1);
        }
        if (airborne) {
            g_kbPadActive = 1;
            return;
        }
        if (g_kbPadActive) {
            ((KbPad*)inputMap->valueSource)->UnknownVirtualSlot12(0, 1, 0);
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
        savedForward = bodyForward;
        savedUp = bodyUp;
        savedYaw = bodyYaw;
        savedPitch = bodyPitch;
        savedRoll = bodyRoll;
        savedSinRoll = bodySinRoll;
        savedCosRoll = bodyCosRoll;
        savedCosPitch = bodyCosPitch;
        savedSinPitch = bodySinPitch;
    } else {
        poseNode->GetAxesIn(0, &savedForward, &savedUp);
        OrientationAnglesFromVectors(savedForward, savedUp, &savedYaw, &savedPitch, &savedRoll,
                                     &savedSinRoll, &savedCosRoll, &savedCosPitch, &savedSinPitch);
    }
    int mode = field_0x604->a_0x44;
    if (!mode || mode == 1) {
        Matrix4 m;
        modelNode->GetMatrixIn(0, &m);
        riderCharacter->c_0x1a0->Method_0x004fb8c0(0, &m);
    }
    if (field_0x431 && !field_0x7a4 && landingLatched) {
        if (field_0x141c < 0.0f) {
            field_0x768 += Fn_00495FF0();
            return;
        }
        field_0x141c -= stepTime;
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
            if (!spawnProtected) {
                if (field_0x736) {
                    ((KbXform*)modelNode)->Fn_00444D80(field_0x15e8);
                    ((KbXform*)riderCharacter->c_0x1a0)->Fn_00444D80(field_0x15e8);
                    field_0x15e8->Fn_0047BBF0(0);
                }
                Fn_00496E30(1);
            }
            field_0x15e6 = 1;
            spawnProtected = 1;
            spawnProtectTimer = 0;
        } else {
            field_0x15e6 = 0;
            if (spawnProtected)
                field_0x7a5 = 1;
        }
        Vehicle::UnknownVirtualSlot50(a, 0, 0);
        return;
    }
    if (field_0x15e6)
        return;
    if (a) {
        if (!spawnProtected) {
            if (field_0x736) {
                ((KbXform*)modelNode)->Fn_00444D80(field_0x15e8);
                ((KbXform*)riderCharacter->c_0x1a0)->Fn_00444D80(field_0x15e8);
                field_0x15e8->Fn_0047BBF0(b);
            }
            Fn_00496E30(1);
        }
        spawnProtected = 1;
        spawnProtectTimer = b;
    } else if (spawnProtected) {
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
    netState.timer = g_kbGame->fullNetPacketIntervalSec;
    field_0x15d0 = g_kbGame->shortNetPacketIntervalSec;
    field_0x138c = 0;
    recordState.timer = 1.7014117e38f;
}

// .data 0x0056cb88: six trick records {id, name[32]}; slot 97 registers the clips by
// these names (retail keeps the two "BackOver" entries apart).
struct KrustyTrickName {
    int id;
    char name[32];
};
static KrustyTrickName s_KrustyTricks[6] = {
    {0, "BackOver"}, {1, "Endo"}, {2, "Kahuna"}, {3, "BackOver"}, {4, "LeftOver"}, {5, "RightOver"},
};

// Slot 97: after the Bike set-up, registers the rider's animation clips (by name) with the
// field_0x604 object, binds both wheels and attaches the scene node (tier 3 semantics).
void KrustyBike::UnknownVirtualSlot97()
{
    Bike::UnknownVirtualSlot97();
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)riderCharacter)->FindMotion("Fall01", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)riderCharacter)->FindMotion("Fall02", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)riderCharacter)->FindMotion("Fall03", 1));
    ((KbA604*)field_0x604)->Fn_005305F0(((KbA5C4*)riderCharacter)->FindMotion("Fall04", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)riderCharacter)->FindMotion("LeftHit", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)riderCharacter)->FindMotion("RightHit", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)riderCharacter)->FindMotion("FeetHitL", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)riderCharacter)->FindMotion("FeetHitR", 1));
    ((KbA604*)field_0x604)->Fn_005305B0(((KbA5C4*)riderCharacter)->FindMotion("HeadHit", 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[0].name, 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[1].name, 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[2].name, 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[3].name, 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[4].name, 1));
    ((KbA604*)field_0x604)->Fn_00530630(((KbA5C4*)riderCharacter)->FindMotion(s_KrustyTricks[5].name, 1));
    ((KbA604*)field_0x604)->Fn_00530680(frontWheel);
    ((KbA604*)field_0x604)->Fn_00530680(rearWheel);
    ((KbA604*)field_0x604)->Fn_005328B0(modelNode->firstChild, Vec3(0.0f, 2.0f, 0.0f), Vec3(0.5f, 1.5f, 3.0f));
}

// Slot 49: per-frame update.  Keeps the 0x154c "pinned" copy of the placement in sync, posts
// on-screen messages to the local player (string ids 0x1481.. / 0x14d6 / 0x14d7 / 0x14e1; tier 3
// semantics), advances the 0x7a4 state, and runs the Bike update unless the race context
// takes over.
void KrustyBike::UnknownVirtualSlot49(float dt)
{
    if (field_0x154c) {
        modelNode->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
    }
    if (field_0x740->field_0x38 == this && g_kbGame->field_0x2d70 == 4) {
        KbMsgSink* sink = g_kbGame->field_0x570->Fn_0045D340();
        KbMessage* msg = 0;
        if (sink) {
            char text[0x100];
            if (field_0x754 == 0) {
                if (field_0x740->field_0x189)
                    g_kbGame->GetStringText(0x14d6, text, 0x80);
                else
                    g_kbGame->GetStringText(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11c6) KbMessage(text, 3.25f);
                if (msg) {
                    sink->Fn_0051B540(msg);
                    delete msg;
                }
            } else if (field_0x7a0 != field_0x7a2 && field_0x7a0 == 1 && field_0x740->field_0x189) {
                g_kbGame->GetStringText(0x14d7, text, 0x80);
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
                    case 1: g_kbGame->GetStringText(0x1481, text, 0x80); break;
                    case 2: g_kbGame->GetStringText(0x1482, text, 0x80); break;
                    case 3: g_kbGame->GetStringText(0x1483, text, 0x80); break;
                    case 4: g_kbGame->GetStringText(0x1484, text, 0x80); break;
                    case 5: g_kbGame->GetStringText(0x1485, text, 0x80); break;
                    case 6: g_kbGame->GetStringText(0x1486, text, 0x80); break;
                    case 7: g_kbGame->GetStringText(0x1487, text, 0x80); break;
                    case 8: g_kbGame->GetStringText(0x1488, text, 0x80); break;
                    case 9: g_kbGame->GetStringText(0x1489, text, 0x80); break;
                    case 10: g_kbGame->GetStringText(0x148a, text, 0x80); break;
                    case 11: g_kbGame->GetStringText(0x148b, text, 0x80); break;
                    default: g_kbGame->GetStringText(0x148c, text, 0x80); break;
                    }
                    msg = new(__FILE__, 0x1206) KbMessage(text, 3.25f);
                    if (msg) {
                        sink->Fn_0051B540(msg);
                        delete msg;
                    }
                } else {
                    g_kbGame->GetStringText(0x14e1, text, 0x80);
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
                    ((KbA5C4*)riderCharacter)->SetMotion(riderPoseHandles[15]);
                    ((KbA5C4*)riderCharacter)->field_0x10 = 0;
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
    if (!crashState && (field_0x734 || field_0x740->field_0xb8)) {
        if (field_0x740->field_0x18a)
            Fn_00413200(dt);
    } else {
        Bike::UnknownVirtualSlot49(dt);
    }
    if (field_0x740->field_0x50->field_0x244 == 3 && field_0x740->field_0x50->field_0x3b0 == this) {
        modelNode->SetPosition(field_0x1540);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(field_0x1540);
        field_0x154c = 1;
    } else {
        field_0x154c = 0;
    }
    field_0x1540 = position;
    field_0x153e = field_0x153c;
    field_0x7a2 = field_0x7a0;
}

// 0x00496F90: both bikes' collision objects ignore each other (AddIgnoredOwner = 0x00439410).
void KrustyBike::Fn_00496F90(KrustyBike* other)
{
    if (altBodyB) {
        ((KbCollider*)other->altBodyB)->AddIgnoredOwner((void*)altBodyB);
        ((KbCollider*)other->altBodyB)->AddIgnoredOwner((void*)altBodyA);
        ((KbCollider*)other->altBodyB)->AddIgnoredOwner((void*)frontWheel);
        ((KbCollider*)other->altBodyB)->AddIgnoredOwner((void*)rearWheel);
        ((KbCollider*)other->altBodyB)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->altBodyA)->AddIgnoredOwner((void*)altBodyB);
        ((KbCollider*)other->altBodyA)->AddIgnoredOwner((void*)altBodyA);
        ((KbCollider*)other->altBodyA)->AddIgnoredOwner((void*)frontWheel);
        ((KbCollider*)other->altBodyA)->AddIgnoredOwner((void*)rearWheel);
        ((KbCollider*)other->altBodyA)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->frontWheel)->AddIgnoredOwner((void*)altBodyB);
        ((KbCollider*)other->frontWheel)->AddIgnoredOwner((void*)altBodyA);
        ((KbCollider*)other->frontWheel)->AddIgnoredOwner((void*)frontWheel);
        ((KbCollider*)other->frontWheel)->AddIgnoredOwner((void*)rearWheel);
        ((KbCollider*)other->frontWheel)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->rearWheel)->AddIgnoredOwner((void*)altBodyB);
        ((KbCollider*)other->rearWheel)->AddIgnoredOwner((void*)altBodyA);
        ((KbCollider*)other->rearWheel)->AddIgnoredOwner((void*)frontWheel);
        ((KbCollider*)other->rearWheel)->AddIgnoredOwner((void*)rearWheel);
        ((KbCollider*)other->rearWheel)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)altBodyB);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)altBodyA);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)frontWheel);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)rearWheel);
        ((KbCollider*)other->field_0x604->a_0x38)->AddIgnoredOwner((void*)field_0x604->a_0x38);
        ((KbCollider*)altBodyB)->AddIgnoredOwner((void*)other->altBodyB);
        ((KbCollider*)altBodyB)->AddIgnoredOwner((void*)other->altBodyA);
        ((KbCollider*)altBodyB)->AddIgnoredOwner((void*)other->frontWheel);
        ((KbCollider*)altBodyB)->AddIgnoredOwner((void*)other->rearWheel);
        ((KbCollider*)altBodyB)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)altBodyA)->AddIgnoredOwner((void*)other->altBodyB);
        ((KbCollider*)altBodyA)->AddIgnoredOwner((void*)other->altBodyA);
        ((KbCollider*)altBodyA)->AddIgnoredOwner((void*)other->frontWheel);
        ((KbCollider*)altBodyA)->AddIgnoredOwner((void*)other->rearWheel);
        ((KbCollider*)altBodyA)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)frontWheel)->AddIgnoredOwner((void*)other->altBodyB);
        ((KbCollider*)frontWheel)->AddIgnoredOwner((void*)other->altBodyA);
        ((KbCollider*)frontWheel)->AddIgnoredOwner((void*)other->frontWheel);
        ((KbCollider*)frontWheel)->AddIgnoredOwner((void*)other->rearWheel);
        ((KbCollider*)frontWheel)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)rearWheel)->AddIgnoredOwner((void*)other->altBodyB);
        ((KbCollider*)rearWheel)->AddIgnoredOwner((void*)other->altBodyA);
        ((KbCollider*)rearWheel)->AddIgnoredOwner((void*)other->frontWheel);
        ((KbCollider*)rearWheel)->AddIgnoredOwner((void*)other->rearWheel);
        ((KbCollider*)rearWheel)->AddIgnoredOwner((void*)other->field_0x604->a_0x38);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->altBodyB);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->altBodyA);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->frontWheel);
        ((KbCollider*)field_0x604->a_0x38)->AddIgnoredOwner((void*)other->rearWheel);
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
    ((KbCollider*)other->collisionObject)->RemoveIgnoredOwner((void*)collisionObject);
    ((KbCollider*)other->collisionObject)->RemoveIgnoredOwner((void*)frontWheel);
    ((KbCollider*)other->collisionObject)->RemoveIgnoredOwner((void*)rearWheel);
    ((KbCollider*)other->collisionObject)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->frontWheel)->RemoveIgnoredOwner((void*)collisionObject);
    ((KbCollider*)other->frontWheel)->RemoveIgnoredOwner((void*)frontWheel);
    ((KbCollider*)other->frontWheel)->RemoveIgnoredOwner((void*)rearWheel);
    ((KbCollider*)other->frontWheel)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->rearWheel)->RemoveIgnoredOwner((void*)collisionObject);
    ((KbCollider*)other->rearWheel)->RemoveIgnoredOwner((void*)frontWheel);
    ((KbCollider*)other->rearWheel)->RemoveIgnoredOwner((void*)rearWheel);
    ((KbCollider*)other->rearWheel)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)collisionObject);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)frontWheel);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)rearWheel);
    ((KbCollider*)other->field_0x604->a_0x38)->RemoveIgnoredOwner((void*)field_0x604->a_0x38);
    if (other->UnknownVirtualSlot51())
        return;
    ((KbCollider*)collisionObject)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((KbCollider*)collisionObject)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((KbCollider*)collisionObject)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((KbCollider*)collisionObject)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)frontWheel)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((KbCollider*)frontWheel)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((KbCollider*)frontWheel)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((KbCollider*)frontWheel)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)rearWheel)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((KbCollider*)rearWheel)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((KbCollider*)rearWheel)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((KbCollider*)rearWheel)->RemoveIgnoredOwner((void*)other->field_0x604->a_0x38);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((KbCollider*)field_0x604->a_0x38)->RemoveIgnoredOwner((void*)other->rearWheel);
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
        for (other = field_0x740->NextBike(&cursor); other; other = field_0x740->NextBike(&cursor)) {
            if (other != this)
                Fn_00496F90(other);
        }
        if (((KbTrackA*)track)->field_0xb4 && !field_0x735) {
            for (int i = 0; i < ((KbTrackA*)track)->field_0xb4->count; i++) {
                KbPart* part = ((KbTrackA*)track)->field_0xb4->items + i;
                if (!(part->flags & 8))
                    part->obj->collider->RemoveIgnoredOwner((void*)collisionObject);
            }
        }
    } else {
        for (other = field_0x740->NextBike(&cursor); other; other = field_0x740->NextBike(&cursor)) {
            if (other != this && (!field_0x735 || !other->field_0x735))
                Fn_00497370(other);
        }
        if (((KbTrackA*)track)->field_0xb4 && !field_0x735) {
            for (int i = 0; i < ((KbTrackA*)track)->field_0xb4->count; i++) {
                if (!(((KbTrackA*)track)->field_0xb4->items[i].flags & 8)) {
                    KbPart* part = ((KbTrackA*)track)->field_0xb4->items + i;
                    KbPartObj* obj = part->obj;
                    obj->collider->AddIgnoredOwner((void*)altBodyB);
                    obj->collider->AddIgnoredOwner((void*)altBodyA);
                }
            }
        }
    }
}

// ==== wave 6 ====

// owner: bracket only (0x48d910; callers 0x48e584, 0x495276, 0x4955f8 are KrustyBike methods)
// Start the "knocked off" rider animation number idx: resets the stunt latches and switches both
// characters (rider and bike body) to the idx-th handle of the animSetA/animSetB tables.
void KrustyBike::Fn_0048D910(int idx)
{
    UnknownVirtualSlot41();
    field_0x430 = 1;
    field_0x431 = 0;
    field_0x432 = 1;
    field_0x1520 = 0;
    field_0x1524 = 0;
    field_0x1528 = 0;
    field_0x433 = (char)idx;
    ((KbA5C4*)riderCharacter)->SetMotion(animSetA[idx]);
    ((KbA5C4*)riderCharacter)->field_0x10 = 0;
    D3DIMSoultreeCharacter::SetMotion((Motion*)animSetB[idx]);
    chr_field_0x10 = 0;
}

// owner: bracket only (0x48d990; callers 0x48eff2, 0x48f115, 0x49562c are KrustyBike methods)
// Variant of Fn_0048D910 that keeps the pending score in field_0x1524 and uses animSetC/animSetD.
void KrustyBike::Fn_0048D990(int idx)
{
    field_0x430 = 1;
    field_0x1524 = field_0x1520;
    field_0x1520 = 0;
    field_0x433 = (char)(idx + 0x10);
    ((KbA5C4*)riderCharacter)->SetMotion(animSetC[idx]);
    D3DIMSoultreeCharacter::SetMotion((Motion*)animSetD[idx]);
    field_0x153f = 1;
}

// owner: bracket only (0x48dcd0; referenced by pointer from the collision setup, not called)
// Collision callback: the first object is this bike's collision object (owner at +0x60), the
// second one the other party (type tag at +0x64).  Mirrors SoultreeCollisionCallback with the
// KrustyBike filters (slot 51/+0x7a4 gates, type tags 0x64/0x65/0x66/0x6a/0x3e8).
void KrustyCollisionCallbackA(CollisionObject* self, CollisionObject* other)
{
    KrustyBike* bike = (KrustyBike*)self->ownerObject;
    int kind = other->ownerType;
    bike->lastCollisionType = kind;
    if ((bike->UnknownVirtualSlot51() || bike->field_0x7a4) && kind != 0x66 && kind != 0x6a)
        return;
    if (kind == 0x64) {
        KrustyBike* o = (KrustyBike*)other->ownerObject;
        if (o->UnknownVirtualSlot51() || o->field_0x7a4)
            return;
    } else if (kind == 0x65) {
        return;
    }
    bike->UnknownVirtualSlot38(bike->UnknownVirtualSlot52() && kind != 0x3e8, kind, other);
}

// owner: bracket only (0x48dd80; referenced by pointer from the collision setup)
// Second collision callback: for type tags 0x2711 / 0x69 the other object's +0x5c vector is added to
// the bike's position and the bike's collision object is refreshed before the common handling.
void KrustyCollisionCallbackB(CollisionObject* self, CollisionObject* other)
{
    KrustyBike* bike = (KrustyBike*)self->ownerObject;
    int kind = other->ownerType;
    if (bike->UnknownVirtualSlot51())
        return;
    if (bike->field_0x735)
        return;
    if (bike->field_0x7a4)
        return;
    if (kind == 0x64 || kind == 0x65)
        return;
    if (kind == 0x2711) {
        Vec3* p = &bike->position;
        Vec3* d = (Vec3*)other->contactRecord;
        p->x = p->x + d->x;
        p->y = d->y + p->y;
        p->z = d->z + p->z;
        bike->modelNode->SetPosition(*p);
        bike->collisionObject->UpdatePlacement();
    } else if (kind == 0x69) {
        Vec3* p = &bike->position;
        Vec3* d = (Vec3*)other->contactRecord;
        p->x = d->x + p->x;
        p->y = d->y + p->y;
        p->z = d->z + p->z;
        bike->modelNode->SetPosition(*p);
        bike->collisionObject->UpdatePlacement();
    }
    KrustyCollisionCallbackA(self, other);
}

// owner: bracket only (0x48e190; callers 0x48e285 and the camera code)
// Nearest other bike of the race (squared planar distance), skipping those with field_0x4a0 set;
// optionally returns the distance.
KrustyBike* KrustyBike::FindNearestRival(float* outDistance)
{
    int cursor = 0;
    KrustyBike* b = field_0x740->NextBike(&cursor);
    while (b && b->field_0x4a0)
        b = field_0x740->NextBike(&cursor);
    KrustyBike* best = 0;
    float bestSq = 3.4028235e38f;
    while (b) {
        if (b != this) {
            float dx = b->position.x - position.x;
            float dz = b->position.z - position.z;
            float sq = dx * dx;
            sq += dz * dz;
            if (sq < bestSq) {
                bestSq = sq;
                best = b;
            }
        }
        b = field_0x740->NextBike(&cursor);
        while (b && b->field_0x4a0)
            b = field_0x740->NextBike(&cursor);
    }
    if (outDistance)
        *outDistance = FastSqrt(bestSq);
    return best;
}

// owner: bracket only (0x48e280; callers 0x48e533, 0x495288)
// Look at the nearest rival: compute the bearing to it relative to the heading (savedYaw +0x50), wrap to
// +-pi and, when it is more than 0.698 rad off-axis, start the head-turn pose (clamped at +-2.7).
void KrustyBike::Fn_0048E280()
{
    nearestRival = FindNearestRival(0);
    if (!nearestRival)
        return;
    Vec3 d = nearestRival->position - position;
    float angle = (float)atan2(d.x, d.z) - savedYaw;
    if (angle < -3.14159274f)
        angle += 6.28318548f;
    else if (angle > 3.14159274f)
        angle -= 6.28318548f;
    field_0x1558 = angle;
    if (angle < 0)
        angle = -angle;
    if (angle <= 0.698f)
        return;
    UnknownVirtualSlot41();
    field_0x431 = 0;
    field_0x430 = 1;
    field_0x432 = 1;
    field_0x154d = 1;
    field_0x433 = (char)0xff;
    if (field_0x1558 > 2.7f) {
        field_0x1558 = 2.7f;
        field_0x1550 = 0.7f;
    } else if (field_0x1558 < -2.7f) {
        field_0x1558 = -2.7f;
        field_0x1550 = -0.7f;
    } else {
        field_0x1550 = field_0x1558 * 0.36963f * 0.7f;
    }
    field_0x1554 = 0;
    ((KbA5C4*)riderCharacter)->Fn_004A8BF0(riderPoseHandles[14], 0.5f);
    D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[14], 0.5f);
}

// 0x004933e0: the decoded network state (+0x1358..+0x1388) accumulates message 13's
// byte deltas; the result is copied into *state.
void KrustyBike::Fn_004933E0(const KbNetDelta* delta, KbNetState* state)
{
    Vec3 d0(delta->delta0[0] * 0.234375f, delta->delta0[1] * 0.234375f, delta->delta0[2] * 0.234375f);
    Vec3 d1(delta->delta1[0] * 0.234375f, delta->delta1[1] * 0.234375f, delta->delta1[2] * 0.234375f);
    Vec3 d2(delta->delta2[0] * 0.049087387f, delta->delta2[1] * 0.049087387f, delta->delta2[2] * 0.049087387f);
    field_0x1358 += d0;
    field_0x1364 += d1;
    field_0x1370 += d2;
    field_0x137c += delta->delta3[0] * 0.049087387f;
    field_0x1380 += delta->delta3[1] * 0.049087387f;
    field_0x1384 += delta->delta3[2] * 0.049087387f;
    int step;
    if (delta->step & 1)
        step = (delta->step >> 1) * 8;
    else
        step = delta->step >> 1;
    state->step = step;
    field_0x1388 += state->step;
    state->field_0x4c = field_0x1388;
    state->field_0x3c = field_0x1358;
    state->field_0x08 = field_0x1364;
    state->field_0x30 = field_0x1370;
    state->field_0x14 = field_0x137c;
    state->field_0x18 = field_0x1380;
    state->field_0x1c = field_0x1384;
    state->field_0x50 = delta->field_0x0e;
    state->field_0x48 = delta->field_0x12;
}

// 0x00495ff0: ends a trick. The angle (+0x1530) drops its part below 100, the
// score is angle x multiplier (+0x1538, +0.5 for a +0x153f landing); the race
// handler hears of it. In game mode 0 (or 4 with +0x2eb8 for the +0x568 racer's
// bike) the score is kept in +0x788 and returned; otherwise the bonus rules
// (+0x3444) turn it into a capped bonus, shown to the +0x50 racer's bike.
// Near miss (871 of 895 bytes): retail shares one stack slot between score and
// gain and one between the integer and float bonus, and loads +0x153f before
// the fsubr; the declaration orders tried do not reproduce that.
float KrustyBike::Fn_00495FF0()
{
    field_0x1530 -= (float)fmod(field_0x1530, 100.0);
    if (field_0x153f)
        field_0x1538 += 0.5f;
    float score = field_0x1530 * field_0x1538;
    if (field_0x740->field_0x38 == this && field_0x740->handler)
        field_0x740->handler->ReportTrickScore(this, score);
    if (!g_kbGame->field_0x2d74 ||
        (g_kbGame->field_0x2d74 == 4 && g_kbGame->field_0x2eb8 && g_kbGame->field_0x568->field_0xa8 == this)) {
        if (field_0x740->field_0x38 == this) {
            if (field_0x740->field_0xbc)
                field_0x740->field_0xbc->Fn_0048D1E0(field_0x1530, field_0x1538);
            int points = (int)score;
            if (field_0x788 > points)
                points = field_0x788;
            field_0x788 = points;
        }
        Fn_0048D8B0();
        return score;
    }
    KbBonusTable* rules = g_kbGame->field_0x3444;
    if (rules) {
        float step = field_0x1530 * 0.000016f;
        if (!(step < 1.0f))
            step = 1.0f;
        int base = rules->base[rules->index];
        int points = (int)(base * rules->baseScale);
        float gain = 0.0f;
        float bonus = points;
        float limit = points * rules->limitScale;
        if (field_0x7b4 * 400.0f < limit) {
            float total = step + field_0x7b4;
            if (total * 400.0f > limit) {
                field_0x7b4 = limit * 0.0025f;
            } else {
                field_0x7b4 = total;
                gain = step * 400.0f;
            }
        }
        if (this == field_0x740->field_0x50->field_0x3b4) {
            KbMsgSink* sink = g_kbGame->field_0x570->Fn_0045D340();
            char text[0x80];
            char line[0x80];
            if (gain > 0.0f) {
                g_kbGame->GetStringText(0x14d8, text, 0x80);
                sprintf(line, "%s %.0f.00", text, gain);
                KbMessage* message = new(__FILE__, 0xa14) KbMessage(line, 3.25f);
                if (message) {
                    sink->Fn_0051B540(message);
                    delete message;
                }
            } else {
                g_kbGame->GetStringText(0x14d9, text, 0x80);
                sprintf(line, "%s %.0f.00)", text, bonus * rules->limitScale);
                KbMessage* message = new(__FILE__, 0xa1d) KbMessage(line, 3.25f);
                if (message) {
                    sink->Fn_0051B540(message);
                    delete message;
                }
            }
        }
    }
    Fn_0048D8B0();
    return 0.0f;
}

// 0x004977a0 (GameObject slot 10, reached through the virtual base): the per-frame update.
// Picks the update path by the network state and game mode, re-targets the mode 4 player
// record, ends the start-up ghosting once no rival overlaps this bike, and prints the camera
// bike's figures (with a 0..88 "TimeTo60" stopwatch) on the debug overlay.
static int s_kbTimeTo60Armed = 1;   // 0x0056d0a0
static float s_kbTimeTo60;          // 0x0067c3b0
static float s_kbLastTimeTo60;      // 0x0067c3b4

int KrustyBike::GameObjectVirtualSlot10(float dt)
{
    if (g_kbGame->field_0x18 == 1 && !field_0x740->field_0x18a)
        return 1;
    if (g_kbGame->field_0x3428) {
        if (field_0x740->field_0x18a) {
            Fn_00493660(dt, 1);
            if (field_0x740->field_0x1dc == 4 && field_0x740->field_0x1e0 == 4)
                GameObject::GameObjectVirtualSlot10(dt);
        }
    } else if (field_0x735) {
        Fn_00493660(dt, 0);
        if (field_0x740->field_0x3fa)
            Fn_00492AD0(dt, 1);
        GameObject::GameObjectVirtualSlot10(dt);
    } else if (g_kbGame->field_0x2d70 == 4) {
        if (field_0x736) {
            Fn_00493660(dt, 1);
            if (Fn_00495C00())
                GameObject::GameObjectVirtualSlot10(dt);
        } else {
            Vehicle::GameObjectVirtualSlot10(dt);
            if (field_0x740->field_0x3fa)
                Fn_00492AD0(dt, 1);
        }
    } else {
        Vehicle::GameObjectVirtualSlot10(dt);
        if (field_0x740->field_0x3fa)
            Fn_00492AD0(dt, 1);
        if (g_kbGame->field_0x18 > 1)
            Fn_00492AD0(dt, 0);
    }
    if (g_kbGame->field_0x2d74 == 4) {
        KbPlayer* p = g_kbGame->field_0x570->Fn_0045D2B0();
        if (p->field_0xdc && p->field_0xa8 == this && crashState) {
            if (g_kbGame->field_0x8->field_0x10)
                p->Fn_004A9E80(0, this, 1);
            else
                Fn_004925A0(0, 1);
        }
    }
    if (field_0x7a5 && !field_0x735 && field_0x740->field_0x18a) {
        int cursor = 0;
        int overlap = 0;
        KrustyBike* bike;
        for (bike = field_0x740->NextBike(&cursor); bike; bike = field_0x740->NextBike(&cursor)) {
            if (!bike->field_0x4a0 && !bike->UnknownVirtualSlot51() && bike != this) {
                overlap = ((KbBody*)collisionObject)->TestMeshBounds((KbBody*)bike->collisionObject);
                if (overlap)
                    break;
            }
        }
        if (!overlap) {
            spawnProtected = 0;
            field_0x7a5 = 0;
            if (field_0x736) {
                ((KbXform*)modelNode)->Fn_00444DE0(field_0x15e8);
                ((KbXform*)riderCharacter->c_0x1a0)->Fn_00444DE0(field_0x15e8);
            }
            Fn_00496E30(0);
        }
    }
    if (g_kbGame->field_0x38 && field_0x740->field_0x38 == this) {
        if (field_0x1410 < 0) {
            int line = g_kbGame->field_0x38->lineCount++;
            field_0x1410 = line;
        }
        g_kbGame->field_0x38->Title(field_0x1410, "KrustyBike");
        g_kbGame->field_0x38->Print(field_0x1410, "Position/Heading=");
        g_kbGame->field_0x38->Print(field_0x1410, "%.2f, %.2f, %.2f / (%.0f)", position.x, position.y,
                                    position.z, bodyYaw * 57.29578f);
        if (s_kbTimeTo60Armed) {
            if (field_0x434 > 0.2f) {
                if (field_0x434 < 88.0f) {
                    s_kbTimeTo60 += g_kbGame->field_0x2f0;
                } else {
                    s_kbLastTimeTo60 = s_kbTimeTo60;
                    s_kbTimeTo60Armed = 0;
                }
            }
        } else if (field_0x434 < 0.2f) {
            s_kbTimeTo60 = 0;
            s_kbTimeTo60Armed = 1;
        }
        g_kbGame->field_0x38->Print(field_0x1410, "Last TimeTo60 = %.2f  seconds", s_kbLastTimeTo60);
        g_kbGame->field_0x38->Print(field_0x1410, "TimeTo60 = %.2f  seconds", s_kbTimeTo60);
    }
    return 1;
}

// 0x00492670: sends this bike's state as message 1 (to every peer, or into the recorder when
// `record` is set) and keeps what was sent in `state`; without a recorder, peers also get
// message 10 every two seconds in modes 0 and 4.
void KrustyBike::Fn_00492670(KbBikeState* state, float dt, int record)
{
    if (!g_kbGame->field_0x8 && (!record || !netRecorder))
        return;
    KbBikeMessage message;
    message.position = position;
    message.roll = bodyRoll;
    message.pitch = bodyPitch;
    message.yaw = bodyYaw;
    message.velocity = velocity;
    message.angularVelocity = angularVelocity;
    message.angularVelocity.y += turnRate;
    message.field_0x04 = field_0x7a0;
    message.field_0x06 = field_0x790;
    message.field_0x54 = field_0x11c0;
    message.field_0x20 = field_0x74c;
    message.field_0x28 = field_0x750;
    message.field_0x2c = field_0x770;
    message.field_0x01 = field_0x784;
    message.poseIndex = poseIndex;
    message.poseState = poseState;
    message.poseParam = poseParam * 100.0f;
    message.poseLeanBlend = poseLeanBlend * 100.0f;
    message.poseBlend = poseBlend * 100.0f;
    if (field_0x430) {
        if (field_0x433 >= 0)
            message.motion = field_0x433 + 1;
        else
            message.motion = 17;
    } else {
        message.motion = 0;
    }
    message.flag7 = field_0x478;
    message.flag8 = field_0x479;
    message.flag9 = field_0x153c;
    message.crashDirection = crashDirection;
    message.crashed = crashState;
    message.flag4 = UnknownVirtualSlot51();
    message.flag5 = field_0x7a4 != 0;
    message.flag6 = field_0x78c;
    if (g_kbGame->field_0x2d74 == 2 || g_kbGame->field_0x2d74 == 3)
        message.field_0x53 = (unsigned char)field_0x7a0;
    else
        message.field_0x53 = field_0x7b8;
    message.time = UnknownFunction4bfa80();
    message.field_0x02 = (short)message.time - (short)state->time;
    if (!record) {
        if (g_kbGame->field_0x8)
            g_kbGame->field_0x8->Send(1, &message, sizeof(message), field_0x11bc, 0);
    } else if (netRecorder) {
        netRecorder->Fn_004E8720(1, field_0x11bc, &message, sizeof(message));
    }
    state->position = position;
    state->roll = bodyRoll;
    state->pitch = bodyPitch;
    state->yaw = bodyYaw;
    state->velocity = velocity;
    state->angularVelocity = angularVelocity;
    state->angularVelocity.y += turnRate;
    state->time = message.time;
    state->velocityError = g_kbZeroVec;
    state->positionError = g_kbZeroVec;
    state->angularVelocityError = g_kbZeroVec;
    state->rollError = 0;
    state->pitchError = 0;
    state->yawError = 0;
    state->timer = 0;
    if (!record) {
        if (g_kbGame->field_0x8 &&
            (!g_kbGame->field_0x2d74 || (g_kbGame->field_0x2d74 == 4 && g_kbGame->field_0x2eb8))) {
            field_0x1604 -= g_kbGame->field_0x2f0;
            if (field_0x1604 < 0.0f) {
                KbBikePing ping;
                ping.field_0x04 = field_0x768;
                ping.field_0x01 = field_0x11c0;
                g_kbGame->field_0x8->Send(10, &ping, sizeof(ping), field_0x11bc, 0);
                field_0x1604 = 2.0f;
            }
        }
    } else if (netRecorder && field_0x740->field_0x3fb) {
        KbBikePing ping;
        ping.field_0x04 = field_0x768;
        ping.field_0x01 = field_0x11c0;
        netRecorder->Fn_004E8720(10, field_0x11bc, &ping, sizeof(ping));
    }
}

// |v| < limit, the range a message 13 delta may carry (written out at every use in retail):
// 30 for velocity and position, 2 pi for angular velocity and the three angles.
#define KB_DELTA_FITS(v, limit) (((v) < 0.0f ? -(v) : (v)) < (limit))

static inline void KbCopy(Vec3* out, const Vec3& a)
{
    out->x = a.x;
    out->y = a.y;
    out->z = a.z;
}

static inline void KbSub(Vec3* out, const Vec3& a, const Vec3& b)
{
    out->x = a.x - b.x;
    out->y = a.y - b.y;
    out->z = a.z - b.z;
}

// 0x00492ad0: sends (or, with `record`, records) message 13, this bike's change since the
// last state as byte deltas plus the pose and flags, carrying what the bytes lose over to the
// next message. A full message 1 (0x00492670) goes instead when a delta does not fit, 1 s
// has passed, or the full interval is due; peers otherwise get one every short interval.
// Near miss (2312 of 2315 bytes, 631 of 641 instructions aligned; frame 0x34 exact): all
// that differs is the record-interval branch and what it drags along. Retail keeps the full
// message block at the end, entered by `jne deltas; jmp sendFull` from the record branch and
// by `push 0; jmp sendFull+1` (record known zero) from the network branch, and the register
// choice in the network branch follows. With the shared `sendFull` label VC6 places the block
// at the end too but inverts the record test (`je sendFull; jmp deltas`) and makes the
// network branch jump there conditionally instead of duplicating the push. Inline calls,
// a literal 0, a boolean flag, explicit gotos to `deltas`, swapped branches, an inverted
// test with an empty then-block and an explicit final return were tried.
// Shapes that mattered: `KbSub` through an out-pointer keeps `diff` a 12-byte slot (the
// operator form is scalar-replaced and repacks the frame); one `ping` next to `message`
// overlays the dead `d` slots; the angular-velocity and angle limits are 2 pi, not 30;
// `scratchVector[1] += turnRate` and `KbCopy` before `state->angularVelocity.y += turnRate`
// make VC6 load turnRate first (a plain aggregate copy before `.y +=` yields the
// read-modify-write order instead).
void KrustyBike::Fn_00492AD0(float dt, int record)
{
    if (!g_kbGame->field_0x8 && (!record || !netRecorder))
        return;
    KbBikeState* state;
    if (record) {
        state = &recordState;
        state->timer += g_kbGame->field_0x2f0;
        if (state->timer > g_kbGame->fullRecordPacketIntervalSec)
            goto sendFull;
    } else {
        state = &netState;
        state->timer += g_kbGame->field_0x2f0;
        field_0x15d0 += g_kbGame->field_0x2f0;
        if (field_0x15d0 < g_kbGame->shortNetPacketIntervalSec)
            return;
        field_0x15d0 = 0;
        if (state->timer > g_kbGame->fullNetPacketIntervalSec)
            goto sendFull;
    }
    {
        KbBikeDeltaMessage message;
        KbBikePing ping;
        Vec3 d;
        Vec3 diff;
        message.field_0x16 = field_0x11c0;
        KbSub(&diff, velocity, state->velocity);
        d = diff + state->velocityError;
        if (KB_DELTA_FITS(d.x, 30.0f) && KB_DELTA_FITS(d.y, 30.0f) && KB_DELTA_FITS(d.z, 30.0f)) {
            message.velocity[0] = (signed char)(d.x * 4.2666669f);
            state->velocityError.x = d.x - message.velocity[0] * 0.234375f;
            message.velocity[1] = (signed char)(d.y * 4.2666669f);
            state->velocityError.y = d.y - message.velocity[1] * 0.234375f;
            message.velocity[2] = (signed char)(d.z * 4.2666669f);
            state->velocityError.z = d.z - message.velocity[2] * 0.234375f;
            KbSub(&diff, position, state->position);
            d = diff + state->positionError;
            if (KB_DELTA_FITS(d.x, 30.0f) && KB_DELTA_FITS(d.y, 30.0f) && KB_DELTA_FITS(d.z, 30.0f)) {
                message.position[0] = (signed char)(d.x * 4.2666669f);
                state->positionError.x = d.x - message.position[0] * 0.234375f;
                message.position[1] = (signed char)(d.y * 4.2666669f);
                state->positionError.y = d.y - message.position[1] * 0.234375f;
                message.position[2] = (signed char)(d.z * 4.2666669f);
                state->positionError.z = d.z - message.position[2] * 0.234375f;
                scratchVector = angularVelocity;
                scratchVector[1] += turnRate;
                KbSub(&diff, scratchVector, state->angularVelocity);
                d = diff + state->angularVelocityError;
                if (KB_DELTA_FITS(d.x, 6.28318548f) && KB_DELTA_FITS(d.y, 6.28318548f) && KB_DELTA_FITS(d.z, 6.28318548f)) {
                    message.angularVelocity[0] = (signed char)(d.x * 20.371832f);
                    state->angularVelocityError.x = d.x - message.angularVelocity[0] * 0.049087387f;
                    message.angularVelocity[1] = (signed char)(d.y * 20.371832f);
                    state->angularVelocityError.y = d.y - message.angularVelocity[1] * 0.049087387f;
                    message.angularVelocity[2] = (signed char)(d.z * 20.371832f);
                    state->angularVelocityError.z = d.z - message.angularVelocity[2] * 0.049087387f;
                    float a = bodyYaw - state->yaw + state->yawError;
                    if (KB_DELTA_FITS(a, 6.28318548f)) {
                        message.yaw = (signed char)(a * 20.371832f);
                        state->yawError = a - message.yaw * 0.049087387f;
                        a = bodyRoll - state->roll + state->rollError;
                        if (KB_DELTA_FITS(a, 6.28318548f)) {
                            message.roll = (signed char)(a * 20.371832f);
                            state->rollError = a - message.roll * 0.049087387f;
                            a = bodyPitch - state->pitch + state->pitchError;
                            if (KB_DELTA_FITS(a, 6.28318548f)) {
                                message.pitch = (signed char)(a * 20.371832f);
                                state->pitchError = a - message.pitch * 0.049087387f;
                                unsigned int now = UnknownFunction4bfa80();
                                unsigned int elapsed = now - state->time;
                                if (elapsed < 1000) {
                                    if (elapsed < 0x80) {
                                        message.coarse = 0;
                                        message.count = elapsed;
                                    } else {
                                        message.coarse = 1;
                                        message.count = elapsed >> 3;
                                    }
                                    message.poseIndex = poseIndex;
                                    message.poseState = poseState;
                                    message.poseParam = poseParam * 100.0f;
                                    message.poseLeanBlend = poseLeanBlend * 100.0f;
                                    message.poseBlend = poseBlend * 100.0f;
                                    if (field_0x430) {
                                        if (field_0x433 >= 0)
                                            message.motion = field_0x433 + 1;
                                        else
                                            message.motion = 17;
                                    } else {
                                        message.motion = 0;
                                    }
                                    message.flag7 = field_0x478;
                                    message.flag8 = field_0x479;
                                    message.flag9 = field_0x153c;
                                    message.crashDirection = crashDirection;
                                    message.crashed = crashState;
                                    message.flag4 = UnknownVirtualSlot51();
                                    message.flag5 = field_0x7a4 != 0;
                                    message.flag6 = field_0x78c;
                                    if (g_kbGame->field_0x2d74 == 2 || g_kbGame->field_0x2d74 == 3)
                                        message.field_0x11 = (unsigned char)field_0x7a0;
                                    else
                                        message.field_0x11 = field_0x7b8;
                                    if (!record) {
                                        if (g_kbGame->field_0x8)
                                            g_kbGame->field_0x8->Send(13, &message, sizeof(message), field_0x11bc, record);
                                    } else if (netRecorder) {
                                        netRecorder->Fn_004E8720(13, field_0x11bc, &message, sizeof(message));
                                    }
                                    state->position = position;
                                    state->roll = bodyRoll;
                                    state->pitch = bodyPitch;
                                    state->yaw = bodyYaw;
                                    state->velocity = velocity;
                                    KbCopy(&state->angularVelocity, angularVelocity);
                                    state->angularVelocity.y += turnRate;
                                    state->time = now;
                                    if (!record) {
                                        if (g_kbGame->field_0x8 &&
                                            (!g_kbGame->field_0x2d74 || (g_kbGame->field_0x2d74 == 4 && g_kbGame->field_0x2eb8))) {
                                            field_0x1604 -= g_kbGame->field_0x2f0;
                                            if (field_0x1604 < 0.0f) {
                                                ping.field_0x04 = field_0x768;
                                                ping.field_0x01 = field_0x11c0;
                                                g_kbGame->field_0x8->Send(10, &ping, sizeof(ping), field_0x11bc, 0);
                                                field_0x1604 = 2.0f;
                                            }
                                        }
                                    } else if (netRecorder && field_0x740->field_0x3fb) {
                                        ping.field_0x04 = field_0x768;
                                        ping.field_0x01 = field_0x11c0;
                                        netRecorder->Fn_004E8720(10, field_0x11bc, &ping, sizeof(ping));
                                    }
                                    return;
                                }
                            }
                        }
                    }
                }
                Fn_00492670(state, g_kbGame->field_0x2f0, record);
                return;
            }
            Fn_00492670(state, g_kbGame->field_0x2f0, record);
            return;
        }
    }
sendFull:
    Fn_00492670(state, g_kbGame->field_0x2f0, record);
}

// Byte-coded pose values of message 1 are 0..255 (tier 3: 1/255 constant at 0x005511e4).
#define KB_BYTE_SCALE (1.0f / 255.0f)

// Retail tests the byte deltas with a copying conditional negate (`mov edx, eax; jns; neg
// edx`), not the `abs` intrinsic (`cdq; xor; sub`).
#define KB_ABS(x) ((x) < 0 ? -(x) : (x))

// Provisional: `dist2` is a squared change in units of the rate limit; the change is
// smoothed (rate limited) only when it is more than one frame's worth and below the warp
// threshold (seconds), and warped otherwise.
static inline int KbWithinWarpBand(float dist2, float frame, float warp)
{
    return dist2 > frame * frame && dist2 < warp * warp;
}

// 0x00493660: the network update of a remote bike. Places the bike from the received
// states (states[0] newest): either interpolating between the two states around the
// current time (g_kbInterpolate) or extrapolating the newest one, with optional latency
// hiding, rate limiting and warping of the velocity and position; then applies the pose,
// lap and crash flags of the state and runs the physics bookkeeping of a step.
// `dt` is unused; `a` is set for the live game (lap and race fields are taken over).
//
// Partial (8545 bytes retail): the instruction streams align block for block with the
// retail frame (0x58) and stack slots. What fixed the shape so far: 4-byte locals are
// declared at function scope in retail slot order (ratio, dist2, len, reset) ahead of the
// 12-byte vectors (step, delta, savedPos, localFwd, localUp); `float clock` hoists the
// race clock read above the replay-mode test; `float ff = f * f` keeps VC6 from
// refactoring `f*f*50 + len*f`; the named `scale` stays FPU resident across the three
// `delta * scale` products; the pose clamps follow both modes; KB_ABS on a variable
// gives the copying `jns; neg`; `states[0]->` is re-read after every call and store
// (no `s` alias); the lap/pose flags are 1/0 if-else stores; the crash-flip test reads
// bodyUp.y; `char m` keeps the motion compare byte sized.
// Still different: retail keeps 1 in ebp from the second SetAxesPtr call on and 0 in edi
// (candidate: 0 in ebp, 1 in ebx, and a zero register through the tail where retail
// uses `test`/`push 0`), which also forces `c` through a stack byte; retail stores
// `(len+50)*(len+50)` before the AllowWarping test; the velocity y/z `fld s; fmul` order;
// retail recomputes `c - b` for the pose lerps while the candidate reuses `d`; the retail
// 100.0f lives at 0x5505ec while the shared bindings bind it to 0x551420 (0x492670).
void KrustyBike::Fn_00493660(float dt, int a)
{
    KbBikeNetState* cur;
    KbBikeNetState* s;
    Vec3 step;
    Vec3 delta;
    Vec3 savedPos;
    Vec3 localFwd;
    Vec3 localUp;
    float ratio;
    float dist2;
    float len;
    int reset;

    justLanded = 0;
    justReset = 0;
    reset = 0;
    if (g_kbGame->field_0x2f0 <= 0.0f)
        return;
    cur = states[g_kbGame->field_0x3428 ? 3 : 2];
    if (cur->position.x != 0.0f && cur->position.y != 0.0f && cur->position.z != 0.0f) {
        if (g_kbGame->field_0x3428 == 0 && g_kbGame->field_0x2d70 != 4) {
            if (field_0x15f8 != states[0]->timeReceived) {
                field_0x15f8 = states[0]->timeReceived;
                if (field_0x15f4 == 0) {
                    field_0x15f4 = 1;
                } else {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                    while (field_0x15ec > field_0x15f0 && field_0x15f0 > 0.0f)
                        field_0x15ec -= field_0x15f0;
                    if (field_0x15ec < 0.0f)
                        field_0x15ec = 0.0f;
                }
            }
            if (field_0x15ec > field_0x15f0) {
                if (field_0x15f4 == 1) {
                    field_0x15f4 = 0;
                    while (field_0x15ec > field_0x15f0 && field_0x15f0 > 0.0f)
                        field_0x15ec -= field_0x15f0;
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                } else {
                    field_0x15ec = field_0x15f0;
                }
            }
        } else {
            // replay: the race clock, less a fixed lag, picks the state interval
            float clock = field_0x740->field_0x1b8;
            if (g_kbGame->field_0x2d70 == 4) {
                float adj;
                field_0x15ec = (clock - 0.17f) - (float)states[2]->time * 0.001f;
                adj = field_0x740->field_0x1b8 - 0.17f;
                if (adj > (float)states[0]->time * 0.001f && field_0x740->field_0x1e4 == -2) {
                    field_0x15ec = field_0x15f0;
                } else if (adj > (float)states[1]->time * 0.001f) {
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15ec -= (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                } else {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                }
            } else {
                float adj;
                field_0x15ec = (clock - 0.27f) - (float)states[3]->time * 0.001f;
                adj = field_0x740->field_0x1b8 - 0.27f;
                if (adj > (float)states[0]->time * 0.001f && field_0x740->field_0x1e4 == -2) {
                    field_0x15ec = field_0x15f0;
                } else if (adj < (float)states[2]->time * 0.001f) {
                    field_0x15f0 = (float)(states[2]->time - states[3]->time) * 0.001f;
                    field_0x15fc = states[2];
                    field_0x1600 = states[3];
                } else if (adj < (float)states[1]->time * 0.001f) {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15ec -= (float)(states[2]->time - states[3]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                } else {
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15ec -= (float)(states[1]->time - states[3]->time) * 0.001f;
                    if (field_0x15ec > field_0x15f0)
                        field_0x15ec = field_0x15f0;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                }
                if (field_0x15ec < 0.0f)
                    field_0x15ec = 0.0f;
            }
        }
        if (!g_kbInterpolate) {
            unsigned int now = UnknownFunction4bfa80();
            if (!g_kbUseTimeReceived)
                field_0x15ec = ((float)now - (float)states[0]->time - field_0x11c4) * 0.001f;
            else
                field_0x15ec = ((float)now - (float)states[0]->timeReceived) * 0.001f;
            if (!g_kbAllowNegative && field_0x15ec < 0.0f)
                field_0x15ec = 0.0f;
            if (g_kbUseExtrapLimit && field_0x15ec > g_kbExtrapLimit)
                field_0x15ec = g_kbExtrapLimit;
            field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
        }
        if (field_0x15f0 > 0.0f)
            ratio = field_0x15ec / field_0x15f0;
        else
            ratio = 0.0f;

        // velocity
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold)
                    delta = states[0]->velocity - velocity;
                else
                    delta = (states[0]->velocity - states[1]->velocity) * ratio + states[0]->velocity - velocity;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // warp when the change (in units of the 100/s rate) is below a frame's
                    // worth or beyond the warp threshold; otherwise move at the rate
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 * 0.0001f, g_kbGame->field_0x2f0, g_kbWarpThreshold)) {
                        velocity += delta;
                    } else {
                        step = delta * 100.0f * g_kbGame->field_0x2f0;
                        velocity += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    velocity += delta;
                }
            } else {
                velocity = states[0]->velocity;
            }
        } else {
            velocity = (field_0x15fc->velocity - field_0x1600->velocity) * ratio + field_0x1600->velocity;
        }
        smoothedVerticalAccel = (velocity.y - prevVelocity.y) / g_kbGame->field_0x2f0;
        prevSpeed = linearSpeed;
        len = KbLength(velocity);
        linearSpeed = len;

        // position
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold)
                    delta = states[0]->position - position;
                else
                    delta = states[0]->velocity * field_0x15ec + states[0]->position - position;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // the position may move at speed + 50 per second
                    // the position may move at speed + 50 per second; retail stores limitSq
                    // before the AllowWarping test, VC6 propagates it here into the division
                    float limitSq = (len + 50.0f) * (len + 50.0f);
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 / limitSq, g_kbGame->field_0x2f0, g_kbWarpThreshold)) {
                        position += delta;
                    } else {
                        float ff = g_kbGame->field_0x2f0 * g_kbGame->field_0x2f0;
                        float scale = ff * 50.0f + len * g_kbGame->field_0x2f0;
                        step = delta * scale;
                        position += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    position += delta;
                }
            } else {
                position = states[0]->position;
            }
        } else {
            position = (field_0x15fc->position - field_0x1600->position) * ratio + field_0x1600->position;
        }

        // orientation
        if (!g_kbInterpolate) {
            UnknownFunction4b5d00(&bodyForward, &bodyUp, states[0]->roll, states[0]->pitch, states[0]->yaw);
            s = states[1];
        } else {
            UnknownFunction4b5d00(&bodyForward, &bodyUp, field_0x15fc->roll, field_0x15fc->pitch, field_0x15fc->yaw);
            s = field_0x1600;
        }
        UnknownFunction4b5d00(&localFwd, &localUp, s->roll, s->pitch, s->yaw);
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold) {
                    angularVelocity = states[0]->angularVelocity;
                } else {
                    bodyForward = (bodyForward - localFwd) * ratio + localFwd;
                    bodyUp = (bodyUp - localUp) * ratio + localUp;
                    angularVelocity = (states[0]->angularVelocity - states[1]->angularVelocity) * ratio + states[0]->angularVelocity;
                }
            } else {
                angularVelocity = states[0]->angularVelocity;
            }
        } else {
            bodyForward = (bodyForward - localFwd) * ratio + localFwd;
            bodyUp = (bodyUp - localUp) * ratio + localUp;
            angularVelocity = (field_0x15fc->angularVelocity - field_0x1600->angularVelocity) * ratio + field_0x1600->angularVelocity;
        }

        savedPos = position;
        modelNode->SetPosition(position);
        modelNode->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        UnknownVirtualSlot34();
        OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                     &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
        if (!field_0x6fc && !field_0x430) {
            savedForward = bodyForward;
            savedUp = bodyUp;
            savedYaw = bodyYaw;
            savedPitch = bodyPitch;
            savedRoll = bodyRoll;
            savedSinRoll = bodySinRoll;
            savedCosRoll = bodyCosRoll;
            savedCosPitch = bodyCosPitch;
            savedSinPitch = bodySinPitch;
        } else {
            poseNode->GetAxesIn(0, &savedForward, &savedUp);
            OrientationAnglesFromVectors(savedForward, savedUp, &savedYaw, &savedPitch, &savedRoll,
                                         &savedSinRoll, &savedCosRoll, &savedCosPitch, &savedSinPitch);
        }
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        centerNode->GetPositionIn(0, &centerOfMass);
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
        ((KbSink*)frontWheel->w_0x2b0)->UnknownVirtualSlot0();
        ((KbSink*)rearWheel->w_0x2ac)->UnknownVirtualSlot0();
        PlaceWheels();
        allWheelsInContact = wheelsInContact == wheelCount;
        anyWheelInContact = wheelsInContact != 0;
        UpdateWheelsInContact();
        {
            bool settled = !anyWheelInContact;
            UnknownVirtualSlot71(settled);
            airborne = settled;
            if (settled)
                landingLatched = 0;
        }
        movingForward = 1;

        // wheel roll from the horizontal speed
        if (!(field_0x740->field_0x3f8 && field_0x740->field_0x1e4 != -2)) {
            float d;
            float roll;
            scratchVector = velocity;
            scratchVector.y = 0.0f;
            d = SquareMagnitude(scratchVector);
            if (d == 1.0f)
                roll = 1.0f;
            else
                roll = FastSqrt(d);
            ((KbWheel*)frontWheel)->SetRollDistance(roll * g_kbGame->field_0x2f0);
            ((KbWheel*)rearWheel)->SetRollDistance(roll * g_kbGame->field_0x2f0);
        }
        if (!g_kbInterpolate)
            turnRate = states[0]->angularVelocity.y;
        else
            turnRate = (field_0x15fc->angularVelocity.y - field_0x1600->angularVelocity.y) * ratio + field_0x1600->angularVelocity.y;

        // pose parameter; the clamp follows both modes
        if (!g_kbInterpolate) {
            int d = field_0x15fc->poseParam - field_0x1600->poseParam;
            if (KB_ABS(d) >= 0x40) {
                poseParam = (float)states[0]->poseParam * KB_BYTE_SCALE;
            } else {
                float p = (float)states[0]->poseParam * KB_BYTE_SCALE;
                poseParam = p;
                if (!(g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold))
                    poseParam = (p - (float)states[1]->poseParam * KB_BYTE_SCALE) * ratio + p;
            }
        } else {
            int b = field_0x1600->poseParam;
            unsigned char c = field_0x15fc->poseParam;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!field_0x1400) {
                    field_0x1400 = 1;
                    field_0x1408 = field_0x1600->poseParam;
                }
                poseParam = (float)field_0x1408 * KB_BYTE_SCALE;
            } else if (field_0x1400) {
                if (c <= 3 || (c >= 0x7e && c <= 0x82))
                    field_0x1400 = 0;
                poseParam = (float)field_0x1408 * KB_BYTE_SCALE;
            } else if (KB_ABS(d) <= 0x18) {
                poseParam = (float)((int)((float)(c - b) * ratio) + b) * KB_BYTE_SCALE;
            } else {
                poseParam = (float)b * KB_BYTE_SCALE;
            }
        }
        if (poseParam > 1.0f)
            poseParam = 1.0f;
        else if (poseParam < 0.0f)
            poseParam = 0.0f;

        // pose lean blend
        if (!g_kbInterpolate) {
            int a = states[0]->poseLeanBlend;
            int d = a - states[1]->poseLeanBlend;
            float v;
            d = KB_ABS(d);
            v = (float)a * KB_BYTE_SCALE;
            if (d >= 0x40) {
                poseLeanBlend = v;
            } else {
                poseLeanBlend = v;
                if (!(g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold))
                    poseLeanBlend = (v - (float)states[1]->poseLeanBlend * KB_BYTE_SCALE) * ratio + v;
            }
        } else {
            int b = field_0x1600->poseLeanBlend;
            unsigned char c = field_0x15fc->poseLeanBlend;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!field_0x1404) {
                    field_0x1404 = 1;
                    field_0x1408 = field_0x1600->poseLeanBlend;   // retail stores +0x1408 but reads +0x140c
                }
                poseLeanBlend = (float)field_0x140c * KB_BYTE_SCALE;
            } else if (field_0x1404) {
                if (c <= 2 || (c >= 0x7f && c <= 0x81))
                    field_0x1404 = 0;
                poseLeanBlend = (float)field_0x140c * KB_BYTE_SCALE;
            } else if (KB_ABS(d) <= 0x20) {
                poseLeanBlend = (float)((int)((float)(c - b) * ratio) + b) * KB_BYTE_SCALE;
            } else {
                poseLeanBlend = (float)b * KB_BYTE_SCALE;
            }
        }
        if (poseLeanBlend > 1.0f)
            poseLeanBlend = 1.0f;
        else if (poseLeanBlend < 0.0f)
            poseLeanBlend = 0.0f;

        // pose blend; only the interpolated value is smoothed through the steer axis
        if (!g_kbInterpolate) {
            int b1 = states[1]->poseBlend;
            int b0 = states[0]->poseBlend;
            int d = b0 - b1;
            if (KB_ABS(d) >= 0x40)
                poseBlend = (float)b1 * KB_BYTE_SCALE;
            else
                poseBlend = (float)b0 * KB_BYTE_SCALE;
        } else {
            int b = field_0x1600->poseBlend;
            int c = field_0x15fc->poseBlend;
            int d = c - b;
            float v;
            float lim;
            float t;
            float r;
            if (KB_ABS(d) >= 0x40)
                v = (float)b;
            else
                v = (float)((int)((float)(c - b) * ratio) + b);
            v *= KB_BYTE_SCALE;
            lim = steerAxis->l_0x4;
            t = g_kbGame->field_0x2f0;
            if (t >= lim)
                t = lim;
            r = t / steerAxis->l_0x4;
            steerAxis->l_0x8 = r;
            v = (v - steerAxis->steerValue) * r + steerAxis->steerValue;
            steerAxis->steerValue = v;
            poseBlend = v;
        }
        if (poseBlend > 1.0f)
            poseBlend = 1.0f;
        else if (poseBlend < 0.0f)
            poseBlend = 0.0f;

        // pose, lap and crash flags (retail keeps two copies of this block, one per mode;
        // they differ in the +0x478 flag only)
        if (!g_kbInterpolate) {
            int lean;
            int prevCrash;
            poseIndex = states[0]->poseIndex;
            lean = poseIndex == 11 || poseIndex == 12;
            if (field_0x6fc && !lean) {
                UnknownVirtualSlot41();
                field_0x431 = 0;
            }
            field_0x6fc = lean;
            poseState = states[0]->poseState;
            if (g_kbGame->field_0x560) {
                if (field_0x7b8 == states[0]->field_0x53 - 1 || (field_0x7b8 == ((KbTrack*)g_kbGame->field_0x560)->field_0xac - 1 && states[0]->field_0x53 == 0))
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a) {
                    int prev = field_0x7b8;
                    field_0x7b8 = states[0]->field_0x53;
                    if (prev != field_0x7b8) {
                        if (field_0x7b8 == 0)
                            field_0x7a0++;
                        field_0x7bc = (field_0x7b8 + 1) % ((KbTrack*)g_kbGame->field_0x560)->field_0xac;
                        if (this == field_0x740->field_0x50->field_0x3b4)
                            ((KbTrack*)g_kbGame->field_0x560)->UnknownFunction404df0(field_0x7b8, field_0x7bc, this);
                    }
                }
            } else {
                if (!states[0]->field_0x5c)
                    states[0]->field_0x04 = (short)((field_0x7a0 & 0xff00) | states[0]->field_0x53);
                if ((unsigned short)field_0x7a0 == (unsigned short)states[0]->field_0x04 - 1)
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a)
                    field_0x7a0 = states[0]->field_0x04;
            }
            if (states[0]->field_0x5c) {
                field_0x74c = states[0]->field_0x20;
                field_0x750 = states[0]->field_0x28;
                if (a) {
                    field_0x770 = states[0]->field_0x2c;
                    field_0x784 = states[0]->field_0x01;
                }
            }
            if (!field_0x736) {
                UnknownVirtualSlot50(states[0]->flag4, 5.0f, 0);
                if (field_0x7a5) {
                    Fn_00496E30(0);
                    field_0x7a5 = 0;
                }
            }
            if (states[0]->flag6)
                field_0x78c = 1;
            else
                field_0x78c = 0;
            prevCrash = crashState;
            crashState = states[0]->crashed;
            field_0x478 = states[0]->flag7;
            field_0x479 = states[0]->flag8;
            field_0x153c = states[0]->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                field_0x464 = bodyUp.y < 0.0f;
                field_0x45c = savedYaw;
                if (field_0x430)
                    field_0x604->Method_0x00532310();
                else
                    field_0x604->Method_0x00532220(states[0]->crashDirection);
                field_0x433 = 0;
                UnknownVirtualSlot41();
                field_0x431 = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                field_0x604->Method_0x005327c0();
                Fn_00496DA0();
                field_0x430 = 0;
                field_0x433 = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                field_0x430 = 0;
                field_0x433 = 0;
            } else if (!field_0x430) {
                char m = states[0]->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!field_0x153f) {
                char m = field_0x1600->motion;
                if (m > 0x11 && m - 0x11 == field_0x433)
                    Fn_0048D990(field_0x433);
            }
        } else {
            int lean;
            int prevCrash;
            poseIndex = field_0x1600->poseIndex;
            lean = poseIndex == 11 || poseIndex == 12;
            if (field_0x6fc && !lean) {
                UnknownVirtualSlot41();
                field_0x431 = 0;
            }
            field_0x6fc = lean;
            poseState = field_0x1600->poseState;
            if (g_kbGame->field_0x560) {
                if (field_0x7b8 == field_0x1600->field_0x53 - 1 || (field_0x7b8 == ((KbTrack*)g_kbGame->field_0x560)->field_0xac - 1 && field_0x1600->field_0x53 == 0))
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a) {
                    int prev = field_0x7b8;
                    field_0x7b8 = field_0x1600->field_0x53;
                    if (prev != field_0x7b8) {
                        if (field_0x7b8 == 0)
                            field_0x7a0++;
                        field_0x7bc = (field_0x7b8 + 1) % ((KbTrack*)g_kbGame->field_0x560)->field_0xac;
                        if (this == field_0x740->field_0x50->field_0x3b4)
                            ((KbTrack*)g_kbGame->field_0x560)->UnknownFunction404df0(field_0x7b8, field_0x7bc, this);
                    }
                }
            } else {
                if (!field_0x1600->field_0x5c)
                    field_0x1600->field_0x04 = (short)((field_0x7a0 & 0xff00) | field_0x1600->field_0x53);
                if ((unsigned short)field_0x7a0 == (unsigned short)field_0x1600->field_0x04 - 1)
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a)
                    field_0x7a0 = field_0x1600->field_0x04;
            }
            if (field_0x1600->field_0x5c) {
                field_0x74c = field_0x1600->field_0x20;
                field_0x750 = field_0x1600->field_0x28;
                if (a) {
                    field_0x770 = field_0x1600->field_0x2c;
                    field_0x784 = field_0x1600->field_0x01;
                }
            }
            if (!field_0x736) {
                UnknownVirtualSlot50(field_0x1600->flag4, 5.0f, 0);
                if (field_0x7a5) {
                    Fn_00496E30(0);
                    field_0x7a5 = 0;
                }
            }
            if (field_0x1600->flag6)
                field_0x78c = 1;
            else
                field_0x78c = 0;
            prevCrash = crashState;
            crashState = field_0x1600->crashed;
            if (!field_0x740->field_0x3f8 && field_0x740->field_0x1e4 != -2)
                field_0x478 = field_0x1600->flag7;
            else
                field_0x478 = 0;
            field_0x479 = field_0x1600->flag8;
            field_0x153c = field_0x1600->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                field_0x464 = bodyUp.y < 0.0f;
                field_0x45c = savedYaw;
                if (field_0x430)
                    field_0x604->Method_0x00532310();
                else
                    field_0x604->Method_0x00532220(field_0x1600->crashDirection);
                field_0x433 = 0;
                UnknownVirtualSlot41();
                field_0x431 = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                field_0x604->Method_0x005327c0();
                Fn_00496DA0();
                field_0x430 = 0;
                field_0x433 = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                field_0x430 = 0;
                field_0x433 = 0;
            } else if (!field_0x430) {
                char m = field_0x1600->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!field_0x153f) {
                char m = field_0x1600->motion;
                if (m > 0x11 && m - 0x11 == field_0x433)
                    Fn_0048D990(field_0x433);
            }
        }

        UnknownVirtualSlot30();
        justReset = reset;
        attachmentResetPending = reset;
        UnknownVirtualSlot21();
        prevCrashState = crashState;
        UnknownVirtualSlot102(g_kbGame->field_0x2f0);
    } else {
        savedPos = position;
    }

    if (g_kbInterpolate)
        field_0x15ec = g_kbGame->field_0x2f0 + field_0x15ec;
    if (!a && !field_0x7a4) {
        if (field_0x15ec >= g_kbStallThreshold) {
            field_0x138c = g_kbStallHoldSec;
            UnknownVirtualSlot50(1, 0.0f, 0);
        } else if (field_0x138c > 0.0f) {
            field_0x138c -= g_kbGame->field_0x2f0;
            if (field_0x138c < 0.0f)
                field_0x138c = 0.0f;
        } else if (!field_0x736) {
            UnknownVirtualSlot50(0, 0.0f, 0);
        }
    }
    if (!g_kbGame->field_0x3428) {
        collisionObject->UpdatePlacement();
        if (g_kbGame->field_0x2d74 == 4)
            UnknownVirtualSlot28(1);
    }
    prevVelocity = velocity;
    position = savedPos;
    lastStepTime = g_kbGame->field_0x2f0;
}
