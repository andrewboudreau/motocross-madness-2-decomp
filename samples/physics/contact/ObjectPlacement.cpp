// ObjectPlacement.cpp (translation unit tier 3: the operator new at 0x4b0fc9 passes the
// __FILE__ string "D:\aardvark\VC\krusty2\ObjectPlacement.cpp", line 0x172).
//
// 0x004b0df0, 3971 bytes, SEH frame, cdecl, 17 stack args (callers: SoulTreePhysics slot
// 11 0x00501dfe and the KrustyBike overrides 0x4920bb/0x4921b9).  Semantics are tier 3,
// every call, branch and offset below is decoded from the target bytes (tier 1).
//
//  * Probes a cross of five points around `position`: the centre and four points
//    at +/-radius along the global axes 0x688738 (0,0,1) and 0x688758 (1,0,0).  The
//    radius starts at radiusStep and grows by it while all five probes are clear.
//  * Each probe is flagged 1 when 0x4b0ac0 (only with useAreaTest) or 0x4b08f0 report it
//    blocked, otherwise 0x507c10 on `terrain` fills a ground hit and the flag byte, ANDed
//    with groundMask.
//  * On the first pass, with allowObjectTest set, once, outKeepPlacement non-null and the
//    centre not clear, it creates a temporary CollisionObject with a two-point vertical
//    sweep shape, orients it with a basis built from the axes, and runs its collision test
//    (0x438e70).  A resulting contact (field_0x58 != 0) sets all four outputs (with
//    outKeepPlacement = 1) and the object is released.
//  * After the loop the two worst failing probes are selected, refined when refinePair is
//    set, written to outPosition / outForward, and the result of the tail call 0x4b0b80 is
//    returned.
//
// Parameter roles from the call sites (SoulTreePhysics slot 11 0x501da0, KrustyBike), tier 3
// beyond the decoded data flow: body (the caller's collisionObject), terrain (thiscall this
// of 0x507c10), position, terrainScale (forwarded to 0x4b0ac0), useAreaTest (0 => margin
// 4.0f and outForward = referenceDir, else 2.0f), limit1/limit2 (0x7fffffff or 0x64/0x65,
// forwarded to 0x4b08f0), allowObjectTest, radiusStep, groundMask (AND mask for probe
// results), referenceDir/targetPoint (nullable), refinePair, and the outputs outPosition,
// outForward, outUp (the contact normal) and outKeepPlacement, which the step loops hand to
// slot 33 as newPosition, newForward, settleUp and keepPlacement.  (targetPoint is a Vec3*:
// 0x4b17f1 and 0x4b1c72 dereference it.)
#include "ObjectPlacementTypes.h"

// Runtime-initialised globals.  The initialisers are at 0x4b1d80 (zero), 0x4b1dd0 (X),
// 0x4b1e20 (Y) and 0x4b1e70 (Z), immediately after 0x4b0df0 (tier 3 for TU ownership;
// the values are decoded stores of 0.0f/1.0f, tier 1).
Vec3 g_PlacementZero(0.0f, 0.0f, 0.0f);   // 0x00688748
Vec3 g_PlacementAxisX(1.0f, 0.0f, 0.0f);  // 0x00688758
Vec3 g_PlacementAxisY(0.0f, 1.0f, 0.0f);  // 0x00688768
Vec3 g_PlacementAxisZ(0.0f, 0.0f, 1.0f);  // 0x00688738

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
        *v = g_PlacementZero;
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
        Vec3 plusAOff = g_PlacementAxisZ * radius;
        Vec3 minusAOff = g_PlacementAxisZ * -radius;
        Vec3 plusCOff = g_PlacementAxisX * radius;
        Vec3 minusCOff = g_PlacementAxisX * -radius;

        if (firstPass) {
            centerPt = *position;
            ((PlacementProbe*)terrain)->QueryGround(&centerPt, &centerOut, 1, &centerHit);
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
                sweep[0] = g_PlacementZero;
                sweep[1] = g_PlacementZero;
                sweep[0].y = 1.0f;
                sweep[1].y = -1.0f;
                obj->SetMeshShape(1, sweep);

                // orthonormal basis from the axes: side = up x forward, up = forward x side
                // (normalised), side = up x forward.  The first up value is never read;
                // retail still stores it (0x4b10fa..0x4b112f).
                Vec3 forward = g_PlacementAxisZ;
                Vec3 up = g_PlacementAxisY;
                Vec3 side = PlacementCross(g_PlacementAxisY, g_PlacementAxisZ);
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
                    obj->Release();
                    // Retail returns no value here: it jumps to the epilogue with the slot-2
                    // call's eax, as do the referenceDir-without-outForward and no-targetPoint paths below.
                    goto done;
                }
                obj->Release();
            }
        }

        if (firstPass) {
            if (useAreaTest && !Fn_4b0ac0(position, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
                centerHit = 1;
            else if (Fn_4b08f0(&centerPt, margin, body, limit1, limit2, 1, 0))
                centerHit = 1;
            firstPass = 0;
        }

        plusAPt = *position + plusAOff;
        if (useAreaTest && !Fn_4b0ac0(&plusAPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            plusAHit = 1;
        else if (Fn_4b08f0(&plusAPt, margin, body, limit1, limit2, 1, 0))
            plusAHit = 1;
        else {
            ((PlacementProbe*)terrain)->QueryGround(&plusAPt, &plusAOut, 1, &plusAHit);
            plusAHit &= groundMask;
        }

        minusAPt = *position + minusAOff;
        if (useAreaTest && !Fn_4b0ac0(&minusAPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            minusAHit = 1;
        else if (Fn_4b08f0(&minusAPt, margin, body, limit1, limit2, 1, 0))
            minusAHit = 1;
        else {
            ((PlacementProbe*)terrain)->QueryGround(&minusAPt, &minusAOut, 1, &minusAHit);
            minusAHit &= groundMask;
        }

        plusCPt = *position + plusCOff;
        if (useAreaTest && !Fn_4b0ac0(&plusCPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            plusCHit = 1;
        else if (Fn_4b08f0(&plusCPt, margin, body, limit1, limit2, 1, 0))
            plusCHit = 1;
        else {
            ((PlacementProbe*)terrain)->QueryGround(&plusCPt, &plusCOut, 1, &plusCHit);
            plusCHit &= groundMask;
        }

        minusCPt = *position + minusCOff;
        if (useAreaTest && !Fn_4b0ac0(&minusCPt, 1.0f, 1.0f, 1.0f, terrainScale, 0, 0, 0, 0))
            minusCHit = 1;
        else if (Fn_4b08f0(&minusCPt, margin, body, limit1, limit2, 1, 0))
            minusCHit = 1;
        else {
            ((PlacementProbe*)terrain)->QueryGround(&minusCPt, &minusCOut, 1, &minusCHit);
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
            return Fn_4b0b80(body, terrain, position, terrainScale, useAreaTest, limit1, limit2, targetPoint, outPosition, outForward);
        }
    } else if (targetPoint) {
        *outForward = *targetPoint - *bestPt;
        PlacementNormalizeInPlace(outForward);
        return Fn_4b0b80(body, terrain, position, terrainScale, useAreaTest, limit1, limit2, targetPoint, outPosition, outForward);
    }
    // No return statement on these paths (retail leaves eax unset; VC6 warns C4715).
done:
    ;
}
