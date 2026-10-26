#include "../../reconstructed/BikeRaceCore.h"
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
#include "../../reconstructed/TrackGameGlobal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle/KrustyBike.h"
#include "../../reconstructed/GearRatios.h"
#include "../../reconstructed/Net.h"
// Recorder.h needs core Win32 synchronization types here, not OLE headers.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "../../reconstructed/Recorder.h"
#include "../../reconstructed/EventManager.h"
#include "../../reconstructed/DebugOverlay.h"
#include "../../reconstructed/TrackGame.h"
#include "math/FastMath.h"
#include "collision/CollisionObject.h"
#include "../../reconstructed/Wrecker.h"
#include "broadphase/Quadtree.h"
#include "core/MemTag.h"
#include "soultree/SoultreePhysicsObject.h"
#include "../../reconstructed/GhostModCore.h"
#include "../../reconstructed/MorphBastardModifierCore.h"

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
    if (crashReason == 9 || motionActive) {
        wrecker->StartTumble();
    } else {
        wrecker->StartAnimatedWreck(crashDirection);
    }
    UnknownVirtualSlot101();
}

void KrustyBike::UnknownVirtualSlot1(float a)
{
    Bike::UnknownVirtualSlot1(a);
    Fn_0048D8B0();
    field_0x152c = 0;
    headTurningToRival = 0;
    headTurningBack = 0;
    nearestRival = 0;
    trickLandTimer = 0;
    offTrackTime = 0;
    trickPoseHeldPrev = 0;
    contactTime = 0;
}

void KrustyBike::UnknownVirtualSlot3(const Vec3* normal, const Vec3* relVel, const Vec3* arm,
                                     const Vec3* angScale, int eventKind, int scaleBits, float* impulseOut)
{
    if (eventKind == 1000) {
        race->handler->Fn_004DE580(this, 0, eventKind);
        return;
    }
    Bike::UnknownVirtualSlot3(normal, relVel, arm, angScale, eventKind, scaleBits, impulseOut);
    if (eventKind != 0x67)
        race->handler->Fn_004DE580(this, *impulseOut, eventKind);
}

void KrustyBike::UnknownVirtualSlot4(const Vec3* normal, Vec3* velA, const Vec3* worldAngVelA,
                                     const Vec3* armA, int eventKind, float invMassB, int nodeB,
                                     const Vec3* velB, const Vec3* worldAngVelB, const Vec3* armB,
                                     Vec3* invInertiaB, Vec3* angVelB, int velBOrNull,
                                     float* impulseOut, float scale)
{
    Bike::UnknownVirtualSlot4(normal, velA, worldAngVelA, armA, eventKind, invMassB, nodeB, velB,
                              worldAngVelB, armB, invInertiaB, angVelB, velBOrNull, impulseOut, scale);
    race->handler->Fn_004DE580(this, *impulseOut, eventKind);
}

void KrustyBike::UnknownVirtualSlot8()
{
    Bike::UnknownVirtualSlot8();
    if (aiKind && frontWheelLifted)
        frontWheelLifted = 0;
}

