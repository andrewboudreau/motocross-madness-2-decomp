// With ConstraintMethodCollisionModel.bindings.json all 19 targets of this file
// (src/krusty2/constraint/targets.json) pass `tools/run_physics_samples.py --strict`.
// ConstraintMethodCollisionModel.cpp -- reconstruction of the retail translation unit
// "ConstraintMethodCollisionModel.cpp" (__FILE__ xref at 0x0043c7f9). Unit extent
// 0x0043b800..0x0043ca20 plus the out-of-line vector helpers 0x0043c890 / 0x0043ca20 it calls.
// ApplyContactImpulse (0x0043bdb0) is a near miss kept in
// samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp, which includes
// this file.
//
// SOLVER OVERVIEW (semantic names are tier 3; instruction behaviour is tier 1)
//   Method: single-contact IMPULSE (velocity-level rigid body response) plus a small
//   PROJECTION step. There is no penalty force and no iterative solver in this file.
//   Structure of one step (UnknownVirtualSlot11 at 0x0043ba70, called every tick by slot 10):
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
//   samples/physics/constraint/PixelBlend.cpp) have no literal tying them to this unit.
//
#include "ConstraintMethodCollisionModel.h"
#include "math/Vec3.h"
#include "../../reconstructed/Matrix4.h"
#include "core/SoultreeObject.h"

#include "core/DebugAlloc.h"   // DebugRealloc

// The two callbacks slot 8 installs (CollisionObject.h, CollisionCallback).
void ConstraintContactCallback(CollisionObject* self, CollisionObject* other);  // 0x0043b800, below
void CollisionEmptyCallback(CollisionObject* self, CollisionObject* other);     // 0x00464e90, shared empty function

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
    groundQuery = 0;
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
        UnknownVirtualSlot11(t);
    return CollisionObject::Tick(t);
}

int ConstraintMethodCollisionModel::UnknownVirtualSlot14()
{
    return CollisionObject::UnknownVirtualSlot14();
}

void ConstraintMethodCollisionModel::SetBody(ConBody* b, int useNodeModel, const char* arg)
{
    body = b;
    if (useNodeModel && !arg)
        SetModelShape(b->node, 1, 0, 0, 0);
    else
        LoadShape(b->node, arg);
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
int ConstraintMethodCollisionModel::UnknownVirtualSlot11(float t)
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
        const ConVec3* contactOffset = (const ConVec3*)contactRecord;
        ConVec3 pos;
        reinterpret_cast<SoultreeObject*>(body->node)->GetPositionIn(
            0, reinterpret_cast<Vec3*>(&pos));
        ConVec3 push;
        push.x = contactOffset->x * 1.005f;
        push.y = contactOffset->y * 1.005f;
        push.z = contactOffset->z * 1.005f;
        pos.x -= push.x;
        pos.y -= push.y;
        pos.z -= push.z;
        reinterpret_cast<SoultreeObject*>(body->node)->SetPositionInFrame(
            0, reinterpret_cast<const Vec3*>(&pos));
        UpdatePlacement();
    }
    if (groundQuery) {
        float maxPenetration = 0.0f;
        int anyHit = 0;
        ConVec3 bestNormal;
        ConVec3 bestPoint;
        ConVec3 surfacePoint;
        float probeY;
        ConVec3 surfaceNormal;
        for (int i = 0; i < probeCount; i++) {
            // The probe stores a primary scene-node pointer. Its returned
            // xyz value remains alive through this assignment's full expression.
            surfacePoint = reinterpret_cast<const ConVec3&>(static_cast<const Vec3&>(
                reinterpret_cast<SoultreeObject*>(probes[i].node)->LocalToWorldPoint(
                    reinterpret_cast<const Vec3&>(probes[i].localPoint))));
            probeY = surfacePoint.y;
            groundQuery->QueryPoint(&surfacePoint, &surfaceNormal, 0, 0);
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
            ConVec3 offset;
            offset.x = (bestNormal.x * negPenetration);
            offset.y = (bestNormal.y * negPenetration);
            offset.z = (bestNormal.z * negPenetration);
            ApplyContactImpulse(0, 1.0f, offset, bestPoint, bestNormal);
            ConVec3 pos;
            reinterpret_cast<SoultreeObject*>(body->node)->GetPositionIn(
                0, reinterpret_cast<Vec3*>(&pos));
            ConVec3 step;
            step.x = offset.x * 1.005f;
            step.y = offset.y * 1.005f;
            step.z = offset.z * 1.005f;
            pos.x -= step.x;
            pos.y -= step.y;
            pos.z -= step.z;
            reinterpret_cast<SoultreeObject*>(body->node)->SetPositionInFrame(
                0, reinterpret_cast<const Vec3*>(&pos));
            UpdatePlacement();
            hasContact = 1;
        }
    }
    return 1;
}

