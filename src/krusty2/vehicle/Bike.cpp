// Bike.cpp: reconstruction of the motorcycle physics layer (see Bike.h).
#include <float.h>
#include <math.h>
#include <string.h>
#include "vehicle/Bike.h"
#include "collision/CollisionObject.h"

static inline float BikeMin(float a, float b) { return a < b ? a : b; }
static inline float BikeMaxF(float a, float b) { return a > b ? a : b; }
static inline float BikeAbs(float x);   // defined below (slot 99 region)

float Bike::UnknownVirtualSlot32()
{
    return loadWeight;
}

float Bike::UnknownVirtualSlot47(float)
{
    return 1.05f - controlInput.y * 0.25f;
}

int Bike::UnknownVirtualSlot98()
{
    if (crashState && field_0x604->a_0x44)
        return 1;
    return 0;
}

int Bike::UnknownVirtualSlot42()
{
    if (crashState == 0 && field_0x433 >= 0 && field_0x430 && riderCharacter->c_0xc)
        return 1;
    return 0;
}

int Bike::UnknownVirtualSlot66()
{
    if (crashState || field_0x430)
        return 1;
    return 0;
}

static inline float BikeDot(const Vec3* a, const Vec3* b)
{
    float d = a->y * b->y + a->x * b->x;
    d += a->z * b->z;
    return d;
}

float Bike::UnknownVirtualSlot75()
{
    float d = BikeDot(&savedForward, &rearWheel->w_0x230);
    if (d < 0.0f)
        d = -d;
    return d;
}

// Out-of-line call views for Method_0x0040a520 (see math/Math3D.h): the Vec3 constructor
// 0x00404e60 and operator*= 0x0040ae00 (Bike.cpp's own COMDAT copy) are called at the
// marked sites.
struct BikeVec3Call : Vec3 {
    BikeVec3Call(float x_, float y_, float z_);
};
// Dot product in the operand order retail 0x0040a520 evaluates (y and x terms, then z).
static inline float BikeDotZ(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}
struct BikeVec3ScaleAssign : Vec3 {
    BikeVec3ScaleAssign& operator*=(float s);
};
// By-value views of the out-of-line operators (hidden result pointer first, same ABI as
// the pointer forms in math/Math3D.h): operator- 0x00421d00, operator*(Vec3, float)
// 0x005015b0 and DotProduct 0x0040ae30.
Vec3 BikeVec3Sub(const Vec3& a, const Vec3& b);
Vec3 BikeVec3Scale(const Vec3& v, float s);
float BikeVec3Dot(const Vec3& a, const Vec3& b);

// 0x0040a520 (tier 3 reading).  Builds a world-space steering torque `torque`: a quarter of
// the force accumulator weightForce, turned by the front wheel's contact (offset x w_0x120) and by each
// active +0x5f8/+0x5fc contact (offset x the contact's axis scaled by the negative
// projection), or by w_0x274 * w_0x20c while airborne.  Its body-space direction
// (projected on the wheel frame's axis, normalised) scales the per-axis gains into the
// integrator field_0x61c; the integrator's length, signed by that axis, is the angle fed to
// the steer state unless the limit (0.785 rad) is reached, in which case the angle is
// clamped and the integrator reversed.  Returns the normalised projection.
float Bike::Method_0x0040a520()
{
    Vec3 push;
    Vec3 base;
    Vec3 torque;
    Vec3 offset;
    float len;
    float result;
    torque.x = weightForce.x * 0.25f;
    torque.y = weightForce.y * 0.25f;
    torque.z = weightForce.z * 0.25f;
    base = torque;
    int driven = 0;

    if (airborne) {
        Vec3 spin = frontWheel->w_0x274 * frontWheel->w_0x20c;
        torque = CrossProduct(spin, torque);
        result = 0.0f;
    } else {
        if (frontWheel->inContact) {
            offset = BikeVec3Sub(frontWheel->wheelPosition, frontWheel->nodePosition);
            driven = 1;
            torque = CrossProduct(offset, frontWheel->w_0x120);
            frontWheel->w_0x16c = 2;
            if (field_0x5f8->active) {
                torque = BikeVec3Sub(field_0x5f8->point, frontWheel->nodePosition);
                float d = BikeVec3Dot(torque, field_0x5f8->axis);
                if (d < 0.0f)
                    push = BikeVec3Scale(field_0x5f8->axis, -d);
                else
                    push = Vec3(0.0f, 0.0f, 0.0f);
                torque += CrossProduct(offset, push);
                field_0x5f8->state = 2;
            } else if (field_0x5fc->active) {
                float d = BikeVec3Dot(base, field_0x5fc->axis);
                if (d < 0.0f)
                    push = BikeVec3Scale(field_0x5fc->axis, -d);
                else
                    push = BikeVec3Call(0.0f, 0.0f, 0.0f);
                offset = BikeVec3Sub(field_0x5fc->point, frontWheel->nodePosition);
                torque += CrossProduct(offset, push);
                field_0x5fc->state = 2;
            }
        } else if (field_0x5f8->active) {
            torque = BikeVec3Sub(field_0x5f8->point, frontWheel->nodePosition);
            driven = 1;
            float d = BikeVec3Dot(torque, field_0x5f8->axis);
            if (d < 0.0f)
                push = BikeVec3Scale(field_0x5f8->axis, -d);
            else
                push = BikeVec3Call(0.0f, 0.0f, 0.0f);
            torque = CrossProduct(offset, push);
            field_0x5f8->state = 2;
        } else if (field_0x5fc->active) {
            driven = 1;
            float d = BikeVec3Dot(base, field_0x5fc->axis);
            if (d < 0.0f)
                push = BikeVec3Scale(field_0x5fc->axis, -d);
            else
                push = BikeVec3Call(0.0f, 0.0f, 0.0f);
            offset = BikeVec3Sub(field_0x5fc->point, frontWheel->nodePosition);
            torque = CrossProduct(offset, push);
            field_0x5fc->state = 2;
        } else {
            result = 0.0f;
            goto integrate;
        }
        float lenSq = BikeVec3Dot(torque, torque);
        if (lenSq == 1.0f) {
            len = 1.0f;
        } else {
            len = (float)sqrt(lenSq);
            if (len <= 0.001f) {
                result = 0.0f;
                goto integrate;
            }
        }
        offset = modelNode->WorldToLocalDirection(torque);
        float d = BikeDotZ(offset, frontWheel->w_0x1c0->axis);
        if (d < 0.0f)
            d = -d;
        result = d / len;
        float k = result * stepTime;
        field_0x61c += BikeVec3Call(offset.x * field_0x610.x * k, offset.y * field_0x610.y * k,
                                    offset.z * field_0x610.z * k);
    }
integrate:
    float sign = -1.0f;
    if (BikeDotZ(field_0x61c, frontWheel->w_0x1c0->axis) >= 0.0f)
        sign = 1.0f;
    float lenSq = BikeVec3Dot(field_0x61c, field_0x61c);
    if (lenSq == 1.0f)
        len = 1.0f;
    else
        len = (float)sqrt(lenSq);
    float angle = len * stepTime * sign;
    if (BikeAbs(angle + steerState->steerAngle) < 0.785f) {
        steerState->AddAngle(angle, poseNode);
        *(BikeVec3ScaleAssign*)&field_0x61c *= 0.99f;
        return result;
    }
    if (BikeAbs(steerState->steerAngle) < 0.785f) {
        steerState->SetAngle((steerState->steerAngle < 0.0f ? -1.0f : 1.0f) * 0.7851f, poseNode);
        float decay = -0.02f;
        if (!driven)
            decay = -0.1f;
        *(BikeVec3ScaleAssign*)&field_0x61c *= decay;
        return result;
    }
    steerState->AddAngle(angle, poseNode);
    *(BikeVec3ScaleAssign*)&field_0x61c *= 0.99f;
    return 0.0f;
}

float Bike::UnknownVirtualSlot53()
{
    if (crashState == 0) {
        float f = steerRate * stepTime;
        steerState->AddAngle(f, poseNode);
        return 1.0f;
    }
    return 1.0f - Method_0x0040a520();
}

void Bike::UnknownVirtualSlot96()
{
    field_0x524[0] = 0.675f;
    field_0x63c = 1.0f;
    field_0x524[1] = 1.0f;
    field_0x524[2] = 1.0f;
    field_0x524[3] = 1.0f;
    field_0x524[4] = 1.0f;
    field_0x524[5] = 1.0f;
}

static inline float BikeAbsRef(const float& x)
{
    float r = x;
    if (r < 0.0f)
        r = -r;
    return r;
}

int Bike::UnknownVirtualSlot5(int arg)
{
    if (arg)
        return BikeAbsRef(savedRoll) > 0.2618f;
    return BikeAbsRef(savedRoll) > 0.2618f || BikeAbsRef(savedPitch) > 1.05f;
}

