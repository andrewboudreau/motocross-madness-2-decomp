// SoulTreePhysics.cpp -- SoultreePhysicsBaseObject (D:\aardvark\VC\krusty2\SoulTreePhysics.cpp,
// confirmed by __FILE__ xrefs at 0x00500d6c/0x00500dbd/0x00500fe2/0x0050117c).
// Translation-unit ownership of the neighbouring functions is tier 2 (proximity
// plus the same __FILE__ string).
#include "../../reconstructed/TrackGameGlobal.h"
#include "../../reconstructed/ParticleEmitters.h"
#include <math.h>
#include <string.h>
#include <float.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "core/DebugAlloc.h"
#include "core/MemTag.h"
#include "soultree/SoultreePhysicsContact.h"
#include "soultree/SoultreePhysicsObject.h"
#include "soultree/SoultreePhysicsCharacter.h"
#include "collision/CollisionPoint.h"

#define g_Zero kVec3Zero

void SoultreePhysicsBaseObject::UnknownVirtualSlot20(SoultreeAttachment* attachment)
{
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot25()
{
    return 0;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot27()
{
}

// ==== chunks ====

// Length with the same shape the retail code uses: 1.0f is returned for a unit
// squared length, otherwise the square root.
static inline float SquareMagnitudeAcc(const Vec3& v)
{
    float s = v.x * v.x;
    s += v.y * v.y;
    s += v.z * v.z;
    return s;
}

static inline float VecLength(const Vec3& v)
{
    float s = SquareMagnitudeAcc(v);
    if (s == 1.0f)
        return 1.0f;
    return (float)sqrt(s);
}

// Cross product in the operand order the retail code uses: r.x = b.z * a.y - b.y * a.z
// (the product with b comes first in every fmul; see slots 4 and 13).
static inline Vec3 SoultreeCross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}

// Dot product with the z term first, the shape the retail code uses.
static inline float SoultreeDot(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

// slot 3 (0x00501310): static-contact impulse.  Runs the TU-local solver helper with this
// body's state, then refreshes cached values.  Parameter names follow the helper's.
void SoultreePhysicsBaseObject::UnknownVirtualSlot3(const Vec3* normal, const Vec3* relVel,
                                                    const Vec3* arm, const Vec3* angScale,
                                                    int eventKind, int scaleBits, float* impulseOut)
{
    ResolveContactImpulse(restitution, invMass, sceneNode, normal, relVel, arm, &invInertia, angScale,
                          &angularVelocity, &velocity, impulseOut, scaleBits);
    linearSpeed = VecLength(velocity);
    worldAngularVelocity = sceneNode->LocalToWorldDirection(angularVelocity);
}

// Cross product built with the three-float constructor, each first product parenthesised:
// slot 4 is exact with this form (the parentheses put each fsubp between the load and the
// store of the previous component's copy; docs/VC6_OPERAND_ORDER.md section 3).
static inline Vec3 SoultreeCrossCtor(const Vec3& a, const Vec3& b)
{
    return Vec3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}

// slot 4 (0x005013d0): body-body contact impulse.  pointVelA/B are each body's velocity at
// its contact arm (v + w x r, scaled); parameter names follow ContactSolveImpulse's A/B.
void SoultreePhysicsBaseObject::UnknownVirtualSlot4(const Vec3* normal, Vec3* velA,
                                                    const Vec3* worldAngVelA, const Vec3* armA,
                                                    int eventKind, float invMassB, int nodeB,
                                                    const Vec3* velB, const Vec3* worldAngVelB,
                                                    const Vec3* armB, Vec3* invInertiaB,
                                                    Vec3* angVelB, int velBOrNull,
                                                    float* impulseOut, float scale)
{
    Vec3 pointVelA = (SoultreeCrossCtor(*worldAngVelA, *armA) + *velA) * scale;
    Vec3 pointVelB = (SoultreeCrossCtor(*worldAngVelB, *armB) + *velB) * scale;
    ContactSolveImpulse(restitution, normal, invMass, sceneNode, &pointVelA, armA, &invInertia,
                        &angularVelocity, velA, invMassB, (SoultreeObject*)nodeB, &pointVelB, armB,
                        invInertiaB, angVelB, (Vec3*)velBOrNull, impulseOut);
    float s = SoultreeDot(*velA, *velA);
    linearSpeed = (s == 1.0f) ? 1.0f : (float)sqrt(s);
    worldAngularVelocity = sceneNode->LocalToWorldDirection(angularVelocity);
}

// slot 0 (0x005008d0): stores value + field_0x150 and derived quantities.
void SoultreePhysicsBaseObject::UnknownVirtualSlot0(float extraWeight)
{
    totalWeight = extraWeight + baseWeight;
    weightForce.y = -totalWeight;
    bodyMass = totalWeight * 0.0310559f;
    invMass = 1.0f / bodyMass;
}

// slot 1 (0x00500910): reset dynamic state.
void SoultreePhysicsBaseObject::UnknownVirtualSlot1(float extraWeight)
{
    UnknownVirtualSlot0(extraWeight);
    velocity = g_Zero;
    prevVelocity = g_Zero;
    prevSpeed = 0.0f;
    linearSpeed = 0.0f;
    acceleration = g_Zero;
    angularAcceleration = g_Zero;
    worldAngularVelocity = g_Zero;
    angularVelocity = g_Zero;
    touchingPointCount = 0;
    pointsTouching = 0;
    bodySinRoll = 0.0f;
    bodyCosRoll = 1.0f;
    bodySinPitch = 0.0f;
    bodyCosPitch = 1.0f;
    rotationPivot = g_Zero;
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedSinRoll = 0.0f;
    savedCosRoll = 1.0f;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedCosPitch = 1.0f;
    savedSinPitch = 0.0f;
    airborne = 0;
    restTimer = 0;
    slot27Pending = 0;
}

// slot 8 (0x00501bd0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot8()
{
    rotationPivot = localCenterOfMass = sceneNode->WorldToLocalPoint(centerOfMass);
}

// slot 9 (0x00501ce0): fixed-timestep accumulator (tier 3 reading; the arithmetic is decoded).
// field_0x1e4 is the fixed step, field_0x1e0 the carried remainder, field_0x1ec the cap on
// steps per frame.  steps = floor((dt + remainder) / step), remainder' = leftover time; when
// less than one step has elapsed the frame runs as a single variable-length step of dt (or
// of one nominal step when dt is 0).  field_0x13c = step length used, field_0x140 = 1/step.
void SoultreePhysicsBaseObject::UnknownVirtualSlot9(float dt, int* steps)
{
    frameTime = dt + stepRemainder;
    int wholeSteps = (int)(frameTime / fixedStepTime);
    *steps = wholeSteps;
    if (wholeSteps) {
        stepRemainder = frameTime - wholeSteps * fixedStepTime;
        *steps = (*steps > maxStepsPerFrame) ? maxStepsPerFrame : *steps;
        stepTime = fixedStepTime;
    } else {
        stepRemainder = 0.0f;
        if (dt != 0.0f) {
            stepTime = frameTime;
        } else {
            stepTime = fixedStepTime;
            frameTime = fixedStepTime;
        }
        *steps = 1;
    }
    invStepTime = 1.0f / stepTime;
}

// slot 11 (0x00501da0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot11(int respawn, Vec3* outPosition, Vec3* outForward,
                                                    Vec3* outUp, int* outKeepPlacement)
{
    return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, 0, 0x7fffffff,
                     0x7fffffff, 0, 3.0f, groundProbeMask, &savedForward, 0, !respawnPending,
                     outPosition, outForward, outUp, outKeepPlacement);
}

