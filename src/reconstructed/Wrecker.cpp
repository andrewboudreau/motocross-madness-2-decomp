// Wrecker.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\wrecker.cpp.
// See Wrecker.h and docs/WRECKER.md.

#include "Wrecker.h"
#include "../krusty2/core/SoultreeObject.h"

#include <stdlib.h>

#include "DebugAlloc.h"

// Per-TU vector constants (tier 2, the shape documented in docs/LZW.md and
// src/krusty2/math/Math3D.h): wrecker.cpp's four dynamic initializers
// (.CRT$XCU 0x00566574..0x00566580, code 0x00532f00..0x0053303b) build
// (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x0068ad00, 0x0068ad10,
// 0x0068ad20 and 0x0068acf0. Slot 10 (0x00530739) reads the zero vector.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// A random value in [0, 1). A macro: as an inline function, VC6 moves the
// constant factor behind the caller's multiply (retail keeps rand * 1/32768
// first, then the scale).
#define RANDOM_UNIT() (rand() * (1.0f / 32768))

Wrecker::Wrecker(int flags) : GraphicsTest(flags) {
    indexedAnimations = 0;
    indexedAnimationCount = 0;
    randomAnimations = 0;
    randomAnimationCount = 0;
    contactAnimations = 0;
    contactAnimationCount = 0;
    ignoreObjects = 0;
    ignoreObjectCount = 0;
    wreckState = 0;
    hadContact = 0;
    carryRiderPose = 0;
    pendingAnimation = 0;
    anyProbeHit = 0;
    probePositions = 0;
    probeHits = 0;
    hitCenter = Vector3(0.0f, 0.0f, 0.0f);
    pushImpulse = Vector3(0.0f, 0.0f, 0.0f);
    pushScale = 5.0f;
    trailFrame = 0;
    sprayFrame = 0x1d;
    particles = 0;
    for (int i = 0; i < 256; i++)
        randomTable[i] = RANDOM_UNIT();
    randomIndex = 0;
    skipProbeEffects = 0;
    blendStarted = 0;
    poseStepsPending = 0;
    tumbleStarted = 0;
}

Wrecker::~Wrecker() {
    if (probePositions) DebugFree(probePositions, __FILE__, 0x3e);
    if (probeHits) DebugFree(probeHits, __FILE__, 0x40);
    if (contactAnimations) DebugFree(contactAnimations, __FILE__, 0x43);
    if (randomAnimations) DebugFree(randomAnimations, __FILE__, 0x45);
    if (indexedAnimations) DebugFree(indexedAnimations, __FILE__, 0x47);
}

Wrecker* Wrecker::SetupWrecker(void* owner, UnknownWreckerCharacter* rider, const char* name) {
    GameObject::SetRenderContext(owner);
    CreateBodies(rider, name);
    return this;
}

void Wrecker::CreateBodies(UnknownWreckerCharacter* rider, const char* name) {
    character = rider;
    rider->modelNode->TranslateIn(0, Vec3(0.0f, 120.0f, 0.0f));
    gravityModel = new (__FILE__, 0x5b) UnknownWreckerGravityModel(1);
    gravityModel->SetRenderContext(renderContext);
    AppendChild(gravityModel, -1);
    rigidBody = new (__FILE__, 0x5f) UnknownWreckerRigidBody(1);
    rigidBody->SetRenderContext(renderContext);
    AppendChild(rigidBody, -1);
    rigidBody->UnknownVirtualSlot28(character->modelNode);
    gravityModel->UnknownVirtualSlot27(rigidBody);
    collisionModel = new (__FILE__, 0x68) UnknownWreckerCollisionModel(1);
    collisionModel->SetRenderContext(renderContext);
    collisionModel->SetBody(rigidBody, 1, name);
    AppendChild(collisionModel, -1);
    collisionModel->shapeCollisionEnabled = 0;
    collisionModel->collisionEnabled = 0;
    collisionModel->collidable = 0;
    collisionModel->applyImpulses = 0;
    collisionModel->damping = 0.99f;
    collisionModel->impulseLoss = 0.95f;
    collisionModel->maxImpulse = 2000.0f;
    g_collisionQuadTree->Remove(collisionModel, collisionModel->quadtreeCell);
    rigidBody->UnknownVirtualSlot29(200.0f);
    inertiaFactor = rigidBody->mass * (1.0f / 12);
    rigidBody->inertia = ZeroMatrix();
    rigidBody->invInertia = ZeroMatrix();
    rigidBody->isEnabled = 0;
    rigidBody->initialAngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
    rigidBody->initialVelocity = Vector3(0.0f, 0.0f, 0.0f);
    character->modelNode->subtreeDirty = 1;
    character->SetMotionByName("Fall02");
    UpdateInertia();
    rigidBody->UnknownVirtualSlot34(&Vector3(0.0f, 0.0f, 0.0f));
    rigidBody->UnknownVirtualSlot33(&Vector3(0.0f, 0.0f, 0.0f));
    rigidBody->UnknownVirtualSlot44();
    poseNode = character->modelNode->firstChild;
    collisionModel->manualStep = 1;
    rigidBody->stepDisabled = 1;
    gravityModel->manualStep = 1;
    boxCenter = Vector3(0.0f, 0.0f, 0.0f);
    boxHalfExtents = Vector3(0.0f, 0.0f, 0.0f);
    boxFrame = 0;
}

