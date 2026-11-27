// NormalDistribution.cpp -- reconstruction of D:\aardvark\VC\krusty2\NormalDistribution.cpp.
#include "core/DebugAlloc.h"
#include "effects/NormalDistribution.h"

#include <math.h>

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

// 0x0056ece0 (initialised .data, ahead of this file's __FILE__ string at 0x0056ece4):
// the table EcoSystem::GenerateObjects weighs slope and altitude with. Tier 3 name.
NormalDistribution* g_SharedNormalDistribution = &g_normalDistribution;