// slot 15 (0x00502200)
void SoultreePhysicsBaseObject::UnknownVirtualSlot15(const Vec3* worldTorque, Vec3* angularAccel)
{
    Vec3 localTorque = sceneNode->WorldToLocalDirection(*worldTorque);
    Vec3 localAccel;
    if (UnknownVirtualSlot10()) {
        localAccel.x = localTorque.x * shapeInvInertia.x;
        localAccel.y = localTorque.y * shapeInvInertia.y;
        localAccel.z = localTorque.z * shapeInvInertia.z;
    } else {
        localAccel.x = localTorque.x * invInertia.x;
        localAccel.y = localTorque.y * invInertia.y;
        localAccel.z = localTorque.z * invInertia.z;
    }
    UnknownAxisSettle_4cb6e0(&angularVelocity.x, &angularAccel->x, &localAccel.x, stepTime, 0);
}

// slot 16 (0x005022b0)
Vec3 SoultreePhysicsBaseObject::UnknownVirtualSlot16(const Vec3* localTorque)
{
    Vec3 accel;
    if (UnknownVirtualSlot10()) {
        accel.x = shapeInvInertia.x * localTorque->x;
        accel.y = shapeInvInertia.y * localTorque->y;
        accel.z = shapeInvInertia.z * localTorque->z;
    } else {
        accel.x = invInertia.x * localTorque->x;
        accel.y = invInertia.y * localTorque->y;
        accel.z = invInertia.z * localTorque->z;
    }
    return accel;
}

// Division through the reciprocal. Inlined, it gives slot 17's mixed schedule (x as
// `fld st0; fmul [sum.x]`, y as `fld [sum.y]; fmul st1`, z consuming the reciprocal);
// the product written in the caller (`sum * (1.0f / count)`, per-component constructors, a
// named reciprocal) or `(1.0f / s) * v` in the helper give other orders.
static inline Vec3 DivideVec(const Vec3& v, float s)
{
    return v * (1.0f / s);
}

// slot 17 (0x00502330): average of the target positions of the non-type-4 attachments.
Vec3 SoultreePhysicsBaseObject::UnknownVirtualSlot17()
{
    Vec3 sum;
    int count = 0;
    int first = 1;
    for (int i = 0; i < attachmentCount; i++) {
        SoultreeAttachment* attachment = &attachments[i];
        if (attachment->type == 4)
            continue;
        if (first) {
            sum = attachment->anchor->anchorPosition;
            first = 0;
        } else {
            sum += attachment->anchor->anchorPosition;
        }
        count++;
    }
    if (count) {
        return DivideVec(sum, (float)count);
    }
    return g_Zero;
}

// slot 18 (0x00502420)
void SoultreePhysicsBaseObject::UnknownVirtualSlot18(SoultreeAttachment* attachment)
{
    for (int i = 0; i < collisionPointCount; i++) {
        SoultreeContact* contact = collisionPoints[i];
        if (contact->touching && !contact->effectSpawned &&
            track->trackData->materialEffectFlags[(unsigned char)contact->materialId] != 0) {
            contact->effectSpawned = 1;
            reinterpret_cast<DustParticleEmitter*>(attachment->contactEmitter)->SetPosition(
                reinterpret_cast<const ParticleVec3&>(contact->field_0x20), 0.0f);
            if (attachment->resetTrail)
                attachment->contactEmitter->previousPosition = attachment->contactEmitter->currentPosition;
            attachment->contactEmitter->activeThisFrame = 1;
            attachment->resetTrail = 0;
            return;
        }
    }
}

// slot 28 (0x00502950)
int SoultreePhysicsBaseObject::UnknownVirtualSlot28(int steps)
{
    collisionObject->UpdatePlacement();
    collisionObject->QueryCollisions();
    if (collisionObject->hasContact) {
        position -= *(Vec3*)collisionObject->contactRecord;
        sceneNode->SetPosition(position);
        collisionObject->UpdatePlacement();
    }
    if (slot27Pending) {
        UnknownVirtualSlot27();
        slot27Pending = 0;
    }
    return steps;
}

// slot 29 (0x005029d0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot29(int lastStep)
{
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
}

// slot 30 (0x00502f10)
void SoultreePhysicsBaseObject::UnknownVirtualSlot30()
{
    for (int i = 0; i < collisionPointCount; i++) {
        collisionPoints[i]->effectSpawned = 0;
        collisionPoints[i]->field_0xac = 0;
        collisionPoints[i]->field_0xb0 = 0;
    }
}

// slot 31 (0x00501e10)
void SoultreePhysicsBaseObject::UnknownVirtualSlot31()
{
    for (int i = 0; i < collisionPointCount; i++) {
        SoultreeContact* contact = collisionPoints[i];
        if (contact->ownerNode && contact->field_0x98 >= 0.0f) {
            if (contact->contactState != 2)
                contact->contactState = 1;
            reinterpret_cast<CollisionPoint*>(contact)->UpdateRelativeMotion(
                reinterpret_cast<const CollisionVec3*>(&centerOfMass),
                reinterpret_cast<const CollisionVec3*>(&worldAngularVelocity),
                reinterpret_cast<const CollisionVec3*>(&velocity),
                reinterpret_cast<const CollisionVec3*>(&contact->leverArm), linearSpeed);
        } else {
            contact->contactState = 0;
        }
    }
}

// slot 33 (0x00501c20)
int SoultreePhysicsBaseObject::UnknownVirtualSlot33(const Vec3* newPosition, const Vec3* newForward,
                                                    const Vec3* newUp, const Vec3* settleUp,
                                                    int keepPlacement, float extraWeight)
{
    UnknownVirtualSlot1(extraWeight);
    bodyForward = *newForward;
    bodyUp = *newUp;
    sceneNode->SetPosition(newPosition->x, newPosition->y, newPosition->z);
    sceneNode->GetPosition(&position);
    UnknownVirtualSlot36();
    if (centerNode)
        centerNode->GetPositionIn(0, &centerOfMass);
    else
        sceneNode->GetPositionIn(0, &centerOfMass);
    if (collisionObject)
        collisionObject->SnapPlacement();
    respawnPending = 0;
    justReset = 1;
    attachmentResetPending = 1;
    return 0;
}

// slot 37 (0x00502a40): append an attachment record.
SoultreeAttachment* SoultreePhysicsBaseObject::UnknownVirtualSlot37(int type, void* target,
                                                                   SoultreeAttachTarget* anchor,
                                                                   const Vec3* v)
{
    if (attachmentCount >= attachmentCapacity || type == 0)
        return 0;
    SoultreeAttachment* attachment = &attachments[attachmentCount];
    attachment->type = type;
    attachment->anchor = anchor;
    attachment->contactEmitter = 0;
    attachment->slideEmitter = 0;
    attachment->scrapeEmitter = 0;
    attachment->pointEmitter = 0;
    attachment->localOffset = g_Zero;
    attachment->resetTrail = 0;
    switch (type) {
    case 1:
        attachment->contactEmitter = (SoultreeAttachedObject*)target;
        break;
    case 2:
        attachment->slideEmitter = target;
        break;
    case 3:
        attachment->scrapeEmitter = target;
        break;
    case 4:
        attachment->pointEmitter = target;
        attachment->localOffset = *v;
        attachment->anchor = 0;
        break;
    }
    attachmentCount++;
    return attachment;
}

// slot 39 (0x005019a0)
int SoultreePhysicsBaseObject::UnknownVirtualSlot39(float dt)
{
    if (respawnPending) {
        position = respawnPosition;
        sceneNode->SetPosition(position);
        return 1;
    }
    return 0;
}

