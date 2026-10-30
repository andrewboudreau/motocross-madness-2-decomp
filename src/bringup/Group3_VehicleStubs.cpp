// Link bring-up definitions (best-guess admissions, see docs/GAME_BUILD.md):
// Vehicle shared-stub entries and the VehicleImpactSink position setters.
//
// Evidence: decoded retail bodies.
//   0x00478fe0  xor eax,eax; ret          -> Vehicle::Method_00478FE0
//   0x00464e80  ret 4                     -> Vehicle slot 65 (shared empty stub)
//   0x00464e90  ret                       -> Vehicle slot 91 (shared empty stub)
//   0x004b8d90  latch +0x44 into +0x50, store the new vector at +0x44 and the
//               float at +0x78 (ret 0x10)
//   0x004b9dc0  latch +0x44 into +0x50, store the new vector at +0x44 (ret 0xc)
#include "vehicle/Vehicle.h"

// 0x00478fe0: the shared `return 0` stub CheckCrash calls directly.
int Vehicle::Method_00478FE0()
{
    return 0;
}

// Slot 65 is the shared empty stub 0x00464e80 (`ret 4`).
void Vehicle::UnknownVirtualSlot65(float)
{
}

// Slot 91 is the shared empty stub 0x00464e90 (`ret`).
void Vehicle::UnknownVirtualSlot91()
{
}

// 0x004b8d90: the emitter position setter keeps the previous position (+0x50) and
// stores the new one (+0x44) plus its intensity at +0x78.  VehicleImpactSink only
// pads past +0x74 (scrapeVector), so +0x78 is addressed through the raw object.
// best guess: +0x78 is the emitter's intensity/rate float (name provisional).
void VehicleImpactSink::SetPosition(Vec3 pos, float intensity)
{
    previousVector = currentVector;
    currentVector = pos;
    *(float*)((char*)this + 0x78) = intensity;
}

// 0x004b9dc0: the other emitter class's position setter (no intensity).
void VehicleImpactSink::SetPosition(Vec3 pos)
{
    previousVector = currentVector;
    currentVector = pos;
}
