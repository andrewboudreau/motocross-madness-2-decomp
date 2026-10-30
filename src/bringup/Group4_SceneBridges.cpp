// Bring-up bridges (best-guess-unreviewed) for src/bringup/Group4_SceneRead.cpp.
//
// The Scene reader near-miss bodies (samples/track/SceneManagerNearMisses.cpp)
// were written against local proxy views (UnknownSceneCollisionObject,
// UnknownScenePhysicsObject, UnknownSceneObject, ...) whose mangled names have
// no provider. This unit includes the real src/krusty2 classes and exposes
// uniquely named free functions that perform the same retail calls; every
// function names the retail address it stands for. Each returned pointer is
// the object's start (the primary base the proxies also start with).
#include "core/DebugAlloc.h"
#include "motion/CollisionCharacter.h"
#include "soultree/SoultreePhysicsObject.h"
#include "collision/CollisionObject.h"
#include "collision/CollisionPoint.h"

// 0x005037c0 SoultreePhysicsObject(flags) (with the hidden most-derived flag),
// allocated with the caller's debug new site.
void* Group4Scene_NewPhysicsObject(const char* file, int line, int flags) {
    return new(file, line) SoultreePhysicsObject(flags);
}

// 0x004318d0 CollisionCharacter(flags) (with the hidden most-derived flag).
void* Group4Scene_NewCollisionCharacter(const char* file, int line, int flags) {
    return new(file, line) CollisionCharacter(flags);
}

// 0x004319c0 CollisionCharacter::Load.
GameObject* Group4Scene_LoadCollisionCharacter(void* character, void* owner, const char* name,
                                               const char* colPath, void* lights, int a4, int a5,
                                               int a6) {
    return ((CollisionCharacter*)character)->Load((int)owner, name, colPath,
                                                  (const SoultreeLoadDesc*)lights, a4, a5, a6);
}

// 0x004a6b30 Character::FindMotion. The scene passes the CollisionCharacter
// pointer unchanged as `this` (Character is its primary base at +0).
void* Group4Scene_FindMotion(void* character, const char* name, int unused) {
    return ((CollisionCharacter*)character)->FindMotion(name, unused);
}

// 0x00431e70 CollisionObject(flags).
void* Group4Scene_NewCollisionObject(const char* file, int line, int flags) {
    return new(file, line) CollisionObject(flags);
}

// 0x004320f0 CollisionObject::Configure.
void Group4Scene_ConfigureCollision(void* object, void* context, int useBroadphase,
                                    int collisionEnabled, int collidable) {
    ((CollisionObject*)object)->Configure(context, useBroadphase, collisionEnabled, collidable);
}

// 0x00432720 CollisionObject::SetHullShape.
void Group4Scene_SetHullShape(void* object, void* node, int a, int b, int c, int d) {
    ((CollisionObject*)object)->SetHullShape(node, a, b, c, d);
}

// 0x00432800 CollisionObject::LoadShape.
void Group4Scene_LoadShape(void* object, void* node, const char* path) {
    ((CollisionObject*)object)->LoadShape(node, path);
}

// 0x00435fe0 CollisionObject::SnapPlacement.
void Group4Scene_SnapPlacement(void* object) {
    ((CollisionObject*)object)->SnapPlacement();
}

// 0x0043a330 AddCollisionPoint (cdecl).
void* Group4Scene_AddCollisionPoint(int capacity, void** points, const void* position, void* owner,
                                    float threshold, int* count, float normalForce, int surfaceOwner) {
    return AddCollisionPoint(capacity, (CollisionPoint**)points, (const CollisionVec3*)position,
                             (CollisionContactOwner*)owner, threshold, count, normalForce, surfaceOwner);
}

// 0x004fbd70 SoultreeObject::SetAxesPtr.
void Group4Scene_SetAxesPtr(void* node, const void* look, const void* up, int orthogonalize, int keepZ) {
    ((SoultreeObject*)node)->SetAxesPtr((const Vec3*)look, (const Vec3*)up, orthogonalize, keepZ);
}

// 0x004fc660 SoultreeObject::SetPosition(const Vec3&).
void Group4Scene_SetPosition(void* node, const void* position) {
    ((SoultreeObject*)node)->SetPosition(*(const Vec3*)position);
}
