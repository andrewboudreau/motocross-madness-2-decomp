// Bring-up definition of ConstraintMethodCollisionModel::ApplyContactImpulse (0x0043bdb0,
// ret 0x2c), the one function of retail ConstraintMethodCollisionModel.cpp that
// src/krusty2/constraint/ConstraintMethodCollisionModel.cpp leaves to its near-miss sample.
//
// Status: best-guess-unreviewed.  The body is the near-miss candidate from
// samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp (2515/2619 strict
// positions; call sequence and every operand match retail per that sample's notes), with the
// TU-local helpers it needs copied here as statics.  The sample includes the canonical .cpp;
// this file includes only the header, so the TU-private zero vector is replicated locally.
//
// Impulse response of the body for one contact: 'offset' is the penetration offset,
// 'point' the surface point (the contact point is point + offset), 'normal' the contact
// normal, 't' the time fraction used to interpolate the linear velocity, 'other' the other
// body or null.
//   vLin = prev + (vel - prev) * t; rA = (point + offset) - centre of mass (world);
//   vA = vLin + wA x rA;  vB = other ? vB + wB x rB : 0;  vn = (vA - vB) . n;
//   j = -(1 + e) vn / (1/mA + ((I_A^-1 (rl x nl)) x rl) . nl + 1/mB + ((M_B (rB x n)) x rB) . n)
//   scaled by 1 - impulseLoss and clamped to maxImpulse;
//   P = m vLin + j n; L = I w + rl x local(j n); v = P / m;
//   velocity = v (1 - dt damping), clamped to maxSpeed; w = I^-1 L (1 - dt damping).
#include "constraint/ConstraintMethodCollisionModel.h"
#include "math/Vec3.h"
#include "../reconstructed/Matrix4.h"
#include "core/SoultreeObject.h"

// Every vector operation is written TU-local: ConstraintTypes.h only declares the
// out-of-line operator copies (0x00404e60, 0x00421cb0, 0x005015b0, ...) and src/ does not
// define them, so they are replaced by inline helpers with the same arithmetic.  The
// ConNode calls go to the canonical SoultreeObject methods (ConNode is a view of it).
float FastSqrt(float x);     // 0x00460b50 (src/reconstructed/FastMath.cpp)
float FastInvSqrt(float x);  // 0x00460c00 (src/reconstructed/FastMath.cpp)

static inline ConVec3 ConMake(float x, float y, float z)
{
    ConVec3 r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
}

static inline ConVec3 ConFromVec3(const Vec3& v)
{
    return ConMake(v.x, v.y, v.z);
}

static inline Vec3 ConToVec3(const ConVec3& v)
{
    return Vec3(v.x, v.y, v.z);
}

// 0x004fd660 / 0x004fd710 / 0x004fc9a0 through the canonical SoultreeObject.
static inline ConVec3 ConLocalToWorldPoint(ConNode* node, const ConVec3& p)
{
    Vec3 in = ConToVec3(p);
    return ConFromVec3(((SoultreeObject*)node)->LocalToWorldPoint(in));
}

static inline ConVec3 ConWorldToLocalDirection(ConNode* node, const ConVec3& v)
{
    Vec3 in = ConToVec3(v);
    return ConFromVec3(((SoultreeObject*)node)->WorldToLocalDirection(in));
}

static inline void ConGetPositionIn(ConNode* node, ConVec3* out)
{
    Vec3 position;
    ((SoultreeObject*)node)->GetPositionIn(0, &position);
    *out = ConFromVec3(position);
}