void Bike::UnknownVirtualSlot49(float arg)
{
    Vehicle::UnknownVirtualSlot49(arg);
    UnknownVirtualSlot102(arg);
}

void Bike::UnknownVirtualSlot44()
{
    Vehicle::UnknownVirtualSlot44();
    riderCharacter->c_0x1a0->Method_0x004444e0();
}

void Bike::UnknownVirtualSlot71(int arg)
{
    if (crashState == 0) {
        if (airborne) {
            if (arg == 0 && landingLatched == 0) {
                wobbleTime = arg;
                Vehicle::UnknownVirtualSlot71(arg);
                return;
            }
        } else if (arg) {
            wobbleStage = 1;
            wobbleSign = (savedRoll < 0.0f) ? -1.0f : 1.0f;
            wobbleOffset = angularVelocity.z;
        }
    }
    Vehicle::UnknownVirtualSlot71(arg);
}

static inline float BikeAbs(float x)
{
    if (x < 0.0f)
        x = -x;
    return x;
}

float Bike::UnknownVirtualSlot57()
{
    if (crashState == 0 && prevSpeed < field_0x724)
        return savedRoll;
    float t = steerState->steerAngle;
    float a = (t < 0.0f) ? -t : t;
    float r = leanAngle;
    if (a < 0.52359878f)
        r = r - (leanAngle - savedRoll) * ((0.52359878f - a) * 1.9098593f);
    return r;
}

void Bike::UnknownVirtualSlot56(Vec3* a, int b, Vec3* c)
{
    Vehicle::UnknownVirtualSlot56(a, b, c);
    if (b) {
        rearWheel->w_0x2a4 = leanCos;
        frontWheel->w_0x2a4 = leanCos;
    }
}

float Bike::UnknownVirtualSlot59()
{
    if (poseState == 10)
        return steerState->field_0x08;
    float r = steerState->field_0x08 -
              FastSqrt(prevSpeed / field_0x438 * (steerState->field_0x08 * steerState->field_0x08));
    r = (0.0f > r) ? 0.0f : r;
    float m = steerState->field_0x08;
    return (m < r) ? m : r;
}

void Bike::UnknownVirtualSlot67()
{
    int i;
    for (i = 0; i < collisionPointCount; i++) {
        BikeElem* e = ((BikeElem**)collisionPoints)[i];
        if (e->h_0x4 != 0)
            e->h_0xa4 = 0;
    }
    crashState = 0;
    crashTimer = 0.0f;
    prevCrashState = 0;
    linkedVehicle = 0;
    field_0x61c = g_BikeVec3_005778a8;
}

float Bike::UnknownVirtualSlot73(const Vec3* a, const Vec3* b)
{
    float r = Vehicle::UnknownVirtualSlot73(a, b);
    float d = a->y * b->y + a->x * b->x;
    d += a->z * b->z;
    return r * d;
}

void Bike::UnknownVirtualSlot90(int* flag, float arg)
{
    if (UnknownVirtualSlot100(stepTime, 0, 0)) {
        *flag = 1;
        stepRemainder = 0.0f;
    } else if (*flag == 1 && stepRemainder <= 0.0001f) {
        UnknownVirtualSlot89(arg);
    }
}

Vec3 Bike::UnknownVirtualSlot16(const Vec3* v)
{
    Vec3 r;
    if (crashState == 0) {
        r.x = invInertia.x * v->x;
        r.y = 0.0f;
        r.z = 0.0f;
    } else {
        r.x = shapeInvInertia.x * v->x;
        r.y = shapeInvInertia.y * v->y;
        r.z = shapeInvInertia.z * v->z;
    }
    return r;
}

void Bike::UnknownVirtualSlot1(float arg)
{
    Vehicle::UnknownVirtualSlot1(arg);
    BikeA644* p = poseSmoother;
    if (p) {
        float v = field_0x704;
        p->smoothedValue = 0;
        if (v != FLT_MAX) {
            p->smoothTime = v;
            p->smoothRatio = 1.0f;
        }
        p->maxRise = 1.0f;
        p->maxFall = -1.0f;
    }
    field_0x6fc = 0;
    field_0x520 = 1;
    field_0x634 = 0;
    field_0x638 = 0;
    field_0x628 = 0;
    field_0x62c = 0;
    field_0x630 = 0;
    poseIndex = 1;
    poseParam = 0.5f;
    poseBlend = 0;
    poseLeanBlend = 0;
    field_0x664 = 0;
    field_0x668 = 0;
    poseState = 5;
    linkedVehicle = 0;
    wobbleSign = 0;
    wobbleOffset = 0;
    wobbleStage = 0;
    field_0x700 = 0;
}

// Retail 0x00409a10 is ~Bike's body (reached through the vbase-adjusted deleting destructor
// 0x0040ca40); ~Vehicle (0x00526380) runs after it.  0x0043a5e0 is CollisionPoint.cpp's
// RemoveCollisionPoint (collision agent's file): removes a point from a pointer list.
// owner: bracket only (not xref-anchored)
void BikeRemoveCollisionPoint(SoultreeContact** points, void* point, int* count);   // 0x0043a5e0

Bike::~Bike()
{
    if (steerAxis)
        delete steerAxis;
    if (poseSmoother)
        delete poseSmoother;
    if (frontWheel)
        BikeRemoveCollisionPoint(collisionPoints, frontWheel->contactPoint_0x0b8, &collisionPointCount);
    if (field_0x60c)
        delete field_0x60c;
    if (rearWheel)
        BikeRemoveCollisionPoint(collisionPoints, rearWheel->contactPoint_0x0b8, &collisionPointCount);
}

// owner: Bike.cpp (neighbour of slot 1 0x00407630; Bike's __FILE__ xrefs start at 0x00407c3a)
// 0x00407700, ret 8 = flags + the hidden most-derived flag.  Retail passes the flags to
// Vehicle's ctor 0x005257a0 as (flags, 0) and runs GameObject(1) first when most-derived.
Bike::Bike(int flags) : GameObject(1), Vehicle(flags)
{
    frontWheel = 0;
    rearWheel = 0;
    field_0x60c = 0;
    steerAxis = 0;
    poseSmoother = 0;
    Bike::UnknownVirtualSlot1(165.0f);
    field_0x5bc = 0;
    field_0x5c0 = 1;
    field_0x604 = 0;
    riderCharacter = 0;
    strcpy(riderName, g_BikeString_00577738);
    field_0x704 = 0.55f;
    field_0x600 = 0;
    field_0x5f8 = 0;
    field_0x5fc = 0;
    field_0x724 = 32.0f;
    field_0x728 = 0;
    field_0x72c = 0;
    field_0x730 = 0;
    field_0x61c = g_BikeVec3_005778a8;
    field_0x700 = 0;
}

int Bike::UnknownVirtualSlot33(const Vec3* a, const Vec3* b, const Vec3* c,
                               const Vec3* d, int e, float f)
{
    Matrix4 tmp;
    RunTickers();
    int r = Vehicle::UnknownVirtualSlot33(a, b, c, d, e, f);
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        modelNode->GetMatrixIn(0, &tmp);
        riderCharacter->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
    field_0x604->a_0x38->Method_0x00435fe0();
    return r;
}

int Bike::UnknownVirtualSlot62(float arg)
{
    if (field_0x700)
        return 2;
    if (anyWheelInContact && crashState == 0) {
        if (allWheelsInContact || turnRate != 0.0f || arg != 0.0f)
            return 1;
        if (rearWheel->inContact) {
            if (frontWheel->w_0x150 > -5.2f)
                return 3;
        } else if (frontWheel->inContact) {
            if (rearWheel->w_0x150 > -5.2f)
                return 1;
        }
    }
    return 0;
}

int Bike::UnknownVirtualSlot100(float a, int b, float c)
{
    float thr = 0.766f;
    if (((BikeA1F4*)terrain)->i_0xbe8 != 1)
        thr = 0.342f;
    crashState = UnknownVirtualSlot99(a, thr, b, c);
    if (crashState) {
        field_0x574 = Vec3(savedForward.x, 0.0f, savedForward.z);
        field_0x433 = 0;
        int neg = savedUp.y < 0.0f;
        field_0x45c = savedYaw;
        field_0x464 = neg;
        field_0x138 = 1;
        UnknownVirtualSlot69();
        UnknownVirtualSlot41();
        field_0x431 = 0;
    }
    return crashState;
}

static inline Vec3 BikeNormalized(const Vec3& v)
{
    float lenSq = v.y * v.y + v.x * v.x;
    lenSq += v.z * v.z;
    if (lenSq == 1.0f)
        return v;
    float s = FastInvSqrt(lenSq);
    return v * s;
}

