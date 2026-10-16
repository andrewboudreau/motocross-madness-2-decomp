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
//   Structure of one step (GameObjectVirtualSlot11 at 0x0043ba70, called every tick by slot 10):
//     1. Save dt (+0xc0). If the model is active (+0xec) refresh the collision node
//        (0x435fb0/0x435fe0); when the base reports an overlap (0x438e70) push the body
//        back along the recorded contact normal by 1.005 (projection) and refresh again.
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
//   scales j by (1 - field_0xf4), clamps to maxImpulse (+0xf8, 0 = unlimited), updates ONLY body A's linear
//   velocity (P' = m*v + j*n, damped by 1 - dt*damping) and angular momentum (L' = I*w + rA x J),
//   and clamps the speed to maxSpeed (+0xfc). Body B (the other party, may be null) only
//   contributes to the denominator and to the relative velocity (null -> zero vector 0x579830).
//   Other content of this TU: the contact callback 0x43b800 and the four kVec3 initializer pairs
//   0x43c8e0..0x43ca1b.  The pixel blend helpers just before it (0x43b320..0x43b7f9,
//   samples/physics/constraint/PixelBlend.cpp) have no literal tying them to this unit.
//
#include "ConstraintMethodCollisionModel.h"

#include "core/DebugAlloc.h"   // DebugRealloc

// The two callbacks slot 8 installs (CollisionObject.h, CollisionCallback).
void ConstraintContactCallback(CollisionObject* self, CollisionObject* other);  // 0x0043b800, below
void CollisionEmptyCallback(CollisionObject* self, CollisionObject* other);     // 0x00464e90, shared empty function

ConstraintMethodCollisionModel::ConstraintMethodCollisionModel(int a) : CollisionObject(a)
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

GameObject* ConstraintMethodCollisionModel::SetRenderContext(int a)
{
    Configure(a, 1, 1, 1);
    onHitCallback = ConstraintContactCallback;
    onHitByCallback = CollisionEmptyCallback;
    ownerObject = this;
    return this;
}

int ConstraintMethodCollisionModel::Tick(float t)
{
    if (!manualStep)
        GameObjectVirtualSlot11(t);
    return CollisionObject::Tick(t);
}

int ConstraintMethodCollisionModel::GameObjectVirtualSlot14()
{
    return CollisionObject::GameObjectVirtualSlot14();
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
// the loop's p, probeY and n are declared in the ground-query block, not in the loop body
// (in the loop body they share frame slots with the push-out's locals, giving a different
// frame); the offset products carry redundant parentheses (docs/VC6_OPERAND_ORDER.md section 3),
// without which VC6 schedules the argument copies of the ApplyContactImpulse call differently.
int ConstraintMethodCollisionModel::GameObjectVirtualSlot11(float t)
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
        const ConVec3* n = (const ConVec3*)contactRecord;
        ConVec3 pos;
        body->node->GetPositionIn(0, &pos);
        ConVec3 d;
        d.x = n->x * 1.005f;
        d.y = n->y * 1.005f;
        d.z = n->z * 1.005f;
        pos.x -= d.x;
        pos.y -= d.y;
        pos.z -= d.z;
        body->node->SetPositionIn(0, &pos);
        UpdatePlacement();
    }
    if (groundQuery) {
        float maxPen = 0.0f;
        int anyHit = 0;
        ConVec3 bestNormal;
        ConVec3 bestPoint;
        ConVec3 p;
        float probeY;
        ConVec3 n;
        for (int i = 0; i < probeCount; i++) {
            p = probes[i].node->LocalToWorldPoint(probes[i].localPoint);
            probeY = p.y;
            groundQuery->QueryPoint(&p, &n, 0, 0);
            if (probeY < p.y) {
                anyHit = 1;
                probes[i].hit = 1;
                float pen = p.y - probeY;
                if (pen > maxPen) {
                    maxPen = pen;
                    bestNormal = n;
                    bestPoint = p;
                }
                probes[i].normal = bestNormal;
            } else {
                probes[i].hit = 0;
            }
        }
        if (anyHit && applyImpulses) {
            float d = -maxPen;
            ConVec3 offset;
            offset.x = (bestNormal.x * d);
            offset.y = (bestNormal.y * d);
            offset.z = (bestNormal.z * d);
            ApplyContactImpulse(0, 1.0f, offset, bestPoint, bestNormal);
            ConVec3 pos;
            body->node->GetPositionIn(0, &pos);
            ConVec3 step;
            step.x = offset.x * 1.005f;
            step.y = offset.y * 1.005f;
            step.z = offset.z * 1.005f;
            pos.x -= step.x;
            pos.y -= step.y;
            pos.z -= step.z;
            body->node->SetPositionIn(0, &pos);
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
// constraint model, b the other collision object. The contact record at a->field_0x5c
// holds three Vec3 (+0x00, +0x0c, +0x18) and a float (+0x24); tier 3 semantics.
struct ConContactRecord {
    ConVec3 v0;       // +0x00
    ConVec3 v0c;      // +0x0c (normal, passed as the last argument)
    ConVec3 v18;      // +0x18
    float t;          // +0x24 time fraction
};

void ConstraintContactCallback(CollisionObject* self, CollisionObject* b)
{
    ConstraintMethodCollisionModel* a = (ConstraintMethodCollisionModel*)self;
    ConContactRecord* rec = (ConContactRecord*)a->contactRecord;
    if (b->ownerType == 0x3ea)
        a->ApplyContactImpulse(*(ConBody**)((char*)b + 0xc4), rec->t, rec->v0, rec->v18, rec->v0c);
    else
        a->ApplyContactImpulse(0, rec->t, rec->v0, rec->v18, rec->v0c);
}
