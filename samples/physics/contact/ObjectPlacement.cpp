// ObjectPlacement.cpp (translation unit tier 3: the operator new at 0x4b0fc9 passes the
// __FILE__ string "D:\aardvark\VC\krusty2\ObjectPlacement.cpp", line 0x172).
//
// 0x004b0df0, 3971 bytes, SEH frame, cdecl, 17 stack args (callers: SoulTreePhysics slot
// 11 0x00501dfe and the KrustyBike overrides 0x4920bb/0x4921b9).  Semantics are tier 3,
// every call, branch and offset below is decoded from the target bytes (tier 1).
//
//  * Probes a cross of five points around the position `c`: the centre and four points
//    at +/-radius along the global axes 0x688738 (0,0,1) and 0x688758 (1,0,0).  The
//    radius starts at `i` and grows by `i` while all five probes are clear.
//  * Each probe is flagged 1 when 0x4b0ac0 (only if e != 0) or 0x4b08f0 report it
//    blocked, otherwise 0x507c10 on `b` fills a ground hit and the flag byte, ANDed
//    with `j`.
//  * On the first pass, with h set, once, q non-null and the centre not clear, it creates
//    a temporary CollisionObject with a two-point vertical sweep shape, orients it with
//    a basis built from the axes, and runs its collision test (0x438e70).  A resulting
//    contact (field_0x58 != 0) is turned into outputs n/o/p/q and the object released.
//  * After the loop the two worst failing probes are selected, refined when m is set,
//    written to n / o, and the result of the tail call 0x4b0b80 is returned.
//
// Parameter roles from the call sites (SoulTreePhysics slot 11 0x501da0, KrustyBike):
//   a body, b probe object (thiscall this of 0x507c10), c position, d float forwarded to
//   0x4b0ac0, e "use radius probe" (0 => margin 4.0f, else 2.0f), f/g 0x7fffffff limits
//   forwarded to 0x4b08f0, h "allow object test", i radius step, j AND mask for probe
//   results, k/l nullable Vec3* (k: reference direction, l: target point), m "pair mode",
//   n/o/p out Vec3*, q out flag.  (SoultreePhysicsCallees.h types l as int; it is a
//   Vec3*: 0x4b17f1 and 0x4b1c72 dereference it.)
#include "ObjectPlacementTypes.h"

// Runtime-initialised globals.  The initialisers are at 0x4b1d80 (zero), 0x4b1dd0 (X),
// 0x4b1e20 (Y) and 0x4b1e70 (Z), immediately after 0x4b0df0 (tier 3 for TU ownership;
// the values are decoded stores of 0.0f/1.0f, tier 1).
Vec3 g_PlacementZero(0.0f, 0.0f, 0.0f);   // 0x00688748
Vec3 g_PlacementAxisX(1.0f, 0.0f, 0.0f);  // 0x00688758
Vec3 g_PlacementAxisY(0.0f, 1.0f, 0.0f);  // 0x00688768
Vec3 g_PlacementAxisZ(0.0f, 0.0f, 1.0f);  // 0x00688738

static void PlacementNormalizeInPlace(Vec3* v)
{
    float lenSq = DotProduct(*v, *v);
    if (lenSq == 0.0f)
        *v = g_PlacementZero;
    else
        *v *= FastInvSqrt(lenSq);
}