// Length as computed by the retail inline helper: 1.0f for a unit squared length,
// otherwise the table-driven square root at 0x00460b50.
static inline float VecLengthFast(const Vec3& v)
{
    float lengthSq = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (lengthSq == 1.0f)
        return 1.0f;
    return FastSqrt(lengthSq);
}

// slot 7 (0x005019e0): distributes the force *force over the contacts, weighting each
// active contact by 1 - (its distance / summed distance).  Tier 3 reading.
// This is an inverse-distance weighting: contacts closer to the body position get the larger
// share (a lone active contact gets the whole vector, weight 1).  Vehicle slot 7 is a near copy.
void SoultreePhysicsBaseObject::UnknownVirtualSlot7(const Vec3* force)
{
    float dist[128];
    int i;
    if (pointsTouching) {
        float total = 0.0f;
        int active = 0;
        int last;
        for (i = 0; i < collisionPointCount; i++) {
            SoultreeContact* contact = collisionPoints[i];
            if (contact->touching) {
                Vec3 offset = centerOfMass - contact->worldPosition;
                float len = VecLengthFast(offset);
                dist[i] = len;
                total += len;
                active++;
                last = i;
            } else {
                contact->loadShare = 0.0f;
                contact->forceShare = g_Zero;
            }
        }
        if (active == 1) {
            collisionPoints[last]->loadShare = 1.0f;
            collisionPoints[last]->forceShare = *force;
            return;
        }
        for (i = 0; active > 0; i++) {
            SoultreeContact* contact = collisionPoints[i];
            if (contact->touching) {
                contact->loadShare = 1.0f - dist[i] / total;
                contact->forceShare = *force * contact->loadShare;
                active--;
            }
        }
    } else {
        for (i = 0; i < collisionPointCount; i++) {
            SoultreeContact* contact = collisionPoints[i];
            contact->loadShare = 0.0f;
            contact->forceShare = g_Zero;
        }
    }
}

// Dword-wise Vec3 copy: VC6 addresses a struct-assignment destination through a register
// (lea) and folds memberwise float copies of a constant temporary; retail slot 13 keeps the
// stack temporary and stores to [contact+disp] directly, which only this form reproduces.
// The long-typed view is provisional (code shape, not a known original form).
static inline void SoultreeCopyBits(Vec3& d, const Vec3& s)
{
    ((long*)&d)[0] = ((const long*)&s)[0];
    ((long*)&d)[1] = ((const long*)&s)[1];
    ((long*)&d)[2] = ((const long*)&s)[2];
}

// slot 13 (0x00501e90): per-contact response; accumulates into *force / *localTorque.
// PARTIAL at full length: normalForceMagnitude = 0 is scheduled after the zero copy
// (retail: between its second and third temporary loads, as in Vehicle 0x00527a20), and
// the y and z cross terms that use leverArm.x load it before frictionForce (operand order,
// a by-value, pointer, swapped-parameter or contact-member cross are inert or worse).
void SoultreePhysicsBaseObject::UnknownVirtualSlot13(Vec3* force, Vec3* localTorque, float torqueScale)
{
    if (!touchingPointCount)
        return;
    for (int i = 0; i < collisionPointCount; i++) {
        SoultreeContact* contact = collisionPoints[i];
        if (!contact->contactState)
            continue;
        Vec3 share;
        float weight = contact->loadShare;
        share.x = weight * force->x;
        share.y = weight * force->y;
        share.z = weight * force->z;
        float normalComponent = SoultreeDot(share, contact->contactNormal);
        contact->normalForceMagnitude = normalComponent;
        if (normalComponent >= 0.0f) {
            contact->normalForceMagnitude = 0.0f;
            SoultreeCopyBits(contact->normalForce, Vec3(0.0f, 0.0f, 0.0f));
        } else {
            contact->normalForceMagnitude = -normalComponent;
            Vec3 scaled;
            SoultreeCopyBits(contact->normalForce, *Vec3ScaleCall(&scaled, &contact->contactNormal, -normalComponent));
        }
        contact->UnknownVirtualSlot1();
        *force += contact->frictionForce;
        *localTorque += sceneNode->WorldToLocalDirection(
            CrossProduct(contact->leverArm, contact->frictionForce));
        if (contact->contactState == 2) {
            *localTorque *= torqueScale;
            contact->contactState = 0;
        }
    }
}

// slot 14 (0x00502080): integrates the accumulated force/torque estimates.
void SoultreePhysicsBaseObject::UnknownVirtualSlot14(Vec3* force, const Vec3* localTorque,
                                                     const Vec3* worldTorque)
{
    angularAcceleration = UnknownVirtualSlot16(localTorque);
    UnknownVirtualSlot15(worldTorque, &angularAcceleration);
    angularVelocity += angularAcceleration * stepTime;
    angularVelocity *= 0.999f;
    acceleration = *force * invMass;
    velocity += acceleration * stepTime;
    linearSpeed = VecLength(velocity);
}


// Event record handed to slot 38 as `event`; +0x60 is the other body (tier 1, decoded).
struct SoultreeCollisionEvent {
    char pad_0x00[0x60];
    SoultreePhysicsBaseObject* other;
};

// slot 38 (0x00501600): collision response for one contact.  Tier 3 reading: eventKind is
// the event/surface kind (0x68 = contact with another physics body, 0x66/0x6a/0x3e9 = static
// or world contact, anything else ignored).  For a body contact the relative contact
// positions and the other body's velocity/angular state are gathered and slot 4 (the impulse
// solver) is run; the other body's speed and world angular velocity are then refreshed.  For
// a static contact the lever arm = contact - centre of mass gives
// scratchVector = (worldAngularVelocity * angScale) x arm, and slot 3 (the static solver) is
// run with velocity + that term.  Finally a nearly resting body (small prevSpeed and speed)
// is snapped to zero velocity.  `notify` is unused here.
// eventKind's address is passed on as the float* impulse result to slots 3/4 (retail does that).
// The static arm scales a local copy of the angular velocity in place (t.x *= angScale.x ...):
// retail copies worldAngularVelocity to the frame, keeps the three scaled components on the x87
// stack through the cross product and pops them afterwards.  A by-value Scale3 helper
// returning a fresh vector consumes them in place instead (919 bytes, not 925).
void SoultreePhysicsBaseObject::UnknownVirtualSlot38(int notify, int eventKind, void* event)
{
    Vec3 angScale;
    float otherInvMass;
    angScale = Vec3(1.0f, 1.0f, 1.0f);
    otherInvMass = 1.0f;
    SoultreeObject* otherNode;
    Vec3* otherVel = 0;
    SoultreePhysicsBaseObject* other = 0;
    Vec3 otherInvInertia, otherArm, otherWorldAngVel;
    int hasBody;

    switch (eventKind) {
    case 0x68:
        other = ((SoultreeCollisionEvent*)event)->other;
        otherInvMass = other->invMass;
        otherNode = other->sceneNode;
        otherVel = &other->velocity;
        otherArm = *(Vec3*)&collisionObject->hitPoint - other->centerOfMass;
        otherWorldAngVel = other->worldAngularVelocity;
        event = &other->angularVelocity;    // retail reuses the argument's slot for it
        otherInvInertia = other->invInertia;
        hasBody = 1;
        break;
    case 0x66:
    case 0x6a:
    case 0x3e9:
        angScale = Vec3(1.0f, 1.0f, 1.0f);
        hasBody = 0;
        break;
    default:
        return;
    }

    Vec3 arm = *(Vec3*)&collisionObject->hitPoint - centerOfMass;
    if (hasBody) {
        UnknownVirtualSlot4((Vec3*)&collisionObject->hitNormal, &velocity, &worldAngularVelocity, &arm,
                            eventKind, otherInvMass, (int)otherNode, otherVel, &otherWorldAngVel,
                            &otherArm, &otherInvInertia, (Vec3*)event, (int)otherVel,
                            (float*)&eventKind, 1.0f);
        if (other) {
            other->asleep = 0;
            other->linearSpeed = VecLength(*otherVel);
            other->worldAngularVelocity = other->sceneNode->LocalToWorldDirection(*(Vec3*)event);
        }
    } else {
        Vec3 scaledAngVel = worldAngularVelocity;
        scaledAngVel.x *= angScale.x;
        scaledAngVel.y *= angScale.y;
        scaledAngVel.z *= angScale.z;
        scratchVector = CrossProduct(scaledAngVel, arm);
        Vec3 contactVelocity = velocity + scratchVector;
        UnknownVirtualSlot3((Vec3*)&collisionObject->hitNormal, &contactVelocity, &arm, &angScale, eventKind,
                            0, (float*)&eventKind);
    }
    if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
        velocity = g_Zero;
        linearSpeed = 0.0f;
    }
}

