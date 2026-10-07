// BikeAI.cpp -- reconstruction of D:\aardvark\VC\krusty2\BikeAI.cpp (string 0x005678d4).
//
// Extent (strong inference): 0x0040d070..0x00416e1f.
//  * Bike.cpp's last function is Bike vtable slot 96 at 0x0040d030 (ends 0x0040d063);
//    bikerace.cpp's first xref is 0x0041807d.  No other source path sorts between
//    Bike.cpp and BikeAI.cpp.
//  * BikeAI's own __FILE__ xrefs: 0x00414847 and 0x0041486f (debug new in 0x00414370)
//    and 0x00416c36 (in 0x00415640); EH funclet 0x00549916.
//  * The file-static filter at 0x00577ac0 initialised by 0x0040d080 is read and
//    written only by 0x00413200, a KrustyBike method called from KrustyBike.cpp
//    (0x00496c27).  So the $E group at 0x0040d070 opens this TU.
//  * One math/Math3D.h constant set (0x00414210..0x0041435b) sits in the range; the
//    next set (0x00417350) belongs to the TU that holds BikeCamera, so BikeCamera
//    (0x00416e20..) is outside this file.
// The free functions below are cdecl helpers used by the AI path code
// (0x0040d200, 0x0040e510, 0x0040eca0).  All names are provisional.
#include "math/Math3D.h"
#include "broadphase/Terrain.h"

// 0x00577ac0, 20 bytes: a first-order smoothing filter used by 0x00413200:
// alpha = min(dt / timeConstant, 1); value += (target - value) * alpha.
// The constructor's store order (value, timeConstant, max, min, alpha) is tier 1;
// member roles are tier 3.
struct UnknownBikeAIFilter {
    float value;        // +0x00
    float timeConstant; // +0x04
    float alpha;        // +0x08
    float maxValue;     // +0x0c
    float minValue;     // +0x10

    UnknownBikeAIFilter(float tc)
    {
        value = 0.0f;
        timeConstant = tc;
        maxValue = 1.0f;
        minValue = -1.0f;
        alpha = 1.0f;
    }
};

UnknownBikeAIFilter g_BikeAIFilter_00577ac0(0.5f);

// Statics with an empty constructor: their $E initializers are a jmp to a bare ret
// (0x0040d0c0, 0x0040d0e0, 0x0040d100 here, 0x0040d1e0 after 0x0040d120 and
// 0x00414350 after the Math3D set).  Their addresses never appear in the code, so
// type and identity are unknown; Vec3 (empty user constructor) reproduces the shape.
static Vec3 s_BikeAIUnknownStatic_0;
static Vec3 s_BikeAIUnknownStatic_1;
static Vec3 s_BikeAIUnknownStatic_2;

// 0x0040d120, 182 bytes, cdecl, hidden result pointer: returns v unchanged when it is
// the zero vector, otherwise v * FastInvSqrt(|v|^2).
Vec3 SafeNormalize(Vec3 v)
{
    if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
        return v;
    float s = FastInvSqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
    return Vec3(v.x * s, v.y * s, v.z * s);
}

static Vec3 s_BikeAIUnknownStatic_3;
static Vec3 s_BikeAIUnknownStatic_4;

// 0x00413190, 106 bytes, cdecl: |v|, with 0 and 1 returned exactly.
float Vec3Magnitude(const Vec3* v)
{
    float lenSq = (v->x * v->x + v->y * v->y) + v->z * v->z;
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

// 0x0040e370, 416 bytes, cdecl: ballistic landing prediction.  Steps a point from
// 'start' with velocity 'velocity' in 0.1 s steps (gravity 32: y += vy*dt - 0.16,
// vy -= 3.2) until it is no longer 6 units above the ground under it.  Writes the
// elapsed time, the displacement and the last ground normal; always returns 1.
// Tier 1 arithmetic; tier 3 names.  Callers: 0x004134f6 and 0x00415544.
int PredictLanding(Vec3 start, Vec3 velocity, float* outTime, Vec3* outDelta,
                          Vec3* outNormal, Terrain* terrain)
{
    Vec3 v = velocity;
    float t = 0.0f;
    Vec3 p = start;
    Vec3 ground = start;
    Vec3 normal;
    terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
    while (p.y + 6.0f > ground.y) {
        p += v * 0.1f;
        p.y -= 0.16f;
        v.y -= 3.2f;
        ground = p;
        terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
        t += 0.1f;
    }
    if (outTime)
        *outTime = t;
    if (outDelta)
        *outDelta = p - start;
    if (outNormal)
        *outNormal = normal;
    return 1;
}
