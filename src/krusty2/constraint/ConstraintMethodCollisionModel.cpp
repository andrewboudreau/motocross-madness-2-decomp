// With ConstraintMethodCollisionModel.bindings.json the 19 exact targets of this file
// (src/krusty2/constraint/targets.json) pass `tools/run_physics_samples.py --strict`.
// ConstraintMethodCollisionModel.cpp -- reconstruction of the retail translation unit
// "ConstraintMethodCollisionModel.cpp" (__FILE__ xref at 0x0043c7f9). Unit extent
// 0x0043b800..0x0043ca20 plus the out-of-line vector helpers 0x0043c890 / 0x0043ca20 it calls.
// Functions are in retail order. ApplyContactImpulse (0x0043bdb0) is not exact; its
// status comment says what differs, and README.md lists the forms tried.
//
// SOLVER OVERVIEW (semantic names are tier 3; instruction behaviour is tier 1)
//   Method: single-contact IMPULSE (velocity-level rigid body response) plus a small
//   PROJECTION step. There is no penalty force and no iterative solver in this file.
//   Structure of one step (Step at 0x0043ba70, called every tick by slot 10):
//     1. Save dt (+0xc0). If the model is active (+0xec) refresh the collision node
//        (0x435fb0/0x435fe0); when the base reports an overlap (0x438e70) push the body
//        back by the record's contact offset (+0x00) * 1.005 (projection) and refresh again.
//     2. For every probe point (32-byte records, AddProbePoint 0x0043c7f0): transform the
//        local point to world space, ask the ground query object (+0xe4, 0x507c10) for the
//        surface height/normal below it. A probe whose world y is below the surface
//        is "hit"; the deepest penetration, its normal and the surface point the query
//        returned are kept (retail copies the queried point, not the probe position, at
//        0x0043bc19).  A hit probe records the best normal so far; a missed probe only
//        clears its hit flag.
//     3. If any probe hit (and +0x108 is set): call ApplyContactImpulse once, with the
//        deepest probe, offset = normal * -penetration, then project the body out of the
//        surface by that offset (position -= offset * 1.005, 0x0043bd23) and mark +0x58
//        dirty.  1.005f is a literal: the constant-pool operand order (fld v; fmul k)
//        only matches with a literal.
//   ApplyContactImpulse (0x0043bdb0) accumulates nothing across iterations; per call it
//   computes  j = -(1+e) * (vA - vB).n / (1/mA + 1/mB + n.((Iinv (rA x n)) x rA) + ...),
//   scales j by (1 - impulseLoss, +0xf4), clamps to maxImpulse (+0xf8, 0 = unlimited), updates ONLY body A's linear
//   velocity (P' = m*v + j*n, damped by 1 - dt*damping) and angular momentum (L' = I*w + rA x J),
//   and clamps the speed to maxSpeed (+0xfc). Body B (the other party, may be null) only
//   contributes to the denominator and to the relative velocity (null -> zero vector 0x579830).
//   Other content of this TU: the contact callback 0x43b800 and the four kVec3 initializer pairs
//   0x43c8e0..0x43ca1b.  The pixel blend helpers just before it (0x43b320..0x43b7f9,
//   src/reconstructed/PixelBlend.cpp; 0x43b660 in its sample) have no literal tying them here.
//
#include "ConstraintMethodCollisionModel.h"
#include "math/Vec3.h"
#include "../../reconstructed/Matrix4.h"
#include "core/SoultreeObject.h"
#include "rigidbody/PhysicsBody.h"
#include "broadphase/Terrain.h"

#include "core/DebugAlloc.h"   // DebugRealloc
#include "../../reconstructed/VectorAddAssign.h"   // Vec3AddAssignCall (0x00428060)

