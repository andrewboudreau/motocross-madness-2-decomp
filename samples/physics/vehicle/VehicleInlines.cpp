// Vehicle virtual overrides whose retail bodies lie OUTSIDE the Vehicle.cpp link-order bracket:
// 0x0040c4c0..0x0040cb60 sit among Bike.cpp's methods, 0x00492220/0x00492270 in KrustyBike.cpp
// and 0x004a6ba0 in Motnctrl.cpp.  Vehicle's own table points at these copies, so they are most
// likely inline members of the Vehicle class whose COMDAT copy the linker kept from the first
// TU that emitted it (tier 2).  They are kept out of src/krusty2/vehicle/Vehicle.cpp until the
// inline-in-header reconstruction is shown to reproduce that placement.
#include <math.h>
#include <float.h>
#include "vehicle/Vehicle.h"
#include "collision/CollisionObject.h"
#include "soultree/SoultreePhysicsCallees.h"

void Vehicle::UnknownVirtualSlot34()
{
    modelNode->GetAxesIn(0, &bodyForward, &bodyUp);
    sideAxis = ((VehV3&)bodyForward).CrossB(bodyUp);
}

void Vehicle::UnknownVirtualSlot35(int a, int b)
{
    modelNode->SetAxesIn(0, &bodyForward, &bodyUp, a, b);
    sideAxis = ((VehV3&)bodyForward).CrossB(bodyUp);
}

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
    steerState->steerNode->SetPosition(g_VehZeroVec3_005778a8);
    ((VehSceneNodeView*)steerState->steerNode)->Method_004FBD70(&bodyForward, &bodyUp, 0, 1);
    steerState->Method_00504E20(0, poseNode);
}

float Vehicle::UnknownVirtualSlot45() { return 3.0f; }

int Vehicle::UnknownVirtualSlot51() { return spawnProtected; }

int Vehicle::UnknownVirtualSlot52() { return crashState == 0; }

int Vehicle::UnknownVirtualSlot68(int* out)
{
    *out = 15;
    return 1;
}

void Vehicle::UnknownVirtualSlot69() { crashTimer = crashTimerReload; }

int Vehicle::UnknownVirtualSlot23()
{
    return !airborne && field_0x434 >= 0.2f;
}

int Vehicle::UnknownVirtualSlot24()
{
    return field_0x5b0 < engineState->field_0x00 && crashState == 0 && linearSpeed < 22.0f;
}

int Vehicle::UnknownVirtualSlot25()
{
    return field_0x5b0 > engineState->field_0x00 && crashState == 0 && linearSpeed < 22.0f;
}

int Vehicle::UnknownVirtualSlot78()
{
    if (inputMap->UnknownVirtualSlot3(3, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return inputMap->UnknownVirtualSlot3(0x1f, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot79()
{
    if (inputMap->UnknownVirtualSlot3(4, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return inputMap->UnknownVirtualSlot3(0x2d, 0, 0x3f, 0) != 0;
}

void Vehicle::UnknownVirtualSlot92(const Vec3*, const Vec3*) {}
