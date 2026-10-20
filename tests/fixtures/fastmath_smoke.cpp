#include <math.h>
#include <stdio.h>
#include "math/FastMath.h"
extern unsigned long g_FastSqrtTable[256];
extern unsigned char g_FastInvSqrtTable[128];
extern float g_TrigTableScale;
extern float g_SinTable[65536], g_CosTable[65536], g_TanTable[65536];
int main() {
    if (g_FastSqrtTable[255] || g_FastInvSqrtTable[127] || g_TrigTableScale ||
        g_SinTable[65535] || g_CosTable[65535] || g_TanTable[65535]) return 1;
    InitFastMath();
    if (g_TrigTableScale != 10430.21875f || g_FastInvSqrtTable[64] != 255 ||
        g_SinTable[0] != 0 || g_CosTable[0] != 1 || g_TanTable[0] != 0) return 2;
    if (FastSqrt(0.0f) != 0.0f || FastSqrt(1.0f) != 1.0f || FastSqrt(4.0f) != 2.0f) return 3;
    for (int i = 1; i <= 10000; ++i) {
        float x = i * 0.037f;
        double root = sqrt(x), inverse = 1.0/root;
        if (fabs(FastSqrt(x)/root - 1.0) > 0.016 ||
            fabs(FastInvSqrt(x)/inverse - 1.0) > 0.000001 ||
            fabs(FastInvSqrtEstimate(x)/inverse - 1.0) > 0.009) { printf("x=%.9g sqrt=%.9g inv=%.9g est=%.9g expected=%.9g\n", x, FastSqrt(x), FastInvSqrt(x), FastInvSqrtEstimate(x), inverse); return 4; }
    }
    for (unsigned int j = 0; j < 65536; j += 127) {
        float angle = j * (1.0f / 10430.21875f);
        if (fabs(g_SinTable[j] - sin(angle)) > 0.000001 ||
            fabs(g_CosTable[j] - cos(angle)) > 0.000001) return 5;
    }
    puts("FastMath: initialized tables, 10000 sqrt/inverse samples, 517 trig samples passed");
    return 0;
}
