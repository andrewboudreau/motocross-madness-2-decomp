// Vehicle.cpp - reconstruction of the retail Vehicle translation unit
// (__FILE__ xrefs near 0x00525e98). See Vehicle.h for the evidence/provisional notes.
#include "vehicle/Vehicle.h"
#include "../../reconstructed/GearRatios.h"
#include "../../reconstructed/InputDevice.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "collision/CollisionObject.h"
#include "soultree/SoultreePhysicsObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "collision/CollisionPoint.h"
#include "../../reconstructed/ParticleEmitters.h"

void Vehicle::UnknownVirtualSlot0(float extraWeight)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot0(extraWeight);
    suspensionMass = (suspensionWeightScale * baseWeight + extraWeight) * 0.0310558993f;   // 1/32.2
}

int Vehicle::UnknownVirtualSlot42()
{
    return crashState == 0 && motionIndex >= 0 && motionActive;
}

void Vehicle::UnknownVirtualSlot50(int protect, float duration, int c)
{
    spawnProtected = protect;
    spawnProtectTimer = duration;
}

float Vehicle::UnknownVirtualSlot53()
{
    steerState->AddAngle(steerRate * stepTime, poseNode);
    return 1.0f;
}

Vec3* Vehicle::UnknownVirtualSlot55(Vec3* out, Vec3* pos)
{
    *pos = kVec3Zero;
    *out = savedForward;
    return out;
}

float Vehicle::UnknownVirtualSlot57() { return leanAngle; }
float Vehicle::UnknownVirtualSlot59() { return steerState->maxAngle; }
float Vehicle::UnknownVirtualSlot61(float arg) { return 0.0f; }

int Vehicle::UnknownVirtualSlot62(float arg)
{
    return anyWheelInContact && crashState == 0;
}

int Vehicle::UnknownVirtualSlot89(float frame)
{
    return UnknownVirtualSlot66();
}

void Vehicle::UnknownVirtualSlot93() { forceFeedbackGain = 0.5f; }

void Vehicle::UnknownVirtualSlot96()
{
    controlResponse[0] = 1.0f;
    controlResponse[1] = 1.0f;
    controlResponse[2] = 1.0f;
    controlResponse[3] = 1.0f;
    controlResponse[4] = 1.0f;
    controlResponse[5] = 1.0f;
}

void Vehicle::UnknownVirtualSlot21()
{
    SoultreePhysicsCharacter::UnknownVirtualSlot21();
    prevThrottleFraction = engineState->throttleFraction;
}

int Vehicle::UnknownVirtualSlot82()
{
    return inputMap->UnknownVirtualSlot3(0xe, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot81()
{
    return inputMap->UnknownVirtualSlot3(0x1d, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot84(int a, int b)
{
    return inputMap->UnknownVirtualSlot2(a, b);
}

// True when the speed change this step (linearSpeed - prevSpeed) is below dt * speedGainLimit;
// Bike slot 99 passes its clamped step time and treats a true answer as crash reason 4.
int Vehicle::UnknownVirtualSlot70(float dt)
{
    return linearSpeed - prevSpeed < dt * speedGainLimit;
}

// Provisional: wheelBase / sin(steer angle), zero when the angle is zero.
float Vehicle::UnknownVirtualSlot73(const Vec3* a, const Vec3* b)
{
    VehicleSteerState* s = steerState;
    float m = s->angle;
    if (m < 0.0f)
        m = -m;
    if (m == 0.0f)
        return 0.0f;
    return wheelBase / sinf(s->angle);
}

// Clears contactActive (+0xa4) on every contact that has an owner node, then resets state.
void Vehicle::UnknownVirtualSlot67()
{
    for (int i = 0; i < collisionPointCount; i++) {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->ownerNode != 0)
            c->contactActive = 0;
    }
    crashState = 0;
    crashTimer = 0.0f;
    prevCrashState = 0;
}

// Reads a float through the input map's value source (written back into the argument slot).
// Tier 2: retail tests the argument as an int (callers pass a bool/flag) and then reuses
// that same stack slot as the float out-buffer.  Retyping it as float (tried) changes the
// test to an FPU compare and the callers' pushes, and loses the exact match of slots
// 77-83, so it stays int with the (float*)&arg pun.
int Vehicle::UnknownVirtualSlot77(int arg)
{
    if (arg && inputMap->valueSource
        && inputMap->valueSource->UnknownVirtualSlot3(0, (float*)&arg)
        && *(float*)&arg != -1.0f)
        return 1;
    return 0;
}

void Vehicle::UnknownVirtualSlot64(float arg)
{
    if (crashState == 0) {
        UnknownVirtualSlot63(arg);
        return;
    }
    controlInput = kVec3Zero;
    steerInput = 0.0f;
    throttleInput = 0.0f;
}

// The drive torque arm x force, which AccumulateWheelContacts adds per driving wheel.
Vec3 Vehicle::UnknownVirtualSlot76(const Vec3* arm, const Vec3* force)
{
    Vec3 r;
    const Vec3& p = *force; const Vec3& q = *arm;
    r.x = p.z * q.y - p.y * q.z;
    r.y = p.x * q.z - p.z * q.x;
    r.z = p.y * q.x - p.x * q.y;
    return r;
}

void Vehicle::UnknownVirtualSlot90(int* steps, float frame)
{
    if (CheckCrash()) {
        *steps = 1;
        stepRemainder = 0.0f;
        return;
    }
    if (*steps == 1 && stepRemainder <= 0.0001f)
        UnknownVirtualSlot89(frame);
}

// steerRate turns the steer angle to -(steerAxis * maxAngle) over the remaining steps (plus
// stepRemainder); zero when the error is tiny or the vehicle is crashed.
void Vehicle::UnknownVirtualSlot60(float steerAxis, float maxAngle, int steps)
{
    if (crashState == 0) {
        steerRate = -(steerAxis * maxAngle) - steerState->angle;
        float m = steerRate;
        if (m < 0.0f) m = -m;
        if (m < 0.00001f) {
            steerRate = 0.0f;
        } else {
            if (stepRemainder > 0.0001f)
                steerRate = steerRate / ((float)steps * stepTime + stepRemainder);
            else
                steerRate = steerRate / ((float)steps * stepTime);
        }
    } else {
        steerRate = 0.0f;
    }
}

void Vehicle::UnknownVirtualSlot85()
{
    float v = movingForward ? stepTime : -stepTime;
    int flag = wheelsInContact == 0;
    for (int i = 0; i < wheelCount; i++)
        wheelList[i]->UpdateRoll(stepTime, v, flag, allWheelsInContact, crashState, invMass);
}

void Vehicle::UnknownVirtualSlot86()
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact) {
            if (w->aux) {
                float sp = linearSpeed;
                float scale = driveForceScale;
                reinterpret_cast<UnknownEngineGearbox*>(w->aux)->DriveForce(
                    sp, reinterpret_cast<const Vector3*>(&w->rollDirection), scale, backingUp, 100.0f,
                    reinterpret_cast<Vector3*>(&w->driveForce), &w->driveMagnitude);
            }
            w->UpdateDriveShare(this);
        }
    }
}

// Vec3 operators as a view over Vec3 (stand-in for common/Math3D.h).
struct VehVecOps : Vec3
{
    Vec3 operator*(float s) const { Vec3 r; r.x = x * s; r.y = y * s; r.z = z * s; return r; }
    Vec3 operator-(const Vec3& o) const { Vec3 r; r.x = x - o.x; r.y = y - o.y; r.z = z - o.z; return r; }
};

// Provisional: position += dt * (3*v - v_prev) / 2   (2-step Adams-Bashforth style advance:
// v = velocity, v_prev = prevVelocity, advancing position).
void Vehicle::UnknownVirtualSlot48()
{
    Vec3 a = ((VehVecOps&)velocity) * 3.0f;
    float dt = stepTime;
    Vec3 d = ((VehVecOps&)a) - prevVelocity;
    Vec3 e = ((VehVecOps&)d) * 0.5f;
    Vec3 f = ((VehVecOps&)e) * dt;
    position.x += f.x;
    position.y += f.y;
    position.z += f.z;
}

void Vehicle::UnknownVirtualSlot71(int nowAirborne)
{
    if (crashState) {
        landingLatched = 0;
        if (airborne && !nowAirborne)
            justLanded = 1;
    } else if (airborne && !nowAirborne && !landingLatched) {
        landingLatched = 1;
        justLanded = 1;
    }
    if (!airborne && nowAirborne) {
        takeoffVelocity = velocity;
        takeoffPosition = position;
    }
}

// Provisional: snapshot respawnPosition and a horizontal respawnHeading (savedForward
// with y forced to 0), normalised with FastInvSqrt; zero vector if degenerate.
static inline float VehLen2(const Vec3& v) { return (v.x * v.x + v.y * v.y) + v.z * v.z; }
void Vehicle::UnknownVirtualSlot43()
{
    respawnPosition = position;
    respawnHeading = savedForward;
    respawnHeading.y = 0.0f;
    float len2 = VehLen2(respawnHeading);
    if (len2 == 0.0f) {
        respawnHeading = kVec3Zero;
        return;
    }
    float inv = FastInvSqrt(len2);
    respawnHeading.x *= inv;
    respawnHeading.y *= inv;
    respawnHeading.z *= inv;
}

// ---- heavy dynamics ----

// Distributes the vector *arg (a force/impulse, tier 3) over the active contacts (or wheels).
// One active contact takes all of it; several share it by inverse-distance blending:
// weight = 1 - dist_i / sum(dist), so the nearest contact takes the biggest share.
// Wheel mode: motionActive gives every wheel its stored weight (+0x294); otherwise the same
// blending as for contacts using the wheels' contact points, or a single lead wheel.
// Retail leaves `last` uninitialised (its slot is the squared-distance temporary), declares
// the running total before the share temporary, and the single-wheel branch zeroes the share
// temporary in the loop with the single-value constructor `Vec3(0.0f)` (dead stores z, y, x)
// instead of copying kVec3Zero like the no-contact branch.
static inline Vec3 VehScale(const Vec3& v, float k)
{
    Vec3 r;
    r.x = v.x * k;
    r.y = v.y * k;
    r.z = v.z * k;
    return r;
}

static inline float VehDistance(const Vec3& a, const Vec3& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    float d2 = (dy * dy + dx * dx) + dz * dz;
    if (d2 == 1.0f)
        return 1.0f;
    return FastSqrt(d2);
}