int KrustyBike::UnknownVirtualSlot12(int a)
{
    if (a && reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 3) {
        float lo = terrainScale * 105.0f;
        if (position.x < lo)
            return 1;
        KbTrackB* t = ((KbTrackA*)track)->trackData;
        if ((t->sizeX * 256.0f - 105.0f) * terrainScale < position.x)
            return 2;
        if (position.z < lo)
            return 3;
        if ((t->sizeZ * 256.0f - 105.0f) * terrainScale < position.z)
            return 4;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot22()
{
    if (staggerCounter == staggerPhase || race->raceCamera->followedRacer == this)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot23()
{
    if (!airborne && forwardSpeed >= 0.2f && race->raceCamera->cameraState != 3)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot24()
{
    if (isRemote) {
        if (prevSpeed < linearSpeed && !crashState && linearSpeed < 22.0f)
            return 1;
        return 0;
    }
    if (prevThrottleFraction < engineState->throttleFraction && !crashState && linearSpeed < 22.0f)
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot25()
{
    if (isRemote) {
        if (prevSpeed > linearSpeed && !crashState && linearSpeed < 22.0f)
            return 1;
    } else {
        if (prevThrottleFraction < engineState->throttleFraction && !crashState && linearSpeed < 22.0f)
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot33(const Vec3* a0, const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, int a4, float a5)
{
    int result = Bike::UnknownVirtualSlot33(a0, a1, a2, a3, a4, a5);
    lastFramePosition = position;
    wrecker->skipProbeEffects = 1;
    return result;
}

int KrustyBike::UnknownVirtualSlot39(float a)
{
    if (!respawnPending && reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && offTrackTime > 6.0f) {
        if (crashState)
            UnknownVirtualSlot67();
        return 1;
    }
    return Bike::UnknownVirtualSlot39(a);
}

int KrustyBike::UnknownVirtualSlot42()
{
    if (!crashState && motionIndex >= 0 && motionActive && riderCharacter->c_0xc) {
        float sum = trickHoldTime + trickClipTime;
        trickClipTime = 0;
        trickCount++;
        trickHoldTime = 0;
        sum += chainedTrickTime;
        chainedTrickTime = 0;
        trickPoints += sum * 2500.0f;
        trickMultiplier = (float)trickCount;
        totalTrickTime += sum;
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
    KbRacer* r = reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent;
    if (r && r->field_0xa8 == this)
        return 0.0f;
    return 3.0f;
}

int KrustyBike::UnknownVirtualSlot52()
{
    if (crashState || isRemote)
        return 0;
    return 1;
}

void KrustyBike::UnknownVirtualSlot63(float a)
{
    if (aiKind) {
        Fn_00414370(a);
        return;
    }
    aiThrottleRequest = 0;
    aiBrakeRequest = 0;
    Bike::UnknownVirtualSlot63(a);
}

void KrustyBike::UnknownVirtualSlot64(float a)
{
    if (crashState) {
        aiThrottleRequest = 0;
        aiBrakeRequest = 0;
    }
    Bike::UnknownVirtualSlot64(a);
}

int KrustyBike::UnknownVirtualSlot66()
{
    if (crashState || motionActive)
        return 1;
    return hasFinished != 0 && hasFinished < 3;
}

int KrustyBike::UnknownVirtualSlot68(int* out)
{
    *out = 15;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !hasFinished)
        return 0;
    return 1;
}

void KrustyBike::UnknownVirtualSlot69()
{
    if (crashDirection == 2) {
        crashTimer = airCrashTimerReload;
        return;
    }
    float t = crashTimerReload;
    crashTimer = t;
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !reinterpret_cast<KbGame*>(g_TrackGame)->eventType)
        crashTimer = t + 1.0f;
}

void KrustyBike::UnknownVirtualSlot71(int a)
{
    if (crashState) {
        trickLandTimer = 0;
    } else if (airborne && !a && !landingLatched && motionCompleted && trickLandTimer <= 0.0f) {
        trickLandTimer = 0.3f;
    }
    Bike::UnknownVirtualSlot71(a);
}

Vec3 KrustyBike::UnknownVirtualSlot76(const Vec3* a, const Vec3* b)
{
    if (aiKind)
        return kVec3Zero;
    return Bike::UnknownVirtualSlot76(a, b);
}

int KrustyBike::UnknownVirtualSlot80()
{
    if (!race->isRacing && !driveDisengaged)
        return 0;
    if (aiThrottleRequest > 0.0f || (inputDevice && throttleInput > 0.33f) ||
        (UnknownVirtualSlot84(4, 0x3f) && !UnknownVirtualSlot77(engineState->throttleFraction == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot81()
{
    if (!aiKind && !isRemote && !isGhost && UnknownVirtualSlot84(12, 0x3f)) {
        if (!UnknownVirtualSlot77(!driveDisengaged))
            return 1;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot82()
{
    if (!aiKind && !isRemote && !isGhost) {
        if (forwardSpeed < 6.0f && UnknownVirtualSlot84(13, 0x3f)) {
            if (!UnknownVirtualSlot77(engineState->throttleFraction == 0.0f))
                return 1;
        }
        backUpPhase = 0;
    }
    return 0;
}

int KrustyBike::UnknownVirtualSlot83(VehicleWheel* wheel)
{
    BikeWheel* input = (BikeWheel*)wheel;
    if (aiBrakeRequest > 0.0f ||
        (UnknownVirtualSlot84(5, 0x3f) && !UnknownVirtualSlot77(input->rampLevel == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::UnknownVirtualSlot84(int a, int b)
{
    if (!aiKind && !isRemote && !isGhost) {
        KbSensor* s = race->chatOverlay;
        if (!s || !s->typing) {
            Bike::UnknownVirtualSlot84(a, b);
            return inputMap->UnknownVirtualSlot2(a, b);
        }
    }
    return 0;
}

// 0x004925A0: tell the other peers (and the recorder) that this bike was hit by `who`.
void KrustyBike::Fn_004925A0(Vehicle* who, bool flag)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->network && !isRemote) {
        KbNetPacket pkt;
        pkt.field_0x8 = who ? ((KbNetBike*)who)->netId : 0;
        pkt.field_0x4 = flag;
        reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send((unsigned char)(flag ? 0x13 : 0x87), &pkt, 12, ((KbNetBike*)this)->netId, 0);
        if (!flag && netRecorder && reinterpret_cast<KbGame*>(g_TrackGame)->saveReplay && !reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay)
            reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(0x87, who ? ((KbNetBike*)who)->netId : 0, &pkt, 1);
    }
}

// 0x00495C00: true when the two cells referenced at +0x11c8/+0x11cc (both fully set) differ in
// their integer x or z (tier 3 semantics).
int KrustyBike::Fn_00495C00()
{
    if (isRemote) {
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
    trickMultiplier = 1.0f;
    trickPoints = 0;
    trickClipTime = 0;
    trickHoldTime = 0;
    chainedTrickTime = 0;
    landingLatched = 0;
    motionCompleted = 0;
    trickLandTimer = 0;
    trickCount = 0;
    field_0x1534 = 0;
    trickPoseHeld = 0;
    trickChained = 0;
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
    wrecker->EndWreck();
    steerTorque = kVec3Zero;
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
    if (race->isRacing) {
        Bike::UnknownVirtualSlot48();
        return;
    }
    Vec3 v = velocity * 3.0f;
    Vec3 d = (v - prevVelocity) * 0.5f;
    position += d * stepTime;
    rearWheel->driveForce = kVec3Zero;
    rearWheel->driveForceMagnitude = 0;
}

// Slot 86: per-frame wheel-contact update (provisional semantics).  When the race context
// flag (field_0x740+0x18a) is set, a lookup ramp derived from backUpPhase (0.12..0.4762,
// scaled by 360 / 0.3562 = 1010.66815f, the float retail holds, so 360 at 0.12) feeds the
// wheel's helper object, and in one game mode the wheel's two force accumulators are
// halved; the wheel is then always updated.
void KrustyBike::UnknownVirtualSlot86()
{
    BikeWheel* w = rearWheel;
    if (!w->inContact)
        return;
    if (race->isRacing) {
        float ramp;
        if (backingUp && backUpPhase >= 0.12f && backUpPhase < 0.4762f)
            ramp = (0.4762f - backUpPhase) * (360.0f / 0.3562f);
        else
            ramp = 0;
        if (w->aux)
            reinterpret_cast<UnknownEngineGearbox*>(w->aux)->DriveForce(
                KbFloat(linearSpeed), reinterpret_cast<const Vector3*>(&w->rollDirection),
                KbFloat(driveForceScale), backingUp, ramp, reinterpret_cast<Vector3*>(&w->driveForce),
                &w->driveForceMagnitude);
        if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !onTrack) {
            rearWheel->driveForce *= 0.5f;
            rearWheel->driveForceMagnitude *= 0.5f;
        }
    }
    rearWheel->UpdateDriveShare(this);
}

// Slot 96: control responses derived from the Options sliders (TrackGame 0xfe0..0xff8).
void KrustyBike::UnknownVirtualSlot96()
{
    if (aiKind)
        controlResponse[0] = 0.675f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->steeringSlider * 0.01f;
            controlResponse[0] = pct * 0.2f + 0.575f;
        }
    if (aiKind)
        crossupScale = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->crossupSlider * 0.01f;
            crossupScale = pct * 0.4f + 0.8f;
        }
    if (aiKind)
        controlResponse[1] = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->pitchSlider * 0.01f;
            controlResponse[1] = pct * 0.4f + 0.8f;
        }
    if (aiKind)
        controlResponse[2] = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->pressThrottleSlider * 0.01f;
            controlResponse[2] = pct * 0.4f + 0.8f;
        }
    if (aiKind)
        controlResponse[3] = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->releaseThrottleSlider * 0.01f;
            controlResponse[3] = pct * 0.4f + 0.8f;
        }
    if (aiKind)
        controlResponse[4] = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->pressBrakesSlider * 0.01f;
            controlResponse[4] = pct * 0.4f + 0.8f;
        }
    if (aiKind)
        controlResponse[5] = 1.0f;
    else
        {
            float pct = reinterpret_cast<KbGame*>(g_TrackGame)->releaseBrakesSlider * 0.01f;
            controlResponse[5] = pct * 0.4f + 0.8f;
        }
}

// Slot 28 (0x0048da50): per-frame hook.  In mode 4 (tier 3: a spectator/replay mode) it
// re-targets the session's player record onto this bike, then runs the base update and,
// while airborne (field_0x444 set, field_0x5b4 clear), jitters the two 0x7ac/0x7b0
// accumulators with random noise (0.15 scale for an aiKind bike, 0.667 otherwise).
int KrustyBike::UnknownVirtualSlot28(int a)
{
    lastCollisionType = 0;
    if (isGhost) return a;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4) {
        KbPlayer* p = reinterpret_cast<KbPlayer*>(reinterpret_cast<EventManager*>(
            reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindRaceMode());
        if (this != p->followedRacer && this == race->localRacer) {
            Vehicle* v = p->followedRacer;
            if (v) {
                if (collisionObject->TestMeshBounds(v->collisionObject))
                    Fn_004925A0(this, 1);
            } else if (p->ball) {
                if (collisionObject->TestMeshBounds(p->ball->collisionObject)) {
                    if (reinterpret_cast<KbGame*>(g_TrackGame)->network->isHost) p->Fn_004A9E80(this, 0, 1);
                    else Fn_004925A0(this, 1);
                }
            }
        }
        if (isRemote) return a;
    }
    int r = Vehicle::UnknownVirtualSlot28(a);
    if (crashState && !prevCrashState) {
        if (aiKind) {
            repairDamage += KbRandUnit() * 0.15f;
            injuryDamage += KbRandUnit() * 0.15f;
        } else {
            repairDamage += KbRandUnit() * 0.667f;
            injuryDamage += KbRandUnit() * 0.667f;
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
        KbTrackB* t = ((KbTrackA*)track)->trackData;
        scratchVector = Vec3(t->sizeX * terrainScale * 128.0f, 20.0f,
                             t->sizeZ * terrainScale * 128.0f);
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
    if (!motionActive && inputMap && inputMap->UnknownVirtualSlot2(8, 0x3f)) {
        if (airborne && !crashState && frontWheel->penetration < -3.5f && rearWheel->penetration < -3.5f) {
            crashDirection = 2;
            crashReason = 1;
            return 1;
        }
    }
    if (!airborne && motionActive) {
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
// re-arm another with a time-scaled value, then derive forceFeedbackGain from the same FeedbackSlider value.
void KrustyBike::UnknownVirtualSlot93()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->field_0xc == 3) {
        pad->UnknownVirtualSlot6(1, 10000, -1);
        pad = (KbPad*)inputMap->valueSource;
        pad->UnknownVirtualSlot6(0, (int)(reinterpret_cast<KbGame*>(g_TrackGame)->feedbackSlider * 83.333336f), 70000);
        forceFeedbackGain = reinterpret_cast<KbGame*>(g_TrackGame)->feedbackSlider * 0.04f;
    }
}

// Slot 94: pad effect update; the force term is smoothedForwardAccel * forceFeedbackGain * 180 clamped to
// [-10000, 10000] while not in state 1, then the effect is toggled on slot 80's result.
void KrustyBike::UnknownVirtualSlot94()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->field_0xc == 3) {
        if (crashState != 1) {
            KbEffectParams params = { 0x4650, 0 };
            int force = (int)(smoothedForwardAccel * forceFeedbackGain * 180.0f);
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
// Normally they are recomputed from the node's world axes; while neither crossoverPose nor
// motionActive is set the old values are just copied.  Then the bike's matrix is mirrored
// onto the 0x5c4 node when the wrecker's mode is 0 or 1, and the 0x141c timer is advanced.
void KrustyBike::UnknownVirtualSlot29(int a)
{
    if (!crossoverPose && !motionActive) {
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
    int mode = wrecker->wreckState;
    if (!mode || mode == 1) {
        Matrix4 m;
        modelNode->GetMatrixIn(0, &m);
        reinterpret_cast<SoultreeObject*>(riderCharacter->c_0x1a0)->SetMatrixIn(0, &m);
    }
    if (motionCompleted && !hasFinished && landingLatched) {
        if (trickLandTimer < 0.0f) {
            score += Fn_00495FF0();
            return;
        }
        trickLandTimer -= stepTime;
    }
}

// Slot 50: switch the 0x4f0 state (and the attached 0x15e8 object) on or off.  The base slot
// only runs for the forced (c != 0) case; otherwise the state is latched here with b stored
// as the countdown at 0x4f4.
void KrustyBike::UnknownVirtualSlot50(int a, float b, int c)
{
    if (race->networkStartPending)
        return;
    if (c) {
        if (a) {
            if (!spawnProtected) {
                if (isGhost) {
                    ((D3DIMSoultreeObject*)modelNode)->AddModifier((GameObject*)ghostMod);
                    ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddModifier((GameObject*)ghostMod);
                    ((GhostMod1*)ghostMod)->StartFade(0);
                }
                Fn_00496E30(1);
            }
            spawnProtectForced = 1;
            spawnProtected = 1;
            spawnProtectTimer = 0;
        } else {
            spawnProtectForced = 0;
            if (spawnProtected)
                spawnProtectEndPending = 1;
        }
        Vehicle::UnknownVirtualSlot50(a, 0, 0);
        return;
    }
    if (spawnProtectForced)
        return;
    if (a) {
        if (!spawnProtected) {
            if (isGhost) {
                ((D3DIMSoultreeObject*)modelNode)->AddModifier((GameObject*)ghostMod);
                ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddModifier((GameObject*)ghostMod);
                ((GhostMod1*)ghostMod)->StartFade(b);
            }
            Fn_00496E30(1);
        }
        spawnProtected = 1;
        spawnProtectTimer = b;
    } else if (spawnProtected) {
        spawnProtectEndPending = 1;
    }
}

// Slot 43: reset of the per-bike state (runs the Vehicle reset, then clears the 0x74c..0x7c0
// block, the statusNode record and a few late fields).  The record's 0x44 vector is taken from
// the race context (0xf8..0x100, or from the object at +0x48 when no 0x144 object exists).
void KrustyBike::UnknownVirtualSlot43()
{
    Vehicle::UnknownVirtualSlot43();
    onTrack = 1;
    offTrackTime = 0;
    lapCount = 0;
    prevLapCount = 0;
    gatesPassed = 0;
    lastLapTime = 0;
    lapStartTime = 0;
    timeBehindLeader = 0;
    finishPosition = 0;
    hasFinished = 0;
    fastestLap = 0;
    totalRunningTime = 0;
    largestStuntPoints = 0;
    totalTrickTime = 0;
    bestTagRun = 0;
    totalTagRun = 0;
    field_0x794 = 0;
    field_0x798 = 0;
    score = 0;
    poseParamLatched = 0;
    poseLeanBlendLatched = 0;
    latchedPoseParam = 0;
    latchedPoseLeanBlend = 0;
    aiThrottleRequest = 0;
    aiBrakeRequest = 0;
    lapGate = 0;
    forceGatePass = 0;
    nextLapGate = 1;
    if (statusNode) {
        KbRaceSub* p = race->raceTrack;
        if (p) {
            if (race->hasStartProbe) {
                statusNode->progressPos.a = race->startProbeNode;
                statusNode->progressPos.b = race->startProbeSegment;
                statusNode->progressPos.c = race->startProbeT;
            } else {
                statusNode->progressPos.a = (int)p->startNode;
                statusNode->progressPos.b = race->raceTrack->startNode[2];
                statusNode->progressPos.c = 0;
            }
            statusNode->placedPos = statusNode->progressPos;
        }
        statusNode->nextGate = race->gateList;
        statusNode->goalDistance = 0;
        statusNode->gateDistance = 0;
        statusNode->progressStep = 0;
        statusNode->progressTotal = 0;
        statusNode->lapOffset = 0;
        statusNode->previousPosition = Vec3(0.0f, 0.0f, 0.0f);
        statusNode->currentPosition = Vec3(0.0f, 0.0f, 0.0f);
    }
    netState.timer = reinterpret_cast<KbGame*>(g_TrackGame)->fullNetPacketIntervalSec;
    shortMessageTimer = reinterpret_cast<KbGame*>(g_TrackGame)->shortNetPacketIntervalSec;
    stallHoldTimer = 0;
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
// wrecker object, binds both wheels and attaches the scene node (tier 3 semantics).
void KrustyBike::UnknownVirtualSlot97()
{
    Bike::UnknownVirtualSlot97();
    wrecker->AddRandomAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Fall01", 1));
    wrecker->AddRandomAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Fall02", 1));
    wrecker->AddRandomAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Fall03", 1));
    wrecker->AddRandomAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Fall04", 1));
    wrecker->AddContactAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("LeftHit", 1));
    wrecker->AddContactAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("RightHit", 1));
    wrecker->AddContactAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("FeetHitL", 1));
    wrecker->AddContactAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("FeetHitR", 1));
    wrecker->AddContactAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeadHit", 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[0].name, 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[1].name, 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[2].name, 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[3].name, 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[4].name, 1));
    wrecker->AddIndexedAnimation((int)reinterpret_cast<Character*>(riderCharacter)->FindMotion(s_KrustyTricks[5].name, 1));
    wrecker->AddIgnoredObject((CollisionObject*)frontWheel);
    wrecker->AddIgnoredObject((CollisionObject*)rearWheel);
    wrecker->SetPushBox((UnknownWreckerSkeleton*)modelNode->firstChild, Vector3(0.0f, 2.0f, 0.0f), Vector3(0.5f, 1.5f, 3.0f));
}

// Slot 49: per-frame update.  Keeps the 0x154c "pinned" copy of the placement in sync, posts
// on-screen messages to the local player (string ids 0x1481.. / 0x14d6 / 0x14d7 / 0x14e1; tier 3
// semantics), advances the 0x7a4 state, and runs the Bike update unless the race context
// takes over.
void KrustyBike::UnknownVirtualSlot49(float dt)
{
    if (drawnAtLastPosition) {
        modelNode->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
    }
    if (race->localRacer == this && reinterpret_cast<KbGame*>(g_TrackGame)->raceMode == 4) {
        TextQueueOverlay* sink = reinterpret_cast<EventManager*>(reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindTextQueue();
        UnknownMessage* msg = 0;
        if (sink) {
            char text[0x100];
            if (totalRunningTime == 0) {
                if (race->startBeforeFinish)
                    g_TrackGame->LoadResourceString(0x14d6, text, 0x80);
                else
                    g_TrackGame->LoadResourceString(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11c6) UnknownMessage(text, 3.25f);
                if (msg) {
                    sink->ReplaceHeadMessage(msg);
                    delete msg;
                }
            } else if (lapCount != prevLapCount && lapCount == 1 && race->startBeforeFinish) {
                g_TrackGame->LoadResourceString(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11d0) UnknownMessage(text, 3.25f);
                if (msg) {
                    sink->ReplaceHeadMessage(msg);
                    delete msg;
                }
            }
        }
    }
    if (hasFinished) {
        if (hasFinished == 1) {
            TextQueueOverlay* sink = reinterpret_cast<EventManager*>(reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindTextQueue();
            if (race->localRacer == this && sink) {
                char text[0x100];
                UnknownMessage* msg = 0;
                if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount == 1) {
                    switch (finishPosition) {
                    case 1: g_TrackGame->LoadResourceString(0x1481, text, 0x80); break;
                    case 2: g_TrackGame->LoadResourceString(0x1482, text, 0x80); break;
                    case 3: g_TrackGame->LoadResourceString(0x1483, text, 0x80); break;
                    case 4: g_TrackGame->LoadResourceString(0x1484, text, 0x80); break;
                    case 5: g_TrackGame->LoadResourceString(0x1485, text, 0x80); break;
                    case 6: g_TrackGame->LoadResourceString(0x1486, text, 0x80); break;
                    case 7: g_TrackGame->LoadResourceString(0x1487, text, 0x80); break;
                    case 8: g_TrackGame->LoadResourceString(0x1488, text, 0x80); break;
                    case 9: g_TrackGame->LoadResourceString(0x1489, text, 0x80); break;
                    case 10: g_TrackGame->LoadResourceString(0x148a, text, 0x80); break;
                    case 11: g_TrackGame->LoadResourceString(0x148b, text, 0x80); break;
                    default: g_TrackGame->LoadResourceString(0x148c, text, 0x80); break;
                    }
                    msg = new(__FILE__, 0x1206) UnknownMessage(text, 3.25f);
                    if (msg) {
                        sink->ReplaceHeadMessage(msg);
                        delete msg;
                    }
                } else {
                    g_TrackGame->LoadResourceString(0x14e1, text, 0x80);
                    msg = new(__FILE__, 0x120d) UnknownMessage(text, 3.25f);
                    if (msg) {
                        sink->ReplaceHeadMessage(msg);
                        delete msg;
                    }
                }
            }
            if (finishPosition == 1) {
                if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType) {
                    UnknownVirtualSlot41();
                    motionCompleted = 0;
                    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(riderPoseHandles[15]));
                    ((KbA5C4*)riderCharacter)->field_0x10 = 0;
                    hasFinished = 3;
                } else {
                    hasFinished = 3;
                    UnknownVirtualSlot41();
                    motionCompleted = 0;
                }
            } else {
                hasFinished = 3;
                UnknownVirtualSlot41();
                motionCompleted = 0;
            }
        }
    }
    if (!crashState && (aiKind || race->forceAiStep)) {
        if (race->isRacing)
            Fn_00413200(dt);
    } else {
        Bike::UnknownVirtualSlot49(dt);
    }
    if (race->raceCamera->cameraState == 3 && race->raceCamera->followedBike == this) {
        modelNode->SetPosition(lastFramePosition);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(lastFramePosition);
        drawnAtLastPosition = 1;
    } else {
        drawnAtLastPosition = 0;
    }
    lastFramePosition = position;
    trickPoseHeldPrev = trickPoseHeld;
    prevLapCount = lapCount;
}

// 0x00495ff0: ends a trick. trickPoints (+0x1530) drops its part below 100, the
// score is points x trickMultiplier (+0x1538, +0.5 when trickChained); the race
// handler hears of it. In game mode 0 (or 4 with +0x2eb8 for the +0x568 racer's
// bike) the score is kept in +0x788 and returned; otherwise the bonus rules
// (+0x3444) turn it into a capped bonus, shown to the +0x50 racer's bike.
// The redundant parentheses around the (float) fmod term are significant: VC6 keeps them
// as an IL node, which schedules retail's `mov al, [+0x153f]` before the fsubr
// (docs/VC6_OPERAND_ORDER.md section 3); without them the load follows it.
float KrustyBike::Fn_00495FF0()
{
    trickPoints -= ((float)fmod(trickPoints, 100.0));
    if (trickChained)
        trickMultiplier += 0.5f;
    float score = trickPoints * trickMultiplier;
    if (race->localRacer == this && race->handler)
        race->handler->ReportTrickScore(this, score);
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->eventType ||
        (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4 && reinterpret_cast<KbGame*>(g_TrackGame)->stuntMode && reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent->field_0xa8 == this)) {
        if (race->localRacer == this) {
            if (race->scoreBoard)
                race->scoreBoard->ShowValue(trickPoints, trickMultiplier);
            int points = (int)score;
            if (largestStuntPoints > points)
                points = largestStuntPoints;
            largestStuntPoints = points;
        }
        Fn_0048D8B0();
        return score;
    }
    KbBonusTable* rules = reinterpret_cast<KbGame*>(g_TrackGame)->proCircuit;
    if (rules) {
        float step = trickPoints * 0.000016f;
        if (!(step < 1.0f))
            step = 1.0f;
        int base = rules->base[rules->index];
        int points = (int)(base * rules->baseScale);
        float gain = 0.0f;
        float limit = points * rules->limitScale;
        if (stuntEarnings * 400.0f < limit) {
            float total = step + stuntEarnings;
            if (total * 400.0f > limit) {
                stuntEarnings = limit * 0.0025f;
            } else {
                stuntEarnings = total;
                gain = step * 400.0f;
            }
        }
        if (this == race->raceCamera->followedRacer) {
            TextQueueOverlay* sink = reinterpret_cast<EventManager*>(reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindTextQueue();
            char text[0x80];
            char line[0x80];
            if (gain > 0.0f) {
                g_TrackGame->LoadResourceString(0x14d8, text, 0x80);
                sprintf(line, "%s %.0f.00", text, gain);
                UnknownMessage* message = new(__FILE__, 0xa14) UnknownMessage(line, 3.25f);
                if (message) {
                    sink->ReplaceHeadMessage(message);
                    delete message;
                }
            } else {
                g_TrackGame->LoadResourceString(0x14d9, text, 0x80);
                sprintf(line, "%s %.0f.00)", text, points * rules->limitScale);
                UnknownMessage* message = new(__FILE__, 0xa1d) UnknownMessage(line, 3.25f);
                if (message) {
                    sink->ReplaceHeadMessage(message);
                    delete message;
                }
            }
        }
    }
    Fn_0048D8B0();
    return 0.0f;
}

// 0x004965e0 (slot 14, exact): apply an impulse.  field_0x24 behaves as an inverse mass
// (dv = k * impulse) after the impulse's x and z are cleared.  Written as `invMass * *a`:
// VC6 forwards the two zero stores into the scaled vector and folds k * 0 into one pooled
// 0.0f on the FPU stack (fld/fst shared by x and z), as retail does; the explicit
// Vec3(0.0f, invMass * a->y, 0.0f) stores immediates instead.
void KrustyBike::UnknownVirtualSlot14(Vec3* a, const Vec3* b, const Vec3* c)
{
    if (race->isRacing) {
        Bike::UnknownVirtualSlot14(a, b, c);
        return;
    }
    angularAcceleration = UnknownVirtualSlot16(b);
    UnknownVirtualSlot15(c, &angularAcceleration);
    angularVelocity += stepTime * angularAcceleration;
    angularVelocity *= 0.999f;
    a->x = 0;
    a->z = 0;
    acceleration = invMass * *a;
    velocity += stepTime * acceleration;
    linearSpeed = KbLength(velocity);
}

// 0x00496d10 (exact): slot 101 forwards to the body below; VC6 compiles the call as a tail
// `jmp 0x496d20`, followed by the usual nop alignment padding.
void KrustyBike::UnknownVirtualSlot101()
{
    Fn_00496D20();
}

// owner: bracket only (0x496d20 sits between slot 101 and Fn_00496DA0)
// 0x00496d20 (exact): the real body of slot 101, switching collisionObject from
// bikeAndRiderCollision to bikeCollision.  The mode test is a nested if with the 0x2d84 test written
// positively: VC6 then computes the GraphicsTest base (+0xc) in the second test block, calls through ecx
// and cross-jumps the outer else into the inner slot-5 tail as retail does (the flat
// `x18 > 1 && !x2d84` form and the nested `!x2d84` form do not).
void KrustyBike::Fn_00496D20()
{
    bikeAndRiderCollision->Disable();
    g_collisionQuadTree->Remove(bikeAndRiderCollision, bikeAndRiderCollision->quadtreeCell);
    collisionObject = bikeCollision;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount > 1) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->riderCollision)
            collisionObject->Enable();
        else
            collisionObject->Disable();
    } else
        collisionObject->Enable();
    collisionObject->SnapPlacement();
}

// owner: bracket only (0x496da0, directly after Fn_00496D20)
// 0x00496da0 (exact): back-camera swap, Fn_00496D20 with the two scene objects exchanged.
void KrustyBike::Fn_00496DA0()
{
    bikeCollision->Disable();
    g_collisionQuadTree->Remove(bikeCollision, bikeCollision->quadtreeCell);
    collisionObject = bikeAndRiderCollision;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount > 1) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->riderCollision)
            collisionObject->Enable();
        else
            collisionObject->Disable();
    } else
        collisionObject->Enable();
    collisionObject->SnapPlacement();
}

// 0x00496F90: both bikes' collision objects ignore each other (AddIgnoredOwner = 0x00439410).
void KrustyBike::Fn_00496F90(KrustyBike* other)
{
    if (bikeCollision) {
        other->bikeCollision->AddIgnoredOwner(bikeCollision);
        other->bikeCollision->AddIgnoredOwner(bikeAndRiderCollision);
        other->bikeCollision->AddIgnoredOwner((void*)frontWheel);
        other->bikeCollision->AddIgnoredOwner((void*)rearWheel);
        other->bikeCollision->AddIgnoredOwner((void*)wrecker->collisionModel);
        other->bikeAndRiderCollision->AddIgnoredOwner(bikeCollision);
        other->bikeAndRiderCollision->AddIgnoredOwner(bikeAndRiderCollision);
        other->bikeAndRiderCollision->AddIgnoredOwner((void*)frontWheel);
        other->bikeAndRiderCollision->AddIgnoredOwner((void*)rearWheel);
        other->bikeAndRiderCollision->AddIgnoredOwner((void*)wrecker->collisionModel);
        ((CollisionObject*)other->frontWheel)->AddIgnoredOwner(bikeCollision);
        ((CollisionObject*)other->frontWheel)->AddIgnoredOwner(bikeAndRiderCollision);
        ((CollisionObject*)other->frontWheel)->AddIgnoredOwner((void*)frontWheel);
        ((CollisionObject*)other->frontWheel)->AddIgnoredOwner((void*)rearWheel);
        ((CollisionObject*)other->frontWheel)->AddIgnoredOwner((void*)wrecker->collisionModel);
        ((CollisionObject*)other->rearWheel)->AddIgnoredOwner(bikeCollision);
        ((CollisionObject*)other->rearWheel)->AddIgnoredOwner(bikeAndRiderCollision);
        ((CollisionObject*)other->rearWheel)->AddIgnoredOwner((void*)frontWheel);
        ((CollisionObject*)other->rearWheel)->AddIgnoredOwner((void*)rearWheel);
        ((CollisionObject*)other->rearWheel)->AddIgnoredOwner((void*)wrecker->collisionModel);
        ((CollisionObject*)other->wrecker->collisionModel)->AddIgnoredOwner(bikeCollision);
        ((CollisionObject*)other->wrecker->collisionModel)->AddIgnoredOwner(bikeAndRiderCollision);
        ((CollisionObject*)other->wrecker->collisionModel)->AddIgnoredOwner((void*)frontWheel);
        ((CollisionObject*)other->wrecker->collisionModel)->AddIgnoredOwner((void*)rearWheel);
        ((CollisionObject*)other->wrecker->collisionModel)->AddIgnoredOwner((void*)wrecker->collisionModel);
        bikeCollision->AddIgnoredOwner(other->bikeCollision);
        bikeCollision->AddIgnoredOwner(other->bikeAndRiderCollision);
        bikeCollision->AddIgnoredOwner((void*)other->frontWheel);
        bikeCollision->AddIgnoredOwner((void*)other->rearWheel);
        bikeCollision->AddIgnoredOwner((void*)other->wrecker->collisionModel);
        bikeAndRiderCollision->AddIgnoredOwner(other->bikeCollision);
        bikeAndRiderCollision->AddIgnoredOwner(other->bikeAndRiderCollision);
        bikeAndRiderCollision->AddIgnoredOwner((void*)other->frontWheel);
        bikeAndRiderCollision->AddIgnoredOwner((void*)other->rearWheel);
        bikeAndRiderCollision->AddIgnoredOwner((void*)other->wrecker->collisionModel);
        ((CollisionObject*)frontWheel)->AddIgnoredOwner(other->bikeCollision);
        ((CollisionObject*)frontWheel)->AddIgnoredOwner(other->bikeAndRiderCollision);
        ((CollisionObject*)frontWheel)->AddIgnoredOwner((void*)other->frontWheel);
        ((CollisionObject*)frontWheel)->AddIgnoredOwner((void*)other->rearWheel);
        ((CollisionObject*)frontWheel)->AddIgnoredOwner((void*)other->wrecker->collisionModel);
        ((CollisionObject*)rearWheel)->AddIgnoredOwner(other->bikeCollision);
        ((CollisionObject*)rearWheel)->AddIgnoredOwner(other->bikeAndRiderCollision);
        ((CollisionObject*)rearWheel)->AddIgnoredOwner((void*)other->frontWheel);
        ((CollisionObject*)rearWheel)->AddIgnoredOwner((void*)other->rearWheel);
        ((CollisionObject*)rearWheel)->AddIgnoredOwner((void*)other->wrecker->collisionModel);
        ((CollisionObject*)wrecker->collisionModel)->AddIgnoredOwner(other->bikeCollision);
        ((CollisionObject*)wrecker->collisionModel)->AddIgnoredOwner(other->bikeAndRiderCollision);
        ((CollisionObject*)wrecker->collisionModel)->AddIgnoredOwner((void*)other->frontWheel);
        ((CollisionObject*)wrecker->collisionModel)->AddIgnoredOwner((void*)other->rearWheel);
        ((CollisionObject*)wrecker->collisionModel)->AddIgnoredOwner((void*)other->wrecker->collisionModel);
    }
}

// 0x00497370: undo the mutual ignore unless one of them is the camera-followed racer.
void KrustyBike::Fn_00497370(KrustyBike* other)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent) {
        void* current = reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent->field_0xa8;
        if (other == current || this == current)
            return;
    }
    other->collisionObject->RemoveIgnoredOwner((void*)collisionObject);
    other->collisionObject->RemoveIgnoredOwner((void*)frontWheel);
    other->collisionObject->RemoveIgnoredOwner((void*)rearWheel);
    other->collisionObject->RemoveIgnoredOwner((void*)wrecker->collisionModel);
    ((CollisionObject*)other->frontWheel)->RemoveIgnoredOwner((void*)collisionObject);
    ((CollisionObject*)other->frontWheel)->RemoveIgnoredOwner((void*)frontWheel);
    ((CollisionObject*)other->frontWheel)->RemoveIgnoredOwner((void*)rearWheel);
    ((CollisionObject*)other->frontWheel)->RemoveIgnoredOwner((void*)wrecker->collisionModel);
    ((CollisionObject*)other->rearWheel)->RemoveIgnoredOwner((void*)collisionObject);
    ((CollisionObject*)other->rearWheel)->RemoveIgnoredOwner((void*)frontWheel);
    ((CollisionObject*)other->rearWheel)->RemoveIgnoredOwner((void*)rearWheel);
    ((CollisionObject*)other->rearWheel)->RemoveIgnoredOwner((void*)wrecker->collisionModel);
    ((CollisionObject*)other->wrecker->collisionModel)->RemoveIgnoredOwner((void*)collisionObject);
    ((CollisionObject*)other->wrecker->collisionModel)->RemoveIgnoredOwner((void*)frontWheel);
    ((CollisionObject*)other->wrecker->collisionModel)->RemoveIgnoredOwner((void*)rearWheel);
    ((CollisionObject*)other->wrecker->collisionModel)->RemoveIgnoredOwner((void*)wrecker->collisionModel);
    if (other->UnknownVirtualSlot51())
        return;
    collisionObject->RemoveIgnoredOwner((void*)other->collisionObject);
    collisionObject->RemoveIgnoredOwner((void*)other->frontWheel);
    collisionObject->RemoveIgnoredOwner((void*)other->rearWheel);
    collisionObject->RemoveIgnoredOwner((void*)other->wrecker->collisionModel);
    ((CollisionObject*)frontWheel)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((CollisionObject*)frontWheel)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((CollisionObject*)frontWheel)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((CollisionObject*)frontWheel)->RemoveIgnoredOwner((void*)other->wrecker->collisionModel);
    ((CollisionObject*)rearWheel)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((CollisionObject*)rearWheel)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((CollisionObject*)rearWheel)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((CollisionObject*)rearWheel)->RemoveIgnoredOwner((void*)other->wrecker->collisionModel);
    ((CollisionObject*)wrecker->collisionModel)->RemoveIgnoredOwner((void*)other->collisionObject);
    ((CollisionObject*)wrecker->collisionModel)->RemoveIgnoredOwner((void*)other->frontWheel);
    ((CollisionObject*)wrecker->collisionModel)->RemoveIgnoredOwner((void*)other->rearWheel);
    ((CollisionObject*)wrecker->collisionModel)->RemoveIgnoredOwner((void*)other->wrecker->collisionModel);
}

