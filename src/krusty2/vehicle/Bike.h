// Bike: motorcycle-specific layer over Vehicle (RTTI ".?AVBike@@", vtables
// 0x00550900 (primary, 103 slots), 0x005508cc @+540 (12 slots),
// 0x0055085c @+1848 (27 slots, virtual base)).
//
// Layout facts (tier 1 unless noted):
//  * Bike : Vehicle : SoultreePhysicsCharacter (SoultreePhysicsBaseObject at 0,
//    D3DIMSoultreeCharacter at object offset 540); GameObject/BaseObject is a
//    virtual base placed at +1848 in a complete Bike (vtordisp at +0x734) and
//    +1472 in a complete Vehicle, so the fields Bike adds occupy [0x5bc, 0x734).
//    (RTTI hierarchy + vtable_write_xrefs.)
//  * Slots 0..96 come from Vehicle (Bike overrides some); Bike introduces
//    97..102.  (vtable_overrides.json)
//
// The bases are the canonical classes (vehicle/Vehicle.h and the headers it
// includes); overrides use their signatures.  Every field named field_0xNN is
// tier 3 (offset is tier 1/2 from decoded instructions, the type and any role
// are guesses).
#ifndef MCM2_PHYSICS_BIKE_H
#define MCM2_PHYSICS_BIKE_H

#include "../../reconstructed/TrackGameGlobal.h"
#include "vehicle/Vehicle.h"
#include "core/DebugAlloc.h"


class Tire;
class Wrecker;
struct BikeA644;
class InlineShock;
class RotatingShock;


// 24-byte block the loader's sixth argument points to; copied into Bike+0x5c8 when present
// (tier 3: contents unknown, only the whole-block copy is decoded).
#include "../../reconstructed/TextureLoader.h"

// Rate-limited smoother at Bike+0x644 (0x14 bytes; the loader builds it with
// `new(__FILE__, 0x7e0)` and an inline constructor). Bike+0x640 is a VehicleSmoother.
struct BikeA644 {
    BikeA644(float tc, float rise, float fall)
    {
        smoothTime = tc;
        smoothedValue = 0;
        maxRise = rise;
        maxFall = fall;
        smoothRatio = 1.0f;
    }
    int smoothedValue;
    float smoothTime;
    float smoothRatio;
    float maxRise;
    float maxFall;
};

// The scene node (d3d_field_0x1a0, poseNode, SoultreeObject::firstChild) is SoultreeObject
// (../common/SoultreeObject.h).

// Bike.cpp uses its Math3D header-local kVec3Zero, initialized at 0x00407890.

#include "BikeCore.h"

#endif