// ---- slot 21 (0x005024f0) -------------------------------------------------------------
// Local views of objects reached through the attachment records / probe (tier 3 names,
// offsets decoded from slot 21 only).
struct SoultreeLightList {          // field_0x08->+0x1bc: count at +0x2c, items from +0x30
    char pad_0x00[0x2c];
    int count;
    struct SoultreeLight* items[1];
};
struct SoultreeLight {        // entries of the list above; tier 3: a light (kind 2 directional, 4 positional)
    char pad_0x00[0x2c];
    int kind;                      // 2 or 4 qualify as the tracked object (tier 3)
    char pad_0x30[0x34];
    Vec3 lightPosition;  // +0x64 slot 21 probes toward it for kind 4 (positional)
    float lightDirX, lightDirY, lightDirZ;  // +0x78 slot 21, kind 2: probe target z = origin.z + lightDirZ * -3000
};
struct SoultreePadObject {         // attachment slideEmitter / scrapeEmitter targets: only +0x60 is written
    char pad_0x00[0x60];
    int activeThisFrame;  // +0x60 slot 21 clears it every frame for type-2/3 targets, like SoultreeAttachedObject::activeThisFrame
};
struct SoultreeSinkObject {        // attachment field_0x14 target (type 4)
    char pad_0x00[0x48];
    Vec3 currentPosition;  // +0x48 set by 0x004ba2c0 in slot 21; copied to previousPosition on a trail reset
    Vec3 previousPosition;  // +0x54 slot 21: previousPosition = currentPosition when the attachment's resetTrail is set
    // SteamParticleEmitter's setters (samples/physics/effects/ParticleEmitters.h); the decoded
    // stores agree. Restart/Burst are that sample's provisional names.
    void SetPosition(Vec3 v);  // 0x004ba2c0 thiscall, callee pops 0xc: previousPosition = +0x48, then +0x48 = v
    void Restart();            // 0x004ba300: +0x60 = 1, +0x44 = +0x40, +0x6c = 1
    void Burst();              // 0x004ba320: +0x44 = 0.0001, +0x6c = +0x70 = 1
    void StopEmitting();       // 0x004ba340: +0x60 = 0, previousPosition = position
    void SetColor(int r, int g, int b);  // 0x004ba360: +0x478 = (r << 16) | (g << 8) | (b & 0xff)
};

// The type-4 attachment target (field_0x14).  Slot 21 reloads it at every use; caching it
// in a local changes register allocation.
static inline SoultreeSinkObject* AttachmentSink(const SoultreeAttachment* a)
{
    return (SoultreeSinkObject*)a->pointEmitter;
}

// Per-frame refresh of the attachment records (field_0x1d4, field_0x1dc of them).  Tier 3
// reading: (1) pick a tracked object from the node's list (kind 2 or 4); (2) on the
// slot 22 cadence, probe from the body position (+1.5 up) along the tracked object's
// velocity * -3000 (kind 2) or its own position, and when the probe result flips relative to
// field_0x1fc retint every type 1/4 attachment (0x40 vs 0xff) and toggle flag 0x800;
// (3) advance the field_0x208 cadence counter (wraps after 5); (4) reset each attachment,
// re-running the slot 18/19/20 updaters, or for type 4 moving the sink to the node-local
// point and calling its 4ba300/4ba320 depending on slots 24/25; (5) finally clear field_0x60
// of every attachment when field_0x20c was set.
void SoultreePhysicsBaseObject::UnknownVirtualSlot21()
{
    if (!attachmentCount)
        return;
    if (!particleManager || !(particleManager->isEnabled))
        return;
    if (!shadowLight) {
        SoultreeLightList* list = *(SoultreeLightList**)((char*)sceneNode + 0x1bc);
        for (int i = 0; i < list->count; i++) {
            SoultreeLight* light = list->items[i];
            if (light->kind == 2 || light->kind == 4)
                shadowLight = light;
        }
    }
    if (UnknownVirtualSlot22()) {
        int hit;
        Vec3 pos = UnknownVirtualSlot17();
        pos.y += 1.5f;
        SoultreeLight* light = shadowLight;
        Vec3 out;
        if (light->kind == 2) {
            Vec3 to = pos + Vec3(light->lightDirX * -3000.0f, light->lightDirY * -3000.0f,
                                 light->lightDirZ * -3000.0f);
            hit = terrain->CastSegment(&pos, &to, &out, 0, 0, 0);
        } else {
            hit = terrain->CastSegment(&pos, &light->lightPosition, &out, 0, 0, 0);
        }
        if (hit != (unsigned char)inShadow) {
            for (int i = 0; i < attachmentCount; i++) {
                switch (attachments[i].type) {
                case 1:
                    if (hit) {
                        reinterpret_cast<DustParticleEmitter*>(attachments[i].contactEmitter)->SetColor(0x40, 0x40, 0x40);
                        attachments[i].contactEmitter->particleFlags |= 0x800;
                    } else {
                        reinterpret_cast<DustParticleEmitter*>(attachments[i].contactEmitter)->SetColor(0xff, 0xff, 0xff);
                        attachments[i].contactEmitter->particleFlags &= ~0x800;
                    }
                    break;
                case 4:
                    if (hit)
                        reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(&attachments[i]))->SetColor(0x80, 0x80, 0x80);
                    else
                        reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(&attachments[i]))->SetColor(0xff, 0xff, 0xff);
                    break;
                }
            }
            inShadow = (char)hit;
        }
    }
    if (++staggerCounter > 5)
        staggerCounter = 0;
    int moving = UnknownVirtualSlot23();
    int settled = UnknownVirtualSlot24();
    for (int i = 0; i < attachmentCount; i++) {
        SoultreeAttachment* attachment = &attachments[i];
        attachment->resetTrail = (justReset || attachment->resetTrail) ? 1 : 0;
        switch (attachment->type) {
        case 1:
            attachment->contactEmitter->activeThisFrame = 0;
            if (moving)
                UnknownVirtualSlot18(attachment);
            break;
        case 2:
            ((SoultreePadObject*)attachment->slideEmitter)->activeThisFrame = 0;
            if (moving)
                UnknownVirtualSlot19(attachment);
            break;
        case 3:
            ((SoultreePadObject*)attachment->scrapeEmitter)->activeThisFrame = 0;
            if (moving)
                UnknownVirtualSlot20(attachment);
            break;
        case 4:
            if (!justReset) {
                scratchVector = sceneNode->LocalToWorldPoint(attachment->localOffset);
                reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(attachment))->SetPosition(
                    reinterpret_cast<const ParticleVec3&>(scratchVector));
                if (attachment->resetTrail) {
                    AttachmentSink(attachment)->previousPosition = AttachmentSink(attachment)->currentPosition;
                    attachment->resetTrail = 0;
                }
                if (settled && !field_0x20e)
                    reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(attachment))->Restart();
                else if (UnknownVirtualSlot25())
                    reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(attachment))->Burst();
            }
            break;
        }
    }
    field_0x20e = (char)settled;
    if (attachmentResetPending) {
        attachmentResetPending = 0;
        for (int i = 0; i < attachmentCount; i++) {
            switch (attachments[i].type) {
            case 1:
                attachments[i].contactEmitter->activeThisFrame = 0;
                break;
            case 2:
                ((SoultreePadObject*)attachments[i].slideEmitter)->activeThisFrame = 0;
                break;
            case 3:
                ((SoultreePadObject*)attachments[i].scrapeEmitter)->activeThisFrame = 0;
                break;
            case 4:
                reinterpret_cast<SteamParticleEmitter*>(AttachmentSink(&attachments[i]))->StopEmitting();
                break;
            }
        }
    }
}

