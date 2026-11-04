// Near-miss CollisionPoint.cpp candidates (D:\aardvark\VC\krusty2\CollisionPoint.cpp), kept out
// of src/krusty2/collision/CollisionPoint.cpp until they match.  The canonical file is included
// first so the inline constructor, the vector set and the helpers are the same.  Each
// function's note records what still differs; the scores are in targets.json and
// docs/NEAR_MISS_INDEX.md.
#include "collision/CollisionPoint.cpp"

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
// (*points)->ownerNode, as UpdateCollisionPointWorldPositions does).  The two paths are an
// if/else with one trailing `return 1`: with a return inside the single-contact path VC6 pushes
// ebx/ebp/esi/edi at entry, with a single exit after the null-a1 return it delays them past it
// as retail does.  The loop walks a pointer local `pp` copied from points after an
// `if (count > 0)` guard: then VC6 loads points before frame and homes the loop counter in
// points' dead argument slot, as retail does (with the pointer local but no guard the points
// load moves above the count test; walking points itself homes the counter in a1's slot).
// Still differs (604/615): in the single-contact path retail stores contactPos.y into
// worldPosition after the fstp of contactPos.z.  No gain from reordered, chained, pointer,
// component-wise or assigned-after-declaration copies of contactPos, a named product or -k
// local, the operands of the sum swapped, or a component-wise sum.
int SoultreeRefreshContacts(void* a1, int* penetrating, int count, CollisionPoint** points,
                            CollisionFrameHelper* frame, CollisionVec3* offset, int mode, float k)
{
    if (!a1)
        return 0;
    *penetrating = 0;
    if (mode == 1 && count == 1) {
        CollisionPoint* point = points[0];
        if (point->ownerNode) {
            CollisionVec3 tmp = *offset;
            frame->QueryGround(&tmp, &point->surfaceNormal, 0, 0);
            CollisionVec3 contactPos = point->surfaceNormal * -k + *offset;
            point->worldPosition = contactPos;
            point->surfacePosition = contactPos;
            CollisionRefreshContact(point, frame, penetrating);
        }
    } else {
        if (count > 0) {
            CollisionPoint** pp = points;
            for (; count > 0; count--, pp++) {
                CollisionPoint* point = *pp;
                if (point->ownerNode) {
                    CollisionVec3 worldBuffer;
                    CollisionVec3* worldPoint =
                        (*pp)->ownerNode->LocalToWorldPoint(&worldBuffer, &point->localPosition);
                    point->worldPosition = *worldPoint;
                    point->surfacePosition = *worldPoint;
                    CollisionRefreshContact(point, frame, penetrating);
                }
            }
        }
    }
    return 1;
}
