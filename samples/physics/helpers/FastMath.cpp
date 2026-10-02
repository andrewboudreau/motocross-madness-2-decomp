// FastMath.cpp -- table-driven float helpers declared in ../common/Math3D.h
// (0x00460b50 FastSqrt, 0x00460c00 FastInvSqrt). They sit in the EventManager.cpp range by
// proximity only (tier 3 for the file).
#include "math/Math3D.h"

// 256 x uint32 mantissa table at 0x005dafe0 (sqrt) and 128 x uint8 table at 0x005daf5c
// (rsqrt), both filled at run time, so they are extern here.
extern unsigned long g_FastSqrtTable[256];
extern unsigned char g_FastInvSqrtTable[128];

// 0x00460b50. Returns 0 for x == 0; otherwise the result is assembled bit-wise from the
// halved exponent and the table entry indexed by the top 8 bits of the mantissa (with the
// exponent's parity folded into bit 23).
float FastSqrt(float x)
{
    if (x == 0.0f)
        return 0.0f;
    unsigned long bits = *(unsigned long*)&x;
    short exponent = (short)((bits >> 23) - 0x7f);
    unsigned long mantissa = bits & 0x7fffff;
    if (exponent & 1)
        mantissa |= 0x800000;
    exponent >>= 1;
    *(unsigned long*)&x = g_FastSqrtTable[mantissa >> 16] | ((exponent + 0x7f) << 23);
    return x;
}

// 0x00460c00. Table estimate then two Newton steps y = 0.5 * y * (3 - x*y*y).
float FastInvSqrt(float x)
{
    unsigned long bits = *(unsigned long*)&x;
    unsigned char e = (unsigned char)(bits >> 23);   // biased exponent byte
    float y;
    *(unsigned long*)&y = ((0x5f000000 - (e << 22)) & 0xff800000) |
                          ((unsigned long)g_FastInvSqrtTable[(bits >> 17) & 0x7f] << 15);
    y = 0.5f * y * (3.0f - x * y * y);
    return 0.5f * y * (3.0f - x * y * y);
}
