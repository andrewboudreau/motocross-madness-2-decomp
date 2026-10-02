// Vehicle.cpp - reconstruction of the retail Vehicle translation unit
// (__FILE__ xrefs near 0x00525e98). See Vehicle.h for the evidence/provisional notes.
#include "Vehicle.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "collision/CollisionObject.h"
#include "../soultree_base/SoultreePhysicsCallees.h"

// ---- small vector helpers (stand-ins for common/Math3D.h) ----

// Cross product as a member of a Vec3 view: VC6 honours source multiplicand order here
// (free-function form canonicalises it).
struct VehV3 : Vec3
{
    Vec3 Cross(const Vec3& b) const
    {
        Vec3 r;
        r.x = y * b.z - z * b.y;
        r.y = z * b.x - x * b.z;
        r.z = x * b.y - b.x * y;
        return r;
    }
    // Same cross product with the y-component written b.x * z (member order is honoured).
    Vec3 CrossB(const Vec3& b) const
    {
        Vec3 r;
        r.x = y * b.z - z * b.y;
        r.y = b.x * z - x * b.z;
        r.z = x * b.y - b.x * y;
        return r;
    }
};

void Vehicle::UnknownVirtualSlot34()
{
    modelNode->GetAxesIn(0, &bodyForward, &bodyUp);
    sideAxis = ((VehV3&)bodyForward).CrossB(bodyUp);
}

void Vehicle::UnknownVirtualSlot35(int a, int b)
{
    modelNode->SetAxesIn(0, &bodyForward, &bodyUp, a, b);
    sideAxis = ((VehV3&)bodyForward).CrossB(bodyUp);
}

float Vehicle::UnknownVirtualSlot45() { return 3.0f; }
int Vehicle::UnknownVirtualSlot51() { return spawnProtected; }
int Vehicle::UnknownVirtualSlot52() { return crashState == 0; }

int Vehicle::UnknownVirtualSlot68(int* out)
{
    *out = 15;
    return 1;
}

void Vehicle::UnknownVirtualSlot69() { crashTimer = crashTimerReload; }

int Vehicle::UnknownVirtualSlot23()
{
    return !airborne && field_0x434 >= 0.2f;
}

int Vehicle::UnknownVirtualSlot24()
{
    return field_0x5b0 < engineState->field_0x00 && crashState == 0 && linearSpeed < 22.0f;
}

int Vehicle::UnknownVirtualSlot25()
{
    return field_0x5b0 > engineState->field_0x00 && crashState == 0 && linearSpeed < 22.0f;
}

void Vehicle::UnknownVirtualSlot92(const Vec3*, const Vec3*) {}

void Vehicle::UnknownVirtualSlot0(float dt)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot0(dt);
    field_0x4dc = (field_0x4d8 * baseWeight + dt) * 0.0310558993f;   // 1/32.2
}

int Vehicle::UnknownVirtualSlot42()
{
    return crashState == 0 && field_0x433 >= 0 && field_0x430;
}

void Vehicle::UnknownVirtualSlot50(int a, float b, int c)
{
    spawnProtected = a;
    spawnProtectTimer = b;
}

float Vehicle::UnknownVirtualSlot53()
{
    steerState->Method_00504EC0(steerRate * stepTime, poseNode);
    return 1.0f;
}

Vec3* Vehicle::UnknownVirtualSlot55(Vec3* unused, Vec3* out)
{
    out->x = g_VehZeroVec3.x;
    out->y = g_VehZeroVec3.y;
    out->z = g_VehZeroVec3.z;
    *out = savedForward;
    return out;
}

float Vehicle::UnknownVirtualSlot57() { return leanAngle; }
float Vehicle::UnknownVirtualSlot59() { return steerState->field_0x08; }
float Vehicle::UnknownVirtualSlot61(float arg) { return 0.0f; }

int Vehicle::UnknownVirtualSlot62(float arg)
{
    return anyWheelInContact && crashState == 0;
}

int Vehicle::UnknownVirtualSlot89(float arg)
{
    return UnknownVirtualSlot66();
}

void Vehicle::UnknownVirtualSlot93() { field_0x51c = 0.5f; }

void Vehicle::UnknownVirtualSlot96()
{
    field_0x524[0] = 1.0f;
    field_0x524[1] = 1.0f;
    field_0x524[2] = 1.0f;
    field_0x524[3] = 1.0f;
    field_0x524[4] = 1.0f;
    field_0x524[5] = 1.0f;
}