// ---- slot 2 (0x00500c50) --------------------------------------------------------------
// Debug allocator (core/DebugAlloc.h, 0x004a3010: size, __FILE__, line); the retail source is
// D:\aardvark\VC\krusty2\SoulTreePhysics.cpp, so the file argument is that literal.
#define SP_FILE "D:\\aardvark\\VC\\krusty2\\SoulTreePhysics.cpp"

extern int g_SoultreeInstanceCounter;      // 0x00689f14: cycles 0..5 (field_0x204 takes the old value)

// cdecl 0x004b5a60 is OrientationAnglesFromVectors (../common/Math3D.h).

// Initializer (tier 3 reading): stores the construction parameters (start position, forward
// and up, weight, contact and attachment capacities, sphere radius or box inertia mode, ...),
// allocates the contact pointer array and attachment records, derives the inverse inertia
// diagonal (sphere: 1/(0.4 m r^2), otherwise a box from the node extents with
// m/12 (h^2 + d^2)), builds the collision body (createCollision) and orientation matrix, and
// optionally creates a child node (createCenterNode).  Returns the GameObject virtual base (`this ? vbase : 0`).
GameObject* SoultreePhysicsBaseObject::UnknownVirtualSlot2(int renderContext, int createCenterNode,
                                                           Vec3 startPosition, Vec3 startForward,
                                                           Vec3 startUp, void* terrainProbe,
                                                           void* linkedObject, float weight,
                                                           int contactCapacity, int attachmentSlots,
                                                           SoultreeSlot1f0* trackObject,
                                                           float fixedStep, int maxSteps, float drag,
                                                           float elasticity, float radius,
                                                           int shapeMode, int restThreshold,
                                                           unsigned char probeMask,
                                                           int createCollision)
{
    restContactThreshold = restThreshold;
    particleManager = (GameObject*)linkedObject;
    terrain = (SoultreeProbe*)terrainProbe;
    baseWeight = weight;
    track = trackObject;
    fixedStepTime = fixedStep;
    maxStepsPerFrame = maxSteps;
    dragCoefficient = drag;
    restitution = elasticity;
    collisionShape = shapeMode;
    collisionRadius = radius;
    respawnPosition = startPosition;
    respawnHeading = startForward;
    groundProbeMask = probeMask;
    if (terrainProbe)
        terrainScale = ((SoultreeProbe*)terrainProbe)->worldScale;
    collisionPointCapacity = contactCapacity;
    if (contactCapacity > 0) {
        collisionPoints = new(SP_FILE, 0x202) SoultreeContact*[contactCapacity];
        for (int i = 0; i < collisionPointCapacity; i++)
            collisionPoints[i] = 0;
    }
    attachmentCapacity = attachmentSlots;
    if (attachmentSlots > 0)
        attachments = new(SP_FILE, 0x20d) SoultreeAttachment[attachmentSlots];
    staggerPhase = g_SoultreeInstanceCounter;
    staggerCounter = 0;
    if (++g_SoultreeInstanceCounter > 5)
        g_SoultreeInstanceCounter = 0;
    respawnPending = 0;
    stepRemainder = 0.0f;
    UnknownVirtualSlot1(0.0f);
    sceneNode->SetPosition(0.0f, 0.0f, 0.0f);
    sceneNode->SetAxes(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0, 1);
    localCenterOfMass = g_Zero;
    rotationPivot = g_Zero;

    Vec3 extents;
    Vec3 center;
    sceneNode->GetSubtreeBounds(&center, &extents);
    float mass = bodyMass;
    Vec3* inertia = &shapeInvInertia;   // always non-null; the test only steers codegen
    float ident[9];
    memset(ident, 0, sizeof(ident));
    ident[0] = ident[4] = ident[8] = 1.0f;
    // Inverse inertia diagonal (tier 3 names; the formulas are decoded).  Sphere mode
    // (shapeMode == 1): I = 0.4 * m * r^2 (solid sphere), stored as 1/I on all three axes.  Otherwise a
    // solid box of full sizes (sx,sy,sz) = 2 * extents: I_x = m/12 * (sy^2 + sz^2), etc.
    if (inertia) {
        if (shapeMode == 1) {
            float inv = 1.0f / ((mass * 0.4f) * (radius * radius));
            inertia->x = inv;
            inertia->y = inv;
            inertia->z = inv;
        } else {
            float sx = extents.x + extents.x;
            float sy = extents.y + extents.y;
            float sz = extents.z + extents.z;
            float zz = sz * sz;
            float yy = sy * sy;
            float boxScale = mass * (1.0f / 12.0f);
            inertia->x = 1.0f / ((yy + zz) * boxScale);
            float xx = sx * sx;
            inertia->y = 1.0f / ((xx + zz) * boxScale);
            inertia->z = 1.0f / ((xx + yy) * boxScale);
        }
    }
    memcpy(field_0x164, ident, sizeof(ident));
    invInertia = shapeInvInertia;

    int prevTag = g_MemTagStack->Push("Collision");
    if (createCollision) {
        collisionObject = new(SP_FILE, 0x245) CollisionObject(1);
        InstallCollisionCallbacks();
        collisionObject->Configure((void*)renderContext, 0, 1, 1);
    } else {
        collisionObject = 0;
    }
    g_MemTagStack->Pop(prevTag);

    bodyForward = startForward;
    bodyUp = startUp;
    sceneNode->SetPosition(startPosition.x, startPosition.y, startPosition.z);
    sceneNode->GetPosition(&position);
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll, &bodySinRoll,
              &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
    if (createCenterNode) {
        centerNode = new(SP_FILE, 0x266) SoultreeObject(1);
        sceneNode->AddChild(centerNode);
        centerNode->SetPosition(center);
        centerNode->GetPositionIn(0, &centerOfMass);
    } else {
        sceneNode->GetPositionIn(0, &centerOfMass);
    }
    justReset = 1;
    respawnPending = 0;
    attachmentResetPending = 1;
    return this;
}

// ==== wave 3 (fork B): constructor, destructor, collision callbacks, frame step ====

// ---- collision callbacks (0x00500c00, 0x00500c30) ----------------------------------------
// Installed into the CollisionObject at field_0x128 by InstallCollisionCallbacks (+0x88 / +0x8c).  The
// collision object's field_0x60 is its owner (this body) and the other object's field_0x64
// its type tag (0x68 = another physics body).  The tag is recorded in field_0x134 and
// forwarded to slot 38 together with the other collision object (tier 1 data flow).
static void SoultreeCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    SoultreePhysicsBaseObject* body = (SoultreePhysicsBaseObject*)self->ownerObject;
    int kind = other->ownerType;
    body->lastCollisionType = kind;
    body->UnknownVirtualSlot38(0, kind, other);
}