Vec3* Bike::UnknownVirtualSlot55(Vec3* out, Vec3* pos)
{
    *pos = modelNode->WorldToLocalPoint(rearWheel->wheelPosition);
    const Vec3* b = &rearWheel->wheelPosition;
    const Vec3* a = &frontWheel->wheelPosition;
    Vec3 d;
    d.x = a->x - b->x;
    d.y = a->y - b->y;
    d.z = a->z - b->z;
    *out = BikeNormalized(d);
    return out;
}

Vec3* Bike::UnknownVirtualSlot54(Vec3* out)
{
    if (wheelsInContact == 0) {
        *out = rearWheel->groundNormal;
        return out;
    }
    if (frontWheel->inContact != 0) {
        if (rearWheel->inContact != 0) {
            Vec3 s(frontWheel->groundNormal.x + rearWheel->groundNormal.x, frontWheel->groundNormal.y + rearWheel->groundNormal.y, frontWheel->groundNormal.z + rearWheel->groundNormal.z);
            Vec3 mid(s.x * 0.5f, s.y * 0.5f, s.z * 0.5f);
            *out = BikeNormalized(mid);
            return out;
        }
        *out = frontWheel->groundNormal;
        return out;
    }
    *out = rearWheel->groundNormal;
    return out;
}

int Bike::UnknownVirtualSlot39(float arg)
{
    int r = Vehicle::UnknownVirtualSlot39(arg);
    if (r)
        return r;
    if (crashState) {
        if (crashTimer != 0.0f && field_0x604->a_0xac) {
            crashTimer -= g_Bike_0056e26c->g_0x2f0;
            if (crashTimer > field_0x710) {
                field_0x604->a_0xac = 0;
            } else if (crashTimer <= 0.0f) {
                if (crashTimer == 0.0f)
                    crashTimer = -0.1f;
                if (rearWheel->groundNormal.y > 0.866 || prevSpeed < 1.0f) {
                    UnknownVirtualSlot67();
                    return 1;
                }
                if (crashTimer <= -2.0f) {
                    UnknownVirtualSlot67();
                    return 1;
                }
            }
        }
        if (crashState == 0) {
            UnknownVirtualSlot67();
            return 1;
        }
    }
    return 0;
}

void Bike::UnknownVirtualSlot65(float arg)
{
    if (smoothedVerticalAccel <= 5.0f || arg == 0.0f)
        return;
    BikeWheel* rear = rearWheel;
    if (rear->w_0x268 == 0 && frontWheel->w_0x268 == 0)
        return;
    arg = (controlInput.y - prevControlInput.y) / arg;
    if (controlInput.y <= 0.4f || arg <= 11.0f)
        return;
    float t = smoothedVerticalAccel * 0.6f;
    t = BikeMin(t, 100.0f);
    t = t * arg * 5.0f;
    t = BikeMin(t, 10000.0f);
    if (rear->w_0x268 && rear->w_0x148 > 30.0f) {
        rear->w_0x2ac->q_0x94 = t;
        return;
    }
    BikeWheel* front = frontWheel;
    if (front->w_0x268 && front->w_0x148 > 30.0f)
        front->w_0x2b0->q_0x94 = t;
}

void Bike::UnknownVirtualSlot8()
{
    localCenterOfMass = poseNode->WorldToLocalPoint(centerOfMass);
    if (crashState == 0 && steerAxis) {
        float steer = (steerAxis->steerValue - 0.5f) * 4.0f;
        localCenterOfMass.x = 0.0f;
        localCenterOfMass.z += steer;
        centerOfMass = poseNode->LocalToWorldPoint(localCenterOfMass);
    }
    rotationPivot = localCenterOfMass;
    field_0x700 = (wheelsInContact > 0 && frontWheel->w_0x150 < -2.5f);
}

Vec3 Bike::UnknownVirtualSlot76(const Vec3* a, const Vec3* b)
{
    Vec3 v = Vehicle::UnknownVirtualSlot76(a, b);
    if (linearSpeed < 10.0f && savedPitch > 0.8727f && frontWheel->inContact) {
        v = g_BikeVec3_005778a8;
        if (angularVelocity.x < 0.0f)
            angularVelocity.x = 0;
    } else if (angularVelocity.x < -1.0f) {
        v *= 0.2f;
    } else {
        float lean = savedPitch;
        if (lean < 0.0f)
            lean = -lean;
        lean = BikeMin(lean, 1.0f);
        float k = -(controlInput.y + 0.125f);
        k = (0.0f > k) ? 0.0f : k;
        // Retail multiplies v.x by the freshly computed scale (fld st(0); fmul [v.x]) and
        // v.y/v.z by the stored copy; only the assignment-expression form reproduces that
        // (a named `s` first, then `v *= s` or per-component scaling, loads v.x first: 389/395).
        float s;
        v.x *= (s = (1.4f - lean) * k * 1.6f);
        v.y *= s;
        v.z *= s;
    }
    return v;
}

void Bike::UnknownVirtualSlot29(int)
{
    if (field_0x6fc == 0 && field_0x430 == 0) {
        savedForward = bodyForward;
        savedUp = bodyUp;
        savedYaw = bodyYaw;
        savedPitch = bodyPitch;
        savedRoll = bodyRoll;
        savedSinRoll = bodySinRoll;
        savedCosRoll = bodyCosRoll;
        savedCosPitch = bodyCosPitch;
        savedSinPitch = bodySinPitch;
    } else {
        poseNode->GetAxesIn(0, &savedForward, &savedUp);
        OrientationAnglesFromVectors(savedForward, savedUp, &savedYaw, &savedPitch, &savedRoll,
                            &savedSinRoll, &savedCosRoll, &savedCosPitch, &savedSinPitch);
    }
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        modelNode->GetMatrixIn(0, &tmp);
        riderCharacter->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
}

void Bike::UnknownVirtualSlot41()
{
    if (field_0x6fc == 0 && field_0x430 == 0)
        return;
    poseNode->GetAxesIn(0, &savedForward, &savedUp);
    modelNode->SetAxesIn(0, &savedForward, &savedUp, 1, 0);
    D3DIMSoultreeCharacter::ApplyRestPose();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
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
    field_0x6fc = 0;
    if (controlInput.x <= 0.0f) {
        if (controlInput.x < -0.9f)
            field_0x520 = 2;
        else if (controlInput.x > -0.032f)
            field_0x520 = 1;
        else
            field_0x520 = 3;
    } else {
        if (controlInput.x > 0.9f)
            field_0x520 = 4;
        else if (controlInput.x < 0.032f)
            field_0x520 = 1;
        else
            field_0x520 = 5;
    }
    for (int i = 0; i < wheelCount; i++)
        wheelList[i]->UpdateAttachment();
}

void Bike::UnknownVirtualSlot102(float)
{
    if (crashState) {
        float t = -(steerState->steerAngle / steerState->field_0x08);
        t = (t + 1.0f) * 0.25f;
        t += 0.25f;
        t = (t > 0.25f) ? BikeMin(t, 0.75f) : 0.25f;
        poseParam = t;
        D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[1], t);
        return;
    }
    if (field_0x430)
        return;
    int cnt = poseState;
    int step = field_0x6fc;
    int idx = poseIndex;
    int next;
    if (step) {
        next = idx + 1;
        if (next > 13)
            next = 13;
    } else if (idx > 2) {
        if (!(cnt == 10 && idx == 5)) {
            float sp = smoothedVerticalAccel;
            sp = (sp < 32.2f) ? ((sp > -32.2f) ? sp : -32.2f) : 32.2f;
            poseLeanBlend = (sp * 0.0310559f + 1.0f) * 0.5f;
        }
        next = idx + 2;
        if (idx == 10)
            next = idx;
    } else {
        next = idx + 1;
        cnt += 2;
    }
    float w = 1.0f - poseBlend;
    riderCharacter->Method_0x004a8c50(riderPoseHandles[idx], riderPoseHandles[next], poseParam, w);
    D3DIMSoultreeCharacter::Method_0x004a8c50(bikePoseHandles[idx], bikePoseHandles[next], poseParam, w);
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        modelNode->GetMatrixIn(0, &tmp);
        riderCharacter->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
}

// Member-form cross product: VC6 keeps source multiplicand order here, where the free
// function form canonicalises it (same observation as Vehicle.cpp's VehV3).
static inline Vec3 BikeCrossMixed(const Vec3& a, const Vec3& n)
{
    Vec3 c;
    c.x = a.z * n.y - a.y * n.z;
    c.y = a.x * n.z - n.x * a.z;
    c.z = n.x * a.y - a.x * n.y;
    return c;
}

