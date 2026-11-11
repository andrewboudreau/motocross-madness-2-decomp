// Group0_ContactArrays.cpp -- bring-up bodies for the contact-array helpers
// the soultree and vehicle step loops call:
//   0x0043ad80 SoultreeRefreshContacts / VehContactsA (one function, two views)
//   0x0043aa30 SoultreeMergeContacts / VehContactsB   (one function, two views)
//   0x0043a570 VehAddContact (the vehicle view of AddExistingCollisionPoint)
//   0x004b5a60 VehBasisToBlock
// The contact elements are CollisionPoint objects (collision/CollisionPoint.h);
// SoultreeContact and VehicleContact are views of them. The refresh and merge
// views forward to the exact bodies in src/krusty2/collision/CollisionPoint.cpp;
// VehBasisToBlock is decoded from retail (byte matching not attempted).
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

// CollisionPoint.cpp (matched) calls the same query through its own provisional view of the
// frame object; the object is the Terrain, so the call forwards to it.
class CollisionFrameHelper {
public:
    void QueryGround(CollisionVec3* v, const CollisionVec3* n, int a, char* flagOut);
};

void CollisionFrameHelper::QueryGround(CollisionVec3* v, const CollisionVec3* n, int a, char* flagOut)
{
    reinterpret_cast<Terrain*>(this)->QueryGround(reinterpret_cast<TerrainVec3*>(v),
                                                  reinterpret_cast<TerrainVec3*>(const_cast<CollisionVec3*>(n)), a,
                                                  reinterpret_cast<unsigned char*>(flagOut));
}

// The exact bodies (src/krusty2/collision/CollisionPoint.cpp, strict exact in calibration).
// The views below forward to them, so the game links retail's logic once instead of a
// bring-up paraphrase. Both signatures are provisional (cdecl, tier 3 names).
int SoultreeRefreshContacts(void* a1, int* penetrating, int count, CollisionPoint** points,
                            CollisionFrameHelper* frame, CollisionVec3* offset, int mode, float k);
int SoultreeMergeContacts(int count, CollisionPoint** points, const CollisionVec3* scaleA,
                          const CollisionVec3* scaleB, const CollisionVec3* bias, CollisionVec3* origin,
                          CollisionVec3* linA, CollisionVec3* delta, CollisionVec3* normal,
                          CollisionVec3* angular);

// ---------------------------------------------------------------------------
// 0x0043ad80: snaps every owned contact to the ground (a single contact in mode 1
// is placed at center - radius * normal) and recomputes its penetration, contact
// flag and surface grip. Returns 0 without doing anything when `enabled` is 0,
// else 1. `terrain` is the Terrain the ground query runs on.
// ---------------------------------------------------------------------------
int SoultreeRefreshContacts(int enabled, int* activeCount, int count, SoultreeContact** contacts,
                            SoultreeProbe* terrain, Vec3* center, int mode, float radius)
{
    return SoultreeRefreshContacts((void*)enabled, activeCount, count, (CollisionPoint**)contacts,
                                   (CollisionFrameHelper*)terrain, (CollisionVec3*)center, mode, radius);
}

int __cdecl VehContactsA(int enabled, int* activeCount, int count, VehicleContact** contacts,
                         SoultreeProbe* terrain, Vec3* center, int mode, float radius)
{
    return SoultreeRefreshContacts((void*)enabled, activeCount, count, (CollisionPoint**)contacts,
                                   (CollisionFrameHelper*)terrain, (CollisionVec3*)center, mode, radius);
}

// ---------------------------------------------------------------------------
// 0x0043aa30: merges the penetrating contacts into one response (mean position,
// normalised summed normal, deepest penetration):
//   outArm = mean position - center, outVel = vel + (angVel * scale) x outArm,
//   pos and center pushed out along the normal by the deepest penetration.
// Nothing is written when no contact penetrates. Retail returns the merged count
// in eax; both views discard it.
// ---------------------------------------------------------------------------
void SoultreeMergeContacts(int count, SoultreeContact** contacts, Vec3* scale, Vec3* angVel,
                           Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                           Vec3* outVel)
{
    SoultreeMergeContacts(count, (CollisionPoint**)contacts, (const CollisionVec3*)scale,
                          (const CollisionVec3*)angVel, (const CollisionVec3*)vel, (CollisionVec3*)center,
                          (CollisionVec3*)pos, (CollisionVec3*)outArm, (CollisionVec3*)outNormal,
                          (CollisionVec3*)outVel);
}

void __cdecl VehContactsB(int count, VehicleContact** contacts, Vec3* scale, Vec3* angVel,
                          Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                          Vec3* outVel)
{
    SoultreeMergeContacts(count, (CollisionPoint**)contacts, (const CollisionVec3*)scale,
                          (const CollisionVec3*)angVel, (const CollisionVec3*)vel, (CollisionVec3*)center,
                          (CollisionVec3*)pos, (CollisionVec3*)outArm, (CollisionVec3*)outNormal,
                          (CollisionVec3*)outVel);
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