void ConstraintMethodCollisionModel::AddProbePoint(ConVec3 point, ConNode* node)
{
    probes = (ConstraintProbe*)DebugRealloc(probes, (probeCount + 1) * sizeof(ConstraintProbe), __FILE__, 331);
    probes[probeCount].node = node;
    probes[probeCount].localPoint = point;
    probes[probeCount].hit = 0;
    probeCount++;
}

// TU-private vector constants, .CRT$XCU entries 66-69: zero 0x00579830 (thunk 0x0043c8e0,
// body 0x0043c8f0), x axis 0x00579840 (0x0043c930/0x0043c940), y axis 0x00579850
// (0x0043c980/0x0043c990) and z axis 0x00579820 (0x0043c9d0/0x0043c9e0).  Each 60-byte body
// builds the value in a stack temporary and copies it out, which is the shape VC6 gives
// copy-initialization from a constructor call (direct initialization stores the floats
// straight into the global).  Only the zero vector has a reader: ApplyContactImpulse copies it
// as a whole ConVec3 (three dword moves), which is why the type derives from ConVec3.
struct ConStaticVec3 : ConVec3 {
    ConStaticVec3(float a, float b, float c) { x = a; y = b; z = c; }
};
static const ConStaticVec3 g_ConZero = ConStaticVec3(0.0f, 0.0f, 0.0f);
static const ConStaticVec3 g_ConXAxis = ConStaticVec3(1.0f, 0.0f, 0.0f);
static const ConStaticVec3 g_ConYAxis = ConStaticVec3(0.0f, 1.0f, 0.0f);
static const ConStaticVec3 g_ConZAxis = ConStaticVec3(0.0f, 0.0f, 1.0f);

// ---- Contact callback (0x0043b800) ----------------------------------------------
// Stored into field_0x88 by slot 8 (with 0x464e90 in field_0x8c). cdecl; self is the
// constraint model, b the other collision object. The contact record at a->contactRecord
// (+0x5c) is the CollisionSweepQuery of CollisionShapeTests.h viewed with a float fraction;
// tier 3 semantics.
struct ConContactRecord {
    ConVec3 contactOffset;  // +0x00 passed as ApplyContactImpulse's offset
    ConVec3 contactNormal;  // +0x0c passed as the normal (last argument)
    ConVec3 contact;        // +0x18 passed as the base point
    float t;                // +0x24 time fraction
};

void ConstraintContactCallback(CollisionObject* self, CollisionObject* b)
{
    ConstraintMethodCollisionModel* a = (ConstraintMethodCollisionModel*)self;
    ConContactRecord* rec = (ConContactRecord*)a->contactRecord;
    if (b->ownerType == 0x3ea)
        a->ApplyContactImpulse(*(ConBody**)((char*)b + 0xc4), rec->t, rec->contactOffset, rec->contact, rec->contactNormal);
    else
        a->ApplyContactImpulse(0, rec->t, rec->contactOffset, rec->contact, rec->contactNormal);
}
