// Group6_KrustyBikeCtor.cpp -- best-guess KrustyBike::KrustyBike(int) (0x0048fa60, ret 8 =
// flags + the hidden most-derived flag), body taken unchanged from
// samples/physics/krustybike/KrustyBikeNearMisses.cpp (86.36%, every store matches; one
// scheduling pair differs; not byte-matched).  The canonical
// src/krusty2/vehicle/KrustyBike.cpp does not define the constructor yet.
//
// BikeRace::CreateRacers (Group6_BikeRaceCreateRacers.cpp) `new`s its racers through
// UnknownBikeRaceNewRacer below: BikeRace.h's UnknownBikeRaceRacer is the provisional
// view of KrustyBike's primary pointer (docs/BIKE_RACE_STATE_CONNECTIONS.md: the
// 0x1638-byte allocation is passed unchanged as ECX to 0x0048fa60 and the result is
// stored as the racer pointer), so constructing the canonical class and handing back
// the same pointer is retail's `new UnknownBikeRaceRacer(1)`.
// Headers are included as Group3_KrustyBike.cpp includes them (KrustyBike.cpp's own set).
#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/TrackGameGlobal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle/KrustyBike.h"
#include "../reconstructed/GearRatios.h"
#include "../reconstructed/Net.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "../reconstructed/Recorder.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/DebugOverlay.h"
#include "../reconstructed/TrackGame.h"
#include "math/FastMath.h"
#include "collision/CollisionObject.h"
#include "../reconstructed/Wrecker.h"
#include "broadphase/Quadtree.h"
#include "core/MemTag.h"
#include "soultree/SoultreePhysicsObject.h"
#include "../reconstructed/GhostModCore.h"
#include "../reconstructed/MorphBastardModifierCore.h"
#include "../reconstructed/DebugAlloc.h"

// best guess: sample body (samples/physics/krustybike/KrustyBikeNearMisses.cpp), unreviewed.
KrustyBike::KrustyBike(int flags) : GameObject(1), Bike(flags)
{
    plateNumber = 0x65;
    lastFramePosition = kVec3Zero;
    hasFullState = 0;
    wrecker = 0;
    aiKind = 0;
    isRemote = 0;
    score = 0;
    pingTimer = 0;
    race = 0;
    lapGate = 0;
    forceGatePass = 0;
    nextLapGate = 1;
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
    field_0x15d4 = 0;
    lastCollisionType = 0;
    bikeCollision = 0;
    bikeAndRiderCollision = 0;
}

class UnknownBikeRaceRacer;

// best guess: retail's `new(__FILE__, line) UnknownBikeRaceRacer(1)` in bikerace.cpp's
// loader (0x0048fa60 on a 0x1638-byte DebugAlloc block); `file`/`line` are the
// caller's, so the allocation is tagged as retail tags it.
UnknownBikeRaceRacer* UnknownBikeRaceNewRacer(const char* file, int line)
{
    return reinterpret_cast<UnknownBikeRaceRacer*>(new(file, line) KrustyBike(1));
}