void Wrecker::AddContactAnimation(int value) {
    contactAnimations = (int*)DebugRealloc(contactAnimations, contactAnimationCount * 4 + 4, __FILE__, 0x9d);
    contactAnimations[contactAnimationCount] = value;
    contactAnimationCount++;
}

void Wrecker::AddRandomAnimation(int value) {
    randomAnimations = (int*)DebugRealloc(randomAnimations, randomAnimationCount * 4 + 4, __FILE__, 0xa4);
    randomAnimations[randomAnimationCount] = value;
    randomAnimationCount++;
}

int Wrecker::AddIndexedAnimation(int value) {
    indexedAnimations = (int*)DebugRealloc(indexedAnimations, indexedAnimationCount * 4 + 4, __FILE__, 0xac);
    indexedAnimations[indexedAnimationCount] = value;
    indexedAnimationCount++;
    return indexedAnimationCount - 1;
}

void Wrecker::AddIgnoredObject(UnknownWreckerCollisionObject* object) {
    ignoreObjects = (UnknownWreckerCollisionObject**)DebugRealloc(ignoreObjects, ignoreObjectCount * 4 + 4, __FILE__, 0xb4);
    ignoreObjects[ignoreObjectCount] = object;
    ignoreObjectCount++;
    collisionModel->AddIgnoredOwner(object);
    object->AddIgnoredOwner(collisionModel);
}

// Vector helpers in the d3dvec.inl style (MatrixUtil.h's Vector3 has only
// the scale and += operators).
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// ---------------------------------------------------------------------------
// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the sites where retail
// calls the COMDAT copy of a header inline instead of expanding it. Declared
// only; the bindings map them to the retail copies.
// ---------------------------------------------------------------------------
struct WreckerVec3Call : Vector3 {
    WreckerVec3Call(float x_, float y_, float z_);                 // 0x00404e60
};
Vector3 WreckerSubtractValue(const Vector3& a, const Vector3& b);  // 0x00421d00
Vector3 WreckerNormalizeCall(const Vector3& v);                    // 0x005087b0
Vector3 WreckerCrossCall(const Vector3& a, const Vector3& b);      // 0x00515600

// Copies a vector member by member into a returned local: retail copies
// the old position (and forward row) through a stack temporary this way.
static inline Vector3 WreckerCopy(const Vector3& v) {
    Vector3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}


static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// Per-step update of a GameObject (slot 11) through the float view. The
// named view pointer (one more local symbol in each of the two expansions)
// is what gives the first carry-over product retail's term order in
// relative._23 (a._23 * b._22 before a._33 * b._23): with the cast written
// inline that element sums the other way (4174/4178). VC6 SP3 orders equal
// candidates there by the count of referenced local symbols created before
// the product, modulo 8: two to seven more give retail's order, one more
// is worse, eight more is back to the old order (dead initialised ints
// probe the same window; the uninitialised ones are not counted).
static inline void WreckerStep(GameObject* object, float step) {
    UnknownWreckerSteppable* steppable = (UnknownWreckerSteppable*)object;
    steppable->UnknownVirtualSlot11(step);
}