void Vehicle::UnknownVirtualSlot7(const Vec3* force)
{
    float total;
    float dist[128];
    int i;
    int active;

    contactTotal = touchingPointCount + wheelsInContact;
    if (pointsTouching) {
        total = 0.0f;
        active = 0;
        int last;
        for (i = 0; i < collisionPointCount; i++) {
            VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
            if (c->contactActive) {
                dist[i] = VehDistance(centerOfMass, c->contactPosition);
                total += dist[i];
                active++;
                last = i;
            } else {
                c->blendWeight = 0.0f;
                c->appliedShare = kVec3Zero;
            }
        }
        if (active == 1) {
            ((VehicleContact**)collisionPoints)[last]->blendWeight = 1.0f;
            ((VehicleContact**)collisionPoints)[last]->appliedShare = *force;
            return;
        }
        for (i = 0; active > 0; i++) {
            VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
            if (c->contactActive) {
                float w = 1.0f - dist[i] / total;
                c->blendWeight = w;
                c->appliedShare = *force * w;
                active--;
            }
        }
        return;
    }
    if (motionActive) {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = wheelList[i]->restLoadWeight;
            float w = wheelList[i]->loadWeight;
            wheelList[i]->appliedShare = *force * w;
        }
        return;
    }
    if (wheelsInContact > 1) {
        total = 0.0f;
        for (i = 0; i < wheelCount; i++) {
            VehicleWheel* w = wheelList[i];
            if (w->inContact) {
                dist[i] = VehDistance(centerOfMass, w->wheelPosition);
                total += dist[i];
            } else {
                w->loadWeight = 0.0f;
                w->appliedShare = kVec3Zero;
            }
        }
        for (i = 0; i < wheelCount; i++) {
            VehicleWheel* w = wheelList[i];
            if (w->inContact) {
                float k = 1.0f - dist[i] / total;
                w->loadWeight = k;
                w->appliedShare = *force * k;
            }
        }
    } else if (wheelsInContact == 1) {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = 0.0f;
            wheelList[i]->appliedShare = Vec3(0.0f);
        }
        primaryWheel->loadWeight = 1.0f;
        primaryWheel->appliedShare = *force;
    } else {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = 0.0f;
            wheelList[i]->appliedShare = kVec3Zero;
        }
    }
}

static inline float VehAbs(float x)
{
    if (x < 0.0f)
        x = -x;
    return x;
}

// Tilt/lean angle of the body about the given axis (tier 3 names). The reference normal
// (arg c, or the average wheel contact normal) is projected perpendicular to the axis and
// normalised; the angle is asin(|axis-perp-normal x right-vector-side|) signed by the
// handedness, stored in leanAngle; its cosine goes to leanCos when b is set.
static inline float VehDotI(const Vec3* a, const Vec3* b)
{
    return a->y * b->y + a->x * b->x + a->z * b->z;
}
static inline Vec3 VehNormalizedD(const Vec3& v)
{
    float len2 = Vec3DotCall(&v, &v);
    if (len2 == 1.0f)
        return v;
    float inv = FastInvSqrt(len2);
    Vec3 r;
    r.x = inv * v.x;
    r.y = inv * v.y;
    r.z = inv * v.z;
    return r;
}
static inline float VehMinF56(float a, float b) { return (a < b) ? a : b; }
static inline float VehDotG(const Vec3& a, const Vec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}
static inline Vec3 VehCross56(const Vec3& u, const Vec3& v)
{
    Vec3 r;
    r.x = v.z * u.y - v.y * u.z;
    r.y = v.x * u.z - v.z * u.x;
    r.z = v.y * u.x - v.x * u.y;
    return r;
}
void Vehicle::UnknownVirtualSlot56(Vec3* forward, int updateLeanCos, Vec3* up)
{
    if (wheelCount == 0) {
        leanAngle = savedRoll;
        leanCos = bodyCosRoll;
        return;
    }
    Vec3 n;
    if (!up) {
        Vec3 t;
        n = *GetAverageGroundNormal(&t);
    } else
        n = *up;
    Vec3* perp = &scratchVector2;
    Vec3 s;
    Vec3 proj = *Vec3ScaleCall(&s, forward, Vec3DotCall(&n, forward));
    Vec3 d;
    *perp = *Vec3SubtractCall(&d, &n, &proj);
    if (!(perp->x == 0.0f && perp->y == 0.0f && perp->z == 0.0f) && perp) {
        *perp = Vec3Normalize(*perp);
        scratchVector = VehCross56(sideAxis, *forward);
        Vec3 side = VehNormalizedD(scratchVector);
        scratchVector = ((VehV3&)scratchVector2).Cross(side);
        float sign = -1.0f;
        if (VehDotG(*forward, scratchVector) < 0.0f)
            sign = 1.0f;
        float len2 = VehLen2(scratchVector);
        float len;
        if (len2 != 1.0f) {
            len = VehMinF56((float)sqrt(len2), 1.0f);
        } else
            len = 1.0f;
        double angle = asin(len * sign);
        leanAngle = (float)angle;
        if (updateLeanCos)
            leanCos = (float)cos(angle);
        return;
    }
    leanAngle = savedRoll;
    leanCos = bodyCosRoll;
}

// Snaps the body pose to the wheel contact state (tier 3): with one wheel only the height
// (position.y) follows that wheel; with several the body basis is rebuilt from the wheel
// normal (or the supplied direction a), the highest wheel sets the height, and a two-wheel
// bike additionally re-aims the frame by the lean angle (leanAngle). Finally the 7-float
// orientation block is rebuilt from the basis vectors and the previous copy is refreshed.
static inline float VehAbsT(float x) { return x < 0.0f ? -x : x; }

void Vehicle::UnknownVirtualSlot58(Vec3* settleUp, int keepPlacement)
{
    if (wheelCount < 1)
        return;
    RunTickers();
    if (wheelCount == 1) {
        if (keepPlacement == 0) {
            float y = primaryWheel->groundPoint.y;
            position.y = y;
            modelNode->SetPosition(position.x, y, position.z);
        }
        primaryWheel->penetration = 0.0f;
    } else {
        VehicleWheel* top = wheelList[0];
        Vec3 dir;
        Vec3 t;
        if (keepPlacement) {
            dir = *settleUp;
        } else {
            VehicleWheel* second = wheelList[1];
            for (int i = 1; i < wheelCount; i++) {
                VehicleWheel* w = wheelList[i];
                if (w->groundPoint.y > top->groundPoint.y) {
                    top = wheelList[i];
                    second = top;
                } else if (w->groundPoint.y > second->groundPoint.y) {
                    second = w;
                }
            }
            dir = *GetAverageGroundNormal(&t);
        }
        modelNode->SetAxesIn(0, &bodyForward, &dir, 1, 0);
        UnknownVirtualSlot34();
        modelNode->GetPositionIn(0, &position);
        if (keepPlacement == 0) {
            float y = top->field_0x228 - top->wheelPosition.y + top->groundPoint.y;
            position.y = y;
            modelNode->SetPosition(position.x, y, position.z);
        }
        if (wheelCount == 2) {
            Vec3 v0;
            t = *UnknownVirtualSlot55(&v0, &rotationPivot);
            UnknownVirtualSlot56(&t, 0, &dir);
            v0 = modelNode->WorldToLocalDirection(t);
            float lean = leanAngle;
            if (VehAbsT(lean) > 0.001f) {
                if (VehAbsT(v0.z) > 0.001f || VehAbsT(v0.y) > 0.001f || VehAbsT(v0.x) > 0.001f) {
                    modelNode->RotateAboutPoint(rotationPivot, v0, lean);
                    UnknownVirtualSlot34();
                    modelNode->GetPositionIn(0, &position);
                }
            }
        }
    }
    steerState->node->SetAxesIn(0, &bodyForward, &bodyUp, 0, 1);
    airborne = false;
    VehBasisToBlock(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                    &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
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

static inline Vec3 VehNormalized(const Vec3& v)
{
    float len2 = VehLen2(v);
    if (len2 == 1.0f)
        return v;
    float inv = FastInvSqrt(len2);
    Vec3 r;
    r.x = inv * v.x;
    r.y = inv * v.y;
    r.z = inv * v.z;
    return r;
}

// (x*x + y*y) + z*z grouping: VC6 then loads the y, x, z products in retail's order.

static inline Vec3 VehSum(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

// Tier 3 reading: an impulse-style velocity deflection.  k is an effective-mass term along dir,
// r = -m^2/k * dt is the impulse magnitude, and the deflected velocity is v' = v + min(1.5c,1) * r * dir,
// rescaled to keep the old speed (a pure turn); the returned value is the signed angle between
// v and v' (acos of the normalised dot product).
// Deflects the velocity by an impulse applied at `point` along `dir` (tier 3).
// k = slot73(effective mass/stiffness) * d; the change is dir * (-(m*m)/k * dt) scaled by
// min(1.5*c, 1); the new velocity is renormalised to the old speed (linearSpeed) and the signed
// deflection angle is returned (0 when the change is negligible or not finite).  The angle is
// its own local: written back into `sign`, its home moved to the d argument slot and len/sign
// traded the point/dir slots (retail: sign, len, angle in point, dir, c).  The Math3D
// operator sum (`u + velocity`) schedules scratchVector's y store as retail (512 -> 523/539).
// Clamping s inside the first product (`u.x = t.x * (s = ...)`) gives retail's dup form
// (fld st(0); fmul [t.x]) for u.x only (523 -> 528/539).
// Left: retail stores field_0x4e8 before the abs compare (fst; fld st(0); fcom).  No gain from
// assigning k through field_0x4e8, ?: or a nested store, `> 0.0f`, an inline or named abs,
// k = d * slot73 or reusing d for k; VehAbs or an if-negated copy costs 6 bytes.
float Vehicle::UnknownVirtualSlot74(Vec3* point, Vec3* dir, float c, float d)
{
    float k = UnknownVirtualSlot73(point, dir) * d;
    field_0x4e8 = k;
    if (!(VehAbsT(k) <= 0.0f)) {
        float m = forwardSpeed;
        float nk = -k;
        float r = m * m / nk * stepTime;
        Vec3 t = VehScale(*dir, r);
        float s = c * 1.5f;
        Vec3 u;
        u.x = t.x * (s = (s < 1.0f) ? s : 1.0f);
        u.y = t.y * s;
        u.z = t.z * s;
        scratchVector = u + velocity;
        Vec3* nv = &scratchVector;
        float len2 = VehLen2(*nv);
        float len;
        if (len2 == 1.0f)
            len = 1.0f;
        else
            len = (float)sqrt(len2);
        if (!(VehAbsT(len) < 0.01f)) {
            float sign = -1.0f;
            if (!(k < 0.0f))
                sign = 1.0f;
            float dot = VehDotG(scratchVector, velocity);
            float angle = VehAbs((float)acos(dot / (len * linearSpeed))) * sign;
            if (_finite(angle)) {
                velocity = VehScale(*nv, linearSpeed / len);
                if (movingForward)
                    return angle;
                angle = -angle;
                return angle;
            }
        }
    }
    return 0.0f;
}

// Smaller of two values; slot 72's load clamp and the scrape clamps go through it (the
// by-value copy is what keeps each component on the x87 stack across its compare, as
// retail does).
static inline float VehMinF(float a, float b) { return (a < b) ? a : b; }

// Tier 3 reading: accumulates per-wheel ground normals and suspension loads into one
// support normal and a lean/steer response (via slot 74).  Wheels are averaged, so more than
// one contact normalises the sum.
// Sums the contact normals (and, for wheels with ownerRef set, their rollDirection vectors) of the
// wheels and pushes the vehicle along the resulting cross direction (tier 3). *out receives the
// summed normal (normalised when more than one contact); turnAngle/turnRate receive the result
// of slot 74 (deflection angle and angle*dt) using the last flagged wheel's point (+0x23c).
// Retail keeps the argument wheel (tested from its stack slot, also in the reset test) apart
// from the last flagged wheel (edi, initialised from the argument): flagged wheels are read
// through that pointer, plain wheels through the loop's element, so the two branches do not
// share a tail.  Load is declared first (frame slot 0), then the three counters ahead of the
// two zero copies (retail's esi copy temp, the dead `out` slot reused for len2), and the
// average divides by a float count (fild/fdivp, not fidiv).
void Vehicle::UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel)
{
    if (crashState != 0)
        return;
    float load = 0.0f;
    int mixed = 0;
    int flagged = 0;
    int plain = 0;
    Vec3 sum = kVec3Zero;
    *out = kVec3Zero;
    VehicleWheel* last = wheel;
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact || wheel) {
            if (w->ownerRef) {
                last = wheelList[i];
                flagged++;
                sum.x += last->rollDirection.x;
                sum.y += last->rollDirection.y;
                sum.z += last->rollDirection.z;
                out->x += last->groundNormal.x;
                out->y += last->groundNormal.y;
                out->z += last->groundNormal.z;
                load += last->contactLoad;
                if (plain != 0)
                    mixed = 1;
            } else {
                plain++;
                out->x += w->groundNormal.x;
                out->y += w->groundNormal.y;
                out->z += w->groundNormal.z;
                load += wheelList[i]->contactLoad;
                if (flagged > 0)
                    mixed = 1;
            }
        }
    }
    if (wheelsInContact > 1) {
        float len2 = VehLen2(*out);
        if (len2 == 0.0f) {
            *out = kVec3Zero;
        } else {
            float inv = FastInvSqrt(len2);
            out->x = inv * out->x;
            out->y = inv * out->y;
            out->z = inv * out->z;
        }
        float n = (float)wheelsInContact;
        load = load / n;
    }
    if (mixed == 0 && wheel == 0) {
        turnAngle = 0.0f;
        turnRate = 0.0f;
        if (wheelsInContact < 1)
            *out = wheelList[0]->groundNormal;
        return;
    }
    Vec3 c = ((VehV3&)sum).Cross(*out);
    Vec3 axis = VehNormalized(c);
    turnAngle = UnknownVirtualSlot74(&last->sideAxis, &axis, VehMinF(load, 1.0f), 1.0f);
    turnRate = turnAngle * invStepTime;
}

