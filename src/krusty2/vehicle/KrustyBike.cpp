// KrustyBike.cpp -- reconstruction of D:\aardvark\VC\krusty2\KrustyBike.cpp
// (string 0x0056d3ac, tier 1).
//
// Extent (strong inference): 0x0048d77c..0x00497cb0.  The unit's own __FILE__ xrefs
// lie in 0x0048fe58..0x00496b19 (the loader 0x0048fc80, the destructor 0x00491540,
// 0x00495ff0, slot 49 0x00496790); every function here is a KrustyBike method or a
// TU-local helper contiguous with them; the .CRT$XCU 155-158 Math3D set (0x00491190)
// sits mid-file and the two vtordisp thunks 0x00497c30/0x00497ca0 close it.
// KrustyBikeCamera (0x00497cb0..) follows as its own unit.  Two KrustyBike methods,
// 0x00413200 and 0x00414370, were emitted inside BikeAI.cpp's code; their near misses
// are in samples/physics/bikeai/BikeAINearMisses.cpp.  This unit's near misses are in
// samples/physics/krustybike/KrustyBikeNearMisses.cpp,
// which includes this file.  Member and helper names are provisional (tier 3); see
// KrustyBikeTypes.h.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle/KrustyBike.h"
#include "math/FastMath.h"
#include "collision/CollisionObject.h"
#include "core/MemTag.h"

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