// In-place inverse of a rigid transform (the InvertRigid helper of
// src/krusty2/bvh/BoundingBoxTreeQuery.h on a Matrix4).
static inline void WreckerInvertRigid(UnknownWreckerMatrix* m) {
    float swap;
    swap = m->_12; m->_12 = m->_21; m->_21 = swap;
    swap = m->_13; m->_13 = m->_31; m->_31 = swap;
    swap = m->_23; m->_23 = m->_32; m->_32 = swap;
    Vector3 t;
    t.x = -(m->_41 * m->_11 + m->_42 * m->_21 + m->_43 * m->_31);
    t.y = -(m->_41 * m->_12 + m->_42 * m->_22 + m->_43 * m->_32);
    t.z = -(m->_41 * m->_13 + m->_42 * m->_23 + m->_43 * m->_33);
    m->_41 = t.x;
    m->_42 = t.y;
    m->_43 = t.z;
}

// Rebuilds the rotation rows of a pose from its up and forward rows.
static inline void WreckerOrthonormalize(UnknownWreckerMatrix* m) {
    Vector3 up = WreckerVec3Call(m->_21, m->_22, m->_23);
    Vector3 forward = WreckerVec3Call(m->_31, m->_32, m->_33);
    forward = WreckerNormalizeCall(forward);
    Vector3 right = WreckerNormalizeCall(WreckerCrossCall(up, forward));
    up = WreckerCrossCall(forward, right);
    m->_11 = right.x;
    m->_12 = right.y;
    m->_13 = right.z;
    m->_14 = 0.0f;
    m->_21 = up.x;
    m->_22 = up.y;
    m->_23 = up.z;
    m->_24 = 0.0f;
    m->_31 = forward.x;
    m->_32 = forward.y;
    m->_33 = forward.z;
    m->_34 = 0.0f;
}

// The 4x4 product expanded inline: MatrixMultiply (0x0042a1a0,
// src/krusty2/math/Math3D.h) with both operands by value, *out = b * a in
// the row-vector convention. Inlined, VC6 copies only the operands that
// can change under it: the member pose (a, rep movsd) in the first product
// of slot 10 and the output itself (b) in the second; an unmodified local
// is read in place.
static inline void WreckerMatrixProduct(UnknownWreckerMatrix* out, UnknownWreckerMatrix a, UnknownWreckerMatrix b) {
    out->_11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
    out->_12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
    out->_13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
    out->_14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
    out->_21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
    out->_22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
    out->_23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
    out->_24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
    out->_31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
    out->_32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
    out->_33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
    out->_34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
    out->_41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
    out->_42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
    out->_43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
    out->_44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
}

