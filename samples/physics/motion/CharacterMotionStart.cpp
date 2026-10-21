// Complete nonexact motion-start candidate, retail 0x004a8bf0..0x004a8c41.
// Requires authentic VC6 /QIfist for the conversion helper below. Original
// translation-unit ownership and the blend method at 0x004a8c50 remain unknown.
#include <float.h>
#include <string.h>
#include "motion/Motnctrl.h"

static void FloatToInt(int* destination, float value)
{
    unsigned bits;
    memcpy(&bits, &value, sizeof(bits));
    unsigned absolute = bits & 0x7fffffff;
    unsigned negative = bits >> 31;
    unsigned exponent = absolute >> 23;
    unsigned control = _control87(0, 0);
    unsigned mode = control & _MCW_RC;
    unsigned magnitude = 0;
    int invalid = absolute >= 0x4f000000 && !(negative && absolute == 0x4f000000);

    // /QIfist emits a current-rounding FISTP64 here instead of __ftol, whose
    // temporary truncation mode would remain active on exception unwinding.
    // This conversion supplies exceptions only; integer bits below supply the
    // signed-32 result. Quiet NaN also raises invalid for signed-32 overflow.
    float exceptionInput = value;
    if (invalid) {
        unsigned quietNaN = 0x7fc00000;
        memcpy(&exceptionInput, &quietNaN, sizeof(quietNaN));
    }
    volatile __int64 exceptionResult = (__int64)exceptionInput;
    // Unmasked invalid conversion does not store an integer destination.
    if (invalid && !(control & _EM_INVALID))
        return;

    if (invalid || absolute == 0x4f000000) {
        magnitude = 0x80000000;
    } else {
        unsigned remainder = 0;
        unsigned half = 0;
        if (exponent < 127) {
            remainder = absolute;
            half = 0x3f000000;
        } else if (exponent < 150) {
            unsigned shift = 150 - exponent;
            unsigned significand = (absolute & 0x7fffff) | 0x800000;
            magnitude = significand >> shift;
            remainder = significand & ((1u << shift) - 1);
            half = 1u << (shift - 1);
        } else {
            magnitude = ((absolute & 0x7fffff) | 0x800000) << (exponent - 150);
        }
        if (remainder) {
            if ((mode == _RC_NEAR && (remainder > half || (remainder == half && (magnitude & 1)))) ||
                (mode == _RC_DOWN && negative) || (mode == _RC_UP && !negative))
                ++magnitude;
        }
        if (negative)
            magnitude = 0u - magnitude;
    }
    memcpy(destination, &magnitude, sizeof(magnitude));
}

void Character::Method_0x004a8bf0(int handle, float phase)
{
    Motion* motion = reinterpret_cast<Motion*>(handle);
    phase = (motion->frameCount - 1) * phase - 0.5f;
    // Retail reuses the count-minus-one temporary as its FISTP destination.
    // An unmasked invalid conversion leaves that previous integer untouched.
    int frame = motion->frameCount - 1;
    FloatToInt(&frame, phase);
    currentMotion = motion;
    currentFrame = &motion->frames[frame];
    ApplyPoseListSlot4(currentFrame, 0);
}
