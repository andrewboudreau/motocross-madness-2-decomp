// Remaining sample-only methods: slot 6 is partial; slot 36 requires an
// unsupplied orientation-vector global. Exact methods are in
// src/reconstructed/SoultreePhysicsMethods.cpp.
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
    // Declared den first: retail loads bodyMass (0x158) before stepTime (0x13c), which this
    // order reproduces (num first loads 0x13c first).
    float den = stepTime;
    float num = bodyMass;
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
