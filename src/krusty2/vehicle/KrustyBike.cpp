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
// are in vehicle/BikeAI.cpp (docs/BIKEAI.md).  The functions below are in retail order;
// the ones that do not match yet carry a `// Not exact (...)` note.  Two slots are compiled
// from samples with /QIfist (config/game_build.json candidate_sources); their places are
// marked.  Member and helper names are provisional (tier 3); see KrustyBikeTypes.h.
#include "../../reconstructed/TrackGameGlobal.h"
#include <float.h>
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
    float lengthSq = v.x * v.x;   // accumulated term by term: a one-line sum loads in a different order
    lengthSq += v.y * v.y;
    lengthSq += v.z * v.z;
    if (lengthSq == 1.0f)
        return 1.0f;
    return (float)sqrt(lengthSq);
}

// File data, in retail .data order (0x0056cb88..0x0056d0a4).  The network and stall
// controls just before it (0x0056cb3c..0x0056cb68) are in src/reconstructed/KrustyBikeState.cpp.
// .data 0x0056cb88: six trick records {id, name[32]}; slot 97 registers the clips by
// these names (retail keeps the two "BackOver" entries apart).
struct KrustyTrickName {
    int id;
    char name[32];
};
static KrustyTrickName s_KrustyTricks[6] = {
    {0, "BackOver"}, {1, "Endo"}, {2, "Kahuna"}, {3, "BackOver"}, {4, "LeftOver"}, {5, "RightOver"},
};

// 0x0056cc60: the trick clip lengths in seconds; UpdateRiderPose clamps the clip time
// (+0x1520) to them and PickTrick reads them for an AI bike's hold time.
static float s_kbTrickLength[16] = {                       // 0x0056cc60 clip lengths (s)
    1.6111f, 1.3f, 2.2666f, 2.3333f, 1.6667f, 1.5f, 2.3333f, 2.4f,
    1.7222f, 1.8f, 1.7647f, 2.3333f, 2.1333f, 2.1333f, 1.9375f, 1.7222f };

// 0x0056cca0..0x0056d05f: PickTrick's twelve 0x50-byte rows (minimum time, group counts,
// trick ids); see samples/physics/bikeai/KrustyBikeTrickPick.cpp.

// 0x0056d060: the frame after which a trick's landing window (two frames, t + 1 and t + 2)
// opens.  Retail's table has 16 entries, but UpdateRiderPose also indexes it with a chained
// trick (+0x433 = 16 + trick, 16..31) and has no bound there, so retail reads the 16 words
// that follow it in .data (tier 1 bytes, tier 2 owners): s_kbTimeTo60Armed 0x0056d0a0
// (initially 1), a zero pad word, the KrustyBike TypeDescriptor 0x0056d0a8 (type_info
// vftable 0x00558f4c, spare 0, ".?AVKrustyBike@@" and its NUL) and the "RiderMorph.mbf" /
// "Interpolate" literals.  Entries 16..31 copy those words so chained tricks hold as in
// retail: the window opens for chained tricks 0 (frames 2-3), 1, 3 and 8 (frames 1-2) and
// never for the others.  Only the debug overlay's stopwatch rewrites s_kbTimeTo60Armed (0
// moves chained trick 0's window to frames 1-2); the copy keeps the 1.
static int s_kbTrickFrame[32] = {
    14, 16, 17, 18, 11, 14, 17, 18, 15, 17, 15, 18, 15, 15, 17, 16,
    1,                                                  // 0x0056d0a0 s_kbTimeTo60Armed
    0,                                                  // 0x0056d0a4 pad
    0x00558f4c, 0,                                      // 0x0056d0a8 TypeDescriptor vftable, spare
    0x56413f2e, 0x7375724b, 0x69427974, 0x4040656b,     // ".?AVKrustyBike@@"
    0,                                                  // NUL and pad
    0x65646952, 0x726f4d72, 0x6d2e6870, 0x00006662,     // 0x0056d0c4 "RiderMorph.mbf"
    0x65746e49, 0x6c6f7072, 0x00657461 };

// The debug overlay's "TimeTo60" stopwatch (Tick).
static int s_kbTimeTo60Armed = 1;   // 0x0056d0a0
static float s_kbTimeTo60;          // 0x0067c3b0
static float s_kbLastTimeTo60;      // 0x0067c3b4

// 0x0048d780 KrustyBike::PickTrick (the AI trick pick) is compiled with /QIfist from
// samples/physics/bikeai/KrustyBikeTrickPick.cpp: retail rounds its row index with the
// inline fistp helper, and this unit truncates through __ftol elsewhere.

// ---- non-virtual state reset (0x0048D8B0) ----
void KrustyBike::ResetTrickState()
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

// owner: bracket only (0x48d910; callers 0x48e584, 0x495276, 0x4955f8 are KrustyBike methods)
// Start trick clip idx: resets the stunt latches and switches both characters (rider and bike
// body) to the idx-th handle of the riderTrickMotions/bikeTrickMotions tables ("BarKneel" ...).
void KrustyBike::StartTrick(int idx)
{
    EndMotion();
    motionActive = 1;
    motionCompleted = 0;
    motionJustStarted = 1;
    trickClipTime = 0;
    chainedTrickTime = 0;
    trickHoldTime = 0;
    motionIndex = (char)idx;
    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(riderTrickMotions[idx]));
    ((KbA5C4*)riderCharacter)->motionTime = 0;
    D3DIMSoultreeCharacter::SetMotion((Motion*)bikeTrickMotions[idx]);
    motionTime = 0;
}

// owner: bracket only (0x48d990; callers 0x48eff2, 0x48f115, 0x49562c are KrustyBike methods)
// Variant of StartTrick that banks trickClipTime in chainedTrickTime and uses riderAltTrickMotions/bikeAltTrickMotions.
void KrustyBike::StartAltTrick(int idx)
{
    motionActive = 1;
    chainedTrickTime = trickClipTime;
    trickClipTime = 0;
    motionIndex = (char)(idx + 0x10);
    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(riderAltTrickMotions[idx]));
    D3DIMSoultreeCharacter::SetMotion((Motion*)bikeAltTrickMotions[idx]);
    trickChained = 1;
}

// Slot 27 (StartWreck, tier 2): run once by slot 28 when a crash set wreckPending;
// starts the wrecker's tumble or animated wreck and switches to the bike collision.
void KrustyBike::StartWreck()
{
    if (crashReason == 9 || motionActive) {
        wrecker->StartTumble();
    } else {
        wrecker->StartAnimatedWreck(crashDirection);
    }
    SwitchToBikeCollision();
}

// Slot 28 (0x0048da50): per-frame hook.  In mode 4 (tier 3: a spectator/replay mode) it
// re-targets the session's player record onto this bike, then runs the base update and,
// while airborne (crashState set, prevCrashState clear), jitters the two 0x7ac/0x7b0
// accumulators with random noise (0.15 scale for an aiKind bike, 0.667 otherwise).
int KrustyBike::ResolveSubstepCollisions(int steps)
{
    lastCollisionType = 0;
    if (isGhost) return steps;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4) {
        KbPlayer* raceMode = reinterpret_cast<KbPlayer*>(reinterpret_cast<EventManager*>(
            reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindRaceMode());
        if (this != raceMode->followedRacer && this == race->localRacer) {
            Vehicle* followed = raceMode->followedRacer;
            if (followed) {
                if (collisionObject->TestMeshBounds(followed->collisionObject))
                    SendFollowedRacer(this, 1);
            } else if (raceMode->ball) {
                if (collisionObject->TestMeshBounds(raceMode->ball->collisionObject)) {
                    if (reinterpret_cast<KbGame*>(g_TrackGame)->network->isHost)
                        raceMode->SetFollowedRacer(this, 0, 1);
                    else
                        SendFollowedRacer(this, 1);
                }
            }
        }
        if (isRemote) return steps;
    }
    int result = Vehicle::ResolveSubstepCollisions(steps);
    if (crashState && !prevCrashState) {
        if (aiKind) {
            repairDamage += KbRandUnit() * 0.15f;
            injuryDamage += KbRandUnit() * 0.15f;
        } else {
            repairDamage += KbRandUnit() * 0.667f;
            injuryDamage += KbRandUnit() * 0.667f;
        }
    }
    return result;
}

void KrustyBike::ApplyStaticContactImpulse(const Vec3* normal, const Vec3* relVel, const Vec3* arm,
                                     const Vec3* angScale, int eventKind, int scaleBits, float* impulseOut)
{
    if (eventKind == 1000) {
        race->handler->NotifyContactImpulse(this, 0, eventKind);
        return;
    }
    Bike::ApplyStaticContactImpulse(normal, relVel, arm, angScale, eventKind, scaleBits, impulseOut);
    if (eventKind != 0x67)
        race->handler->NotifyContactImpulse(this, *impulseOut, eventKind);
}

void KrustyBike::ApplyBodyContactImpulse(const Vec3* normal, Vec3* velA, const Vec3* worldAngVelA,
                                     const Vec3* armA, int eventKind, float invMassB, int nodeB,
                                     const Vec3* velB, const Vec3* worldAngVelB, const Vec3* armB,
                                     Vec3* invInertiaB, Vec3* angVelB, int velBOrNull,
                                     float* impulseOut, float scale)
{
    Bike::ApplyBodyContactImpulse(normal, velA, worldAngVelA, armA, eventKind, invMassB, nodeB, velB,
                              worldAngVelB, armB, invInertiaB, angVelB, velBOrNull, impulseOut, scale);
    race->handler->NotifyContactImpulse(this, *impulseOut, eventKind);
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
    if ((bike->IsSpawnProtected() || bike->hasFinished) && kind != 0x66 && kind != 0x6a)
        return;
    if (kind == 0x64) {
        KrustyBike* otherBike = (KrustyBike*)other->ownerObject;
        if (otherBike->IsSpawnProtected() || otherBike->hasFinished)
            return;
    } else if (kind == 0x65) {
        return;
    }
    bike->ApplyCollisionEvent(bike->CanCrash() && kind != 0x3e8, kind, other);
}

// owner: bracket only (0x48dd80; referenced by pointer from the collision setup)
// Second collision callback: for type tags 0x2711 / 0x69 the other object's +0x5c vector is added to
// the bike's position and the bike's collision object is refreshed before the common handling.
void KrustyCollisionCallbackB(CollisionObject* self, CollisionObject* other)
{
    KrustyBike* bike = (KrustyBike*)self->ownerObject;
    int kind = other->ownerType;
    if (bike->IsSpawnProtected())
        return;
    if (bike->isRemote)
        return;
    if (bike->hasFinished)
        return;
    if (kind == 0x64 || kind == 0x65)
        return;
    if (kind == 0x2711) {
        Vec3* pos = &bike->position;
        Vec3* offset = (Vec3*)other->contactRecord;
        pos->x = pos->x + offset->x;
        pos->y = offset->y + pos->y;
        pos->z = offset->z + pos->z;
        bike->modelNode->SetPosition(*pos);
        bike->collisionObject->UpdatePlacement();
    } else if (kind == 0x69) {
        Vec3* pos = &bike->position;
        Vec3* offset = (Vec3*)other->contactRecord;
        pos->x = offset->x + pos->x;
        pos->y = offset->y + pos->y;
        pos->z = offset->z + pos->z;
        bike->modelNode->SetPosition(*pos);
        bike->collisionObject->UpdatePlacement();
    }
    KrustyCollisionCallbackA(self, other);
}

int KrustyBike::ClassifyOutOfBounds(int grounded)
{
    if (grounded && reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 3) {
        float lo = terrainScale * 105.0f;
        if (position.x < lo)
            return 1;
        KbTrackB* grid = ((KbTrackA*)track)->trackData;
        if ((grid->sizeX * 256.0f - 105.0f) * terrainScale < position.x)
            return 2;
        if (position.z < lo)
            return 3;
        if ((grid->sizeZ * 256.0f - 105.0f) * terrainScale < position.z)
            return 4;
    }
    return 0;
}

