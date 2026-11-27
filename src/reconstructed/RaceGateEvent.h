#pragma once

#include "MatrixUtil.h"

// The gate records of the Baja/Enduro event (TrackGame+0x560, event types 1
// and 5; tier 3 names). BikeRace::Create (0x00417ed0) passes the event that
// creates the race (BaseQuarryEvent::LoadEventObjects, 0x004de590, pushes
// `this`) to BuildNodeList (0x0041d170), which appends one RaceGate per gate,
// and RadarOverlay (0x0051a560) draws the same records. Only an event with
// gates fills +0xac/+0xd8/+0x420: NationalRace's +0xac is its last member and
// BaseQuarryEvent ends at +0xa4, and the demo never allocates the gate event.
// TrackGame.h's TrackGameViewOwner views the same owner; these are views, not
// the RaceGate boxes themselves.

// The 0x18-byte gate placement records 0x0041d0d0 copies.
struct RaceGatePose {
    Vector3 center;  // +0x0  AppendNode copies it into the node's center
    Vector3 direction;  // +0xC  AppendNode copies it into the node's direction
};

// The object at a gate's +0x58. SelectLapGates (0x00404df0) raises its +0x44
// flag on the two gates it leaves and clears it on the two it picks.
struct RaceGateMarker {
    unsigned char field_0x00[0x44];
    int field_0x44;
};

// The per-gate object at RaceGateEvent+0x420 (TrackGameViewOwner+0x420).
struct RaceGateObject {
    unsigned char field_0x00[0x30];
    int drawEnabled;  // +0x30  RadarOverlay skips the gate while it is 0
    unsigned char field_0x34[0x50 - 0x34];
    float width;  // +0x50  BuildNodeList passes it as AppendNode's scale (the node's width); the radar draws it as a half width
    int height;  // +0x54 BuildNodeList passes it as AppendNode's height
    RaceGateMarker* marker;  // +0x58  SelectLapGates (0x00404df0) flags it; skipped while null
};

// The gate event as 0x0041d170 and the radar walk it: +0xac counts the
// entries at +0x420, and +0xd8 holds the placements.
struct RaceGateEvent {
    unsigned char field_0x000[0xac];
    int gateCount;  // +0xAC  BuildNodeList's loop bound (one gate node per entry)
    int lapGate;  // +0xB0  the gate SelectLapGates picked (the radar highlights it)
    int nextLapGate;  // +0xB4  (lapGate + 1) % gateCount
    unsigned char field_0x0b8[0xd8 - 0xb8];
    RaceGatePose gateSources[0x23];  // +0xD8  BuildNodeList hands the records to AppendNode (center/direction per gate)
    RaceGateObject* gateEntries[1];  // +0x420  BuildNodeList reads entry i's width and value
};
