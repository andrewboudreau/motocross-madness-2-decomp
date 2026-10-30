// Group2_SoultreeSlot36.cpp -- link bring-up: SoultreePhysicsBaseObject slot 36 (0x004aa210).
//
// Taken from samples/physics/soultree_base/SoultreePhysicsInlines.cpp (exact there under
// another profile) and checked against the retail body: when the forward axis is vertical
// (x == z == 0) it is replaced by -/+ the up axis, the up axis is reset to the Y axis
// constant at 0x00685190 (MSZoneInterface.cpp's file-static kVec3YAxis, initialised to
// (0, 1, 0) by 0x004aa0c0), forward.y is cleared, slot 35 (SetAxesIn(1, 0)) and slot 34
// (GetAxesIn) run, then the pose angles and their saved copies are reset.
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"

namespace {
// Retail reads MSZoneInterface.cpp's static (0, 1, 0) vector; it is file-static there,
// so this unit keeps its own copy of the same value.
const Vec3 kSlot36UpAxis(0.0f, 1.0f, 0.0f);
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot36()
{
    if (bodyForward.x == 0.0f && bodyForward.z == 0.0f) {
        float s = (bodyForward.y < 0.0f) ? 1.0f : -1.0f;
        Vec3 flipped;
        flipped.x = s * bodyUp.x;
        flipped.y = s * bodyUp.y;
        flipped.z = s * bodyUp.z;
        bodyForward = flipped;
    }
    bodyUp = kSlot36UpAxis;
    bodyForward.y = 0.0f;
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    bodyRoll = 0.0f;
    bodyPitch = 0.0f;
    bodySinRoll = 0.0f;
    bodyCosRoll = 1.0f;
    savedForward = bodyForward;
    savedUp = bodyUp;
    bodySinPitch = 0.0f;
    bodyCosPitch = 1.0f;
    savedPitch = 0.0f;
    savedRoll = 0.0f;
    savedYaw = bodyYaw;
    savedSinRoll = 0.0f;
    savedCosRoll = 1.0f;
    savedCosPitch = 1.0f;
    savedSinPitch = 0.0f;
}