int Fn_4b0df0(SoultreeBody* a, SoultreeProbe* b, Vec3* c, float d, int e, int f, int g, int h,
              float i, unsigned char j, const Vec3* k, const Vec3* l, int m, Vec3* n, Vec3* o,
              Vec3* p, int* q)
{
    float radius = i;
    float margin = 2.0f;
    if (e == 0)
        margin = 4.0f;
    if (q)
        *q = 0;

    int firstPass = 1;
    int canPlace = 1;
    unsigned char centerHit, plusAHit, minusAHit, plusCHit, minusCHit;
    Vec3 centerPt, plusAPt, minusAPt, plusCPt, minusCPt;
    Vec3 centerOut, plusAOut, minusAOut, plusCOut, minusCOut;

    for (;;) {
        Vec3 plusAOff = g_PlacementAxisZ * radius;
        Vec3 plusCOff = g_PlacementAxisX * radius;
        float negRadius = -radius;
        Vec3 minusAOff = g_PlacementAxisZ * negRadius;
        Vec3 minusCOff = g_PlacementAxisX * negRadius;

        if (firstPass) {
            centerPt = *c;
            ((PlacementProbe*)b)->Fn_507c10(&centerPt, &centerOut, 1, &centerHit);
            centerHit &= j;
        }

        if (h && canPlace) {
            canPlace = 0;
            if (q && !centerHit) {
                CollisionObject* obj = new(__FILE__, 0x172) CollisionObject(1);
                obj->Fn_004320f0(0, 1, 1, 0);
                obj->field_0x68 = 1;
                ((PlacementBodyOps*)obj)->SetField_0x74(1);
                ((PlacementBodyOps*)obj)->AddIgnoredOwner(a);

                // two-point vertical sweep: (0,1,0) and (0,-1,0)
                Vec3 sweep[2];
                sweep[0] = g_PlacementZero;
                sweep[1] = g_PlacementZero;
                sweep[0].y = 1.0f;
                sweep[1].y = -1.0f;
                obj->Fn_00432ab0(1, sweep);

                // orthonormal basis from the axes
                Vec3 forward = g_PlacementAxisZ;
                Vec3 upRef = g_PlacementAxisY;
                Vec3 side = CrossProduct(upRef, forward);
                Vec3 t = PlacementCross(forward, side);
                float tLenSq = PlacementDot(&t, &t);
                Vec3 up;
                if (tLenSq == 1.0f)
                    up = upRef;
                else
                    up = *SoultreeScaleVec3(&t, &upRef, FastInvSqrt(tLenSq));
                forward = Vec3Normalize(forward);
                Vec3 right = PlacementCross(up, forward);

                Matrix4 xf;
                xf._11 = right.x;   xf._12 = right.y;   xf._13 = right.z;   xf._14 = 0.0f;
                xf._21 = up.x;      xf._22 = up.y;      xf._23 = up.z;      xf._24 = 0.0f;
                xf._31 = forward.x; xf._32 = forward.y; xf._33 = forward.z; xf._34 = 0.0f;
                xf._41 = c->x;      xf._42 = c->y;      xf._43 = c->z;      xf._44 = 1.0f;
                ((PlacementBodyOps*)obj)->SetTransform(&xf);
                obj->Fn_00438e70();

                if (obj->field_0x58 != 0) {
                    // contact: rec[0] = fraction, rec + 8 = contact normal (tier 3)
                    float* rec = (float*)obj->field_0x5c;
                    c->y += (1.0f - rec[0]) * 2.0f - 1.0f;
                    *n = *c;
                    if (l) {
                        *o = *l - *c;
                        PlacementNormalizeInPlace(o);
                    }
                    if (p)
                        *p = *(Vec3*)(rec + 2);
                    *q = 1;
                    obj->BaseObjectVirtualSlot2();
                    return 0;   // TODO: retail leaves the slot-2 call's eax in place
                }
                obj->BaseObjectVirtualSlot2();
            }
        }

        if (firstPass) {
            if (e && !Fn_4b0ac0(c, 1.0f, 1.0f, 1.0f, d, 0, 0, 0, 0))
                centerHit = 1;
            else if (Fn_4b08f0(&centerPt, margin, a, f, g, 1, 0))
                centerHit = 1;
            firstPass = 0;
        }

        plusAPt = *c + plusAOff;
        if (e && !Fn_4b0ac0(&plusAPt, 1.0f, 1.0f, 1.0f, d, 0, 0, 0, 0))
            plusAHit = 1;
        else if (Fn_4b08f0(&plusAPt, margin, a, f, g, 1, 0))
            plusAHit = 1;
        else {
            ((PlacementProbe*)b)->Fn_507c10(&plusAPt, &plusAOut, 1, &plusAHit);
            plusAHit &= j;
        }

        minusAPt = *c + minusAOff;
        if (e && !Fn_4b0ac0(&minusAPt, 1.0f, 1.0f, 1.0f, d, 0, 0, 0, 0))
            minusAHit = 1;
        else if (Fn_4b08f0(&minusAPt, margin, a, f, g, 1, 0))
            minusAHit = 1;
        else {
            ((PlacementProbe*)b)->Fn_507c10(&minusAPt, &minusAOut, 1, &minusAHit);
            minusAHit &= j;
        }

        plusCPt = *c + plusCOff;
        if (e && !Fn_4b0ac0(&plusCPt, 1.0f, 1.0f, 1.0f, d, 0, 0, 0, 0))
            plusCHit = 1;
        else if (Fn_4b08f0(&plusCPt, margin, a, f, g, 1, 0))
            plusCHit = 1;
        else {
            ((PlacementProbe*)b)->Fn_507c10(&plusCPt, &plusCOut, 1, &plusCHit);
            plusCHit &= j;
        }

        minusCPt = *c + minusCOff;
        if (e && !Fn_4b0ac0(&minusCPt, 1.0f, 1.0f, 1.0f, d, 0, 0, 0, 0))
            minusCHit = 1;
        else if (Fn_4b08f0(&minusCPt, margin, a, f, g, 1, 0))
            minusCHit = 1;
        else {
            ((PlacementProbe*)b)->Fn_507c10(&minusCPt, &minusCOut, 1, &minusCHit);
            minusCHit &= j;
        }

        if (centerHit && plusAHit && minusAHit && plusCHit && minusCHit)
            radius += i;
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
        bestOut = nextOut = &minusCOut;
        bestPt = &minusCPt;
        nextPt = 0;
    }

    if (m) {
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

    *n = *bestPt;

    if (k) {
        if (!o)
            return 0;
        if (m && nextPt) {
            // horizontal direction between the two worst points, flipped to face k
            Vec3 w;
            w.x = nextPt->x - bestPt->x;
            w.y = nextPt->y - bestPt->y;
            w.z = nextPt->z - bestPt->z;
            w.y = 0.0f;
            if (w.x * k->x + w.z * k->z < 0.0f) {
                o->x = -w.x;
                o->y = 0.0f;
                o->z = -w.z;
            } else {
                *o = w;
            }
            PlacementNormalizeInPlace(o);
        } else if (e == 0) {
            *o = *k;
        }
    } else {
        if (!l)
            return 0;
        *o = *l - *bestPt;
        PlacementNormalizeInPlace(o);
    }
    return Fn_4b0b80(a, b, c, d, e, f, g, l, n, o);
}