void Vehicle::UnknownVirtualSlot21()
{
    SoultreePhysicsCharacter::UnknownVirtualSlot21();
    field_0x5b0 = engineState->field_0x00;
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

// Provisional semantics: true when the accumulated travel (0xbc - 0xb8) is less than arg * 0x450.
int Vehicle::UnknownVirtualSlot70(float arg)
{
    return linearSpeed - prevSpeed < arg * speedGainLimit;
}

// Provisional: field_0x4ec / sin(steer angle), zero when the angle is zero.
float Vehicle::UnknownVirtualSlot73(const Vec3* a, const Vec3* b)
{
    VehicleSteerState* s = steerState;
    float m = s->steerAngle;
    if (m < 0.0f)
        m = -m;
    if (m == 0.0f)
        return 0.0f;
    return wheelBase / sinf(s->steerAngle);
}

// Clears the per-contact flag at +0xa4 for contacts whose +4 field is set, then resets state.
void Vehicle::UnknownVirtualSlot67()
{
    for (int i = 0; i < collisionPointCount; i++) {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->field_0x04 != 0)
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

int Vehicle::UnknownVirtualSlot78()
{
    if (inputMap->UnknownVirtualSlot3(3, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return inputMap->UnknownVirtualSlot3(0x1f, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot79()
{
    if (inputMap->UnknownVirtualSlot3(4, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return inputMap->UnknownVirtualSlot3(0x2d, 0, 0x3f, 0) != 0;
}

void Vehicle::UnknownVirtualSlot64(float arg)
{
    if (crashState == 0) {
        UnknownVirtualSlot63(arg);
        return;
    }
    controlInput = g_VehZeroVec3;
    steerInput = 0.0f;
    throttleInput = 0.0f;
}

// Provisional semantics: a cross product returned through out (a x b).
Vec3 Vehicle::UnknownVirtualSlot76(const Vec3* a, const Vec3* b)
{
    Vec3 r;
    const Vec3& p = *b; const Vec3& q = *a;
    r.x = p.z * q.y - p.y * q.z;
    r.y = p.x * q.z - p.z * q.x;
    r.z = p.y * q.x - p.x * q.y;
    return r;
}

void Vehicle::UnknownVirtualSlot90(int* a, float b)
{
    if (Method_00526830()) {
        *a = 1;
        stepRemainder = 0.0f;
        return;
    }
    if (*a == 1 && stepRemainder <= 0.0001f)
        UnknownVirtualSlot89(b);
}

// Provisional: steering-error style term, t = -(a*b) - angle; result is t / (c*mass + damping)
// unless |t| is tiny or the vehicle is in state 0x444.
void Vehicle::UnknownVirtualSlot60(float a, float b, int c)
{
    if (crashState == 0) {
        steerRate = -(a * b) - steerState->steerAngle;
        float m = steerRate;
        if (m < 0.0f) m = -m;
        if (m < 0.00001f) {
            steerRate = 0.0f;
        } else {
            if (stepRemainder > 0.0001f)
                steerRate = steerRate / ((float)c * stepTime + stepRemainder);
            else
                steerRate = steerRate / ((float)c * stepTime);
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
        wheelList[i]->Method_005143D0(stepTime, v, flag, allWheelsInContact, crashState, invMass);
}

void Vehicle::UnknownVirtualSlot86()
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact) {
            if (w->field_0x2a8) {
                float sp = linearSpeed;
                float dt = field_0x4a4;
                w->field_0x2a8->Method_004D31B0(sp, &w->field_0x230, dt, field_0x47a, 100.0f,
                                                &w->field_0x248, &w->field_0x280);
            }
            w->Method_00513F90(this);
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
// field_0x64 = velocity, field_0x7c = previous velocity, field_0x0c = position).
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

void Vehicle::UnknownVirtualSlot71(int arg)
{
    if (crashState) {
        landingLatched = 0;
        if (airborne && !arg)
            justLanded = 1;
    } else if (airborne && !arg && !landingLatched) {
        landingLatched = 1;
        justLanded = 1;
    }
    if (!airborne && arg) {
        takeoffVelocity = velocity;
        takeoffPosition = position;
    }
}

// Provisional: snapshot position (0x10c) and a horizontal heading vector (0x118 = field_0xa0
// with y forced to 0), normalised with FastInvSqrt; zero vector if degenerate.
static inline float VehLen2(const Vec3& v) { return (v.x * v.x + v.y * v.y) + v.z * v.z; }
void Vehicle::UnknownVirtualSlot43()
{
    respawnPosition = position;
    respawnHeading = savedForward;
    respawnHeading.y = 0.0f;
    float len2 = VehLen2(respawnHeading);
    if (len2 == 0.0f) {
        respawnHeading = g_VehZeroVec3;
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
// Wheel mode: field_0x430 gives every wheel its stored weight (+0x294); otherwise the same
// blending as for contacts using the wheels' contact points, or a single lead wheel.
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
    float d2 = dx * dx + dy * dy + dz * dz;
    if (d2 == 1.0f)
        return 1.0f;
    return FastSqrt(d2);
}

void Vehicle::UnknownVirtualSlot7(const Vec3* arg)
{
    Vec3 share;
    float total;
    float dist[128];
    int i;
    int active;

    contactTotal = touchingPointCount + wheelsInContact;
    if (pointsTouching) {
        active = 0;
        total = 0.0f;
        int last = 0;
        for (i = 0; i < collisionPointCount; i++) {
            VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
            if (c->contactActive) {
                dist[i] = VehDistance(centerOfMass, c->contactPosition);
                total += dist[i];
                active++;
                last = i;
            } else {
                c->blendWeight = 0.0f;
                c->appliedShare = g_VehZeroVec3;
            }
        }
        if (active == 1) {
            ((VehicleContact**)collisionPoints)[last]->blendWeight = 1.0f;
            ((VehicleContact**)collisionPoints)[last]->appliedShare = *arg;
            return;
        }
        for (i = 0; active > 0; i++) {
            VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
            if (c->contactActive) {
                float w = 1.0f - dist[i] / total;
                active--;
                c->blendWeight = w;
                share.x = arg->x * w;
                share.y = arg->y * w;
                share.z = arg->z * w;
                c->appliedShare = share;
            }
        }
        return;
    }
    if (field_0x430) {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = wheelList[i]->field_0x294;
            float w = wheelList[i]->loadWeight;
            share.x = arg->x * w;
            share.y = arg->y * w;
            share.z = arg->z * w;
            wheelList[i]->appliedShare = share;
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
                w->appliedShare = g_VehZeroVec3;
            }
        }
        for (i = 0; i < wheelCount; i++) {
            VehicleWheel* w = wheelList[i];
            if (w->inContact) {
                float k = 1.0f - dist[i] / total;
                w->loadWeight = k;
                share.x = arg->x * k;
                share.y = arg->y * k;
                share.z = arg->z * k;
                w->appliedShare = share;
            }
        }
    } else if (wheelsInContact == 1) {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = 0.0f;
            wheelList[i]->appliedShare = g_VehZeroVec3;
        }
        primaryWheel->loadWeight = 1.0f;
        primaryWheel->appliedShare = *arg;
    } else {
        for (i = 0; i < wheelCount; i++) {
            wheelList[i]->loadWeight = 0.0f;
            wheelList[i]->appliedShare = g_VehZeroVec3;
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
// handedness, stored in field_0x4ac; its cosine goes to field_0x4b0 when b is set.
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
void Vehicle::UnknownVirtualSlot56(Vec3* a, int b, Vec3* c)
{
    if (wheelCount == 0) {
        leanAngle = savedRoll;
        leanCos = bodyCosRoll;
        return;
    }
    Vec3 n;
    Vec3 t;
    if (!c)
        n = *Method_00528400(&t);
    else
        n = *c;
    Vec3 proj = *Vec3ScaleCall(&t, a, Vec3DotCall(&n, a));
    Vec3* perp = &scratchVector2;
    *perp = *Vec3SubtractCall(&t, &n, &proj);
    if (!(perp->x == 0.0f && perp->y == 0.0f && perp->z == 0.0f) && perp) {
        *perp = Vec3Normalize(*perp);
        scratchVector = ((VehV3&)sideAxis).Cross(*a);
        Vec3 side = VehNormalizedD(scratchVector);
        scratchVector = ((VehV3&)scratchVector2).Cross(side);
        float sign = -1.0f;
        if (VehDotI(&scratchVector, a) < 0.0f)
            sign = 1.0f;
        float len2 = VehDotI(&scratchVector, &scratchVector);
        float len = 1.0f;
        if (len2 != 1.0f) {
            len = (float)sqrt(len2);
            if (!(len < 1.0f))
                len = 1.0f;
        }
        double angle = asin(len * sign);
        leanAngle = (float)angle;
        if (b)
            leanCos = (float)cos(angle);
        return;
    }
    leanAngle = savedRoll;
    leanCos = bodyCosRoll;
}

// Snaps the body pose to the wheel contact state (tier 3): with one wheel only the height
// (field_0x0c.y) follows that wheel; with several the body basis is rebuilt from the wheel
// normal (or the supplied direction a), the highest wheel sets the height, and a two-wheel
// bike additionally re-aims the frame by the lean angle (field_0x4ac). Finally the 7-float
// orientation block is rebuilt from the basis vectors and the previous copy is refreshed.
static inline float VehAbsT(float x) { return x < 0.0f ? -x : x; }

void Vehicle::UnknownVirtualSlot58(Vec3* a, int b)
{
    if (wheelCount < 1)
        return;
    Method_00525C60();
    if (wheelCount == 1) {
        if (b == 0) {
            float y = primaryWheel->groundPoint.y;
            position.y = y;
            modelNode->SetPosition(position.x, y, position.z);
        }
        primaryWheel->field_0x150 = 0.0f;
    } else {
        VehicleWheel* top = wheelList[0];
        Vec3 dir;
        Vec3 t;
        if (b) {
            dir = *a;
        } else {
            VehicleWheel* second = wheelList[1];
            for (int i = 1; i < wheelCount; i++) {
                VehicleWheel* w = wheelList[i];
                if (w->groundPoint.y > top->groundPoint.y) {
                    second = top = w;
                } else if (w->groundPoint.y > second->groundPoint.y) {
                    second = w;
                }
            }
            dir = *Method_00528400(&t);
        }
        modelNode->SetAxesIn(0, &bodyForward, &dir, 1, 0);
        UnknownVirtualSlot34();
        modelNode->GetPositionIn(0, &position);
        if (b == 0) {
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
    steerState->steerNode->SetAxesIn(0, &bodyForward, &bodyUp, 0, 1);
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
    float len2 = v.y * v.y + v.x * v.x + v.z * v.z;
    if (len2 == 1.0f)
        return v;
    float inv = FastInvSqrt(len2);
    Vec3 r;
    r.x = inv * v.x;
    r.y = inv * v.y;
    r.z = inv * v.z;
    return r;
}

// Tier 3 reading: an impulse-style velocity deflection.  k is an effective-mass term along dir,
// r = -m^2/k * dt is the impulse magnitude, and the deflected velocity is v' = v + min(1.5c,1) * r * dir,
// rescaled to keep the old speed (a pure turn); the returned value is the signed angle between
// v and v' (acos of the normalised dot product).
// Deflects the velocity (field_0x64) by an impulse applied at `point` along `dir` (tier 3).
// k = slot73(effective mass/stiffness) * d; the change is dir * (-(m*m)/k * dt) scaled by
// min(1.5*c, 1); the new velocity is renormalised to the old speed (field_0xbc) and the signed
// deflection angle is returned (0 when the change is negligible or not finite).
float Vehicle::UnknownVirtualSlot74(Vec3* point, Vec3* dir, float c, float d)
{
    float k = UnknownVirtualSlot73(point, dir) * d;
    field_0x4e8 = k;
    if (VehAbs(k) <= 0.0f)
        return 0.0f;
    float m = field_0x434;
    float r = (m * m) / -k * stepTime;
    Vec3 t;
    t.x = r * dir->x;
    t.y = r * dir->y;
    t.z = r * dir->z;
    float s = c * 1.5f;
    if (!(s < 1.0f))
        s = 1.0f;
    Vec3 u;
    u.x = s * t.x;
    u.y = t.y * s;
    u.z = t.z * s;
    Vec3 sumv;
    sumv.x = u.x + velocity.x;
    sumv.y = u.y + velocity.y;
    sumv.z = u.z + velocity.z;
    scratchVector = sumv;
    Vec3* nv = &scratchVector;
    float len2 = nv->y * nv->y + nv->x * nv->x + nv->z * nv->z;
    float len;
    if (len2 == 1.0f)
        len = 1.0f;
    else
        len = (float)sqrt(len2);
    if (VehAbs(len) < 0.01f)
        return 0.0f;
    float sign = -1.0f;
    if (!(k < 0.0f))
        sign = 1.0f;
    float dot = nv->y * velocity.y + nv->x * velocity.x + nv->z * velocity.z;
    sign = VehAbs((float)acos(dot / (len * linearSpeed))) * sign;
    if (!_finite(sign))
        return 0.0f;
    float scale = linearSpeed / len;
    velocity.x = scale * nv->x;
    velocity.y = scale * nv->y;
    velocity.z = scale * nv->z;
    if (movingForward)
        return sign;
    return -sign;
}

// Tier 3 reading: accumulates per-wheel ground normals and suspension loads into one
// support normal and a lean/steer response (via slot 74).  Wheels are averaged, so more than
// one contact normalises the sum.
// Sums the contact normals (and, for wheels with field_0x1c0 set, their +0x230 vectors) of the
// wheels and pushes the vehicle along the resulting cross direction (tier 3). *out receives the
// summed normal (normalised when more than one contact); field_0x4b8/0x43c receive the result
// of slot 74 (deflection angle and angle*dt) using the last flagged wheel's point (+0x23c).
void Vehicle::UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel)
{
    if (crashState != 0)
        return;
    Vec3 sum = g_VehZeroVec3;
    *out = g_VehZeroVec3;
    float load = 0.0f;
    int mixed = 0;
    int flagged = 0;
    int plain = 0;
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (w->inContact || wheel) {
            if (w->field_0x1c0) {
                wheel = w;
                flagged++;
                sum.x += w->field_0x230.x;
                sum.y += w->field_0x230.y;
                sum.z += w->field_0x230.z;
                out->x += w->groundNormal.x;
                out->y += w->groundNormal.y;
                out->z += w->groundNormal.z;
                load += w->contactLoad;
                if (plain > 0)
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
        float len2 = out->y * out->y + out->x * out->x + out->z * out->z;
        if (len2 == 0.0f) {
            *out = g_VehZeroVec3;
        } else {
            float inv = FastInvSqrt(len2);
            out->x = inv * out->x;
            out->y = inv * out->y;
            out->z = inv * out->z;
        }
        load = load / wheelsInContact;
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
    float mag = load;
    if (!(mag < 1.0f))
        mag = 1.0f;
    turnAngle = UnknownVirtualSlot74(&wheel->field_0x23c, &axis, mag, 1.0f);
    turnRate = turnAngle * invStepTime;
}

// ---- impact handlers (slots 18..20) ----
// Commit an impact to a sink: latch the current vector into the previous one, mark it dirty.
static inline void VehCommitImpact(VehicleImpactEvent* ev, VehicleImpactSink** sink)
{
    if (ev->field_0x24)
        (*sink)->previousVector = (*sink)->currentVector;
    (*sink)->updatePending = 1;
    ev->field_0x24 = 0;
}

// Posts a one-shot impact for the first wheel (aux-driven first, then any) or contact whose
// surface material is flagged in the material table and which has not yet reported one.
void Vehicle::UnknownVirtualSlot18(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    VehicleWheel* w;
    int i;
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->field_0x2a8 && w->inContact && !w->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->field_0xa4[0x400 + w->surfaceMaterial]))
            break;
    }
    if (i < wheelCount) {
        w->impactPosted = 1;
        float v = w->field_0x2bc * 20.0f;
        v = (v > 1.0f) ? v : 1.0f;
        ev->impactSink->Method_004B8D90(w->groundPoint, v);
        VehCommitImpact(ev, &ev->impactSink);
        return;
    }
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->inContact && !w->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->field_0xa4[0x400 + w->surfaceMaterial]))
            break;
    }
    if (i < wheelCount) {
        w->impactPosted = 1;
        ev->impactSink->Method_004B8D90(w->groundPoint, 0.0f);
        VehCommitImpact(ev, &ev->impactSink);
        return;
    }
    for (i = 0; i < collisionPointCount; i++) {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->contactActive && !c->impactPosted &&
            (!track || ((VehicleMaterialSet*)track)->field_0xa4[0x400 + c->surfaceMaterial])) {
            c->impactPosted = 1;
            ev->impactSink->Method_004B8D90(c->impactPosition, 0.0f);
            VehCommitImpact(ev, &ev->impactSink);
            return;
        }
    }
}

// Slide/scrape impact: like slot 18 but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot19(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    VehicleWheel* w;
    int i;
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->field_0x2a8 && w->inContact && !w->slidePosted &&
            (!track || ((VehicleMaterialSet*)track)->field_0xa4[0x408 + w->surfaceMaterial]))
            break;
    }
    if (i < wheelCount) {
        if (!(w->field_0x2b8 < 0.95f))
            return;
        w->slidePosted = 1;
        ev->slideSink->Method_004B9DC0(w->groundPoint);
        VehCommitImpact(ev, &ev->slideSink);
        scratchVector2.x = (turnAngle < 0.0f ? -1.0f : 1.0f) * w->field_0x280.z;
        scratchVector2.y = 0.0f;
        scratchVector2.z = -w->field_0x28c;
        scratchVector = modelNode->LocalToWorldDirection(scratchVector2);
        scratchVector.y = w->field_0x2bc * 3.0f;
        float s = w->field_0x290 * invStepTime;
        scratchVector.x *= s;
        scratchVector.y *= s;
        scratchVector.z *= s;
        if (!(scratchVector.x < 15.0f))
            scratchVector.x = 15.0f;
        if (!(scratchVector.y < 18.0f))
            scratchVector.y = 18.0f;
        if (!(scratchVector.z < 15.0f))
            scratchVector.z = 15.0f;
        ev->slideSink->scrapeVector = scratchVector;
        return;
    }
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->inContact && !w->slidePosted &&
            (!track || ((VehicleMaterialSet*)track)->field_0xa4[0x408 + w->surfaceMaterial]))
            break;
    }
    if (i < wheelCount) {
        w->slidePosted = 1;
        if (w->field_0x280.z > 0.2f) {
            VehCommitImpact(ev, &ev->slideSink);
        }
    }
}

// Second scrape channel: like slot 19 (sink at event+0x0c, no per-material table gate) but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot20(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    VehicleWheel* w = 0;
    int i;
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->field_0x2a8 && w->inContact && !w->scrapePosted)
            break;
    }
    if (i < wheelCount) {
        if (!(w->field_0x2b8 < 0.95f))
            return;
        w->scrapePosted = 1;
        ev->scrapeSink->Method_004B9DC0(w->groundPoint);
        VehCommitImpact(ev, &ev->scrapeSink);
        scratchVector2.x = (turnAngle < 0.0f ? -1.0f : 1.0f) * w->field_0x280.z;
        scratchVector2.y = 0.0f;
        scratchVector2.z = -w->field_0x28c;
        scratchVector = modelNode->LocalToWorldDirection(scratchVector2);
        scratchVector.y = w->field_0x2bc * 3.0f;
        float s = w->field_0x290 * invStepTime;
        scratchVector.x *= s;
        scratchVector.y *= s;
        scratchVector.z *= s;
        if (!(scratchVector.x < 15.0f))
            scratchVector.x = 15.0f;
        if (!(scratchVector.y < 18.0f))
            scratchVector.y = 18.0f;
        if (!(scratchVector.z < 15.0f))
            scratchVector.z = 15.0f;
        ev->scrapeSink->scrapeVector = scratchVector;
        return;
    }
    for (i = 0; i < wheelCount; i++) {
        w = wheelList[i];
        if (w->inContact && !w->scrapePosted)
            break;
    }
    if (i < wheelCount) {
        w->scrapePosted = 1;
        if (w->field_0x280.z > 0.2f) {
            VehCommitImpact(ev, &ev->scrapeSink);
        }
    }
}

