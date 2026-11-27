// NormalDistribution.h -- the tabulated bell curve of D:\aardvark\VC\krusty2\NormalDistribution.cpp.
#ifndef NORMAL_DISTRIBUTION_H
#define NORMAL_DISTRIBUTION_H

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

// 0x0056ece0: points at the file's static object (0x006886e8); EcoSystem weighs slope and
// altitude with it. Tier 3 name.
extern NormalDistribution* g_SharedNormalDistribution;

#endif