// Out-of-line copies of the header inline vector operators that ApplyContactImpulse calls at
// its late sites (VC6 inline budget, docs/VC6_INLINE_BUDGET.md). Declared, not defined, so the
// calls stay calls; the bindings map them to the retail copies. This unit does not include
// math/Math3D.h, which defines them inline.
Vec3 operator+(const Vec3& a, const Vec3& b);       // 0x00421cb0
Vec3 operator*(const Vec3& v, float s);             // 0x005015b0
Vec3 operator*(float s, const Vec3& v);             // 0x00428090
Vec3 operator/(const Vec3& v, float s);             // 0x0043c890 (v * (1/s))
Vec3 CrossProduct(const Vec3& a, const Vec3& b);    // 0x00515600
float Magnitude(const Vec3& v);                     // 0x00435ec0 (1.0 if |v|^2 == 1 else FastSqrt)
Vec3 Vec3Normalize(const Vec3& v);                  // 0x005087b0 (src/reconstructed/Math3D.cpp)
// 0x0043ca20: row vector times the upper 3x3 of m (x' = v.x*m00 + v.y*m10 + v.z*m20).
Vec3 RotateVector(const Vec3& v, const Matrix4& m);

// The two callbacks slot 8 installs (CollisionObject.h, CollisionCallback).
void ConstraintContactCallback(CollisionObject* self, CollisionObject* other);  // 0x0043b800, below
void CollisionEmptyCallback(CollisionObject* self, CollisionObject* other);     // 0x00464e90, shared empty function

// TU-private vector constants, .CRT$XCU entries 66-69: zero 0x00579830 (thunk 0x0043c8e0,
// body 0x0043c8f0), x axis 0x00579840 (0x0043c930/0x0043c940), y axis 0x00579850
// (0x0043c980/0x0043c990) and z axis 0x00579820 (0x0043c9d0/0x0043c9e0).  Each 60-byte body
// builds the value in a stack temporary and copies it out, which is the shape VC6 gives
// copy-initialization from a constructor call (direct initialization stores the floats
// straight into the global).  Only the zero vector has a reader: ApplyContactImpulse copies it
// as a whole Vec3 (three dword moves).
static const Vec3 g_ConZero = Vec3(0.0f, 0.0f, 0.0f);
static const Vec3 g_ConXAxis = Vec3(1.0f, 0.0f, 0.0f);
static const Vec3 g_ConYAxis = Vec3(0.0f, 1.0f, 0.0f);
static const Vec3 g_ConZAxis = Vec3(0.0f, 0.0f, 1.0f);

// ---- Contact callback (0x0043b800) ----------------------------------------------
// Stored into onHitCallback by slot 8 (with 0x464e90 in onHitByCallback). cdecl; self is the
// constraint model, b the other collision object. The contact record at a->contactRecord
// (+0x5c) is the CollisionSweepQuery of CollisionShapeTests.h viewed with a float fraction;
// tier 3 semantics.
struct ConContactRecord {
    Vec3 contactOffset;  // +0x00 passed as ApplyContactImpulse's offset
    Vec3 contactNormal;  // +0x0c passed as the normal (last argument)
    Vec3 contact;        // +0x18 passed as the base point
    float t;                // +0x24 time fraction
};

void ConstraintContactCallback(CollisionObject* self, CollisionObject* other)
{
    ConstraintMethodCollisionModel* model = (ConstraintMethodCollisionModel*)self;
    ConContactRecord* rec = (ConContactRecord*)model->contactRecord;
    if (other->ownerType == 0x3ea)
        model->ApplyContactImpulse(*(PhysicsBody**)((char*)other + 0xc4), rec->t, rec->contactOffset, rec->contact,
                                   rec->contactNormal);
    else
        model->ApplyContactImpulse(0, rec->t, rec->contactOffset, rec->contact, rec->contactNormal);
}

ConstraintMethodCollisionModel::ConstraintMethodCollisionModel(int flags) : CollisionObject(flags)
{
    // Statement order found by a placement search against the retail store schedule
    // (0x0043b8d0); the order of these independent stores has no semantic meaning.
    field_0xbc = 0.9f;
    shapeCollisionEnabled = 1;
    restitution = 0;
    applyImpulses = 1;
    body = 0;
    probeCount = 0;
    terrain = 0;
    probes = 0;
    manualStep = 0;
    damping = 0;
    impulseLoss = 0;
    maxImpulse = 0;
    maxSpeed = 0;
}

void ConstraintMethodCollisionModel::UnknownVirtualSlot2()
{
}

ConstraintMethodCollisionModel::~ConstraintMethodCollisionModel()
{
}

GameObject* ConstraintMethodCollisionModel::SetRenderContext(void* context)
{
    Configure(context, 1, 1, 1);
    onHitCallback = ConstraintContactCallback;
    onHitByCallback = CollisionEmptyCallback;
    ownerObject = this;
    return this;
}

