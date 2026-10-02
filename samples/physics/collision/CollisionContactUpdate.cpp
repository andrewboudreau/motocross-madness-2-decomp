// Follow-up to CollisionPoint::Fn_0043a640: per-contact refresh (tier 3 semantics).
// Provisional TU: proximity to CollisionPoint.cpp is not evidence of the retail file split.
#include "CollisionPoint.h"

// Provisional helper type for the out-of-line frame callee (thiscall, unknown owner class).
// The contact owner view (0x004fd660) is in CollisionPoint.h.
class CollisionFrameHelper {
public:
    // 0x00507c10, ret 0x10: transforms/projects a vector by the frame; last arg receives a small flag byte.
    void Fn_00507c10(CollisionVec3* v, const CollisionVec3* n, int a, char* flagOut);
};

// Recomputes the contact position/penetration field_0x98 and tallies the contacts that
// penetrate (field_0xa4). Both the 1-contact and N-contact shapes exist in retail.
static void CollisionRefreshContact(CollisionPoint* p, CollisionFrameHelper* frame, int* penetrating)
{
    frame->Fn_00507c10(&p->surfacePosition, &p->surfaceNormal, 0, &p->surfaceType);
    p->penetration = (p->surfacePosition.y - p->worldPosition.y) * p->surfaceNormal.y;
    p->surfaceType &= 7;
    p->inContact = (p->penetration >= p->penetrationThreshold) ? 1.0f : 0.0f;
    if (p->inContact != 0.0f)
        ++*penetrating;
    if (p->surfaceOwner) {
        // field_0xc0 -> object whose +0xa4 table holds per-surface values at +0x3a0 (tier 3: surface friction/grip).
        char* table = *(char**)(p->surfaceOwner + 0xa4);
        p->surfaceGrip = ((float*)(table + 0x3a0))[(unsigned char)p->surfaceType];
    } else {
        p->surfaceGrip = 1.0f;
    }
}

// 0x0043ad80 (cdecl).  Provisional signature.
int Fn_0043ad80(void* a1, int* penetrating, int count, CollisionPoint** points,
                CollisionFrameHelper* frame, CollisionVec3* offset, int mode, float k)
{
    if (!a1)
        return 0;
    *penetrating = 0;
    if (mode == 1 && count == 1) {
        // Single contact: the position is offset - k * normal instead of coming from the owner.
        CollisionPoint* p = points[0];
        if (p->ownerNode) {
            CollisionVec3 tmp = *offset;
            frame->Fn_00507c10(&tmp, &p->surfaceNormal, 0, 0);
            p->worldPosition.x = -k * p->surfaceNormal.x + offset->x;
            p->worldPosition.y = -k * p->surfaceNormal.y + offset->y;
            p->worldPosition.z = -k * p->surfaceNormal.z + offset->z;
            p->surfacePosition = p->worldPosition;
            CollisionRefreshContact(p, frame, penetrating);
        }
        return 1;
    }
    for (int i = 0; i < count; ++i) {
        CollisionPoint* p = points[i];
        if (p->ownerNode) {
            CollisionVec3 tmp;
            CollisionVec3* r = p->ownerNode->Fn_004fd660(&tmp, &p->localPosition);
            p->worldPosition = *r;
            p->surfacePosition = *r;
            CollisionRefreshContact(p, frame, penetrating);
        }
    }
    return 1;
}

// 0x0043aa30 (cdecl, provisional signature).  Merges the penetrating contacts of a manifold
// (tier 3 semantics): mean position, summed and normalised normal, deepest penetration, and
// the resulting linear (out1/out2) and angular (out3) response terms.  Returns the merged count.
int Fn_0043aa30(int count, CollisionPoint** points, const CollisionVec3* scaleA, const CollisionVec3* scaleB,
                const CollisionVec3* bias, CollisionVec3* origin, CollisionVec3* linA,
                CollisionVec3* delta, CollisionVec3* normal, CollisionVec3* angular)
{
    CollisionVec3 sum;
    float deepest;
    int merged = 0;
    CollisionPoint* first = points[0];
    if (first->inContact != 0.0f) {
        deepest = first->penetration > 0.0f ? first->penetration : 0.0f;
        sum = first->worldPosition;
        *normal = first->surfaceNormal;
        merged = 1;
    } else {
        sum = g_CollisionZeroVec3;
        *normal = g_CollisionZeroVec3;
        deepest = 0.0f;
    }
    for (int i = 1; i < count; ++i) {
        CollisionPoint* p = points[i];
        if (p->inContact != 0.0f) {
            if (p->penetration > 0.0f && p->penetration > deepest)
                deepest = p->penetration;
            sum.x += p->worldPosition.x;
            sum.y += p->worldPosition.y;
            sum.z += p->worldPosition.z;
            ++merged;
            normal->x += p->surfaceNormal.x;
            normal->y += p->surfaceNormal.y;
            normal->z += p->surfaceNormal.z;
        }
    }
    if (merged <= 0)
        return 0;
    if (merged > 1) {
        float inv = 1.0f / merged;
        sum.x *= inv;
        sum.y *= inv;
        sum.z *= inv;
        float len2 = normal->y * normal->y + normal->x * normal->x + normal->z * normal->z;
        if (len2 == 0.0f) {
            *normal = g_CollisionZeroVec3;
        } else {
            float s = FastInvSqrt(len2);
            normal->x *= s;
            normal->y *= s;
            normal->z *= s;
        }
    }
    delta->x = sum.x - origin->x;
    delta->y = sum.y - origin->y;
    delta->z = sum.z - origin->z;
    float wx = scaleB->x * scaleA->x;
    float wy = scaleB->y * scaleA->y;
    float wz = scaleB->z * scaleA->z;
    angular->x = wy * delta->z - wz * delta->y;
    angular->y = wz * delta->x - wx * delta->z;
    angular->z = wx * delta->y - wy * delta->x;
    angular->x += bias->x;
    angular->y += bias->y;
    angular->z += bias->z;
    CollisionVec3 push;
    push.x = deepest * normal->x;
    push.y = deepest * normal->y;
    push.z = deepest * normal->z;
    linA->x += push.x;
    linA->y += push.y;
    linA->z += push.z;
    origin->x += push.x;
    origin->y += push.y;
    origin->z += push.z;
    return merged;
}
