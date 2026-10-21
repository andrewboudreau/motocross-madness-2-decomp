// Vehicle slot 36 remains an exact sample because its vector globals and
// VehSceneNodeView::SetAxesPtr dependency are not supplied. The thirteen
// other exact methods are in src/reconstructed/VehicleMethods.cpp.
#include <math.h>
#include <float.h>
#include "vehicle/Vehicle.h"
#include "collision/CollisionObject.h"
#include "soultree/SoultreePhysicsCallees.h"

// Tier 3 reading: when the facing vector (0x88) has no horizontal part it is rebuilt from the
// previous up vector (0x94) with the sign of its y, then the basis block is reset to the
// identity-like values and pushed to the scene node.
void Vehicle::UnknownVirtualSlot36()
{
    if (bodyForward.x == 0.0f && bodyForward.z == 0.0f) {
        float s = bodyForward.y >= 0.0f ? -1.0f : 1.0f;
        bodyForward = s * bodyUp;
    }
    bodyUp = g_VehZeroVec3_005778c8;
    bodyForward.y = 0.0f;
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    bodyCosRoll = 1.0f;
    bodyCosPitch = 1.0f;
    bodyRoll = 0.0f;
    bodyPitch = 0.0f;
    bodySinRoll = 0.0f;
    bodySinPitch = 0.0f;
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
    steerState->node->SetPosition(g_VehZeroVec3_005778a8);
    ((VehSceneNodeView*)steerState->node)->SetAxesPtr(&bodyForward, &bodyUp, 0, 1);
    steerState->SetAngle(0, poseNode);
}
