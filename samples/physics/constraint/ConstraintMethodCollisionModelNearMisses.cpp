// Near-miss candidates of D:\aardvark\VC\krusty2\ConstraintMethodCollisionModel.cpp, kept out of
// src/krusty2/constraint/ConstraintMethodCollisionModel.cpp until they match.  The canonical
// file is included first so the TU-local kVec3 set and types are the same.  Each function's
// note records what still differs; the scores are in targets.json and docs/NEAR_MISS_INDEX.md.
#include "constraint/ConstraintMethodCollisionModel.cpp"

// ---- ApplyContactImpulse helpers ------------------------------------------------
// Retail expands these at the early sites of 0x0043bdb0; the ConVec3 constructor and the
// operators declared in ConstraintTypes.h stay out of line (the late sites call them, which
// is the VC6 inline-budget pattern of docs/VC6_INLINE_BUDGET.md).  ConSub builds a named
// result (no constructor call: retail keeps vel - prev on the x87 stack); the other helpers
// return through the out-of-line constructor 0x00404e60 as retail does.
static inline ConVec3 ConSub(const ConVec3& a, const ConVec3& b)
{
    ConVec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// Constructor-built forms (the constructor itself is the out-of-line 0x00404e60).
static inline ConVec3 ConAdd(const ConVec3& a, const ConVec3& b)
{
    return ConVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline ConVec3 ConDiff(const ConVec3& a, const ConVec3& b)
{
    return ConVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline ConVec3 ConScale(const ConVec3& v, float s)
{
    return ConVec3(v.x * s, v.y * s, v.z * s);
}

static inline ConVec3 ConCross(const ConVec3& a, const ConVec3& b)
{
    ConVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// Row vector times the upper 3x3 (same formula as the out-of-line copy 0x0043ca20).
static inline ConVec3 ConRotate(const ConVec3& v, const ConMatrix& m)
{
    ConVec3 r;
    r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
    r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
    r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
    return r;
}

// Row vector times the matrix including the translation row.
static inline ConVec3 ConTransform(const ConVec3& v, const ConMatrix& m)
{
    ConVec3 r;
    r.x = (v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0]) + m.m[3][0];
    r.y = (v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1]) + m.m[3][1];
    r.z = (v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]) + m.m[3][2];
    return r;
}

static inline float ConDot(const ConVec3& a, const ConVec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// 0x0043bdb0 (ret 0x2c): impulse response of the body for one contact.  'a' is the
// penetration offset, 'b' the surface point (the contact point is a + b), 'c' the contact
// normal, 't' the time fraction used to interpolate the linear velocity, 'other' the other
// body or null.  Decoded steps (instruction behaviour tier 1, names tier 3):
//   vLin = prev + (vel - prev) * t; rA = (a + b) - centre of mass (world);
//   vA = vLin + wA x rA;  vB = other ? vB + wB x rB : 0;  vn = (vA - vB) . c;
//   j = -(1 + e) vn / (1/mA + ((I_A^-1 (rl x nl)) x rl) . nl + 1/mB + ((M_B (rB x c)) x rB) . c)
//   (body A's term in its local frame, body B's in world with the translation row of B's
//   inverse-inertia matrix), scaled by 1 - impulseLoss and clamped to maxImpulse;
//   P = m vLin + j c; L = I w + rl x local(j c); v = P / m;
//   velocity = v (1 - dt damping), clamped to maxSpeed; w = I^-1 L (1 - dt damping).
// The last statement forms v + w' x local(rA) and discards it (retail computes and drops it).
// Retail recomputes local(J) twice into the same local and converts rA to the local frame in
// place (both written that way below); wA is reloaded from slot 43 at the end.
//
// NEAR MISS: 1570/2619 strict positions (candidate 2621 bytes), instruction alignment 0.962.
// The frame (0xb8) and its slot order match, as do the call sequence and every operand.
// What still differs:
//  - termB (the 'other' branch): retail forms the x term of (axisB x rB) as axisB.y first
//    (fld [axis.y]; fmul rB.z; fld st(1); fmul rB.y) and accumulates the dot product in the
//    register of the cross product's y (fld c.y; fmulp st(2), sum order y, x, z); VC6 here
//    loads rB first and sums x, y, z in fresh registers, which ends the branch with an extra
//    fstp st(2) (the 2-byte size difference shifts everything after 0x43c68b).
//  - scheduling only: the copy of the LocalToWorldPoint result is interleaved with the a + b
//    adds differently, the nl copy and one push around the wA x rA cross product, one dword
//    move in the 'other' path and the `test edi, edi` of `if (other)` (retail issues it
//    inside the termA rotation).
// Forms tried without reaching those (about 35): dot product as by-reference or by-value
// helper with every grouping of the three terms (x+y)+z, flat, z+(x+y), (y+x)+z, x+(y+z),
// (x+z)+y, y+(x+z), both argument orders, written out with a named cross local; termB through
// a helper function (exceeds the inline budget: the transform is then called out of line);
// the declaration/initialisation order of invMassB/termB and the denominator grouping; axisB
// as a declared-then-assigned local or folded into one expression; rA as a separate cA local
// (wrong frame); vB declared at other positions; parenthesised products in the cross product
// (Math3D's form also exceeds the inline budget: J's scale is then called out of line).
// Session 12: the cross product's term order and operand order and the dot product's grouping
// and argument order (32 combinations through separate helpers) all compile to the same bytes:
// VC6 canonicalises them here, so the termB difference is not an operand-order choice.
// Also identical: termB's transform or either cross computed into float locals before the
// result is filled (the store-schedule lever of docs/VC6_OPERAND_ORDER.md 1.3); with both
// crosses and the transform in that form the inline budget runs out (ConScale is called).
// Kept: b = b + a (a's components load first), vn = c . vRel, the zero vector copied as a
// whole struct (needed for the frame: component copies put vB in the wrong slot).
void ConstraintMethodCollisionModel::ApplyContactImpulse(ConBody* other, float t,
                                                         ConVec3 a, ConVec3 b, ConVec3 c)
{
    ConVec3 wA = body->GetWorldAngularVelocity();
    ConVec3 vLin = ConAdd(ConScale(ConSub(body->velocity, body->prevVelocity), t), body->prevVelocity);
    ConVec3 rA = body->node->LocalToWorldPoint(body->centerOfMass);
    b = ConAdd(b, a);
    rA = ConDiff(b, rA);
    contactPoint = b;
    ConVec3 rl = body->node->WorldToLocalDirection(rA);
    ConVec3 nl = body->node->WorldToLocalDirection(c);
    ConVec3 vA = ConAdd(vLin, ConCross(wA, rA));
    ConVec3 vB;
    ConVec3 rB;
    if (!other) {
        vB = g_ConZero;
    } else {
        ConVec3 vbLin = other->velocity;
        ConVec3 wB = other->GetWorldAngularVelocity();
        other->node->GetPositionIn(0, &rB);
        rB = ConDiff(b, rB);
        vB = ConAdd(vbLin, ConCross(wB, rB));
    }
    ConVec3 vRel = ConDiff(vA, vB);
    float num = -((restitution + 1.0f) * ConDot(c, vRel));

    float invMassA = 1.0f / body->mass;
    ConVec3 axisA = ConRotate(ConCross(rl, nl), body->invInertia);
    float termA = ConDot(ConCross(axisA, rl), nl);
    float invMassB = 0.0f;
    float termB = 0.0f;
    if (other) {
        invMassB = 1.0f / other->mass;
        ConVec3 axisB = ConTransform(ConCross(rB, c), other->invInertia);
        termB = ConDot(ConCross(axisB, rB), c);
    }
    float j = num / (invMassA + termA + invMassB + termB);
    j *= 1.0f - impulseLoss;
    if (maxImpulse != 0 && j > maxImpulse)
        j = maxImpulse;

    ConVec3 J = ConScale(c, j);
    ConVec3 P = vLin * body->mass;
    ConVec3 L = RotateVector(body->angularVelocity, body->inertia);
    P += J;
    rA = body->node->WorldToLocalDirection(rA);
    ConVec3 Jl = body->node->WorldToLocalDirection(J);
    Jl = body->node->WorldToLocalDirection(J);
    L += CrossProduct(rl, Jl);
    ConVec3 v = P / body->mass;
    body->velocity = v * (1.0f - dt * damping);
    if (maxSpeed != 0 && Magnitude(body->velocity) > maxSpeed)
        body->velocity = maxSpeed * Normalize(body->velocity);
    body->angularVelocity = RotateVector(L, body->invInertia) * (1.0f - dt * damping);
    wA = body->GetWorldAngularVelocity();
    ConVec3 vPoint = v + CrossProduct(wA, rA);
}
