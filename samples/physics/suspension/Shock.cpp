// Shock.cpp -- candidate for the unnamed translation unit at 0x004f9a60..0x004fb1d7
// (Shock, InlineShock, RotatingShock).  Not SelectiveGravityModel.cpp: see README.md.
#include "Suspension.h"
#include "core/DebugAlloc.h"
#include "math/FastMath.h"
#include <math.h>
#include <float.h>

// The four TU-private constant vectors (dynamic initializers, bodies 0x4fafe0/0x4fb030/0x4fb080/0x4fb0d0
// $E1/$E4/$E7/$E10).  Copy-initialisation gives retail's stack temporary plus copy.
const ShockVec3 kShockZero = ShockVec3(0.0f, 0.0f, 0.0f);
const ShockVec3 kShockAxisX = ShockVec3(1.0f, 0.0f, 0.0f);
const ShockVec3 kShockAxisY = ShockVec3(0.0f, 1.0f, 0.0f);
const ShockVec3 kShockAxisZ = ShockVec3(0.0f, 0.0f, 1.0f);

// ---------------------------------------------------------------------------
// Shock
// ---------------------------------------------------------------------------

Shock::Shock(void* name, int root, float stiffnessMin_, float stiffnessMax_, float dampingRatio_,
             float speedThreshold_, float retractScale_)
    : MovingPart(name, root, 0)
{
    stiffnessMin = stiffnessMin_;
    stiffnessMax = stiffnessMax_;
    dampingRatio = dampingRatio_;
    atLimit = 0;
    extending = 0;
    prevLength = 0.0f;
    clearedLength = 0.0f;
    length = 0.0f;
    ratio = 0.0f;
    speedThreshold = speedThreshold_;
    retractScale = retractScale_;
    spring = kShockZero;
    damper = kShockZero;
    displacement = kShockZero;
    stepDisplacement = kShockZero;
    velocity = kShockZero;
    speed = 0.0f;
    impulseScale = 0.0f;
}

Shock::~Shock()
{
}

void Shock::Reset()
{
    atLimit = 0;
    ratio = 0.0f;
    length = 0.0f;
    spring = kShockZero;
    damper = kShockZero;
    displacement = kShockZero;
    stepDisplacement = kShockZero;
    velocity = kShockZero;
    speed = 0.0f;
    extending = 0;
    prevLength = 0.0f;
    clearedLength = 0.0f;
    impulseScale = 0.0f;
}

void Shock::ClearForces()
{
    atLimit = 0;
    spring = kShockZero;
    damper = kShockZero;
    clearedLength = length;
    extending = 0;
    speed = 0.0f;
    displacement = kShockZero;
    stepDisplacement = kShockZero;
    velocity = kShockZero;
}

void Shock::ApplyDamping(float loadWeight, float massFactor, float dt, float speedOut)
{
    float stiffness = stiffnessMin + (stiffnessMax - stiffnessMin) * ratio;
    float mass = loadWeight * massFactor;
    float len = velocity.LengthSquared();
    len = (len == 1.0f) ? 1.0f : FastSqrt(len);
    if (len <= speedThreshold) {
        float coeff = FastSqrt(mass * stiffness) * dampingRatio * -2.0f;
        damper = velocity * coeff;
    } else {
        float coeff = FastSqrt(mass * stiffness) * dampingRatio * -2.0f;
        coeff *= ((len - speedThreshold) * 0.5f + speedThreshold) / len;
        damper = velocity * coeff;
    }
    spring = displacement * stiffness;
    spring += damper;
}

// ---------------------------------------------------------------------------
// InlineShock
// ---------------------------------------------------------------------------

InlineShock::InlineShock(void* name, int root, ShockNode* anchorNode_, float minLength_, float stiffnessMin_,
                         float stiffnessMax_, float dampingRatio_, float speedThreshold_, float retractScale_)
    : Shock(name, root, stiffnessMin_, stiffnessMax_, dampingRatio_, speedThreshold_, retractScale_)
{
    minLength = minLength_;
    axisLength = minLength_;
    anchorNode = anchorNode_;
    axis = ShockVec3(-kShockAxisY.x, -kShockAxisY.y, -kShockAxisY.z);
}

InlineShock::~InlineShock()
{
}

// The store to axisLength is embedded in the divisor: retail's `fld st(0); fdivr [1.0]` (the
// only one in .text) needs it; a separate `axisLength = len;` statement gives `fld [1.0];
// fdiv st(1)`, as do named inverse locals, `1.0f / axisLength` and parenthesised forms.
#include "../../../src/reconstructed/InlineShockAxis.cpp"