static inline ConVec3 ConSub(const ConVec3& a, const ConVec3& b)
{
    return ConMake(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline ConVec3 ConAdd(const ConVec3& a, const ConVec3& b)
{
    return ConMake(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline ConVec3 ConScale(const ConVec3& v, float s)
{
    return ConMake(v.x * s, v.y * s, v.z * s);
}

// 0x00515600: a x b
static inline ConVec3 ConCross(const ConVec3& a, const ConVec3& b)
{
    return ConMake(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// 0x0043ca20: row vector times the upper 3x3.
static inline ConVec3 ConRotate(const ConVec3& v, const ConMatrix& m)
{
    return ConMake(v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
                   v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
                   v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]);
}

// Row vector times the matrix including the translation row.
// best guess (kept from the sample): retail adds B's inverse-inertia translation row here.
static inline ConVec3 ConTransform(const ConVec3& v, const ConMatrix& m)
{
    return ConMake((v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0]) + m.m[3][0],
                   (v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1]) + m.m[3][1],
                   (v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]) + m.m[3][2]);
}

static inline float ConDot(const ConVec3& a, const ConVec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// 0x00435ec0: 1.0 when |v|^2 == 1, else FastSqrt(|v|^2).
static inline float ConMagnitude(const ConVec3& v)
{
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

// 0x005087b0: v unchanged when |v|^2 == 1, else v * FastInvSqrt(|v|^2) (no zero check).
static inline ConVec3 ConNormalize(const ConVec3& v)
{
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (squared == 1.0f)
        return v;
    return ConScale(v, FastInvSqrt(squared));
}

// 0x0043bdb0 (ret 0x2c)
void ConstraintMethodCollisionModel::ApplyContactImpulse(ConBody* other, float t,
                                                         ConVec3 offset, ConVec3 point, ConVec3 normal)
{
    ConVec3 wA = body->GetWorldAngularVelocity();
    ConVec3 vLin = ConAdd(ConScale(ConSub(body->velocity, body->prevVelocity), t), body->prevVelocity);
    ConVec3 rA = ConLocalToWorldPoint(body->node, body->centerOfMass);
    point = ConAdd(point, offset);
    rA = ConSub(point, rA);
    contactPoint = point;
    ConVec3 rl = ConWorldToLocalDirection(body->node, rA);
    ConVec3 nl = ConWorldToLocalDirection(body->node, normal);
    ConVec3 vA = ConAdd(vLin, ConCross(wA, rA));
    ConVec3 vB;
    ConVec3 rB;
    if (!other) {
        vB = ConMake(0.0f, 0.0f, 0.0f);  // retail copies the TU zero vector 0x00579830
    } else {
        ConVec3 vbLin = other->velocity;
        ConVec3 wB = other->GetWorldAngularVelocity();
        ConGetPositionIn(other->node, &rB);
        rB = ConSub(point, rB);
        vB = ConAdd(vbLin, ConCross(wB, rB));
    }
    ConVec3 vRel = ConSub(vA, vB);
    float num = -((restitution + 1.0f) * ConDot(normal, vRel));

    float invMassA = 1.0f / body->mass;
    ConVec3 axisA = ConRotate(ConCross(rl, nl), body->invInertia);
    float termA = ConDot(ConCross(axisA, rl), nl);
    float invMassB = 0.0f;
    float termB = 0.0f;
    if (other) {
        invMassB = 1.0f / other->mass;
        ConVec3 axisB = ConTransform(ConCross(rB, normal), other->invInertia);
        termB = ConDot(ConCross(axisB, rB), normal);
    }
    float j = num / (invMassA + termA + invMassB + termB);
    j *= 1.0f - impulseLoss;
    if (maxImpulse != 0 && j > maxImpulse)
        j = maxImpulse;

    ConVec3 J = ConScale(normal, j);
    ConVec3 P = ConScale(vLin, body->mass);
    ConVec3 L = ConRotate(body->angularVelocity, body->inertia);
    P = ConAdd(P, J);
    // Retail converts rA to the local frame in place and computes local(J) twice; the
    // node calls are kept (they have no side effects beyond the result).
    rA = ConWorldToLocalDirection(body->node, rA);
    ConVec3 Jl = ConWorldToLocalDirection(body->node, J);
    Jl = ConWorldToLocalDirection(body->node, J);
    L = ConAdd(L, ConCross(rl, Jl));
    // 0x0043c890: v / s is v * (1 / s).
    ConVec3 v = ConScale(P, 1.0f / body->mass);
    body->velocity = ConScale(v, 1.0f - dt * damping);
    if (maxSpeed != 0 && ConMagnitude(body->velocity) > maxSpeed)
        body->velocity = ConScale(ConNormalize(body->velocity), maxSpeed);
    body->angularVelocity = ConScale(ConRotate(L, body->invInertia), 1.0f - dt * damping);
    // Retail reloads wA (slot 43) and forms v + wA x local(rA), then discards it; only the
    // virtual call can have a side effect, so it is kept.
    wA = body->GetWorldAngularVelocity();
}