// Slot 43: reset of the per-bike state (runs the Vehicle reset, then clears the 0x74c..0x7c0
// block, the statusNode record and a few late fields).  The record's 0x44 vector is taken from
// the race context (0xf8..0x100, or from the object at +0x48 when no 0x144 object exists).
void KrustyBike::SnapshotRespawnPose()
{
    Vehicle::SnapshotRespawnPose();
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
        KbRaceSub* track = race->raceTrack;
        if (track) {
            if (race->hasStartProbe) {
                statusNode->progressPos.a = race->startProbeNode;
                statusNode->progressPos.b = race->startProbeSegment;
                statusNode->progressPos.c = race->startProbeT;
            } else {
                statusNode->progressPos.a = (int)track->startNode;
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

int KrustyBike::ResetToPlacement(const Vec3* newPosition, const Vec3* newForward, const Vec3* newUp,
                                     const Vec3* settleUp, int keepPlacement, float extraWeight)
{
    int result = Bike::ResetToPlacement(newPosition, newForward, newUp, settleUp, keepPlacement, extraWeight);
    lastFramePosition = position;
    wrecker->skipProbeEffects = 1;
    return result;
}

// owner: bracket only (0x48e190; callers 0x48e285 and the camera code)
// Nearest other bike of the race (squared planar distance), skipping those with outOfRace set;
// optionally returns the distance.
KrustyBike* KrustyBike::FindNearestRival(float* outDistance)
{
    int cursor = 0;
    KrustyBike* candidate =
        reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
    while (candidate && candidate->outOfRace)
        candidate = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
    KrustyBike* best = 0;
    float bestSq = 3.4028235e38f;
    while (candidate) {
        if (candidate != this) {
            float dx = candidate->position.x - position.x;
            float dz = candidate->position.z - position.z;
            float sq = dx * dx;
            sq += dz * dz;
            if (sq < bestSq) {
                bestSq = sq;
                best = candidate;
            }
        }
        candidate = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
        while (candidate && candidate->outOfRace)
            candidate = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor));
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
void KrustyBike::LookBackAtNearestRival()
{
    nearestRival = FindNearestRival(0);
    if (!nearestRival)
        return;
    Vec3 toRival = nearestRival->position - position;
    float angle = (float)(atan2(toRival.x, toRival.z)) - savedYaw;
    if (angle < -3.14159274f)
        angle += 6.28318548f;
    else if (angle > 3.14159274f)
        angle -= 6.28318548f;
    rivalBearing = angle;
    if (angle < 0)
        angle = -angle;
    if (angle <= 0.698f)
        return;
    EndMotion();
    motionCompleted = 0;
    motionActive = 1;
    motionJustStarted = 1;
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
    reinterpret_cast<Character*>(riderCharacter)->ApplyMotionAtPhase(riderPoseHandles[RiderPose_LookBack], 0.5f);
    D3DIMSoultreeCharacter::ApplyMotionAtPhase(bikePoseHandles[RiderPose_LookBack], 0.5f);
}

// ---- slot 89: rider pose selection (0x0048e3e0, overrides Bike 0x00406ae0) ----
// Retail rounds the pose index with its inline fistp helper (three sites), and this unit
// also truncates through __ftol in five functions as retail does, so it cannot take /QIfist.
// The body is samples/physics/krustybike/KrustyBikeSelectRiderPose.cpp, which the game
// build compiles with /QIfist (config/game_build.json candidate_sources).

// 0x0048eea0, KrustyBike slot 102 (overrides Bike 0x00407440): the rider/bike pose update.
// Crashed (or +0x5b8 set): Bike's steer pose (pose 1) unless remote.  Otherwise, with +0x430
// set, either a trick clip runs (+0x433 = 16 + trick) or the head turns towards the nearest
// rival (+0x433 = -1, started by 0x0048e280); without it, the +0x7a4 == 2 end pose or Bike's
// lean-pose blend with the KrustyBike extras (poses 16/17 and the four-pose blend 0x004a9050).
// It reads the trick tables s_kbTrickLength and s_kbTrickFrame (the file data above).

// Whether trick `trick` may chain from the current input direction (0..7): not when the
// input is the trick's own direction or one of its two neighbours in the table.
static inline int KbTrickChains(int trick, int input)
{
    switch (trick % 8) {
    case 0:
        return input != 0 && input != 4 && input != 5;
    case 1:
        return input != 1 && input != 5 && input != 6;
    case 2:
        return input != 2 && input != 6 && input != 7;
    case 3:
        return input != 3 && input != 4 && input != 7;
    case 4:
        return input != 4 && input != 0 && input != 3;
    case 5:
        return input != 5 && input != 0 && input != 1;
    case 6:
        return input != 6 && input != 1 && input != 2;
    case 7:
        return input != 7 && input != 2 && input != 3;
    }
    return 0;
}

static inline float KbMinF(float a, float b) { return a < b ? a : b; }

// Near miss (0x0048eea0..0x0048fa08, 2920 bytes with the 8-entry switch table at 0x0048f9e8;
// sdiff ratio 0.955, 69 differing instruction lines): calls, the 0x50 frame, every branch,
// constant and store match.  Measured: `int remote = a || b;` (not if/assign) gives retail's
// `cmp [g+0x3428], ebp`; the rider pose-16 test on the member poseIndex (the bike-side test
// reloads it) stops VC6 threading the idx == 16 path past the 11..13 test; the head-turn
// look value is computed after each join (a per-arm `headTurnAngle * 0.71f` folds 0.24 * 0.71)
// and the turning value is a local (retail's dt-slot temporary); `(x * 0.36963f) * 0.7f`
// keeps the two multiplies.  Left: (1) VC6 cross-jumps the head-turn arms differently: retail
// keeps the + side hold block (the - side jumps into it after `fld head`) and the - side
// `1558 = angle; 1550 = target` tail; the store order of the hold block moves the merge
// (1554/154d/1558 gives retail's per-arm schedule without the merge, 1554/1558/154d merges
// into the - copy); (2) the trick-chain switch ends each case with `jne call; jmp skip`
// where retail has `je skip; jmp call` (an inline predicate, a blocked-predicate, per-case
// if/return and a goto-skip switch compile identically); (3) the pending-trick else block
// (`+0x153c = 0`) sits before the shared advance instead of after it (inverted, nested and
// goto forms, and the advance copied into both arms, do not move it); (4) the crash path keeps
// t on the x87 stack where retail spills it to the dead dt slot (Bike's own slot 102 spills
// with the same text; t as a separate statement, the dt parameter itself, if/else and an
// inline max do not).
// Not exact (608 of 2952 bytes, 2952 against retail's 2920; calls, frame 0x50 and constants
// match: VC6 cross-jumps the head-turn arms and the trick-chain switch tails differently, places
// the +0x153c clear before the shared advance, and keeps the crash clamp's t on the x87 stack).
void KrustyBike::UpdateRiderPose(float dt)
{
    if (crashState || *(int*)pad_0x5B8) {
        if (!isRemote && !reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay) {
            float t = -(steerState->angle / steerState->maxAngle);
            t = (t + 1.0f) * 0.25f;
            t += 0.25f;
            poseParam = (t > 0.25f) ? KbMinF(t, 0.75f) : 0.25f;
        }
        D3DIMSoultreeCharacter::ApplyMotionAtPhase(bikePoseHandles[RiderPose_LeanR02], poseParam);
        return;
    }
    if (motionActive) {
        if (motionIndex >= 0) {
            // Trick clip.
            if (motionJustStarted) {
                motionJustStarted = 0;
                ((Character*)riderCharacter)->AdvanceMotionInterpolated(0, 0, 0);
                AdvanceMotionInterpolated(0, 0, 0);
            } else {
                int remote = isRemote || reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay;
                int held;
                if (!aiKind && !remote && (QueryControlInput(6, 0x3f) || QueryControlInput(7, 0x3f)))
                    held = 1;
                else
                    held = 0;
                trickClipTime += dt;
                if (motionIndex < 16 && trickClipTime > s_kbTrickLength[motionIndex])
                    trickClipTime = s_kbTrickLength[motionIndex];
                if (!held && aiTrickHoldLeft == 0.0f) {
                    if (remote) {
                        if (!trickPoseHeld) {
                            if (trickPoseHeldPrev && motionIndex > 15)
                                StartAltTrick(motionIndex - 16);
                            ((Character*)riderCharacter)->AdvanceMotionInterpolated(dt, 0, 0);
                            AdvanceMotionInterpolated(dt, 0, 0);
                        }
                    } else {
                        if (trickPoseHeld && motionIndex < 16 && riderAltTrickMotions[motionIndex]) {
                            trickPoseHeld = 0;
                            int input = ClassifyInputDirection();
                            if (input != -1 && KbTrickChains(motionIndex, input))
                                StartAltTrick(motionIndex);
                        } else {
                            trickPoseHeld = 0;
                        }
                        ((Character*)riderCharacter)->AdvanceMotionInterpolated(dt, 0, 0);
                        AdvanceMotionInterpolated(dt, 0, 0);
                    }
                } else {
                    int frame = D3DIMSoultreeCharacter::currentFrame->frameIndex;
                    if (frame > s_kbTrickFrame[motionIndex] && frame < s_kbTrickFrame[motionIndex] + 3) {
                        trickPoseHeld = 1;
                        if (aiKind) {
                            aiTrickHoldLeft -= dt;
                            if (aiTrickHoldLeft < 0.0) {
                                aiTrickHoldLeft = 0;
                                trickPoseHeld = 0;
                            }
                        } else {
                            trickHoldTime += dt;
                            trickClipTime -= dt;
                        }
                    } else {
                        ((Character*)riderCharacter)->AdvanceMotionInterpolated(dt, 0, 0);
                        AdvanceMotionInterpolated(dt, 0, 0);
                    }
                }
            }
        } else {
            // Head turn towards the nearest rival: +0x1554 is the current head angle, +0x1550
            // its target (bearing +0x1558 scaled into the pose range), +0x154d the turn phase,
            // +0x154e the return to straight ahead.
            float look;
            if (headTurningBack) {
                // Turning back to straight ahead.
                if (rivalBearingScaled < 0.0f) {
                    headTurnAngle += dt;
                    if (headTurnAngle > 0.0f) {
                        headTurnAngle = 0;
                        motionActive = 0;
                        motionIndex = 0;
                        headTurningBack = 0;
                    }
                } else {
                    headTurnAngle -= dt;
                    if (headTurnAngle < 0.0f) {
                        headTurnAngle = 0;
                        motionActive = 0;
                        motionIndex = 0;
                        headTurningBack = 0;
                    }
                }
                look = headTurnAngle * 0.71f;
            } else if (headTurningToRival) {
                // Turning towards the target; near it, the rival's bearing is taken again.
                if (rivalBearingScaled > 0.0f) {
                    float head = headTurnAngle + dt;
                    headTurnAngle = head;
                    if (head > rivalBearingScaled) {
                        headTurningToRival = 0;
                        headTurnAngle = 0.24f;
                        look = rivalBearing * 0.1837f;
                    } else if (rivalBearingScaled - 0.1f < head) {
                        Vec3 toRival = nearestRival->position - position;
                        float angle = (float)(atan2(toRival.x, toRival.z)) - savedYaw;
                        if (angle < -3.14159274f)
                            angle += 6.28318548f;
                        else if (angle > 3.14159274f)
                            angle -= 6.28318548f;
                        float target = (angle * 0.36963f) * 0.7f;
                        if (angle < rivalBearing) {
                            if (head > target) {
                                // Passed the new target: hold the head where it is.
                                headTurnAngle = 0.24f;
                                headTurningToRival = 0;
                                rivalBearing = head * 3.864687f;
                            } else {
                                rivalBearing = angle;
                                rivalBearingScaled = target;
                            }
                        } else if (angle > 2.7f) {
                            rivalBearing = 2.7f;
                            rivalBearingScaled = 0.7f;
                        } else {
                            rivalBearing = angle;
                            rivalBearingScaled = target;
                        }
                        look = headTurnAngle * 0.71f;
                    } else {
                        look = head * 0.71f;
                    }
                } else if (rivalBearingScaled < 0.0f) {
                    float head = headTurnAngle - dt;
                    headTurnAngle = head;
                    if (head < rivalBearingScaled) {
                        headTurningToRival = 0;
                        headTurnAngle = 0.24f;
                        look = rivalBearing * 0.1837f;
                    } else if (rivalBearingScaled + 0.1f > head) {
                        Vec3 toRival = nearestRival->position - position;
                        float angle = (float)(atan2(toRival.x, toRival.z)) - savedYaw;
                        if (angle < -3.14159274f)
                            angle += 6.28318548f;
                        else if (angle > 3.14159274f)
                            angle -= 6.28318548f;
                        float target = (angle * 0.36963f) * 0.7f;
                        if (angle > rivalBearing) {
                            if (head < target) {
                                // Passed the new target: hold the head where it is.
                                headTurnAngle = 0.24f;
                                headTurningToRival = 0;
                                rivalBearing = head * 3.864687f;
                            } else {
                                rivalBearing = angle;
                                rivalBearingScaled = target;
                            }
                        } else if (angle < -2.7f) {
                            rivalBearing = -2.7f;
                            rivalBearingScaled = -0.7f;
                        } else {
                            rivalBearing = angle;
                            rivalBearingScaled = target;
                        }
                        look = headTurnAngle * 0.71f;
                    } else {
                        look = head * 0.71f;
                    }
                } else {
                    headTurningToRival = 0;
                    headTurnAngle = 0.24f;
                    look = rivalBearing * 0.1837f;
                }
            } else {
                // Holding: once the hold time runs out, turn back from the current bearing.
                headTurnAngle -= dt;
                if (headTurnAngle < 0.0f) {
                    headTurningBack = 1;
                    rivalBearingScaled = headTurnAngle = (rivalBearing * 0.36963f) * 0.7f;
                    look = headTurnAngle * 0.71f;
                } else {
                    look = rivalBearing * 0.1837f;
                }
            }
            float pose = look + 0.5f;
            reinterpret_cast<Character*>(riderCharacter)->ApplyMotionAtPhase(riderPoseHandles[RiderPose_LookBack], pose);
            D3DIMSoultreeCharacter::ApplyMotionAtPhase(bikePoseHandles[RiderPose_LookBack], pose);
        }
    } else if (hasFinished == 2) {
        // The end pose (riderPoseHandles[RiderPose_Victory]) once the rider character is ready.
        if (riderCharacter->c_0xc) {
            reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(riderPoseHandles[RiderPose_Victory]));
            ((KbA5C4*)riderCharacter)->motionTime = 0;
            hasFinished++;
        } else {
            ((Character*)riderCharacter)->AdvanceMotionInterpolated(dt, 0, 0);
        }
        if (hasFinished > 2) {
            if (crossoverPose)
                EndMotion();
            motionCompleted = 0;
        }
    } else {
        int cnt = poseState;
        int idx = poseIndex;
        int next;
        int other;
        int blend;
        if (crossoverPose) {
            next = idx + 1;
            if (next > RiderPose_CrosOv3)
                next = RiderPose_CrosOv3;
            blend = 0;
        } else if (idx > RiderPose_LeanR03) {
            next = idx + 2;
            if (idx == RiderPose_Wheelie) {
                next = idx;
                blend = 0;
            } else {
                other = cnt + 2;
                if (other > RiderPose_Wheelie)
                    other = RiderPose_Wheelie;
                blend = 1;
            }
        } else {
            other = cnt + 2;
            next = idx + 1;
            if (other > RiderPose_Turn3U)
                other = RiderPose_Turn3U;
            blend = 1;
        }
        float inverseBlend = 1.0f - poseBlend;
        if (idx > RiderPose_LeanR03)
            blend = 0;
        if (poseIndex == RiderPose_BackUpTop) {
            if (backUpPhase < 0.45f) {
                float lean = forwardSpeed;
                if (lean < 0.0f)
                    lean = -lean;
                float minimum = dt * 0.55371f;
                float rate = lean * dt * 0.55371f;
                backUpPhaseRate = (rate > minimum) ? rate : minimum;
            }
            backUpPhase += backUpPhaseRate;
            if (backUpPhase > 1.0f)
                backUpPhase -= 1.0f;
            reinterpret_cast<Character*>(riderCharacter)->ApplyMotionAtPhase(riderPoseHandles[RiderPose_BackUpTop], poseParam);
            reinterpret_cast<Character*>(riderCharacter)->ApplyMotionAtPhase(riderPoseHandles[RiderPose_BackUpBottom], backUpPhase);
        } else if (blend) {
            ((Character*)riderCharacter)->ApplyFourMotionBlendAtPhase(riderPoseHandles[idx], riderPoseHandles[next],
                riderPoseHandles[cnt], riderPoseHandles[other], poseParam, inverseBlend, poseLeanBlend);
        } else {
            reinterpret_cast<Character*>(riderCharacter)->ApplyMotionBlendAtPhase(riderPoseHandles[idx], riderPoseHandles[next], poseParam, inverseBlend);
        }
        if (idx >= RiderPose_CrosOv1 && idx <= RiderPose_CrosOv3) {
            D3DIMSoultreeCharacter::ApplyMotionBlendAtPhase(bikePoseHandles[idx], bikePoseHandles[next], poseParam, inverseBlend);
        } else if (poseIndex == RiderPose_BackUpTop) {
            D3DIMSoultreeCharacter::ApplyMotionAtPhase(bikePoseHandles[RiderPose_BackUpTop], poseParam);
        } else if (blend) {
            D3DIMSoultreeCharacter::ApplyFourMotionBlendAtPhase(bikePoseHandles[idx], bikePoseHandles[next],
                bikePoseHandles[cnt], bikePoseHandles[other], poseParam, inverseBlend, poseLeanBlend);
        } else {
            D3DIMSoultreeCharacter::ApplyMotionBlendAtPhase(bikePoseHandles[idx], bikePoseHandles[next], poseParam, inverseBlend);
        }
    }
    int mode = wrecker->wreckState;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        modelNode->GetMatrixIn(0, &tmp);
        reinterpret_cast<SoultreeObject*>(riderCharacter->c_0x1a0)->SetMatrixIn(0, &tmp);
    }
}

void KrustyBike::ResetDynamicState(float extraWeight)
{
    Bike::ResetDynamicState(extraWeight);
    ResetTrickState();
    aiTrickHoldLeft = 0;
    headTurningToRival = 0;
    headTurningBack = 0;
    nearestRival = 0;
    trickLandTimer = 0;
    offTrackTime = 0;
    trickPoseHeldPrev = 0;
    contactTime = 0;
}

// 0x005541e8: a TU-local constant (it sits in KrustyBike.cpp's .rdata between 1/3600 and the
// first vtable pointers, not in the shared `__real@4@` pool at 0x005507d4 that the remote
// update uses for its own 0.001f).  Only the loader's three millisecond settings read it.
static const float kbSecondsPerMillisecond = 0.001f;

// 0x0048fa60 (532 bytes, ret 8 = flags + the hidden most-derived flag; exact): the
// GameObject virtual base is built only for the most-derived object (GameObject(1)), then
// Bike(flags); SEH handler 0x0054bc9d.  The rest is member initialisation.  VC6 fills the
// load delay of the +0x1540 kVec3Zero copy with the two stores at a fixed distance from
// the end of the block (here hasFullState and wrecker, written after nextLapGate), which
// is where retail has them; with those two written right after the copy, VC6 hoists
// forceGatePass/nextLapGate instead (472/532).  The statement order is the evidence; the
// original grouping of these stores is not otherwise known.
// Re-measured 2026-11-14: with the pair at ten other places VC6 always hoists the two
// statements that have 31 and 30 statements after them; removing a later statement of any
// kind (int, byte, float or Vec3 copy) moves the pick down by one, removing one between
// the copy and the pair does not.  Vehicle's constructor does not behave this way.
KrustyBike::KrustyBike(int flags) : GameObject(1), Bike(flags)
{
    plateNumber = 0x65;
    lastFramePosition = kVec3Zero;
    aiKind = 0;
    isRemote = 0;
    score = 0;
    pingTimer = 0;
    race = 0;
    lapGate = 0;
    forceGatePass = 0;
    nextLapGate = 1;
    hasFullState = 0;
    wrecker = 0;
    finishTimeStamp = 0;
    netId = 0;
    trickLandTimer = 0;
    driveForceScale = 1.0f;
    largestStuntPoints = 0;
    totalTrickTime = 0;
    bestTagRun = 0;
    tagRun = 0;
    totalTagRun = 0;
    ghostMod = 0;
    debugPage = -1;
    spawnProtectForced = 0;
    spawnProtectEndPending = 0;
    isGhost = 0;
    repairDamage = 0;
    injuryDamage = 0;
    stuntEarnings = 0;
    deltaBaseVelocity = kVec3Zero;
    deltaBasePosition = kVec3Zero;
    deltaBaseAngularVelocity = kVec3Zero;
    deltaBaseRoll = 0;
    deltaBasePitch = 0;
    deltaBaseYaw = 0;
    deltaBaseTime = 0;
    rivalBearing = 0;
    aiTrickId = 0;
    lastCollisionType = 0;
    bikeCollision = 0;
    bikeAndRiderCollision = 0;
}

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
//  * the dead `cmp g->eventType, 0` survives as an if/else with identical 100.0f arms
//    (tail-merged), which also restores the register rotation of the whole tail;
//  * a local `BikeA640* axis` keeps the steer axis in one register for its three stores and
//    0.42210004f is needed for retail's 0x3ed81d7f (0.4221f rounds to 0x3ed81d7e);
//  * 0.001f comes from the TU-local kbSecondsPerMillisecond, not the shared pool.
GameObject* KrustyBike::Load(int context, int engineName, const char* modelPath, const SoultreeLoadDesc* riderModel, int desc, int info,
                             Vec3 startPosition, Vec3 forward, Vec3 up, int terrainProbe, VehicleInputMap* map, unsigned char kind,
                             unsigned char remote, KbRace* race, int riderName, int a16, int netId,
                             const KbBikeSetup* setup, int gasGyroOption, int brakeGyroOption, int linkedObject, int trackObject,
                             KbRecorder* recorder, void* aiSkill, int withRider)
{
    aiAccelScale1 = 1.1f;
    aiAccelScale2 = 1.3f;
    aiAccelScale3 = 1.5f;
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
    isRemote = remote;
    memset(&netState, 0, sizeof(netState));
    netState.timer = reinterpret_cast<KbGame*>(g_TrackGame)->fullNetPacketIntervalSec;
    shortMessageTimer = reinterpret_cast<KbGame*>(g_TrackGame)->shortNetPacketIntervalSec;
    this->race = race;
    netRecorder = recorder;
    this->aiSkill = aiSkill;
    memset(&recordState, 0, sizeof(recordState));
    recordState.timer = 1.7014117e38f;

    int localFlag;
    Vec3 points[2];
    if (!aiKind && !isRemote && !isGhost) {
        int type = reinterpret_cast<KbGame*>(g_TrackGame)->controlSetup[reinterpret_cast<KbGame*>(g_TrackGame)->controlSetup->currentDevice].throttleCode;
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
        setupFloat = (float)copy.weight;
        engineSize = copy.engineSize;
        fourStroke = copy.fourStroke;
    }
    if (isGhost) {
        ghostMod = (KbObj128*)new(__FILE__, 0x690) GhostMod1(1);
        GameObject::AppendChild((GameObject*)ghostMod, -1);
    }
    statusNode = 0;
    ((KbBikeLoader*)this)->LoadBike(context, engineName, modelPath, riderModel, desc, info, startPosition, forward, up, terrainProbe, map, riderName, a16,
                                       setup == 0,
                                       setup ? (void*)setup->eqBands : 0,
                                       setup ? setup->firstBand : 0,
                                       setup ? setup->lastBand : 0,
                                       setup ? setup->bandStep : 0,
                                       setup ? setup->frontComp : 0,
                                       setup ? setup->frontDecomp : 0,
                                       setup ? setup->rearComp : 0,
                                       setup ? setup->rearDecomp : 0,
                                       setup ? setup->frontDamp : 0,
                                       setup ? setup->rearDamp : 0,
                                       gasGyroOption, brakeGyroOption, linkedObject, localFlag,
                                       reinterpret_cast<KbGame*>(g_TrackGame)->steerBinding, reinterpret_cast<KbGame*>(g_TrackGame)->leanBinding, reinterpret_cast<KbGame*>(g_TrackGame)->throttleBinding,
                                       trackObject, setupFloat, 0);
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
    SnapshotRespawnPose();

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

    LoadBodyAxes();
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
    crashHeading = Vec3(bodyForward.x, 0.0f, bodyForward.z);

    bikeTrickMotions[0] = (int)FindMotion("bBarKneel", 1);
    bikeTrickMotions[1] = (int)FindMotion("bSplitX", 1);
    bikeTrickMotions[2] = (int)FindMotion("bBKahuna", 1);
    bikeTrickMotions[3] = (int)FindMotion("bCordova", 1);
    bikeTrickMotions[4] = (int)FindMotion("bHeelClicker", 1);
    bikeTrickMotions[5] = (int)FindMotion("bNacNac", 1);
    bikeTrickMotions[6] = (int)FindMotion("bSuperMan", 1);
    bikeTrickMotions[7] = (int)FindMotion("bAirWalk", 1);
    bikeTrickMotions[8] = (int)FindMotion("bBarHop", 1);
    bikeTrickMotions[9] = (int)FindMotion("bSeatGrab", 1);
    bikeTrickMotions[10] = (int)FindMotion("bSaranWrap", 1);
    bikeTrickMotions[11] = (int)FindMotion("bCliffHanger", 1);
    bikeTrickMotions[12] = (int)FindMotion("bHeartAttack", 1);
    bikeTrickMotions[13] = (int)FindMotion("bTailGrab", 1);
    bikeTrickMotions[14] = (int)FindMotion("bLazyBoy", 1);
    bikeTrickMotions[15] = (int)FindMotion("bDCanCan", 1);
    bikeAltTrickMotions[0] = (int)FindMotion("bBarKneelAlt", 1);
    bikeAltTrickMotions[1] = 0;
    bikeAltTrickMotions[2] = 0;
    bikeAltTrickMotions[3] = 0;
    bikeAltTrickMotions[4] = (int)FindMotion("bHeelClickerAlt", 1);
    bikeAltTrickMotions[5] = (int)FindMotion("bNacNacAlt", 1);
    bikeAltTrickMotions[6] = (int)FindMotion("bSuperManAlt", 1);
    bikeAltTrickMotions[7] = 0;
    bikeAltTrickMotions[8] = 0;
    bikeAltTrickMotions[9] = 0;
    bikeAltTrickMotions[10] = (int)FindMotion("bSaranWrapAlt", 1);
    bikeAltTrickMotions[11] = 0;
    bikeAltTrickMotions[12] = 0;
    bikeAltTrickMotions[13] = 0;
    bikeAltTrickMotions[14] = 0;
    bikeAltTrickMotions[15] = (int)FindMotion("bDCanCanAlt", 1);
    riderTrickMotions[0] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarKneel", 1);
    riderTrickMotions[1] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SplitX", 1);
    riderTrickMotions[2] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BKahuna", 1);
    riderTrickMotions[3] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("Cordova", 1);
    riderTrickMotions[4] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeelClicker", 1);
    riderTrickMotions[5] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("NacNac", 1);
    riderTrickMotions[6] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SuperMan", 1);
    riderTrickMotions[7] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("AirWalk", 1);
    riderTrickMotions[8] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarHop", 1);
    riderTrickMotions[9] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SeatGrab", 1);
    riderTrickMotions[10] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SaranWrap", 1);
    riderTrickMotions[11] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("CliffHanger", 1);
    riderTrickMotions[12] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeartAttack", 1);
    riderTrickMotions[13] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("TailGrab", 1);
    riderTrickMotions[14] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("LazyBoy", 1);
    riderTrickMotions[15] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("DCanCan", 1);
    riderAltTrickMotions[0] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("BarKneelAlt", 1);
    riderAltTrickMotions[1] = 0;
    riderAltTrickMotions[2] = 0;
    riderAltTrickMotions[3] = 0;
    riderAltTrickMotions[4] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("HeelClickerAlt", 1);
    riderAltTrickMotions[5] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("NacNacAlt", 1);
    riderAltTrickMotions[6] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SuperManAlt", 1);
    riderAltTrickMotions[7] = 0;
    riderAltTrickMotions[8] = 0;
    riderAltTrickMotions[9] = 0;
    riderAltTrickMotions[10] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("SaranWrapAlt", 1);
    riderAltTrickMotions[11] = 0;
    riderAltTrickMotions[12] = 0;
    riderAltTrickMotions[13] = 0;
    riderAltTrickMotions[14] = 0;
    riderAltTrickMotions[15] = (int)reinterpret_cast<Character*>(riderCharacter)->FindMotion("DCanCanAlt", 1);

    bikeAndRiderCollision = new(__FILE__, 0x77e) CollisionObject(1);
    bikeAndRiderCollision->Configure((void*)context, 1, 1, 1);
    bikeAndRiderCollision->LoadShape(modelNode->firstChild, "BikeAndRider.col");
    GameObject::AppendChild((GraphicsTest*)bikeAndRiderCollision, -1);
    bikeCollision = new(__FILE__, 0x784) CollisionObject(1);
    bikeCollision->Configure((void*)context, 1, 1, 1);
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

    g_kbLatencyHiding = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("LatencyHiding", 1);
    g_kbRateLimiting = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("RateLimiting", 1);
    g_kbAllowWarping = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("AllowWarping", 1);
    g_kbUseLatencyThreshold = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("UseLatencyThreshold", 0);
    g_kbUseExtrapLimit = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("UseExtrapLimit", 1);
    g_kbExtrapLimit = (float)reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("ExtrapLimit", 100) * kbSecondsPerMillisecond;
    g_kbLatencyThreshold = (float)reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("LatencyThreshold", 500) * kbSecondsPerMillisecond;
    g_kbWarpThreshold = (float)reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("WarpThreshold", 200) * kbSecondsPerMillisecond;
    g_kbUseTimeReceived = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("UseTimeReceived", 1);
    g_kbAllowNegative = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("AllowNegative", 1);
    g_kbInterpolate = reinterpret_cast<KbGame*>(g_TrackGame)->GetRegistryInt("Interpolate", 1);
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
            aiLineOffset = KbRandUnit() * 14.0f - 7.0f;
        else
            aiLineOffset = KbRandUnit() * 6.0f - 3.0f;
    } else {
        probeSegment = 0;
        field_0x810 = 0;
        aiTricksEnabled = 0;
    }

    // Retail keeps a dead `cmp g_kbGame->eventType, 0` here: two arms that store the
    // same constant, tail-merged by VC6 (the original values were presumably tuned equal).
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 0)
        aiCrashImpactLimit = 100.0f;
    else
        aiCrashImpactLimit = 100.0f;
    lookAheadFrontVelocity = Vec3(0.0f, 0.0f, 0.0f);
    lookAheadRearVelocity = Vec3(0.0f, 0.0f, 0.0f);
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
        SetSpawnProtection(1, 0.0f, 1);
        collisionObject->Disable();
    }
    if (withRider) {
        reinterpret_cast<Character*>(riderCharacter)->ApplyRestPose();
        ((SoultreeObject*)riderCharacter->c_0x1a0)->TranslateIn(0, Vec3(0.0f, 0.0f, 0.0f));
        Vec3 axisY(0.0f, 1.0f, 0.0f);
        Vec3 axisZ(0.0f, 0.0f, 1.0f);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesIn(0, &axisZ, &axisY, 0, 1);
        KbMorphMod* morph = (KbMorphMod*)new(__FILE__, 0x891) MorphBastardModifier(1);
        morph->LoadParameterFile((void*)context, "RiderMorph.mbf", (SoultreeObject*)riderCharacter->c_0x1a0);
        GameObject::AppendChild((GameObject*)morph, -1);
        ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddSecondModifier((GameObject*)morph);
    }
    UpdateRiderPose(0.001f);
    return this;
}