static inline Vec3 VehCrossA(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}
static inline Vec3 VehCrossB(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
// ---- slot 4 / slot 46 ----
// Slot 4: builds two lever-arm offsets (a1 + a2 x a3, a7 + a8 x a9), each scaled by a14, and hands
// them with the frame data to helper 0x5004a0. Afterwards it caches |*a1| in field_0xbc and
// field_0xcc = the frame transform of field_0xd8 (tier 3).
void Vehicle::UnknownVirtualSlot4(const Vec3* a0, Vec3* a1, const Vec3* a2,
                                  const Vec3* a3, int a4, float a5, int a6, const Vec3* a7,
                                  const Vec3* a8, const Vec3* a9, Vec3* a10, Vec3* a11,
                                  int a12, float* a13, float a14)
{
    scratchVector = VehCrossA(*a2, *a3);
    Vec3 p;
    p.x = (a1->x + scratchVector.x) * a14;
    p.y = (scratchVector.y + a1->y) * a14;
    p.z = (scratchVector.z + a1->z) * a14;
    scratchVector = VehCrossB(*a8, *a9);
    Vec3 q;
    q.x = (scratchVector.x + a7->x) * a14;
    q.y = (scratchVector.y + a7->y) * a14;
    q.z = (scratchVector.z + a7->z) * a14;
    ContactSolveImpulse(restitution, a0, invMass, modelNode, &p, a3,
                        crashState ? &shapeInvInertia : &invInertia, &angularVelocity, a1, a5,
                        (SoultreeObject*)a6, &q, a9, a10, a11, (Vec3*)a12, a13);
    float len2 = a1->x * a1->x + a1->y * a1->y + a1->z * a1->z;
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
// rotated by the frame transform; falls back to field_0x194 when disabled or > 2 wheels.
Vec3* Vehicle::UnknownVirtualSlot46(Vec3* out, float arg)
{
    Vec3* src = &rotationPivot;
    Vec3 t;
    if (!airborne && !pointsTouching && wheelsInContact <= 2) {
        if (wheelsInContact == 2) {
            VehicleWheel* a = secondaryWheel;
            VehicleWheel* b = primaryWheel;
            if (!a->field_0x2a8) {
                a = primaryWheel;
                if (!a->field_0x2a8) {
                    *out = *src;
                    return out;
                }
                b = secondaryWheel;
            } else {
                b = primaryWheel;
            }
            // a has the aux object, b is the other wheel
            if (a->field_0x2b8 < 0.9f) {
                scratchVector = VehOffset(b->wheelPosition, position);
            } else {
                Vec3 d = VehOffset(a->wheelPosition, b->wheelPosition);
                Vec3 mid;
                mid.x = d.x * 0.5f;
                mid.y = d.y * 0.5f;
                mid.z = d.z * 0.5f;
                mid.x += b->wheelPosition.x;
                mid.y += b->wheelPosition.y;
                mid.z += b->wheelPosition.z;
                scratchVector = VehOffset(mid, position);
            }
        } else {
            Vec3* p = &primaryWheel->wheelPosition;
            scratchVector = VehOffset(*p, position);
        }
        t = modelNode->WorldToLocalDirection(scratchVector);
        src = &t;
    }
    *out = *src;
    return out;
}

// ---- slot 49: per-frame vehicle step ----
// Slot 49 is the per-frame step of the vehicle (tier 3 phase names, from the slot calls):
//   A. setup      - slot 9 fixed-timestep accumulator gives the substep count; slots 30/64/60/65
//                   refresh per-frame inputs; an initial contact query (VehContactsA) when more
//                   than one substep is pending.
//   B. per substep, in order:
//        1. input/state  - decay the speed-state timer, poll slots 80-82 (input flags), and
//                          the field_0x4f4 countdown (slot 50 clears it);
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
// where tau is field_0x04, field_0x08 is the blend factor a, and field_0x00 the smoothed value.
static inline void VehSmooth(VehicleSmoother* s, float dt, float target)
{
    float step = dt;
    if (!(step < s->timeConstant))
        step = s->timeConstant;
    s->blendFactor = step / s->timeConstant;
    s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;
}

void Vehicle::UnknownVirtualSlot49(float frame)
{
    int steps;
    Vec3 up;          // frame reference vector (from field_0x188)
    Vec3 tmp;
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
    while (steps > 0) {
        VehicleSpeedState* ss = engineState;
        if (ss->gearTimer != 0.0f) {
            ss->gearTimer -= stepTime;
            if (ss->gearTimer < 0.0f)
                ss->gearTimer = 0.0f;
        }
        Vec3 zero = g_VehZeroVec3;
        if (crashState == 0) {
            if (Method_00529280())
                engineState->field_0x08 = 1;
        } else if (totalWeight != baseWeight) {
            if (UnknownVirtualSlot10())
                UnknownVirtualSlot0(0.0f);
        }
        field_0x47a = UnknownVirtualSlot82() != 0;
        field_0x479 = crashState == 0 && (field_0x47a || UnknownVirtualSlot81());
        field_0x478 = crashState == 0 && UnknownVirtualSlot80();
        engineState->Method_004D2F50(stepTime, field_0x478, field_0x479);
        engineState->Method_004D3030(field_0x478, linearSpeed);
        if (hit) {
            int dummy = 0;
            UnknownVirtualSlot11(hit, &up, &zero, &tmp, &dummy);
            UnknownVirtualSlot33(&up, &tmp, &bodyUp, &zero, dummy, UnknownVirtualSlot32());
            steps = 0;
            stepRemainder = 0.0f;
            break;
        }
        prevSpeed = linearSpeed;
        if (linearSpeed < 0.001f || !_finite(linearSpeed)) {
            prevSpeed = 0.0f;
            velocity = g_VehZeroVec3;
            linearSpeed = 0.0f;
        }
        centerNode->GetPositionIn(0, &centerOfMass);
        float t53 = UnknownVirtualSlot53();
        Method_00528EB0();
        if (spawnProtectTimer > 0.0f) {
            spawnProtectTimer -= stepTime;
            if (spawnProtectTimer <= 0.0f)
                UnknownVirtualSlot50(0, 0, 0);
        }
        allWheelsInContact = wheelsInContact == wheelCount;
        anyWheelInContact = wheelsInContact != 0;
        speed = prevSpeed;
        up = weightForce;
        Vec3 a, b;
        Vec3 basis = *UnknownVirtualSlot55(&a, &b);
        UnknownVirtualSlot56(&basis, 1, 0);
        res = UnknownVirtualSlot5(0);
        res = VehContactsA(crashState || (steps == 1 && res), &touchingPointCount, collisionPointCount, ((VehicleContact**)collisionPoints),
                           terrain, &centerOfMass, collisionShape, collisionRadius);
        if (res)
            pointsTouching = touchingPointCount > 0;
        Method_00529A20();
        UnknownVirtualSlot8();
        UnknownVirtualSlot6(&up, &speed);
        UnknownVirtualSlot7(&up);
        field_0x4b4 = UnknownVirtualSlot75();
        bool settled = !anyWheelInContact && !pointsTouching;
        UnknownVirtualSlot71(settled);
        airborne = settled;
        if (settled)
            landingLatched = 0;
        Vec3 lift;
        if (contactTotal > 0) {
            if (anyWheelInContact) {
                UnknownVirtualSlot72(&lift, 0);
                UnknownVirtualSlot86();
            } else {
                field_0x5a0 = 0;
                turnAngle = 0.0f;
                turnRate = 0.0f;
            }
            if (pointsTouching) {
                if (!res)
                    VehContactsC(collisionPointCount, ((VehicleContact**)collisionPoints));
                UnknownVirtualSlot31();
            }
        } else {
            field_0x5a0 = 0;
            turnAngle = 0.0f;
            turnRate = 0.0f;
        }
        if (field_0x5a0 || (pointsTouching && (!anyWheelInContact || crashState))) {
            Vec3 scale;
            if (crashState) {
                scale.x = 1.0f;
                scale.y = 1.0f;
                scale.z = 1.0f;
            } else {
                scale.x = 0.3f;
                scale.y = 0.1f;
                scale.z = 0.3f;
            }
            Vec3 o1, o2, o3;
            VehContactsB(collisionPointCount, ((VehicleContact**)collisionPoints), &scale, &worldAngularVelocity, &velocity, &centerOfMass,
                         &position, &o3, &o2, &o1);
            UnknownVirtualSlot3(&o2, &o1, &o3, &scale, 0x67, 0, &tmp.x);
            if (prevSpeed * 1.3f < linearSpeed && linearSpeed > 5.0f) {
                float r = prevSpeed / linearSpeed;
                velocity.x *= r;
                velocity.y *= r;
                velocity.z *= r;
                linearSpeed = prevSpeed;
            }
            if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
                velocity = g_VehZeroVec3;
                angularVelocity.y = 0.0f;
                linearSpeed = 0.0f;
            }
        }
        float aq = turnAngle;
        if (aq < 0.0f)
            aq = -aq;
        Vec3 zeroB = g_VehZeroVec3;
        UnknownVirtualSlot13(&up, &zeroB, t53);
        if (anyWheelInContact) {
            Method_00527A20(&speed, &zero, &up);
            if (crashState == 0 || !pointsTouching) {
                Method_005293E0(&speed);
                Method_00529450(&up, &zeroB);
            }
            Method_00529C20(&up, &zeroB, t53);
        }
        UnknownVirtualSlot14(&up, &zeroB, &zero);
        VehSmooth(verticalAccelSmoother, stepTime, (velocity.y - prevVelocity.y) * invStepTime);
        smoothedVerticalAccel = verticalAccelSmoother->smoothedValue;
        float k = UnknownVirtualSlot61(aq);
        Vec3 aim = *UnknownVirtualSlot46(&tmp, k);
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
                    bool doit = true;
                    if (aq < 0.0001f) {
                        if (VehAbs(k) > 0.0001f) {
                            scratchVector = *UnknownVirtualSlot54(&tmp);
                            tiltAxisLocal = modelNode->WorldToLocalDirection(scratchVector);
                        } else {
                            doit = false;
                        }
                    } else {
                        tiltAxisLocal = modelNode->WorldToLocalDirection(lift);
                    }
                    if (doit) {
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
                    UnknownVirtualSlot92(&up, &tmp);
            }
            UnknownVirtualSlot90(&steps, frame);
        }
        modelNode->GetPosition(&position);
        centerNode->GetPositionIn(0, &centerOfMass);
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
        steps = UnknownVirtualSlot28(steps) - 1;
        lastStepTime = stepTime;
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
    }
    UnknownVirtualSlot21();
    scratchVector = modelNode->WorldToLocalDirection(velocity);
    VehSmooth(forwardAccelSmoother, frameTime, (scratchVector.z - prevLocalForwardVelocity) / frameTime);
    smoothedForwardAccel = forwardAccelSmoother->smoothedValue;
    prevLocalForwardVelocity = scratchVector.z;
}

