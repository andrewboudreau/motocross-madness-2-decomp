// ConstraintMethodCollisionModel.cpp -- reconstruction of the retail translation unit
// "ConstraintMethodCollisionModel.cpp" (__FILE__ xref at 0x0043c7f9). Region 0x0043b320..0x0043c8e0.
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
//   Other content of this TU: static pixel blend helpers (0x43b320..0x43b660, PixelBlend.cpp),
//   the contact callback 0x43b800, and the TU initializer 0x43c8e0 (zeroes the Vec3 at 0x579830).
//
#include "ConstraintMethodCollisionModel.h"

#include "../common/DebugAlloc.h"   // DebugRealloc

// The two callbacks slot 8 installs (CollisionObject.h, CollisionCallback).
void ConstraintContactCallback(CollisionObject* self, CollisionObject* other);  // 0x0043b800, below
void CollisionEmptyCallback(CollisionObject* self, CollisionObject* other);     // 0x00464e90, shared empty function

ConstraintMethodCollisionModel::ConstraintMethodCollisionModel(int a) : CollisionObject(a)
{
    // Statement order found by a placement search against the retail store schedule
    // (0x0043b8d0); the order of these independent stores has no semantic meaning.
    field_0xbc = 0.9f;
    field_0xec = 1;
    field_0xb8 = 0;
    field_0x108 = 1;
    body = 0;
    probeCount = 0;
    groundQuery = 0;
    probes = 0;
    field_0xe8 = 0;
    damping = 0;
    field_0xf4 = 0;
    maxImpulse = 0;
    maxSpeed = 0;
}

void ConstraintMethodCollisionModel::UnknownVirtualSlot2()
{
}

ConstraintMethodCollisionModel::~ConstraintMethodCollisionModel()
{
}

GameObject* ConstraintMethodCollisionModel::GameObjectVirtualSlot8(int a)
{
    Fn_004320f0(a, 1, 1, 1);
    field_0x88 = ConstraintContactCallback;
    field_0x8c = CollisionEmptyCallback;
    field_0x60 = this;
    return this;
}

int ConstraintMethodCollisionModel::GameObjectVirtualSlot10(float t)
{
    if (!field_0xe8)
        GameObjectVirtualSlot11(t);
    return CollisionObject::GameObjectVirtualSlot10(t);
}

int ConstraintMethodCollisionModel::GameObjectVirtualSlot14()
{
    return CollisionObject::GameObjectVirtualSlot14();
}

void ConstraintMethodCollisionModel::SetBody(ConBody* b, int useNodeModel, const char* arg)
{
    body = b;
    if (useNodeModel && !arg)
        Fn_004324b0(b->node, 1, 0, 0, 0);
    else
        Fn_00432800(b->node, arg);
    Fn_00435fb0();
}

void ConstraintMethodCollisionModel::AddProbePoint(ConVec3 point, ConNode* node)
{
    probes = (ConstraintProbe*)DebugRealloc(probes, (probeCount + 1) * sizeof(ConstraintProbe), __FILE__, 331);
    probes[probeCount].node = node;
    probes[probeCount].localPoint = point;
    probes[probeCount].hit = 0;
    probeCount++;
}

// ---- Slot 11 / contact solver --------------------------------------------------