// Slot 97: after the Bike set-up, registers the rider's animation clips (by name) with the
// wrecker object, binds both wheels and attaches the scene node (tier 3 semantics).
void KrustyBike::CreateWrecker()
{
    Bike::CreateWrecker();
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

// Destructor body (0x00491540, reached through the vbase-adjusted scalar deleting
// destructor 0x00497c40): releases two debug-heap buffers (__FILE__ lines 0x8bb, 0x8be).
KrustyBike::~KrustyBike()
{
    if (heapBufferA)
        DebugFree(heapBufferA, __FILE__, 0x8bb);
    if (heapBufferB)
        DebugFree(heapBufferB, __FILE__, 0x8be);
}

void KrustyBike::UpdateLocalCenterOfMass()
{
    Bike::UpdateLocalCenterOfMass();
    if (aiKind && frontWheelLifted)
        frontWheelLifted = 0;
}

int KrustyBike::QueryControlInput(int control, int modifier)
{
    if (!aiKind && !isRemote && !isGhost) {
        KbSensor* chat = race->chatOverlay;
        if (!chat || !chat->typing) {
            Bike::QueryControlInput(control, modifier);
            return inputMap->IsControlDown(control, modifier);
        }
    }
    return 0;
}

// Slot 99 (0x004916a0): tier 3 -- out-of-bounds / off-track recovery.  When slot 12 reports
// the bike outside the track box it aims scratchVector at the track centre (20 units up),
// sets velocity to a 0.055 fraction of the offset, and gives a small random kick to
// angularVelocity; otherwise it may enter recovery states 2 (both wheels loaded past -3.5)
// or 1/3/4/5/9, and falls through to the base handler when nothing applies.
int KrustyBike::EvaluateCrash(float dt, float noseOverLimit, int impact, float impulse)
{
    if (crashState) return crashState;
    if (ClassifyOutOfBounds(anyWheelInContact)) {
        KbTrackB* grid = ((KbTrackA*)track)->trackData;
        scratchVector = Vec3(grid->sizeX * terrainScale * 128.0f, 20.0f,
                             grid->sizeZ * terrainScale * 128.0f);
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
    if (!motionActive && inputMap && inputMap->IsControlDown(8, 0x3f)) {
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
        ResetTrickState();
        return 1;
    }
    if (IsSpawnProtected()) return 0;
    return Bike::EvaluateCrash(dt, noseOverLimit, impact, impulse);
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
// Same bytes (151/230): level/row int locals, a flat
// (&b[0][0])[row * 3 + level - 1] index, swapped products, an else arm. A KbGame*
// local for both arms drops to 22.
// Measured (s14): the else arm gets retail's registers (g in eax, index in edx) when the
// true arm binds `KbGame&` (100/229), uses `i += eventType != 3 ? 3 : 0`, or comes second
// (`if (!aiKind)`, wrong layout); the true arm then still loads +0x60c after the cmp into
// the eventType register, where retail loads it into the g register before the xor/cmp.
// Index spellings (level-first sums, pointer rows, inline accessors, top-level locals)
// all compile to the base bytes.
// Measured (s14f): a true-arm `KbGame*` local gives the else arm retail's registers but
// holds the game in eax with `setne dl` (100/230). Type/level int locals in either
// order, flat sums, `== 3 ? 0 : 1`, `!(== 3)`, `3 !=`, `else` arms and a scale local
// give the base bytes or fewer; a `>` compare, a shared speedGain or limit local and a
// conditional row pointer lose more (123, 59, 26, 22).
// Session 20, base bytes (151): inline accessors for difficulty, eventType, the SX test or
// the level (each alone or together), a row local before the speed gain, a separate level
// local, a six-float flat table, and returning the comparison directly. Worse: a named
// factor (93), `!= 3 ? 3 : 0` added to the flat index (95), a bool row (115).
// Not exact (151 of 230 bytes, one byte long: register allocation only; retail keeps the
// game in eax and the +0x60c index in edx in the else arm, and loads +0x60c before the setne in
// the true arm).
int KrustyBike::IsSpeedGainBelowLimit(float dt)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (aiKind) {
        float speedGain = linearSpeed - prevSpeed;
        if (speedGain * b[reinterpret_cast<KbGame*>(g_TrackGame)->eventType != 3][reinterpret_cast<KbGame*>(g_TrackGame)->difficulty - 1] < dt * speedGainLimit)
            return 1;
        return 0;
    }
    if (linearSpeed - prevSpeed < a[reinterpret_cast<KbGame*>(g_TrackGame)->difficulty - 1] * speedGainLimit * dt)
        return 1;
    return 0;
}

void KrustyBike::ClearCrashState()
{
    UseBikeAndRiderCollision();
    for (int i = 0; i < collisionPointCount; i++) {
        KbChild* point = ((KbChild**)collisionPoints)[i];
        if (point->ownerNode)
            point->inContact = 0;
    }
    crashState = 0;
    crashTimer = 0;
    prevCrashState = 0;
    linkedVehicle = 0;
    wrecker->EndWreck();
    steerTorque = kVec3Zero;
}

int KrustyBike::ApplyPendingRespawn(float dt)
{
    if (!respawnPending && reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && offTrackTime > 6.0f) {
        if (crashState)
            ClearCrashState();
        return 1;
    }
    return Bike::ApplyPendingRespawn(dt);
}

// Slot 86: per-frame wheel-contact update (provisional semantics).  When the race context
// flag (race+0x18a) is set, a lookup ramp derived from backUpPhase (0.12..0.4762,
// scaled by 360 / 0.3562 = 1010.66815f, the float retail holds, so 360 at 0.12) feeds the
// wheel's helper object, and in one game mode the wheel's two force accumulators are
// halved; the wheel is then always updated.
void KrustyBike::UpdateWheelDriveForces()
{
    BikeWheel* wheel = rearWheel;
    if (!wheel->inContact)
        return;
    if (race->isRacing) {
        float ramp;
        if (backingUp && backUpPhase >= 0.12f && backUpPhase < 0.4762f)
            ramp = (0.4762f - backUpPhase) * (360.0f / 0.3562f);
        else
            ramp = 0;
        if (wheel->aux)
            reinterpret_cast<EngineGearbox*>(wheel->aux)->DriveForce(
                KbFloat(linearSpeed), reinterpret_cast<const Vector3*>(&wheel->rollDirection),
                KbFloat(driveForceScale), backingUp, ramp, reinterpret_cast<Vector3*>(&wheel->driveForce),
                &wheel->driveForceMagnitude);
        if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !onTrack) {
            rearWheel->driveForce *= 0.5f;
            rearWheel->driveForceMagnitude *= 0.5f;
        }
    }
    rearWheel->UpdateDriveShare(this);
}

Vec3 KrustyBike::ComputeDriveTorque(const Vec3* arm, const Vec3* force)
{
    if (aiKind)
        return kVec3Zero;
    return Bike::ComputeDriveTorque(arm, force);
}

// 0x004b0ac0 is PointInPlacementSquare (src/reconstructed/PlacementQueries.cpp, which has no
// header; ObjectPlacement.cpp declares it the same way).
int PointInPlacementSquare(const CollisionVec3* point, float x, float z, float size, float scale,
                           float* minX, float* maxX, float* minZ, float* maxZ);

// Slot 11 (0x00491d10): tier 3 -- respawn placement.  Fills outPosition and outForward
// (unit direction) according to the game mode (g->0x2d74: 0..5), then issues the same
// collision query as the base class with per-mode ranges.  The two "copy the stored
// vectors" paths fall off the end without setting a result; the returns below give the
// value retail leaves in eax (a fall-off-the-end spelling loses bytes and keeps other eax).
// Near miss (523/1296 strict, 1300 bytes): flag and lim are assigned in both arms (an
// initialiser plus a conditional store keeps 0 in edi and spills lim), and the mode 2/3
// copy path ends with `break` (retail returns there; it does not fall into case 0) and
// copies position from *outPosition.  Left: the 2/3 normalise places its join after the invsqrt
// arm (retail: after the zero arm, the invsqrt arm jumps back) and its query call is not
// cross-jumped into the shared case 0/4 call; case 0 jumps to a call homed in case 4 in
// retail (here case 4 jumps into case 0's).  `d != 0` first, a Vec3 ternary, `*outForward * inv`
// and an inv local do not help (494-523).
// Measured (s14g): a return per arm of the 2/3 normalise gives retail's arm layout (the zero
// arm falls into the call, the invsqrt arm jumps back to it), but the call's register
// choice then differs from case 4's tail, so it is not cross-jumped (1392 B; 271 aligned
// instructions against 275, 341 strict against 523).
// Every game read goes through the shared global (no `KbGame* g` copy): retail keeps it in
// ebx but reloads it after case 4's `*outForward = bodyForward`, a store that may alias the global
// (aligned 275 -> 309, 526 strict, 1288 bytes).
// Not exact (548 of 1296 bytes, same length; prologue, frame 0x20 and lim in ebp match: the
// mode 2/3 normalise joins after its invsqrt arm and its query call is not cross-jumped into the
// shared call, and the case 0/4 call is homed in case 0 where retail homes it in case 4).
int KrustyBike::FindPlacement(int respawn, Vec3* outPosition, Vec3* outForward, Vec3* outUp, int* outKeepPlacement)
{
    int flag;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->mode.GetSelectedSeries() == 3 || reinterpret_cast<KbGame*>(g_TrackGame)->mode.GetSelectedSeries() == 4)
        flag = 1;
    else
        flag = 0;
    int lim;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->mode.GetSelectedSeries() == 5 || reinterpret_cast<KbGame*>(g_TrackGame)->mode.GetSelectedSeries() == 4)
        lim = 0x7fffffff;
    else
        lim = 0x66;
    unsigned char hit = 0;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->mode.GetSelectedSeries() == 5)
        hit = (unsigned char)PointInPlacementSquare((const CollisionVec3*)&position, 2.0f, 2.0f, 2.0f, terrainScale, 0, 0, 0, 0);
    switch (reinterpret_cast<KbGame*>(g_TrackGame)->eventType) {
    case 2:
    case 3:
        if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode || onTrack || aiKind || isGhost || respawn == 2 ||
            reinterpret_cast<KbGame*>(g_TrackGame)->wreckResetOnTrack) {
            *outPosition = respawnPosition;
            *outForward = respawnHeading;
            position = *outPosition;
            // Retail returns no value here: eax keeps the last word of that copy, the
            // bits of outPosition->z (zero when the stored position has z == +0.0).
            // The reconstructed callers ignore it; tools/emu_cases/kb_find_placement.py.
            return *(int*)&outPosition->z;
        } else {
            Vec3 target = respawnPosition;
            *outPosition = position;
            *outForward = Vec3(target.x - outPosition->x, -outPosition->y, target.z - outPosition->z);
            float lengthSq = outForward->x * outForward->x;
            lengthSq += outForward->y * outForward->y;
            lengthSq += outForward->z * outForward->z;
            if (lengthSq == 0.0f) *outForward = kVec3Zero;
            else *outForward *= FastInvSqrt(lengthSq);
            return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, 0x64,
                               0x65, hit, 3.0f, 8, 0, &respawnPosition, !respawnPending, outPosition, outForward, outUp, outKeepPlacement);
        }
    case 0: {
        const Vec3* heading = respawnPending ? &respawnHeading : &bodyForward;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, 6.0f, 8, heading, 0, !respawnPending, outPosition, outForward, outUp, outKeepPlacement);
    }
    case 4: {
        KbPlayer* tagRacer = (KbPlayer*)reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent;
        const Vec3* target = 0;
        const Vec3* heading = 0;
        if (tagRacer->ball) target = &tagRacer->ball->position;
        else if (tagRacer->followedRacer == this) { *outForward = bodyForward; heading = &bodyForward; }
        else target = &tagRacer->followedRacer->position;
        float radius = 9.0f;
        if (((KbPlayer*)reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent)->followedRacer != this)
            radius = 3.0f;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, radius, 8, heading, target, !respawnPending, outPosition, outForward, outUp,
                           outKeepPlacement);
    }
    case 1:
    case 5: {
        if (respawnPending) scratchVector = respawnPosition + respawnHeading;
        else scratchVector = reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent[lapGate + 9].position;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim,
                           respawnPending ? 0x64 : 0x7fffffff, respawnPending ? 0x65 : 0x7fffffff,
                           3.0f, 8, 0, &scratchVector, !respawnPending, outPosition, outForward, outUp, outKeepPlacement);
    }
    default:
        *outPosition = position;
        *outForward = bodyForward;
    }
    // Retail leaves eax holding outForward (nonzero) on this path.
    return (int)outForward;
}

