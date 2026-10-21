// Remaining sample-only methods: slot 6 is partial; slot 36 requires an
// unsupplied orientation-vector global. Exact methods are in
// src/reconstructed/SoultreePhysicsMethods.cpp.
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsContact.h"

#include "SoultreeDrag.cpp"

// slot 36 (0x004aa210): resolve the initial orientation vectors and reset the pose state.
void SoultreePhysicsBaseObject::UnknownVirtualSlot36()
{
    if (bodyForward.x == 0.0f && bodyForward.z == 0.0f) {
        float s = (bodyForward.y >= 0.0f) ? -1.0f : 1.0f;
        bodyForward = s * bodyUp;
    }
    bodyUp = g_SoultreeVec3_685190;
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