// ---- impact handlers (slots 18..20) ----
// Commit an impact to a sink: latch the current vector into the previous one, mark it dirty.
static inline void VehCommitImpact(VehicleImpactEvent* ev, VehicleImpactSink** sink)
{
    if (ev->resetTrail)
        (*sink)->previousVector = (*sink)->currentVector;
    (*sink)->updatePending = 1;
    ev->resetTrail = 0;
}

// Posts a one-shot impact for the first wheel (aux-driven first, then any) or contact whose
// surface material is flagged in the material table and which has not yet reported one.
// Slots 18 and 19 scan with guarded do-while loops (retail's unrotated `jge exit; jmp top`
// back edges), slot 20 with for loops (rotated `jl top`).
void Vehicle::UnknownVirtualSlot18(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    int i = 0;
    if (wheelCount > 0) do {
        VehicleWheel* w = wheelList[i];
        if (w->aux && w->inContact && !w->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->environment[0x400 + w->surfaceMaterial])) {
            w->impactPosted = 1;
            float v = w->driveShareComplement * 20.0f;
            ev->impactSink->SetPosition(w->groundPoint, (v > 1.0f) ? v : 1.0f);
            VehCommitImpact(ev, &ev->impactSink);
            return;
        }
    } while (++i < wheelCount);
    i = 0;
    if (wheelCount > 0) do {
        VehicleWheel* w = wheelList[i];
        if (w->inContact && !w->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->environment[0x400 + w->surfaceMaterial])) {
            w->impactPosted = 1;
            ev->impactSink->SetPosition(w->groundPoint, 0.0f);
            VehCommitImpact(ev, &ev->impactSink);
            return;
        }
    } while (++i < wheelCount);
    i = 0;
    if (collisionPointCount > 0) do {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->contactActive && !c->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->environment[0x400 + c->surfaceMaterial])) {
            c->impactPosted = 1;
            ev->impactSink->SetPosition(c->impactPosition, 0.0f);
            VehCommitImpact(ev, &ev->impactSink);
            return;
        }
    } while (++i < collisionPointCount);
}

// Slide/scrape impact: like slot 18 but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot19(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    int i = 0;
    if (wheelCount > 0) do {
        VehicleWheel* w = wheelList[i];
        if (w->aux && w->inContact && !w->slidePosted &&
            (!track || ((VehicleMaterialSet*)track)->environment[0x408 + w->surfaceMaterial])) {
            if (!(w->driveShare < 0.95f))
                return;
            w->slidePosted = 1;
            ev->slideSink->SetPosition(w->groundPoint);
            VehCommitImpact(ev, &ev->slideSink);
            scratchVector2.x = (turnAngle >= 0.0f ? -1.0f : 1.0f) * w->tangentSin;
            scratchVector2.y = 0.0f;
            scratchVector2.z = -w->tangentCos;
            scratchVector = modelNode->LocalToWorldDirection(scratchVector2);
            scratchVector.y = w->driveShareComplement * 3.0f;
            scratchVector *= w->driveDistance * invStepTime;
            scratchVector.x = VehMinF(scratchVector.x, 15.0f);
            scratchVector.y = VehMinF(scratchVector.y, 18.0f);
            scratchVector.z = VehMinF(scratchVector.z, 15.0f);
            ev->slideSink->scrapeVector = scratchVector;
            return;
        }
    } while (++i < wheelCount);
    i = 0;
    if (wheelCount > 0) do {
        VehicleWheel* w = wheelList[i];
        if (w->inContact && !w->slidePosted &&
            (!track || ((VehicleMaterialSet*)track)->environment[0x408 + w->surfaceMaterial])) {
            w->slidePosted = 1;
            if (w->tangentSin > 0.2f) {
                VehCommitImpact(ev, &ev->slideSink);
            }
            return;
        }
    } while (++i < wheelCount);
}

// Second scrape channel: like slot 19 (sink at event+0x0c, no per-material table gate) but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot20(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    int i;
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->aux && w->inContact && !w->scrapePosted) {
            if (!(w->driveShare < 0.95f))
                return;
            w->scrapePosted = 1;
            ev->scrapeSink->SetPosition(w->groundPoint);
            VehCommitImpact(ev, &ev->scrapeSink);
            scratchVector2.x = (turnAngle >= 0.0f ? -1.0f : 1.0f) * w->tangentSin;
            scratchVector2.y = 0.0f;
            scratchVector2.z = -w->tangentCos;
            scratchVector = modelNode->LocalToWorldDirection(scratchVector2);
            scratchVector.y = w->driveShareComplement * 3.0f;
            scratchVector *= w->driveDistance * invStepTime;
            scratchVector.x = VehMinF(scratchVector.x, 15.0f);
            scratchVector.y = VehMinF(scratchVector.y, 18.0f);
            scratchVector.z = VehMinF(scratchVector.z, 15.0f);
            ev->scrapeSink->scrapeVector = scratchVector;
            return;
        }
    }
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact && !w->scrapePosted) {
            w->scrapePosted = 1;
            if (w->tangentSin > 0.2f) {
                VehCommitImpact(ev, &ev->scrapeSink);
            }
            return;
        }
    }
}

// ---- slot 4 / slot 46 ----
// Slot 4: builds each body's velocity at its contact arm (velA + worldAngVelA x armA, likewise
// for B), each scaled by `scale`, and hands them with the frame data to ContactSolveImpulse
// 0x5004a0.  Afterwards it caches |*velA| in linearSpeed and worldAngularVelocity = the frame
// transform of angularVelocity (tier 3).  The offsets are Math3D operator
// expressions (retail stores each sum and product through its own temporary).  Left: two
// products of each cross load a before b, and VC6 evaluates the inertia ?: argument ahead of
// the pushes where retail branches between them (spellings of the ?: measured identical).
void Vehicle::UnknownVirtualSlot4(const Vec3* normal, Vec3* velA, const Vec3* worldAngVelA,
                                  const Vec3* armA, int eventKind, float invMassB, int nodeB,
                                  const Vec3* velB, const Vec3* worldAngVelB, const Vec3* armB,
                                  Vec3* invInertiaB, Vec3* angVelB, int velBOrNull,
                                  float* impulseOut, float scale)
{
    scratchVector = CrossProduct(*worldAngVelA, *armA);
    Vec3 pointVelA = (*velA + scratchVector) * scale;
    scratchVector = CrossProduct(*worldAngVelB, *armB);
    Vec3 pointVelB = (scratchVector + *velB) * scale;
    ContactSolveImpulse(restitution, normal, invMass, modelNode, &pointVelA, armA,
                        crashState ? &shapeInvInertia : &invInertia, &angularVelocity, velA, invMassB,
                        (SoultreeObject*)nodeB, &pointVelB, armB, invInertiaB, angVelB,
                        (Vec3*)velBOrNull, impulseOut);
    float len2 = velA->z * velA->z + (velA->x * velA->x + velA->y * velA->y);
    linearSpeed = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
    worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
}

