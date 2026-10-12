// Near-miss CollisionPoint.cpp candidates (D:\aardvark\VC\krusty2\CollisionPoint.cpp), kept out
// of src/krusty2/collision/CollisionPoint.cpp until they match.  The canonical file is included
// first so the inline constructor, the vector set and the helpers are the same.  Each
// function's note records what still differs; the scores are in targets.json and
// docs/NEAR_MISS_INDEX.md.
#include "collision/CollisionPoint.cpp"

// Helpers of the two contact functions below, in the header-inline shapes the rest of the
// unit uses (results through the inline constructor of CollisionPointVec3).  Tier 3 names.
static inline float ClampNonNegative(float v) { return v > 0.0f ? v : 0.0f; }
static inline CollisionPointVec3 InlineSum(const CollisionVec3& a, const CollisionVec3& b) {
    return CollisionPointVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline CollisionPointVec3 InlineDifference(const CollisionVec3& a, const CollisionVec3& b) {
    return CollisionPointVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// a is taken by value: retail copies *scaleB to a stack temporary before the product
// (the output may alias it).
static inline CollisionVec3 ComponentProduct(CollisionVec3 a, const CollisionVec3& b) {
    CollisionVec3 r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

// 0x0043aa30 (cdecl, provisional signature; near miss).  Merges the penetrating contacts of a
// manifold (tier 3 semantics): mean position, summed and normalised normal, deepest
// penetration, and the resulting linear (linA/origin) and angular response terms.  Returns
// the merged count.  The deepest penetration is homed in the dead `points` argument slot and
// the normal's squared length in the `normal` slot, as in retail.
// Matches retail's frame, block layout, first-contact clamp (a by-value clamp keeps the
// value on the x87 stack across the compare), the loop's single penetration load, the
// copies through by-value/inline-constructor helpers and the trailing `return 0` block.
// Still differs: (1) in sum * (1/merged) retail multiplies x and z scalar-first and y
// component-first (`fld sum.y; fmul st(1)`), the candidate all scalar-first; explicit,
// operator, helper and accumulation forms (+= per component, a += helper, sum = sum + p),
// a separately named scale, by-value or scale-first helpers, y written `sum.y * s` and a
// named temporary do not move it; (2) the push's fstp of z and the linA load are scheduled
// one copy later.  The normal's squared length is accumulated y, then x, then z (retail
// squares y in place, consumed by fmulp, and pops through fxch); the one-expression
// z * z + (x * x + y * y) and (y * y + x * x) + z * z forms give Vec3Normalize's order
// and the frame offsets after it shift (409 bytes).
int SoultreeMergeContacts(int count, CollisionPoint** points, const CollisionVec3* scaleA, const CollisionVec3* scaleB,
                          const CollisionVec3* bias, CollisionVec3* origin, CollisionVec3* linA,
                          CollisionVec3* delta, CollisionVec3* normal, CollisionVec3* angular)
{
    CollisionVec3 sum;
    float deepest;
    int merged;
    CollisionPoint* first = points[0];
    if (first->inContact) {
        merged = 1;
        deepest = ClampNonNegative(first->penetration);
        sum = first->worldPosition;
        *normal = first->surfaceNormal;
    } else {
        sum = g_CollisionZeroVec3;
        *normal = g_CollisionZeroVec3;
        merged = 0;
        deepest = 0.0f;
    }
    for (int i = 1; i < count; ++i) {
        CollisionPoint* p = points[i];
        if (p->inContact) {
            float pen = p->penetration;
            if (pen > 0.0f && pen > deepest)
                deepest = p->penetration;
            ++merged;
            sum.x += p->worldPosition.x;
            sum.y += p->worldPosition.y;
            sum.z += p->worldPosition.z;
            normal->x += p->surfaceNormal.x;
            normal->y += p->surfaceNormal.y;
            normal->z += p->surfaceNormal.z;
        }
    }
    if (merged > 0) {
        if (merged > 1) {
            sum = ScaleVec3(sum, 1.0f / merged);
            float len2 = normal->y * normal->y;
            len2 += normal->x * normal->x;
            len2 += normal->z * normal->z;
            if (len2 == 0.0f) {
                *normal = g_CollisionZeroVec3;
            } else {
                float s = FastInvSqrt(len2);
                normal->x *= s;
                normal->y *= s;
                normal->z *= s;
            }
        }
        *delta = InlineDifference(sum, *origin);
        *angular = CollisionCross(ComponentProduct(*scaleB, *scaleA), *delta);
        *angular = InlineSum(*bias, *angular);
        CollisionVec3 push = *normal * deepest;
        linA->x += push.x;
        linA->y += push.y;
        linA->z += push.z;
        origin->x += push.x;
        origin->y += push.y;
        origin->z += push.z;
        return merged;
    }
    return 0;
}

// Provisional view of the frame object passed to 0x0043ad80 (thiscall callee, owner class
// unknown).  The contact owner view (0x004fd660) is in CollisionPoint.h.
class CollisionFrameHelper {
public:
    // 0x00507c10, ret 0x10: snaps v to the surface along n; the last argument receives the
    // surface flag byte.
    void QueryGround(CollisionVec3* v, const CollisionVec3* n, int a, char* flagOut);
};

// Recomputes the contact position/penetration and tallies the penetrating contacts; expanded
// in both paths of 0x0043ad80 (retail repeats it, including the grip store and return).
static inline void CollisionRefreshContact(CollisionPoint* p, CollisionFrameHelper* frame, int* penetrating)
{
    frame->QueryGround(&p->surfacePosition, &p->surfaceNormal, 0, &p->surfaceType);
    p->penetration = (p->surfacePosition.y - p->worldPosition.y) * p->surfaceNormal.y;
    p->surfaceType &= 7;
    p->inContact = (p->penetration >= p->penetrationThreshold) ? 1 : 0;
    if (p->inContact)
        ++*penetrating;
    if (p->surfaceOwner) {
        // surfaceOwner -> object whose +0xa4 table holds per-surface grip values at +0x3a0 (tier 3).
        char* table = *(char**)(p->surfaceOwner + 0xa4);
        p->surfaceGrip = ((float*)(table + 0x3a0))[(unsigned char)p->surfaceType];
    } else {
        p->surfaceGrip = 1.0f;
    }
}

// 0x0043ad80 (cdecl, provisional signature; near miss).  Refreshes every owned contact's world
// and surface position (a single contact in mode 1 is placed at offset - k * normal instead)
// and its penetration, flag and grip.  Matches retail's body, the single-contact path, the
// counted loop over the pointer array and the reload of the owner for the call (written as
// (*points)->ownerNode, as UpdateCollisionPointWorldPositions does).  Still differs: retail
// tests a1 and returns before pushing ebx/ebp/esi (edi later); VC6 here pushes all four first
// (`a1 == 0`, if/else and result-local forms tried); retail keeps the loop counter in the dead
// `points` argument slot, the candidate in a1's slot (counted for/while/do-while forms tried).
int SoultreeRefreshContacts(void* a1, int* penetrating, int count, CollisionPoint** points,
                            CollisionFrameHelper* frame, CollisionVec3* offset, int mode, float k)
{
    if (!a1)
        return 0;
    *penetrating = 0;
    if (mode == 1 && count == 1) {
        CollisionPoint* p = points[0];
        if (p->ownerNode) {
            CollisionVec3 tmp = *offset;
            frame->QueryGround(&tmp, &p->surfaceNormal, 0, 0);
            CollisionVec3 w = p->surfaceNormal * -k + *offset;
            p->worldPosition = w;
            p->surfacePosition = w;
            CollisionRefreshContact(p, frame, penetrating);
        }
        return 1;
    }
    for (; count > 0; count--, points++) {
        CollisionPoint* q = *points;
        if (q->ownerNode) {
            CollisionVec3 t;
            CollisionVec3* r = (*points)->ownerNode->LocalToWorldPoint(&t, &q->localPosition);
            q->worldPosition = *r;
            q->surfacePosition = *r;
            CollisionRefreshContact(q, frame, penetrating);
        }
    }
    return 1;
}