void Bike::UnknownVirtualSlot72(Vec3* out, VehicleWheel*)
{
    if (crashState)
        return;
    BikeWheel* a = frontWheel;
    if (!(a->inContact || (rearWheel->inContact && a->w_0x150 > -0.2f) || field_0x700)) {
        *out = rearWheel->groundNormal;
    } else {
        *out = a->groundNormal;
        BikeWheel* b = rearWheel;
        float weight = frontWheel->contactLoad;
        if (b->w_0x150 > -0.2f) {
            out->x += b->groundNormal.x;
            out->y += b->groundNormal.y;
            out->z += b->groundNormal.z;
            weight += rearWheel->contactLoad;
            float lenSq = out->y * out->y + out->x * out->x;
            lenSq += out->z * out->z;
            if (lenSq == 0.0f) {
                *out = g_BikeVec3_005778a8;
            } else {
                float s = FastInvSqrt(lenSq);
                out->x *= s;
                out->y *= s;
                out->z *= s;
            }
            weight *= 0.5f;
            const Vec3& n = frontWheel->w_0x230;
            Vec3 c = CrossProduct(n, *out);
            Vec3 perp = BikeNormalized(c);
            float scale;
            if (field_0x700) {
                float k = savedCosPitch * 0.35f;
                k = (0.001f > k) ? 0.001f : k;
                scale = 1.0f / k;
            } else {
                scale = 1.0f;
            }
            float w = (weight < 1.0f) ? weight : 1.0f;
            turnAngle = UnknownVirtualSlot74(&frontWheel->w_0x23c, &perp, w, scale);
            turnRate = turnAngle * invStepTime;
            return;
        }
    }
    turnAngle = 0.0f;
    turnRate = 0.0f;
}

float Bike::UnknownVirtualSlot61(float threshold)
{
    float result = 0.0f;
    if (allWheelsInContact) {
        BikeWheel* rear = rearWheel;
        if (rear->w_0x148 > 7.0f && rear->w_0x288 >= 0.0001f && frontWheel->w_0x288 < 0.5f) {
            Vec3 c;
            c.x = rear->w_0x108.z * rear->w_0x230.y - rear->w_0x108.y * rear->w_0x230.z;
            c.y = rear->w_0x108.x * rear->w_0x230.z - rear->w_0x108.z * rear->w_0x230.x;
            c.z = rear->w_0x108.y * rear->w_0x230.x - rear->w_0x108.x * rear->w_0x230.y;
            scratchVector = c;
            float lenSq = scratchVector.y * scratchVector.y + scratchVector.x * scratchVector.x + scratchVector.z * scratchVector.z;
            if (lenSq != 1.0f) {
                float len = (float)sqrt(lenSq);
                if (len < 0.4795f) {
                    len = (float)asin(len);
                    if (_finite(len)) {
                        float sign = (scratchVector.y < 0.0f) ? -1.0f : 1.0f;
                        float cap = stepTime * 0.8f;
                        float m = (len < cap) ? len : cap;
                        result = sign * m;
                        float mag = (result < 0.0f) ? -result : result;
                        if (threshold > mag)
                            result = 0.0f;
                    }
                }
            }
        }
    }
    bool decay = false;
    if (allWheelsInContact) {
        BikeWheel* rear = rearWheel;
        if (rear->w_0x2b8 < 0.9f && rear->w_0x280 > 0.0f) {
            float t = wheelBase * 0.5f;
            float u = (t + t * rear->w_0x2bc) * invInertia.y * stepTime * rear->w_0x2bc * rear->w_0x280;
            float a = leanAngle;
            if (a < 0.0f)
                a = -a;
            if (a > 0.005f) {
                u *= 32.0f;
                u = BikeMin(u, 1.57f);
                float b = a * 5.0f;
                b = BikeMin(b, 1.5f);
                u = u * stepTime * b;
                field_0x628 = (leanAngle < 0.0f) ? -u : u;
                field_0x62c = fixedStepTime * -1.333f * field_0x628;
                field_0x630 = (int)(0.75f / fixedStepTime - 0.5f);
            } else {
                field_0x628 = 0.0f;
            }
        } else if (field_0x628 != 0.0f) {
            decay = true;
        }
    } else if (anyWheelInContact == 0 || field_0x628 == 0.0f) {
        field_0x628 = 0.0f;
    } else {
        decay = true;
    }
    if (decay) {
        float old = field_0x628;
        field_0x628 = field_0x628 + field_0x62c;
        int count = --field_0x630;
        if (old < 0.0f) {
            if (field_0x628 > 0.0f || count < 0)
                field_0x628 = 0.0f;
        } else {
            if (field_0x628 < 0.0f || count < 0)
                field_0x628 = 0.0f;
        }
    }
    if (field_0x628 != 0.0f)
        return field_0x628;
    return result;
}

// ---------------------------------------------------------------------------
// Second pass: larger dynamics functions (all provisional, tier 3 semantics).
// ---------------------------------------------------------------------------

