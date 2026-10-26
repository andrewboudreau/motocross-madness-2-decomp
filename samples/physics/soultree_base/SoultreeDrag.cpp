// Complete primary slot 6 candidate; original translation-unit ownership is unproven.
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
void SoultreePhysicsBaseObject::UnknownVirtualSlot6(Vec3* force, float* speedBudget)
{
    if (prevSpeed <= 0.001f)
        return;
    float dragScale = -(dragCoefficient * linearSpeed);
    scratchVector = velocity * dragScale;
    float length = scratchVector.z * scratchVector.z + (scratchVector.x * scratchVector.x + scratchVector.y * scratchVector.y);
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
    if (!(charge <= *speedBudget))
        charge = *speedBudget;
    if (charge <= 0.0f)
        charge = 0.0f;
    float massRate = num / den;
    float limit = massRate * charge;
    Vec3 drag;
    if (length > limit) {
        drag = scratchVector * (limit / length);
        *speedBudget -= charge;
    } else {
        *speedBudget -= length / massRate;
        drag = scratchVector;
    }
    scratchVector2 = drag;
    *force += scratchVector2;
}
