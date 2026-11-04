// Exact primary slot 6 of SoultreePhysicsBaseObject (vtable 0x00557d90, object offset 0).
// This is a reconstruction grouping, not a claim about original TU ownership: the slot
// lies in Bike.cpp's address range, most likely an inline member kept from the first TU
// that emitted it (src/krusty2/README.md).
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsContact.h"

// Primary slot 6 at 0x0040b410 (complete retail extent 432 bytes).
// The member names and drag/budget interpretation remain provisional. Retail first
// stores squared magnitude, then overwrites that same float with FastSqrt's result.
// Its clamp and budget charge therefore use magnitude, not squared magnitude.
// The unscaled branch charges *speedBudget before reading scratchVector: aliases observe it.
// Reading the magnitude through `s` gives retail's [esi] loads (the members directly give
// [edi + 0x1b0]); `*speedBudget < charge` gives its fcom (`!(charge <= *speedBudget)` and
// `charge > *speedBudget` add a dup and fcomp). The scaled branch builds the product in a
// named temporary and copies it to `drag` after the budget charge: retail subtracts before
// the copy's z load. Assigning the product to `drag` directly puts the fsub after it
// (426/432); subtracting before the product is worse (164/434).
void SoultreePhysicsBaseObject::UnknownVirtualSlot6(Vec3* force, float* speedBudget)
{
    if (prevSpeed <= 0.001f)
        return;
    float dragScale = -(dragCoefficient * linearSpeed);
    scratchVector = velocity * dragScale;
    const Vec3& s = scratchVector;
    float length = s.z * s.z + (s.x * s.x + s.y * s.y);
    if (length == 1.0f) {
        length = 1.0f;
    } else {
        length = FastSqrt(length);
        if (length <= 0.0f)
            return;
    }
    if (linearSpeed == 0.0f)
        return;
    // Declared den first: retail loads bodyMass (0x158) before stepTime (0x13c), which this
    // order reproduces (num first loads 0x13c first).
    float den = stepTime;
    float num = bodyMass;
    float charge = linearSpeed;
    if (*speedBudget < charge)
        charge = *speedBudget;
    if (charge <= 0.0f)
        charge = 0.0f;
    float massRate = num / den;
    float limit = massRate * charge;
    Vec3 drag;
    if (length > limit) {
        Vec3 scaled = scratchVector * (limit / length);
        *speedBudget -= charge;
        drag = scaled;
    } else {
        *speedBudget -= length / massRate;
        drag = scratchVector;
    }
    scratchVector2 = drag;
    *force += scratchVector2;
}