int KrustyBike::IsThrottleRequested()
{
    if (!race->isRacing && !driveDisengaged)
        return 0;
    if (aiThrottleRequest > 0.0f || (inputDevice && throttleInput > 0.33f) ||
        (QueryControlInput(4, 0x3f) && !IsPovHatPressed(engineState->throttleFraction == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::IsBrakeRequested(VehicleWheel* wheel)
{
    BikeWheel* input = (BikeWheel*)wheel;
    if (aiBrakeRequest > 0.0f ||
        (QueryControlInput(5, 0x3f) && !IsPovHatPressed(input->rampLevel == 0.0f)))
        return 1;
    return 0;
}

int KrustyBike::IsClutchRequested()
{
    if (!aiKind && !isRemote && !isGhost && QueryControlInput(12, 0x3f)) {
        if (!IsPovHatPressed(!driveDisengaged))
            return 1;
    }
    return 0;
}

int KrustyBike::IsReverseRequested()
{
    if (!aiKind && !isRemote && !isGhost) {
        if (forwardSpeed < 6.0f && QueryControlInput(13, 0x3f)) {
            if (!IsPovHatPressed(engineState->throttleFraction == 0.0f))
                return 1;
        }
        backUpPhase = 0;
    }
    return 0;
}

void KrustyBike::ReadControlAxes(float dt)
{
    if (aiKind) {
        UpdateAIRacingLine(dt);
        return;
    }
    aiThrottleRequest = 0;
    aiBrakeRequest = 0;
    Bike::ReadControlAxes(dt);
}

void KrustyBike::UpdateControlInput(float dt)
{
    if (crashState) {
        aiThrottleRequest = 0;
        aiBrakeRequest = 0;
    }
    Bike::UpdateControlInput(dt);
}

void KrustyBike::UpdateAirborneState(int nowAirborne)
{
    if (crashState) {
        trickLandTimer = 0;
    } else if (airborne && !nowAirborne && !landingLatched && motionCompleted && trickLandTimer <= 0.0f) {
        trickLandTimer = 0.3f;
    }
    Bike::UpdateAirborneState(nowAirborne);
}

// 0x004925A0: message 0x13 (request: this bike touched the followed racer or ball) or 0x87 (the new followed racer, also recorded; BikeRace sets tagEvent->followedRacer from it).
void KrustyBike::SendFollowedRacer(Vehicle* racer, bool request)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->network && !isRemote) {
        KbNetPacket pkt;
        pkt.followedRacerId = racer ? ((KbNetBike*)racer)->netId : 0;
        pkt.request = request;
        reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send((unsigned char)(request ? 0x13 : 0x87), &pkt, 12, ((KbNetBike*)this)->netId, 0);
        if (!request && netRecorder && reinterpret_cast<KbGame*>(g_TrackGame)->saveReplay && !reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay)
            reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(0x87, racer ? ((KbNetBike*)racer)->netId : 0, &pkt, 1);
    }
}

// 0x00492670: sends this bike's state as message 1 (to every peer, or into the recorder when
// `record` is set) and keeps what was sent in `state`; without a recorder, peers also get
// message 10 every two seconds in modes 0 and 4.
void KrustyBike::SendFullState(KbBikeState* state, float dt, int record)
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
    message.throttleOn = throttleOn;
    message.driveDisengaged = driveDisengaged;
    message.trickPoseHeld = trickPoseHeld;
    message.crashDirection = crashDirection;
    message.crashed = crashState;
    message.spawnProtected = IsSpawnProtected();
    message.finished = hasFinished != 0;
    message.onTrack = onTrack;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 2 || reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 3)
        message.lapGate = (unsigned char)lapCount;
    else
        message.lapGate = lapGate;
    message.time = ReadClock();
    message.elapsedMs = (short)message.time - (short)state->time;
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
                ping.score = score;
                ping.loaderKind = loaderKind;
                reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send(10, &ping, sizeof(ping), netId, 0);
                pingTimer = 2.0f;
            }
        }
    } else if (netRecorder && race->scoreRecordDue) {
        KbBikePing ping;
        ping.score = score;
        ping.loaderKind = loaderKind;
        reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(10, netId, &ping, sizeof(ping));
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
// test with an empty then-block and an explicit final return were tried.  Retail has three
// physical full-send copies (two early ones with their own epilogues, the end block the
// `je` early returns use); a physical `{ SendFullState(..); return; }` in both branches gives
// VC6's cross-jumping retail's local shape (`jne deltas` and `push 0; jmp home+1`) but it
// keeps the first copy (record branch, inline) as the home and drops the end block
// (2295 B, 78 differing instructions); `goto deltas` + inline calls move the block inline
// after the record branch and the network copy to the end (2337-2348 B).
// Measured (s14): with the network copy passing 0, a record copy passing a constant (`1`)
// leaves one differing instruction (`push 1; jmp end+1` for retail's `jmp end`, 2317 B);
// with `record` the record copy and the end block are identical and VC6 keeps the record
// copy as home, early returns included (2295 B). Equal-value spellings (`record + 0`, casts,
// a copied local, `record != 0 ? record : 1`), per-copy inline helpers, `if (0) goto`,
// dead statements, an explicit end return, `if (!record)` order and `goto deltas` forms
// leave the home there.  So retail's end block was home at the merge; the cause is open.
// One end call taking an `int arg` that each path sets (record, or 0 on the network
// path) is worse: 30-127 differing instructions against 11.
// Shapes that mattered: `KbSub` through an out-pointer keeps `diff` a 12-byte slot (the
// operator form is scalar-replaced and repacks the frame); one `ping` next to `message`
// overlays the dead `d` slots; the angular-velocity and angle limits are 2 pi, not 30;
// `scratchVector[1] += turnRate` and `KbCopy` before `state->angularVelocity.y += turnRate`
// make VC6 load turnRate first (a plain aggregate copy before `.y +=` yields the
// read-modify-write order instead).
// Not exact (192 of 2315 bytes, 2312 long; frame 0x34 exact: the record-interval branch is
// `je end ; jmp deltas` where retail has `jne deltas ; jmp end`, the network branch jumps
// conditionally where retail pushes 0 and jumps, and the register choice follows).
void KrustyBike::SendStateDelta(float dt, int record)
{
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->network && (!record || !netRecorder))
        return;
    KbBikeState* state;
    if (record) {
        state = &recordState;
        state->timer += reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
        if (state->timer > reinterpret_cast<KbGame*>(g_TrackGame)->fullRecordPacketIntervalSec)
            goto sendFull;
    } else {
        state = &netState;
        state->timer += reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
        shortMessageTimer += reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
        if (shortMessageTimer < reinterpret_cast<KbGame*>(g_TrackGame)->shortNetPacketIntervalSec)
            return;
        shortMessageTimer = 0;
        if (state->timer > reinterpret_cast<KbGame*>(g_TrackGame)->fullNetPacketIntervalSec)
            goto sendFull;
    }
    {
        KbBikeDeltaMessage message;
        KbBikePing ping;
        Vec3 carried;
        Vec3 diff;
        message.loaderKind = loaderKind;
        KbSub(&diff, velocity, state->velocity);
        carried = diff + state->velocityError;
        if (KB_DELTA_FITS(carried.x, 30.0f) &&
            KB_DELTA_FITS(carried.y, 30.0f) && KB_DELTA_FITS(carried.z, 30.0f)) {
            message.velocity[0] = (signed char)(carried.x * 4.2666669f);
            state->velocityError.x = carried.x - message.velocity[0] * 0.234375f;
            message.velocity[1] = (signed char)(carried.y * 4.2666669f);
            state->velocityError.y = carried.y - message.velocity[1] * 0.234375f;
            message.velocity[2] = (signed char)(carried.z * 4.2666669f);
            state->velocityError.z = carried.z - message.velocity[2] * 0.234375f;
            KbSub(&diff, position, state->position);
            carried = diff + state->positionError;
            if (KB_DELTA_FITS(carried.x, 30.0f) &&
                KB_DELTA_FITS(carried.y, 30.0f) && KB_DELTA_FITS(carried.z, 30.0f)) {
                message.position[0] = (signed char)(carried.x * 4.2666669f);
                state->positionError.x = carried.x - message.position[0] * 0.234375f;
                message.position[1] = (signed char)(carried.y * 4.2666669f);
                state->positionError.y = carried.y - message.position[1] * 0.234375f;
                message.position[2] = (signed char)(carried.z * 4.2666669f);
                state->positionError.z = carried.z - message.position[2] * 0.234375f;
                scratchVector = angularVelocity;
                scratchVector[1] += turnRate;
                KbSub(&diff, scratchVector, state->angularVelocity);
                carried = diff + state->angularVelocityError;
                if (KB_DELTA_FITS(carried.x, 6.28318548f) &&
                    KB_DELTA_FITS(carried.y, 6.28318548f) && KB_DELTA_FITS(carried.z, 6.28318548f)) {
                    message.angularVelocity[0] = (signed char)(carried.x * 20.371832f);
                    state->angularVelocityError.x = carried.x - message.angularVelocity[0] * 0.049087387f;
                    message.angularVelocity[1] = (signed char)(carried.y * 20.371832f);
                    state->angularVelocityError.y = carried.y - message.angularVelocity[1] * 0.049087387f;
                    message.angularVelocity[2] = (signed char)(carried.z * 20.371832f);
                    state->angularVelocityError.z = carried.z - message.angularVelocity[2] * 0.049087387f;
                    float angleDelta = bodyYaw - state->yaw + state->yawError;
                    if (KB_DELTA_FITS(angleDelta, 6.28318548f)) {
                        message.yaw = (signed char)(angleDelta * 20.371832f);
                        state->yawError = angleDelta - message.yaw * 0.049087387f;
                        angleDelta = bodyRoll - state->roll + state->rollError;
                        if (KB_DELTA_FITS(angleDelta, 6.28318548f)) {
                            message.roll = (signed char)(angleDelta * 20.371832f);
                            state->rollError = angleDelta - message.roll * 0.049087387f;
                            angleDelta = bodyPitch - state->pitch + state->pitchError;
                            if (KB_DELTA_FITS(angleDelta, 6.28318548f)) {
                                message.pitch = (signed char)(angleDelta * 20.371832f);
                                state->pitchError = angleDelta - message.pitch * 0.049087387f;
                                unsigned int now = ReadClock();
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
                                    message.throttleOn = throttleOn;
                                    message.driveDisengaged = driveDisengaged;
                                    message.trickPoseHeld = trickPoseHeld;
                                    message.crashDirection = crashDirection;
                                    message.crashed = crashState;
                                    message.spawnProtected = IsSpawnProtected();
                                    message.finished = hasFinished != 0;
                                    message.onTrack = onTrack;
                                    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 2 || reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 3)
                                        message.lapGate = (unsigned char)lapCount;
                                    else
                                        message.lapGate = lapGate;
                                    if (!record) {
                                        if (reinterpret_cast<KbGame*>(g_TrackGame)->network)
                                            reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send(13, &message, sizeof(message), netId, record);
                                    } else if (netRecorder) {
                                        reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(13, netId, &message, sizeof(message));
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
                                        if (reinterpret_cast<KbGame*>(g_TrackGame)->network &&
                                            (!reinterpret_cast<KbGame*>(g_TrackGame)->eventType || (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4 && reinterpret_cast<KbGame*>(g_TrackGame)->stuntMode))) {
                                            pingTimer -= reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
                                            if (pingTimer < 0.0f) {
                                                ping.score = score;
                                                ping.loaderKind = loaderKind;
                                                reinterpret_cast<NetworkInterface*>(reinterpret_cast<KbGame*>(g_TrackGame)->network)->Send(10, &ping, sizeof(ping), netId, 0);
                                                pingTimer = 2.0f;
                                            }
                                        }
                                    } else if (netRecorder && race->scoreRecordDue) {
                                        ping.score = score;
                                        ping.loaderKind = loaderKind;
                                        reinterpret_cast<VCRInterface*>(netRecorder)->QueueRecord(10, netId, &ping, sizeof(ping));
                                    }
                                    return;
                                }
                            }
                        }
                    }
                }
                SendFullState(state, reinterpret_cast<KbGame*>(g_TrackGame)->frameTime, record);
                return;
            }
            SendFullState(state, reinterpret_cast<KbGame*>(g_TrackGame)->frameTime, record);
            return;
        }
    }
