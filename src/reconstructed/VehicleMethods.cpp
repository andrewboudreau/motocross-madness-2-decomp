// Exact Vehicle primary-vtable methods. Retail vtable 0x00558a84 has
// object offset 0; each method is verified against its own slot. This is
// a reconstruction grouping, not original translation-unit ownership.
#include <math.h>
#include <float.h>
#include "vehicle/Vehicle.h"
#include "collision/CollisionObject.h"
#include "soultree/SoultreePhysicsCallees.h"

// The cross product goes through a named local. Without it (the slot 35 shape) VC6 loads
// bodyForward.z / .y before bodyUp.x in the y and z terms; any extra live symbol in the
// function (this local, a node or axis pointer local, or slot 35's two parameters) gives
// retail's bodyUp.x-first order. An unused int local does not.
void Vehicle::UnknownVirtualSlot34()
{
    modelNode->GetAxesIn(0, &bodyForward, &bodyUp);
    Vec3 side = ((VehV3&)bodyForward).CrossB(bodyUp);
    sideAxis = side;
}

void Vehicle::UnknownVirtualSlot35(int a, int b)
{
    modelNode->SetAxesIn(0, &bodyForward, &bodyUp, a, b);
    sideAxis = ((VehV3&)bodyForward).CrossB(bodyUp);
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
    return !airborne && forwardSpeed >= 0.2f;
}

int Vehicle::UnknownVirtualSlot24()
{
    return prevThrottleFraction < engineState->throttleFraction && crashState == 0 && linearSpeed < 22.0f;
}

int Vehicle::UnknownVirtualSlot25()
{
    return prevThrottleFraction > engineState->throttleFraction && crashState == 0 && linearSpeed < 22.0f;
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