// ---- slot 38: collision response dispatch (tier 3 names) ----
// Called with an event code and the other party of a collision. Builds the relative contact
// geometry (contact point minus our position, other body's velocity/normal) and hands it to the
// impulse solver in slot 4 (other body present) or slot 3 (static/no body). Afterwards kills
// residual creep velocity and, if requested, runs the post-collision update.
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

void Vehicle::UnknownVirtualSlot38(int a, int b, void* c)
{
    float l10;                     // the other body's field_0x24 (float) or 0
    int l14;
    Vec3 s;                     // scale/mask vector (1,1,1)
    Vec3 p;                     // lever vector
    Vec3 v30;
    Vec3 v3c;
    Vec3 v48;
    int ctx;
    int hasBody;
    Vehicle* other = 0;
    Vec3* otherVel = 0;
    Vec3 rel;

    s = VehOnes();
    switch (b) {
    case 0x66:
    case 0x6a:
        s = VehOnes();
        hasBody = 0;
        if (field_0x124 && (field_0x124->statusFlags & 1)) {
            Vec3 pos = ((VehicleContactSet*)collisionObject)->contactPoint;
            ((VehicleImpactSink*)field_0x5ac)->Method_004B9DC0(pos);
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->scrapeVector = p;
            ((VehicleImpactSink*)field_0x5ac)->updatePending = 1;
        }
        break;
    case 0x3e9:
        s = VehOnes();
        hasBody = 0;
        break;
    case 0:
    case 1:
        hasBody = 1;
        other = ((VehicleCollisionEvent*)c)->otherBody;
        l10 = other->invMass;
        otherVel = &other->velocity;
        ctx = (int)other->modelNode;
        v3c = VehOffset(((VehicleContactSet*)collisionObject)->contactPoint, other->centerOfMass);
        v48 = other->worldAngularVelocity;
        c = &other->angularVelocity;
        v30 = other->invInertia;
        break;
    case 0x69:
        hasBody = 1;
        ctx = *(int*)((char*)((VehicleCollisionEvent*)c)->otherBody + 0x34);
        v30 = g_VehZeroVec3; v3c = g_VehZeroVec3; v48 = g_VehZeroVec3;
        l10 = 0;
        p.x = 1.0f; p.y = 1.0f; p.z = 1.0f;
        s = p;
        otherVel = (Vec3*)(ctx + 0x40);
        c = 0;
        break;
    case 0x2711:
        hasBody = 1;
        ctx = *(int*)((char*)((VehicleCollisionEvent*)c)->otherBody + 0x1a0);
        v30 = g_VehZeroVec3; v3c = g_VehZeroVec3; v48 = g_VehZeroVec3;
        l10 = 0;
        p.x = 1.0f; p.y = 1.0f; p.z = 1.0f;
        s = p;
        otherVel = (Vec3*)(ctx + 0x224);
        c = 0;
        break;
    default:
        return;
    }

    rel = VehOffset(((VehicleContactSet*)collisionObject)->contactPoint, centerOfMass);
    if (hasBody) {
        l14 = (b == 0x2711 || b == 0x69) ? 0 : (int)otherVel;
        UnknownVirtualSlot4(&((VehicleContactSet*)collisionObject)->contactNormal, &velocity, &worldAngularVelocity, &rel, b, l10, ctx,
                            otherVel, (Vec3*)c, &v48, &v3c, &v30, l14, &l10, 1.0f);
        if (other) {
            *((char*)other + 0x10a) = 0;
            float len2 = otherVel->x * otherVel->x + otherVel->y * otherVel->y + otherVel->z * otherVel->z;
            linearSpeed = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
            worldAngularVelocity = other->modelNode->LocalToWorldDirection(*(Vec3*)c);
        }
    } else {
        Vec3 t;
        t.x = worldAngularVelocity.x * s.x; t.y = worldAngularVelocity.y * s.y; t.z = worldAngularVelocity.z * s.z;
        scratchVector.x = rel.z * t.y - rel.y * t.z;
        scratchVector.y = rel.x * t.z - rel.z * t.x;
        scratchVector.z = rel.y * t.x - rel.x * t.y;
        p.x = velocity.x + scratchVector.x;
        p.y = velocity.y + scratchVector.y;
        p.z = velocity.z + scratchVector.z;
        UnknownVirtualSlot3(&((VehicleContactSet*)collisionObject)->contactNormal, &p, &rel, &s, b, 0, &l10);
    }
    if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
        velocity = g_VehZeroVec3;
        angularVelocity.y = 0.0f;
        linearSpeed = 0;
    }
    if (a)
        Method_00526830();
}

