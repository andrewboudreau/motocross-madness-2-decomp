// Group6_KrustyBikeCtor.cpp -- the racer allocation helper.  KrustyBike::KrustyBike(int)
// (0x0048fa60) itself is strict exact in the canonical src/krusty2/vehicle/KrustyBike.cpp.
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

class UnknownBikeRaceRacer;

// best guess: retail's `new(__FILE__, line) UnknownBikeRaceRacer(1)` in bikerace.cpp's
// loader (0x0048fa60 on a 0x1638-byte DebugAlloc block); `file`/`line` are the
// caller's, so the allocation is tagged as retail tags it.
UnknownBikeRaceRacer* UnknownBikeRaceNewRacer(const char* file, int line)
{
    return reinterpret_cast<UnknownBikeRaceRacer*>(new(file, line) KrustyBike(1));
}