static inline Vec3 VehOffset(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// Slot 46: the point the follow/aim logic looks at, relative to the vehicle position: toward the
// lead wheel (or the midpoint of two wheels when only the aux one is not yet fully settled),
// rotated by the frame transform; falls back to rotationPivot when disabled or > 2 wheels.
// Each branch converts through its own WorldToLocalDirection call (retail has three) and VC6
// merges the `*out = ` copies; the midpoint goes through three temporaries (difference, half,
// sum) as retail's 0x24 frame shows.  The Math3D operators (constructor-return temporaries)
// give retail's store and return-pointer schedule; VehOffset's member-wise temporaries
// store the last component one instruction late.
Vec3* Vehicle::UnknownVirtualSlot46(Vec3* out, float arg)
{
    if (!airborne && !pointsTouching && wheelsInContact <= 2) {
        if (wheelsInContact == 2) {
            // a has the aux object, b is the other wheel
            VehicleWheel* a;
            VehicleWheel* b = secondaryWheel;
            if (b->aux) {
                a = b;
                b = primaryWheel;
            } else {
                a = primaryWheel;
                if (!a->aux) {
                    *out = rotationPivot;
                    return out;
                }
            }
            if (a->driveShare < 0.9f) {
                scratchVector = b->wheelPosition - position;
                *out = modelNode->WorldToLocalDirection(scratchVector);
            } else {
                Vec3 d = a->wheelPosition - b->wheelPosition;
                Vec3 half = d * 0.5f;
                Vec3 mid = half + b->wheelPosition;
                scratchVector = mid - position;
                *out = modelNode->WorldToLocalDirection(scratchVector);
            }
        } else {
            scratchVector = primaryWheel->wheelPosition - position;
            *out = modelNode->WorldToLocalDirection(scratchVector);
        }
    } else {
        *out = rotationPivot;
    }
    return out;
}

// ---- slot 49: per-frame vehicle step ----
// Slot 49 is the per-frame step of the vehicle (tier 3 phase names, from the slot calls):
//   A. setup      - slot 9 fixed-timestep accumulator gives the substep count; slots 30/64/60/65
//                   refresh per-frame inputs; an initial contact query (VehContactsA) when more
//                   than one substep is pending.
//   B. per substep, in order:
//        1. input/state  - decay the speed-state timer, poll slots 80-82 (input flags), and
//                          the spawnProtectTimer countdown (slot 50 clears it);
//        2. ground query - contact query, then slots 8/6/7 (position sample, drag, weighting
//                          of contact reactions) and slots 71/72/86 (support normal, wheel loads);
//        3. dynamics     - slots 13/14 (force/torque accumulation), Methods 00527A20/005293E0/
//                          00529450/00529C20 (wheel forces), VehSmooth (exponential smoothers);
//        4. integrate    - slots 26/48/85/87-95 (integration, lean/pose), write the position
//                          back into the transform node, slot 28 advances the substep counter;
//        5. bookkeeping  - slot 34, basis block refresh, previous velocity, slot 29.
//   C. epilogue   - slot 21 attachments refresh, final smoothers.
// Runs the substep loop of the vehicle simulation (tier 3 names). Each pass: pulls the substep
// count, updates the smoothed speed/lean state, gathers contact reactions, integrates the
// frame transform from the wheels/contacts, and refreshes the cached basis block.
// Exponential smoother (tier 3 reading): x += (target - x) * a with a = min(dt, tau) / tau,
// where tau is timeConstant, a is blendFactor and x is smoothedValue.
static inline float VehSmooth(VehicleSmoother* s, float target, float dt)
{
    s->blendFactor = VehMinF(dt, s->timeConstant) / s->timeConstant;
    return s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;
}

void Vehicle::UnknownVirtualSlot49(float frame)
{
    int steps;
    Vec3 force;       // force accumulator, seeded with weightForce
    Vec3 pivot;
    float speed;
    int hit;
    int res;

    justReset = 0;
    UnknownVirtualSlot9(frame, &steps);
    UnknownVirtualSlot30();
    prevControlInput = controlInput;
    UnknownVirtualSlot64(frame);
    UnknownVirtualSlot60(controlInput.x, UnknownVirtualSlot59(), steps);
    UnknownVirtualSlot65(frame);
    pointsTouching = 0;
    hit = UnknownVirtualSlot39(frame);
    if (!hit && !crashState && steps != 1) {
        VehContactsA(UnknownVirtualSlot5(0), &touchingPointCount, collisionPointCount, ((VehicleContact**)collisionPoints), terrain,
                     &centerOfMass, collisionShape, collisionRadius);
        pointsTouching = touchingPointCount > 0;
    }
    justLanded = 0;
    if (steps > 0) do {
        engineState->TickGearTimer(stepTime);
        Vec3 worldTorque = kVec3Zero;
        Vec3 o2;          // VehContactsB's second output (loop scope: see the slot 49 note)
        if (crashState == 0) {
            if (UpdateWheelRampLevels())
                engineState->driveCut = 1;
        } else if (totalWeight != baseWeight) {
            if (UnknownVirtualSlot10())
                UnknownVirtualSlot0(0.0f);
        }
        backingUp = UnknownVirtualSlot82();
        driveDisengaged = crashState == 0 && (backingUp || UnknownVirtualSlot81());
        throttleOn = crashState == 0 && UnknownVirtualSlot80();
        reinterpret_cast<UnknownEngineGearbox*>(engineState)->UpdateThrottle(stepTime, throttleOn, driveDisengaged);
        reinterpret_cast<UnknownEngineGearbox*>(engineState)->AutoShift(throttleOn, linearSpeed);
        if (hit) {
            Vec3 hitA, hitB, hitC;
            int dummy = 0;
            UnknownVirtualSlot11(hit, &hitA, &hitB, &hitC, &dummy);
            UnknownVirtualSlot33(&hitA, &hitB, &bodyUp, &hitC, dummy, UnknownVirtualSlot32());
            steps = 0;
            stepRemainder = 0.0f;
            break;
        }
        prevSpeed = linearSpeed;
        if (prevSpeed < 0.001f || !_finite(prevSpeed)) {
            prevSpeed = 0.0f;
            velocity = kVec3Zero;
            linearSpeed = 0.0f;
        }
        centerNode->GetPositionIn(0, &centerOfMass);
        float t53 = UnknownVirtualSlot53();
        PlaceWheels();
        if (spawnProtectTimer > 0.0f) {
            spawnProtectTimer -= stepTime;
            if (spawnProtectTimer <= 0.0f)
                UnknownVirtualSlot50(0, 0, 0);
        }
        int wheels = wheelsInContact;
        allWheelsInContact = wheels == wheelCount;
        anyWheelInContact = wheels != 0;
        speed = prevSpeed;
        force = weightForce;
        Vec3 savedOut;
        Vec3 basis = *UnknownVirtualSlot55(&savedOut, &pivot);
        UnknownVirtualSlot56(&basis, 1, 0);
        res = VehContactsA(crashState || (steps == 1 && UnknownVirtualSlot5(0)), &touchingPointCount, collisionPointCount, ((VehicleContact**)collisionPoints),
                           terrain, &centerOfMass, collisionShape, collisionRadius);
        if (res)
            pointsTouching = touchingPointCount > 0;
        UpdateWheelsInContact();
        UnknownVirtualSlot8();
        UnknownVirtualSlot6(&force, &speed);
        UnknownVirtualSlot7(&force);
        wheelForwardAlignment = UnknownVirtualSlot75();
        int nowAirborne;
        if (!anyWheelInContact && !pointsTouching)
            nowAirborne = 1;
        else
            nowAirborne = 0;
        UnknownVirtualSlot71(nowAirborne);
        airborne = nowAirborne;
        if (airborne)
            landingLatched = 0;
        Vec3 lift;
        if (contactTotal > 0) {
            if (anyWheelInContact) {
                UnknownVirtualSlot72(&lift, 0);
                UnknownVirtualSlot86();
            } else {
                wheelPointInContact = 0;
                turnAngle = 0.0f;
                turnRate = 0.0f;
            }
            if (pointsTouching) {
                if (!res)
                    UpdateCollisionPointWorldPositions(collisionPointCount,
                        reinterpret_cast<CollisionPoint**>(collisionPoints));
                UnknownVirtualSlot31();
            }
        } else {
            wheelPointInContact = 0;
            turnAngle = 0.0f;
            turnRate = 0.0f;
        }
        if (wheelPointInContact || (pointsTouching && (!anyWheelInContact || crashState))) {
            Vec3 scale;
            if (crashState) {
                scale = Vec3(1.0f, 1.0f, 1.0f);
            } else {
                scale = Vec3(0.3f, 0.1f, 0.3f);
            }
            Vec3 o1, o3;
            VehContactsB(collisionPointCount, ((VehicleContact**)collisionPoints), &scale, &worldAngularVelocity, &velocity, &centerOfMass,
                         &position, &o3, &o2, &o1);
            float slot3Out;
            UnknownVirtualSlot3(&o2, &o1, &o3, &scale, 0x67, 0, &slot3Out);
            if (prevSpeed * 1.3f < linearSpeed && linearSpeed > 5.0f) {
                float r = prevSpeed / linearSpeed;
                velocity *= r;
                linearSpeed = prevSpeed;
            }
            if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
                velocity = kVec3Zero;
                angularVelocity.y = 0.0f;
                linearSpeed = 0.0f;
            }
        }
        float aq = VehAbsT(turnAngle);
        Vec3 localTorque = kVec3Zero;
        UnknownVirtualSlot13(&force, &localTorque, t53);
        if (anyWheelInContact) {
            Method_00527A20(&speed, &worldTorque, &force);
            if (crashState == 0 || !pointsTouching) {
                Method_005293E0(&speed);
                Method_00529450(&force, &localTorque);
            }
            AccumulateWheelContacts(&force, &localTorque, t53);
        }
        UnknownVirtualSlot14(&force, &localTorque, &worldTorque);
        smoothedVerticalAccel = VehSmooth(verticalAccelSmoother, (velocity.y - prevVelocity.y) * invStepTime, stepTime);
        float k = UnknownVirtualSlot61(aq);
        Vec3 aimOut;
        Vec3 aim = *UnknownVirtualSlot46(&aimOut, k);
        int mode = UnknownVirtualSlot62(k);
        UnknownVirtualSlot26();
        UnknownVirtualSlot48();
        UnknownVirtualSlot85();
        modelNode->SetPosition(position);
        UnknownVirtualSlot95();
        if (crashState) {
            UnknownVirtualSlot87();
        } else {
            if (airborne) {
                UnknownVirtualSlot91();
            } else if (mode) {
                if (mode == 1 || mode == 2) {
                    if (aq >= 0.0001f || VehAbsT(k) > 0.0001f) {
                        if (aq >= 0.0001f) {
                            tiltAxisLocal = modelNode->WorldToLocalDirection(lift);
                        } else {
                            Vec3 contactNormal;
                            scratchVector = *UnknownVirtualSlot54(&contactNormal);
                            tiltAxisLocal = modelNode->WorldToLocalDirection(scratchVector);
                        }
                        if (k != 0.0f && aq < 0.003f)
                            modelNode->RotateAboutPoint(aim, tiltAxisLocal, k);
                        else
                            modelNode->RotateAboutPoint(aim, tiltAxisLocal, k + turnAngle);
                        angularVelocity.x *= 0.85f;
                        angularVelocity.y *= 0.85f;
                        angularVelocity.z *= 0.85f;
                    }
                }
                if (mode == 1 || mode == 3)
                    UnknownVirtualSlot92(&basis, &pivot);
            }
            UnknownVirtualSlot90(&steps, frame);
        }
        modelNode->GetPosition(&position);
        centerNode->GetPositionIn(0, &centerOfMass);
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
        steps = UnknownVirtualSlot28(steps);
        lastStepTime = stepTime;
        steps--;
        if (steps == 0 && stepRemainder > 0.0001f) {
            steps = 1;
            stepTime = stepRemainder;
            invStepTime = 1.0f / stepTime;
            stepRemainder = 0.0f;
        }
        UnknownVirtualSlot34();
        VehBasisToBlock(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                        &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
        prevVelocity = velocity;
        prevCrashState = crashState;
        UnknownVirtualSlot29(steps == 0);
    } while (steps > 0);
    UnknownVirtualSlot21();
    scratchVector = modelNode->WorldToLocalDirection(velocity);
    smoothedForwardAccel = VehSmooth(forwardAccelSmoother, (scratchVector.z - prevLocalForwardVelocity) / frameTime, frameTime);
    prevLocalForwardVelocity = scratchVector.z;
}

// ---- slot 38: collision response dispatch (tier 3 names) ----
// Called with an event code and the other party of a collision. Builds the relative contact
// geometry (contact point minus each centre of mass, the other body's velocity, angular state
// and inverse inertia) and hands it to the impulse solver in slot 4 (other body present) or
// slot 3 (static/no body). Afterwards kills residual creep velocity and, if `notify` is set,
// runs CheckCrash.
static inline Vec3 VehOnes()
{
    Vec3 r;
    r.x = 1.0f;
    r.y = 1.0f;
    r.z = 1.0f;
    return r;
}
struct VehicleContactSet {
    char pad_0x00[0xA0];
    Vec3 contactPoint;            // +0xa0 contact point
    Vec3 contactNormal;            // +0xac contact normal
};
struct VehicleCollisionEvent {     // 'c' argument (provisional)
    char pad_0x00[0x60];
    Vehicle* otherBody;           // +0x60 other body
};

void Vehicle::UnknownVirtualSlot38(int notify, int eventKind, void* event)
{
    float otherInvMass;         // the other body's invMass or 0; then slot 3/4's impulse result
    int otherVelOrNull;         // the other body's velocity, or 0 for static parties
    Vec3 angScale;              // per-axis angular response scale (1,1,1)
    Vec3 otherInvInertia;       // the other body's inverse inertia diagonal
    Vec3 otherArm;              // contact point relative to the other body's centre of mass
    Vec3 otherWorldAngVel;      // the other body's world angular velocity
    int otherNode;              // the other body's scene node, passed as an int
    int hasBody;
    Vec3* otherVel = 0;
    Vehicle* other = 0;
    Vec3* otherAngVel;          // the other body's angularVelocity, or 0
    Vec3 arm;                   // contact point relative to our centre of mass

    angScale = VehOnes();
    switch (eventKind) {
    case 0x66:
    case 0x6a:
        angScale = VehOnes();
        hasBody = 0;
        if (particleManager && (particleManager->isEnabled)) {
            reinterpret_cast<SparkParticleEmitter*>(sparkEmitter)->SetPosition(
                reinterpret_cast<const ParticleVec3&>(((VehicleContactSet*)collisionObject)->contactPoint));
            sparkEmitter->launchVelocity = Vec3(0.0f, 12.0f, 0.0f);
            sparkEmitter->burstPending = 1;
        }
        break;
    case 0x3e9:
        angScale = VehOnes();
        hasBody = 0;
        break;
    case 0:
    case 1:
        hasBody = 1;
        other = ((VehicleCollisionEvent*)event)->otherBody;
        otherInvMass = other->invMass;
        otherVel = &other->velocity;
        otherNode = (int)other->modelNode;
        otherArm = VehOffset(((VehicleContactSet*)collisionObject)->contactPoint, other->centerOfMass);
        otherWorldAngVel = other->worldAngularVelocity;
        otherAngVel = &other->angularVelocity;
        otherInvInertia = other->invInertia;
        break;
    case 0x69: {
        otherInvMass = 0;
        char* body = (char*)((VehicleCollisionEvent*)event)->otherBody;
        otherNode = *(int*)(body + 0x34);
        otherArm = kVec3Zero; otherWorldAngVel = kVec3Zero; otherInvInertia = kVec3Zero;
        otherVel = (Vec3*)(body + 0x40);
        otherAngVel = 0;
        angScale = VehOnes();
        hasBody = 1;
        break;
    }
    case 0x2711: {
        otherInvMass = 0;
        char* body = (char*)((VehicleCollisionEvent*)event)->otherBody;
        otherNode = *(int*)(body + 0x1a0);
        otherArm = kVec3Zero; otherWorldAngVel = kVec3Zero; otherInvInertia = kVec3Zero;
        otherVel = (Vec3*)(body + 0x224);
        otherAngVel = 0;
        angScale = VehOnes();
        hasBody = 1;
        break;
    }
    default:
        return;
    }

    arm = VehOffset(((VehicleContactSet*)collisionObject)->contactPoint, centerOfMass);
    if (hasBody) {
        otherVelOrNull = (eventKind == 0x2711 || eventKind == 0x69) ? 0 : (int)otherVel;
        UnknownVirtualSlot4(&((VehicleContactSet*)collisionObject)->contactNormal, &velocity,
                            &worldAngularVelocity, &arm, eventKind, otherInvMass, otherNode,
                            otherVel, &otherWorldAngVel, &otherArm, &otherInvInertia, otherAngVel,
                            otherVelOrNull, &otherInvMass, 1.0f);
        if (other) {
            *((char*)other + 0x10a) = 0;
            float len2 = VehLen2(*otherVel);
            other->linearSpeed = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
            other->worldAngularVelocity = other->sceneNode->LocalToWorldDirection(*otherAngVel);
        }
    } else {
        Vec3 t = worldAngularVelocity;
        t.x *= angScale.x;
        t.y *= angScale.y;
        t.z *= angScale.z;
        scratchVector = CrossProduct(t, arm);
        Vec3 q = velocity + scratchVector;
        UnknownVirtualSlot3(&((VehicleContactSet*)collisionObject)->contactNormal, &q, &arm,
                            &angScale, eventKind, 0, &otherInvMass);
    }
    if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
        velocity = kVec3Zero;
        angularVelocity.y = 0.0f;
        linearSpeed = 0;
    }
    if (notify)
        CheckCrash();
}

