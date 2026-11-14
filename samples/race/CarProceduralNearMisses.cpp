// Near-miss CarProcedural.cpp candidates, kept out of src/reconstructed until
// they match. They use src/reconstructed/CarProcedural.h; the bindings are in
// CarProceduralNearMisses.bindings.json. Scores are matching bytes under
// vc6_o2_mt with relocations resolved.
//
// CarProcedural::PositionAtTime (0x00430b10, 842 bytes; 299 of 842):
// the path evaluation. The key search loops, the Hermite weights call (its
// fourth weight written over the `time` parameter, as retail's
// `lea ecx,[esp+0x60]` shows), the tangent temporaries and the final
// h0*fromKey + h1*toKey + t0*h2 + t1*h3 sum order match. Retail inlines all fourteen
// vector operators; written with operators throughout, VC6 runs out of
// inline budget (docs/PHYSICS_VALIDATION.md) and calls the Vector3
// constructor and operator+ out of line. With the chord written per
// component (below) everything inlines, but VC6 then keeps `this` in edi and
// the key count in esi (retail: esi, edi) and addresses the second key
// through its base instead of retail's `lea ecx,[key + 4]` (the inlined
// operator-'s reference). Named intermediate vectors barely change the
// budget. The budget follows the caller's front-end tree size
// (docs/VC6_INLINE_BUDGET.md): about 30 extra trivial statements (dead
// code counts, empty statements do not) make VC6 expand all 14 operators
// and match retail's first 0x19b bytes; the tangent temporaries' slots
// then differ. No padded source is kept.
//
// CarProcedural::Tick (0x0042fd80, 2909 bytes): the
// per-frame update along the path (wheel spin and steering, the collision
// objects' transform and the body hull position). 719 of retail's 782
// instructions align (92%). Retail facts the source follows: the sensor contact's
// normal is at +0x08 (`add ecx, 8`); the back and steering points subtract
// pathPosition straight from the call result (no named copy) and keep the
// time offset on the x87 stack (`fld t; fsub st(1)` ... `fstp st(0)`, a
// float local); the collider frame is built by an inline helper taking
// the axes by value (its locals share slots with the blocks' vectors); the
// target is built per component (no y sum) and the 4fd5c0 result is a
// named copy; the body hull position is corrected per component through one
// `body` pointer to the shape (aligned 713 -> 719, candidate 2865 -> 2880 bytes,
// over a `Vector3&` to its position). The wheel loop's axisA is declared at
// function scope (axisB stays in the loop): the frame becomes retail's 0xdc and
// strict goes 220 -> 580 of 2909 (aligned 716 -> 719 of 782); axisB at function
// scope instead gives 226, both axes 0xe8. Score: strict 580/2909, aligned
// 719/782, candidate 2880 bytes, 63 instruction hunks; the game links this body
// (src/bringup/Group2_CarProcedural.cpp compiles the same Tick through
// CarProcedural.h's views, bound by config/game_build.json link_aliases).
// Retail's slot groups (frame offsets) are {temps, local, target, first-update
// position} 0x0, {side, forward axis} 0xc, t 0x18, {box, ahead, right axis,
// offset} 0x1c, {back, up axis} 0x34, a float spill 0x40, normal 0x44, {flat,
// axisA} 0x50, position 0x5c, ground 0x68, forward 0x74, result 0x80, axisB
// 0x84, placement 0x90, transform 0x9c; a bare block or an inline helper around
// flat moves it into the box group or runs out of inline budget (operator-
// called). Left: the steering clamp stores the angle to memory in retail (`fst
// [esi+0x190]; fst [t's slot]; fcomp lo`, reloaded twice, the `fld lo` arm out
// of line); by-value, const-reference, member-argument, expression-argument and
// nested clamp helpers, the if/else, `if (a > lo)` and ternary forms, a chained
// `a = steeringAngle = atan2(...)`, reusing t, a pointer alias and a
// `(float)(x * 1.0)` round trip all keep it on the x87 stack (a volatile local
// reaches 699 strict but is a matching trick, not kept). Also left: the
// normalisation of `back` loads its components first in retail, the 1f4 blend
// the scalar first (here the reverse), load/store scheduling in the
// forward/path-velocity section, and argument-push scheduling around the
// steering call (OrientationAxesFromAngles: a float roll argument or a static
// definition is inert), the frame helper's copies and the 4fd5c0 call. Earlier
// forms: one named ahead for all three path points, the frame written in the
// body and the contact normal at +0x0c gave 807/3018 with frame 0xf4; frame 0xd0
// with both axes in the loop gave 221 strict (196 masked before GetPosition was
// bound).

