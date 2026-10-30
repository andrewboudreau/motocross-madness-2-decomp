// Group6_TerrainShadow.cpp -- bring-up bodies for the call views that
// samples/physics/shadow/TerrainShadow.cpp (admitted best-guess candidate) declares but
// nothing defines.  None of this is a byte-match claim; it exists so the source-built game
// links.  Where the retail VA already has a definition under its owner's name, the view
// forwards to it:
//   0x00484d70 TerrainShutdownObject::LookupCell -> written out (GridNode::FindLeaf has no
//              src/ definition; the body is samples/render/GridNodeNearMisses.cpp's)
//   0x004a1410 TerrainMatrixIdentityInto  -> ShadowMatrixIdentity (-> IdentityMatrix)
//   0x004a1860 TerrainMatrixMultiplyInto  -> ShadowMatrixMultiply (-> MatrixMult)
//   0x00421cb0 TerrainShadowSum           -> Vec3AddCall
//   0x00421d00 TerrainShadowDifference    -> Vec3SubtractCall
//   0x00404e60 TerrainShadowVecCall ctor  -> written out (Vec3::Vec3 is inline everywhere)
//   0x0052f190 TerrainShadowClipper::ProjectVertices -> VisibilityClipper::ProjectVertices
//   0x00506e90 TerrainShadowCaster::CastSegment      -> Terrain::CastSegment
//   0x00575a98 g_terrainShadowClipper     -> the same object as g_visibilityClipper
#include "broadphase/Terrain.h"
#include "shadow/ProjectedShadow.h"
#include "visibility/VisibilityClipper.h"

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
                    TerrainCell** cell, int a, int b);                             // 0x00506e90
};
class TerrainShadowClipper {
public:
    void ProjectVertices(const void* camera, const void* matrix, int count,
                         const TerrainShadowVec* vertices, TerrainShadowVec* screen, int* codes);
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
// 0x00484d70 (thiscall, ret 8): the heightField is the root GridNode (src/reconstructed/
// Griddraw.h); children at +0x04 (16 x 16 table), shift at +0x29, grid position at +0x24/+0x26.
// The row table at 0x0056c2ac is g_gridRow16 (row * 16), written here as the multiply.
// best guess: GridNodeNearMisses.cpp's FindLeaf body (81/90, same instructions) over a local view.
struct Group6TerrainGridNodeView {
    void* vtable;                                   // +0x00
    Group6TerrainGridNodeView** children;           // +0x04
    char pad_0x08[0x24 - 0x08];
    unsigned short gridX;                           // +0x24
    unsigned short gridZ;                           // +0x26
    unsigned char level;                            // +0x28
    unsigned char shift;                            // +0x29
};

TerrainCell* TerrainShutdownObject::LookupCell(int x, int z)
{
    Group6TerrainGridNodeView* node = reinterpret_cast<Group6TerrainGridNodeView*>(this);
    Group6TerrainGridNodeView** table = node->children;
    while (table) {
        node = table[((z >> node->shift) & 0xf) * 16 + ((x >> node->shift) & 0xf)];
        if (!node)
            return 0;
        table = node->children;
    }
    if (node->gridX != x || node->gridZ != z)
        return 0;
    return reinterpret_cast<TerrainCell*>(node);
}

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
// 0x0052f190 (thiscall): VisibilityClipper::ProjectVertices.  The sample passes the viewer's
// camera and camera + 0xec (the same arguments VisibilityQuadTree.cpp passes) and count 1, so
// the 0x20-byte VisibilityVertex stride never applies to the 12-byte TerrainShadowVec.
// best guess: forwarding unchanged to the canonical method at the same VA.
void TerrainShadowClipper::ProjectVertices(const void* camera, const void* matrix, int count,
                                           const TerrainShadowVec* vertices,
                                           TerrainShadowVec* screen, int* codes)
{
    reinterpret_cast<VisibilityClipper*>(this)->ProjectVertices(
        reinterpret_cast<const VisibilityCamera*>(camera),
        reinterpret_cast<const VisibilityMatrix*>(matrix), count,
        reinterpret_cast<const VisibilityVertex*>(vertices),
        reinterpret_cast<VisibilityBoxVec*>(screen), codes);
}

// 0x00575a98: the clipper pointer VisibilityClipperGlobal.h names g_visibilityClipper
// (storage: src/bringup/Group1_Globals.cpp).  None of the clipper's methods reads its fields.
// best guess: alias the same object.  This is a dynamic initializer, but g_visibilityClipper is
// statically initialized (the address of a static), so it is set before this runs.
TerrainShadowClipper* g_terrainShadowClipper = reinterpret_cast<TerrainShadowClipper*>(g_visibilityClipper);

// ---------------------------------------------------------------------------------------------
// 0x00506e90 (thiscall, ret 0x18): Terrain::CastSegment.  The sample's caster is the
// TerrainShadow's Terrain; its fourth argument (the hit cell out-pointer) is Terrain's first
// forwarded int, which reaches GridNode slot 1's outNode.
// best guess: forwarding to the canonical method at the same VA.
int TerrainShadowCaster::CastSegment(const TerrainShadowVec* from, const TerrainShadowVec* to,
                                     TerrainShadowVec* hit, TerrainCell** cell, int a, int b)
{
    return reinterpret_cast<Terrain*>(this)->CastSegment(
        reinterpret_cast<const TerrainVec3*>(from), reinterpret_cast<const TerrainVec3*>(to),
        reinterpret_cast<TerrainVec3*>(hit), reinterpret_cast<int>(cell), a, b);
}