static inline float Dot3(const ConVec3& a, const ConVec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

int ConstraintMethodCollisionModel::GameObjectVirtualSlot11(float t)
{
    dt = t;
    if (field_0xec) {
        if (field_0xc8) {
            Fn_00435fe0();
            field_0xc8 = 0;
        } else {
            Fn_00435fb0();
        }
        if (field_0xec && Fn_00438e70()) {
            const ConVec3* n = (const ConVec3*)field_0x5c;
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
            Fn_00435fb0();
        }
    }
    if (groundQuery) {
        float maxPen = 0.0f;
        int anyHit = 0;
        ConVec3 bestNormal;
        ConVec3 bestPoint;
        for (int i = 0; i < probeCount; i++) {
            ConVec3 p = probes[i].node->LocalToWorldPoint(probes[i].localPoint);
            float probeY = p.y;
            ConVec3 n;
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
        if (anyHit && field_0x108) {
            float d = -maxPen;
            ConVec3 offset;
            offset.x = bestNormal.x * d;
            offset.y = bestNormal.y * d;
            offset.z = bestNormal.z * d;
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
            Fn_00435fb0();
            field_0x58 = 1;
        }
    }
    return 1;
}

// TU-private zero vector at 0x00579830; the dynamic initializer 0x0043c8e0 builds it
// from an inline three-float constructor (tier 2: only the stores are visible).
struct ConZeroVec {
    float x, y, z;
    ConZeroVec(float a, float b, float c) : x(a), y(b), z(c) {}
};
static ConZeroVec g_ConZero(0.0f, 0.0f, 0.0f);

void ConstraintMethodCollisionModel::ApplyContactImpulse(ConBody* other, float t,
                                                         ConVec3 a, ConVec3 b, ConVec3 c)
{
    ConVec3 wA = body->GetWorldAngularVelocity();
    ConVec3 vLin(body->prevVelocity.x + (body->velocity.x - body->prevVelocity.x) * t,
                 body->prevVelocity.y + (body->velocity.y - body->prevVelocity.y) * t,
                 body->prevVelocity.z + (body->velocity.z - body->prevVelocity.z) * t);
    ConVec3 cA = body->node->LocalToWorldPoint(body->centerOfMass);
    b = a + b;
    contactPoint = b;
    ConVec3 rA(b.x - cA.x, b.y - cA.y, b.z - cA.z);
    ConVec3 vA = vLin + CrossProduct(wA, rA);
    ConVec3 vB(g_ConZero.x, g_ConZero.y, g_ConZero.z);
    ConVec3 rB;
    if (other) {
        ConVec3 posB;
        other->node->GetPositionIn(0, &posB);
        rB = ConVec3(b.x - posB.x, b.y - posB.y, b.z - posB.z);
        ConVec3 wB = other->GetWorldAngularVelocity();
        vB = other->velocity + CrossProduct(wB, rB);
    }
    ConVec3 vRel(vA.x - vB.x, vA.y - vB.y, vA.z - vB.z);
    float vn = Dot3(vRel, c);
    float num = -(1.0f + field_0xb8) * vn;

    ConVec3 nl = body->node->WorldToLocalDirection(c);
    ConVec3 rl = body->node->WorldToLocalDirection(rA);
    ConVec3 axisA = RotateVector(CrossProduct(rl, nl), body->invInertia);
    float denom = body->invMass + Dot3(nl, CrossProduct(axisA, rl));
    if (other) {
        ConVec3 nlB = other->node->WorldToLocalDirection(c);
        ConVec3 rlB = other->node->WorldToLocalDirection(rB);
        ConVec3 axisB = RotateVector(CrossProduct(rlB, nlB), other->invInertia);
        denom += other->invMass + Dot3(nlB, CrossProduct(axisB, rlB));
    }
    float j = num / denom;
    j *= 1.0f - field_0xf4;
    if (maxImpulse != 0 && j > maxImpulse)
        j = maxImpulse;

    ConVec3 J = c * j;
    ConVec3 P = vLin * body->mass + J;
    body->velocity = (P / body->mass) * (1.0f - dt * damping);

    ConVec3 Lang = RotateVector(body->angularVelocity, body->inertia)
                 + CrossProduct(rl, body->node->WorldToLocalDirection(J));
    body->angularVelocity = RotateVector(Lang, body->invInertia) * (1.0f - dt * damping);

    if (maxSpeed != 0 && Magnitude(body->velocity) > maxSpeed)
        body->velocity = maxSpeed * Normalize(body->velocity);
}

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
    ConContactRecord* rec = (ConContactRecord*)a->field_0x5c;
    if (b->field_0x64 == 0x3ea)
        a->ApplyContactImpulse(*(ConBody**)((char*)b + 0xc4), rec->t, rec->v0, rec->v18, rec->v0c);
    else
        a->ApplyContactImpulse(0, rec->t, rec->v0, rec->v18, rec->v0c);
}