// ---- wave 2 ----

void Vehicle::UnknownVirtualSlot44()
{
    outOfRace = 1;
    Disable();
    reinterpret_cast<D3DIMSoultreeObject*>(modelNode)->ClearScreenRects();
}

// Input-map driven state test (provisional): the 0x46c/0x470 pair short-circuits to "true".
int Vehicle::UnknownVirtualSlot80()
{
    if ((inputDevice && throttleInput > 0.33f)
        || (inputMap->UnknownVirtualSlot3(0, 2, 0x3f, 0)
            && !UnknownVirtualSlot77(engineState->throttleFraction == 0.0f)))
        return 1;
    return inputMap->UnknownVirtualSlot3(200, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot83(VehicleWheel* wheel)
{
    if (inputMap->UnknownVirtualSlot3(1, 2, 0x3f, 0)
        && !UnknownVirtualSlot77(wheel->rampLevel == 0.0f))
        return 1;
    return inputMap->UnknownVirtualSlot3(0xd0, 0, 0x3f, 0) != 0;
}

// Tier 3 reading: re-seats the collision object (0x00435fb0 before and after), removes the
// contact offset from the position when one is recorded (a crash that starts there leaves
// one remaining sub-step), then runs slot 27 when slot27Pending is set and clears it.
int Vehicle::UnknownVirtualSlot28(int steps)
{
    collisionObject->UpdatePlacement();
    int wasSet = crashState != 0;
    collisionObject->QueryCollisions();
    if (collisionObject->hasContact) {
        position -= *(const Vec3*)collisionObject->contactRecord;
        modelNode->SetPosition(position);
        collisionObject->UpdatePlacement();
        if (crashState && !wasSet) {
            steps = 1;
            stepRemainder = 0.0f;
        }
    }
    if (slot27Pending) {
        UnknownVirtualSlot27();
        slot27Pending = 0;
    }
    return steps;
}

// Tier 3: with the 0x109 flag the saved position is restored; otherwise slot 68 supplies a
// code that the key table at input-map+0x34 must accept before slot 67 runs again.
int Vehicle::UnknownVirtualSlot39(float dt)
{
    int code;
    if (respawnPending) {
        UnknownVirtualSlot67();
        position = respawnPosition;
        modelNode->SetPosition(position);
        return 1;
    }
    if (UnknownVirtualSlot68(&code) && inputMap
        && inputMap->keyTable->UnknownVirtualSlot5(code, 0x3f, 0)) {
        UnknownVirtualSlot67();
        return 2;
    }
    return 0;
}

// Tier 3 reading: latches the throttle-like axis (+0x500) into throttleInput (dead zone 0.2,
// -1 when there is no control), mirrors the two other axes into the steer/lean members and
// writes the 0x504 vector (cubed lean for axis kinds 2 and 3).
void Vehicle::UnknownVirtualSlot63(float dt)
{
    if (inputDevice && throttleAxis) {
        float v = throttleAxis->axisValue;
        if (v > 0.2f)
            throttleInput = v;
        else
            throttleInput = 0.0f;
    } else {
        throttleInput = -1.0f;
    }
    controlInput.y = -leanAxis->axisValue;
    steerInput = -steerAxis->axisValue;
    VehicleAxisSource* src = steerAxis->axisSource;
    // Retail never sets eax on any path, so the slot returns void (KrustyBike's override
    // likewise only calls through).
    if (src && !reinterpret_cast<InputDevice*>(src)->IsDeviceSubtype(4)
        && (steerAxis->axisSource->kindCode == 2 || steerAxis->axisSource->kindCode == 3)) {
        float v = steerInput;
        float sq = v * v;
        controlInput.z = 0.0f;
        controlInput.x = sq * steerInput;
    } else {
        controlInput.z = 0.0f;
        controlInput.x = steerInput;
    }
}

// Tier 3 reading: classifies the (controlInput.y, steerInput) input pair into one of eight
// directions: -1 inside the dead zone (|v|^2 <= 0.25), else the sector of the pair relative to
// the band +-0.52057 * |v| (0..7).
int Vehicle::UnknownVirtualSlot88()
{
    float m2 = steerInput * steerInput + controlInput.y * controlInput.y;
    if (m2 > 0.25f) {
        float m = FastSqrt(m2) * 0.52057f;
        if (controlInput.y > m) {
            if (steerInput > m)
                return 3;
            if (-m > steerInput)
                return 0;
            return 4;
        }
        float n = -m;
        if (controlInput.y < n) {
            if (steerInput > m)
                return 2;
            if (steerInput < n)
                return 1;
            return 6;
        }
        if (steerInput > 0.0f)
            return 7;
        return 5;
    }
    return -1;
}

// Tier 3 reading: sums the +0x230 vectors of the wheels without the +0x1c0 flag, normalises
// the sum when more than one wheel contributed (zero stays zero) and returns |sum . savedForward|;
// 1.0 when no wheel contributed.
// z*z + (x*x + y*y) through a reference: the grouping and the helper give retail's x87 order.
static inline float VehLenSqZ(const Vec3& v)
{
    return v.z * v.z + (v.x * v.x + v.y * v.y);
}

float Vehicle::UnknownVirtualSlot75()
{
    Vec3 sum = kVec3Zero;
    int n = 0;
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (!w->ownerRef) {
            sum += w->rollDirection;
            n++;
        }
    }
    if (n > 1) {
        float scale = VehLenSqZ(sum);
        if (scale == 0.0f) {
            sum = kVec3Zero;
        } else {
            scale = FastInvSqrt(scale);
            sum.x = sum.x * scale;
            sum.y = sum.y * scale;
            sum.z = sum.z * scale;
        }
    } else if (n == 0) {
        return 1.0f;
    }
    float d = sum.z * savedForward.z + (sum.x * savedForward.x + sum.y * savedForward.y);
    if (d < 0.0f)
        d = -d;
    return d;
}

// Mean of a sum over n entries.  VC6 keeps the x87 order of the retail products (`fld st(0);
// fmul [sum.x]`) only when the reciprocal is a local of the inlined helper: a caller-side
// `k = 1.0f / n; sum * k` loads sum.x first.
static inline Vec3 VehMean(const Vec3& sum, int n)
{
    float k = 1.0f / n;
    return Vec3(k * sum.x, k * sum.y, k * sum.z);
}

// Normalise with the fast inverse square root; |v|^2 == 1 returns v as is.  The by-value
// parameter is what gives retail's `fld st(0); fmul [v.x]` scaling (a const reference loads
// v.x first); the squared terms must be grouped (x*x + y*y) + z*z.
static inline Vec3 VehNormalizedV(Vec3 v)
{
    float len2 = (v.x * v.x + v.y * v.y) + v.z * v.z;
    if (len2 == 1.0f)
        return v;
    float inv = FastInvSqrt(len2);
    return Vec3(v.x * inv, v.y * inv, v.z * inv);
}

// 0x00528400: average of the wheels' contact normals (+0xe4), normalised (tier 3 reading).
Vec3* Vehicle::GetAverageGroundNormal(Vec3* out)
{
    Vec3 sum = wheelList[0]->groundNormal;
    int n = wheelCount;
    for (int i = 1; i < n; i++)
        sum += wheelList[i]->groundNormal;
    Vec3 avg = VehMean(sum, n);
    *out = VehNormalizedV(avg);
    return out;
}

// Same average restricted to the wheels in contact (+0x260), divided by the contact count
// wheelsInContact; with no contacts the first wheel's normal is returned unchanged.
Vec3* Vehicle::UnknownVirtualSlot54(Vec3* out)
{
    int count = wheelsInContact;
    if (count == 0) {
        *out = wheelList[0]->groundNormal;
        return out;
    }
    Vec3 sum;
    int first = 1;
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact) {
            if (first) {
                sum = w->groundNormal;
                first = 0;
            } else {
                sum += w->groundNormal;
            }
        }
    }
    Vec3 avg = VehMean(sum, count);
    *out = VehNormalizedV(avg);
    return out;
}

