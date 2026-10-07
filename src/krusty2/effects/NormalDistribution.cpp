// NormalDistribution.cpp -- reconstruction of D:\aardvark\VC\krusty2\NormalDistribution.cpp.
#include "core/DebugAlloc.h"

#include <math.h>

// A tabulated bell curve.  Tier 3 name: the RTTI has no class for it (it is a plain object
// with no vtable), but the constructor fills a table with the polynomial
// 1 - 0.948667x + 0.2964x^2 - 0.030333x^3 for x = 0..3 and Lookup indexes it by |x - mean| / sigma.
class NormalDistribution {
public:
    explicit NormalDistribution(int resolution);
    ~NormalDistribution();
    float Lookup(float mean, float value, float sigma);

    float* table;   // +0x00 allocated in the ctor (DebugMalloc(count*4, __FILE__, 0x11)), freed by the dtor (line 0x1e)
    int count;      // +0x04 resolution + 1: table length, loop bound and scale in Lookup
};

NormalDistribution::NormalDistribution(int resolution)
{
    count = resolution + 1;
    table = (float*)DebugMalloc(count * sizeof(float), __FILE__, 0x11);
    float x = 0.0f;
    for (int i = 0; i < count; i++) {
        table[i] = 1.0f - 0.948667f * x + 0.2964f * x * x - 0.030333f * x * x * x;
        x += 3.0f / count;
    }
}

NormalDistribution::~NormalDistribution()
{
    if (table)
        DebugFree(table, __FILE__, 0x1e);
}

float NormalDistribution::Lookup(float mean, float value, float sigma)
{
    if (sigma <= 0.0f)
        return 0.0f;
    float z = (float)fabs(value - mean) / sigma;
    if (z >= 3.0f)
        return 0.0f;
    int index = (int)(count * z * -0.33333334f);
    return table[-index];
}

// $E initialiser at 0x004aff90 (+ atexit 0x004affa0 / $F 0x004affb0).
static NormalDistribution g_normalDistribution(100);
