// Group6_TerrainShadow.cpp -- bring-up bodies for the call views that
// samples/physics/shadow/TerrainShadow.cpp (admitted best-guess candidate) declares but
// nothing defines.  None of this is a byte-match claim; it exists so the source-built game
// links.  Where the retail VA already has a definition under its owner's name, the view
// forwards to it:
//   0x004a1410 TerrainMatrixIdentityInto  -> ShadowMatrixIdentity (-> IdentityMatrix)
//   0x004a1860 TerrainMatrixMultiplyInto  -> ShadowMatrixMultiply (-> MatrixMult)
//   0x00421cb0 TerrainShadowSum           -> Vec3AddCall
//   0x00421d00 TerrainShadowDifference    -> Vec3SubtractCall
//   0x00404e60 TerrainShadowVecCall ctor  -> written out (Vec3::Vec3 is inline everywhere)
//   0x00506e90 TerrainShadowCaster::CastSegment      -> Terrain::CastSegment
#include "broadphase/Terrain.h"
#include "shadow/ProjectedShadow.h"
#include "Griddraw.h"

// The sample's local views, redeclared with its class keys so the mangled names agree.
struct TerrainShadowVec {
    TerrainShadowVec() {}
    TerrainShadowVec(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float x, y, z;
};
struct TerrainShadowVecCall : TerrainShadowVec {
    TerrainShadowVecCall(float x_, float y_, float z_);                            // 0x00404e60
};
class TerrainShadowCaster {
public:
    int CastSegment(const TerrainShadowVec* from, const TerrainShadowVec* to, TerrainShadowVec* hit,
                    DrawableGridNode** cell, int a, int b);                             // 0x00506e90
};
ShadowMatrix* TerrainMatrixIdentityInto(ShadowMatrix* out);                         // 0x004a1410
ShadowMatrix* TerrainMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b);  // 0x004a1860
TerrainShadowVec TerrainShadowSum(const TerrainShadowVec& a, const TerrainShadowVec& b);
TerrainShadowVec TerrainShadowDifference(const TerrainShadowVec& a, const TerrainShadowVec& b);

// Providers (src/reconstructed/VectorCalls.cpp), declared only as far as these calls need.
struct Vec3;
Vec3* Vec3AddCall(Vec3* out, const Vec3* a, const Vec3* b);        // 0x00421cb0
Vec3* Vec3SubtractCall(Vec3* out, const Vec3* a, const Vec3* b);   // 0x00421d00

// ---------------------------------------------------------------------------------------------
// 0x004a1410 / 0x004a1860 with an explicit destination (returned), as the sample calls them.
// best guess: same retail functions as ShadowMatrixIdentity / ShadowMatrixMultiply
// (src/bringup/Group5_ShadowBridges.cpp forwards those to IdentityMatrix / MatrixMult).
ShadowMatrix* TerrainMatrixIdentityInto(ShadowMatrix* out)
{
    *out = ShadowMatrixIdentity();
    return out;
}

ShadowMatrix* TerrainMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b)
{
    *out = ShadowMatrixMultiply(a, b);
    return out;
}

// ---------------------------------------------------------------------------------------------
// 0x00421cb0 / 0x00421d00: the out-of-line Vec3 operator+ / operator- (the hidden result
// pointer is Vec3AddCall's explicit output).  TerrainShadowVec has Vec3's layout (3 floats).
// best guess: forwarding to the canonical call views of the same retail VAs.
TerrainShadowVec TerrainShadowSum(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    TerrainShadowVec result;
    Vec3AddCall(reinterpret_cast<Vec3*>(&result), reinterpret_cast<const Vec3*>(&a),
                reinterpret_cast<const Vec3*>(&b));
    return result;
}

TerrainShadowVec TerrainShadowDifference(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    TerrainShadowVec result;
    Vec3SubtractCall(reinterpret_cast<Vec3*>(&result), reinterpret_cast<const Vec3*>(&a),
                     reinterpret_cast<const Vec3*>(&b));
    return result;
}

// 0x00404e60 (thiscall, ret 0xc): Vec3::Vec3(float, float, float), the COMDAT copy.
// best guess: the three component stores.
TerrainShadowVecCall::TerrainShadowVecCall(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

// ---------------------------------------------------------------------------------------------
// 0x00506e90 (thiscall, ret 0x18): Terrain::CastSegment.  The sample's caster is the
// TerrainShadow's Terrain; its fourth argument (the hit cell out-pointer) is Terrain's outNode,
// which reaches GridNode slot 1 (IntersectSegment).
// best guess: forwarding to the canonical method at the same VA.
int TerrainShadowCaster::CastSegment(const TerrainShadowVec* from, const TerrainShadowVec* to,
                                     TerrainShadowVec* hit, DrawableGridNode** cell, int a, int b)
{
    return reinterpret_cast<Terrain*>(this)->CastSegment(
        reinterpret_cast<const TerrainVec3*>(from), reinterpret_cast<const TerrainVec3*>(to),
        reinterpret_cast<TerrainVec3*>(hit), reinterpret_cast<GridNode**>(cell), (int*)a, (int*)b);
}
