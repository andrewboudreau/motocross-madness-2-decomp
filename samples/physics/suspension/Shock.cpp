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

void Shock::ApplyDamping(float a, float b, float c, float d)
{
    float k = stiffnessMin + (stiffnessMax - stiffnessMin) * ratio;
    float m = a * b;
    float len = velocity.LengthSquared();
    len = (len == 1.0f) ? 1.0f : FastSqrt(len);
    if (len <= speedThreshold) {
        float coeff = FastSqrt(m * k) * dampingRatio * -2.0f;
        damper = velocity * coeff;
    } else {
        float coeff = FastSqrt(m * k) * dampingRatio * -2.0f;
        coeff *= ((len - speedThreshold) * 0.5f + speedThreshold) / len;
        damper = velocity * coeff;
    }
    spring = displacement * k;
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
            float t = (delta < maxDelta) ? delta : maxDelta;
            velocity = displacement * (t / (dt * length));
        } else {
            float t = (delta > -maxDelta) ? delta : -maxDelta;
            velocity = displacement * (t / (dt * length));
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
        float v = amount * (minLength / (carrier->field_0x2a4 * retractScale));
        if (!_finite(v) || v < 0.0f)
            return;
        if (v >= length) {
            Reset();
            return;
        }
        length = length - v;
        ratio = length / minLength;
        sceneNode->SetPosition(axis * -length);
    }
    Shock::ClearForces();
}

// *out = the projection of v onto `onto` (zero when `onto` is zero).  Writing through
// an out pointer lets VC6 place the result in the dead `d` slot like retail.
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
    float t;
    ShockVec3 anchor;
    anchorNode->GetPositionIn(0, &anchor);
    ShockVec3 d = *point - anchor;
    float dist = ShockDot(d, *normal);
    if (dist >= 0.0f)
        dist = ShockDot(d, ShockVec3(-normal->x, normal->y, -normal->z));
    if (dist < 0.0) {
        t = limit / dist;
        miss = 0;
    } else {
        t = 0.0f;
        miss = 1;
    }
    float len = d.LengthSquared();
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    float over = axisLength - len;
    if (!(over > 0.0f))
        over = 0.0f;
    length = over - len * t;
    float k = -(length / len);
    displacement = d * k;
    ShockVec3 pos = axis * -length;
    ShockVec3 w = *base + *offset;
    {
        ShockVec3 proj;
        ProjectInto(&proj, w, displacement);
        float u = ShockDotCall(proj, proj);
        speed = (u == 1.0f) ? 1.0f : FastSqrt(u);
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
    float s = ShockDot(v, onto) / ShockDot(onto, onto);
    return onto * s;
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
        float v = amount * (chord / (carrier->field_0x2a4 * retractScale));
        if (!_finite(v) || v < 0.0f)
            return;
        if (v >= length) {
            Reset();
            return;
        }
        length = length - v;
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
        float s = chord / length;
        atLimit = 1;
        ratio = 1.0f;
        angle = maxAngle;
        displacement = displacement * s;
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
            float t = (delta < maxDelta) ? delta : maxDelta;
            velocity = displacement * (t / (dt * length));
        } else {
            float t = (delta > -maxDelta) ? delta : -maxDelta;
            velocity = displacement * (t / (dt * length));
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
    float t;
    ShockVec3 anchor;
    sceneNode->GetPositionIn(0, &anchor);
    ShockVec3 d = *point - anchor;
    ShockVec3 r;
    switch (axisId) {
    case 0: r = ShockCross(*axisA, d); break;
    case 1: r = ShockCross(d, *axisA); break;
    case 2: r = ShockCross(*axisB, d); break;
    case 3: r = ShockCross(d, *axisB); break;
    }
    float dist = (r.x * normal->x + r.y * normal->y) + r.z * normal->z;
    if (dist >= 0.0f) {
        if (dist == 0.0f) {
            *outLoad = limit;
            float side = (offsetA->y * normal->y + normal->x * offsetA->x) + offsetA->z * normal->z;
            *outActive = (side < -1.0f) ? 1 : 0;
            speed = 0.0f;
            return;
        }
        dist = ShockDot(r, ShockVec3(-normal->x, normal->y, -normal->z));
    }
    if (dist < 0.0) {
        t = limit / dist;
    } else {
        t = 0.0f;
        miss = 1;
    }
    length = clearedLength - t * armLength;
    displacement = ShockScaleCall(r, (-length) / armLength);
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