// 0x00496E30: a != 0 makes this bike's collision objects ignore every other bike (0x00496F90)
// and stops the part collision objects ignoring its main body; a == 0 does the reverse
// (0x00497370) and makes the part collision objects ignore both of its scene bodies.
void KrustyBike::Fn_00496E30(int a)
{
    int cursor = 0;
    KrustyBike* other;
    if (a) {
        for (other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor)); other; other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor))) {
            if (other != this)
                Fn_00496F90(other);
        }
        if (((KbTrackA*)track)->partList && !isRemote) {
            for (int i = 0; i < ((KbTrackA*)track)->partList->count; i++) {
                KbPart* part = ((KbTrackA*)track)->partList->items + i;
                if (!(part->flags & 8))
                    part->obj->collider->RemoveIgnoredOwner((void*)collisionObject);
            }
        }
    } else {
        for (other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor)); other; other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor))) {
            if (other != this && (!isRemote || !other->isRemote))
                Fn_00497370(other);
        }
        if (((KbTrackA*)track)->partList && !isRemote) {
            for (int i = 0; i < ((KbTrackA*)track)->partList->count; i++) {
                if (!(((KbTrackA*)track)->partList->items[i].flags & 8)) {
                    KbPart* part = ((KbTrackA*)track)->partList->items + i;
                    KbPartObj* obj = part->obj;
                    obj->collider->AddIgnoredOwner(bikeCollision);
                    obj->collider->AddIgnoredOwner(bikeAndRiderCollision);
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
    motionActive = 1;
    motionCompleted = 0;
    field_0x432 = 1;
    trickClipTime = 0;
    chainedTrickTime = 0;
    trickHoldTime = 0;
    motionIndex = (char)idx;
    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(animSetA[idx]));
    ((KbA5C4*)riderCharacter)->field_0x10 = 0;
    D3DIMSoultreeCharacter::SetMotion((Motion*)animSetB[idx]);
    chr_field_0x10 = 0;
}

// owner: bracket only (0x48d990; callers 0x48eff2, 0x48f115, 0x49562c are KrustyBike methods)
// Variant of Fn_0048D910 that banks trickClipTime in chainedTrickTime and uses animSetC/animSetD.
void KrustyBike::Fn_0048D990(int idx)
{
    motionActive = 1;
    chainedTrickTime = trickClipTime;
    trickClipTime = 0;
    motionIndex = (char)(idx + 0x10);
    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(animSetC[idx]));
    D3DIMSoultreeCharacter::SetMotion((Motion*)animSetD[idx]);
    trickChained = 1;
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
    if ((bike->UnknownVirtualSlot51() || bike->hasFinished) && kind != 0x66 && kind != 0x6a)
        return;
    if (kind == 0x64) {
        KrustyBike* o = (KrustyBike*)other->ownerObject;
        if (o->UnknownVirtualSlot51() || o->hasFinished)
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
    if (bike->isRemote)
        return;
    if (bike->hasFinished)
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
// Nearest other bike of the race (squared planar distance), skipping those with outOfRace set;
// optionally returns the distance.
KrustyBike* KrustyBike::FindNearestRival(float* outDistance)
{
    int cursor = 0;
    KrustyBike* b = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
    while (b && b->outOfRace)
        b = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
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
        b = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
        while (b && b->outOfRace)
            b = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
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
    rivalBearing = angle;
    if (angle < 0)
        angle = -angle;
    if (angle <= 0.698f)
        return;
    UnknownVirtualSlot41();
    motionCompleted = 0;
    motionActive = 1;
    field_0x432 = 1;
    headTurningToRival = 1;
    motionIndex = (char)0xff;
    if (rivalBearing > 2.7f) {
        rivalBearing = 2.7f;
        rivalBearingScaled = 0.7f;
    } else if (rivalBearing < -2.7f) {
        rivalBearing = -2.7f;
        rivalBearingScaled = -0.7f;
    } else {
        rivalBearingScaled = (rivalBearing * 0.36963f) * 0.7f;
    }
    headTurnAngle = 0;
    reinterpret_cast<Character*>(riderCharacter)->Method_0x004a8bf0(riderPoseHandles[14], 0.5f);
    D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[14], 0.5f);
}

// 0x004933e0: the decoded network state (+0x1358..+0x1388) accumulates message 13's
// byte deltas; the result is copied into *state.
void KrustyBike::Fn_004933E0(const KbNetDelta* delta, KbNetState* state)
{
    Vec3 d0(delta->delta0[0] * 0.234375f, delta->delta0[1] * 0.234375f, delta->delta0[2] * 0.234375f);
    Vec3 d1(delta->delta1[0] * 0.234375f, delta->delta1[1] * 0.234375f, delta->delta1[2] * 0.234375f);
    Vec3 d2(delta->delta2[0] * 0.049087387f, delta->delta2[1] * 0.049087387f, delta->delta2[2] * 0.049087387f);
    deltaBaseVelocity += d0;
    deltaBasePosition += d1;
    deltaBaseAngularVelocity += d2;
    deltaBaseRoll += delta->delta3[0] * 0.049087387f;
    deltaBasePitch += delta->delta3[1] * 0.049087387f;
    deltaBaseYaw += delta->delta3[2] * 0.049087387f;
    int step;
    if (delta->step & 1)
        step = (delta->step >> 1) * 8;
    else
        step = delta->step >> 1;
    state->step = step;
    deltaBaseTime += state->step;
    state->time = deltaBaseTime;
    state->velocity = deltaBaseVelocity;
    state->position = deltaBasePosition;
    state->angularVelocity = deltaBaseAngularVelocity;
    state->roll = deltaBaseRoll;
    state->pitch = deltaBasePitch;
    state->yaw = deltaBaseYaw;
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

int KrustyBike::Tick(float dt)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount == 1 && !race->isRacing)
        return 1;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay) {
        if (race->isRacing) {
            Fn_00493660(dt, 1);
            if (race->replayMode == 4 && race->appliedReplayMode == 4)
                GameObject::Tick(dt);
        }
    } else if (isRemote) {
        Fn_00493660(dt, 0);
        if (race->recorderTickDue)
            Fn_00492AD0(dt, 1);
        GameObject::Tick(dt);
    } else if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode == 4) {
        if (isGhost) {
            Fn_00493660(dt, 1);
            if (Fn_00495C00())
                GameObject::Tick(dt);
        } else {
            Vehicle::Tick(dt);
            if (race->recorderTickDue)
                Fn_00492AD0(dt, 1);
        }
    } else {
        Vehicle::Tick(dt);
        if (race->recorderTickDue)
            Fn_00492AD0(dt, 1);
        if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount > 1)
            Fn_00492AD0(dt, 0);
    }
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4) {
        KbPlayer* p = reinterpret_cast<KbPlayer*>(reinterpret_cast<EventManager*>(
            reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindRaceMode());
        if (p->ball && p->followedRacer == this && crashState) {
            if (reinterpret_cast<KbGame*>(g_TrackGame)->network->isHost)
                p->Fn_004A9E80(0, this, 1);
            else
                Fn_004925A0(0, 1);
        }
    }
    if (spawnProtectEndPending && !isRemote && race->isRacing) {
        int cursor = 0;
        int overlap = 0;
        KrustyBike* bike;
        for (bike = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor)); bike; bike = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor))) {
            if (!bike->outOfRace && !bike->UnknownVirtualSlot51() && bike != this) {
                overlap = collisionObject->TestMeshBounds(bike->collisionObject);
                if (overlap)
                    break;
            }
        }
        if (!overlap) {
            spawnProtected = 0;
            spawnProtectEndPending = 0;
            if (isGhost) {
                ((D3DIMSoultreeObject*)modelNode)->RemoveModifier((GameObject*)ghostMod);
                ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->RemoveModifier((GameObject*)ghostMod);
            }
            Fn_00496E30(0);
        }
    }
    if (reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay && race->localRacer == this) {
        if (debugPage < 0) {
            int line = reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay->lineCount++;
            debugPage = line;
        }
        reinterpret_cast<DebugOverlay*>(reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay)->Title(debugPage, "KrustyBike");
        reinterpret_cast<DebugOverlay*>(reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay)->Print(debugPage, "Position/Heading=");
        reinterpret_cast<DebugOverlay*>(reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay)->Print(debugPage, "%.2f, %.2f, %.2f / (%.0f)", position.x, position.y,
                                    position.z, bodyYaw * 57.29578f);
        if (s_kbTimeTo60Armed) {
            if (forwardSpeed > 0.2f) {
                if (forwardSpeed < 60.0f * 1.46666f) {   // 60 mph in ft/s: 87.9996f, not 88
                    s_kbTimeTo60 += reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
                } else {
                    s_kbLastTimeTo60 = s_kbTimeTo60;
                    s_kbTimeTo60Armed = 0;
                }
            }
        } else if (forwardSpeed < 0.2f) {
            s_kbTimeTo60 = 0;
            s_kbTimeTo60Armed = 1;
        }
        reinterpret_cast<DebugOverlay*>(reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay)->Print(debugPage, "Last TimeTo60 = %.2f  seconds", s_kbLastTimeTo60);
        reinterpret_cast<DebugOverlay*>(reinterpret_cast<KbGame*>(g_TrackGame)->debugOverlay)->Print(debugPage, "TimeTo60 = %.2f  seconds", s_kbTimeTo60);
    }
    return 1;
}