// ---- wave 2 ----

// Scene-node method 0x004444e0 (purpose unknown) is not in SoultreeObject yet; view the
// node through a local stand-in that declares it.
struct VehSceneNodeView {
    void Method_004444E0();
    void Method_004FBD70(const Vec3* axisZ, const Vec3* axisY, int a, int b);   // 0x004fbd70, purpose unknown
};

void Vehicle::UnknownVirtualSlot44()
{
    field_0x4a0 = 1;
    GameObjectVirtualSlot4();
    ((VehSceneNodeView*)modelNode)->Method_004444E0();
}

// Input-map driven state test (provisional): the 0x46c/0x470 pair short-circuits to "true".
int Vehicle::UnknownVirtualSlot80()
{
    if ((inputDevice && throttleInput > 0.33f)
        || (inputMap->UnknownVirtualSlot3(0, 2, 0x3f, 0)
            && !UnknownVirtualSlot77(engineState->field_0x00 == 0.0f)))
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
// contact offset from the position when one is recorded, and clears the dirty byte at 0x138.
int Vehicle::UnknownVirtualSlot28(int arg)
{
    collisionObject->Fn_00435fb0();
    int wasSet = crashState != 0;
    collisionObject->Fn_00438e70();
    if (collisionObject->hasContact) {
        position -= *(const Vec3*)collisionObject->contactRecord;
        modelNode->SetPosition(position);
        collisionObject->Fn_00435fb0();
        if (crashState && !wasSet) {
            arg = 1;
            stepRemainder = 0.0f;
        }
    }
    if (field_0x138) {
        UnknownVirtualSlot27();
        field_0x138 = 0;
    }
    return arg;
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

// Tier 3 reading: latches the throttle-like axis (+0x500) into field_0x470 (dead zone 0.2,
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
    if (src && !src->Method_004897E0(4)
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

// Tier 3 reading: when the facing vector (0x88) has no horizontal part it is rebuilt from the
// previous up vector (0x94) with the sign of its y, then the basis block is reset to the
// identity-like values and pushed to the scene node.
void Vehicle::UnknownVirtualSlot36()
{
    if (bodyForward.x == 0.0f && bodyForward.z == 0.0f) {
        float s = bodyForward.y >= 0.0f ? -1.0f : 1.0f;
        bodyForward = s * bodyUp;
    }
    bodyUp = g_VehZeroVec3_005778c8;
    bodyForward.y = 0.0f;
    UnknownVirtualSlot35(1, 0);
    UnknownVirtualSlot34();
    bodyCosRoll = 1.0f;
    bodyCosPitch = 1.0f;
    bodyRoll = 0.0f;
    bodyPitch = 0.0f;
    bodySinRoll = 0.0f;
    bodySinPitch = 0.0f;
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
    steerState->steerNode->SetPosition(g_VehZeroVec3_005778a8);
    ((VehSceneNodeView*)steerState->steerNode)->Method_004FBD70(&bodyForward, &bodyUp, 0, 1);
    steerState->Method_00504E20(0, poseNode);
}

// Tier 3 reading: classifies the (field_0x504.y, field_0x474) input pair into one of eight
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
// the sum when more than one wheel contributed (zero stays zero) and returns |sum . field_0xa0|;
// 1.0 when no wheel contributed.
float Vehicle::UnknownVirtualSlot75()
{
    Vec3 sum = g_VehZeroVec3;
    int n = 0;
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        if (!w->field_0x1c0) {
            sum += w->field_0x230;
            n++;
        }
    }
    if (n > 1) {
        float scale = sum.x * sum.x + sum.z * sum.z + sum.y * sum.y;
        if (scale == 0.0f) {
            sum = g_VehZeroVec3;
        } else {
            scale = FastInvSqrt(scale);
            sum.x = sum.x * scale;
            sum.y = sum.y * scale;
            sum.z = sum.z * scale;
        }
    } else if (n == 0) {
        return 1.0f;
    }
    float d = sum.y * savedForward.y + sum.x * savedForward.x + sum.z * savedForward.z;
    if (d < 0.0f)
        d = -d;
    return d;
}

// 0x00528400: average of the wheels' contact normals (+0xe4), normalised (tier 3 reading).
Vec3* Vehicle::Method_00528400(Vec3* out)
{
    Vec3 sum = wheelList[0]->groundNormal;
    int n = wheelCount;
    for (int i = 1; i < n; i++)
        sum += wheelList[i]->groundNormal;
    float k = 1.0f / n;
    Vec3 avg = sum * k;
    *out = VehNormalized(avg);
    return out;
}

// Same average restricted to the wheels in contact (+0x260), divided by the contact count
// field_0x4a8; with no contacts the first wheel's normal is returned unchanged.
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
    float k = 1.0f / count;
    Vec3 avg = sum * k;
    *out = VehNormalized(avg);
    return out;
}

// Tier 3 reading: resets the vehicle state after the base reset.  The contact flags and the
// state counters are cleared exactly as in slot 67, the lean/steer/wheel members get their
// defaults, and the speed state at +0x480 is re-seeded with a random start value
// (rand() / 32768 scaled by a table entry).
void Vehicle::UnknownVirtualSlot1(float value)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot1(value);
    field_0x4e8 = 0.0f;
    field_0x434 = 0.0f;
    movingForward = 1;
    for (int i = 0; i < collisionPointCount; i++) {
        VehicleContact* c = ((VehicleContact**)collisionPoints)[i];
        if (c->field_0x04 != 0)
            c->contactActive = 0;
    }
    crashState = 0;
    crashTimer = 0.0f;
    prevCrashState = 0;
    field_0x5a0 = 0;
    landingLatched = 0;
    smoothedVerticalAccel = 0.0f;
    smoothedForwardAccel = 0.0f;
    prevLocalForwardVelocity = 0.0f;
    tiltAxisLocal = g_VehZeroVec3;
    leanError = 0.0f;
    turnRate = 0.0f;
    steerRate = 0.0f;
    leanAngle = 0.0f;
    leanCos = 1.0f;
    field_0x4b4 = 1.0f;
    targetLeanAngle = 0.0f;
    contactTotal = 0;
    field_0x520 = 1;
    throttleInput = 0.0f;
    prevControlInput = g_VehZeroVec3;
    field_0x478 = false;
    field_0x479 = false;
    field_0x47a = false;
    VehicleSpeedState* s = engineState;
    if (s) {
        s->gear = 0;
        s->field_0x04 = 0;
        s->field_0x84 = 0;
        s->field_0x00 = 0.0f;
        s->gearTimer = 0.0f;
        s->field_0x08 = 1;
        float unit = (float)rand() * 3.05175781e-05f;
        s->randomStart = unit * s->field_0x60[s->gear] + s->field_0x30[s->gear].a;
    }
    Method_00525C60();
    Method_00525A90();
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

// Same solver call as SoultreePhysicsBaseObject slot 3, but while the vehicle is in state
// 0x444 it uses a fixed 0.1 time scale, the vector at +0xf0 and no a6 (tier 3 reading).
// The world-space copy of the local velocity (+0xcc) is skipped for the event code 0x67.
void Vehicle::UnknownVirtualSlot3(const Vec3* a, const Vec3* b, const Vec3* c, const Vec3* d,
                                  int e, int f, float* g)
{
    if (crashState)
        Fn_500220(0.1f, invMass, modelNode, a, b, c, &shapeInvInertia, d, &angularVelocity,
                  &velocity, g, 0);
    else
        Fn_500220(restitution, invMass, modelNode, a, b, c, &invInertia, d, &angularVelocity,
                  &velocity, g, f);
    linearSpeed = VehLengthAcc(velocity);
    if (e != 0x67)
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
}

// Tier 3 reading: places the vehicle at a (with its y forced to -1000 when e is zero), takes
// the orientation vectors b/c, settles the wheels twice via slot 58 (d, e), refreshes the wheel
// and body node positions and finishes with slot 50 (flag, slot 45 value, 0).
int Vehicle::UnknownVirtualSlot33(const Vec3* a, const Vec3* b, const Vec3* c, const Vec3* d,
                                  int e, float f)
{
    UnknownVirtualSlot1(f);
    bodyForward = *b;
    bodyUp = *c;
    float y;
    if (e)
        y = a->y;
    else
        y = -1000.0f;
    modelNode->SetPosition(a->x, y, a->z);
    modelNode->GetPosition(&position);
    UnknownVirtualSlot36();
    Method_00528EB0();
    UnknownVirtualSlot58((Vec3*)d, e);
    Method_00528EB0();
    UnknownVirtualSlot58((Vec3*)d, e);
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* w = wheelList[i];
        w->sceneNode->GetPositionIn(0, &w->nodePosition);
    }
    centerNode->GetPositionIn(0, &centerOfMass);
    Method_0x004a8b00();
    if (collisionObject)
        collisionObject->Fn_00435fe0();
    justReset = 1;
    attachmentResetPending = 1;
    respawnPending = 0;
    int flag = 1;
    if (UnknownVirtualSlot45() == 0.0f)
        flag = 0;
    UnknownVirtualSlot50(flag, UnknownVirtualSlot45(), 0);
    return 0;
}

