// Group0_ContactArrays.cpp -- bring-up bodies for the contact-array helpers
// the soultree and vehicle step loops call:
//   0x0043ad80 SoultreeRefreshContacts / VehContactsA (one function, two views)
//   0x0043aa30 SoultreeMergeContacts / VehContactsB   (one function, two views)
//   0x0043a570 VehAddContact (the vehicle view of AddExistingCollisionPoint)
//   0x004b5a60 VehBasisToBlock
// The contact elements are CollisionPoint objects (collision/CollisionPoint.h);
// SoultreeContact and VehicleContact are views of them. The refresh and merge
// bodies follow samples/physics/collision/CollisionPointNearMisses.cpp
// (604/615 and near-exact there); VehBasisToBlock is decoded from retail.
// Byte matching was not attempted.
#include <math.h>

#include "math/Math3D.h"
#include "collision/CollisionPoint.h"

class SoultreeContact;
struct VehicleContact;
struct SoultreeProbe;
struct TerrainVec3;

// The terrain ground query 0x00507c10 (broadphase/Terrain.h, not includable
// beside these headers): snaps pos->y to the surface under (x, z), writes the
// surface normal and, when given, the cell's surface byte.
class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

static inline void ContactQueryGround(SoultreeProbe* terrain, CollisionVec3* pos,
                                      CollisionVec3* normal, char* surface)
{
    reinterpret_cast<Terrain*>(terrain)->QueryGround(reinterpret_cast<TerrainVec3*>(pos),
                                                     reinterpret_cast<TerrainVec3*>(normal), 0,
                                                     reinterpret_cast<unsigned char*>(surface));
}

// ---------------------------------------------------------------------------
// 0x0043ad80
// ---------------------------------------------------------------------------

// Snaps the contact to the ground, recomputes its penetration, contact flag
// and surface grip, and counts it when it penetrates.
static void RefreshContact(CollisionPoint* p, SoultreeProbe* terrain, int* activeCount)
{
    ContactQueryGround(terrain, &p->surfacePosition, &p->surfaceNormal, &p->surfaceType);
    p->penetration = (p->surfacePosition.y - p->worldPosition.y) * p->surfaceNormal.y;
    p->surfaceType &= 7;
    p->inContact = (p->penetration >= p->penetrationThreshold) ? 1 : 0;
    if (p->inContact)
        ++*activeCount;
    if (p->surfaceOwner) {
        // The surface owner's +0xa4 table holds per-surface grip values at +0x3a0.
        char* table = *(char**)(p->surfaceOwner + 0xa4);
        p->surfaceGrip = ((float*)(table + 0x3a0))[(unsigned char)p->surfaceType];
    } else {
        p->surfaceGrip = 1.0f;
    }
}

// Refreshes every owned contact's world and surface position from its owner
// node (a single contact in mode 1 is placed at center - radius * normal
// instead), then its penetration, flag and grip. Returns 0 without doing
// anything when `enabled` is 0, else 1.
static int RefreshContacts(int enabled, int* activeCount, int count, CollisionPoint** points,
                           SoultreeProbe* terrain, CollisionVec3* center, int mode, float radius)
{
    if (!enabled)
        return 0;
    *activeCount = 0;
    if (mode == 1 && count == 1) {
        CollisionPoint* point = points[0];
        if (point->ownerNode) {
            CollisionVec3 probe = *center;
            ContactQueryGround(terrain, &probe, &point->surfaceNormal, 0);
            CollisionVec3 contactPos;
            contactPos.x = point->surfaceNormal.x * -radius + center->x;
            contactPos.y = point->surfaceNormal.y * -radius + center->y;
            contactPos.z = point->surfaceNormal.z * -radius + center->z;
            point->worldPosition = contactPos;
            point->surfacePosition = contactPos;
            RefreshContact(point, terrain, activeCount);
        }
    } else {
        for (int i = 0; i < count; i++) {
            CollisionPoint* point = points[i];
            if (point->ownerNode) {
                // The owner is the scene node (SoultreeObject, 0x004fd660).
                Vec3 world = reinterpret_cast<SoultreeObject*>(point->ownerNode)
                                 ->LocalToWorldPoint(*reinterpret_cast<const Vec3*>(&point->localPosition));
                point->worldPosition = *reinterpret_cast<CollisionVec3*>(&world);
                point->surfacePosition = *reinterpret_cast<CollisionVec3*>(&world);
                RefreshContact(point, terrain, activeCount);
            }
        }
    }
    return 1;
}