// Tier 3 reading: resets the vehicle state after the base reset.  The contact flags and the
// state counters are cleared exactly as in slot 67, the lean/steer/wheel members get their
// defaults, and the speed state at +0x480 is re-seeded with a random start value
// (rand() / 32768 scaled by a table entry).
void Vehicle::UnknownVirtualSlot1(float extraWeight)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot1(extraWeight);
    field_0x4e8 = 0.0f;
    forwardSpeed = 0.0f;
    movingForward = 1;
    for (int i = 0; i < collisionPointCount; i++) {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->ownerNode != 0)
            c->contactActive = 0;
    }
    crashState = 0;
    crashTimer = 0.0f;
    prevCrashState = 0;
    wheelPointInContact = 0;
    landingLatched = 0;
    smoothedVerticalAccel = 0.0f;
    smoothedForwardAccel = 0.0f;
    prevLocalForwardVelocity = 0.0f;
    tiltAxisLocal = kVec3Zero;
    leanError = 0.0f;
    turnRate = 0.0f;
    steerRate = 0.0f;
    leanAngle = 0.0f;
    leanCos = 1.0f;
    wheelForwardAlignment = 1.0f;
    targetLeanAngle = 0.0f;
    contactTotal = 0;
    steerPoseState = 1;
    throttleInput = 0.0f;
    prevControlInput = kVec3Zero;
    throttleOn = false;
    driveDisengaged = false;
    backingUp = 0;
    VehicleSpeedState* s = engineState;
    if (s) {
        s->gear = 0;
        s->currentRpm = 0;
        s->lastUpdateRpm = 0;
        s->throttleFraction = 0.0f;
        s->gearTimer = 0.0f;
        s->driveCut = 1;
        float unit = (float)rand() * 3.05175781e-05f;
        s->randomStart = unit * s->shiftWindowWidths[s->gear] + s->shiftRanges[s->gear].a;
    }
    RunTickers();
    ResetWheelContacts();
}

// Length helper in the shape SoultreePhysicsBaseObject uses: x*x accumulated, 1.0f for a unit
// squared length, otherwise the square root.
static inline float VehLengthAcc(const Vec3& v)
{
    float s = v.x * v.x;
    s += v.y * v.y;
    s += v.z * v.z;
    if (s == 1.0f)
        return 1.0f;
    return (float)sqrt(s);
}

// Same solver call as SoultreePhysicsBaseObject slot 3, but while crashState is set it uses a
// fixed 0.1 restitution, shapeInvInertia (+0xf0) and a zero scale (tier 3 reading).  The
// world-space copy of the angular velocity (+0xcc) is skipped for the event code 0x67.
void Vehicle::UnknownVirtualSlot3(const Vec3* normal, const Vec3* relVel, const Vec3* arm,
                                  const Vec3* angScale, int eventKind, int scaleBits, float* impulseOut)
{
    if (crashState)
        ResolveContactImpulse(0.1f, invMass, modelNode, normal, relVel, arm, &shapeInvInertia, angScale,
                              &angularVelocity, &velocity, impulseOut, 0);
    else
        ResolveContactImpulse(restitution, invMass, modelNode, normal, relVel, arm, &invInertia, angScale,
                              &angularVelocity, &velocity, impulseOut, scaleBits);
    linearSpeed = VehLengthAcc(velocity);
    if (eventKind != 0x67)
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
}

// Tier 3 reading: places the vehicle at newPosition (with its y forced to -1000 unless
// keepPlacement), takes newForward/newUp, settles the wheels twice via slot 58 (which with
// keepPlacement keeps the height and takes settleUp as the up axis, else the ground normal),
// refreshes the wheel and body node positions and finishes with slot 50 (flag, slot 45 value, 0).
int Vehicle::UnknownVirtualSlot33(const Vec3* newPosition, const Vec3* newForward, const Vec3* newUp, const Vec3* settleUp,
                                  int keepPlacement, float extraWeight)
{
    UnknownVirtualSlot1(extraWeight);
    bodyForward = *newForward;
    bodyUp = *newUp;
    float y;
    if (keepPlacement)
        y = newPosition->y;
    else
        y = -1000.0f;
    modelNode->SetPosition(newPosition->x, y, newPosition->z);
    modelNode->GetPosition(&position);
    UnknownVirtualSlot36();
    PlaceWheels();
    UnknownVirtualSlot58((Vec3*)settleUp, keepPlacement);
    PlaceWheels();
    UnknownVirtualSlot58((Vec3*)settleUp, keepPlacement);
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        w->sceneNode->GetPositionIn(0, &w->nodePosition);
    }
    centerNode->GetPositionIn(0, &centerOfMass);
    ApplyRestPose();
    if (collisionObject)
        collisionObject->SnapPlacement();
    justReset = 1;
    attachmentResetPending = 1;
    respawnPending = 0;
    int flag = 1;
    if (UnknownVirtualSlot45() == 0.0f)
        flag = 0;
    UnknownVirtualSlot50(flag, UnknownVirtualSlot45(), 0);
    return 0;
}

// Both helpers load y, x, z first and sum z*z + (x*x + y*y), as retail's x87 sequence shows.
static inline float VehSqLength(const Vec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    return z * z + (x * x + y * y);
}

static inline float VehMagnitude(const Vec3& v)
{
    float sq = VehSqLength(v);
    if (sq == 0.0f)
        return 0.0f;
    if (sq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(sq);
}

// Tier 3 reading: turns the scene node about rotationPivot by |angularVelocity| * stepTime,
// around the angular velocity's direction (kept in scratchVector), when that angle is finite
// and at least 1e-4.
void Vehicle::UnknownVirtualSlot95()
{
    float angle = VehMagnitude(angularVelocity) * stepTime;
    if (_finite(angle) && angle >= 0.0001f) {
        scratchVector = angularVelocity;
        float sq = VehSqLength(scratchVector);
        if (sq == 0.0f) {
            scratchVector = kVec3Zero;
        } else {
            float s = FastInvSqrt(sq);
            scratchVector.x *= s;
            scratchVector.y *= s;
            scratchVector.z *= s;
        }
        modelNode->RotateAboutPoint(rotationPivot, scratchVector, angle);
    }
}

// ---- wave 3: non-virtual helpers ----

// 0x00526830: asks the shared stub 0x00478fe0 (always zero here) whether the vehicle crashed; when it
// did, the control state is cleared and the horizontal heading is recorded (tier 3 reading;
// the result goes to crashState, see Bike slot 99).
int Vehicle::CheckCrash()
{
    int crashed = Method_00478FE0();
    crashState = crashed;
    if (crashed) {
        Vec3 heading(bodyForward.x, 0.0f, bodyForward.z);
        motionActive = 0;
        motionCompleted = 0;
        motionIndex = 0;
        crashHeading = heading;
        crashedUpsideDown = savedUp.y < 0.0f;
        crashYaw = savedYaw;
        slot27Pending = 1;
    }
    return crashed;
}

// 0x00525c60: runs virtual slot 0 of every object in the two owned arrays.
void Vehicle::RunTickers()
{
    for (int i = 0; i < earlyTickerCount; i++)
        earlyTickers[i]->UnknownVirtualSlot0();
    for (int j = 0; j < lateTickerCount; j++)
        lateTickers[j]->UnknownVirtualSlot0();
}

int Vehicle::AddEarlyTicker(VehicleTicker* t)
{
    if (earlyTickerCount < earlyTickerCapacity) {
        earlyTickers[earlyTickerCount] = t;
        earlyTickerCount++;
        return 1;
    }
    return 0;
}

int Vehicle::AddLateTicker(VehicleTicker* t)
{
    if (lateTickerCount < lateTickerCapacity) {
        lateTickers[lateTickerCount] = t;
        lateTickerCount++;
        return 1;
    }
    return 0;
}

// 0x00525d30: appends a wheel (with its two attachment values and optional aux object) and registers
// its collision point (wheel + 0xb8) with the owner's contact array.
int Vehicle::AddWheel(VehicleWheel* wheel, int a2, int a3, int a4, VehicleWheelAux* aux)
{
    if (wheelCount < wheelCapacity) {
        wheel->primaryAux = a3;
        wheel->secondaryAux = a4;
        wheel->aux = aux;
        wheelList[wheelCount] = wheel;
        void* contact = wheel ? (char*)wheel + 0xb8 : 0;
        VehAddContact(collisionPointCapacity, collisionPoints, a2, &collisionPointCount, contact);
        wheelCount++;
        if (aux)
            auxWheelCount++;
        return 1;
    }
    return 0;
}

// 0x005293e0: for every flagged wheel, its shock (primary, else secondary) applies the
// wheel load weight to the speed (0x004f9d30).
void Vehicle::Method_005293E0(float* speed)
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->shockEngaged) {
            VehicleShock* shock = (VehicleShock*)(wheel->primaryAux ? wheel->primaryAux : wheel->secondaryAux);
            if (shock)
                shock->ApplyDamping(wheel->loadWeight, suspensionMass, lastStepTime, speed);
        }
    }
}

// 0x005299e0: exact component-wise equality of two vectors.
int __cdecl VehVec3Equal(const Vec3* a, const Vec3* b)
{
    if (a->x == b->x && a->y == b->y && a->z == b->z)
        return 1;
    return 0;
}

// Out-of-line call views for Method_00529450 (see math/Math3D.h): the retail function has
// used up its inline budget by its second helper site, so the Vec3 constructor
// (0x00404e60), operator+= (0x00428060), DotProduct (0x0040ae30), operator*(Vec3, float)
// (0x005015b0) and the equality 0x005299e0 are called at the sites marked below.
struct VehVec3Call : Vec3 {
    VehVec3Call(float x_, float y_, float z_);
};