// Slot 10, 0x005306e0 (4178 bytes): the per-frame update. Retail's frame
// (0x128) and order come from the rider carry-over written with inline
// helpers: the inverse's translation, the by-value product operands and the
// axis rebuild's locals live in sibling inline scopes that share slots (the
// inverse's t with the rebuild's forward); the old position is copied
// through WreckerCopy from a pointer to the member pose (the dead temporary
// at frame 0, the y/z loads through the rep movsd source); the subtract is
// called by value (its result a temporary). Retail rebuilds up as
// forward x right. See WreckerStep for the last term order.
int Wrecker::Tick(float frameTime) {
    GameObject::Tick(frameTime);
    if (wreckState == 0) {
        collisionModel->UnknownFunctionStep(frameTime);
        if (skipProbeEffects)
            skipProbeEffects = 0;
        else
            EmitProbeEffects(frameTime);
        anyProbeHit = 0;
        PushProbesOutOfBox(frameTime);
        hitCenter = kVec3Zero;
        float touching = 0.0f;
        for (int i = 0; i < collisionModel->probeCount; i++) {
            if (collisionModel->probes[i].hit) {
                probeHits[i] = 1;
                anyProbeHit = 1;
                hitCenter += probePositions[i];
                touching += 1.0f;
            } else {
                probeHits[i] = 0;
            }
        }
        if (anyProbeHit)
            hitCenter *= 1.0f / touching;
        RebuildPoseSteps();
        linearStep = linearStep * (1.0f / frameTime);
        angularStep = angularStep * (1.0f / frameTime);
        poseNode->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&lastPose));
    } else {
        if (pendingAnimation && !character->blendActive) {
            character->BlendToMotion(reinterpret_cast<Motion*>(pendingAnimation), 0.5f);
            pendingAnimation = 0;
            if (tumbleStarted == 0 && blendStarted == 0) {
                blendStarted = 1;
            } else {
                carryRiderPose = 1;
                tumbleStarted = 0;
                blendStarted = 0;
            }
        }
        float remaining = frameTime;
        contactThisFrame = 0;
        float step = 0.02f;
        if (frameTime > 0.06f)
            step = frameTime * (1.0f / 3);
        while (remaining > 0.0f) {
            if (remaining < step)
                step = remaining;
            if (wreckState != 1) {
                WreckerStep(gravityModel, step);
                WreckerStep(rigidBody, step);
            }
            if (skipProbeEffects)
                skipProbeEffects = 0;
            else
                EmitProbeEffects(frameTime);
            for (int i = 0; i < collisionModel->probeCount; i++)
                probeHits[i] = collisionModel->probes[i].hit;
            PushProbesOutOfBox(step);
            if (wreckState != 1) {
                Vector3 position;
                rigidBody->node->GetPositionIn(0, reinterpret_cast<Vec3*>(&position));
                position += pushImpulse;
                rigidBody->node->SetPositionInFrame(0, reinterpret_cast<const Vec3*>(&position));
            }
            collisionModel->UnknownFunctionStep(step);
            remaining -= step;
        }
        if (collisionModel->hasContact) {
            contactThisFrame = 1;
            hadContact = 1;
            if (wreckState == 2) {
                wreckState = 3;
                pendingAnimation = contactAnimations[(int)(RANDOM_UNIT() * contactAnimationCount)];
            }
        }
        if (wreckState) {
            if (wreckState == 1) {
                UnknownWreckerMotion* motion = reinterpret_cast<UnknownWreckerMotion*>(character->currentMotion);
                if ((float)(motion->frameCount - character->currentFrame->frameIndex) / motion->fps < 0.5f) {
                    ContinueAsTumble();
                    RebuildPoseSteps();
                    linearStep = linearStep * (1.0f / frameTime);
                    angularStep = angularStep * (1.0f / frameTime);
                    rigidBody->UnknownVirtualSlot44();
                    rigidBody->velocity = linearStep;
                    rigidBody->prevVelocity = linearStep;
                    rigidBody->angularVelocity = reinterpret_cast<const Vector3&>(rigidBody->node->WorldToLocalDirection(reinterpret_cast<const Vec3&>(angularStep)));
                }
            }
            if (blendStarted && !character->blendActive) {
                carryRiderPose = 1;
                blendStarted = 0;
            }
            if (carryRiderPose && poseStepsPending) {
                RebuildPoseSteps();
                linearStep = linearStep * (1.0f / frameTime);
                angularStep = angularStep * (1.0f / frameTime);
                poseStepsPending = 0;
            }
            poseNode->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&lastPose));
            character->CharacterVirtualSlot7(frameTime, 0, 0);
        }
    }
    if (carryRiderPose) {
        // Carry the rider's root over by the motion of the wreck node since
        // the pose lastPose was taken, then re-orthonormalise its axes.
        UnknownWreckerMatrix pose;
        UnknownWreckerMatrix relative;
        poseNode->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&pose));
        UnknownWreckerFrame* frame = &lastPose;
        Vector3 oldPosition = WreckerCopy(frame->position);
        WreckerInvertRigid(&pose);
        WreckerMatrixProduct(&relative, *(UnknownWreckerMatrix*)frame, pose);
        character->modelNode->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&pose));
        WreckerMatrixProduct(&pose, relative, pose);
        WreckerOrthonormalize(&pose);
        character->modelNode->SetMatrixIn(0, reinterpret_cast<Matrix4*>(&pose));
        Vector3 position;
        poseNode->GetPositionIn(0, reinterpret_cast<Vec3*>(&position));
        Vector3 delta = WreckerSubtractValue(oldPosition, position);
        character->modelNode->SetPositionIn(0, reinterpret_cast<const Vec3*>(&delta));
        carryRiderPose = 0;
    }
    UpdateInertia();
    return 1;
}

// Division by a scalar through the reciprocal. Inlined, it gives retail's
// `fld st(0); fmul [delta.x]` for the first component of the step in
// 0x00531da0; the same reciprocal product written in the caller
// (`delta * (1.0f / count)`, or a `scale` local) loads delta.x first.
inline Vector3 operator/(const Vector3& v, float s) {
    return v * (1.0f / s);
}