sendFull:
    SendFullState(state, reinterpret_cast<KbGame*>(g_TrackGame)->frameTime, record);
}

// 0x004933e0: the decoded network state (+0x1358..+0x1388) accumulates message 13's
// byte deltas; the result is copied into *state.
void KrustyBike::ApplyStateDelta(const KbNetDelta* delta, KbNetState* state)
{
    Vec3 d0(delta->velocity[0] * 0.234375f, delta->velocity[1] * 0.234375f, delta->velocity[2] * 0.234375f);
    Vec3 d1(delta->position[0] * 0.234375f, delta->position[1] * 0.234375f, delta->position[2] * 0.234375f);
    Vec3 d2(delta->angularVelocity[0] * 0.049087387f, delta->angularVelocity[1] * 0.049087387f, delta->angularVelocity[2] * 0.049087387f);
    deltaBaseVelocity += d0;
    deltaBasePosition += d1;
    deltaBaseAngularVelocity += d2;
    deltaBaseRoll += delta->angles[0] * 0.049087387f;
    deltaBasePitch += delta->angles[1] * 0.049087387f;
    deltaBaseYaw += delta->angles[2] * 0.049087387f;
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
    state->motionFlags = delta->motionFlags;
    state->poseBytes = delta->poseBytes;
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

// The same band for a change against a squared limit; dividing inside the helper (rather
// than passing the quotient) keeps limitSq a stored local ahead of the AllowWarping test.
static inline int KbWithinWarpBandOf(float dist2, float limitSq, float frame, float warp)
{
    return dist2 / limitSq > frame * frame && dist2 / limitSq < warp * warp;
}

// 0x00493660: the network update of a remote bike. Places the bike from the received
// states (states[0] newest): either interpolating between the two states around the
// current time (g_kbInterpolate) or extrapolating the newest one, with optional latency
// hiding, rate limiting and warping of the velocity and position; then applies the pose,
// lap and crash flags of the state and runs the physics bookkeeping of a step.
// `dt` is unused; `a` is set for the live game (lap and race fields are taken over).
//
// Partial (8545 bytes retail, candidate 8558; 2305 strict; aligned 2183 instructions, 77
// differing): the frame (0x58)
// and stack slots match retail (tools/frame_layout.py). What fixed the shape so far: 4-byte
// locals are declared at function scope in retail slot order (ratio, dist2, len, reset)
// ahead of the 12-byte vectors (step, delta, savedPos, localFwd, localUp); an `int k`
// state index (not a `cur` pointer) gives retail's registers through the live-game
// interval code; `float clock` hoists the race clock read above the replay-mode test;
// dividing by limitSq inside KbWithinWarpBandOf stores limitSq before the AllowWarping
// test as retail does; both orientation calls written in each branch (VC6 cross-jumps the
// second, retail's single deferred `add esp, 0x28`), with the angles passed yaw, pitch,
// roll (+0x1c pushed first); `float ff = f * f` keeps VC6 from refactoring
// `f*f*50 + len*f`; the named `scale` stays FPU resident across the three `delta * scale`
// products; the pose clamps follow both modes; KB_ABS on a variable gives the copying
// `jns; neg`; `states[0]->` is re-read after every call and store (no alias); the
// lap/pose flags are 1/0 if-else stores; the crash-flip test reads bodyUp.y; `char m`
// keeps the motion compare byte sized. The pose-param and lean lerps subtract the member
// read (`c - interpSource->poseParam`): VC6 then negates the tested `d` in place and
// recomputes `c - b` (`sub ecx, edi`) as retail does, and the function's constant
// registers follow retail (1 in ebp from the second SetAxesPtr call, 0 in edi up to the
// pose section, `test`/`push 0` in the tail; aligned 2052 -> 2183). With `c - b` in the
// lerps VC6 kept the first difference in a stack slot and allocated 0 in ebp, 1 in ebx.
// Still different: `c` is widened through its stack home (`mov [esp+0x20], bl`) where
// retail widens bl in registers, the 12 bytes of the size gap; dropping the `c` local
// gives retail's widening but returns the old constant allocation (aligned 2090), as do a
// `const unsigned char&` c and reading interpTarget in the lerp too. The poseBlend lerp
// keeps `c - b` (its member-read form recomputes but scores 2182). Before this:
// Writing the rider placement in both orientation-save branches, extra `= 1` stores
// before PlaceWheels, moving `movingForward = 1` or an int `nowAirborne` do not move it; a
// probe third SetAxesPtr call removes the middle zero register and the tail then matches
// (233). Named locals for the 1 (SetAxesPtr arguments, movingForward, the latch stores),
// for the 0 arguments or for g_kbAllowWarping read before the dist2 sum are propagated away
// and change nothing. The choice is function wide: a diagnostic `return` before the closing
// prevVelocity/position/lastStepTime stores moves even the entry zero register to ebx.
// Also: the replay-branch entry registers (g/race in edx/ecx for retail's ecx/edx),
// the Vec3 copy registers from the interpolated velocity on (retail takes edx for the
// destination before the expression, the candidate ecx after it, which rotates eax/ecx/edx
// through the position block; KbCopy, a Vec3 reference, pointer or named temporary and
// either operand order do not move it), the velocity
// AllowWarping load (`cmp mem` for retail's `mov eax`; a local, `!= 0` and an inline test
// do not change it), the velocity y/z `fld s; fmul` order, and retail recomputing `c - b`
// for the pose lerps while the candidate reuses `d`. The 100.0f here is 0x5505ec;
// 0x492670/0x492ad0 scale the message-1 pose bytes by 255.0f (0x551420), which the
// bindings used to give the 100.0f key.
// Not exact (2305 of 8558 bytes, 8558 against retail's 8545; frame 0x58 and slots match, 77
// of 2183 aligned instructions differ: `c` widened through its stack home, the
// `(len + 50) * (len + 50)` store before the AllowWarping test, the velocity y/z load order).
void KrustyBike::InterpolateNetworkState(float dt, int copyRaceProgress)
{
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
    if (reinterpret_cast<KbGame*>(g_TrackGame)->frameTime <= 0.0f)
        return;
    // the newest state that is old enough to be complete (3 in a replay)
    int newest = reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay ? 3 : 2;
    if (states[newest]->position.x != 0.0f && states[newest]->position.y != 0.0f &&
        states[newest]->position.z != 0.0f) {
        if (reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay == 0 && reinterpret_cast<KbGame*>(g_TrackGame)->raceMode != 4) {
            if (lastTimeReceived != states[0]->timeReceived) {
                lastTimeReceived = states[0]->timeReceived;
                if (firstPacketLatch == 0) {
                    firstPacketLatch = 1;
                } else {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                    while (stateIntervalElapsed > stateIntervalLength && stateIntervalLength > 0.0f)
                        stateIntervalElapsed -= stateIntervalLength;
                    if (stateIntervalElapsed < 0.0f)
                        stateIntervalElapsed = 0.0f;
                }
            }
            if (stateIntervalElapsed > stateIntervalLength) {
                if (firstPacketLatch == 1) {
                    firstPacketLatch = 0;
                    while (stateIntervalElapsed > stateIntervalLength && stateIntervalLength > 0.0f)
                        stateIntervalElapsed -= stateIntervalLength;
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    interpTarget = states[0];
                    interpSource = states[1];
                } else {
                    stateIntervalElapsed = stateIntervalLength;
                }
            }
        } else {
            // replay: the race clock, less a fixed lag, picks the state interval
            float clock = this->race->replayTime;
            if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode == 4) {
                float adj;
                stateIntervalElapsed = (clock - 0.17f) - (float)states[2]->time * 0.001f;
                adj = this->race->replayTime - 0.17f;
                if (adj > (float)states[0]->time * 0.001f && this->race->lastRecordKind == -2) {
                    stateIntervalElapsed = stateIntervalLength;
                } else if (adj > (float)states[1]->time * 0.001f) {
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[0];
                    interpSource = states[1];
                } else {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                }
            } else {
                float adj;
                stateIntervalElapsed = (clock - 0.27f) - (float)states[3]->time * 0.001f;
                adj = this->race->replayTime - 0.27f;
                if (adj > (float)states[0]->time * 0.001f && this->race->lastRecordKind == -2) {
                    stateIntervalElapsed = stateIntervalLength;
                } else if (adj < (float)states[2]->time * 0.001f) {
                    stateIntervalLength = (float)(states[2]->time - states[3]->time) * 0.001f;
                    interpTarget = states[2];
                    interpSource = states[3];
                } else if (adj < (float)states[1]->time * 0.001f) {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[2]->time - states[3]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                } else {
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[1]->time - states[3]->time) * 0.001f;
                    if (stateIntervalElapsed > stateIntervalLength)
                        stateIntervalElapsed = stateIntervalLength;
                    interpTarget = states[0];
                    interpSource = states[1];
                }
                if (stateIntervalElapsed < 0.0f)
                    stateIntervalElapsed = 0.0f;
            }
        }
        if (!g_kbInterpolate) {
            unsigned int now = ReadClock();
            if (!g_kbUseTimeReceived)
                stateIntervalElapsed = ((float)now - (float)states[0]->time - clockOffset) * 0.001f;
            else
                stateIntervalElapsed = ((float)now - (float)states[0]->timeReceived) * 0.001f;
            if (!g_kbAllowNegative && stateIntervalElapsed < 0.0f)
                stateIntervalElapsed = 0.0f;
            if (g_kbUseExtrapLimit && stateIntervalElapsed > g_kbExtrapLimit)
                stateIntervalElapsed = g_kbExtrapLimit;
            stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
        }
        if (stateIntervalLength > 0.0f)
            ratio = stateIntervalElapsed / stateIntervalLength;
        else
            ratio = 0.0f;

        // velocity
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold)
                    delta = states[0]->velocity - velocity;
                else
                    delta = (states[0]->velocity - states[1]->velocity) * ratio + states[0]->velocity - velocity;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // warp when the change (in units of the 100/s rate) is below a frame's
                    // worth or beyond the warp threshold; otherwise move at the rate
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 * 0.0001f, reinterpret_cast<KbGame*>(g_TrackGame)->frameTime, g_kbWarpThreshold)) {
                        velocity += delta;
                    } else {
                        step = delta * 100.0f * reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
                        velocity += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    velocity += delta;
                }
            } else {
                velocity = states[0]->velocity;
            }
        } else {
            velocity = (interpTarget->velocity - interpSource->velocity) * ratio + interpSource->velocity;
        }
        smoothedVerticalAccel = (velocity.y - prevVelocity.y) / reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
        prevSpeed = linearSpeed;
        len = KbLength(velocity);
        linearSpeed = len;

        // position
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold)
                    delta = states[0]->position - position;
                else
                    delta = states[0]->velocity * stateIntervalElapsed + states[0]->position - position;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // the position may move at speed + 50 per second
                    float limitSq = (len + 50.0f) * (len + 50.0f);
                    if (g_kbAllowWarping
                        && !KbWithinWarpBandOf(dist2, limitSq, reinterpret_cast<KbGame*>(g_TrackGame)->frameTime, g_kbWarpThreshold)) {
                        position += delta;
                    } else {
                        float ff = reinterpret_cast<KbGame*>(g_TrackGame)->frameTime * reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
                        float scale = ff * 50.0f + len * reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
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
            position = (interpTarget->position - interpSource->position) * ratio + interpSource->position;
        }

        // orientation
        if (!g_kbInterpolate) {
            OrientationAxesFromAngles(&bodyForward, &bodyUp, states[0]->yaw, states[0]->pitch, states[0]->roll);
            OrientationAxesFromAngles(&localFwd, &localUp, states[1]->yaw, states[1]->pitch, states[1]->roll);
        } else {
            OrientationAxesFromAngles(&bodyForward, &bodyUp, interpTarget->yaw, interpTarget->pitch, interpTarget->roll);
            OrientationAxesFromAngles(&localFwd, &localUp, interpSource->yaw, interpSource->pitch, interpSource->roll);
        }
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold) {
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
            angularVelocity = (interpTarget->angularVelocity - interpSource->angularVelocity) * ratio + interpSource->angularVelocity;
        }

        savedPos = position;
        modelNode->SetPosition(position);
        modelNode->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        LoadBodyAxes();
        OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                     &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
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
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        centerNode->GetPositionIn(0, &centerOfMass);
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
        ((KbSink*)frontWheel->primaryAux)->Reset();
        ((KbSink*)rearWheel->secondaryAux)->Reset();
        PlaceWheels();
        allWheelsInContact = wheelsInContact == wheelCount;
        anyWheelInContact = wheelsInContact != 0;
        UpdateWheelsInContact();
        {
            bool nowAirborne = !anyWheelInContact;
            UpdateAirborneState(nowAirborne);
            airborne = nowAirborne;
            if (nowAirborne)
                landingLatched = 0;
        }
        movingForward = 1;

        // wheel roll from the horizontal speed
        if (!(this->race->replayPaused && this->race->lastRecordKind != -2)) {
            float speedSq;
            float roll;
            scratchVector = velocity;
            scratchVector.y = 0.0f;
            speedSq = SquareMagnitude(scratchVector);
            if (speedSq == 1.0f)
                roll = 1.0f;
            else
                roll = FastSqrt(speedSq);
            ((KbWheel*)frontWheel)->SetRollDistance(roll * reinterpret_cast<KbGame*>(g_TrackGame)->frameTime);
            ((KbWheel*)rearWheel)->SetRollDistance(roll * reinterpret_cast<KbGame*>(g_TrackGame)->frameTime);
        }
        if (!g_kbInterpolate)
            turnRate = states[0]->angularVelocity.y;
        else
            turnRate = (interpTarget->angularVelocity.y - interpSource->angularVelocity.y) * ratio + interpSource->angularVelocity.y;

        // pose parameter; the clamp follows both modes
        if (!g_kbInterpolate) {
            int delta = interpTarget->poseParam - interpSource->poseParam;
            if (KB_ABS(delta) >= 0x40) {
                poseParam = (float)states[0]->poseParam * KB_BYTE_SCALE;
            } else {
                float current = (float)states[0]->poseParam * KB_BYTE_SCALE;
                poseParam = current;
                if (!(g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold))
                    poseParam = (current - (float)states[1]->poseParam * KB_BYTE_SCALE) * ratio + current;
            }
        } else {
            int from = interpSource->poseParam;
            unsigned char to = interpTarget->poseParam;
            int delta = to - from;
            if (KB_ABS(delta) >= 0x40 && KB_ABS(delta) <= 0x7c) {
                if (!poseParamLatched) {
                    poseParamLatched = 1;
                    latchedPoseParam = interpSource->poseParam;
                }
                poseParam = (float)latchedPoseParam * KB_BYTE_SCALE;
            } else if (poseParamLatched) {
                if (to <= 3 || (to >= 0x7e && to <= 0x82))
                    poseParamLatched = 0;
                poseParam = (float)latchedPoseParam * KB_BYTE_SCALE;
            } else if (KB_ABS(delta) <= 0x18) {
                poseParam = (float)((int)((float)(to - interpSource->poseParam) * ratio) + from)
                    * KB_BYTE_SCALE;
            } else {
                poseParam = (float)from * KB_BYTE_SCALE;
            }
        }
        if (poseParam > 1.0f)
            poseParam = 1.0f;
        else if (poseParam < 0.0f)
            poseParam = 0.0f;

        // pose lean blend
        if (!g_kbInterpolate) {
            int current = states[0]->poseLeanBlend;
            int delta = current - states[1]->poseLeanBlend;
            float value;
            delta = KB_ABS(delta);
            value = (float)current * KB_BYTE_SCALE;
            if (delta >= 0x40) {
                poseLeanBlend = value;
            } else {
                poseLeanBlend = value;
                if (!(g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold))
                    poseLeanBlend = (value - (float)states[1]->poseLeanBlend * KB_BYTE_SCALE) * ratio + value;
            }
        } else {
            int from = interpSource->poseLeanBlend;
            unsigned char to = interpTarget->poseLeanBlend;
            int delta = to - from;
            if (KB_ABS(delta) >= 0x40 && KB_ABS(delta) <= 0x7c) {
                if (!poseLeanBlendLatched) {
                    poseLeanBlendLatched = 1;
                    latchedPoseParam = interpSource->poseLeanBlend;   // retail stores +0x1408 but reads +0x140c
                }
                poseLeanBlend = (float)latchedPoseLeanBlend * KB_BYTE_SCALE;
            } else if (poseLeanBlendLatched) {
                if (to <= 2 || (to >= 0x7f && to <= 0x81))
                    poseLeanBlendLatched = 0;
                poseLeanBlend = (float)latchedPoseLeanBlend * KB_BYTE_SCALE;
            } else if (KB_ABS(delta) <= 0x20) {
                poseLeanBlend = (float)((int)((float)(to - interpSource->poseLeanBlend) * ratio) + from)
                    * KB_BYTE_SCALE;
            } else {
                poseLeanBlend = (float)from * KB_BYTE_SCALE;
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
            int delta = b0 - b1;
            if (KB_ABS(delta) >= 0x40)
                poseBlend = (float)b1 * KB_BYTE_SCALE;
            else
                poseBlend = (float)b0 * KB_BYTE_SCALE;
        } else {
            int from = interpSource->poseBlend;
            int to = interpTarget->poseBlend;
            int delta = to - from;
            float value;
            float lim;
            float step;
            float rate;
            if (KB_ABS(delta) >= 0x40)
                value = (float)from;
            else
                value = (float)((int)((float)(to - from) * ratio) + from);
            value *= KB_BYTE_SCALE;
            lim = steerAxis->l_0x4;
            step = reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
            if (step >= lim)
                step = lim;
            rate = step / steerAxis->l_0x4;
            steerAxis->l_0x8 = rate;
            value = (value - steerAxis->steerValue) * rate + steerAxis->steerValue;
            steerAxis->steerValue = value;
            poseBlend = value;
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
            lean = poseIndex == RiderPose_CrosOv1 || poseIndex == RiderPose_CrosOv2;
            if (crossoverPose && !lean) {
                EndMotion();
                motionCompleted = 0;
            }
            crossoverPose = lean;
            poseState = states[0]->poseState;
            if (reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent) {
                if (lapGate == states[0]->lapGate - 1 || (lapGate == ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->gateCount - 1 && states[0]->lapGate == 0))
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (copyRaceProgress) {
                    int prev = lapGate;
                    lapGate = states[0]->lapGate;
                    if (prev != lapGate) {
                        if (lapGate == 0)
                            lapCount++;
                        nextLapGate = (lapGate + 1) % ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->gateCount;
                        if (this == this->race->raceCamera->followedRacer)
                            ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->SelectLapGates(lapGate, nextLapGate, this);
                    }
                }
            } else {
                if (!states[0]->fullState)
                    states[0]->lapCount = (short)((lapCount & 0xff00) | states[0]->lapGate);
                if ((unsigned short)lapCount == (unsigned short)states[0]->lapCount - 1)
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (copyRaceProgress)
                    lapCount = states[0]->lapCount;
            }
            if (states[0]->fullState) {
                lastLapTime = states[0]->lastLapTime;
                fastestLap = states[0]->fastestLap;
                if (copyRaceProgress) {
                    timeBehindLeader = states[0]->timeBehindLeader;
                    finishPosition = states[0]->finishPosition;
                }
            }
            if (!isGhost) {
                SetSpawnProtection(states[0]->spawnProtected, 5.0f, 0);
                if (spawnProtectEndPending) {
                    ApplySpawnCollisionIgnores(0);
                    spawnProtectEndPending = 0;
                }
            }
            if (states[0]->onTrack)
                onTrack = 1;
            else
                onTrack = 0;
            prevCrash = crashState;
            crashState = states[0]->crashed;
            throttleOn = states[0]->throttleOn;
            driveDisengaged = states[0]->driveDisengaged;
            trickPoseHeld = states[0]->trickPoseHeld;
            if (crashState && !prevCrash) {
                crashHeading = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                crashedUpsideDown = bodyUp.y < 0.0f;
                crashYaw = savedYaw;
                if (motionActive)
                    wrecker->StartTumble();
                else
                    wrecker->StartAnimatedWreck(states[0]->crashDirection);
                motionIndex = 0;
                EndMotion();
                motionCompleted = 0;
                UseBikeCollision();
            } else if (!crashState && prevCrash) {
                wrecker->EndWreck();
                UseBikeAndRiderCollision();
                motionActive = 0;
                motionIndex = 0;
                reset = 1;
            } else if (CheckMotionComplete()) {
                motionActive = 0;
                motionIndex = 0;
            } else if (!motionActive) {
                char motion = states[0]->motion;
                if (motion) {
                    if (motion < 0x11)
                        StartTrick(motion - 1);
                    else if (motion == 0x11)
                        LookBackAtNearestRival();
                }
            } else if (!trickChained) {
                char motion = interpSource->motion;
                if (motion > 0x11 && motion - 0x11 == motionIndex)
                    StartAltTrick(motionIndex);
            }
        } else {
            int lean;
            int prevCrash;
            poseIndex = interpSource->poseIndex;
            lean = poseIndex == RiderPose_CrosOv1 || poseIndex == RiderPose_CrosOv2;
            if (crossoverPose && !lean) {
                EndMotion();
                motionCompleted = 0;
            }
            crossoverPose = lean;
            poseState = interpSource->poseState;
            if (reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent) {
                if (lapGate == interpSource->lapGate - 1 || (lapGate == ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->gateCount - 1 && interpSource->lapGate == 0))
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (copyRaceProgress) {
                    int prev = lapGate;
                    lapGate = interpSource->lapGate;
                    if (prev != lapGate) {
                        if (lapGate == 0)
                            lapCount++;
                        nextLapGate = (lapGate + 1) % ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->gateCount;
                        if (this == this->race->raceCamera->followedRacer)
                            ((KbTrack*)reinterpret_cast<KbGame*>(g_TrackGame)->bajaEnduroEvent)->SelectLapGates(lapGate, nextLapGate, this);
                    }
                }
            } else {
                if (!interpSource->fullState)
                    interpSource->lapCount = (short)((lapCount & 0xff00) | interpSource->lapGate);
                if ((unsigned short)lapCount == (unsigned short)interpSource->lapCount - 1)
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (copyRaceProgress)
                    lapCount = interpSource->lapCount;
            }
            if (interpSource->fullState) {
                lastLapTime = interpSource->lastLapTime;
                fastestLap = interpSource->fastestLap;
                if (copyRaceProgress) {
                    timeBehindLeader = interpSource->timeBehindLeader;
                    finishPosition = interpSource->finishPosition;
                }
            }
            if (!isGhost) {
                SetSpawnProtection(interpSource->spawnProtected, 5.0f, 0);
                if (spawnProtectEndPending) {
                    ApplySpawnCollisionIgnores(0);
                    spawnProtectEndPending = 0;
                }
            }
            if (interpSource->onTrack)
                onTrack = 1;
            else
                onTrack = 0;
            prevCrash = crashState;
            crashState = interpSource->crashed;
            if (!this->race->replayPaused && this->race->lastRecordKind != -2)
                throttleOn = interpSource->throttleOn;
            else
                throttleOn = 0;
            driveDisengaged = interpSource->driveDisengaged;
            trickPoseHeld = interpSource->trickPoseHeld;
            if (crashState && !prevCrash) {
                crashHeading = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                crashedUpsideDown = bodyUp.y < 0.0f;
                crashYaw = savedYaw;
                if (motionActive)
                    wrecker->StartTumble();
                else
                    wrecker->StartAnimatedWreck(interpSource->crashDirection);
                motionIndex = 0;
                EndMotion();
                motionCompleted = 0;
                UseBikeCollision();
            } else if (!crashState && prevCrash) {
                wrecker->EndWreck();
                UseBikeAndRiderCollision();
                motionActive = 0;
                motionIndex = 0;
                reset = 1;
            } else if (CheckMotionComplete()) {
                motionActive = 0;
                motionIndex = 0;
            } else if (!motionActive) {
                char motion = interpSource->motion;
                if (motion) {
                    if (motion < 0x11)
                        StartTrick(motion - 1);
                    else if (motion == 0x11)
                        LookBackAtNearestRival();
                }
            } else if (!trickChained) {
                char motion = interpSource->motion;
                if (motion > 0x11 && motion - 0x11 == motionIndex)
                    StartAltTrick(motionIndex);
            }
        }

        ClearContactEffectFlags();
        justReset = reset;
        attachmentResetPending = reset;
        UpdateAttachmentEffects();
        prevCrashState = crashState;
        UpdateRiderPose(reinterpret_cast<KbGame*>(g_TrackGame)->frameTime);
    } else {
        savedPos = position;
    }

    if (g_kbInterpolate)
        stateIntervalElapsed = reinterpret_cast<KbGame*>(g_TrackGame)->frameTime + stateIntervalElapsed;
    if (!copyRaceProgress && !hasFinished) {
        if (stateIntervalElapsed >= g_kbStallThreshold) {
            stallHoldTimer = g_kbStallHoldSec;
            SetSpawnProtection(1, 0.0f, 0);
        } else if (stallHoldTimer > 0.0f) {
            stallHoldTimer -= reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
            if (stallHoldTimer < 0.0f)
                stallHoldTimer = 0.0f;
        } else if (!isGhost) {
            SetSpawnProtection(0, 0.0f, 0);
        }
    }
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay) {
        collisionObject->UpdatePlacement();
        if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4)
            ResolveSubstepCollisions(1);
    }
    prevVelocity = velocity;
    position = savedPos;
    lastStepTime = reinterpret_cast<KbGame*>(g_TrackGame)->frameTime;
}

