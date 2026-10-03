// SoultreePhysicsBaseObject virtual overrides whose retail bodies lie OUTSIDE the
// SoulTreePhysics.cpp bracket: 0x0040b410..0x0040b5e0 (Bike.cpp), 0x00464e80/0x00464e90
// (FollowCam.cpp), 0x004aa150..0x004aa210 (between Motnctrl.cpp and MSZoneInterface.cpp) and
// 0x00507920 (Terrain.cpp).  The class's own table points at them, so they are most likely inline
// members kept from the first TU that emitted each COMDAT (tier 2).
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsContact.h"

// slot 6 (0x0040b410): shared with Vehicle/Character (same address).  Applies a drag-like
// correction along field_0x64 limited by the amount in *b.  Semantic reading is tier 3;
// note that the retail code compares and divides by the SQUARED length (see below).
// Reading: v = -(k * speed) * dir is a velocity-proportional (linear drag) vector.  The
// amount applied is clamped by a budget (num/den scaled by the available time t, num/den
// being a mass over the step) - if the drag exceeds the budget it is scaled down, otherwise it
// is applied whole and the budget is charged for what was used.  The result accumulates into *a.
void SoultreePhysicsBaseObject::UnknownVirtualSlot6(Vec3* a, float* b)
{
    if (prevSpeed <= 0.001f)
        return;
    float k = -(dragCoefficient * linearSpeed);
    scratchVector = velocity * k;
    float lenSq = scratchVector.z * scratchVector.z + (scratchVector.x * scratchVector.x + scratchVector.y * scratchVector.y);
    if (lenSq == 1.0f) {
        lenSq = 1.0f;
    } else {
        float root = FastSqrt(lenSq);
        if (root <= 0.0f)
            return;
    }
    if (linearSpeed == 0.0f)
        return;
    float num = bodyMass;
    float den = stepTime;
    float t = linearSpeed;
    if (!(t <= *b))
        t = *b;
    if (t <= 0.0f)
        t = 0.0f;
    float q = num / den;
    float r = q * t;
    Vec3 v;
    if (lenSq > r) {
        v = scratchVector * (r / lenSq);
        *b -= t;
    } else {
        *b -= lenSq / q;
        v = scratchVector;
    }
    scratchVector2 = v;
    *a += scratchVector2;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot10()
{
    if (totalWeight == baseWeight)
        return 1;
    return 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot22()
{
    return staggerCounter == staggerPhase;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot19(SoultreeAttachment* a)
{
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot26()
{
}

// slot 5 (0x004aa150): shared with Vehicle/Character, compares its argument with zero.
int SoultreePhysicsBaseObject::UnknownVirtualSlot5(int value)
{
    return value == 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot23()
{
    return !airborne && linearSpeed >= 1.0f;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot24()
{
    return 0;
}

float SoultreePhysicsBaseObject::UnknownVirtualSlot32()
{
    if (UnknownVirtualSlot10())
        return 0.0f;
    return loadWeight;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot34()
{
    sceneNode->GetAxesIn(0, &bodyForward, &bodyUp);
}

// slot 35 (0x004aa1e0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot35(int a, int b)
{
    sceneNode->SetAxesIn(0, &bodyForward, &bodyUp, a, b);
}

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

int SoultreePhysicsBaseObject::UnknownVirtualSlot12(int value)
{
    return 0;
}