// Second callback: the same response, except for body-to-body contacts (tag 0x68).
static void SoultreeStaticCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    if (other->ownerType != 0x68)
        SoultreeCollisionCallback(self, other);
}

// 0x00501230
void SoultreePhysicsBaseObject::InstallCollisionCallbacks()
{
    collisionObject->onHitCallback = SoultreeCollisionCallback;
    collisionObject->onHitByCallback = SoultreeStaticCollisionCallback;
}

// ---- constructor (0x00500aa0) / destructor (0x00501260) ---------------------------------
// Defining them here also makes VC6 emit the vbase deleting destructor 0x00504290, its
// vtordisp thunk 0x00504280 and the GameObject slot 10 vtordisp thunk 0x005042d0.
SoultreePhysicsBaseObject::SoultreePhysicsBaseObject(int flags)
    : GameObject(flags)
{
    restContactThreshold = 1;
    asleep = 0;
    field_0x28 = 1.0f;
    loadWeight = 0.0f;
    weightForce = g_Zero;
    terrainScale = 1.0f;
    terrain = 0;
    particleManager = 0;
    inShadow = 0;
    shadowLight = 0;
    attachmentResetPending = 0;
    centerNode = 0;
    attachments = 0;
    attachmentCount = 0;
    attachmentCapacity = 0;
    stepTime = 0.025f;          // default step 1/40 s ...
    invStepTime = 40.0f;           // ... and its reciprocal
    frameTime = 0.0f;
    lastStepTime = 0.025f;
    track = 0;
    collisionPoints = 0;
    collisionPointCapacity = 0;
    collisionPointCount = 0;
    touchingPointCount = 0;
    field_0x20e = 0;
    justReset = 0;
    groundProbeMask = 0;
    UnknownVirtualSlot1(0.0f);
}

SoultreePhysicsBaseObject::~SoultreePhysicsBaseObject()
{
    if (collisionPoints) {
        for (int i = 0; i < collisionPointCapacity; i++) {
            if (collisionPoints[i] && collisionPoints[i]->ownerNode)
                delete collisionPoints[i];
        }
        delete collisionPoints;
        collisionPoints = 0;
    }
    if (attachments) {
        delete attachments;
        attachments = 0;
    }
}

// ---- rest / settle check (0x00502c40) ------------------------------------------------------
// Called once per frame by RunSteps.  Tier 3 reading: when the body is slow (speed
// field_0xbc < 2) and its angular velocity field_0xd8 is small on every axis, with at least
// field_0x210 - 1 active contacts (field_0x1cc), the timer field_0x214 runs; after 0.5 s,
// on more than one contact or on a single near-flat one (normal y > 0.95), the body is put
// to sleep: velocities zeroed and field_0x10a set (GameObject slot 10 then skips the step).
// A body slower than 5 with enough contacts is damped instead (angular 0.95 per frame, and
// after 3 s angular 0.5 and linear 0.95).
static inline float AbsF(float v)
{
    if (v < 0.0f)
        v = -v;
    return v;
}

void SoultreePhysicsBaseObject::UpdateRestState()
{
    int spinSlow;
    if (linearSpeed < 5.0f && AbsF(angularVelocity.y) < 0.98f && AbsF(angularVelocity.x) < 0.98f &&
        AbsF(angularVelocity.z) < 0.98f)
        spinSlow = 1;
    else
        spinSlow = 0;
    if (linearSpeed < 2.0f && spinSlow && !airborne) {
        if (touchingPointCount < restContactThreshold && restTimer <= 0.5f) {
            if (touchingPointCount >= restContactThreshold - 1)
                restTimer += stepTime;
            return;
        }
        if (collisionPointCount > 1 || collisionPoints[0]->contactNormal.y > 0.95f) {
            velocity = g_Zero;
            linearSpeed = 0.0f;
            prevSpeed = 0.0f;
            angularVelocity = g_Zero;
            worldAngularVelocity = g_Zero;
            restTimer = 0.0f;
            asleep = 1;
        }
        return;
    }
    if (linearSpeed > 5.0f) {
        restTimer = 0.0f;
        return;
    }
    if (touchingPointCount < restContactThreshold - 1)
        return;
    restTimer += stepTime;
    if (restTimer > 3.0f) {
        angularVelocity *= 0.5f;
        velocity *= 0.95f;
    } else {
        angularVelocity *= 0.95f;
    }
}

// ---- GameObject slot 10 override (0x005036f0, via vtordisp thunk 0x005042d0) ---------------
// 0x0043ad80 (cdecl, CollisionPoint.cpp; near miss in samples/physics/collision/CollisionPointNearMisses.cpp): refreshes every contact point
// from its owner node, counts the penetrating ones into *activeCount and returns nonzero
// when it ran (first argument zero: nothing done).  Parameter names tier 3.
int SoultreeRefreshContacts(int enabled, int* activeCount, int count, SoultreeContact** contacts,
                            SoultreeProbe* terrain, Vec3* center, int mode, float radius);

// Per-frame update: skipped while asleep (field_0x10a); otherwise slot 9 splits dt into
// fixed steps, slot 30 clears the contact flags, slot 39 handles a pending reset, the
// contacts are refreshed (field_0x1d0 = any contact active), RunSteps runs the steps and
// slot 21 refreshes the attachments.  Tier 2 for the call structure.
int SoultreePhysicsBaseObject::Tick(float dt)
{
    int steps;
    int refreshed = 0;
    justReset = 0;
    if (asleep)
        return 1;
    UnknownVirtualSlot9(dt, &steps);
    UnknownVirtualSlot30();
    pointsTouching = 0;
    int respawn = UnknownVirtualSlot39(dt);
    if (!respawn) {
        refreshed = SoultreeRefreshContacts(UnknownVirtualSlot5(1), &touchingPointCount, collisionPointCount,
                                            collisionPoints, terrain, &centerOfMass, collisionShape,
                                            collisionRadius);
        if (refreshed)
            pointsTouching = touchingPointCount > 0;
    }
    RunSteps(steps, respawn, refreshed);
    UnknownVirtualSlot21();
    return 1;
}

// ---- SoultreePhysicsObject (0x005037c0, 0x005038d0, 0x00503c50) -------------------------
// The physics body that owns its own D3DIMSoultreeObject model node (the sibling of
// SoultreePhysicsCharacter).  The constructor makes that node the scene node; the
// destructor has no own cleanup (VC6 runs ~D3DIMSoultreeObject 0x0043f2b0 and
// ~SoultreePhysicsBaseObject 0x00501260).  Defining them here also emits the deleting
// destructor 0x00503890 and the vbase-vtable thunks 0x00504260/0x00504270.
SoultreePhysicsObject::SoultreePhysicsObject(int flags)
    : GameObject(1), SoultreePhysicsBaseObject(flags), D3DIMSoultreeObject(flags)
{
    sceneNode = (SoultreeObject*)(D3DIMSoultreeObject*)this;
}

SoultreePhysicsObject::~SoultreePhysicsObject()
{
}

// GameObject slot 10: the physics update (0x005036f0), then the model node's own update
// (D3DIMSoultreeObject's override 0x00443490) with the same frame time.
int SoultreePhysicsObject::Tick(float dt)
{
    SoultreePhysicsBaseObject::Tick(dt);
    return D3DIMSoultreeObject::Tick(dt);
}

// ---- slot 40 loaders (0x00503970 SoultreePhysicsObject, 0x00503de0 SoultreePhysicsCharacter) --
// Both build "<name without extension>.col" and look it up with new(SP_FILE, line) of the
// 0x134-byte file object; retail pushes this TU's __FILE__ string 0x00574320 at 0x00503b13 and
// 0x00503f87 (tier 1), which places both functions here.