static inline float VehLenSq(const Vec3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Tier 3 reading: when field_0xd8 (a velocity-like vector) is long enough, remembers its
// direction in field_0x1ac and rotates the scene node about field_0x194 by |v| * dt.
void Vehicle::UnknownVirtualSlot95()
{
    float lenSq = SquareMagnitude(angularVelocity);
    float len;
    if (lenSq == 0.0f)
        len = 0.0f;
    else if (lenSq == 1.0f)
        len = 1.0f;
    else
        len = 1.0f / FastInvSqrt(lenSq);
    float mag = len * stepTime;
    if (_finite(mag) && lenSq >= 0.0001f) {
        scratchVector = angularVelocity;
        float sq = VehLenSq(scratchVector);
        if (sq == 0.0f) {
            scratchVector = g_VehZeroVec3;
        } else {
            float s = FastInvSqrt(sq);
            scratchVector.x = s * scratchVector.x;
            scratchVector.y = s * scratchVector.y;
            scratchVector.z = s * scratchVector.z;
        }
        modelNode->RotateAboutPoint(rotationPivot, scratchVector, lenSq);
    }
}

// ---- wave 3: non-virtual helpers ----

// 0x00526830: asks the shared stub 0x00478fe0 (always zero here) whether the vehicle crashed; when it
// did, the control state is cleared and the horizontal heading is recorded (tier 3 reading;
// field_0x444 is the crash state, see Bike slot 99).
int Vehicle::Method_00526830()
{
    int crashed = Method_00478FE0();
    crashState = crashed;
    if (crashed) {
        Vec3 heading(bodyForward.x, 0.0f, bodyForward.z);
        field_0x430 = 0;
        field_0x431 = 0;
        field_0x433 = 0;
        field_0x574 = heading;
        field_0x464 = savedUp.y < 0.0f;
        field_0x45c = savedYaw;
        field_0x138 = 1;
    }
    return crashed;
}

// 0x00525c60: runs virtual slot 0 of every object in the two owned arrays.
void Vehicle::Method_00525C60()
{
    for (int i = 0; i < earlyTickerCount; i++)
        earlyTickers[i]->UnknownVirtualSlot0();
    for (int j = 0; j < lateTickerCount; j++)
        lateTickers[j]->UnknownVirtualSlot0();
}

int Vehicle::Method_00525CB0(VehicleTicker* t)
{
    if (earlyTickerCount < earlyTickerCapacity) {
        earlyTickers[earlyTickerCount] = t;
        earlyTickerCount++;
        return 1;
    }
    return 0;
}

int Vehicle::Method_00525CF0(VehicleTicker* t)
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
int Vehicle::Method_00525D30(VehicleWheel* wheel, int a2, int a3, int a4, VehicleWheelAux* aux)
{
    if (wheelCount < wheelCapacity) {
        wheel->primaryAux = a3;
        wheel->secondaryAux = a4;
        wheel->field_0x2a8 = aux;
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

// 0x005293e0: for every wheel that has an attachment value, apply its load weight.
void Vehicle::Method_005293E0(float* speed)
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->field_0x268 && (wheel->primaryAux || wheel->secondaryAux))
            VehWheelApply(wheel->loadWeight, field_0x4dc, lastStepTime, speed);
    }
}

