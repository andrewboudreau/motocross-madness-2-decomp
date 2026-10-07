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

// The same unit (tier 2 by position: between FastInvSqrt and the stream code at
// 0x00460d10, all on the same tables) also builds the tables and has an estimate-only
// variant.  Names tier 3.
#include <math.h>

extern float g_TrigTableScale;          // 0x005dafdc: table entries per radian
extern float g_SinTable[0x10000];       // 0x0061b3e0
extern float g_CosTable[0x10000];       // 0x005db3e0
extern float g_TanTable[0x10000];       // 0x0059af5c

// 0x00460ae0: fills the 256-entry square root mantissa table from sqrt over
// two exponent octaves ([1,2) at i, [2,4) at i + 0x80), keeping the result's
// top 7 mantissa bits in place.
void InitFastSqrtTable()
{
    for (unsigned int i = 0; i < 0x80; i++) {
        float f;
        *(unsigned long*)&f = (i | 0x3f80) << 16;
        f = (float)sqrt(f);
        g_FastSqrtTable[i] = *(unsigned long*)&f & 0x7f0000;
        *(unsigned long*)&f = (i | 0x4000) << 16;
        f = (float)sqrt(f);
        g_FastSqrtTable[i + 0x80] = *(unsigned long*)&f & 0x7f0000;
    }
}

void InitFastInvSqrtTable();
void InitTrigTables();

// 0x00460ad0: builds all three tables; called once from 0x0046799c.
void InitFastMath()
{
    InitFastSqrtTable();
    InitFastInvSqrtTable();
    InitTrigTables();
}

// 0x00460bb0: fills the 128-entry reciprocal square root table from 1 / sqrt over one
// exponent octave (mantissa top bits | 0x1f80 << 17), rounded to 8 mantissa bits.
void InitFastInvSqrtTable()
{
    for (int i = 0; i < 0x80; i++) {
        float x;
        float r;
        *(unsigned long*)&x = (i | 0x1f80) << 17;
        r = 1.0f / (float)sqrt(x);
        g_FastInvSqrtTable[i] = (unsigned char)((*(unsigned long*)&r + 0x2000) >> 15);
    }
    g_FastInvSqrtTable[0x40] = 0xff;
}

// 0x00460c70: the table estimate of FastInvSqrt without the Newton steps.
float FastInvSqrtEstimate(float x)
{
    unsigned long bits = *(unsigned long*)&x;
    unsigned char e = (unsigned char)(bits >> 23);   // biased exponent byte
    *(unsigned long*)&x = ((0x5f000000 - (e << 22)) & 0xff800000) |
                          ((unsigned long)g_FastInvSqrtTable[(bits >> 17) & 0x7f] << 15);
    return x;
}

// 0x00460cb0: 65536-entry sine, cosine and tangent tables over one turn.
void InitTrigTables()
{
    g_TrigTableScale = 10430.21875f;
    for (unsigned int i = 0; i < 0x10000; i++) {
        float angle = i * 9.5873799e-05f;
        float s = (float)sin(angle);
        g_SinTable[i] = s;
        g_CosTable[i] = (float)cos(angle);
        g_TanTable[i] = s / (float)cos(angle);
    }
}