// 0x00495ff0: ends a trick. The angle (+0x1530) drops its part below 100, the
// score is angle x multiplier (+0x1538, +0.5 for a +0x153f landing); the race
// handler hears of it. In game mode 0 (or 4 with +0x2eb8 for the +0x568 racer's
// bike) the score is kept in +0x788 and returned; otherwise the bonus rules
// (+0x3444) turn it into a capped bonus, shown to the +0x50 racer's bike.
// The redundant parentheses around the (float) fmod term are significant: VC6 keeps them
// as an IL node, which schedules retail's `mov al, [+0x153f]` before the fsubr
// (docs/VC6_OPERAND_ORDER.md section 3); without them the load follows it.
float KrustyBike::Fn_00495FF0()
{
    field_0x1530 -= ((float)fmod(field_0x1530, 100.0));
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
                sprintf(line, "%s %.0f.00)", text, points * rules->limitScale);
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
// Look at the nearest rival: the bearing to it relative to the heading (savedYaw +0x50),
// wrapped to +-pi; when it is more than 0.698 rad off-axis the head-turn pose starts
// (+0x430/+0x432/+0x154d set, +0x433 = -1; slot 102 runs it), its target clamped at +-2.7.
// The bearing is `(float)(atan2(...))` minus the yaw: the parenthesised operand of the cast
// keeps the dead duplicate of the x argument around fpatan and pops it before the yaw fsub,
// as retail does (see docs/VC6_OPERAND_ORDER.md).
void KrustyBike::Fn_0048E280()
{
    nearestRival = FindNearestRival(0);
    if (!nearestRival)
        return;
    Vec3 d = nearestRival->position - position;
    float angle = (float)(atan2(d.x, d.z)) - savedYaw;
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
        field_0x1550 = (field_0x1558 * 0.36963f) * 0.7f;
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

// 0x005541e8: a TU-local constant (it sits in KrustyBike.cpp's .rdata between 1/3600 and the
// first vtable pointers, not in the shared `__real@4@` pool at 0x005507d4 that the remote
// update uses for its own 0.001f).  Only the loader's three millisecond settings read it.
static const float kbSecondsPerMillisecond = 0.001f;

// 0x0048fc80 (5366 bytes of code + 2 pad + the 4-entry jump table at 0x00491178 = 5384, ret
// 0x7c; exact): the bike loader.  Not a vtable entry (KrustyBike's and Bike's slot 40 both
// hold 0x00503de0); Bike's own loader 0x004079c0 (ret 0xa0) is called directly with `this`,
// like Vehicle::LoadVehicle.  Five `new(__FILE__, line)` sites give the function its SEH
// frame (states 0..4; handler 0x0054bd32).  Literal names: the sixteen trick motions with and
// without the `b` prefix, their `Alt` variants, BikeAndRider.col, Bike.col, RiderMorph.mbf,
// the eleven network setting names and the Collision memory tag.
//
// Shapes the match depends on:
//  * the `setup ? setup->x : 0` call arguments are jump-threaded in groups by type: the six
//    float fields zero their temps with immediate stores and form their own chain (ending in
//    a `jmp +0`), the int ones share the zero register; so KbBikeSetup 0xc..0x20 are floats;
//  * `Vec3 points[2]` lives at function scope next to `localFlag` (frame slot 0x40, frame
//    0xbc) and is cleared with two memsets: block scope moved it to 0x14, named locals or
//    brace-init produced immediates or temporary copies;
//  * the dead `cmp g->field_0x2d74, 0` survives as an if/else with identical 100.0f arms
//    (tail-merged), which also restores the register rotation of the whole tail;
//  * a local `BikeA640* axis` keeps the steer axis in one register for its three stores and
//    0.42210004f is needed for retail's 0x3ed81d7f (0.4221f rounds to 0x3ed81d7e);
//  * 0.001f comes from the TU-local kbSecondsPerMillisecond, not the shared pool.
GameObject* KrustyBike::Load(int a1, int a2, const char* name, const SoultreeLoadDesc* desc, int a5, int a6,
                             Vec3 a7, Vec3 a8, Vec3 a9, int a10, VehicleInputMap* map, unsigned char kind,
                             unsigned char a13, KbRace* race, int a15, int a16, int netId,
                             const KbBikeSetup* setup, int a19, int a20, int a21, int a22,
                             KbRecorder* recorder, void* a24, int withRider)
{
    field_0x7c4 = 1.1f;
    field_0x7c8 = 1.3f;
    field_0x7cc = 1.5f;
    field_0x7d0 = 25.0f;
    field_0x7d4 = 15.0f;
    field_0x7d8 = 5.0f;
    field_0x7dc = 0;
    inputMap = map;
    field_0x11bc = netId;
    if (kind == 0xff) {
        field_0x736 = 1;
        field_0x734 = 0;
    } else {
        field_0x734 = kind;
    }
    field_0x11c0 = kind;
    field_0x735 = a13;
    memset(&netState, 0, sizeof(netState));
    netState.timer = g_kbGame->fullNetPacketIntervalSec;
    field_0x15d0 = g_kbGame->shortNetPacketIntervalSec;
    field_0x740 = race;
    netRecorder = recorder;
    field_0x15d8 = a24;
    memset(&recordState, 0, sizeof(recordState));
    recordState.timer = 1.7014117e38f;

    int localFlag;
    Vec3 points[2];
    if (!field_0x734 && !field_0x735 && !field_0x736) {
        int type = g_kbGame->field_0x33fc[g_kbGame->field_0x33fc->field_0x0].field_0x28;
        if (type == -6 || type == -7 || type == -12 || type == -13)
            localFlag = 1;
        else
            localFlag = 0;
    } else {
        localFlag = 0;
    }

    float setupFloat;
    field_0x738 = 250;
    setupFloat = 0.0f;
    field_0x737 = 0;
    if (setup) {
        KbBikeSetup copy = *setup;
        setupFloat = (float)copy.field_0x8;
        field_0x738 = copy.field_0x0;
        field_0x737 = copy.field_0x4;
    }
    if (field_0x736) {
        field_0x15e8 = (KbObj128*)new(__FILE__, 0x690) KbGhostMod(1);
        GameObject::Method_0x00469190(field_0x15e8, -1);
    }
    field_0x744 = 0;
    ((KbBikeLoader*)this)->Fn_004079C0(a1, a2, name, desc, a5, a6, a7, a8, a9, a10, map, a15, a16,
                                       setup == 0,
                                       setup ? (void*)setup->field_0x24 : 0,
                                       setup ? setup->field_0x50 : 0,
                                       setup ? setup->field_0x54 : 0,
                                       setup ? setup->field_0x58 : 0,
                                       setup ? setup->field_0xc : 0,
                                       setup ? setup->field_0x10 : 0,
                                       setup ? setup->field_0x14 : 0,
                                       setup ? setup->field_0x18 : 0,
                                       setup ? setup->field_0x1c : 0,
                                       setup ? setup->field_0x20 : 0,
                                       a19, a20, a21, localFlag,
                                       g_kbGame->field_0x3344, g_kbGame->field_0x3380, g_kbGame->field_0x33bc,
                                       a22, setupFloat, 0);
    if (field_0x735) {
        BikeA640* axis = steerAxis;
        axis->steerValue = 0.5f;
        axis->l_0x4 = 0.42210004f;
        axis->l_0x8 = 1.0f;
    }
    if (field_0x734 || field_0x736) {
        field_0x5bc = 0;
        field_0x5c0 = 0;
    }
    field_0x7a8 = 3.5f;
    field_0x1520 = 0;
    field_0x1524 = 0;
    field_0x1528 = 0;
    field_0x1550 = 0;
    field_0x1554 = 0;
    crashTimer = 0;
    states[0] = &stateBuffers[0];
    states[1] = &stateBuffers[1];
    states[2] = &stateBuffers[2];
    states[3] = &stateBuffers[3];
    memset(states[0], 0, sizeof(KbBikeNetState));
    memset(states[1], 0, sizeof(KbBikeNetState));
    memset(states[2], 0, sizeof(KbBikeNetState));
    memset(states[3], 0, sizeof(KbBikeNetState));
    stateBuffers[0].timeReceived = 0;
    stateBuffers[1].timeReceived = 0;
    stateBuffers[2].timeReceived = 0;
    stateBuffers[3].timeReceived = 0;
    field_0x15fc = states[1];
    field_0x1600 = states[2];
    UnknownVirtualSlot43();

    if (g_kbGame->field_0x2d74 != 0 && g_kbGame->field_0x2d74 != 5 && g_kbGame->field_0x2d74 != 1) {
        heapBufferA = DebugCalloc(100, 4, __FILE__, 0x6fa);
        if (!heapBufferA)
            return 0;
    } else {
        heapBufferA = 0;
    }
    if (g_kbGame->field_0x2d74 == 1 || g_kbGame->field_0x2d74 == 5) {
        heapBufferB = DebugCalloc(600, 4, __FILE__, 0x702);
        if (!heapBufferB)
            return 0;
    } else {
        heapBufferB = 0;
    }

    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                 &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
    field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);

    animSetB[0] = (int)FindMotion("bBarKneel", 1);
    animSetB[1] = (int)FindMotion("bSplitX", 1);
    animSetB[2] = (int)FindMotion("bBKahuna", 1);
    animSetB[3] = (int)FindMotion("bCordova", 1);
    animSetB[4] = (int)FindMotion("bHeelClicker", 1);
    animSetB[5] = (int)FindMotion("bNacNac", 1);
    animSetB[6] = (int)FindMotion("bSuperMan", 1);
    animSetB[7] = (int)FindMotion("bAirWalk", 1);
    animSetB[8] = (int)FindMotion("bBarHop", 1);
    animSetB[9] = (int)FindMotion("bSeatGrab", 1);
    animSetB[10] = (int)FindMotion("bSaranWrap", 1);
    animSetB[11] = (int)FindMotion("bCliffHanger", 1);
    animSetB[12] = (int)FindMotion("bHeartAttack", 1);
    animSetB[13] = (int)FindMotion("bTailGrab", 1);
    animSetB[14] = (int)FindMotion("bLazyBoy", 1);
    animSetB[15] = (int)FindMotion("bDCanCan", 1);
    animSetD[0] = (int)FindMotion("bBarKneelAlt", 1);
    animSetD[1] = 0;
    animSetD[2] = 0;
    animSetD[3] = 0;
    animSetD[4] = (int)FindMotion("bHeelClickerAlt", 1);
    animSetD[5] = (int)FindMotion("bNacNacAlt", 1);
    animSetD[6] = (int)FindMotion("bSuperManAlt", 1);
    animSetD[7] = 0;
    animSetD[8] = 0;
    animSetD[9] = 0;
    animSetD[10] = (int)FindMotion("bSaranWrapAlt", 1);
    animSetD[11] = 0;
    animSetD[12] = 0;
    animSetD[13] = 0;
    animSetD[14] = 0;
    animSetD[15] = (int)FindMotion("bDCanCanAlt", 1);
    animSetA[0] = (int)((KbA5C4*)riderCharacter)->FindMotion("BarKneel", 1);
    animSetA[1] = (int)((KbA5C4*)riderCharacter)->FindMotion("SplitX", 1);
    animSetA[2] = (int)((KbA5C4*)riderCharacter)->FindMotion("BKahuna", 1);
    animSetA[3] = (int)((KbA5C4*)riderCharacter)->FindMotion("Cordova", 1);
    animSetA[4] = (int)((KbA5C4*)riderCharacter)->FindMotion("HeelClicker", 1);
    animSetA[5] = (int)((KbA5C4*)riderCharacter)->FindMotion("NacNac", 1);
    animSetA[6] = (int)((KbA5C4*)riderCharacter)->FindMotion("SuperMan", 1);
    animSetA[7] = (int)((KbA5C4*)riderCharacter)->FindMotion("AirWalk", 1);
    animSetA[8] = (int)((KbA5C4*)riderCharacter)->FindMotion("BarHop", 1);
    animSetA[9] = (int)((KbA5C4*)riderCharacter)->FindMotion("SeatGrab", 1);
    animSetA[10] = (int)((KbA5C4*)riderCharacter)->FindMotion("SaranWrap", 1);
    animSetA[11] = (int)((KbA5C4*)riderCharacter)->FindMotion("CliffHanger", 1);
    animSetA[12] = (int)((KbA5C4*)riderCharacter)->FindMotion("HeartAttack", 1);
    animSetA[13] = (int)((KbA5C4*)riderCharacter)->FindMotion("TailGrab", 1);
    animSetA[14] = (int)((KbA5C4*)riderCharacter)->FindMotion("LazyBoy", 1);
    animSetA[15] = (int)((KbA5C4*)riderCharacter)->FindMotion("DCanCan", 1);
    animSetC[0] = (int)((KbA5C4*)riderCharacter)->FindMotion("BarKneelAlt", 1);
    animSetC[1] = 0;
    animSetC[2] = 0;
    animSetC[3] = 0;
    animSetC[4] = (int)((KbA5C4*)riderCharacter)->FindMotion("HeelClickerAlt", 1);
    animSetC[5] = (int)((KbA5C4*)riderCharacter)->FindMotion("NacNacAlt", 1);
    animSetC[6] = (int)((KbA5C4*)riderCharacter)->FindMotion("SuperManAlt", 1);
    animSetC[7] = 0;
    animSetC[8] = 0;
    animSetC[9] = 0;
    animSetC[10] = (int)((KbA5C4*)riderCharacter)->FindMotion("SaranWrapAlt", 1);
    animSetC[11] = 0;
    animSetC[12] = 0;
    animSetC[13] = 0;
    animSetC[14] = 0;
    animSetC[15] = (int)((KbA5C4*)riderCharacter)->FindMotion("DCanCanAlt", 1);

    altBodyA = (KbObj128*)new(__FILE__, 0x77e) CollisionObject(1);
    ((CollisionObject*)altBodyA)->Configure(a1, 1, 1, 1);
    ((CollisionObject*)altBodyA)->LoadShape(modelNode->firstChild, "BikeAndRider.col");
    GameObject::Method_0x00469190((GraphicsTest*)(CollisionObject*)altBodyA, -1);
    altBodyB = (KbObj128*)new(__FILE__, 0x784) CollisionObject(1);
    ((CollisionObject*)altBodyB)->Configure(a1, 1, 1, 1);
    ((CollisionObject*)altBodyB)->LoadShape(modelNode->firstChild, "Bike.col");
    GameObject::Method_0x00469190((GraphicsTest*)(CollisionObject*)altBodyB, -1);
    altBodyB->field_0xc.UnknownVirtualSlot4();
    ((CollisionObject*)altBodyA)->ownerType = 100;
    ((CollisionObject*)altBodyB)->ownerType = 100;
    ((CollisionObject*)altBodyA)->ownerObject = this;
    ((CollisionObject*)altBodyB)->ownerObject = this;
    collisionObject = (CollisionObject*)altBodyA;
    collisionObject->Fn_00435fe0();
    ((KbA604*)field_0x604)->Fn_00530680(altBodyA);
    ((KbA604*)field_0x604)->Fn_00530680(altBodyB);
    ((CollisionHullShape*)collisionObject->shape)->sceneNode = (int)modelNode->firstChild;
    ((KbCollider*)frontWheel)->SetUseBroadphase(1);
    ((KbCollider*)rearWheel)->SetUseBroadphase(1);
    ((KbCollider*)field_0x604->a_0x38)->SetUseBroadphase(1);
    if (field_0x735) {
        if (g_kbGame->field_0x2d84) {
            ((CollisionObject*)altBodyB)->onHitCallback = 0;
            ((CollisionObject*)altBodyB)->onHitByCallback = 0;
            ((CollisionObject*)altBodyA)->onHitCallback = 0;
            ((CollisionObject*)altBodyA)->onHitByCallback = 0;
        } else {
            ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
            altBodyB->field_0xc.UnknownVirtualSlot4();
            altBodyA->field_0xc.UnknownVirtualSlot4();
        }
    } else if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84) {
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
        altBodyB->field_0xc.UnknownVirtualSlot4();
        altBodyA->field_0xc.UnknownVirtualSlot4();
    } else {
        ((CollisionObject*)altBodyB)->onHitCallback = KrustyCollisionCallbackA;
        ((CollisionObject*)altBodyB)->onHitByCallback = KrustyCollisionCallbackB;
        ((CollisionObject*)altBodyA)->onHitCallback = KrustyCollisionCallbackA;
        ((CollisionObject*)altBodyA)->onHitByCallback = KrustyCollisionCallbackB;
    }
    field_0x604->Method_0x005327c0();
    field_0x1540 = position;
    if (field_0x735) {
        BikeA640* axis = steerAxis;
        axis->steerValue = 0.5f;
        axis->l_0x4 = 0.42210004f;
        axis->l_0x8 = 1.0f;
    }
    field_0x154c = 0;
    field_0x15cc = 0;
    field_0x11c4 = 0;

    g_kbLatencyHiding = g_kbGame->UnknownVirtualSlot20("LatencyHiding", 1);
    g_kbRateLimiting = g_kbGame->UnknownVirtualSlot20("RateLimiting", 1);
    g_kbAllowWarping = g_kbGame->UnknownVirtualSlot20("AllowWarping", 1);
    g_kbUseLatencyThreshold = g_kbGame->UnknownVirtualSlot20("UseLatencyThreshold", 0);
    g_kbUseExtrapLimit = g_kbGame->UnknownVirtualSlot20("UseExtrapLimit", 1);
    g_kbExtrapLimit = (float)g_kbGame->UnknownVirtualSlot20("ExtrapLimit", 100) * kbSecondsPerMillisecond;
    g_kbLatencyThreshold = (float)g_kbGame->UnknownVirtualSlot20("LatencyThreshold", 500) * kbSecondsPerMillisecond;
    g_kbWarpThreshold = (float)g_kbGame->UnknownVirtualSlot20("WarpThreshold", 200) * kbSecondsPerMillisecond;
    g_kbUseTimeReceived = g_kbGame->UnknownVirtualSlot20("UseTimeReceived", 1);
    g_kbAllowNegative = g_kbGame->UnknownVirtualSlot20("AllowNegative", 1);
    g_kbInterpolate = g_kbGame->UnknownVirtualSlot20("Interpolate", 1);
    field_0x15ec = 0;
    field_0x15f0 = 0;
    field_0x15f4 = 1;
    field_0x15f8 = 0;
    if (field_0x738 <= 125)
        field_0x79c = 0;
    else if (field_0x738 < 500)
        field_0x79c = 1;
    else
        field_0x79c = 2;

    if (field_0x734) {
        field_0x15e4 = 1;
        field_0x810 = 1;
        if (g_kbGame->field_0x2d74 == 0) {
            switch (g_kbGame->field_0x60c) {
            case 3:
            case 4:
                field_0x15dc = field_0x740->field_0xa4;
                break;
            case 1:
                field_0x15dc = field_0x740->field_0x9c;
                break;
            case 2:
                field_0x15dc = field_0x740->field_0xa0;
                break;
            }
        } else {
            field_0x15dc = 2.0f;
        }
        int prevTag = g_MemTagStack->Push("Collision");
        field_0x15e0 = new(__FILE__, 0x82f) CollisionObject(1);
        field_0x15e0->Configure((int)field_0x18, 1, 1, 0);
        field_0x15e0->ignoreVegetation = 1;
        GameObject::Method_0x00469190((GraphicsTest*)field_0x15e0, -1);
        memset(&points[0], 0, sizeof(Vec3));
        memset(&points[1], 0, sizeof(Vec3));
        points[0].y = 4.0f;
        points[1].y = -1.0f;
        field_0x15e0->SetMeshShape(1, points);
        field_0x15e0->SetIgnoreListMode(1);
        field_0x15e0->AddIgnoredOwner(altBodyB);
        field_0x15e0->AddIgnoredOwner(altBodyA);
        field_0x15e0->AddIgnoredOwner(rearWheel);
        field_0x15e0->AddIgnoredOwner(frontWheel);
        g_MemTagStack->Pop(prevTag);
        field_0x80c = 0;
        field_0x814 = 0;
        field_0x818 = 3000.0f;
        field_0x820 = 3000.0f;
        field_0x81c = 0;
        if (g_kbGame->field_0x2d74 != 5 && g_kbGame->field_0x2d74 != 1)
            field_0x824 = KbRandUnit() * 14.0f - 7.0f;
        else
            field_0x824 = KbRandUnit() * 6.0f - 3.0f;
    } else {
        field_0x15e0 = 0;
        field_0x810 = 0;
        field_0x15e4 = 0;
    }

    // Retail keeps a dead `cmp g_kbGame->field_0x2d74, 0` here: two arms that store the
    // same constant, tail-merged by VC6 (the original values were presumably tuned equal).
    if (g_kbGame->field_0x2d74 == 0)
        field_0x778 = 100.0f;
    else
        field_0x778 = 100.0f;
    field_0x7e0 = Vec3(0.0f, 0.0f, 0.0f);
    field_0x7ec = Vec3(0.0f, 0.0f, 0.0f);
    field_0x7f8 = 0;
    field_0x7fc = 0;
    ((KbCollider*)frontWheel)->SetIgnoreListMode(1);
    ((KbCollider*)frontWheel)->AddIgnoredOwner(altBodyB);
    ((KbCollider*)frontWheel)->AddIgnoredOwner(altBodyA);
    ((KbCollider*)frontWheel)->AddIgnoredOwner(rearWheel);
    ((KbCollider*)rearWheel)->SetIgnoreListMode(1);
    ((KbCollider*)rearWheel)->AddIgnoredOwner(altBodyB);
    ((KbCollider*)rearWheel)->AddIgnoredOwner(altBodyA);
    ((KbCollider*)rearWheel)->AddIgnoredOwner(frontWheel);
    ((KbCollider*)altBodyB)->SetIgnoreListMode(1);
    ((KbCollider*)altBodyB)->AddIgnoredOwner(frontWheel);
    ((KbCollider*)altBodyB)->AddIgnoredOwner(rearWheel);
    ((KbCollider*)altBodyA)->SetIgnoreListMode(1);
    ((KbCollider*)altBodyA)->AddIgnoredOwner(frontWheel);
    ((KbCollider*)altBodyA)->AddIgnoredOwner(rearWheel);
    if (field_0x740->field_0x18c) {
        ((KbXform*)modelNode)->UnknownFunction444d40(0);
        ((KbXform*)riderCharacter->c_0x1a0)->UnknownFunction444d40(0);
        ((KbXform*)modelNode)->UnknownFunction444d00(0);
        ((KbXform*)riderCharacter->c_0x1a0)->UnknownFunction444d00(0);
    } else {
        ((KbXform*)modelNode)->UnknownFunction444d40(1);
        ((KbXform*)riderCharacter->c_0x1a0)->UnknownFunction444d40(1);
        ((KbXform*)modelNode)->UnknownFunction444d00(1);
        ((KbXform*)riderCharacter->c_0x1a0)->UnknownFunction444d00(1);
    }
    if (field_0x736) {
        UnknownVirtualSlot50(1, 0.0f, 1);
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
    }
    if (withRider) {
        ((KbA5C4*)riderCharacter)->ApplyRestPose();
        ((SoultreeObject*)riderCharacter->c_0x1a0)->TranslateIn(0, Vec3(0.0f, 0.0f, 0.0f));
        Vec3 axisY(0.0f, 1.0f, 0.0f);
        Vec3 axisZ(0.0f, 0.0f, 1.0f);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesIn(0, &axisZ, &axisY, 0, 1);
        KbMorphMod* morph = new(__FILE__, 0x891) KbMorphMod(1);
        morph->UnknownFunction4a33b0((void*)a1, "RiderMorph.mbf", (SoultreeObject*)riderCharacter->c_0x1a0);
        GameObject::Method_0x00469190(morph, -1);
        ((KbXform*)riderCharacter->c_0x1a0)->UnknownFunction444eb0(morph);
    }
    UnknownVirtualSlot102(0.001f);
    return this;
}
