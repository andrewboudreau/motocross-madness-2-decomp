// ObjectPlacement.cpp -- D:\aardvark\VC\krusty2\ObjectPlacement.cpp (literal __FILE__,
// xref 0x004b0fca inside 0x004b0df0).  The unit is 0x004b0df0..0x004b1ebb: the placement
// query 0x004b0df0 (ObjectPlacement.h) followed by the Math3D.h vector set, .CRT$XCU
// 208-211 (0x004b1d80..0x004b1ebb, globals 0x00688738..0x00688770).  The query reads
// that set and is the only reader, which closes the unit (tier 2).
//
// The vector set comes from math/Math3D.h through ObjectPlacement.h; the query is defined
// below.  Its semantics and parameter roles are tier 3; every call, branch and offset is
// decoded from the target bytes (tier 1).
//
//  * Probes a cross of five points around `position`: the centre and four points at
//    +/-radius along the axes 0x688738 (0,0,1) and 0x688758 (1,0,0).  The radius starts
//    at radiusStep and grows by it while all five probes are blocked.
//  * A probe is flagged 1 when 0x4b0ac0 (only with useAreaTest) or 0x4b08f0 report it
//    blocked, otherwise 0x507c10 on `terrain` fills a ground hit and the flag byte, ANDed
//    with groundMask.
//  * On the first pass, with allowObjectTest set, outKeepPlacement non-null and the centre
//    clear, it creates a temporary CollisionObject with a two-point vertical sweep shape,
//    orients it with a basis built from the axes and runs its collision query (0x438e70).
//    A contact (hasContact != 0) sets all four outputs (outKeepPlacement = 1) and the
//    object is released.
//  * After the loop the first clear probes are selected, refined when refinePair is set,
//    written to outPosition / outForward, and 0x4b0b80 finishes.
//
// Parameter roles from the call sites (SoulTreePhysics slot 11 0x501da0 and the KrustyBike
// overrides 0x4920bb/0x4921b9), tier 3 beyond the decoded data flow: body (the caller's
// collision object), terrain (thiscall this of 0x507c10), position, terrainScale
// (forwarded to 0x4b0ac0), useAreaTest (0 => margin 4.0f and outForward = referenceDir,
// else 2.0f), limit1/limit2 (0x7fffffff or 0x64/0x65, forwarded to 0x4b08f0),
// allowObjectTest, radiusStep, groundMask (AND mask for the probe results),
// referenceDir/targetPoint (nullable), refinePair, and the outputs outPosition,
// outForward, outUp (the contact normal) and outKeepPlacement, which the step loops hand
// to slot 33 as newPosition, newForward, settleUp and keepPlacement.
#include "contact/ObjectPlacement.h"
#include "collision/CollisionObject.h"
#include "core/DebugAlloc.h"

// The terrain ground query (broadphase/Terrain.h, which cannot be included beside the
// collision headers): 0x00507c10, thiscall, ret 0x10.
struct TerrainVec3;
class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

// The placement area queries in src/reconstructed/PlacementQueries.cpp, which has no
// header (it declares its own partial CollisionObject view).  CollisionVec3 is three
// floats, as Vec3.
class PlacementProbe;
int PlacementSphereBlocked(CollisionVec3* point, float radius, CollisionObject* self, int ownerA,
                           int ownerB, int testObjects, int testVegetation);       // 0x004b08f0
int PointInPlacementSquare(const CollisionVec3* point, float x, float z, float size, float scale,
                           float* minX, float* maxX, float* minZ, float* maxZ);    // 0x004b0ac0
void ClampToPlacementSquare(CollisionObject* object, PlacementProbe* probe,
                            const CollisionVec3* center, float scale, int clamp, int unused6,
                            int unused7, const CollisionVec3* target, CollisionVec3* position,
                            CollisionVec3* offset);                                  // 0x004b0b80