void InlineShock::ClampAndStep(float maxDelta, float dt, ShockVec3& pos)
{
    if (length >= minLength) {
        float negMin = -minLength;
        atLimit = 1;
        ratio = 1.0f;
        pos = axis * negMin;
        displacement = displacement * (minLength / length);
        length = minLength;
    } else {
        atLimit = 0;
        ratio = length / minLength;
    }
    sceneNode->SetPositionVec(pos);
    float delta = prevLength - length;
    if (length != 0.0f) {
        if (delta >= 0.0f) {
            float clamped = (delta < maxDelta) ? delta : maxDelta;
            velocity = displacement * (clamped / (dt * length));
        } else {
            float clamped = (delta > -maxDelta) ? delta : -maxDelta;
            velocity = displacement * (clamped / (dt * length));
        }
        stepDisplacement = displacement * -(delta / length);
    } else {
        velocity = kShockZero;
        stepDisplacement = kShockZero;
    }
    extending = (length > prevLength);
    prevLength = length;
}

void InlineShock::Retract(float amount, const ShockCarrier* carrier)
{
    if (ratio != 0.0f) {
        float retraction = amount * (minLength / (carrier->field_0x2a4 * retractScale));
        if (!_finite(retraction) || retraction < 0.0f)
            return;
        if (retraction >= length) {
            Reset();
            return;
        }
        length = length - retraction;
        ratio = length / minLength;
        sceneNode->SetPosition(axis * -length);
    }
    Shock::ClearForces();
}