int ConstraintMethodCollisionModel::Tick(float t)
{
    if (!manualStep)
        Step(t);
    return CollisionObject::Tick(t);
}

int ConstraintMethodCollisionModel::Render3D()
{
    return CollisionObject::Render3D();
}

void ConstraintMethodCollisionModel::SetBody(PhysicsBody* b, int useNodeModel, const char* shapePath)
{
    body = b;
    if (useNodeModel && !shapePath)
        SetModelShape(b->node, 1, 0, 0, 0);
    else
        LoadShape(b->node, shapePath);
    UpdatePlacement();
}

// Slot 11 (0x0043ba70, ret 4; this == the subobject at +0xc): one solver step, see the overview
// above.  Source shapes that retail's bytes require: the refresh and the overlap push-out are two
// separate if statements (one block keeps the CollisionObject base pointer in ebx across both);
// the loop's surfacePoint, probeY and surfaceNormal are declared in the ground-query block,
// not in the loop body
// (in the loop body they share frame slots with the push-out's locals, giving a different
// frame); the offset products carry redundant parentheses (docs/VC6_OPERAND_ORDER.md section 3),
// without which VC6 schedules the argument copies of the ApplyContactImpulse call differently.
int ConstraintMethodCollisionModel::Step(float t)
{
    dt = t;
    if (shapeCollisionEnabled) {
        if (refreshPending) {
            SnapPlacement();
            refreshPending = 0;
        } else {
            UpdatePlacement();
        }
    }
    if (shapeCollisionEnabled && QueryCollisions()) {
        const Vec3* contactOffset = (const Vec3*)contactRecord;
        Vec3 pos;
        body->node->GetPositionIn(0, &pos);
        Vec3 push;
        push.x = contactOffset->x * 1.005f;
        push.y = contactOffset->y * 1.005f;
        push.z = contactOffset->z * 1.005f;
        pos.x -= push.x;
        pos.y -= push.y;
        pos.z -= push.z;
        body->node->SetPositionInFrame(0, &pos);
        UpdatePlacement();
    }
    if (terrain) {
        float maxPenetration = 0.0f;
        int anyHit = 0;
        Vec3 bestNormal;
        Vec3 bestPoint;
        Vec3 surfacePoint;
        float probeY;
        Vec3 surfaceNormal;
        for (int i = 0; i < probeCount; i++) {
            // The probe stores a primary scene-node pointer.
            surfacePoint = probes[i].node->LocalToWorldPoint(probes[i].localPoint);
            probeY = surfacePoint.y;
            terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&surfacePoint),
                                 reinterpret_cast<TerrainVec3*>(&surfaceNormal), 0, 0);
            if (probeY < surfacePoint.y) {
                anyHit = 1;
                probes[i].hit = 1;
                float penetration = surfacePoint.y - probeY;
                if (penetration > maxPenetration) {
                    maxPenetration = penetration;
                    bestNormal = surfaceNormal;
                    bestPoint = surfacePoint;
                }
                probes[i].normal = bestNormal;
            } else {
                probes[i].hit = 0;
            }
        }
        if (anyHit && applyImpulses) {
            float negPenetration = -maxPenetration;
            Vec3 offset;
            offset.x = (bestNormal.x * negPenetration);
            offset.y = (bestNormal.y * negPenetration);
            offset.z = (bestNormal.z * negPenetration);
            ApplyContactImpulse(0, 1.0f, offset, bestPoint, bestNormal);
            Vec3 pos;
            body->node->GetPositionIn(0, &pos);
            Vec3 step;
            step.x = offset.x * 1.005f;
            step.y = offset.y * 1.005f;
            step.z = offset.z * 1.005f;
            pos.x -= step.x;
            pos.y -= step.y;
            pos.z -= step.z;
            body->node->SetPositionInFrame(0, &pos);
            UpdatePlacement();
            hasContact = 1;
        }
    }
    return 1;
}

