// Group0_ObjectPlacement.cpp -- bring-up body for the placement query
// FindObjectPlacement (0x004b0df0), declared in contact/ObjectPlacement.h.
// Follows samples/physics/contact/ObjectPlacement.cpp, whose calls, branches
// and offsets are decoded from the retail bytes; the sample's provisional
// call views are replaced by the defined functions at the same addresses:
//   0x004b08f0 PlacementSphereBlocked, 0x004b0ac0 PointInPlacementSquare,
//   0x004b0b80 ClampToPlacementSquare (src/reconstructed/PlacementQueries.cpp),
//   0x00507c10 Terrain::QueryGround.
// Byte matching was not attempted.
#include "contact/ObjectPlacement.h"
#include "collision/CollisionObject.h"
#include "DebugAlloc.h"

static const char kObjectPlacementFile[] = "D:\\aardvark\\VC\\krusty2\\ObjectPlacement.cpp";

class PlacementProbe;
struct TerrainVec3;

// The terrain ground query (broadphase/Terrain.h, which cannot be included
// beside the collision headers): snaps pos->y to the surface, writes the
// surface normal and the cell's surface byte.
class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

// src/reconstructed/PlacementQueries.cpp (local declarations there; the
// CollisionVec3 is three floats, as Vec3).
int PlacementSphereBlocked(CollisionVec3* point, float radius, CollisionObject* self, int ownerA,
                           int ownerB, int testObjects, int testVegetation);       // 0x004b08f0
int PointInPlacementSquare(const CollisionVec3* point, float x, float z, float size, float scale,
                           float* minX, float* maxX, float* minZ, float* maxZ);    // 0x004b0ac0
void ClampToPlacementSquare(CollisionObject* object, PlacementProbe* probe,
                            const CollisionVec3* center, float scale, int clamp, int unused6,
                            int unused7, const CollisionVec3* target, CollisionVec3* position,
                            CollisionVec3* offset);                                  // 0x004b0b80

// The unit's axis globals (0x00688738..0x00688770, Math3D.h's per-unit set).
static const Vec3 kPlacementZero(0.0f, 0.0f, 0.0f);
static const Vec3 kPlacementAxisX(1.0f, 0.0f, 0.0f);
static const Vec3 kPlacementAxisY(0.0f, 1.0f, 0.0f);
static const Vec3 kPlacementAxisZ(0.0f, 0.0f, 1.0f);

static inline void PlacementNormalizeInPlace(Vec3* v)
{
    float lenSq = v->z * v->z + (v->x * v->x + v->y * v->y);
    if (lenSq == 0.0f)
        *v = kPlacementZero;
    else
        *v *= FastInvSqrt(lenSq);
}

static inline Vec3 PlacementNormalize(const Vec3& v)
{
    float lenSq = DotProduct(v, v);
    if (lenSq == 1.0f)
        return v;
    return v * FastInvSqrt(lenSq);
}

static inline void QueryGround(SoultreeProbe* terrain, Vec3* point, Vec3* normal,
                               unsigned char* surface)
{
    reinterpret_cast<Terrain*>(terrain)->QueryGround(reinterpret_cast<TerrainVec3*>(point),
                                                     reinterpret_cast<TerrainVec3*>(normal), 1,
                                                     surface);
}

// One probe of the cross: blocked (1) when outside the placement square (only
// with useAreaTest) or touching another object, otherwise the ground hit and
// its surface byte ANDed with groundMask.
static inline void ProbePoint(Vec3* point, Vec3* normal, unsigned char* hit,
                              CollisionObject* body, SoultreeProbe* terrain, float terrainScale,
                              int useAreaTest, int limit1, int limit2, float margin,
                              unsigned char groundMask)
{
    if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)point, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
        *hit = 1;
    else if (PlacementSphereBlocked((CollisionVec3*)point, margin, body, limit1, limit2, 1, 0))
        *hit = 1;
    else {
        QueryGround(terrain, point, normal, hit);
        *hit &= groundMask;
    }
}