// *out = the projection of v onto `onto` (zero when `onto` is zero).  Writing through
// an out pointer lets VC6 place the result in the dead `toPoint` slot like retail.
static inline void ProjectInto(ShockVec3* out, const ShockVec3& v, const ShockVec3& onto)
{
    if (onto.x == kShockZero.x && onto.y == kShockZero.y && onto.z == kShockZero.z)
        *out = kShockZero;
    else
        *out = ShockScaleCall(onto, ShockDotCall(v, onto) / ShockDotCall(onto, onto));
}
void InlineShock::SolveContact(float dt, const ShockVec3* offset, const ShockVec3* base,
                               const ShockVec3* point, const ShockVec3* normal, float limit,
                               float* outLoad, int* outActive)
{
    int miss;
    float contactRatio;
    ShockVec3 anchor;
    anchorNode->GetPositionIn(0, &anchor);
    ShockVec3 toPoint = *point - anchor;
    float dist = ShockDot(toPoint, *normal);
    if (dist >= 0.0f)
        dist = ShockDot(toPoint, ShockVec3(-normal->x, normal->y, -normal->z));
    if (dist < 0.0) {
        contactRatio = limit / dist;
        miss = 0;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    float len = toPoint.LengthSquared();
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    float over = axisLength - len;
    if (!(over > 0.0f))
        over = 0.0f;
    length = over - len * contactRatio;
    float scale = -(length / len);
    displacement = toPoint * scale;
    ShockVec3 pos = axis * -length;
    ShockVec3 pointVelocity = *base + *offset;
    {
        ShockVec3 proj;
        ProjectInto(&proj, pointVelocity, displacement);
        float speedSq = ShockDotCall(proj, proj);
        speed = (speedSq == 1.0f) ? 1.0f : FastSqrt(speedSq);
    }
    ClampAndStep(speed * dt, dt, pos);
    if (atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    float load = ShockDot(stepDisplacement, *normal);
    *outLoad = limit - load;
    *outActive = 1;
}

void InlineShock::Reset()
{
    sceneNode->SetPosition(0.0f, 0.0f, 0.0f);
    Shock::Reset();
}

ShockVec3 ShockProject(const ShockVec3& v, const ShockVec3& onto)
{
    if (onto.x == kShockZero.x && onto.y == kShockZero.y && onto.z == kShockZero.z)
        return kShockZero;
    float scale = ShockDot(v, onto) / ShockDot(onto, onto);
    return onto * scale;
}

RotatingShock::RotatingShock(void* name, int root, float maxAngle_, float armLength_, float stiffnessMin_,
                             float stiffnessMax_, float dampingRatio_, float speedThreshold_,
                             float retractScale_, int axisId_, const ShockVec3* axisOverride)
    : Shock(name, root, stiffnessMin_, stiffnessMax_, dampingRatio_, speedThreshold_, retractScale_)
{
    maxAngle = maxAngle_;
    armLengthSq = armLength_ * armLength_;
    armLength = armLength_;
    axisId = axisId_;
    chord = 2.0f * (float)sin(maxAngle_ * 0.5f) * armLength_;
    if (axisOverride) {
        rotationAxis = *axisOverride;
        return;
    }
    switch (axisId_) {
    case 1:
        rotationAxis = ShockVec3(-kShockAxisX.x, -kShockAxisX.y, -kShockAxisX.z);
        break;
    case 2:
        rotationAxis = ShockVec3(-kShockAxisZ.x, -kShockAxisZ.y, -kShockAxisZ.z);
        break;
    case 3:
        rotationAxis = kShockAxisZ;
        break;
    default:
        rotationAxis = kShockAxisX;
        break;
    }
}

RotatingShock::~RotatingShock()
{
}

void RotatingShock::Retract(float amount, const ShockCarrier* carrier)
{
    if (ratio != 0.0f) {
        float retraction = amount * (chord / (carrier->field_0x2a4 * retractScale));
        if (!_finite(retraction) || retraction < 0.0f)
            return;
        if (retraction >= length) {
            Reset();
            return;
        }
        length = length - retraction;
        float delta = length / armLength - angle;
        if (_finite(delta)) {
            sceneNode->Rotate(rotationAxis, delta);
            ratio = length / chord;
            angle = delta + angle;
        }
    }
    Shock::ClearForces();
}

void RotatingShock::ClampAndStep(float maxDelta, float dt)
{
    if (length >= chord) {
        float limitScale = chord / length;
        atLimit = 1;
        ratio = 1.0f;
        angle = maxAngle;
        displacement = displacement * limitScale;
        length = chord;
    } else {
        angle = length / armLength;
        atLimit = 0;
        ratio = length / chord;
    }
    if (_finite(angle)) {
        sceneNode->SetMatrixLike((const char*)this + 4);
        sceneNode->Rotate(rotationAxis, angle);
    }
    float delta = prevLength - length;
    if (length != 0.0f) {
        if (delta >= 0.0f) {
            float clamped = (delta < maxDelta) ? delta : maxDelta;
            velocity = displacement * (clamped / (dt * length));
        } else {
            float clamped = (delta > -maxDelta) ? delta : -maxDelta;
            velocity = displacement * (clamped / (dt * length));
        }
        stepDisplacement = displacement * -(delta / length);
    } else {
        velocity = kShockZero;
        stepDisplacement = kShockZero;
    }
    extending = (length > prevLength);
    prevLength = length;
}

void RotatingShock::Reset()
{
    sceneNode->SetMatrixLike((const char*)this + 4);
    angle = 0.0f;
    Shock::Reset();
}

void RotatingShock::SolveContact(float dt, const ShockVec3* axisA, const ShockVec3* axisB,
                                 const ShockVec3* point, const ShockVec3* normal,
                                 const ShockVec3* offsetA, const ShockVec3* offsetB, float limit,
                                 float* outLoad, int* outActive)
{
    int miss = 0;
    float contactRatio;
    ShockVec3 anchor;
    sceneNode->GetPositionIn(0, &anchor);
    ShockVec3 toPoint = *point - anchor;
    ShockVec3 swing;
    switch (axisId) {
    case 0: swing = ShockCross(*axisA, toPoint); break;
    case 1: swing = ShockCross(toPoint, *axisA); break;
    case 2: swing = ShockCross(*axisB, toPoint); break;
    case 3: swing = ShockCross(toPoint, *axisB); break;
    }
    float dist = (swing.x * normal->x + swing.y * normal->y) + swing.z * normal->z;
    if (dist >= 0.0f) {
        if (dist == 0.0f) {
            *outLoad = limit;
            float side = (offsetA->y * normal->y + normal->x * offsetA->x) + offsetA->z * normal->z;
            *outActive = (side < -1.0f) ? 1 : 0;
            speed = 0.0f;
            return;
        }
        dist = ShockDot(swing, ShockVec3(-normal->x, normal->y, -normal->z));
    }
    if (dist < 0.0) {
        contactRatio = limit / dist;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    length = clearedLength - contactRatio * armLength;
    displacement = ShockScaleCall(swing, (-length) / armLength);
    speed = ShockLength(ShockProject(ShockAddCall(*offsetA, *offsetB), displacement));
    ClampAndStep(speed * dt, dt);
    if (atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    *outLoad = limit - ShockDotCall(*normal, stepDisplacement);
    *outActive = 1;
}
