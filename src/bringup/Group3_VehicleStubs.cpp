// Link bring-up definitions (best-guess admissions, see docs/GAME_BUILD.md):
// Vehicle shared-stub entries.
//
// Evidence: decoded retail bodies.
//   0x00478fe0  xor eax,eax; ret          -> Vehicle::DetectCrash
//   0x00464e80  ret 4                     -> Vehicle slot 65 (shared empty stub)
//   0x00464e90  ret                       -> Vehicle slot 91 (shared empty stub)
//
// The VehicleImpactSink position setters (0x004b8d90, 0x004b9dc0) are the strict-exact
// DustParticleEmitter::SetPosition and SparkParticleEmitter::SetPosition of
// src/reconstructed/; link_aliases entries bind the view names to them (12-byte vector by
// value, same +0x44/+0x50/+0x78 stores, no this adjustment).
#include "vehicle/Vehicle.h"

// 0x00478fe0: the shared `return 0` stub CheckCrash calls directly.
int Vehicle::DetectCrash()
{
    return 0;
}

// Slot 65 is the shared empty stub 0x00464e80 (`ret 4`).
void Vehicle::ApplyLaunchImpulse(float)
{
}

// Slot 91 is the shared empty stub 0x00464e90 (`ret`).
void Vehicle::ApplyAirControl()
{
}