float KrustyBike::GetSpawnProtectionDuration()
{
    KbRacer* tagRacer = reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent;
    if (tagRacer && tagRacer->tagFollowedRacer == this)
        return 0.0f;
    return 3.0f;
}

// Slot 52 (CanCrash, tier 2): gates UpdateCrashState in the collision callbacks; a
// remote (network) bike never crashes locally.
int KrustyBike::CanCrash()
{
    if (crashState || isRemote)
        return 0;
    return 1;
}

int KrustyBike::CheckMotionComplete()
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

int KrustyBike::ShouldRestartSteamEffect()
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

int KrustyBike::ShouldBurstSteamEffect()
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

void KrustyBike::ResetCrashTimer()
{
    if (crashDirection == 2) {
        crashTimer = airCrashTimerReload;
        return;
    }
    float reload = crashTimerReload;
    crashTimer = reload;
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !reinterpret_cast<KbGame*>(g_TrackGame)->eventType)
        crashTimer = reload + 1.0f;
}

int KrustyBike::GetRespawnKey(int* out)
{
    *out = 15;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode && !hasFinished)
        return 0;
    return 1;
}

int KrustyBike::IsRiderPoseLocked()
{
    if (crashState || motionActive)
        return 1;
    return hasFinished != 0 && hasFinished < 3;
}