// 0x00531da0: emits a trail of particles along a probe's movement. Retail
// needs the squared length written `(y*y + x*x) + z*z` (or
// `z*z + (x*x + y*y)`); `x*x + y*y + z*z` reuses delta.z from the FPU stack.
void Wrecker::EmitProbeTrail(Vector3 from, Vector3 to, float frameTime) {
    Vector3 delta = to - from;
    float lengthSq = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z;
    float length = lengthSq == 1.0f ? 1.0f : FastSqrt(lengthSq);
    float speed = length / frameTime;
    if (speed < 0.5f)
        return;
    speed *= 0.013333f;
    if (wreckState) {
        if (speed < 1.5f)
            speed = 1.5f;
        else if (speed > 2.0f)
            speed = 2.0f;
    } else {
        if (speed < 0.25f)
            speed = 0.25f;
        else if (speed > 2.0f)
            speed = 2.0f;
    }
    float lifetime = speed * 2000.0f;
    if (lifetime < 500.0f)
        lifetime = 500.0f;
    else if (lifetime > 1000.0f)
        lifetime = 1000.0f;
    int count = (int)(length / (speed * 0.6f));
    if (!count)
        count = 1;
    float age = lifetime * 0.25f;
    Vector3 step = delta / (float)count;
    for (int i = 0; i < count; i++) {
        Vector3 position = from + step * (float)i;
        particles->AddParticle(age, &position, trailFrame, speed, lifetime, speed + speed, 10, 0xffffff);
        if (++trailFrame >= 13)
            trailFrame = 0;
    }
}

void Wrecker::EmitProbeEffects(float frameTime) {
    for (int i = 0; i < collisionModel->probeCount; i++) {
        if (collisionModel->probes[i].hit) {
            Vector3 point = reinterpret_cast<const Vector3&>(collisionModel->probes[i].node->LocalToWorldPoint(reinterpret_cast<const Vec3&>(collisionModel->probes[i].localPoint)));
            EmitProbeTrail(probePositions[i], point, frameTime);
            SprayContactParticles(probePositions[i], point, collisionModel->probes[i].normal, frameTime);
        }
    }
}

void Wrecker::UpdateInertia() {
    Vector3 size;
    character->modelNode->GetSubtreeBounds(reinterpret_cast<Vec3*>(&rigidBody->centerOfMass), reinterpret_cast<Vec3*>(&size));
    rigidBody->inertia(0, 0) = (size.y * size.y + size.z * size.z) * inertiaFactor;
    rigidBody->inertia(1, 1) = (size.x * size.x + size.z * size.z) * inertiaFactor;
    rigidBody->inertia(2, 2) = (size.x * size.x + size.y * size.y) * inertiaFactor;
    rigidBody->invInertia(0, 0) = 1.0f / rigidBody->inertia(0, 0);
    rigidBody->invInertia(1, 1) = 1.0f / rigidBody->inertia(1, 1);
    rigidBody->invInertia(2, 2) = 1.0f / rigidBody->inertia(2, 2);
}

void Wrecker::StartAnimatedWreck(int index) {
    collisionModel->shapeCollisionEnabled = 1;
    collisionModel->SnapPlacement();
    collisionModel->Enable();
    collisionModel->collisionEnabled = 1;
    collisionModel->collidable = 1;
    collisionModel->applyImpulses = 1;
    character->SetMotion(reinterpret_cast<Motion*>(indexedAnimations[index]));
    wreckState = 1;
    rigidBody->isEnabled = 0;
    rigidBody->UnknownVirtualSlot44();
    rigidBody->angularVelocity = Vector3(0.0f, 0.0f, 0.0f);
    rigidBody->velocity = Vector3(0.0f, 0.0f, 0.0f);
    hadContact = 0;
}

void Wrecker::StartTumble() {
    collisionModel->shapeCollisionEnabled = 1;
    collisionModel->SnapPlacement();
    collisionModel->Enable();
    collisionModel->collisionEnabled = 1;
    collisionModel->collidable = 1;
    collisionModel->applyImpulses = 1;
    wreckState = 2;
    character->BlendToMotion(reinterpret_cast<Motion*>(randomAnimations[(int)(RANDOM_UNIT() * randomAnimationCount)]), 0.5f);
    blendStarted = 1;
    poseStepsPending = 1;
    rigidBody->isEnabled = 1;
    rigidBody->UnknownVirtualSlot44();
    rigidBody->velocity = linearStep;
    rigidBody->prevVelocity = linearStep;
    rigidBody->angularVelocity = reinterpret_cast<const Vector3&>(rigidBody->node->WorldToLocalDirection(reinterpret_cast<const Vec3&>(angularStep)));
    for (int i = 0; i < collisionModel->probeCount; i++)
        probePositions[i] = Vector3(0.0f, 0.0f, 0.0f);
    hadContact = 0;
    tumbleStarted = 1;
}