int SoultreeRefreshContacts(int enabled, int* activeCount, int count, SoultreeContact** contacts,
                            SoultreeProbe* terrain, Vec3* center, int mode, float radius)
{
    return RefreshContacts(enabled, activeCount, count, (CollisionPoint**)contacts, terrain,
                           (CollisionVec3*)center, mode, radius);
}

int __cdecl VehContactsA(int enabled, int* activeCount, int count, VehicleContact** contacts,
                         SoultreeProbe* terrain, Vec3* center, int mode, float radius)
{
    return RefreshContacts(enabled, activeCount, count, (CollisionPoint**)contacts, terrain,
                           (CollisionVec3*)center, mode, radius);
}

// ---------------------------------------------------------------------------
// 0x0043aa30
// ---------------------------------------------------------------------------

// Merges the penetrating contacts into one response: mean position, summed
// and normalised normal, deepest penetration. Then
//   outArm    = mean position - center
//   outVel    = vel + (angVel * scale) x outArm   (component product, cross)
//   pos and center are pushed out along the normal by the deepest penetration.
// Nothing is written when no contact penetrates. Retail returns the merged
// count in eax; both declarations are void.
static void MergeContacts(int count, CollisionPoint** points, const Vec3* scale, const Vec3* angVel,
                          const Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                          Vec3* outVel)
{
    Vec3 sum(0.0f, 0.0f, 0.0f);
    float deepest = 0.0f;
    int merged = 0;
    *outNormal = Vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < count; ++i) {
        CollisionPoint* point = points[i];
        if (!point->inContact)
            continue;
        if (merged == 0) {
            deepest = point->penetration > 0.0f ? point->penetration : 0.0f;
        } else if (point->penetration > 0.0f && point->penetration > deepest) {
            deepest = point->penetration;
        }
        ++merged;
        sum.x += point->worldPosition.x;
        sum.y += point->worldPosition.y;
        sum.z += point->worldPosition.z;
        outNormal->x += point->surfaceNormal.x;
        outNormal->y += point->surfaceNormal.y;
        outNormal->z += point->surfaceNormal.z;
    }
    if (merged <= 0)
        return;
    if (merged > 1) {
        sum = sum * (1.0f / merged);
        float len2 = outNormal->y * outNormal->y + outNormal->x * outNormal->x + outNormal->z * outNormal->z;
        if (len2 == 0.0f)
            *outNormal = Vec3(0.0f, 0.0f, 0.0f);
        else
            *outNormal *= FastInvSqrt(len2);
    }
    *outArm = sum - *center;
    Vec3 spin(angVel->x * scale->x, angVel->y * scale->y, angVel->z * scale->z);
    *outVel = *vel + CrossProduct(spin, *outArm);
    Vec3 push = *outNormal * deepest;
    *pos += push;
    *center += push;
}

void SoultreeMergeContacts(int count, SoultreeContact** contacts, Vec3* scale, Vec3* angVel,
                           Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                           Vec3* outVel)
{
    MergeContacts(count, (CollisionPoint**)contacts, scale, angVel, vel, center, pos, outArm,
                  outNormal, outVel);
}

void __cdecl VehContactsB(int count, VehicleContact** contacts, Vec3* scale, Vec3* angVel,
                          Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                          Vec3* outVel)
{
    MergeContacts(count, (CollisionPoint**)contacts, scale, angVel, vel, center, pos, outArm,
                  outNormal, outVel);
}