// Slot 96: control responses derived from the Options sliders (TrackGame 0xfe0..0xff8).
void KrustyBike::InitControlResponses()
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

// 0x00495C00: true when the two newest received states (+0x11c8/+0x11cc), both with a
// nonzero position x and z, differ in their integer x or z (tier 3 semantics).
int KrustyBike::ReceivedPositionChanged()
{
    if (isRemote) {
        KbBikeNetState* newest = states[0];
        if (newest->position.x != 0 && newest->position.z != 0) {
            KbBikeNetState* previous = states[1];
            if (previous->position.x != 0 && previous->position.z != 0) {
                if ((int)newest->position.x != (int)previous->position.x
                    || (int)newest->position.z != (int)previous->position.z)
                    return 1;
            }
        }
    }
    return 0;
}

void KrustyBike::RemoveFromRace()
{
    Bike::RemoveFromRace();
}

// Slot 15: worldTorque into the body frame, scaled per axis as in slot 16, then 0x4CB6E0
// (which looks like a clamp/settle step on angularVelocity) with angularAccel.
void KrustyBike::ApplyWorldTorque(const Vec3* worldTorque, Vec3* angularAccel)
{
    Vec3 localAccel;
    Vec3 localTorque = ((KbXform*)modelNode)->WorldToLocalDirection(worldTorque);
    if (!crashState) {
        localAccel.x = localTorque.x * invInertia.x;
        localAccel.y = localTorque.y * invInertia.y;
        localAccel.z = localTorque.z * invInertia.z;
    } else {
        localAccel.x = localTorque.x * shapeInvInertia.x;
        localAccel.y = localTorque.y * shapeInvInertia.y;
        localAccel.z = localTorque.z * shapeInvInertia.z;
    }
    UnknownAxisSettle_4cb6e0(&angularVelocity.x, &angularAccel->x, &localAccel.x, stepTime, 0);
}

// Slot 16: localTorque scaled per axis by invInertia, or by shapeInvInertia while crashed.
Vec3 KrustyBike::LocalTorqueToAngularAcceleration(const Vec3* localTorque)
{
    Vec3 accel;
    if (!crashState) {
        accel.x = invInertia.x * localTorque->x;
        accel.y = invInertia.y * localTorque->y;
        accel.z = invInertia.z * localTorque->z;
    } else {
        accel.x = shapeInvInertia.x * localTorque->x;
        accel.y = shapeInvInertia.y * localTorque->y;
        accel.z = shapeInvInertia.z * localTorque->z;
    }
    return accel;
}

int KrustyBike::ShouldUpdateShadowLight()
{
    if (staggerCounter == staggerPhase || race->raceCamera->followedRacer == this)
        return 1;
    return 0;
}

int KrustyBike::ShouldEmitMovingEffects()
{
    if (!airborne && forwardSpeed >= 0.2f && race->raceCamera->cameraState != 3)
        return 1;
    return 0;
}

// Slot 29: refresh the previous/current orientation angles (0x48..0x60 from 0x2c..0x44).
// Normally they are recomputed from the node's world axes; while neither crossoverPose nor
// motionActive is set the old values are just copied.  Then the bike's matrix is mirrored
// onto the 0x5c4 node when the wrecker's mode is 0 or 1, and the 0x141c timer is advanced.
void KrustyBike::SaveBodyOrientation(int lastStep)
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
            score += AwardTrickScore();
            return;
        }
        trickLandTimer -= stepTime;
    }
}