void Wrecker::ContinueAsTumble() {
    wreckState = 2;
    pendingAnimation = randomAnimations[(int)(RANDOM_UNIT() * randomAnimationCount)];
    if (!character->blendActive) {
        character->BlendToMotion(reinterpret_cast<Motion*>(pendingAnimation), 0.5f);
        pendingAnimation = 0;
        if (blendStarted) {
            carryRiderPose = 1;
            tumbleStarted = 0;
            blendStarted = 0;
        } else {
            blendStarted = 1;
        }
    }
    rigidBody->isEnabled = 1;
    for (int i = 0; i < collisionModel->probeCount; i++)
        probePositions[i] = Vector3(0.0f, 0.0f, 0.0f);
    poseStepsPending = 1;
}

// The parenthesised first product puts each fsubp between the load and the
// store of the previous component's copy, as retail does
// (docs/VC6_OPERAND_ORDER.md section 3).
inline Vector3 WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - a.x * b.z;
    r.z = (a.x * b.y) - a.y * b.x;
    return r;
}

// 0x00532580: rebuilds the wreck pose's linear (linearStep) and angular
// (angularStep) step from the node's frame. Two references fix the leaf ages
// (docs/VC6_OPERAND_ORDER.md section 1.2): `target` for the new forward row
// in the equality tests makes its components newer than the copied old
// forward, so the cross product loads them first as retail does (511 -> 523
// of 562; through the reference only in the cross product it stays 511), and
// `up` for the old up row gives retail's schedule of the step's last store
// and the forward copy around the up-row loads (a pointer, a reference to
// linearStep for the subtract or the reference for up.x alone do the same).
void Wrecker::RebuildPoseSteps() {
    UnknownWreckerFrame pose;
    linearStep = WreckerCopy(lastPose.position);
    poseNode->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&pose));
    linearStep = pose.position - linearStep;
    Vector3 forward = WreckerCopy(lastPose.forward);
    const Vector3& target = pose.forward;
    // The old up row stays on the FPU stack: float locals.
    const Vector3& up = lastPose.up;
    float upX = up.x;
    float upY = up.y;
    float upZ = up.z;
    if (upX == pose.up.x && upY == pose.up.y && upZ == pose.up.z) {
        if (forward.x == target.x && forward.y == target.y && forward.z == target.z)
            angularStep = Vector3(0.0f, 0.0f, 0.0f);
        else
            angularStep = WreckerCross(forward, pose.forward);
    } else {
        angularStep = WreckerCross(Vector3(upX, upY, upZ), pose.up);
    }
    angularStep = Vec3ScaleCall(angularStep, 1.0f);
}

void Wrecker::EndWreck() {
    if (wreckState) {
        for (int i = 0; i < ignoreObjectCount; i++) {
            collisionModel->AddIgnoredOwner(ignoreObjects[i]);
            ignoreObjects[i]->AddIgnoredOwner(collisionModel);
        }
        wreckState = 0;
        rigidBody->isEnabled = 0;
        collisionModel->shapeCollisionEnabled = 0;
        collisionModel->collisionEnabled = 0;
        collisionModel->collidable = 0;
        collisionModel->applyImpulses = 0;
        collisionModel->hasContact = 0;
        g_collisionQuadTree->Remove(collisionModel, collisionModel->quadtreeCell);
        pendingAnimation = 0;
        skipProbeEffects = 1;
        character->modelNode->ResetMatrices();
        character->ApplyPoseListSlot6(&character->poseList, 0);
        UpdateInertia();
    }
}

int Wrecker::UnknownVirtualSlot14() {
    return GameObject::UnknownVirtualSlot14();
}

int Wrecker::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return GameObject::OnControlPress(event, entry);
}

void Wrecker::SetPushBox(UnknownWreckerSkeleton* frame, Vector3 center, Vector3 halfExtents) {
    boxCenter = center;
    boxHalfExtents = halfExtents;
    boxFrame = frame;
}

void Wrecker::AddProbe(Vector3 offset, int bone) {
    collisionModel->AddProbePoint(offset, bone);
    probePositions = (Vector3*)DebugRealloc(probePositions, collisionModel->probeCount * sizeof(Vector3), __FILE__, 0x308);
    probePositions[collisionModel->probeCount - 1] = Vector3(0.0f, 0.0f, 0.0f);
    probeHits = (int*)DebugRealloc(probeHits, collisionModel->probeCount * 4, __FILE__, 0x30b);
    probeHits[collisionModel->probeCount - 1] = 0;
}