// Shared archive stream and scene-file finder. The finder pointer is read
// from TrackGame+0x574; its complete layout is declared independently.
#include "../../reconstructed/FileStream.h"
#include "../../reconstructed/SceneArchiveFinder.h"

struct SoultreeGlobals {
    char field_0x000[0x574];
    UnknownTrackGameObject574* field_0x574;
};
class UnknownResourceManager;
extern UnknownResourceManager* g_ResourceManager; // pointer stored at 0x00572b44

// SoultreePhysicsObject slot 40 (0x00503970, `ret 0x70`).  Loads the model through the
// D3DIM object base (slot 9) and runs the physics setup (slot 2).  If the ".col" file is
// found it becomes the collision shape, otherwise the hull comes from the node (0x00432720).
// SoultreePhysicsObject has two GameObjects, so field_0x08 and the GameObject call are
// qualified.
GameObject* SoultreePhysicsObject::UnknownVirtualSlot40(int renderContext, const char* modelPath,
                                                        const SoultreeLoadDesc* desc, int a4,
                                                        Vec3 startPosition, Vec3 startForward,
                                                        Vec3 startUp, void* terrainProbe, void* linkedObject,
                                                        float weight, int contactCapacity, int attachmentSlots,
                                                        SoultreeSlot1f0* trackObject, float fixedStep,
                                                        int maxSteps, float drag, float elasticity,
                                                        float radius, int shapeMode, int restThreshold,
                                                        unsigned char probeMask, int createCollision)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeObject::D3DIMObjectVirtualSlot9(renderContext, modelPath, desc, a4, 1);
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(renderContext, 1, startPosition, startForward, startUp, terrainProbe, linkedObject, weight, contactCapacity, attachmentSlots,
                                                   trackObject, fixedStep, maxSteps, drag, elasticity, radius, shapeMode, restThreshold,
                                                   probeMask, createCollision);
    if (desc && (desc->field_0x25 & 1)) {
        reinterpret_cast<D3DIMSoultreeObject*>(SoultreePhysicsBaseObject::sceneNode)->SetUseLighting(1);
    }

    int len = strlen(modelPath);
    int copyLength = len > 0x103 ? 0x103 : len;
    strncpy(name, modelPath, copyLength);
    name[copyLength] = 0;
    strcpy(strrchr(name, '.'), ".col");

    UnknownTextureStream* file = new(SP_FILE, 0x84e) UnknownTextureStream(g_ResourceManager);
    if (!reinterpret_cast<SoultreeGlobals*>(g_TrackGame)->field_0x574->OpenArchiveFile(file, name, "rb", (int)colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (collisionObject) {
        if (colPath[0]) {
            collisionObject->LoadShape(SoultreePhysicsBaseObject::sceneNode, colPath);
        } else {
            collisionObject->SetHullShape(SoultreePhysicsBaseObject::sceneNode, 1, 0, 0, 0);
        }
        collisionObject->ownerType = 0;
        collisionObject->ownerObject = this;
        collisionObject->SnapPlacement();
        GameObject* child = collisionObject;
        D3DIMSoultreeObject::AppendChild(child, -1);
    }
    return static_cast<D3DIMSoultreeObject*>(this);
}

// SoultreePhysicsCharacter slot 40 (0x00503de0, `ret 0x6c`).  The same loader for the
// character: D3DIM character slot 11 instead of the object's slot 9, line 0x8cd, the pose
// node set from the model node, and shape setup 0x004324b0 when no ".col" file exists.
GameObject* SoultreePhysicsCharacter::UnknownVirtualSlot40(int renderContext, int createCenterNode, const char* modelPath,
                                                           const SoultreeLoadDesc* desc, int a5,
                                                           Vec3 startPosition, Vec3 startForward,
                                                           Vec3 startUp, void* terrainProbe, void* linkedObject,
                                                           float weight, int contactCapacity, int attachmentSlots,
                                                           SoultreeSlot1f0* trackObject, float fixedStep,
                                                           int maxSteps, float drag, float elasticity,
                                                           int restThreshold, unsigned char probeMask, int createCollision)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeCharacter::D3DIMVirtualSlot11(renderContext, modelPath, desc, a5, 1, 1);
    poseNode = sceneNode = modelNode;
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(renderContext, createCenterNode, startPosition, startForward, startUp, terrainProbe, linkedObject, weight, contactCapacity, attachmentSlots,
                                                   trackObject, fixedStep, maxSteps, drag, elasticity, 0.0f, 0, restThreshold, probeMask,
                                                   createCollision);
    if (desc && (desc->field_0x25 & 1)) {
        reinterpret_cast<D3DIMSoultreeObject*>(sceneNode)->SetUseLighting(1);
    }

    int len = strlen(modelPath);
    int copyLength = len > 0x103 ? 0x103 : len;
    strncpy(name, modelPath, copyLength);
    name[copyLength] = 0;
    strcpy(strrchr(name, '.'), ".col");

    UnknownTextureStream* file = new(SP_FILE, 0x8cd) UnknownTextureStream(g_ResourceManager);
    if (!reinterpret_cast<SoultreeGlobals*>(g_TrackGame)->field_0x574->OpenArchiveFile(file, name, "rb", (int)colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (collisionObject) {
        if (colPath[0]) {
            collisionObject->LoadShape(sceneNode, colPath);
        } else {
            collisionObject->SetModelShape(sceneNode, 1, 0, 0, 0);
        }
        collisionObject->ownerType = 0;
        collisionObject->ownerObject = this;
        collisionObject->SnapPlacement();
        GameObject* child = collisionObject;
        GameObject::AppendChild(child, -1);
    }
    return this;
}

// ---- per-frame step loop (0x00502f60) ----------------------------------------------------
// 0x0043aa30 (cdecl, collision/CollisionObject.cpp area): merges the penetrating contacts
// into one response (mean position, summed normal, deepest penetration); the three outputs
// feed slot 3.  0x0043aff0 (cdecl) resets the contact records when the refresh did not run.
// Parameter names tier 3; the Vehicle step 0x0052a940 calls the same three helpers.
void SoultreeMergeContacts(int count, SoultreeContact** contacts, Vec3* scale, Vec3* angVel,
                           Vec3* vel, Vec3* center, Vec3* pos, Vec3* outArm, Vec3* outNormal,
                           Vec3* outVel);
void SoultreeResetContacts(int count, SoultreeContact** contacts);

// Squared length with the components read into locals first (retail loads y, x, z; the
// same shape as Vec3Normalize 0x005087b0).
static inline float SquareMagnitudeYXZ(const Vec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    return z * z + (x * x + y * y);
}

// Length through the reciprocal square-root table: 0 and 1 are returned for squared
// lengths of exactly 0 and 1.
static inline float VecLengthRsq(const Vec3& v)
{
    float sq = SquareMagnitudeYXZ(v);
    if (sq == 0.0f)
        return 0.0f;
    if (sq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(sq);
}

// Runs `steps` fixed sub-steps (tier 3 reading, call structure tier 1).  Each step:
// records the previous speed, gathers the contacts (refresh 0x0043ad80, slots 8/6/7/31),
// merges the penetrating contacts into one impulse (slot 3) with a speed clamp, integrates
// the forces (slots 13/14, rest check), advances the node position by the Adams-Bashforth
// estimate (3v - v_prev)/2 * dt, rotates the node about the pivot by |w| * dt, then
// refreshes the cached world state (slots 28/34/29, orientation angles).  A leftover
// remainder after the last step is run as one extra short step.  `refreshed` is unused.
// While slot 39 asks for a respawn (`respawn`), the placement search (slot 11) and reset (slot 33)
// replace the step.
// Exact once `mergedNormal` lives at loop scope.  The respawn block's locals then share
// slots with its sibling blocks as in retail: keepPlacement with sq, placedForward with
// mergedArm (-76), placedUp with mergedVelocity (-52), and placedPosition is left for the
// integration result temporary (-136); mergedNormal gets its own slot next to impulse.
// With all three outputs in the touching block, VC6 pairs placedPosition/mergedArm,
// placedForward/mergedVelocity, placedUp/mergedNormal and the cold respawn block needs
// disp32 displacements (1804/1930).
void SoultreePhysicsBaseObject::RunSteps(int steps, int respawn, int refreshed)
{
    Vec3 force;
    float speed;

    if (steps <= 0)
        return;
    do {
        int res;
        Vec3 worldTorque = kVec3Zero;
        prevSpeed = linearSpeed;
        if (prevSpeed < 0.001f || !_finite(prevSpeed)) {
            prevSpeed = 0.0f;
            velocity = kVec3Zero;
            linearSpeed = 0.0f;
        }
        if (respawn) {
            int keepPlacement;
            Vec3 placedPosition, placedForward, placedUp;
            UnknownVirtualSlot11(respawn, &placedPosition, &placedForward, &placedUp, &keepPlacement);
            UnknownVirtualSlot33(&placedPosition, &placedForward, &bodyUp, &placedUp, keepPlacement, UnknownVirtualSlot32());
            stepRemainder = 0.0f;
            break;
        }
        if (centerNode)
            centerNode->GetPositionIn(0, &centerOfMass);
        else
            sceneNode->GetPositionIn(0, &centerOfMass);
        speed = prevSpeed;
        force = weightForce;
        res = SoultreeRefreshContacts(UnknownVirtualSlot5(0), &touchingPointCount, collisionPointCount,
                                      collisionPoints, terrain, &centerOfMass, collisionShape,
                                      collisionRadius);
        if (res)
            pointsTouching = touchingPointCount > 0;
        UnknownVirtualSlot8();
        UnknownVirtualSlot6(&force, &speed);
        UnknownVirtualSlot7(&force);
        airborne = !pointsTouching;
        if (pointsTouching) {
            if (!res)
                UpdateCollisionPointWorldPositions(collisionPointCount,
                    reinterpret_cast<CollisionPoint**>(collisionPoints));
            UnknownVirtualSlot31();
        }
        Vec3 mergedNormal;
        if (pointsTouching) {
            Vec3 scale;
            Vec3 mergedVelocity, mergedArm;
            float impulse;
            scale = Vec3(1.0f, 1.0f, 1.0f);
            SoultreeMergeContacts(collisionPointCount, collisionPoints, &scale, &worldAngularVelocity,
                                  &velocity, &centerOfMass, &position, &mergedArm, &mergedNormal,
                                  &mergedVelocity);
            UnknownVirtualSlot3(&mergedNormal, &mergedVelocity, &mergedArm, &scale, 0x67, 0, &impulse);
            if (prevSpeed * 5.0f < linearSpeed && linearSpeed > 5.0f) {
                velocity *= prevSpeed / linearSpeed;
                linearSpeed = prevSpeed;
            }
            if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
                velocity = kVec3Zero;
                linearSpeed = 0.0f;
            }
        }
        Vec3 localTorque = kVec3Zero;
        UnknownVirtualSlot13(&force, &localTorque, 1.0f);
        UpdateRestState();
        UnknownVirtualSlot14(&force, &localTorque, &worldTorque);
        UnknownVirtualSlot26();
        position += (velocity * 3.0f - prevVelocity) * 0.5f * stepTime;
        sceneNode->SetPosition(position);
        float mag = VecLengthRsq(angularVelocity) * stepTime;
        if (_finite(mag) && mag >= 0.0001f) {
            scratchVector = angularVelocity;
            float sq = SquareMagnitudeYXZ(scratchVector);
            if (sq == 0.0f) {
                scratchVector = kVec3Zero;
            } else {
                float inv = FastInvSqrt(sq);
                scratchVector.x *= inv;
                scratchVector.y *= inv;
                scratchVector.z *= inv;
            }
            if (mag > 2.0f) {
                mag *= 0.95f;
                angularVelocity *= 0.95f;
            }
            sceneNode->RotateAboutPoint(rotationPivot, scratchVector, mag);
        }
        sceneNode->GetPosition(&position);
        if (centerNode)
            centerNode->GetPositionIn(0, &centerOfMass);
        else
            centerOfMass = position;
        worldAngularVelocity = sceneNode->LocalToWorldDirection(angularVelocity);
        steps = UnknownVirtualSlot28(steps);
        lastStepTime = stepTime;
        steps--;
        if (steps == 0 && stepRemainder > 0.0001f) {
            float t = stepRemainder;
            steps = 1;
            stepRemainder = 0.0f;
            stepTime = t;
            invStepTime = 1.0f / t;
        }
        UnknownVirtualSlot34();
        OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll, &bodySinRoll,
                                     &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
        prevVelocity = velocity;
        UnknownVirtualSlot29(steps == 0);
    } while (steps > 0);
}

// 0x00500220 (cdecl, called by slot 3): impulse response of one contact.  Tier 3 reading
// (decoded arithmetic, tier 1): normal = contact normal, relVel = relative contact
// velocity, arm = contact point relative to the centre of mass.  When the contact is
// approaching (relVel . normal < 0) the impulse j = -(1 + restitution) * approach /
// (invMass + n . ((I^-1 (arm x n)) x arm)) is written to *impulseOut, the linear velocity
// gains n * j * invMass (j scaled first when the scale argument is non-zero), and the
// body-space angular velocity gains I^-1 (arm x n) * j, scaled per axis by *angScale.
// Slot 3 passes field_0x14c, field_0x24, field_0x08, &field_0xe4 (body-space inverse
// inertia diagonal), &field_0xd8 and &field_0x64.
// Matching levers (541/639, size exact): the denominator dot is SoultreeDot (retail sums
// (x + y) + z), and `scaled` is assigned member-wise; the constructor form costs 4 bytes.
void ResolveContactImpulse(float restitution, float invMass, SoultreeObject* node, const Vec3* normal,
               const Vec3* relVel, const Vec3* arm, Vec3* invInertia, const Vec3* angScale,
               Vec3* angVel, Vec3* velocity, float* impulseOut, int scaleBits)
{
    // The last argument holds a float (fcomp/fmul as a single); slot 3 forwards it as
    // its int a6, so it is reinterpreted here.
    float scale = *(float*)&scaleBits;
    float approach = SoultreeDot(*relVel, *normal);
    if (approach < 0.0) {
        Vec3 axis = SoultreeCross(*arm, *normal);
        Vec3 local = node->WorldToLocalDirection(axis);
        Vec3 scaled;
        scaled.x = local.x * invInertia->x;
        scaled.y = local.y * invInertia->y;
        scaled.z = local.z * invInertia->z;
        Vec3 world = node->LocalToWorldDirection(scaled);
        Vec3 c = CrossProduct(world, *arm);
        float j = ((restitution + 1.0f) * -approach) / (SoultreeDot(c, *normal) + invMass);
        *impulseOut = j;
        Vec3 impulse;
        if (scale != 0.0f)
            impulse = *normal * (j * scale);
        else
            impulse = *normal * j;
        *velocity += impulse * invMass;
        Vec3 spin = *impulseOut * scaled;
        spin.x *= angScale->x;
        spin.y *= angScale->y;
        spin.z *= angScale->z;
        *angVel += spin;
    }
}