// Complete 0x00404e60 constructor behavior for this provisional call view.
VehVec3Call::VehVec3Call(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

// operator*(const Vec3&, float) expanded around the out-of-line constructor (the nested
// expansion is the first one VC6 gives up).
static inline Vec3 VehScaleCtorCall(const Vec3& v, float s)
{
    return VehVec3Call(s * v.x, s * v.y, s * v.z);
}
// The equality 0x005299e0 as retail expands it at the first site of Method_00529450.
static inline int VehVec3EqualInline(const Vec3* a, const Vec3* b)
{
    if (a->x == b->x && a->y == b->y && a->z == b->z)
        return 1;
    return 0;
}
// Projection of v on `onto` (zero when `onto` is the zero vector): the dot products and
// the scale are called out of line at every site.
static inline Vec3 VehProjectTail(const Vec3& v, const Vec3& onto)
{
    if (VehVec3Equal(&onto, &kVec3Zero))
        return kVec3Zero;
    Vec3 r;
    return *Vec3ScaleCall(&r, &onto, Vec3DotCall(&v, &onto) / Vec3DotCall(&onto, &onto));
}
static inline Vec3 VehProjectFirst(const Vec3& v, const Vec3& onto)
{
    if (VehVec3EqualInline(&onto, &kVec3Zero))
        return kVec3Zero;
    Vec3 r;
    return *Vec3ScaleCall(&r, &onto, Vec3DotCall(&v, &onto) / Vec3DotCall(&onto, &onto));
}

// 0x00529450: folds each wheel's shock forces into the accumulators.  For a wheel with a
// shock (primary first, else the rotating one) the spring force projected on the ground
// normal, scaled by leanCos, becomes scratchVector; a pending displacement impulse
// (impulseScale * displacement, projected on the Y axis and scaled the same way) is added to
// *up and consumed.  Then scratchVector is added to the wheel's applied share and to *up,
// and its moment about relativePosition (in body space, x only) to *zero.
void Vehicle::Method_00529450(Vec3* force, Vec3* localTorque)
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (!wheel->shockEngaged || wheel->pointInContact)
            continue;
        if (wheel->primaryAux) {
            Vec3 p = VehProjectFirst(((VehicleShock*)wheel->primaryAux)->spring, wheel->groundNormal);
            scratchVector = p * leanCos;
            if (((VehicleShock*)wheel->primaryAux)->impulseScale != 0.0f) {
                Vec3 v = ((VehicleShock*)wheel->primaryAux)->impulseScale
                         * ((VehicleShock*)wheel->primaryAux)->displacement;
                Vec3 q = VehProjectTail(v, kVec3YAxis);
                scratchVector2 = VehScaleCtorCall(q, leanCos);
                *force += scratchVector2;
                ((VehicleShock*)wheel->primaryAux)->impulseScale = 0.0f;
            }
        } else if (wheel->secondaryAux) {
            Vec3 p = VehProjectTail(((VehicleShock*)wheel->secondaryAux)->spring, wheel->groundNormal);
            scratchVector = VehScaleCtorCall(p, leanCos);
            if (((VehicleShock*)wheel->secondaryAux)->impulseScale != 0.0f) {
                Vec3 v = VehScaleCtorCall(((VehicleShock*)wheel->secondaryAux)->displacement,
                                          ((VehicleShock*)wheel->secondaryAux)->impulseScale);
                Vec3 q = VehProjectTail(v, kVec3YAxis);
                Vec3& impulse = scratchVector2;
                impulse = VehScaleCtorCall(q, leanCos);
                *force += impulse;   // through the reference: loads impulse.x first (byte-exact)
                ((VehicleShock*)wheel->secondaryAux)->impulseScale = 0.0f;
            }
        }
        wheel->appliedShare += scratchVector;
        *force += scratchVector;
        scratchVector2 = CrossProductCall(wheel->relativePosition, scratchVector);
        scratchVector = modelNode->WorldToLocalDirection(scratchVector2);
        scratchVector.y = 0.0f;
        scratchVector.z = 0.0f;
        *localTorque += scratchVector;
    }
}

// 0x00529280: asks every wheel (slot 83) and ramps its +0x29c level toward 1 when the answer is
// non-zero, toward 0 otherwise. Returns the first non-zero answer.
int Vehicle::UpdateWheelRampLevels()
{
    int first = 0;
    for (int i = 0; i < wheelCount; i++) {
        int answer = UnknownVirtualSlot83(wheelList[i]);
        if (!first)
            first = answer;
        float step = stepTime;
        VehicleWheel* wheel = wheelList[i];
        if (answer) {
            if (wheel->rampLevel < 1.0f) {
                float level = wheel->rampLevel + step * wheel->levelRiseRate;
                wheel->rampLevel = level;
                if (level >= 1.0f)
                    level = 1.0f;
                wheel->rampLevel = level;
            }
        } else if (wheel->rampLevel > 0.0f) {
            float level = wheel->rampLevel - step * wheel->levelFallRate;
            wheel->rampLevel = level;
            if (level <= 0.0f)
                level = 0.0f;
            wheel->rampLevel = level;
        }
    }
    return first;
}

// 0x00529a20: updates every wheel in contact against the body, otherwise resets its aux ramp.
void Vehicle::UpdateWheelsInContact()
{
    int i = 0;
    wheelPointInContact = 0;
    for (; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->inContact) {
            wheel->UpdateContactPatch(&centerOfMass, worldAngularVelocity, velocity, linearSpeed, &forwardSpeed, &movingForward);
            wheel->UpdateShock(lastStepTime, (unsigned char)pointsTouching, crashState, linearSpeed, &velocity, &savedForward);
            if (wheel->pointInContact)
                wheelPointInContact = 1;
        } else if (wheel->primaryAux) {
            VehicleWheelAux* aux = (VehicleWheelAux*)wheel->primaryAux;
            aux->field_0x8c = aux->field_0x90;
        } else {
            VehicleWheelAux* aux = (VehicleWheelAux*)wheel->secondaryAux;
            if (aux)
                aux->field_0x8c = aux->field_0x90;
        }
    }
}

// 0x00529c20: folds each in-contact wheel's contact vectors into the two accumulators.
void Vehicle::AccumulateWheelContacts(Vec3* force, Vec3* localTorque, float torqueScale)
{
    if (!airborne) {
        for (int i = 0; i < wheelCount; i++) {
            VehicleWheel* wheel = wheelList[i];
            if (wheel->inContact) {
                if (wheel->field_0x16c == 2) {
                    localTorque->x = torqueScale * localTorque->x;
                    localTorque->y = torqueScale * localTorque->y;
                    localTorque->z = torqueScale * localTorque->z;
                    wheel->field_0x16c = 0;
                }
                if (wheel->aux) {
                    *force += wheel->driveForce;
                    scratchVector2 = UnknownVirtualSlot76(&wheel->relativePosition, &wheel->driveForce);
                    scratchVector = modelNode->WorldToLocalDirection(scratchVector2);
                    *localTorque += scratchVector;
                }
            }
        }
    }
}

// 0x00525a90: resets every wheel's contact state and vectors to defaults.
void Vehicle::ResetWheelContacts()
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        wheel->field_0x2a4 = 1.0f;
        wheel->slipVector = kVec3Zero;
        wheel->driveForce = kVec3Zero;
        wheel->driveMagnitude = 0.0f;
        wheel->field_0x254 = kVec3Zero;
        wheel->driveShare = 1.0f;
        wheel->tangentSin = 1.0f;
        wheel->driveShareComplement = 0.0f;
        wheel->rampLevel = 0.0f;
        wheel->driveDistance = 0.0f;
        wheel->rollAngle = 0;
        wheel->slipSpeed = 0;
        wheel->inContact = 0;
        wheel->tangentCos = 0.0f;
        wheel->shockExtending = 0;
        wheel->sceneNode->GetPositionIn(0, &wheel->nodePosition);
        wheel->field_0x16c = 0;
        wheel->frictionDirection = kVec3Zero;
        wheel->field_0x114 = kVec3Zero;
        wheel->penetration = -999.0f;
        wheel->tangentSpeed = 0;
        wheel->spinSpeed = 0;
        wheel->appliedShare = kVec3Zero;
        wheel->pointInContact = 0;
        wheel->frictionMagnitude = 0.0f;
        wheel->frictionForce = kVec3Zero;
        wheel->normalLoad = 0;
        wheel->normalLoadVector = kVec3Zero;
    }
}

// ---- hit callbacks installed by Vehicle's init 0x00525e20 (tier 2 contiguity) ----
// 0x00525dd0 / 0x00525e00 are stored at CollisionObject +0x88 (onHitCallback) and +0x8c
// (onHitByCallback) by 0x00525e20 (stores at 0x0052632b / 0x0052633b).  owner: Vehicle.cpp
// by contiguity (its own __FILE__ xrefs start at 0x00525e98) -- tier 2.
// The owner behind CollisionObject::ownerObject (+0x60) is NOT shown to be a Vehicle: its
// vtable slot 52 takes (int, CollisionObject*) and slot 38 one int, unlike Vehicle's own
// slots 52 and 38, so only the used slots are declared (provisional view; hitTag +0x134).
struct HitOwner {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35(); virtual void s36(); virtual void s37();
    virtual void Slot38(int a);
    virtual void s39(); virtual void s40(); virtual void s41(); virtual void s42(); virtual void s43(); virtual void s44(); virtual void s45(); virtual void s46(); virtual void s47(); virtual void s48(); virtual void s49();
    virtual void s50(); virtual void s51();
    virtual int Slot52(int a, CollisionObject* b);
    char pad[0x134-4];
    int hitTag;
};
void VehicleHit(CollisionObject* a, CollisionObject* b)
{
    HitOwner* owner = (HitOwner*)a->ownerObject;
    int tag = b->ownerType;
    owner->hitTag = tag;
    owner->Slot38(owner->Slot52(tag, b));
}
void VehicleHitBy(CollisionObject* a, CollisionObject* b)
{
    VehicleHit(a, b);
}

// 0x00525e20: the vehicle loader (`ret 0x94`; Bike's loader 0x004079c0 calls it directly).  Allocates the wheel and
// ticker arrays and the two acceleration smoothers, builds the gearbox (0x004d2940) from
// the caller's tables or with zeros, runs the base loader, then the spark emitter
// (0x004b9830, registered as a child through its slot 27) and the steering control
// (0x00504b60) aligned with the negated third axis, and installs the hit callbacks.
GameObject* Vehicle::LoadVehicle(int context, const char* engineName, const char* modelPath,
                                 const SoultreeLoadDesc* desc, int a5, Vec3 startPosition, Vec3 startForward,
                                 Vec3 startUp, void* terrainProbe, VehicleInputMap* map, void* linkedObject,
                                 float weight, float arg18, float steerScale, void* device, int wheelCap,
                                 int extraContacts, int attachmentSlots, int earlyCap, int lateCap, int defaultEngine,
                                 int* torqueTable, int rpmLow, int rpmHigh, int rpmStep, float gearArg,
                                 VehicleAxis* steer, VehicleAxis* lean, VehicleAxis* throttle,
                                 SoultreeSlot1f0* trackObject, int createCollision)
{
    int i;

    inputMap = map;
    steerAxis = steer;
    leanAxis = lean;
    throttleAxis = throttle;
    suspensionWeightScale = arg18;
    inputDevice = device;
    wheelCapacity = wheelCap;
    if (wheelCapacity > 0) {
        wheelList = (VehicleWheel**)new(__FILE__, 0x151) char[wheelCapacity * sizeof(VehicleWheel*)];
        for (i = 0; i < wheelCapacity; i++)
            wheelList[i] = 0;
    }
    earlyTickerCapacity = earlyCap;
    if (earlyCap > 0) {
        earlyTickers = (VehicleTicker**)new(__FILE__, 0x15c) char[earlyCap * sizeof(VehicleTicker*)];
        for (i = 0; i < earlyCap; i++)
            earlyTickers[i] = 0;
    }
    lateTickerCapacity = lateCap;
    if (lateCap > 0) {
        lateTickers = (VehicleTicker**)new(__FILE__, 0x167) char[lateCap * sizeof(VehicleTicker*)];
        for (i = 0; i < lateCap; i++)
            lateTickers[i] = 0;
    }
    crashReason = 0;
    justLanded = 0;
    referenceSpeed = 180.0f;
    maxLeanAngle = 40.0f * 0.01745329f;
    maxLeanRate = 120.0f * 0.01745329f;
    crashTimerReload = 2.0f;
    impactAccelLimit = 200.0f;
    speedGainLimit = -1000.0f;
    verticalAccelSmoother = new(__FILE__, 0x19d) VehicleSmoother(0.14f);
    forwardAccelSmoother = new(__FILE__, 0x1a0) VehicleSmoother(0.5f);
    UnknownVirtualSlot96();
    // The provisional float gearArg is passed as an unchanged DWORD in retail;
    // the canonical constructor stores it at +0x1d8 without numeric conversion.
    if (defaultEngine)
        engineState = reinterpret_cast<VehicleSpeedState*>(new(__FILE__, 0x1aa) UnknownEngineGearbox(
            engineName, 0.02f, 0, 0, 0, 0, reinterpret_cast<const int&>(gearArg),
            controlResponse[2], controlResponse[3]));
    else
        engineState = reinterpret_cast<VehicleSpeedState*>(new(__FILE__, 0x1b5) UnknownEngineGearbox(
            engineName, 0.02f, torqueTable, rpmLow, rpmHigh, rpmStep, reinterpret_cast<const int&>(gearArg),
            controlResponse[2], controlResponse[3]));
    SoultreePhysicsCharacter::UnknownVirtualSlot40(context, 0, modelPath, desc, a5, startPosition,
                                                   startForward, startUp, terrainProbe, linkedObject,
                                                   weight, wheelCap + extraContacts, attachmentSlots,
                                                   trackObject, 0.02f, 100, 0.001f, 0.1f, 3,
                                                   trackObject ? ((VehicleTrackView*)trackObject)->field_0x88 : 0,
                                                   createCollision);
    if (particleManager) {
        sparkEmitter = reinterpret_cast<VehicleSparkEmitter*>(
            new(__FILE__, 0x1dc) SparkParticleEmitter(1));
        GameObject::AppendChild((GameObject*)sparkEmitter->UnknownVirtualSlot27(renderContext, particleManager), -1);
    }
    steerState = new(__FILE__, 0x1e0) VehicleSteerState(steerScale, controlResponse[0], &startForward, &startUp);
    Vec3 down = startUp * -1.0f;
    steerState->SetAxisFromDirection(&down, modelNode);
    steerState->SetAngle(0, modelNode);
    spawnProtectTimer = 0.0f;
    crashDirection = 0;
    if (collisionObject) {
        collisionObject->onHitCallback = VehicleHit;
        collisionObject->onHitByCallback = VehicleHitBy;
    }
    return this;
}