#include <math.h>

#include "../../src/reconstructed/CarProcedural.h"
#include "../../src/reconstructed/D3DIMSoulTree.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);

// Constructor-free helpers: with CarProcedural.cpp's constructor forms even
// the per-component chord leaves the function over budget.
static inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}
static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// 0x00430b10: the path position at `time` (wrapped to the path's period):
// a cubic Hermite segment between the keys around it, whose tangents are
// the neighbouring chords scaled by the key spacing.
Vector3 CarProcedural::PositionAtTime(float time)
{
    unsigned int count = pathKeyCount;
    CarProceduralKey* keys = pathKeys;
    float period = keys[count - 1].time * inversePathSpeed;
    float t = fmod(time, period);
    if (t < 0.0)
        t += period;
    if (count > 0) {
        t *= pathSpeed;
        unsigned int next = currentPathKey + 1;
        while (next < count && t > keys[next].time) {
            currentPathKey++;
            next++;
        }
        while (currentPathKey > 0 && t < keys[currentPathKey].time) {
            currentPathKey--;
            next--;
        }
        // The fourth weight is written over `time`.
        float h0, h1, h2;
        HermiteWeights((t - keys[currentPathKey].time) / (keys[next].time - keys[currentPathKey].time),
                       &h0, &h1, &h2, &time);

        CarProceduralKey* fromKey = &pathKeys[currentPathKey];
        CarProceduralKey* toKey = &pathKeys[next];
        Vector3 chord;
        chord.x = toKey->position.x - fromKey->position.x;
        chord.y = toKey->position.y - fromKey->position.y;
        chord.z = toKey->position.z - fromKey->position.z;
        Vector3 t0;
        if (currentPathKey == 0) {
            t0 = chord;
        } else {
            float spacing = (toKey->time - fromKey->time) / (toKey->time - fromKey[-1].time);
            t0 = ((fromKey->position - fromKey[-1].position) + chord) * spacing;
        }
        Vector3 t1;
        if (next == pathKeyCount - 1) {
            t1 = chord;
        } else {
            float spacing = (toKey->time - fromKey->time) / (toKey[1].time - fromKey->time);
            t1 = ((toKey[1].position - toKey->position) + chord) * spacing;
        }
        return h0 * fromKey->position + h1 * toKey->position + t0 * h2 + t1 * time;
    }
    return kVec3Zero;
}


// ---------------------------------------------------------------------------
// Slot 10, 0x0042fd80 (2907 bytes): the per-frame update along the path.
// ---------------------------------------------------------------------------

static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x004b5d00 (cdecl): the look and up vectors of a heading and pitch
// (samples/track/SceneManagerNearMisses.cpp declares the same function).
void OrientationAxesFromAngles(Vector3* look, Vector3* up, float heading, float pitch, int a);

// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the orthonormalisation
// of the collider frame calls the COMDAT cross product and normalisation.
Vector3 CarCrossCall(const Vector3& a, const Vector3& b);                 // 0x00515600
Vector3 CarNormalizeCall(const Vector3& v);                               // 0x005087b0