// ---------------------------------------------------------------------------
// 0x0043a570: the vehicle's view of AddExistingCollisionPoint (CollisionPoint.cpp);
// `threshold` carries the float threshold's bits (Vehicle::AddWheel passes its
// float argument through as an int).
// ---------------------------------------------------------------------------
CollisionPoint* AddExistingCollisionPoint(int capacity, CollisionPoint** points, float threshold,
                                          int* count, CollisionPoint* point);

void __cdecl VehAddContact(int capacity, SoultreeContact** arr, int threshold, int* count,
                           void* contact)
{
    AddExistingCollisionPoint(capacity, (CollisionPoint**)arr, *(float*)&threshold, count,
                              (CollisionPoint*)contact);
}

// ---------------------------------------------------------------------------
// 0x004b5a60: decomposes the body basis (forward a, up b) into yaw, pitch and
// roll with their cosines/sines:
//   yaw   (*p0) from forward's horizontal direction (signed by forward.x),
//   pitch (*p1) from forward rotated into the yaw plane (signed by forward.y),
//         cos/sin in *p5/*p6,
//   roll  (*p2) from up rotated by yaw and pitch (signed by the rotated up.x),
//         sin/cos in *p3/*p4.
// Within 10 degrees of straight up/down the roll is folded into the yaw
// (wrapped into [0, 2pi]) and cleared. Retail returns 0 when p0, p1 or p2 is
// null (nothing written) and 1 otherwise; the declaration is void.
// ---------------------------------------------------------------------------
void __cdecl VehBasisToBlock(Vec3 a, Vec3 b, float* p0, float* p1, float* p2,
                             float* p3, float* p4, float* p5, float* p6)
{
    const float kHalfPi = 1.5707964f;
    const float kTwoPi = 6.2831855f;
    const float kGimbalTolerance = 0.17453292f;   // 10 degrees

    if (!p0 || !p1 || !p2)
        return;

    // Yaw.
    float c, s;
    float h = (float)sqrt(a.z * a.z + a.x * a.x);
    if (h == 0.0f) {
        c = 1.0f;
        s = 0.0f;
    } else {
        c = a.z / h;
        s = a.x / h;
    }
    *p0 = (float)acos(c);
    if (a.x < 0.0f)
        *p0 = -*p0;

    // Forward and up rotated into the yaw frame.
    float forwardZ = c * a.z + s * a.x;
    float upZ = c * b.z + s * b.x;
    float upX = c * b.x - s * b.z;

    // Pitch.
    float len = (float)sqrt(forwardZ * forwardZ + a.y * a.y);
    if (len == 0.0f) {
        *p5 = 1.0f;
        *p6 = 0.0f;
    } else {
        *p5 = forwardZ / len;
        *p6 = a.y / len;
    }
    *p1 = (float)acos(*p5);
    if (a.y < 0.0f)
        *p1 = -*p1;

    // Roll: up rotated by the pitch.
    float upY = b.y * *p5 - upZ * *p6;
    float upLen = (float)sqrt(upY * upY + upX * upX);
    if (upLen == 0.0f) {
        *p4 = 1.0f;
        *p3 = 0.0f;
    } else {
        *p4 = upY / upLen;
        *p3 = upX / upLen;
    }
    *p2 = (float)acos(*p4);
    if (upX < 0.0f)
        *p2 = -*p2;

    // Near vertical: fold the roll into the yaw.
    if (fabs(*p1 - kHalfPi) < kGimbalTolerance) {
        *p0 = *p0 - *p2;
        if (*p0 < 0.0f)
            *p0 += kTwoPi;
        *p2 = 0.0f;
    } else if (fabs(*p1 + kHalfPi) < kGimbalTolerance) {
        *p0 = *p0 + *p2;
        if (*p0 > kTwoPi)
            *p0 -= kTwoPi;
        *p2 = 0.0f;
    }
}