// owner: Vehicle.cpp (__FILE__ 0x005257a0 region), PARTIAL 90.89%: remaining diff is store
// scheduling of the 0x4b0/0x4b4 vs 0x4fc/0x500 float stores around the weightForce copy.
Vehicle::Vehicle(int flags) : GameObject(1), SoultreePhysicsCharacter(flags)
{
    field_0x28 = 1.0f;
    loadWeight = 0;
    engineState = 0;
    auxWheelCount = 0;
    steerState = 0;
    verticalAccelSmoother = 0;
    forwardAccelSmoother = 0;
    weightForce = kVec3Zero;
    leanCos = 1.0f;
    wheelForwardAlignment = 1.0f;
    particleManager = 0;
    sparkEmitter = 0;
    inShadow = 0;
    shadowLight = 0;
    attachmentResetPending = 0;
    crashYaw = 0;
    crashedUpsideDown = 0;
    pointsTouching = 0;
    outOfRace = 0;
    wheelList = 0;
    wheelCapacity = 0;
    wheelCount = 0;
    lateTickers = 0;
    lateTickerCapacity = 0;
    lateTickerCount = 0;
    earlyTickers = 0;
    earlyTickerCapacity = 0;
    earlyTickerCount = 0;
    collisionPoints = 0;
    collisionPointCapacity = 0;
    collisionPointCount = 0;
    attachments = 0;
    attachmentCount = 0;
    attachmentCapacity = 0;
    steerAxis = 0;
    driveForceScale = 1.0f;
    terrainScale = 1.0f;
    throttleAxis = 0;
    inputMap = 0;
    leanAxis = 0;
    anyWheelInContact = 0;
    wheelPointInContact = 0;
    turnAngle = 0;
    turnRate = 0;
    primaryWheel = 0;
    secondaryWheel = 0;
    wheelsInContact = 0;
    ownedWheelsInContact = 0;
    steerRate = 0;
    stepTime = 0.025f;
    invStepTime = 40.0f;
    frameTime = 0;
    lastStepTime = 0.025f;
    allWheelsInContact = 0;
    prevCrashState = 0;
    prevThrottleFraction = 0;
    controlInput = kVec3Zero;
    prevControlInput = kVec3Zero;
    steerInput = 0;
    contactTotal = 0;
    touchingPointCount = 0;
    track = 0;
    inputDevice = 0;
    throttleInput = 0;
    steerPoseState = 1;
    *(int*)&pad_0x5B8 = 0;
    Vehicle::UnknownVirtualSlot1(165.0f);
    spawnProtected = 0;
    takeoffVelocity = kVec3Zero;
    takeoffPosition = kVec3Zero;
}

// 0x00526380, ~Vehicle's body (entered through the vbase-adjusted deleting destructor
// 0x0052b630, like ~Bike 0x00409a10).  The smoothers are plain structs; the engine and steer
// states have out-of-line destructors (0x00464e90 is the shared empty one); the three owned
// arrays are freed and cleared; the input map's value source is told to detach (slots 15/16).
// ~SoultreePhysicsCharacter (0x00503d40) follows implicitly.
Vehicle::~Vehicle()
{
    if (verticalAccelSmoother)
        delete verticalAccelSmoother;
    if (forwardAccelSmoother)
        delete forwardAccelSmoother;
    if (engineState)
        delete engineState;
    if (steerState)
        delete steerState;
    if (wheelList) {
        delete wheelList;
        wheelList = 0;
    }
    if (lateTickers) {
        delete lateTickers;
        lateTickers = 0;
    }
    if (earlyTickers) {
        delete earlyTickers;
        earlyTickers = 0;
    }
    if (inputMap && inputMap->valueSource) {
        inputMap->valueSource->UnknownVirtualSlot15();
        inputMap->valueSource->UnknownVirtualSlot16();
    }
}

// 0x0052a830 (GameObject slot 10 via the vtordisp thunk 0x0052b690): when the current
// motion is finished (slot 42) motionActive is cleared and motionCompleted set, then the per-frame step
// (slot 49) runs and GameObject's own slot 10 is called directly, bypassing
// SoultreePhysicsCharacter's version (0x00504210).
int Vehicle::Tick(float dt)
{
    if (UnknownVirtualSlot42()) {
        motionActive = 0;
        motionCompleted = 1;
    }
    UnknownVirtualSlot49(dt);
    return GameObject::Tick(dt);
}

// 0x00528eb0: wheel placement pass.  Each wheel's shock is retracted by a quarter of the last
// step and the wheel is probed against the ground; a wheel that is not in contact is retracted
// by three quarters and probed again, one in contact has its shock forces cleared.  The second
// loop counts the wheels in contact (primary/secondary wheel = first/second), counts those with
// ownerRef into ownedWheelsInContact and flags (reportContactOutputs) the first in-contact wheel without it,
// else wheel 0.
void Vehicle::PlaceWheels()
{
    int i;
    int unflagged;
    if (wheelCount == 0)
        return;
    wheelsInContact = 0;
    unflagged = 1;
    ownedWheelsInContact = 0;
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->primaryAux)
            ((VehicleInlineShock*)wheel->primaryAux)->Retract(lastStepTime * 0.25f, wheel);
        else if (wheel->secondaryAux)
            ((VehicleRotatingShock*)wheel->secondaryAux)->Retract(lastStepTime * 0.25f, wheel);
        wheel->UpdateSuspensionProbe(terrain, &position, savedPitch, savedSinRoll, savedYaw,
                               &savedForward, &savedUp, poseNode);
        if (!wheel->inContact) {
            if (wheel->primaryAux)
                ((VehicleInlineShock*)wheel->primaryAux)->Retract(lastStepTime * 0.75f, wheel);
            else if (wheel->secondaryAux)
                ((VehicleRotatingShock*)wheel->secondaryAux)->Retract(lastStepTime * 0.75f, wheel);
            wheel->UpdateSuspensionProbe(terrain, &position, savedPitch, savedSinRoll, savedYaw,
                                   &savedForward, &savedUp, poseNode);
        } else if (wheel->primaryAux) {
            ((VehicleShock*)wheel->primaryAux)->ClearForces();
        } else if (wheel->secondaryAux) {
            ((VehicleShock*)wheel->secondaryAux)->ClearForces();
        }
    }
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->inContact) {
            if (wheel->ownerRef) {
                ownedWheelsInContact++;
            } else if (unflagged) {
                wheel->reportContactOutputs = 1;
                unflagged = 0;
            }
            if (wheelsInContact == 0)
                primaryWheel = wheelList[i];
            else if (wheelsInContact == 1)
                secondaryWheel = wheelList[i];
            wheelsInContact++;
        }
    }
    if (unflagged)
        wheelList[0]->reportContactOutputs = 1;
}

// 0x00527a20 (PARTIAL, 350/870, 866 bytes): the first 0x120 bytes (count loop, drive
// loop, dot product and the branch on its sign) match.  Retail copies Vec3 values into the
// wheel with direct [wheel+disp] moves, while VC6 forms the destination (or source) address
// of a struct assignment in a register (lea).  The memberwise VehSetV gives the direct
// moves for the scaled normal and the friction copy, and the impulse block then matches.
// Left: the (0,0,0) store.  Retail materialises the temporary on the stack and copies it
// with direct stores; the struct assignment keeps the lea, and VehSetV (also via a named
// zero, a reference, an inline wheel method or a by-value helper) lets VC6 fold the
// temporary into immediate stores (832-836 bytes).
// Wheel drive and contact impulse pass (tier 3 reading).  Counts the in-contact
// wheels with a nonzero +0x2a0 and, unless crashed, lets each wheel apply its share of the
// drive (0x00514170).  Then for every wheel in contact: the load pushing into the ground
// (-(appliedShare . groundNormal), floored at 0) is stored with its vector, the contact
// point's friction update runs, and while the wheel's time scale and its load budget
// (tangentSpeed + spinSpeed) are positive an impulse limited by min(*speed, budget) is taken
// from *speed: the wheel's frictionForce scaled by (mass/dt * m) / timeScale, or unscaled when
// the time scale is the smaller, is added to *force and its moment about relativePosition to *worldTorque.
static inline void VehSetV(Vec3& d, const Vec3& s) { d.x = s.x; d.y = s.y; d.z = s.z; }
void Vehicle::Method_00527A20(float* speed, Vec3* worldTorque, Vec3* force)
{
    int i;
    float driven = 0.0f;
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact && w->driveStrength != 0.0f)
            driven += 1.0f;
    }
    if (crashState == 0 && driven != 0.0f) {
        float share = 1.0f / driven;
        for (i = 0; i < wheelCount; i++)
            wheelList[i]->ApplyDrive(share, stepTime, movingForward, bodyMass, speed, worldTorque, force);
    }
    for (i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (!w->inContact)
            continue;
        float d = VehDotG(w->appliedShare, w->groundNormal);
        w->normalLoad = d;
        if (d >= 0.0f) {
            w->normalLoadVector = Vec3(0.0f, 0.0f, 0.0f);
            w->normalLoad = 0.0f;
        } else {
            Vec3 t;
            d = -d;
            w->normalLoad = d;
            VehSetV(w->normalLoadVector, *Vec3ScaleCall(&t, &w->groundNormal, d));
        }
        w->contactPoint.UnknownVirtualSlot1();
        float budget = w->spinSpeed + w->tangentSpeed;
        float timeScale = w->frictionMagnitude;
        if (timeScale > 0.0f && budget != 0.0f) {
            float dt = stepTime;
            float mass = bodyMass;
            float m = (*speed < budget) ? *speed : budget;
            if (m <= 0.0f)
                m = 0.0f;
            float rate = mass / dt;
            Vec3 v;
            if (timeScale > rate * m) {
                float k = rate * m / timeScale;
                v = w->frictionForce * k;
                *speed = *speed - m;
            } else {
                *speed = *speed - timeScale / rate;
                VehSetV(v, w->frictionForce);
            }
            *force += v;
            *worldTorque += CrossProduct(w->relativePosition, v);
        }
    }
}