static inline Vector3 CarCross(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
static inline float CarDot(const Vector3& a, const Vector3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}
// |v| as 0x005087b0 reads its components (y, x, z), exact for unit vectors.
static inline float CarLength(const Vector3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float squared = z * z + (x * x + y * y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}
// v / |v| in place; the zero vector stays zero.
static inline void CarNormalizeInPlace(Vector3& v)
{
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 0.0f) {
        v = kVec3Zero;
    } else {
        float inv = FastInvSqrt(squared);
        v.x *= inv;
        v.y *= inv;
        v.z *= inv;
    }
}
static inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// The orthonormal frame of a forward and an up direction at a position.
static inline void CarFrame(Matrix4* m, Vector3 forwardAxis, Vector3 upAxis, const Vector3& position)
{
    Vector3 rightAxis = CarCross(upAxis, forwardAxis);
    upAxis = CarCrossCall(forwardAxis, rightAxis);
    upAxis = CarNormalizeCall(upAxis);
    forwardAxis = CarNormalizeCall(forwardAxis);
    rightAxis = CarCrossCall(upAxis, forwardAxis);
    (*m)(0, 0) = rightAxis.x;
    (*m)(0, 1) = rightAxis.y;
    (*m)(0, 2) = rightAxis.z;
    (*m)(0, 3) = 0.0f;
    (*m)(1, 0) = upAxis.x;
    (*m)(1, 1) = upAxis.y;
    (*m)(1, 2) = upAxis.z;
    (*m)(1, 3) = 0.0f;
    (*m)(2, 0) = forwardAxis.x;
    (*m)(2, 1) = forwardAxis.y;
    (*m)(2, 2) = forwardAxis.z;
    (*m)(2, 3) = 0.0f;
    (*m)(3, 0) = position.x;
    (*m)(3, 1) = position.y;
    (*m)(3, 2) = position.z;
    (*m)(3, 3) = 1.0f;
}

// The sensor collider's contact record: the penetration depth at +0 and
// the surface normal at +0x08 (tier 3 roles).
struct CarProceduralSensorContact {
    float fraction;   // +0x00 CollisionBoxResult fraction
    float point;      // +0x04 CollisionBoxResult point
    Vector3 normal;
};

// v limited to [lo, hi].
static inline float CarClamp(float v, float lo, float hi)
{
    return v > lo ? (v < hi ? v : hi) : lo;
}

int CarProcedural::Tick(float frameTime)
{
    Vector3 axisA;
    int result = GameObject::Tick(frameTime);
    if (frameTime <= 0.0)
        return result;
    if (!started) {
        // First update: a two-point mesh for the sensor collider, and the
        // wheels' height from the first wheel above the origin.
        Vector3 box[2];
        box[0] = kVec3Zero;
        box[0].y += sensorHalfHeight;
        box[1] = kVec3Zero;
        box[1].y -= sensorHalfHeight;
        sensorCollider->SetMeshShape(1, box);
        for (unsigned int i = 0; i < numWheels; i++) {
            Vector3 position;
            reinterpret_cast<SoultreeObject*>(wheelNodes[i])->GetPosition(reinterpret_cast<Vec3*>(&position));
            if (position.z > 0.0)
                frontWheelZ = position.z;
        }
        started = 1;
    }
    pathTime = (float)fmod(frameTime + pathTime, pathKeys[pathKeyCount - 1].time * inversePathSpeed);
    float t;
    if (trailDistance > 0.0) {
        trailTimeOffset = FindOffsetAtDistance(pathTime, trailDistance);
        t = pathTime - trailTimeOffset;
        pathPosition = PositionAtTime(t);
    } else {
        trailTimeOffset = 0.0f;
        t = pathTime;
        pathPosition = PositionAtTime(t);
    }
    Vector3 ahead = PositionAtTime(t + lookAheadTime);
    Vector3 forward = ahead - pathPosition;
    forward.y = 0.0f;
    pathVelocity = forward * velocityScale;
    groundSpeed = CarLength(pathVelocity);
    Vector3 local = carModel->WorldToLocalDirection(forward);
    if (numWheels > 0)
        wheelSpinAngle = (float)fmod(inverseWheelRadius * velocityScale * local.z * frameTime + wheelSpinAngle, 6.2831855f);
    float behind = FindOffsetAtDistance(t, -1.0f);
    Vector3 back = PositionAtTime(t - behind) - pathPosition;
    CarNormalizeInPlace(back);
    if (skipSmoothing)
        smoothedBack = back;
    else
        smoothedBack = smoothedBack * backBlend + back * (1.0f - backBlend);
    if (steersFrontWheels && numWheels > 0) {
        float wheelbase = FindOffsetAtDistance(t, -frontWheelZ);
        Vector3 side = PositionAtTime(t - wheelbase) - pathPosition;
        Vector3 steer = carModel->WorldToLocalDirection(side);
        steeringAngle = (float)atan2(steer.x, steer.z);
        steeringAngle = CarClamp(steeringAngle, -0.52359873f, 0.52359873f);
    }
    Vector3 position;
    carModel->GetPositionIn(0, &position);
    Vector3 ground = position;
    Vector3 normal;
    ((CarProceduralTerrain*)groundProvider)->QueryGround(&ground, &normal, 0, 0);
    // The sensor collider's frame: the world axes orthonormalised.
    Matrix4 transform;
    CarFrame(&transform, kVec3ZAxis, kVec3YAxis, position);
    sensorCollider->SetTransform(&transform);
    sensorCollider->QueryCollisions();
    if (sensorCollider->hasContact) {
        CarProceduralContact* contact = (CarProceduralContact*)sensorCollider->contactRecord;
        Vector3 lifted = kVec3YAxis * ((1.0f - contact->contactOffset.x) * sensorHalfHeight * 2.0f - sensorHalfHeight) + position;
        if (lifted.y > ground.y) {
            ground = lifted;
            normal = ((CarProceduralSensorContact*)contact)->normal;
        }
    }
    if (skipSmoothing)
        smoothedNormal = normal;
    else
        smoothedNormal = smoothedNormal * normalBlend + normal * (1.0f - normalBlend);
    Vector3 flat = smoothedBack - smoothedNormal * CarDot(smoothedBack, smoothedNormal);
    carModel->SetAxesPtr(&flat, &smoothedNormal, 0, 1);
    Vector3 target(pathOffset.x + pathPosition.x, ground.y, pathOffset.z + pathPosition.z);
    Vector3 offset = carModel->LocalToWorldDirection(bodyOffset);
    Vector3 placement = target - offset;
    carModel->SetPositionInFrame(0, &placement);
    for (unsigned int i = 0; i < numWheels; i++) {
        Vector3 axisB;
        if (steersFrontWheels && i < 2) {
            OrientationAxesFromAngles(&axisA, &axisB, steeringAngle, -wheelSpinAngle, 0);
        } else {
            axisA.x = 0.0f;
            axisA.y = (float)sin(-wheelSpinAngle);
            axisA.z = (float)cos(-wheelSpinAngle);
            axisB.x = 0.0f;
            axisB.y = axisA.z;
            axisB.z = -axisA.y;
        }
        wheelNodes[i]->SetAxesPtr(&axisA, &axisB, 0, 1);
    }
    bodyCollider->UpdatePlacement();
    if (bodyCollider->QueryCollisions()) {
        CarProceduralContact* contact = (CarProceduralContact*)bodyCollider->contactRecord;
        bodyContactPoint = contact->contactPoint;
        bodyContactNormal = contact->contactNormal;
        CarProceduralBody* body = (CarProceduralBody*)bodyCollider->shape;
        body->position.x -= contact->contactOffset.x * 1.005f;
        body->position.y -= contact->contactOffset.y * 1.005f;
        body->position.z -= contact->contactOffset.z * 1.005f;
    }
    return result;
}