// Length of a vector from its squared length; FastInvSqrt is 1/sqrt.
static inline float BikeLength(float lenSq)
{
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

static inline float BikeMagnitude(const Vec3& v)
{
    float lenSq = BikeDot(&v, &v);
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

static inline Vec3 BikeDirection(const Vec3& v)
{
    float lenSq = BikeDot(&v, &v);
    if (lenSq == 0.0f)
        return g_BikeVec3_005778a8;
    float s = FastInvSqrt(lenSq);
    return Vec3(s * v.x, s * v.y, s * v.z);
}

// Slot 97: builds the rider ragdoll ("rider.col") and attaches the 15 body
// bones by name (Pelvis, UprTorso, Head, arms, legs, feet) with local offsets.
// BikeAttachBone is __forceinline because retail expands the same sequence at each of the 15
// call sites; the original was probably a macro or an inline member (tier 3), and a plain
// call does not reproduce the bytes.
static __forceinline void BikeAttachBone(Bike* self, const char* name, float x, float y, float z)
{
    Vec3 offset;
    offset.x = x;
    offset.y = y;
    offset.z = z;
    int bone = self->field_0x604->a_0x34->r_0x1a0->FindByName(name);
    self->field_0x604->Method_0x00532900(offset, bone);
}

void Bike::UnknownVirtualSlot97()
{
    field_0x604 = new(__FILE__, 2023) BikeA604(1);
    if (field_0x604->Method_0x00530190(GameObject::field_0x18, riderCharacter, "rider.col")) {
        GameObject::Method_0x00469190(field_0x604, -1);
        field_0x604->a_0x38->n_0xe4 = ((BikeA1F4*)terrain);
        field_0x604->a_0x38->n_0x64 = 0x65;
        BikeAttachBone(this, "Pelvis", 0.0f, 0.0f, -0.35f);
        BikeAttachBone(this, "UprTorso", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "Head", 0.0f, 0.75f, 0.0f);
        BikeAttachBone(this, "UprArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "UprArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "LwrArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "HandL", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(this, "LwrArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "HandR", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(this, "LwrLegL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "FootL", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(this, "FootL", 0.0f, -1.0f, 0.0f);
        BikeAttachBone(this, "LwrLegR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "FootR", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(this, "FootR", 0.0f, -1.0f, 0.0f);
        field_0x604->a_0x11c = field_0x124;
    }
}

// Slot 99: crash test.  Classifies the current impact/attitude and, if the
// rider is thrown, records the crash direction (field_0x448: 1..5) and the
// reason code (field_0x460) and returns 1.
int Bike::UnknownVirtualSlot99(float a, float b, int c, float d)
{
    if (crashState != 0)
        return 0;
    BikeA604* rider = field_0x604;
    if (rider->a_0xd8) {
        scratchVector = rider->a_0x34->r_0x1a0->d_0x140->WorldToLocalPoint(rider->a_0xdc);
        if (scratchVector.y > 1.0f) {
            if (scratchVector.x < -1.0f) {
                crashDirection = 5;
                crashReason = 11;
                return 1;
            }
            if (scratchVector.x > 1.0f) {
                crashDirection = 4;
                crashReason = 11;
                return 1;
            }
            if (scratchVector.z < -0.9f)
                crashDirection = 1;
            else
                crashDirection = 3;
            crashReason = 11;
            return 1;
        }
    }
    // front wheel planted, rear wheel not: nose-over crash
    if ((airborne || field_0x6fc == 0) && frontWheel->inContact && !rearWheel->inContact
        && field_0x4b4 < b && angularVelocity.x > 0.0f) {
        crashDirection = 1;
        crashReason = 7;
        return 1;
    }
    float d1 = BikeDot(&g_BikeVec3_005778c8, &savedForward);
    if (d1 < 0.0f)
        d1 = -d1;
    float lieLimit = sideLieThreshold;
    if (d1 > lieLimit) {
        // bike is lying on its side
        if (savedForward.y > 0.0f) {
            crashDirection = 3;
            crashReason = 8;
            return 1;
        }
        crashDirection = 1;
        crashReason = 8;
        return 1;
    }
    float roll = savedRoll;
    if (roll < 0.0f)
        roll = -roll;
    if (roll > 1.91986f) {
        // rolled past ~110 degrees
        if (savedRoll > 0.0f) {
            crashDirection = 5;
            crashReason = 6;
            return 1;
        }
        crashDirection = 4;
        crashReason = 6;
        return 1;
    }
    if (rearWheel->w_0x288 > 0.707f && leanCos < 0.707f) {
        if (savedRoll < 0.0f) {
            crashDirection = 4;
            crashReason = 13;
            return 1;
        }
        crashDirection = 5;
        crashReason = 13;
        return 1;
    }
    if (a < fixedStepTime) {
        float m = fixedStepTime * 0.8f;
        if (m > a)
            a = m;
    }
    if (UnknownVirtualSlot51()) {
        linkedVehicle = 0;
        return 0;
    }
    if (UnknownVirtualSlot70(a)) {
        crashDirection = 1;
        crashReason = 4;
        return 1;
    }
    if (c && d * invMass > a * field_0x44c) {
        if (angularVelocity.z > 0.0f) {
            crashDirection = 4;
            crashReason = 10;
            return 1;
        }
        crashDirection = 5;
        crashReason = 10;
        return 1;
    }
    if (linkedVehicle) {
        // collision with another vehicle: compare headings
        Vehicle* other = linkedVehicle;
        float speed = linearSpeed;
        float inv = 1.0f / speed;
        Vec3 u1(inv * velocity.x, inv * velocity.y, inv * velocity.z);
        float otherSpeed = other->linearSpeed;
        float inv2 = 1.0f / otherSpeed;
        Vec3 u2(inv2 * other->velocity.x, inv2 * other->velocity.y, inv2 * other->velocity.z);
        float dot = BikeDot(&u1, &u2);
        if (dot > 0.707f) {
            crashDirection = 1;
            crashReason = 2;
            return 1;
        }
        if (dot < -0.707f) {
            crashDirection = 3;
            crashReason = 2;
            return 1;
        }
        if (angularVelocity.z > 0.0f) {
            crashDirection = 4;
            crashReason = 2;
            return 1;
        }
        crashDirection = 5;
        crashReason = 2;
        return 1;
    }
    return 0;
}

// Slot 7: distributes an applied force/direction over the tyre contacts.
// Mode field_0x1d0: weights the active contact elements by distance.
// Otherwise splits the force between front and rear wheel.
void Bike::UnknownVirtualSlot7(const Vec3* dir)
{
    contactTotal = touchingPointCount + wheelsInContact;
    if (pointsTouching) {
        float dist[124];
        float sum = 0.0f;
        int active = 0;
        int last = 0;
        int i;
        for (i = 0; i < collisionPointCount; i++) {
            BikeElem* e = ((BikeElem**)collisionPoints)[i];
            if (e->h_0xa4) {
                Vec3 dv(centerOfMass.x - e->h_0x14.x, centerOfMass.y - e->h_0x14.y, centerOfMass.z - e->h_0x14.z);
                float len = BikeLength(BikeDot(&dv, &dv));
                dist[i] = len;
                sum += len;
                active++;
                last = i;
            } else {
                e->h_0xa0 = 0.0f;
                e->h_0x44 = g_BikeVec3_005778a8;
            }
        }
        if (active == 1) {
            ((BikeElem**)collisionPoints)[last]->h_0xa0 = 1.0f;
            ((BikeElem**)collisionPoints)[last]->h_0x44 = *dir;
            return;
        }
        for (i = 0; active > 0; i++) {
            BikeElem* e = ((BikeElem**)collisionPoints)[i];
            if (e->h_0xa4) {
                float w = 1.0f - dist[i] / sum;
                e->h_0xa0 = w;
                e->h_0x44 = Vec3(w * dir->x, w * dir->y, w * dir->z);
                active--;
            }
        }
        return;
    }
    BikeA640* p = steerAxis;
    if (field_0x430) {
        float f = stepTime;
        if (!(f < p->l_0x4))
            f = p->l_0x4;
        float r = f / p->l_0x4;
        p->l_0x8 = r;
        p->steerValue = (0.5f - p->steerValue) * r + p->steerValue;
        rearWheel->loadWeight = rearWheel->w_0x294;
        rearWheel->appliedShare = Vec3(rearWheel->loadWeight * dir->x, rearWheel->loadWeight * dir->y, rearWheel->loadWeight * dir->z);
        frontWheel->loadWeight = frontWheel->w_0x294;
        frontWheel->appliedShare = Vec3(frontWheel->loadWeight * dir->x, frontWheel->loadWeight * dir->y, frontWheel->loadWeight * dir->z);
        return;
    }
    float target = controlInput.y * 0.25f + 0.5f;
    int mode = wheelsInContact;
    {
        float f = stepTime;
        if (!(f < p->l_0x4))
            f = p->l_0x4;
        float r = f / p->l_0x4;
        p->l_0x8 = r;
        p->steerValue = (target - p->steerValue) * r + p->steerValue;
    }
    if (mode > 1) {
        // weight rear/front by distance from the centre position
        Vec3 df(centerOfMass.x - frontWheel->wheelPosition.x, centerOfMass.y - frontWheel->wheelPosition.y, centerOfMass.z - frontWheel->wheelPosition.z);
        float dfSq = BikeDot(&df, &df);
        float lf = BikeLength(dfSq);
        Vec3 dr(centerOfMass.x - rearWheel->wheelPosition.x, centerOfMass.y - rearWheel->wheelPosition.y, centerOfMass.z - rearWheel->wheelPosition.z);
        float drSq = BikeDot(&dr, &dr);
        float lr = BikeLength(drSq);
        rearWheel->loadWeight = drSq / (lr + lf);
        rearWheel->appliedShare = Vec3(rearWheel->loadWeight * dir->x, rearWheel->loadWeight * dir->y, rearWheel->loadWeight * dir->z);
        frontWheel->loadWeight = 1.0f - rearWheel->loadWeight;
        frontWheel->appliedShare = Vec3(frontWheel->loadWeight * dir->x, frontWheel->loadWeight * dir->y, frontWheel->loadWeight * dir->z);
    } else if (mode == 1) {
        if (frontWheel->inContact) {
            frontWheel->loadWeight = 1.0f;
            frontWheel->appliedShare = *dir;
            rearWheel->loadWeight = 0.0f;
            rearWheel->appliedShare = g_BikeVec3_005778a8;
        } else {
            frontWheel->loadWeight = 0.0f;
            frontWheel->appliedShare = g_BikeVec3_005778a8;
            rearWheel->loadWeight = 1.0f;
            rearWheel->appliedShare = *dir;
        }
    } else {
        rearWheel->loadWeight = 0.0f;
        rearWheel->appliedShare = g_BikeVec3_005778a8;
        frontWheel->loadWeight = 0.0f;
        frontWheel->appliedShare = g_BikeVec3_005778a8;
    }
}

// Slot 91: per-step drive/lateral force.  Builds a force from throttle and
// wheel state, turns the velocity toward the heading, applies the force
// through the transform, then decays the accumulated lean terms.
void Bike::UnknownVirtualSlot91()
{
    float dt = stepTime * 0.83f;
    float f = field_0x524[1] * controlInput.y * dt;
    Vec3 force;
    if (field_0x430)
        force = Vec3(f * 0.5f, 0.0f, 0.0f);
    else
        force = Vec3(f, controlInput.x * stepTime * -0.2f, 0.0f);
    if (field_0x6fc && rearWheel->w_0x150 < -2.0f) {
        if (field_0x5c0 && rearWheel->w_0x29c > 0.0f)
            force.x += dt * rearWheel->w_0x29c;
        if (field_0x5bc && field_0x478 && engineState->field_0x00 > 0.0f && !field_0x479)
            force.x -= dt * engineState->field_0x00;
        float speed = linearSpeed;
        float inv = 1.0f / speed;
        Vec3* heading = &scratchVector;
        *heading = Vec3(inv * velocity.x, inv * velocity.y, inv * velocity.z);
        float d = BikeDot(&savedForward, heading);
        if (d > 0.55f && d < 0.984f) {
            // steer the velocity toward the lean plane
            scratchVector2 = Vec3(scratchVector.z, 0.0f, -heading->x);
            float s = (poseParam < 0.5f) ? 1.0f : -1.0f;
            float k = s * ((1.0f - d) * 3.41296911f * linearSpeed * 0.075f) * stepTime;
            velocity += scratchVector2 * k;
            velocity *= linearSpeed / (k + linearSpeed);
            float lenSq = BikeDot(&velocity, &velocity);
            linearSpeed = (lenSq == 1.0f) ? 1.0f : (float)sqrt(lenSq);
        }
    }
    float mag = BikeMagnitude(force);
    if (_finite(mag) && ((mag < 0.0f) ? -mag : mag) >= 0.0001f)
        modelNode->RotateAboutPoint(rotationPivot, BikeDirection(force), mag);
    angularVelocity.x = angularVelocity.x * 0.9f;
    angularVelocity.y = angularVelocity.y * 0.9f;
    angularVelocity.z = angularVelocity.z * 0.9f;
    // wobble: ramps a short-lived oscillation on field_0xd8.z (cnt 1..2)
    float t = wobbleTime + stepTime;
    int cnt = wobbleStage;
    wobbleTime = t;
    if (cnt > 0 && cnt <= 2) {
        float sgn = (bodyRoll < 0.0f) ? -1.0f : 1.0f;
        float v = invStepTime * bodyRoll;
        if (v < 0.0f)
            v = -v;
        float cube = t * t * t * 0.357792467f;
        if (cube < v)
            v = cube;
        angularVelocity.z = sgn * v + wobbleOffset;
        wobbleOffset = wobbleOffset * 0.9f;
        if (sgn != wobbleSign) {
            wobbleStage = cnt + 1;
            if (wobbleStage > 2)
                wobbleStage = 1;
            wobbleSign = sgn;
            wobbleOffset = angularVelocity.z;
            wobbleTime = t * 0.5f;
        }
    }
}

// Vec3 whose (x, y, z) constructor is out of line in retail (0x00404e60,
// thiscall, ret 0xc); declared only, so VC6 emits the call instead of inlining.
struct BikeOolVec3 : public Vec3 {
    BikeOolVec3() {}
    BikeOolVec3(float x_, float y_, float z_);
};

static inline BikeOolVec3 BikeOolSub(const Vec3& a, const Vec3& b)
{
    return BikeOolVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline BikeOolVec3 BikeOolAdd(const Vec3& a, const Vec3& b)
{
    return BikeOolVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline BikeOolVec3 BikeOolHalf(const Vec3& v)
{
    return BikeOolVec3(v.x * 0.5f, v.y * 0.5f, v.z * 0.5f);
}

// Slot 46: focus/anchor point of the bike in local space.  Airborne or flagged
// bikes return the stored anchor (field_0x194); otherwise it is the front
// wheel position (t != 0) or a blend toward the rear wheel, relative to the
// body, mapped through the transform.
Vec3* Bike::UnknownVirtualSlot46(Vec3* out, float t)
{
    if (airborne || pointsTouching) {
        *out = rotationPivot;
        return out;
    }
    if (wheelsInContact == 2) {
        if (t != 0.0f) {
            *out = modelNode->WorldToLocalDirection(scratchVector = frontWheel->wheelPosition - position);
            return out;
        } else if (rearWheel->w_0x2b8 < 0.9f) {
            float s = rearWheel->w_0x2b8 - 0.4f;
            if (!(s > 0.0f))
                s = 0.0f;
            Vec3 d = rearWheel->wheelPosition - frontWheel->wheelPosition;
            Vec3 p = d * s + frontWheel->wheelPosition;
            *out = modelNode->WorldToLocalDirection(scratchVector = p - position);
            return out;
        } else {
            // the midpoint chain goes through the out-of-line Vec3 constructor
            Vec3 d = BikeOolSub(rearWheel->wheelPosition, frontWheel->wheelPosition);
            Vec3 half = BikeOolHalf(d);
            Vec3 mid = BikeOolAdd(half, frontWheel->wheelPosition);
            Vec3 rel = BikeOolSub(mid, position);
            *out = modelNode->WorldToLocalDirection(scratchVector = rel);
            return out;
        }
    } else {
        Vec3 d = BikeOolSub(rearWheel->wheelPosition, frontWheel->wheelPosition);
        Vec3 half = BikeOolHalf(d);
        Vec3 mid, rel;
        Vec3AddCall(&mid, &frontWheel->wheelPosition, &half);
        Vec3SubtractCall(&rel, &mid, &position);
        *out = modelNode->WorldToLocalDirection(scratchVector = rel);
        return out;
    }
}

// Moves a stored value toward zero by 1.5 * delta without crossing zero.
static inline void BikeCancelToward(float* value, float delta)
{
    if (delta > 0.0f) {
        if (*value < 0.0f) {
            float n = delta * 1.5f + *value;
            if (n > 0.0f)
                n = 0.0f;
            *value = n;
        }
    } else if (*value > 0.0f) {
        float n = delta * 1.5f + *value;
        if (!(n >= 0.0f))
            n = 0.0f;
        *value = n;
    }
}

// Slot 92: steering-torque limiter.  Derives a target steering angle from
// speed/lean terms, clamps it (rider lean, field_0x580), computes the residual
// vs the current angle (slot 57) and applies a corrective force through the
// transform, then bleeds off the accumulated lean impulses.
void Bike::UnknownVirtualSlot92(const Vec3* worldDir, const Vec3* point)
{
    float target = (steerState->steerAngle * 0.05f + turnRate * 0.2f) * field_0x4b4;
    targetLeanAngle = target;
    if (target < 0.0f)
        target = -target;
    if (field_0x628 != 0.0f) {
        float lim = ((field_0x628 < 0.0f) ? -1.0f : 1.0f) * 0.087f;
        float absLim = lim;
        if (absLim < 0.0f)
            absLim = -absLim;
        if (target < absLim)
            targetLeanAngle = lim;
    }
    if (target > maxLeanAngle) {
        if (targetLeanAngle < 0.0f)
            targetLeanAngle = -maxLeanAngle;
        else
            targetLeanAngle = maxLeanAngle;
    }
    leanError = UnknownVirtualSlot57() - targetLeanAngle;
    if (_finite(leanError) && ((leanError < 0.0f) ? -leanError : leanError) >= 0.0001f) {
        int errSign = (leanError < 0.0f) ? -1 : 1;
        int steerSign = (bodyRoll < 0.0f) ? -1 : 1;
        float lim = maxLeanRate;
        if (errSign == steerSign)
            lim = lim * 0.8f;
        float mag = (leanError < 0.0f) ? -leanError : leanError;
        mag = mag * invStepTime;
        if (mag > lim)
            leanError = ((leanError < 0.0f) ? -1.0f : 1.0f) * stepTime * lim;
        if (anyWheelInContact) {
            Vec3 w = modelNode->WorldToLocalDirection(*worldDir);
            float az = (w.z < 0.0f) ? -w.z : w.z;
            float ay = (w.y < 0.0f) ? -w.y : w.y;
            float ax = (w.x < 0.0f) ? -w.x : w.x;
            if (az > 0.001f || ay > 0.001f || ax > 0.001f) {
                modelNode->RotateAboutPoint(w, *point, leanError);
                float k = leanError;
                float tx = k * w.x;
                float ty = k * w.y;
                float tz = k * w.z;
                scratchVector2 = Vec3(tx * invStepTime, ty * invStepTime, tz * invStepTime);
                BikeCancelToward(&angularVelocity.x, scratchVector2.x);
                BikeCancelToward(&angularVelocity.y, scratchVector2.y);
                BikeCancelToward(&angularVelocity.z, scratchVector2.z);
            }
        }
    }
}

// ---- slot 38: collision-response dispatch (tier 3 names) ----
// Bike's version of Vehicle slot 38 (0x005268d0).  `b` is the collision kind, `c` the event
// record whose +0x60 member is the other party.  The kind selects what the other party looks
// like (another vehicle, a static body, a terrain node) and how the contact is described; the
// tail then hands the contact to the impulse solver (slot 4 with a body, slot 3 without) and
// optionally notifies via slot 100.  Kind 0x67 and unknown kinds return immediately.
struct BikeContactSet {
    char pad_0x00[0xA0];
    Vec3 field_0xa0;            // contact point
    Vec3 field_0xac;            // contact normal
};
struct BikeCollisionEvent {
    char pad_0x00[0x60];
    Vehicle* field_0x60;        // other party
};
// Polymorphic body reached through the other party's +0xc4 (kind 0x3ea); slot 43 (+0xac)
// returns a velocity vector.  The placeholder slots only exist to number it.
struct BikeBodyObj {
    virtual void S0(); virtual void S1(); virtual void S2(); virtual void S3();
    virtual void S4(); virtual void S5(); virtual void S6(); virtual void S7();
    virtual void S8(); virtual void S9(); virtual void S10(); virtual void S11();
    virtual void S12(); virtual void S13(); virtual void S14(); virtual void S15();
    virtual void S16(); virtual void S17(); virtual void S18(); virtual void S19();
    virtual void S20(); virtual void S21(); virtual void S22(); virtual void S23();
    virtual void S24(); virtual void S25(); virtual void S26(); virtual void S27();
    virtual void S28(); virtual void S29(); virtual void S30(); virtual void S31();
    virtual void S32(); virtual void S33(); virtual void S34(); virtual void S35();
    virtual void S36(); virtual void S37(); virtual void S38(); virtual void S39();
    virtual void S40(); virtual void S41(); virtual void S42();
    virtual Vec3* UnknownVirtualSlot43(Vec3* out);
    char pad_0x004[0x150];
    Vec3 f154;                  // current point
    char pad_0x160[0x0c];
    Vec3 f16c;
    float f17c;
    char pad_0x180[0x50];
    float f1d0;
    char pad_0x1d4[0x10];
    float f1e4;
    char pad_0x1e8[0x0c];
    float f1f8;
    char pad_0x1fc[0x2c];
    Vec3 f228;
    SoultreeObject* f234;
    Vec3 f23c;                  // previous point
};
struct BikeKindInfo {           // reached through the other party's +0x5c
    char pad_0x00[0x24];
    float field_0x24;
};

void Bike::UnknownVirtualSlot38(int a, int b, void* c)
{
    float out;                  // slot 3/4 result, forwarded to slot 100
    Vec3 s;                     // contact mask / scale (1,1,1 for static parties)
    Vec3 velA;                  // other party's linear velocity (field_0xcc)
    Vec3 velB;                  // other party's second vector (field_0xe4/0xf0)
    Vec3 leverC;                // contact point relative to the other party
    Vec3 rel;                   // contact point relative to us
    Vec3 blend;                 // kind 0x3ea: other point interpolated by its kind info
    Vec3 tmp;
    float k = 1.0f;             // angle-based response scale (kind 100)
    float l10 = 0.0f;           // other party's field_0x24
    int hasBody = 0;
    int notifyOther = 0;
    Vehicle* other = 0;
    int ctx = 0;
    Vec3* otherVel = 0;
    Vec3* velPtr = 0;

    switch (b) {
    case 100: {
        notifyOther = 1;
        other = ((BikeCollisionEvent*)c)->field_0x60;
        l10 = other->invMass;
        velA = other->worldAngularVelocity;
        velB = other->crashState ? other->shapeInvInertia : other->invInertia;
        hasBody = 1;
        otherVel = &other->velocity;
        ctx = *(int*)((char*)other + 0x3bc);
        Vec3 cp = ((BikeContactSet*)collisionObject)->field_0xa0;
        leverC.x = cp.x - other->position.x;
        leverC.y = cp.y - other->position.y;
        leverC.z = cp.z - other->position.z;
        velPtr = &other->angularVelocity;
        if (other->position.y - position.y > 1.5f)
            linkedVehicle = other;
        else if (position.y - other->position.y > 1.5f)
            ((Bike*)other)->linkedVehicle = this;
        {
            Vec3 w = ((BikeContactSet*)collisionObject)->field_0xa0;
            float ang = (float)atan2(w.x - position.x, w.z - position.z) - savedYaw;
            if (ang < 0.0f)
                ang = -ang;
            if (ang > 3.1415927f)
                ang -= 6.2831853f;
            if (ang > 2.62f)
                k = 0.2f;
            else if (ang > 2.09f)
                k = 0.6f;
            else if (ang > 0.52359878f)
                k = 0.8f;
            else
                k = 1.0f;
        }
        if (field_0x124 && (field_0x124->statusFlags & 1)) {
            Vec3 pos = ((BikeContactSet*)collisionObject)->field_0xa0;
            ((VehicleImpactSink*)field_0x5ac)->SetPosition(pos);
            Vec3 p;
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->scrapeVector = p;
            ((VehicleImpactSink*)field_0x5ac)->updatePending = 1;
        }
        break;
    }
    case 0x68:
        notifyOther = 1;
        other = ((BikeCollisionEvent*)c)->field_0x60;
        hasBody = 1;
        l10 = other->invMass;
        ctx = (int)other->sceneNode;
        otherVel = &other->velocity;
        {
            Vec3 cp = ((BikeContactSet*)collisionObject)->field_0xa0;
            leverC.x = cp.x - other->centerOfMass.x;
            leverC.y = cp.y - other->centerOfMass.y;
            leverC.z = cp.z - other->centerOfMass.z;
        }
        velA = other->worldAngularVelocity;
        velPtr = &other->angularVelocity;
        velB = other->invInertia;
        break;
    case 0x65:
        break;
    case 0:
    case 1:
    case 0x66:
    case 0x6a:
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        if (field_0x124 && (field_0x124->statusFlags & 1)) {
            Vec3 pos = ((BikeContactSet*)collisionObject)->field_0xa0;
            ((VehicleImpactSink*)field_0x5ac)->SetPosition(pos);
            Vec3 p;
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->scrapeVector = p;
            ((VehicleImpactSink*)field_0x5ac)->updatePending = 1;
        }
        break;
    case 0x69: {
        char* obj = (char*)((BikeCollisionEvent*)c)->field_0x60;
        ctx = *(int*)(obj + 0x34);
        velA = g_BikeVec3_005778a8; velB = g_BikeVec3_005778a8; leverC = g_BikeVec3_005778a8;
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        l10 = 0.0f;
        velPtr = 0;
        otherVel = (Vec3*)(obj + 0x40);
        hasBody = 1;
        break;
    }
    case 0x3e8: {
        Vec3 r;
        r.x = ((BikeContactSet*)collisionObject)->field_0xa0.x - centerOfMass.x;
        r.y = ((BikeContactSet*)collisionObject)->field_0xa0.y - centerOfMass.y;
        r.z = ((BikeContactSet*)collisionObject)->field_0xa0.z - centerOfMass.z;
        UnknownVirtualSlot3(&((BikeContactSet*)collisionObject)->field_0xac, &velocity, &r, &s, 0x3e8, 0, &out);
        return;
    }
    case 0x3e9:
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        break;
    case 0x3ea: {
        Vehicle* o = ((BikeCollisionEvent*)c)->field_0x60;
        BikeKindInfo* kind = *(BikeKindInfo**)((char*)o + 0x5c);
        BikeBodyObj* body = *(BikeBodyObj**)((char*)o + 0xc4);
        velA = *body->UnknownVirtualSlot43(&tmp);
        // other point: interpolate body->f23c toward body->f154 by kind->field_0x24
        float t = kind->field_0x24;
        float dx = body->f154.x - body->f23c.x;
        float dy = body->f154.y - body->f23c.y;
        float dz = body->f154.z - body->f23c.z;
        blend.x = dx * t + body->f23c.x;
        blend.y = dy * t + body->f23c.y;
        blend.z = dz * t + body->f23c.z;
        otherVel = &blend;
        velB.x = body->f1d0;
        velB.y = body->f1e4;
        velB.z = body->f1f8;
        l10 = body->f17c;
        velPtr = &body->f16c;
        Vec3 wp = body->f234->LocalToWorldPoint(body->f228);
        Vec3 cp = ((BikeContactSet*)collisionObject)->field_0xa0;
        leverC.x = cp.x - wp.x;
        leverC.y = cp.y - wp.y;
        leverC.z = cp.z - wp.z;
        ctx = (int)body->f234;
        hasBody = 1;
        break;
    }
    case 0x2711: {
        char* obj = (char*)((BikeCollisionEvent*)c)->field_0x60;
        ctx = *(int*)(obj + 0x1a0);
        velA = g_BikeVec3_005778a8; velB = g_BikeVec3_005778a8; leverC = g_BikeVec3_005778a8;
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        l10 = 0.0f;
        velPtr = 0;
        otherVel = (Vec3*)(obj + 0x224);
        hasBody = 1;
        break;
    }
    default:
        return;
    }

    rel.x = ((BikeContactSet*)collisionObject)->field_0xa0.x - centerOfMass.x;
    rel.y = ((BikeContactSet*)collisionObject)->field_0xa0.y - centerOfMass.y;
    rel.z = ((BikeContactSet*)collisionObject)->field_0xa0.z - centerOfMass.z;
    if (hasBody) {
        int l14 = (b == 0x2711 || b == 0x69 || b == 0x3ea) ? 0 : (int)otherVel;
        UnknownVirtualSlot4(&((BikeContactSet*)collisionObject)->field_0xac, &velocity, &worldAngularVelocity, &rel, b, l10,
                            ctx, otherVel, &velA, &leverC, &velB, velPtr, l14, &out, k);
        if (other) {
            other->asleep = 0;
            float len2 = otherVel->x * otherVel->x + otherVel->y * otherVel->y + otherVel->z * otherVel->z;
            other->linearSpeed = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
            other->worldAngularVelocity = other->sceneNode->LocalToWorldDirection(*velPtr);
        }
    } else {
        Vec3 t = worldAngularVelocity;
        scratchVector.x = t.y * rel.z - t.z * rel.y;
        scratchVector.y = t.z * rel.x - rel.z * t.x;
        scratchVector.z = rel.y * t.x - t.y * rel.x;
        Vec3 p;
        p.x = scratchVector.x + velocity.x;
        p.y = velocity.y + scratchVector.y;
        p.z = velocity.z + scratchVector.z;
        UnknownVirtualSlot3(&((BikeContactSet*)collisionObject)->field_0xac, &p, &rel, &s, b, 0, &out);
        if (prevSpeed < 0.001f && linearSpeed < 0.1f) {
            velocity = g_BikeVec3_005778a8;
            angularVelocity.y = 0.0f;
            linearSpeed = 0.0f;
        }
        out = 0.0f;
    }
    if (a)
        UnknownVirtualSlot100(fixedStepTime, hasBody, out);
    if (notifyOther && other->UnknownVirtualSlot52())
        ((Bike*)other)->UnknownVirtualSlot100(((Bike*)other)->fixedStepTime, 1, out);
}

// ---- slot 89: rider pose blend (tier 3 names) ----
// Reached from slot 90 (and Vehicle's per-frame code) with the frame time.  Derives a lean
// value from the wheel state, picks a pose pair (field_0x650/field_0x658) with a blend weight
// (field_0x654/field_0x660) from the animation parameter at field_0x640, then advances a
// smoothed value (field_0x644) toward field_0x504 and maps it through a small 5-state machine
// (field_0x520) to field_0x65c.  Retail converts the pose index with an inlined fistp helper
// (round-to-nearest after the -0.5); plain (int) is used here.
static inline float BikeRange(float x, float lo, float hi)
{
    if (x <= lo)
        return lo;
    return x < hi ? x : hi;
}

int Bike::UnknownVirtualSlot89(float t)
{
    int r = UnknownVirtualSlot66();
    if (r)
        return r;

    float lean;
    if (airborne || rearWheel->inContact)
        lean = -frontWheel->w_0x150;
    else
        lean = 0.0f;

    if (airborne && lean > 2.0f && rearWheel->w_0x150 < -1.0f) {
        float u = (0.75f - steerAxis->steerValue) * 3.99f;
        poseIndex = (int)(u - 0.5f);
        int idx = poseIndex;
        poseIndex = idx + 11;
        poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
        if (field_0x6fc == 0) {
            BikeA644* p = poseSmoother;
            float v = field_0x704;
            *(float*)&p->smoothedValue = 0.0f;
            if (v != FLT_MAX) {
                p->smoothTime = v;
                p->smoothRatio = 1.0f;
            }
            p->maxRise = 1.0f;
            p->maxFall = -1.0f;
            field_0x520 = 1;
        }
        field_0x6fc = 1;
    } else {
        UnknownVirtualSlot41();
        if (!airborne && lean > leanPoseMax) {
            poseBlend = 1.0f;
            poseIndex = 10;
            poseState = 10;
        } else if (!airborne && lean > leanPoseMin) {
            poseIndex = 5;
            poseState = 10;
            poseBlend = (controlInput.y + 1.0f) * 0.5f;
            poseLeanBlend = (lean - leanPoseMin) / (leanPoseMax - leanPoseMin);
        } else if (rearWheel->w_0x27c > 14.0f) {
            poseIndex = (int)((0.75f - steerAxis->steerValue) * 3.99f - 0.5f);
            int idx = poseIndex;
            if (idx == 2) {
                if (rearWheel->w_0x27c > 32.0f)
                    poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - 2.0f;
                else
                    poseBlend = (rearWheel->w_0x27c - 14.0f) * 0.0555f;
            } else {
                poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
            }
            poseState = idx * 2 + 4;
            poseIndex = poseState - 1;
        } else {
            poseIndex = (int)((0.75f - steerAxis->steerValue) * 3.99f - 0.5f);
            int idx = poseIndex;
            poseState = idx * 2 + 3;
            poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
            poseLeanBlend = (14.0f - rearWheel->w_0x27c) * 0.0714285746f;
        }
    }

    float p = controlInput.x;
    float x;
    if (field_0x6fc) {
        // advance the smoothed value toward p, limited by its rate m_0x4
        BikeA644* q = poseSmoother;
        float d = p - *(float*)&q->smoothedValue;
        if (d < 0.0f) {
            if (d <= q->maxFall)
                d = q->maxFall;
        } else if (d >= q->maxRise) {
            d = q->maxRise;
        }
        float step = t < q->smoothTime ? t : q->smoothTime;
        q->smoothRatio = step / q->smoothTime;
        *(float*)&q->smoothedValue = d * q->smoothRatio + *(float*)&q->smoothedValue;
        t = *(float*)&q->smoothedValue;
        switch (field_0x520) {
        case 1:
            x = t * -0.25f;
            poseParam = x;
            if (x < 0.0f) {
                field_0x520 = 3;
                poseParam = 0.5f - x;
            } else if (x > 0.225f) {
                field_0x520 = 2;
                poseParam = 0.5f - x;
            }
            break;
        case 2:
            x = t * 0.25f + 0.5f;
            poseParam = x;
            if (x > 0.5f)
                field_0x520 = 3;
            break;
        case 3:
            x = t * 0.25f + 0.5f;
            poseParam = x;
            if (x < 0.5f) {
                poseParam = 0.5f - x;
                field_0x520 = 1;
            } else if (x > 0.725f) {
                poseParam = 1.5f - x;
                field_0x520 = 4;
            }
            break;
        case 4:
            x = t * -0.25f + 1.0f;
            poseParam = x;
            if (x > 1.0f) {
                poseParam = x - 1.0f;
                field_0x520 = 1;
            }
            break;
        }
    } else {
        switch (field_0x520) {
        case 1:
            if (p < 0.0f) {
                if (p < -0.9f)
                    field_0x520 = 2;
                x = p * -0.25f;
                poseParam = BikeRange(x, 0.0f, 0.25f);
            } else {
                if (p > 0.9f)
                    field_0x520 = 4;
                x = p * 0.25f + 0.5f;
                poseParam = BikeRange(x, 0.5f, 0.75f);
            }
            break;
        case 2:
            if (p > prevControlInput.x) {
                field_0x520 = 3;
                x = p * 0.25f + 0.5f;
                poseParam = BikeRange(x, 0.25f, 0.5f);
            } else {
                x = p * -0.25f;
                poseParam = BikeRange(x, 0.0f, 0.25f);
            }
            break;
        case 3:
            if (p > -0.032f) {
                field_0x520 = 1;
                if (p < 0.0f) {
                    x = p * -0.25f;
                    poseParam = BikeRange(x, 0.0f, 0.25f);
                } else {
                    x = p * 0.25f + 0.5f;
                    poseParam = BikeRange(x, 0.5f, 0.75f);
                }
            } else {
                x = p * 0.25f + 0.5f;
                poseParam = BikeRange(x, 0.25f, 0.5f);
            }
            break;
        case 4:
            if (p < prevControlInput.x) {
                field_0x520 = 5;
                x = p * -0.25f + 1.0f;
                poseParam = BikeRange(x, 0.75f, 1.0f);
            } else {
                x = p * 0.25f + 0.5f;
                poseParam = BikeRange(x, 0.5f, 0.75f);
            }
            break;
        case 5:
            if (p < 0.032f) {
                field_0x520 = 1;
                if (p < 0.0f) {
                    x = p * 0.25f + 0.5f;
                    poseParam = BikeRange(x, 0.5f, 0.75f);
                } else {
                    x = p * -0.25f;
                    poseParam = BikeRange(x, 0.0f, 0.25f);
                }
            } else {
                x = p * -0.25f + 1.0f;
                poseParam = BikeRange(x, 0.75f, 1.0f);
            }
            break;
        }
    }
    return 0;
}

// ---- collision callbacks (0x00405cd0, 0x00405d70) -------------------------------------
// The bike's version of SoulTreePhysics.cpp's pair: 0x004079c0 stores them at the collision
// object's +0x88 / +0x8c (0x004092c0, 0x004092d0).  The other object's type tag is recorded
// and forwarded to slot 38; slot 38's first argument is slot 52's answer unless the tag is
// 0x3e8.  Tags 0x64/0x65 (and 0x68 for the second) are filtered as below (tier 1 data flow).
void BikeCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    Vehicle* vehicle = (Vehicle*)self->ownerObject;
    int kind = other->ownerType;
    vehicle->lastCollisionType = kind;
    if (!vehicle->UnknownVirtualSlot51() || kind == 0x66 || kind == 0x3e9 || kind == 0x6a) {
        if (kind == 0x64) {
            if (((Vehicle*)other->ownerObject)->UnknownVirtualSlot51())
                return;
        } else if (kind == 0x65) {
            return;
        }
        vehicle->UnknownVirtualSlot38(vehicle->UnknownVirtualSlot52() && kind != 0x3e8 ? 1 : 0, kind, other);
    }
}

void BikeStaticCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    Vehicle* vehicle = (Vehicle*)self->ownerObject;
    int kind = other->ownerType;
    if (!vehicle->UnknownVirtualSlot51() && kind != 0x64 && kind != 0x65 && kind != 0x68)
        BikeCollisionCallback(self, other);
}