// 0x004b0df0. Probes the centre and four points at +/-radius along z and x
// around `position`, growing the radius by radiusStep while all five are
// blocked. On the first pass (allowObjectTest, outKeepPlacement given, centre
// clear) a temporary collision object sweeps down through the position; a
// contact sets the outputs directly. Otherwise the first clear probe (and,
// with refinePair, the two highest clear ones) give outPosition/outForward and
// ClampToPlacementSquare finishes.
// best guess: retail returns whatever eax holds on the paths that end without
// the tail call (and the tail call is void); this returns 0 there and after it.
// Its callers only pass the value through.
int FindObjectPlacement(CollisionObject* body, SoultreeProbe* terrain, Vec3* position,
                        float terrainScale, int useAreaTest, int limit1, int limit2,
                        int allowObjectTest, float radiusStep, unsigned char groundMask,
                        const Vec3* referenceDir, const Vec3* targetPoint, int refinePair,
                        Vec3* outPosition, Vec3* outForward, Vec3* outUp, int* outKeepPlacement)
{
    float radius = radiusStep;
    float margin = 2.0f;
    if (useAreaTest == 0)
        margin = 4.0f;
    if (outKeepPlacement)
        *outKeepPlacement = 0;

    int canPlace = 1;
    int firstPass = 1;
    unsigned char centerHit = 0, plusAHit = 0, minusAHit = 0, plusCHit = 0, minusCHit = 0;
    Vec3 centerPt, plusAPt, minusAPt, plusCPt, minusCPt;
    Vec3 centerOut, plusAOut, minusAOut, plusCOut, minusCOut;

    for (;;) {
        Vec3 plusAOff = kPlacementAxisZ * radius;
        Vec3 minusAOff = kPlacementAxisZ * -radius;
        Vec3 plusCOff = kPlacementAxisX * radius;
        Vec3 minusCOff = kPlacementAxisX * -radius;

        if (firstPass) {
            centerPt = *position;
            QueryGround(terrain, &centerPt, &centerOut, &centerHit);
            centerHit &= groundMask;
        }

        if (allowObjectTest && canPlace) {
            canPlace = 0;
            if (outKeepPlacement && !centerHit) {
                CollisionObject* obj = new (kObjectPlacementFile, 0x172) CollisionObject(1);
                obj->Configure(0, 1, 1, 0);
                obj->ignoreVegetation = 1;
                obj->SetIgnoreListMode(1);
                obj->AddIgnoredOwner(body);

                // Two-point vertical sweep shape: (0, 1, 0) and (0, -1, 0).
                Vec3 sweep[2];
                sweep[0] = kPlacementZero;
                sweep[1] = kPlacementZero;
                sweep[0].y = 1.0f;
                sweep[1].y = -1.0f;
                obj->SetMeshShape(1, sweep);

                // Orthonormal basis from the global axes (identity in effect),
                // placed at position.
                Vec3 forward = kPlacementAxisZ;
                Vec3 side = CrossProduct(kPlacementAxisY, kPlacementAxisZ);
                Vec3 up = PlacementNormalize(CrossProduct(forward, side));
                forward = PlacementNormalize(forward);
                side = CrossProduct(up, forward);

                Matrix4 xf;
                xf._11 = side.x;    xf._12 = side.y;    xf._13 = side.z;    xf._14 = 0.0f;
                xf._21 = up.x;      xf._22 = up.y;      xf._23 = up.z;      xf._24 = 0.0f;
                xf._31 = forward.x; xf._32 = forward.y; xf._33 = forward.z; xf._34 = 0.0f;
                xf._41 = position->x; xf._42 = position->y; xf._43 = position->z; xf._44 = 1.0f;
                obj->SetTransform(&xf);
                obj->QueryCollisions();

                if (obj->hasContact != 0) {
                    // Contact record: [0] the hit fraction, [2..4] the contact normal.
                    float* rec = (float*)obj->contactRecord;
                    position->y += (1.0f - rec[0]) * 2.0f - 1.0f;
                    *outPosition = *position;
                    if (targetPoint) {
                        *outForward = *targetPoint - *position;
                        PlacementNormalizeInPlace(outForward);
                    }
                    if (outUp)
                        *outUp = *(Vec3*)(rec + 2);
                    *outKeepPlacement = 1;
                    obj->Release();
                    return 0;
                }
                obj->Release();
            }
        }

        if (firstPass) {
            if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)position, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
                centerHit = 1;
            else if (PlacementSphereBlocked((CollisionVec3*)&centerPt, margin, body, limit1, limit2, 1, 0))
                centerHit = 1;
            firstPass = 0;
        }

        plusAPt = *position + plusAOff;
        ProbePoint(&plusAPt, &plusAOut, &plusAHit, body, terrain, terrainScale, useAreaTest,
                   limit1, limit2, margin, groundMask);
        minusAPt = *position + minusAOff;
        ProbePoint(&minusAPt, &minusAOut, &minusAHit, body, terrain, terrainScale, useAreaTest,
                   limit1, limit2, margin, groundMask);
        plusCPt = *position + plusCOff;
        ProbePoint(&plusCPt, &plusCOut, &plusCHit, body, terrain, terrainScale, useAreaTest,
                   limit1, limit2, margin, groundMask);
        minusCPt = *position + minusCOff;
        ProbePoint(&minusCPt, &minusCOut, &minusCHit, body, terrain, terrainScale, useAreaTest,
                   limit1, limit2, margin, groundMask);

        if (centerHit && plusAHit && minusAHit && plusCHit && minusCHit)
            radius += radiusStep;
        else
            break;
    }

    // The first clear probe (flag 0) and the next one when it sits lower.
    const Vec3* bestOut = &minusCOut;
    const Vec3* bestPt = &minusCPt;
    const Vec3* nextOut = &minusCOut;
    const Vec3* nextPt = 0;
    if (!centerHit) {
        bestOut = &centerOut;  bestPt = &centerPt;
        if (!plusAHit && plusAOut.y < centerOut.y) { nextOut = &plusAOut; nextPt = &plusAPt; }
        else { nextOut = &centerOut; nextPt = 0; }
    } else if (!plusAHit) {
        bestOut = &plusAOut;  bestPt = &plusAPt;
        if (!minusAHit && minusAOut.y < plusAOut.y) { nextOut = &minusAOut; nextPt = &minusAPt; }
        else { nextOut = &plusAOut; nextPt = 0; }
    } else if (!minusAHit) {
        bestOut = &minusAOut;  bestPt = &minusAPt;
        if (!plusCHit && plusCOut.y < minusAOut.y) { nextOut = &plusCOut; nextPt = &plusCPt; }
        else { nextOut = &minusAOut; nextPt = 0; }
    } else if (!plusCHit) {
        bestOut = &plusCOut;  bestPt = &plusCPt;
        if (!minusCHit && minusCOut.y < plusCOut.y) { nextOut = &minusCOut; nextPt = &minusCPt; }
        else { nextOut = &plusCOut; nextPt = 0; }
    }

    if (refinePair) {
        // Keep the two highest clear hits (by y) among -A, +C and -C.
        if (!minusAHit) {
            if (!(minusAOut.y <= bestOut->y)) { nextOut = bestOut; nextPt = bestPt; bestOut = &minusAOut; bestPt = &minusAPt; }
            else if (!(minusAOut.y <= nextOut->y)) { nextOut = &minusAOut; nextPt = &minusAPt; }
        }
        if (!plusCHit) {
            if (!(plusCOut.y <= bestOut->y)) { nextOut = bestOut; nextPt = bestPt; bestOut = &plusCOut; bestPt = &plusCPt; }
            else if (!(plusCOut.y <= nextOut->y)) { nextOut = &plusCOut; nextPt = &plusCPt; }
        }
        if (!minusCHit) {
            if (!(minusCOut.y <= bestOut->y)) { nextOut = bestOut; nextPt = bestPt; bestOut = &minusCOut; bestPt = &minusCPt; }
            else if (!(minusCOut.y <= nextOut->y)) { nextOut = &minusCOut; nextPt = &minusCPt; }
        }
    }

    *outPosition = *bestPt;

    if (referenceDir) {
        if (!outForward)
            return 0;
        if (refinePair && nextPt) {
            // Horizontal direction between the two points, turned to face referenceDir.
            Vec3 w = *nextPt - *bestPt;
            w.y = 0.0f;
            if (w.x * referenceDir->x + w.z * referenceDir->z < 0.0f)
                *outForward = Vec3(-w.x, 0.0f, -w.z);
            else
                *outForward = w;
            PlacementNormalizeInPlace(outForward);
        } else if (useAreaTest == 0) {
            *outForward = *referenceDir;
        }
    } else if (targetPoint) {
        *outForward = *targetPoint - *bestPt;
        PlacementNormalizeInPlace(outForward);
    } else {
        return 0;
    }
    ClampToPlacementSquare(body, (PlacementProbe*)terrain, (CollisionVec3*)position, terrainScale,
                           useAreaTest, limit1, limit2, (const CollisionVec3*)targetPoint,
                           (CollisionVec3*)outPosition, (CollisionVec3*)outForward);
    return 0;
}