// d3dvec.inl's CrossProduct spelling: the components of a and b are read through
// operator[].  The shared Math3D.h CrossProduct (member names) compiles to the same
// instructions, but with the axis globals VC6 then loads Z.z and Z.y before Y.x in the y
// and z components; through operator[] it loads Y.x first, as retail does at 0x4b10dd and
// 0x4b1110.  Giving Math3D.h this spelling loses five exact physics functions
// (0x4cc230, 0x4cc3f0, 0x424ab0, 0x4278d0, 0x405db0), so the spelling stays local.
static inline Vec3 PlacementCross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = (a[1] * b[2]) - (a[2] * b[1]);
    r.y = (a[2] * b[0]) - (a[0] * b[2]);
    r.z = (a[0] * b[1]) - (a[1] * b[0]);
    return r;
}

static inline void PlacementNormalizeInPlace(Vec3* v)
{
    float lenSq = v->z * v->z + (v->x * v->x + v->y * v->y);
    if (lenSq == 0.0f)
        *v = kVec3Zero;
    else
        *v *= FastInvSqrt(lenSq);
}

// v scaled by 1/sqrt(lenSq) through 0x005015b0, or v itself when lenSq is exactly 1.
static inline Vec3 PlacementRescale(const Vec3& v, float lenSq)
{
    if (lenSq == 1.0f)
        return v;
    float inv = FastInvSqrt(lenSq);
    Vec3 t;
    return *Vec3ScaleCall(&t, &v, inv);
}

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

    Vec3 w;   // the referenceDir/outForward branch below; declared here, VC6 schedules its fchs as retail does
    int canPlace = 1;
    int firstPass = 1;
    unsigned char centerHit, plusAHit, minusAHit, plusCHit, minusCHit;
    Vec3 centerPt, plusAPt, minusAPt, plusCPt, minusCPt;
    Vec3 centerOut, plusAOut, minusAOut, plusCOut, minusCOut;

    for (;;) {
        Vec3 plusAOff = kVec3ZAxis * radius;
        Vec3 minusAOff = kVec3ZAxis * -radius;
        Vec3 plusCOff = kVec3XAxis * radius;
        Vec3 minusCOff = kVec3XAxis * -radius;

        if (firstPass) {
            centerPt = *position;
            ((Terrain*)terrain)->QueryGround((TerrainVec3*)&centerPt, (TerrainVec3*)&centerOut, 1, &centerHit);
            centerHit &= groundMask;
        }

        if (allowObjectTest && canPlace) {
            canPlace = 0;
            if (outKeepPlacement && !centerHit) {
                CollisionObject* obj = new(__FILE__, 0x172) CollisionObject(1);
                obj->Configure(0, 1, 1, 0);
                obj->ignoreVegetation = 1;
                obj->SetIgnoreListMode(1);
                obj->AddIgnoredOwner(body);

                // two-point vertical sweep: (0,1,0) and (0,-1,0)
                Vec3 sweep[2];
                sweep[0] = kVec3Zero;
                sweep[1] = kVec3Zero;
                sweep[0].y = 1.0f;
                sweep[1].y = -1.0f;
                obj->SetMeshShape(1, sweep);

                // orthonormal basis from the axes: side = up x forward, up = forward x side
                // (normalised), side = up x forward.  The first up value is never read;
                // retail still stores it (0x4b10fa..0x4b112f).
                Vec3 forward = kVec3ZAxis;
                Vec3 up = kVec3YAxis;
                Vec3 side = PlacementCross(kVec3YAxis, kVec3ZAxis);
                up = CrossProductCall(forward, side);
                float lenSq = Vec3DotCall(&up, &up);
                up = PlacementRescale(up, lenSq);
                forward = Vec3Normalize(forward);
                side = CrossProductCall(up, forward);

                Matrix4 xf;
                xf._11 = side.x;    xf._12 = side.y;    xf._13 = side.z;   xf._14 = 0.0f;
                xf._21 = up.x;      xf._22 = up.y;      xf._23 = up.z;      xf._24 = 0.0f;
                xf._31 = forward.x; xf._32 = forward.y; xf._33 = forward.z; xf._34 = 0.0f;
                xf._41 = position->x; xf._42 = position->y; xf._43 = position->z; xf._44 = 1.0f;
                obj->SetTransform(&xf);
                obj->QueryCollisions();

                if (obj->hasContact != 0) {
                    // contact: rec[0] = fraction, rec + 8 = contact normal (tier 3)
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
                    // Retail jumps to the epilogue with the slot-2 call's eax, so the
                    // release's result is the return value here.
                    return obj->Release();
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
        if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)&plusAPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            plusAHit = 1;
        else if (PlacementSphereBlocked((CollisionVec3*)&plusAPt, margin, body, limit1, limit2, 1, 0))
            plusAHit = 1;
        else {
            ((Terrain*)terrain)->QueryGround((TerrainVec3*)&plusAPt, (TerrainVec3*)&plusAOut, 1, &plusAHit);
            plusAHit &= groundMask;
        }

        minusAPt = *position + minusAOff;
        if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)&minusAPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            minusAHit = 1;
        else if (PlacementSphereBlocked((CollisionVec3*)&minusAPt, margin, body, limit1, limit2, 1, 0))
            minusAHit = 1;
        else {
            ((Terrain*)terrain)->QueryGround((TerrainVec3*)&minusAPt, (TerrainVec3*)&minusAOut, 1, &minusAHit);
            minusAHit &= groundMask;
        }

        plusCPt = *position + plusCOff;
        if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)&plusCPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            plusCHit = 1;
        else if (PlacementSphereBlocked((CollisionVec3*)&plusCPt, margin, body, limit1, limit2, 1, 0))
            plusCHit = 1;
        else {
            ((Terrain*)terrain)->QueryGround((TerrainVec3*)&plusCPt, (TerrainVec3*)&plusCOut, 1, &plusCHit);
            plusCHit &= groundMask;
        }

        minusCPt = *position + minusCOff;
        if (useAreaTest && !PointInPlacementSquare((CollisionVec3*)&minusCPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            minusCHit = 1;
        else if (PlacementSphereBlocked((CollisionVec3*)&minusCPt, margin, body, limit1, limit2, 1, 0))
            minusCHit = 1;
        else {
            ((Terrain*)terrain)->QueryGround((TerrainVec3*)&minusCPt, (TerrainVec3*)&minusCOut, 1, &minusCHit);
            minusCHit &= groundMask;
        }

        if (centerHit && plusAHit && minusAHit && plusCHit && minusCHit)
            radius += radiusStep;
        else
            break;
    }

    // Two "worst" (first failing) probes: best* and next*, each an (out, point) pair.
    // A flag of 0 means the probe failed.  In retail the all-clear else branch leaves the
    // pointers uninitialised (unreachable: the loop only exits when a flag is 0).
    const Vec3* bestOut;
    const Vec3* bestPt;
    const Vec3* nextOut;
    const Vec3* nextPt;
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
    } else if (!minusCHit) {
        bestOut = &minusCOut;
        bestPt = &minusCPt;
        nextOut = &minusCOut;
        nextPt = 0;
    }

    if (refinePair) {
        // keep the two highest failing hits (by out.y) among -A, +C, -C
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
        if (outForward) {
            if (refinePair && nextPt) {
                // horizontal direction between the two worst points, flipped to face referenceDir
                w = *nextPt - *bestPt;
                w.y = 0.0f;
                if (w.x * referenceDir->x + w.z * referenceDir->z < 0.0f) {
                    *outForward = Vec3(-w.x, 0.0f, -w.z);
                } else {
                    *outForward = w;
                }
                PlacementNormalizeInPlace(outForward);
            } else if (useAreaTest == 0) {
                *outForward = *referenceDir;
            }
            ClampToPlacementSquare(body, (PlacementProbe*)terrain, (CollisionVec3*)position,
                                   terrainScale, useAreaTest, limit1, limit2,
                                   (const CollisionVec3*)targetPoint, (CollisionVec3*)outPosition,
                                   (CollisionVec3*)outForward);
        }
    } else if (targetPoint) {
        *outForward = *targetPoint - *bestPt;
        PlacementNormalizeInPlace(outForward);
        ClampToPlacementSquare(body, (PlacementProbe*)terrain, (CollisionVec3*)position,
                               terrainScale, useAreaTest, limit1, limit2,
                               (const CollisionVec3*)targetPoint, (CollisionVec3*)outPosition,
                               (CollisionVec3*)outForward);
    }
    // 0x4b0b80 returns nothing, and retail returns whatever eax holds after it, as on the
    // referenceDir-without-outForward and no-targetPoint paths (VC6 warns C4715).
}
