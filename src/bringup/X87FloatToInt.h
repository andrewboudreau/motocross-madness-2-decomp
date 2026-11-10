// X87FloatToInt.h -- the float-to-int conversion of retail's inline-asm fistp helper,
// in ordinary C++ for the bring-up bodies.
//
// Retail converts some floats with an inlined helper of the shape
//   fld dword ptr [value] ; mov eax, [destination] ; fistp dword ptr [eax]
// (confirmed by the bytes: the value is stored to a binary32 stack slot and reloaded
// first, so it is rounded to float before the conversion). FISTP rounds under the
// current x87 rounding mode; a VC6 `(int)` cast instead truncates through __ftol, and
// `floor(v + 0.5f)` rounds ties up. Call sites written `X87FloatToInt(v - 0.5f)` are
// retail's `fistp(v - 0.5)`, i.e. floor(v) except at exact ties.
//
// Provisional: the game is assumed to run with the default round-to-nearest-even
// control word (nothing found so far changes it); the helper still reads the current
// mode with _control87 so it follows any other mode the way FISTP would.
//
// Out-of-range values (|v| >= 2^31, infinities, NaNs) give the integer indefinite
// 0x80000000, as FISTP does with the invalid-operation exception masked. Exception
// flags are not reproduced. Modelled on Terrain.cpp's FloatToInt (0x00507c10) and
// samples/physics/motion/CharacterMotionFloatToInt.h, which also keep the flags.
#ifndef X87_FLOAT_TO_INT_H
#define X87_FLOAT_TO_INT_H

#include <float.h>
#include <string.h>

static inline int X87FloatToInt(float value)
{
    unsigned bits;
    memcpy(&bits, &value, sizeof(bits));
    unsigned absolute = bits & 0x7fffffff;
    unsigned negative = bits >> 31;
    unsigned exponent = absolute >> 23;

    // Every float below 2^31 in magnitude rounds into range (the largest is 2^31 - 128),
    // and -2^31 itself is 0x80000000 too.
    if (absolute >= 0x4f000000)
        return (int)0x80000000u;

    unsigned magnitude = 0;
    unsigned remainder = 0;
    unsigned half = 0;
    if (exponent < 127) {
        // |value| < 1: the bit patterns of non-negative floats order like their values.
        remainder = absolute;
        half = 0x3f000000;  // 0.5f
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
        switch (_control87(0, 0) & _MCW_RC) {
        case _RC_NEAR:
            if (remainder > half || (remainder == half && (magnitude & 1)))
                ++magnitude;
            break;
        case _RC_DOWN:
            if (negative)
                ++magnitude;
            break;
        case _RC_UP:
            if (!negative)
                ++magnitude;
            break;
        default:  // _RC_CHOP
            break;
        }
    }
    return negative ? -(int)magnitude : (int)magnitude;
}

#endif