// 0x00495ff0: ends a trick. trickPoints (+0x1530) drops its part below 100, the
// score is points x trickMultiplier (+0x1538, +0.5 when trickChained); the race
// handler hears of it. In game mode 0 (or 4 with +0x2eb8 for the +0x568 racer's
// bike) the score is kept in +0x788 and returned; otherwise the bonus rules
// (+0x3444) turn it into a capped bonus, shown to the +0x50 racer's bike.
// The redundant parentheses around the (float) fmod term are significant: VC6 keeps them
// as an IL node, which schedules retail's `mov al, [+0x153f]` before the fsubr
// (docs/VC6_OPERAND_ORDER.md section 3); without them the load follows it.
float KrustyBike::AwardTrickScore()
{
    trickPoints -= ((float)fmod(trickPoints, 100.0));
    if (trickChained)
        trickMultiplier += 0.5f;
    float score = trickPoints * trickMultiplier;
    if (race->localRacer == this && race->handler)
        race->handler->ReportTrickScore(this, score);
    if (!reinterpret_cast<KbGame*>(g_TrackGame)->eventType ||
        (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4 && reinterpret_cast<KbGame*>(g_TrackGame)->stuntMode && reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent->tagFollowedRacer == this)) {
        if (race->localRacer == this) {
            if (race->scoreBoard)
                race->scoreBoard->ShowValue(trickPoints, trickMultiplier);
            int points = (int)score;
            if (largestStuntPoints > points)
                points = largestStuntPoints;
            largestStuntPoints = points;
        }
        ResetTrickState();
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
                TextQueueMessage* message = new(__FILE__, 0xa14) TextQueueMessage(line, 3.25f);
                if (message) {
                    sink->ReplaceHeadMessage(message);
                    delete message;
                }
            } else {
                g_TrackGame->LoadResourceString(0x14d9, text, 0x80);
                sprintf(line, "%s %.0f.00)", text, points * rules->limitScale);
                TextQueueMessage* message = new(__FILE__, 0xa1d) TextQueueMessage(line, 3.25f);
                if (message) {
                    sink->ReplaceHeadMessage(message);
                    delete message;
                }
            }
        }
    }
    ResetTrickState();
    return 0.0f;
}

// Slot 93: when the joystick (input map +0x0c) is of kind 3, recreates constant-force effects
// 1 (magnitude 10000, infinite) and 0 (scaled by FeedbackSlider, 70 s), then derives
// forceFeedbackGain from the same FeedbackSlider value.
void KrustyBike::InitForceFeedback()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->deviceKind == 3) {
        pad->CreateConstantForce(1, 10000, -1);
        pad = (KbPad*)inputMap->valueSource;
        pad->CreateConstantForce(0, (int)(reinterpret_cast<KbGame*>(g_TrackGame)->feedbackSlider * 83.333336f), 70000);
        forceFeedbackGain = reinterpret_cast<KbGame*>(g_TrackGame)->feedbackSlider * 0.04f;
    }
}

// Slot 94: force-feedback update; effect 1's magnitude is smoothedForwardAccel *
// forceFeedbackGain * 180 clamped to [-10000, 10000] while not in state 1, and the effect is
// started or stopped on slot 80's result; effect 0 restarts on landing.
void KrustyBike::UpdateForceFeedback()
{
    KbPad* pad = (KbPad*)inputMap->valueSource;
    if (pad && pad->deviceKind == 3) {
        if (crashState != 1) {
            KbEffectParams params = { 0x4650, 0 };
            int force = (int)(smoothedForwardAccel * forceFeedbackGain * 180.0f);
            if (force <= -10000)
                force = -10000;
            else if (force >= 10000)
                force = 10000;
            pad = (KbPad*)inputMap->valueSource;
            pad->SetConstantForce(1, &params, force);
            if (IsThrottleRequested())
                ((KbPad*)inputMap->valueSource)->StartEffect(1, 1, 0);
            else
                ((KbPad*)inputMap->valueSource)->StopEffect(1);
        }
        if (airborne) {
            g_kbPadActive = 1;
            return;
        }
        if (g_kbPadActive) {
            ((KbPad*)inputMap->valueSource)->StartEffect(0, 1, 0);
            g_kbPadActive = 0;
        }
    }
}

// Slot 48: position update from the (velocity*3 - prevVelocity) * 0.5 * dt step.
void KrustyBike::AdvancePosition()
{
    if (race->isRacing) {
        Bike::AdvancePosition();
        return;
    }
    Vec3 tripled = velocity * 3.0f;
    Vec3 stepVelocity = (tripled - prevVelocity) * 0.5f;
    position += stepVelocity * stepTime;
    rearWheel->driveForce = kVec3Zero;
    rearWheel->driveForceMagnitude = 0;
}

// 0x004965e0 (slot 14, exact): outside a race, the base integration with the force's x and
// z cleared first.  Written as `invMass * *force`:
// VC6 forwards the two zero stores into the scaled vector and folds k * 0 into one pooled
// 0.0f on the FPU stack (fld/fst shared by x and z), as retail does; the explicit
// Vec3(0.0f, invMass * force->y, 0.0f) stores immediates instead.
void KrustyBike::IntegrateMotion(Vec3* force, const Vec3* localTorque, const Vec3* worldTorque)
{
    if (race->isRacing) {
        Bike::IntegrateMotion(force, localTorque, worldTorque);
        return;
    }
    angularAcceleration = LocalTorqueToAngularAcceleration(localTorque);
    ApplyWorldTorque(worldTorque, &angularAcceleration);
    angularVelocity += stepTime * angularAcceleration;
    angularVelocity *= 0.999f;
    force->x = 0;
    force->z = 0;
    acceleration = invMass * *force;
    velocity += stepTime * acceleration;
    linearSpeed = KbLength(velocity);
}

// Slot 49: per-frame update.  Keeps the 0x154c "pinned" copy of the placement in sync, posts
// on-screen messages to the local player (string ids 0x1481.. / 0x14d6 / 0x14d7 / 0x14e1; tier 3
// semantics), advances the 0x7a4 state, and runs the Bike update unless the race context
// takes over.
void KrustyBike::SimulateFrame(float dt)
{
    if (drawnAtLastPosition) {
        modelNode->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
    }
    if (race->localRacer == this && reinterpret_cast<KbGame*>(g_TrackGame)->raceMode == 4) {
        TextQueueOverlay* sink = reinterpret_cast<EventManager*>(reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindTextQueue();
        TextQueueMessage* msg = 0;
        if (sink) {
            char text[0x100];
            if (totalRunningTime == 0) {
                if (race->startBeforeFinish)
                    g_TrackGame->LoadResourceString(0x14d6, text, 0x80);
                else
                    g_TrackGame->LoadResourceString(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11c6) TextQueueMessage(text, 3.25f);
                if (msg) {
                    sink->ReplaceHeadMessage(msg);
                    delete msg;
                }
            } else if (lapCount != prevLapCount && lapCount == 1 && race->startBeforeFinish) {
                g_TrackGame->LoadResourceString(0x14d7, text, 0x80);
                msg = new(__FILE__, 0x11d0) TextQueueMessage(text, 3.25f);
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
                TextQueueMessage* msg = 0;
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
                    msg = new(__FILE__, 0x1206) TextQueueMessage(text, 3.25f);
                    if (msg) {
                        sink->ReplaceHeadMessage(msg);
                        delete msg;
                    }
                } else {
                    g_TrackGame->LoadResourceString(0x14e1, text, 0x80);
                    msg = new(__FILE__, 0x120d) TextQueueMessage(text, 3.25f);
                    if (msg) {
                        sink->ReplaceHeadMessage(msg);
                        delete msg;
                    }
                }
            }
            if (finishPosition == 1) {
                if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType) {
                    EndMotion();
                    motionCompleted = 0;
                    reinterpret_cast<Character*>(riderCharacter)->SetMotion(reinterpret_cast<Motion*>(riderPoseHandles[RiderPose_Victory]));
                    ((KbA5C4*)riderCharacter)->motionTime = 0;
                    hasFinished = 3;
                } else {
                    hasFinished = 3;
                    EndMotion();
                    motionCompleted = 0;
                }
            } else {
                hasFinished = 3;
                EndMotion();
                motionCompleted = 0;
            }
        }
    }
    if (!crashState && (aiKind || race->forceAiStep)) {
        if (race->isRacing)
            SimulateAIFrame(dt);
    } else {
        Bike::SimulateFrame(dt);
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

// 0x00496d10 (exact): slot 101 forwards to the body below; VC6 compiles the call as a tail
// `jmp 0x496d20`, followed by the usual nop alignment padding.
void KrustyBike::SwitchToBikeCollision()
{
    UseBikeCollision();
}

// owner: bracket only (0x496d20 sits between slot 101 and UseBikeAndRiderCollision)
// 0x00496d20 (exact): the real body of slot 101, switching collisionObject from
// bikeAndRiderCollision to bikeCollision.  The mode test is a nested if with the 0x2d84 test written
// positively: VC6 then computes the GraphicsTest base (+0xc) in the second test block, calls through ecx
// and cross-jumps the outer else into the inner slot-5 tail as retail does (the flat
// `x18 > 1 && !x2d84` form and the nested `!x2d84` form do not).
void KrustyBike::UseBikeCollision()
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

// owner: bracket only (0x496da0, directly after UseBikeCollision)
// 0x00496da0 (exact): back-camera swap, UseBikeCollision with the two scene objects exchanged.
void KrustyBike::UseBikeAndRiderCollision()
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

void KrustyBike::SetNetRecorder(KbRecorder* recorder)
{
    netRecorder = recorder;
}

// 0x00496E30 (spawn protection on/off): `protect` makes this bike's collision objects ignore
// every other bike (0x00496F90) and stops the part collision objects ignoring its main body;
// without it, the reverse (0x00497370), and the part collision objects ignore both of its
// scene bodies.
void KrustyBike::ApplySpawnCollisionIgnores(int protect)
{
    int cursor = 0;
    KrustyBike* other;
    if (protect) {
        for (other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor)); other; other = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor))) {
            if (other != this)
                IgnoreCollisionsWith(other);
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
                RestoreCollisionsWith(other);
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

// 0x00496F90: both bikes' collision objects ignore each other (AddIgnoredOwner = 0x00439410).
void KrustyBike::IgnoreCollisionsWith(KrustyBike* other)
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
void KrustyBike::RestoreCollisionsWith(KrustyBike* other)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent) {
        void* current = reinterpret_cast<KbGame*>(g_TrackGame)->tagEvent->tagFollowedRacer;
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
    if (other->IsSpawnProtected())
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

// Slot 50: switch spawn protection (and the ghost modifier) on or off.  The base slot only
// runs for the forced case; otherwise the state is latched here with duration stored as
// spawnProtectTimer.
void KrustyBike::SetSpawnProtection(int protect, float duration, int forced)
{
    if (race->networkStartPending)
        return;
    if (forced) {
        if (protect) {
            if (!spawnProtected) {
                if (isGhost) {
                    ((D3DIMSoultreeObject*)modelNode)->AddModifier((GameObject*)ghostMod);
                    ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddModifier((GameObject*)ghostMod);
                    ((GhostMod1*)ghostMod)->StartFade(0);
                }
                ApplySpawnCollisionIgnores(1);
            }
            spawnProtectForced = 1;
            spawnProtected = 1;
            spawnProtectTimer = 0;
        } else {
            spawnProtectForced = 0;
            if (spawnProtected)
                spawnProtectEndPending = 1;
        }
        Vehicle::SetSpawnProtection(protect, 0, 0);
        return;
    }
    if (spawnProtectForced)
        return;
    if (protect) {
        if (!spawnProtected) {
            if (isGhost) {
                ((D3DIMSoultreeObject*)modelNode)->AddModifier((GameObject*)ghostMod);
                ((D3DIMSoultreeObject*)riderCharacter->c_0x1a0)->AddModifier((GameObject*)ghostMod);
                ((GhostMod1*)ghostMod)->StartFade(duration);
            }
            ApplySpawnCollisionIgnores(1);
        }
        spawnProtected = 1;
        spawnProtectTimer = duration;
    } else if (spawnProtected) {
        spawnProtectEndPending = 1;
    }
}

// 0x004977a0 (GameObject slot 10, reached through the virtual base): the per-frame update.
// Picks the update path by the network state and game mode, re-targets the mode 4 player
// record, ends the start-up ghosting once no rival overlaps this bike, and prints the camera
// bike's figures (with a 0..88 "TimeTo60" stopwatch) on the debug overlay.
int KrustyBike::Tick(float dt)
{
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount == 1 && !race->isRacing)
        return 1;
    if (reinterpret_cast<KbGame*>(g_TrackGame)->playingReplay) {
        if (race->isRacing) {
            InterpolateNetworkState(dt, 1);
            if (race->replayMode == 4 && race->appliedReplayMode == 4)
                GameObject::Tick(dt);
        }
    } else if (isRemote) {
        InterpolateNetworkState(dt, 0);
        if (race->recorderTickDue)
            SendStateDelta(dt, 1);
        GameObject::Tick(dt);
    } else if (reinterpret_cast<KbGame*>(g_TrackGame)->raceMode == 4) {
        if (isGhost) {
            InterpolateNetworkState(dt, 1);
            if (ReceivedPositionChanged())
                GameObject::Tick(dt);
        } else {
            Vehicle::Tick(dt);
            if (race->recorderTickDue)
                SendStateDelta(dt, 1);
        }
    } else {
        Vehicle::Tick(dt);
        if (race->recorderTickDue)
            SendStateDelta(dt, 1);
        if (reinterpret_cast<KbGame*>(g_TrackGame)->playerCount > 1)
            SendStateDelta(dt, 0);
    }
    if (reinterpret_cast<KbGame*>(g_TrackGame)->eventType == 4) {
        KbPlayer* raceMode = reinterpret_cast<KbPlayer*>(reinterpret_cast<EventManager*>(
            reinterpret_cast<KbGame*>(g_TrackGame)->eventManager)->FindRaceMode());
        if (raceMode->ball && raceMode->followedRacer == this && crashState) {
            if (reinterpret_cast<KbGame*>(g_TrackGame)->network->isHost)
                raceMode->SetFollowedRacer(0, this, 1);
            else
                SendFollowedRacer(0, 1);
        }
    }
    if (spawnProtectEndPending && !isRemote && race->isRacing) {
        int cursor = 0;
        int overlap = 0;
        KrustyBike* bike;
        for (bike = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor)); bike; bike = reinterpret_cast<KrustyBike*>(reinterpret_cast<BikeRace*>(race)->NextBike(&cursor))) {
            if (!bike->outOfRace && !bike->IsSpawnProtected() && bike != this) {
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
            ApplySpawnCollisionIgnores(0);
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
