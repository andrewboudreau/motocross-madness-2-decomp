#include "vehicle/Bike.h"
#include "Wrecker.h"
#include <stddef.h>

typedef char CompleteWreckerSize[sizeof(Wrecker) == 0x53c ? 1 : -1];
typedef char BikeWreckerField[offsetof(Bike, wrecker) == 0x604 ? 1 : -1];
typedef char WreckerCharacterField[offsetof(Wrecker, character) == 0x34 ? 1 : -1];
typedef char WreckerCollisionField[offsetof(Wrecker, collisionModel) == 0x38 ? 1 : -1];
typedef char WreckerStateField[offsetof(Wrecker, wreckState) == 0x44 ? 1 : -1];
typedef char WreckerContactField[offsetof(Wrecker, hadContact) == 0xac ? 1 : -1];
typedef char WreckerProbeField[offsetof(Wrecker, anyProbeHit) == 0xd8 ? 1 : -1];
typedef char WreckerHitField[offsetof(Wrecker, hitCenter) == 0xdc ? 1 : -1];
typedef char WreckerParticlesField[offsetof(Wrecker, particles) == 0x11c ? 1 : -1];
typedef char WreckerSkipField[offsetof(Wrecker, skipProbeEffects) == 0x52c ? 1 : -1];

// The subordinate declarations remain explicit views at their old boundaries.
typedef char RiderModelView[offsetof(BikeA34, r_0x1a0) == 0x1a0 ? 1 : -1];
typedef char CollisionTypeView[offsetof(BikeA38, n_0x64) == 0x64 ? 1 : -1];
typedef char CollisionTerrainView[offsetof(BikeA38, n_0xe4) == 0xe4 ? 1 : -1];
typedef char VectorSizes[sizeof(Vec3) == 12 && sizeof(Vector3) == 12 ? 1 : -1];
typedef char VectorY[offsetof(Vec3, y) == 4 && offsetof(Vector3, y) == 4 ? 1 : -1];
typedef char VectorZ[offsetof(Vec3, z) == 8 && offsetof(Vector3, z) == 8 ? 1 : -1];

GameObject* BikeWreckerBase(Wrecker* value) { return value; }
Wrecker* BikeWreckerPointer(Bike* bike) { return bike->wrecker; }

// The append's canonical return is an index, and setup returns this. The
// callers intentionally ignore the former and test the latter for nonnull.
void CheckWreckerSignatures() {
    int (Wrecker::*append)(int) = &Wrecker::AddIndexedAnimation;
    Wrecker* (Wrecker::*setup)(void*, UnknownWreckerCharacter*, const char*) =
        &Wrecker::SetupWrecker;
}
