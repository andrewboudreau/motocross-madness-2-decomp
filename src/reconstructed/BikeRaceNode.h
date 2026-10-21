#pragma once

#include "Track.h"

// A 0x3c-byte gate: a box at +0x00 facing +0x0c with half extents +0x18,
// placed on the track at +0x2c. The course gates are DebugCalloc'd by
// 0x0041d0d0 (bikerace.cpp line 0x840) and chained through +0x38 from
// BikeRace+0xc8; the start and finish probes (BikeRace+0xcc and +0x108) are
// the same structure (BikeRace's constructor clears their +0x38). gameui.cpp
// 0x0047b670 tests crossings of either kind (RaceStatus.cpp), and slot 14
// draws the probes' boxes; 0x0041d1e0 measures the track distance between
// the probes' positions.
struct UnknownBikeRaceNode {
    Vector3 center;                           // +0x00 centre
    Vector3 direction;                        // +0x0c direction
    Vector3 halfExtents;                      // +0x18 half extents (across the direction for gates)
    float width;  // +0x24  AppendNode's scale; halfExtents = scale*dir*0.5; Create scales the finish arch by it / modelWidth
    int field_0x28;
    TrackPos trackPos;                        // +0x2c position along the track (DistanceAlong/PlaceNear operand, 0x004e6a50)
    UnknownBikeRaceNode* nextGate;            // +0x38 next gate
};