// 0x005299e0: exact component-wise equality of two vectors.
int __cdecl VehVec3Equal(const Vec3* a, const Vec3* b)
{
    if (a->x == b->x && a->y == b->y && a->z == b->z)
        return 1;
    return 0;
}

// 0x00529280: asks every wheel (slot 83) and ramps its +0x29c level toward 1 when the answer is
// non-zero, toward 0 otherwise. Returns the first non-zero answer.
int Vehicle::Method_00529280()
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
void Vehicle::Method_00529A20()
{
    int i = 0;
    field_0x5a0 = 0;
    for (; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        if (wheel->inContact) {
            wheel->Method_005135F0(&centerOfMass, worldAngularVelocity, velocity, linearSpeed, &field_0x434, &movingForward);
            wheel->Method_00513C70(lastStepTime, (unsigned char)pointsTouching, crashState, linearSpeed, &velocity, &savedForward);
            if (wheel->field_0x15c)
                field_0x5a0 = 1;
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
void Vehicle::Method_00529C20(Vec3* up, Vec3* zero, float d)
{
    if (!airborne) {
        for (int i = 0; i < wheelCount; i++) {
            VehicleWheel* wheel = wheelList[i];
            if (wheel->inContact) {
                if (wheel->field_0x16c == 2) {
                    zero->x = d * zero->x;
                    zero->y = d * zero->y;
                    zero->z = d * zero->z;
                    wheel->field_0x16c = 0;
                }
                if (wheel->field_0x2a8) {
                    up->x = up->x + wheel->field_0x248.x;
                    Vec3* force = &wheel->field_0x248;
                    up->y = force->y + up->y;
                    up->z = force->z + up->z;
                    Vec3* world = &scratchVector2;
                    Vec3* local = &scratchVector;
                    *world = UnknownVirtualSlot76(&wheel->field_0xf0, force);
                    *local = modelNode->WorldToLocalDirection(*world);
                    zero->x = zero->x + local->x;
                    zero->y = local->y + zero->y;
                    zero->z = local->z + zero->z;
                }
            }
        }
    }
}

// 0x00525a90: resets every wheel's contact state and vectors to defaults.
void Vehicle::Method_00525A90()
{
    for (int i = 0; i < wheelCount; i++) {
        VehicleWheel* wheel = wheelList[i];
        wheel->field_0x2a4 = 1.0f;
        wheel->field_0x1e8 = g_VehZeroVec3;
        wheel->field_0x248 = g_VehZeroVec3;
        wheel->field_0x280.x = 0.0f;
        wheel->field_0x254 = g_VehZeroVec3;
        wheel->field_0x2b8 = 1.0f;
        wheel->field_0x280.z = 1.0f;
        wheel->field_0x2bc = 0.0f;
        wheel->rampLevel = 0.0f;
        wheel->field_0x290 = 0.0f;
        wheel->field_0x278 = 0;
        wheel->field_0x27c = 0;
        wheel->inContact = 0;
        wheel->field_0x28c = 0.0f;
        wheel->field_0x26c = 0;
        wheel->sceneNode->GetPositionIn(0, &wheel->nodePosition);
        wheel->field_0x16c = 0;
        wheel->field_0x108 = g_VehZeroVec3;
        wheel->field_0x114 = g_VehZeroVec3;
        wheel->field_0x150 = -999.0f;
        wheel->field_0x148 = 0;
        wheel->field_0x14c = 0;
        wheel->appliedShare = g_VehZeroVec3;
        wheel->field_0x15c = 0;
        wheel->field_0x13c = 0.0f;
        wheel->field_0x130 = g_VehZeroVec3;
        wheel->field_0x12c = 0;
        wheel->field_0x120 = g_VehZeroVec3;
    }
}