// 0x00492670: sends this bike's state as message 1 (to every peer, or into the recorder when
// `record` is set) and keeps what was sent in `state`; without a recorder, peers also get
// message 10 every two seconds in modes 0 and 4.
void KrustyBike::Fn_00492670(KbBikeState* state, float dt, int record)
{
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->network && (!record || !netRecorder))
        return;
    KbBikeMessage message;
    message.position = position;
    message.roll = bodyRoll;
    message.pitch = bodyPitch;
    message.yaw = bodyYaw;
    message.velocity = velocity;
    message.angularVelocity = angularVelocity;
    message.angularVelocity.y += turnRate;
    message.lapCount = lapCount;
    message.gatesPassed = gatesPassed;
    message.loaderKind = loaderKind;
    message.lastLapTime = lastLapTime;
    message.fastestLap = fastestLap;
    message.timeBehindLeader = timeBehindLeader;
    message.finishPosition = finishPosition;
    message.poseIndex = poseIndex;
    message.poseState = poseState;
    message.poseParam = poseParam * 255.0f;
    message.poseLeanBlend = poseLeanBlend * 255.0f;
    message.poseBlend = poseBlend * 255.0f;
    if (motionActive) {
        if (motionIndex >= 0)
            message.motion = motionIndex + 1;
        else
            message.motion = 17;
    } else {
        message.motion = 0;
    }
    message.flag7 = throttleOn;
    message.flag8 = driveDisengaged;
    message.flag9 = trickPoseHeld;
    message.crashDirection = crashDirection;
    message.crashed = crashState;
    message.flag4 = UnknownVirtualSlot51();
    message.flag5 = hasFinished != 0;
    message.flag6 = onTrack;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 2 || reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 3)
        message.lapGate = (unsigned char)lapCount;
    else
        message.lapGate = lapGate;
    message.time = ReadClock();
    message.field_0x02 = (short)message.time - (short)state->time;
    if (!record) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->network)
            reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send(1, &message, sizeof(message), netId, 0);
    } else if (netRecorder) {
        reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(1, netId, &message, sizeof(message));
    }
    state->position = position;
    state->roll = bodyRoll;
    state->pitch = bodyPitch;
    state->yaw = bodyYaw;
    state->velocity = velocity;
    state->angularVelocity = angularVelocity;
    state->angularVelocity.y += turnRate;
    state->time = message.time;
    state->velocityError = kVec3Zero;
    state->positionError = kVec3Zero;
    state->angularVelocityError = kVec3Zero;
    state->rollError = 0;
    state->pitchError = 0;
    state->yawError = 0;
    state->timer = 0;
    if (!record) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->network &&
            (!reinterpret_cast<KbGame*>(g_TrackGame)->eventType || (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4 && reinterpret_cast<KbGame*>(g_TrackGame)->stuntMode))) {
            pingTimer -= reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
            if (pingTimer < 0.0f) {
                KbBikePing ping;
                ping.field_0x04 = score;
                ping.field_0x01 = loaderKind;
                reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send(10, &ping, sizeof(ping), netId, 0);
                pingTimer = 2.0f;
            }
        }
    } else if (netRecorder && race->scoreRecordDue) {
        KbBikePing ping;
        ping.field_0x04 = score;
        ping.field_0x01 = loaderKind;
        reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(10, netId, &ping, sizeof(ping));
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
    trickDelay1 = 25.0f;
    trickDelay2 = 15.0f;
    trickDelay3 = 5.0f;
    trickTimer = 0;
    inputMap = map;
    this->netId = netId;
    if (kind == 0xff) {
        isGhost = 1;
        aiKind = 0;
    } else {
        aiKind = kind;
    }
    loaderKind = kind;
    isRemote = a13;
    memset(&netState, 0, sizeof(netState));
    netState.timer = reinterpret_cast<KbGame*>(g_TrackGame)->fullNetPacketIntervalSec;
    shortMessageTimer = reinterpret_cast<KbGame*>(g_TrackGame)->shortNetPacketIntervalSec;
    this->race = race;
    netRecorder = recorder;
    field_0x15d8 = a24;
    memset(&recordState, 0, sizeof(recordState));
    recordState.timer = 1.7014117e38f;

    int localFlag;
    Vec3 points[2];
    if (!aiKind && !isRemote && !isGhost) {
        int type = reinterpret_cast<KbGame*>(g_TrackGame)->controlSetup[reinterpret_cast<KbGame*>(g_TrackGame)->controlSetup->field_0x0].field_0x28;
        if (type == -6 || type == -7 || type == -12 || type == -13)
            localFlag = 1;
        else
            localFlag = 0;
    } else {
        localFlag = 0;
    }

    float setupFloat;
    engineSize = 250;
    setupFloat = 0.0f;
    fourStroke = 0;
    if (setup) {
        KbBikeSetup copy = *setup;
        setupFloat = (float)copy.field_0x8;
        engineSize = copy.engineSize;
        fourStroke = copy.fourStroke;
    }
    if (isGhost) {
        ghostMod = (KbObj128*)new(__FILE__, 0x690) GhostMod1(1);
        GameObject::AppendChild((GameObject*)ghostMod, -1);
    }
    statusNode = 0;
    ((KbBikeLoader*)this)->Fn_004079C0(a1, a2, name, desc, a5, a6, a7, a8, a9, a10, map, a15, a16,
                                       setup == 0,
                                       setup ? (void*)setup->setupName : 0,
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
                                       reinterpret_cast<KbGame*>(g_TrackGame)->steerBinding, reinterpret_cast<KbGame*>(g_TrackGame)->leanBinding, reinterpret_cast<KbGame*>(g_TrackGame)->throttleBinding,
                                       a22, setupFloat, 0);
    if (isRemote) {
        BikeA640* axis = steerAxis;
        axis->steerValue = 0.5f;
        axis->l_0x4 = 0.42210004f;
        axis->l_0x8 = 1.0f;
    }
    if (aiKind || isGhost) {
        gasGyro = 0;
        brakeGyro = 0;
    }
    airCrashTimerReload = 3.5f;
    trickClipTime = 0;
    chainedTrickTime = 0;
    trickHoldTime = 0;
    rivalBearingScaled = 0;
    headTurnAngle = 0;
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
    interpTarget = states[1];
    interpSource = states[2];
    UnknownVirtualSlot43();

    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 0 && reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 5 && reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 1) {
        heapBufferA = DebugCalloc(100, 4, __FILE__, 0x6fa);
        if (!heapBufferA)
            return 0;
    } else {
        heapBufferA = 0;
    }
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 1 || reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 5) {
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
    animSetA[0] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarKneel", 1);
    animSetA[1] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SplitX", 1);
    animSetA[2] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BKahuna", 1);
    animSetA[3] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Cordova", 1);
    animSetA[4] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeelClicker", 1);
    animSetA[5] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("NacNac", 1);
    animSetA[6] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SuperMan", 1);
    animSetA[7] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("AirWalk", 1);
    animSetA[8] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarHop", 1);
    animSetA[9] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SeatGrab", 1);
    animSetA[10] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SaranWrap", 1);
    animSetA[11] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("CliffHanger", 1);
    animSetA[12] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeartAttack", 1);
    animSetA[13] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("TailGrab", 1);
    animSetA[14] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("LazyBoy", 1);
    animSetA[15] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("DCanCan", 1);
    animSetC[0] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarKneelAlt", 1);
    animSetC[1] = 0;
    animSetC[2] = 0;
    animSetC[3] = 0;
    animSetC[4] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeelClickerAlt", 1);
    animSetC[5] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("NacNacAlt", 1);
    animSetC[6] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SuperManAlt", 1);
    animSetC[7] = 0;
    animSetC[8] = 0;
    animSetC[9] = 0;
    animSetC[10] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SaranWrapAlt", 1);
    animSetC[11] = 0;
    animSetC[12] = 0;
    animSetC[13] = 0;
    animSetC[14] = 0;
    animSetC[15] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("DCanCanAlt", 1);

    bikeAndRiderCollision = new(__FILE__, 0x77e) CollisionObject(1);
    bikeAndRiderCollision->Configure((void*)a1, 1, 1, 1);
    bikeAndRiderCollision->LoadShape(modelNode->firstChild, "BikeAndRider.col");
    GameObject::AppendChild((GraphicsTest*)bikeAndRiderCollision, -1);
    bikeCollision = new(__FILE__, 0x784) CollisionObject(1);
    bikeCollision->Configure((void*)a1, 1, 1, 1);
    bikeCollision->LoadShape(modelNode->firstChild, "Bike.col");
    GameObject::AppendChild((GraphicsTest*)bikeCollision, -1);
    bikeCollision->Disable();
    bikeAndRiderCollision->ownerType = 100;
    bikeCollision->ownerType = 100;
    bikeAndRiderCollision->ownerObject = this;
    bikeCollision->ownerObject = this;
    collisionObject = bikeAndRiderCollision;
    collisionObject->SnapPlacement();
    wrecker->AddIgnoredObject(bikeAndRiderCollision);
    wrecker->AddIgnoredObject(bikeCollision);
    ((CollisionHullShape*)collisionObject->shape)->sceneNode = (int)modelNode->firstChild;
    ((CollisionObject*)frontWheel)->SetUseBroadphase(1);
    ((CollisionObject*)rearWheel)->SetUseBroadphase(1);
    ((CollisionObject*)wrecker->collisionModel)->SetUseBroadphase(1);
    if (isRemote) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->riderCollision) {
            bikeCollision->onHitCallback = 0;
            bikeCollision->onHitByCallback = 0;
            bikeAndRiderCollision->onHitCallback = 0;
            bikeAndRiderCollision->onHitByCallback = 0;
        } else {
            collisionObject->Disable();
            bikeCollision->Disable();
            bikeAndRiderCollision->Disable();
        }
    } else if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount > 1 && !reinterpret_cast<KbGame*>(g_TrackGame)->riderCollision) {
        collisionObject->Disable();
        bikeCollision->Disable();
        bikeAndRiderCollision->Disable();
    } else {
        bikeCollision->onHitCallback = KrustyCollisionCallbackA;
        bikeCollision->onHitByCallback = KrustyCollisionCallbackB;
        bikeAndRiderCollision->onHitCallback = KrustyCollisionCallbackA;
        bikeAndRiderCollision->onHitByCallback = KrustyCollisionCallbackB;
    }
    wrecker->EndWreck();
    lastFramePosition = position;
    if (isRemote) {
        BikeA640* axis = steerAxis;
        axis->steerValue = 0.5f;
        axis->l_0x4 = 0.42210004f;
        axis->l_0x8 = 1.0f;
    }
    drawnAtLastPosition = 0;
    field_0x15cc = 0;
    clockOffset = 0;

    g_kbLatencyHiding = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("LatencyHiding", 1);
    g_kbRateLimiting = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("RateLimiting", 1);
    g_kbAllowWarping = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("AllowWarping", 1);
    g_kbUseLatencyThreshold = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("UseLatencyThreshold", 0);
    g_kbUseExtrapLimit = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("UseExtrapLimit", 1);
    g_kbExtrapLimit = (float)reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("ExtrapLimit", 100) * kbSecondsPerMillisecond;
    g_kbLatencyThreshold = (float)reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("LatencyThreshold", 500) * kbSecondsPerMillisecond;
    g_kbWarpThreshold = (float)reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("WarpThreshold", 200) * kbSecondsPerMillisecond;
    g_kbUseTimeReceived = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("UseTimeReceived", 1);
    g_kbAllowNegative = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("AllowNegative", 1);
    g_kbInterpolate = reinterpret_cast<KbGame*>(g_TrackGame)->UnknownVirtualSlot20("Interpolate", 1);
    stateIntervalElapsed = 0;
    stateIntervalLength = 0;
    firstPacketLatch = 1;
    lastTimeReceived = 0;
    if (engineSize <= 125)
        engineClass = 0;
    else if (engineSize < 500)
        engineClass = 1;
    else
        engineClass = 2;

    if (aiKind) {
        aiTricksEnabled = 1;
        field_0x810 = 1;
        if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 0) {
            switch (reinterpret_cast<KbGame*>(g_TrackGame)->difficulty) {
            case 3:
            case 4:
                aiStuntChance = this->race->aiStuntChance3;
                break;
            case 1:
                aiStuntChance = this->race->aiStuntChance1;
                break;
            case 2:
                aiStuntChance = this->race->aiStuntChance2;
                break;
            }
        } else {
            aiStuntChance = 2.0f;
        }
        int prevTag = g_MemTagStack->Push("Collision");
        probeSegment = new(__FILE__, 0x82f) CollisionObject(1);
        probeSegment->Configure(renderContext, 1, 1, 0);
        probeSegment->ignoreVegetation = 1;
        GameObject::AppendChild((GraphicsTest*)probeSegment, -1);
        memset(&points[0], 0, sizeof(Vec3));
        memset(&points[1], 0, sizeof(Vec3));
        points[0].y = 4.0f;
        points[1].y = -1.0f;
        probeSegment->SetMeshShape(1, points);
        probeSegment->SetIgnoreListMode(1);
        probeSegment->AddIgnoredOwner(bikeCollision);
        probeSegment->AddIgnoredOwner(bikeAndRiderCollision);
        probeSegment->AddIgnoredOwner(rearWheel);
        probeSegment->AddIgnoredOwner(frontWheel);
        g_MemTagStack->Pop(prevTag);
        startRunUpTime = 0;
        contactTime = 0;
        roamTarget.x = 3000.0f;
        roamTarget.z = 3000.0f;
        roamTarget.y = 0;
        if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 5 && reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 1)
            field_0x824 = KbRandUnit() * 14.0f - 7.0f;
        else
            field_0x824 = KbRandUnit() * 6.0f - 3.0f;
    } else {
        probeSegment = 0;
        field_0x810 = 0;
        aiTricksEnabled = 0;
    }

    // Retail keeps a dead `cmp g_kbGame->field_0x2d74, 0` here: two arms that store the
    // same constant, tail-merged by VC6 (the original values were presumably tuned equal).
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 0)
        field_0x778 = 100.0f;
    else
        field_0x778 = 100.0f;
    field_0x7e0 = Vec3(0.0f, 0.0f, 0.0f);
    field_0x7ec = Vec3(0.0f, 0.0f, 0.0f);
    lookAheadFrontTravel = 0;
    lookAheadRearTravel = 0;
    ((CollisionObject*)frontWheel)->SetIgnoreListMode(1);
    ((CollisionObject*)frontWheel)->AddIgnoredOwner(bikeCollision);
    ((CollisionObject*)frontWheel)->AddIgnoredOwner(bikeAndRiderCollision);
    ((CollisionObject*)frontWheel)->AddIgnoredOwner(rearWheel);
    ((CollisionObject*)rearWheel)->SetIgnoreListMode(1);
    ((CollisionObject*)rearWheel)->AddIgnoredOwner(bikeCollision);
    ((CollisionObject*)rearWheel)->AddIgnoredOwner(bikeAndRiderCollision);
    ((CollisionObject*)rearWheel)->AddIgnoredOwner(frontWheel);
    bikeCollision->SetIgnoreListMode(1);
    bikeCollision->AddIgnoredOwner(frontWheel);
    bikeCollision->AddIgnoredOwner(rearWheel);
    bikeAndRiderCollision->SetIgnoreListMode(1);
    bikeAndRiderCollision->AddIgnoredOwner(frontWheel);
    bikeAndRiderCollision->AddIgnoredOwner(rearWheel);
    if (this->race->forceHighLod) {
        reinterpret_cast<D3DIMSoultreeObject*>(modelNode)->SetLowestLod(0);
        reinterpret_cast<D3DIMSoultreeObject*>(riderCharacter->c_0x1a0)->SetLowestLod(0);
        reinterpret_cast<D3DIMSoultreeObject*>(modelNode)->SetLod(0);
        reinterpret_cast<D3DIMSoultreeObject*>(riderCharacter->c_0x1a0)->SetLod(0);
    } else {
        reinterpret_cast<D3DIMSoultreeObject*>(modelNode)->SetLowestLod(1);
        reinterpret_cast<D3DIMSoultreeObject*>(riderCharacter->c_0x1a0)->SetLowestLod(1);
        reinterpret_cast<D3DIMSoultreeObject*>(modelNode)->SetLod(1);
        reinterpret_cast<D3DIMSoultreeObject*>(riderCharacter->c_0x1a0)->SetLod(1);
    }
    if (isGhost) {
        UnknownVirtualSlot50(1, 0.0f, 1);
        collisionObject->Disable();
    }
    if (withRider) {
        reinterpret_cast<Character*>(riderCharacter)->ApplyRestPose();
        ((SoultreeObject*)riderCharacter->c_0x1a0)->TranslateIn(0, Vec3(0.0f, 0.0f, 0.0f));
        Vec3 axisY(0.0f, 1.0f, 0.0f);
        Vec3 axisZ(0.0f, 0.0f, 1.0f);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesIn(0, &axisZ, &axisY, 0, 1);
        KbMorphMod* morph = (KbMorphMod*)new(__FILE__, 0x891) MorphBastardModifier(1);
        morph->LoadParameterFile((void*)a1, "RiderMorph.mbf", (SoultreeObject*)riderCharacter->c_0x1a0);
        GameObject::AppendChild((GameObject*)morph, -1);
        ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddSecondModifier((GameObject*)morph);
    }
    UnknownVirtualSlot102(0.001f);
    return this;
}