// ---- ApplyContactImpulse helpers ------------------------------------------------
// Retail expands these at the early sites of 0x0043bdb0; the Vec3 constructor and the
// operators declared above stay out of line (the late sites call them, which
// is the VC6 inline-budget pattern of docs/VC6_INLINE_BUDGET.md).  ConSub builds a named
// result (no constructor call: retail keeps vel - prev on the x87 stack); the other helpers
// return through the out-of-line constructor 0x00404e60 as retail does.
static inline Vec3 ConSub(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// Constructor-built forms (the constructor itself is the out-of-line 0x00404e60).
// Call view of that copy (thiscall, ret 0xc), declared only and bound to 0x00404e60.
struct ConstraintVec3Call : Vec3 {
    ConstraintVec3Call(float x_, float y_, float z_);
};

static inline ConstraintVec3Call ConAdd(const Vec3& a, const Vec3& b)
{
    return ConstraintVec3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline ConstraintVec3Call ConDiff(const Vec3& a, const Vec3& b)
{
    return ConstraintVec3Call(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline ConstraintVec3Call ConScale(const Vec3& v, float s)
{
    return ConstraintVec3Call(v.x * s, v.y * s, v.z * s);
}

static inline Vec3 ConCross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// Row vector times the upper 3x3 (same formula as the out-of-line copy 0x0043ca20).
static inline Vec3 ConRotate(const Vec3& v, const Matrix4& m)
{
    Vec3 r;
    r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
    r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
    r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
    return r;
}

// Row vector times the matrix including the translation row.
static inline Vec3 ConTransform(const Vec3& v, const Matrix4& m)
{
    Vec3 r;
    r.x = (v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0]) + m.m[3][0];
    r.y = (v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1]) + m.m[3][1];
    r.z = (v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]) + m.m[3][2];
    return r;
}

static inline float ConDot(const Vec3& a, const Vec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// ConCross with every product parenthesised (d3dvec's CrossProduct spelling), for vA's
// wA x rA and termA's rl x nl. At wA x rA the extra nodes move the copy of rl (the
// WorldToLocalDirection result) ahead of the cross's subtractions and the vLin add, as retail
// schedules vA = vLin + wA x rA (2471 -> 2499; the form with the z row's second product bare
// gives the same bytes). At rl x nl they move the `test edi, edi` of `if (other)` up into
// termA's rotation, three instructions from retail's position (2499 -> 2515); there every
// form with four or six parenthesised products gives these bytes, none, one, two, three or
// five give 2495..2511.
static inline Vec3 ConCrossParen(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - (a.x * b.z);
    r.z = (a.x * b.y) - (a.y * b.x);
    return r;
}

// ConDot with the y product parenthesised: the extra node makes VC6 accumulate termB's dot
// in the register of the cross product's y, as retail does (fld c.y; fmulp st(2)).
static inline float ConDotGroupedY(const Vec3& a, const Vec3& b)
{
    return (a.x * b.x + (a.y * b.y)) + a.z * b.z;
}

// 0x0043bdb0 (ret 0x2c): impulse response of the body for one contact.  'offset' is
// the penetration offset, 'point' the surface point (the contact point is point + offset),
// 'normal' (n below) the contact normal, 't' the time fraction used to interpolate the
// linear velocity, 'other' the other body or null.  Decoded steps (instruction behaviour tier 1, names tier 3):
//   vLin = prev + (vel - prev) * t; rA = (point + offset) - centre of mass (world);
//   vA = vLin + wA x rA;  vB = other ? vB + wB x rB : 0;  vn = (vA - vB) . n;
//   j = -(1 + e) vn / (1/mA + ((I_A^-1 (rl x nl)) x rl) . nl + 1/mB + ((M_B (rB x n)) x rB) . n)
//   (body A's term in its local frame, body B's in world with the translation row of B's
//   inverse-inertia matrix), scaled by 1 - impulseLoss and clamped to maxImpulse;
//   P = m vLin + j n; L = I w + rl x local(j n); v = P / m;
//   velocity = v (1 - dt damping), clamped to maxSpeed; w = I^-1 L (1 - dt damping).
// The last statement forms v + w' x local(rA) and discards it (retail computes and drops it).
// Retail recomputes local(J) twice into the same local and converts rA to the local frame in
// place (both written that way below); wA is reloaded from slot 43 at the end.
//
// Not exact (2515 of 2619 bytes in place, size exact, 11 differing runs).
// The frame (0xb8) and its slot order match, as do the call sequence and every operand.
// Session 14: termB's dot through ConDotGroupedY (y product parenthesised) makes VC6
// accumulate it in the cross product's y register as retail does (removes the extra
// fstp st(2), the old 2-byte size gap; 1570 -> 2471), and vA's cross through ConCrossParen
// moves the rl copy ahead of the subtractions (2471 -> 2499). What still differs:
//  - the copy of the LocalToWorldPoint result is interleaved with the point + offset adds
//    differently (0x43beb4..0x43bf02);
//  - the rl copy's last two dwords and one fsubp of the wA x rA cross (0x43bff5);
//  - one dword move against an fld in the 'other' path (0x43c1ce);
//  - the `test edi, edi` of `if (other)`: retail issues it inside the termA rotation, three
//    instructions before the candidate (rl x nl through ConCrossParen; it was eight);
//  - one fld/fmul st(2) pair of termB's cross that VC6 forms as fld st(1); fmul (0x43c3ea).
// Kept: point = point + offset (offset's components load first), vn = normal . vRel, the
// zero vector copied as a whole struct (needed for the frame: component copies put vB in the
// wrong slot).
void ConstraintMethodCollisionModel::ApplyContactImpulse(PhysicsBody* other, float t,
                                                         Vec3 offset, Vec3 point, Vec3 normal)
{
    Vec3 wA = body->GetWorldAngularVelocity();
    Vec3 vLin = ConAdd(ConScale(ConSub(body->velocity, body->prevVelocity), t), body->prevVelocity);
    Vec3 rA = body->node->LocalToWorldPoint(body->centerOfMass);
    point = ConAdd(point, offset);
    rA = ConDiff(point, rA);
    contactPoint = point;
    Vec3 rl = body->node->WorldToLocalDirection(rA);
    Vec3 nl = body->node->WorldToLocalDirection(normal);
    Vec3 vA = ConAdd(vLin, ConCrossParen(wA, rA));
    Vec3 vB;
    Vec3 rB;
    if (!other) {
        vB = g_ConZero;
    } else {
        Vec3 vbLin = other->velocity;
        Vec3 wB = other->GetWorldAngularVelocity();
        other->node->GetPositionIn(0, &rB);
        rB = ConDiff(point, rB);
        vB = ConAdd(vbLin, ConCross(wB, rB));
    }
    Vec3 vRel = ConDiff(vA, vB);
    float num = -((restitution + 1.0f) * ConDot(normal, vRel));

    float invMassA = 1.0f / body->mass;
    Vec3 axisA = ConRotate(ConCrossParen(rl, nl), body->invInertia);
    float termA = ConDot(ConCross(axisA, rl), nl);
    float invMassB = 0.0f;
    float termB = 0.0f;
    if (other) {
        invMassB = 1.0f / other->mass;
        Vec3 axisB = ConTransform(ConCross(rB, normal), other->invInertia);
        termB = ConDotGroupedY(ConCross(axisB, rB), normal);
    }
    float j = num / (invMassA + termA + invMassB + termB);
    j *= 1.0f - impulseLoss;
    if (maxImpulse != 0 && j > maxImpulse)
        j = maxImpulse;

    Vec3 J = ConScale(normal, j);
    Vec3 P = vLin * body->mass;
    Vec3 L = RotateVector(body->angularVelocity, body->inertia);
    static_cast<Vec3AddAssignCall&>(P) += J;   // 0x00428060
    rA = body->node->WorldToLocalDirection(rA);
    Vec3 Jl = body->node->WorldToLocalDirection(J);
    Jl = body->node->WorldToLocalDirection(J);
    static_cast<Vec3AddAssignCall&>(L) += CrossProduct(rl, Jl);
    Vec3 v = P / body->mass;
    body->velocity = v * (1.0f - dt * damping);
    if (maxSpeed != 0 && Magnitude(body->velocity) > maxSpeed)
        body->velocity = maxSpeed * Vec3Normalize(body->velocity);
    body->angularVelocity = RotateVector(L, body->invInertia) * (1.0f - dt * damping);
    wA = body->GetWorldAngularVelocity();
    Vec3 vPoint = v + CrossProduct(wA, rA);
}

void ConstraintMethodCollisionModel::AddProbePoint(Vec3 point, SoultreeObject* node)
{
    probes = (ConstraintProbe*)DebugRealloc(probes, (probeCount + 1) * sizeof(ConstraintProbe), __FILE__, 331);
    probes[probeCount].node = node;
    probes[probeCount].localPoint = point;
    probes[probeCount].hit = 0;
    probeCount++;
}
