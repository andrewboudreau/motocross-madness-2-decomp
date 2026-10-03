// CollisionShapeTests.cpp -- narrow-phase shape tests between CollisionObject payloads.
//
// Translation-unit ownership is PROVISIONAL (tier 3): the file split is chosen for
// readability.  Retail has no source string in 0x00436100-0x004389b0 (nearest __FILE__ is
// CollisionPoint.cpp at 0x0043a346), so these functions may belong to
// CollisionObject.cpp, CollisionCharacter.cpp or a separate shape-test unit.
#include "CollisionShapeTests.h"

// 0x00436500 (cdecl, void): out = b * a in the row-vector convention, i.e.
// out[i][j] = sum_k b[i][k] * a[k][j].  Operand naming is tier 3.
void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b)
{
    out->m[0][0] = b->m[0][0] * a->m[0][0] + b->m[0][1] * a->m[1][0] +
                   b->m[0][2] * a->m[2][0] + b->m[0][3] * a->m[3][0];
    out->m[0][1] = b->m[0][0] * a->m[0][1] + b->m[0][1] * a->m[1][1] +
                   b->m[0][2] * a->m[2][1] + b->m[0][3] * a->m[3][1];
    out->m[0][2] = b->m[0][0] * a->m[0][2] + b->m[0][1] * a->m[1][2] +
                   b->m[0][2] * a->m[2][2] + b->m[0][3] * a->m[3][2];
    out->m[0][3] = b->m[0][0] * a->m[0][3] + b->m[0][1] * a->m[1][3] +
                   b->m[0][2] * a->m[2][3] + b->m[0][3] * a->m[3][3];
    out->m[1][0] = b->m[1][0] * a->m[0][0] + b->m[1][1] * a->m[1][0] +
                   b->m[1][2] * a->m[2][0] + b->m[1][3] * a->m[3][0];
    out->m[1][1] = b->m[1][0] * a->m[0][1] + b->m[1][1] * a->m[1][1] +
                   b->m[1][2] * a->m[2][1] + b->m[1][3] * a->m[3][1];
    out->m[1][2] = b->m[1][0] * a->m[0][2] + b->m[1][1] * a->m[1][2] +
                   b->m[1][2] * a->m[2][2] + b->m[1][3] * a->m[3][2];
    out->m[1][3] = b->m[1][0] * a->m[0][3] + b->m[1][1] * a->m[1][3] +
                   b->m[1][2] * a->m[2][3] + b->m[1][3] * a->m[3][3];
    out->m[2][0] = b->m[2][0] * a->m[0][0] + b->m[2][1] * a->m[1][0] +
                   b->m[2][2] * a->m[2][0] + b->m[2][3] * a->m[3][0];
    out->m[2][1] = b->m[2][0] * a->m[0][1] + b->m[2][1] * a->m[1][1] +
                   b->m[2][2] * a->m[2][1] + b->m[2][3] * a->m[3][1];
    out->m[2][2] = b->m[2][0] * a->m[0][2] + b->m[2][1] * a->m[1][2] +
                   b->m[2][2] * a->m[2][2] + b->m[2][3] * a->m[3][2];
    out->m[2][3] = b->m[2][0] * a->m[0][3] + b->m[2][1] * a->m[1][3] +
                   b->m[2][2] * a->m[2][3] + b->m[2][3] * a->m[3][3];
    out->m[3][0] = b->m[3][0] * a->m[0][0] + b->m[3][1] * a->m[1][0] +
                   b->m[3][2] * a->m[2][0] + b->m[3][3] * a->m[3][0];
    out->m[3][1] = b->m[3][0] * a->m[0][1] + b->m[3][1] * a->m[1][1] +
                   b->m[3][2] * a->m[2][1] + b->m[3][3] * a->m[3][1];
    out->m[3][2] = b->m[3][0] * a->m[0][2] + b->m[3][1] * a->m[1][2] +
                   b->m[3][2] * a->m[2][2] + b->m[3][3] * a->m[3][2];
    out->m[3][3] = b->m[3][0] * a->m[0][3] + b->m[3][1] * a->m[1][3] +
                   b->m[3][2] * a->m[2][3] + b->m[3][3] * a->m[3][3];
}

// 0x00436430: can this object collide with `other`, and if so run the shape test.
// Filter logic (decoded): other must be a different object with +0x70 set and bit 0 of the
// byte at +0x31; then each side's list (ptr array +0x78, count +0x7c, mode +0x74) is
// consulted: a listed partner is rejected when mode == 1, an unlisted partner is rejected
// when mode == 0.  The survivor dispatches on this->field_0x50 to 0x438b90/0x438c10/0x438c90.
int CollisionObject::TestAgainst(CollisionObject* other)
{
    if (other == this)
        return 0;
    if (other->collidable == 0)
        return 0;
    if ((other->field_0x1c[9] & 1) == 0)
        return 0;

    // The partner list is re-read inside the loop (retail loads +0x78 after the count
    // test), and a found flag rather than an index test decides the mode check.
    if (ignoreCount > 0) {
        int found = 0;
        for (int i = 0; i < ignoreCount; i++) {
            if ((CollisionObject*)((int**)ignoreList)[i] == other) {
                found = 1;
                break;
            }
        }
        if (found) {
            if (ignoreListMode == 1)
                return 0;
        } else if (ignoreListMode == 0) {
            return 0;
        }
    }
    if (other->ignoreCount > 0) {
        int found = 0;
        for (int i = 0; i < other->ignoreCount; i++) {
            if ((CollisionObject*)((int**)other->ignoreList)[i] == this) {
                found = 1;
                break;
            }
        }
        if (found) {
            if (other->ignoreListMode == 1)
                return 0;
        } else if (other->ignoreListMode == 0) {
            return 0;
        }
    }

    switch (shapeType) {
    case 0:
        return TestHullAgainst(other);
    case 1:
        return TestModelAgainst(other);
    case 2:
        return TestMeshAgainst(other);
    }
    return 0;
}

// 0x00438b90: this is a hull; pick the test by the other object's shape type
// (jump table 0x00438bf0: 0 hull, 1 model, 2 none, 3 capsule, 4 sphere).
int CollisionObject::TestHullAgainst(CollisionObject* other)
{
    CollisionHullBody* mine = (CollisionHullBody*)shape;
    switch (other->shapeType) {
    case 0:
        return HullVsHull(mine, (CollisionHullBody*)other->shape, (CollisionSweepQuery*)contactRecord);
    case 1:
        return HullVsModel(mine, (CollisionModelBody*)other->shape, (CollisionSweepQuery*)contactRecord);
    case 3:
        return HullVsCapsule(mine, (CollisionCapsuleShape*)other->shape);
    case 4:
        return HullVsSphere(mine, (CollisionSphereShape*)other->shape);
    }
    return 0;
}

// 0x00438c10: this is a model (array of hulls); same dispatch shape as above
// (jump table 0x00438c70).
int CollisionObject::TestModelAgainst(CollisionObject* other)
{
    CollisionModelBody* mine = (CollisionModelBody*)shape;
    switch (other->shapeType) {
    case 0:
        return ModelVsHull(mine, (CollisionHullBody*)other->shape, (CollisionSweepQuery*)contactRecord);
    case 1:
        return ModelVsModel(mine, (CollisionModelBody*)other->shape, (CollisionSweepQuery*)contactRecord);
    case 3:
        return ModelVsCapsule(mine, (CollisionCapsuleShape*)other->shape);
    case 4:
        return ModelVsSphere(mine, (CollisionSphereShape*)other->shape);
    }
    return 0;
}

// 0x004379c0: hull vs sphere.  The sphere center is scaled by sphere->field_0x18 and moved
// to world space by the sphere's own matrix (+0x1c); the effective radius is
// field_0x14 * radius.  The contact generator 0x00429570 does the rest.
int CollisionObject::HullVsSphere(CollisionHullBody* hull, CollisionSphereShape* sphere)
{
    CollisionVec3 c = sphere->center * sphere->centerScale;
    CollisionVec3 world = c;
    const float* m = sphere->transform.m;
    world.x = c.z * m[8] + c.y * m[4] + c.x * m[0] + m[12];
    world.y = c.z * m[9] + c.y * m[5] + c.x * m[1] + m[13];
    world.z = c.z * m[10] + c.y * m[6] + c.x * m[2] + m[14];
    float r = sphere->radiusScale * sphere->radius;
    float rr = r * r;
    return Fn_00429570(&world, r, rr, hull->pointTree, &hull->worldTransform, 0);
}

// 0x00437aa0: hull vs capsule.  Both capsule endpoints are scaled by capsule->field_0x24 and
// moved to world space by the capsule matrix (+0x28); effective radius is
// field_0x20 * radius.  The contact generator 0x00429890 does the rest.
int CollisionObject::HullVsCapsule(CollisionHullBody* hull, CollisionCapsuleShape* capsule)
{
    CollisionVec3 ends[2];
    CollisionVec3 a = capsule->p0 * capsule->endpointScale;
    ends[0] = a;
    CollisionVec3 b = capsule->p1 * capsule->endpointScale;
    ends[1] = b;
    const float* m = capsule->transform.m;
    ends[0].x = (a.z * m[8] + a.y * m[4]) + a.x * m[0] + m[12];
    ends[0].y = (a.z * m[9] + a.y * m[5]) + a.x * m[1] + m[13];
    ends[0].z = (a.z * m[10] + a.y * m[6]) + a.x * m[2] + m[14];
    ends[1].x = (b.z * m[8] + b.y * m[4]) + b.x * m[0] + m[12];
    ends[1].y = (b.z * m[9] + b.y * m[5]) + b.x * m[1] + m[13];
    ends[1].z = (b.z * m[10] + b.y * m[6]) + b.x * m[2] + m[14];
    float r = capsule->radiusScale * capsule->radius;
    return Fn_00429890(ends, r, r * r, hull->pointTree, &hull->worldTransform, 0, hull->vertices);
}

// 0x00438860: model vs sphere.  Same world-space sphere as HullVsSphere; the model's bounds
// (+0x18/+0x24, frame +0x88) reject first (0x00425750), then each element hull is tested
// with 0x00429570 until one reports a contact.
int CollisionObject::ModelVsSphere(CollisionModelBody* model, CollisionSphereShape* sphere)
{
    CollisionVec3 c = sphere->center * sphere->centerScale;
    CollisionVec3 world = c;
    const float* m = sphere->transform.m;
    world.x = c.z * m[8] + c.y * m[4] + c.x * m[0] + m[12];
    world.y = c.z * m[9] + c.y * m[5] + c.x * m[1] + m[13];
    world.z = c.z * m[10] + c.y * m[6] + c.x * m[2] + m[14];
    float r = sphere->radiusScale * sphere->radius;
    float rr = r * r;
    if (Fn_00425750(&model->center, &model->halfExtents, world, r, rr, &model->field_0x88)) {
        for (int i = 0; i < model->elementCount; i++) {
            if (Fn_00429570(&world, r, rr, model->elements[i].pointTree,
                            &model->elements[i].worldTransform, 0))
                return 1;
        }
    }
    return 0;
}

// 0x004389b0: model vs capsule; as ModelVsSphere with two endpoints (0x00425900 bounds
// test, then 0x00429890 per element).
int CollisionObject::ModelVsCapsule(CollisionModelBody* model, CollisionCapsuleShape* capsule)
{
    CollisionVec3 ends[2];
    CollisionVec3 a = capsule->p0 * capsule->endpointScale;
    ends[0] = a;
    CollisionVec3 b = capsule->p1 * capsule->endpointScale;
    ends[1] = b;
    const float* m = capsule->transform.m;
    ends[0].x = (a.z * m[8] + a.y * m[4]) + a.x * m[0] + m[12];
    ends[0].y = (a.z * m[9] + a.y * m[5]) + a.x * m[1] + m[13];
    ends[0].z = (a.z * m[10] + a.y * m[6]) + a.x * m[2] + m[14];
    ends[1].x = (b.z * m[8] + b.y * m[4]) + b.x * m[0] + m[12];
    ends[1].y = (b.z * m[9] + b.y * m[5]) + b.x * m[1] + m[13];
    ends[1].z = (b.z * m[10] + b.y * m[6]) + b.x * m[2] + m[14];
    float r = capsule->radiusScale * capsule->radius;
    float rr = r * r;
    if (Fn_00425900(&model->center, &model->halfExtents, ends, r, rr, &model->field_0x88)) {
        for (int i = 0; i < model->elementCount; i++) {
            if (Fn_00429890(ends, r, rr, model->elements[i].pointTree,
                            &model->elements[i].worldTransform, 0, model->elements[i].vertices))
                return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Model families.  A "model" is an array of hulls with shared bounds; the four functions
// below share one skeleton: (optionally forward to the swept variant), move A's frame into
// B's, reject with the broad-phase box test 0x00424730, then loop the enabled elements
// and average the contact positions the element tests wrote to query->contact.
// ---------------------------------------------------------------------------

// 0x00437c20: model vs hull.  Forwards to ModelVsHullSwept when model->swept is set;
// otherwise rel = inverse(model+0x88) * hull+0x48 and each enabled element is tested
// against the hull with the box-box test 0x00436720; the contact is the mean of the hits.
int CollisionObject::ModelVsHull(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return ModelVsHullSwept(a, b, q);

    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->field_0x88);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->worldTransform);
    int hit = 0;
    if (Fn_00424730(a->center, a->halfExtents, &b->triangleTree->center,
                    &b->triangleTree->halfExtents, &rel, &a->field_0xc8)) {
        Vec3 sum(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < a->elementCount; i++) {
            if (a->elementEnabled[i] && HullVsHull(&a->elements[i], b, q)) {
                sum.x += q->contact.x;
                sum.y += q->contact.y;
                sum.z += q->contact.z;
                hit = 1;
                count++;
            }
        }
        if (hit) {
            float s = 1.0f / count;
            q->contact.x = sum.x * s;
            q->contact.y = sum.y * s;
            q->contact.z = sum.z * s;
        }
        return hit;
    }
    return 0;
}

// 0x00437ef0: swept model vs hull.  Like ModelVsHull, but the relative frame uses the hull's
// +0xc8 matrix, the model's swept bounds come from 0x004290d0 (cached in the model at
// +0x108/+0x114) and each enabled element is tested with the swept hull test 0x00436af0.
int CollisionObject::ModelVsHullSwept(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->field_0x88);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->bodyTransform);
    Fn_004290d0(&a->sweptCenter, &a->sweptHalfExtents, a->field_0x30, a->field_0x3c, &a->field_0xc8);
    int hit = 0;
    if (Fn_00424730(a->sweptCenter, a->sweptHalfExtents, &b->triangleTree->center,
                    &b->triangleTree->halfExtents, &rel, &a->field_0xc8)) {
        Vec3 sum(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < a->elementCount; i++) {
            if (a->elementEnabled[i] && HullVsHullSwept(&a->elements[i], b, q)) {
                sum.x += q->contact.x;
                sum.y += q->contact.y;
                sum.z += q->contact.z;
                hit = 1;
                count++;
            }
        }
        if (hit) {
            float s = 1.0f / count;
            q->contact.x = sum.x * s;
            q->contact.y = sum.y * s;
            q->contact.z = sum.z * s;
        }
        return hit;
    }
    return 0;
}

// 0x00438280: model vs model.  Forwards to ModelVsModelSwept when a->swept; otherwise
// rel = inverse(a+0x88) * b+0x88, broad phase on the two model bounds, then every element
// of b is tested against the whole model a with ModelVsHull.
int CollisionObject::ModelVsModel(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return ModelVsModelSwept(a, b, q);

    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->field_0x88);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->field_0x88);
    int hit = 0;
    if (Fn_00424730(a->center, a->halfExtents, &b->center, &b->halfExtents, &rel, &a->field_0xc8)) {
        Vec3 sum(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < b->elementCount; i++) {
            if (ModelVsHull(a, &b->elements[i], q)) {
                sum.x += q->contact.x;
                sum.y += q->contact.y;
                sum.z += q->contact.z;
                hit = 1;
                count++;
            }
        }
        if (hit) {
            float s = 1.0f / count;
            q->contact.x = sum.x * s;
            q->contact.y = sum.y * s;
            q->contact.z = sum.z * s;
        }
        return hit;
    }
    return 0;
}

// 0x00438550: swept model vs model: as ModelVsModel with a's swept bounds (0x004290d0) and
// ModelVsHullSwept per element of b.
int CollisionObject::ModelVsModelSwept(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->field_0x88);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->field_0x88);
    Fn_004290d0(&a->sweptCenter, &a->sweptHalfExtents, a->field_0x30, a->field_0x3c, &a->field_0xc8);
    int hit = 0;
    if (Fn_00424730(a->sweptCenter, a->sweptHalfExtents, &b->center, &b->halfExtents, &rel, &a->field_0xc8)) {
        Vec3 sum(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < b->elementCount; i++) {
            if (ModelVsHullSwept(a, &b->elements[i], q)) {
                sum.x += q->contact.x;
                sum.y += q->contact.y;
                sum.z += q->contact.z;
                hit = 1;
                count++;
            }
        }
        if (hit) {
            float s = 1.0f / count;
            q->contact.x = sum.x * s;
            q->contact.y = sum.y * s;
            q->contact.z = sum.z * s;
        }
        return hit;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Hull vs hull.  Both call the oriented-box test 0x00428950, which leaves contact points in
// the global scratch list (g_CollisionScratchPoints[0 .. g_CollisionScratchCount)) and a hit
// count in query->contactCount, then bring the results back into world space.
// ---------------------------------------------------------------------------

// 0x00436720: hull A vs hull B, non-swept.  Forwards to HullVsHullSwept when a->field_0x00
// is set.  The query vectors and the scratch points are first rotated into B's frame, the
// box test runs with mode 2, and on a hit the contact point is averaged over contactCount
// and transformed back; the vectors/points are always rotated back afterwards.
int CollisionObject::HullVsHull(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return HullVsHullSwept(a, b, q);

    const Matrix4* xf = &b->worldTransform;
    q->contactOffset = CollisionRotateRows(q->contactOffset, xf);
    q->contactNormal = CollisionRotateRows(q->contactNormal, xf);
    q->contact = CollisionVec3(0.0f, 0.0f, 0.0f);
    q->contactCount = 0;
    for (int i = 0; i < g_CollisionScratchCount; i++)
        g_CollisionScratchPoints[i] = CollisionRotateRows(g_CollisionScratchPoints[i], xf);

    int hit = Fn_00428950(a->pointTree, b->triangleTree, &a->worldTransform, xf, 2, b->vertices,
                          &a->motionTransform);
    if (hit) {
        float s = 1.0f / q->contactCount;
        CollisionVec3 c = q->contact;
        c.x *= s;
        c.y *= s;
        c.z *= s;
        q->contact = CollisionTransformPoint(c, xf);
    }
    q->contactOffset = CollisionRotateCols(q->contactOffset, xf);
    q->contactNormal = CollisionRotateCols(q->contactNormal, xf);
    for (int j = 0; j < g_CollisionScratchCount; j++)
        Fn_0042a450(&g_CollisionScratchPoints[j], g_CollisionScratchPoints[j], xf);
    return hit;
}

// 0x00436af0: swept hull test (a is the moving hull).  The stale scratch points are rotated
// by a's matrix, the query vectors are negated and rotated into a's frame, the relative
// frame of b over a's two history matrices is built with 0x00432260 into b+0x148, and the
// box test runs with b as the first shape.  On a hit the contact is averaged and moved by
// a's matrix; the query vectors and scratch points are always rotated back.
int CollisionObject::HullVsHullSwept(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    const Matrix4* xf = &a->worldTransform;
    for (int i = 0; i < g_CollisionScratchCount; i++) {
        CollisionVec3 p = g_CollisionScratchPoints[i];
        g_CollisionScratchPoints[i].x = p.x * xf->m[0][0] + p.z * xf->m[0][2] + p.y * xf->m[0][1];
        g_CollisionScratchPoints[i].y = p.z * xf->m[1][2] + p.y * xf->m[1][1] + p.x * xf->m[1][0];
        g_CollisionScratchPoints[i].z = p.z * xf->m[2][2] + p.y * xf->m[2][1] + p.x * xf->m[2][0];
    }
    q->contactOffset = CollisionRotateRows(CollisionVec3(-q->contactOffset.x, -q->contactOffset.y, -q->contactOffset.z), xf);
    q->contactNormal = CollisionRotateRows(CollisionVec3(-q->contactNormal.x, -q->contactNormal.y, -q->contactNormal.z), xf);
    q->contact = CollisionVec3(0.0f, 0.0f, 0.0f);
    q->contactCount = 0;
    CollisionRelativeFrame(&b->relativeFrame, &b->worldTransform, &a->bodyTransform, &a->prevBodyTransform);

    int hit = 0;
    if (Fn_00428950(b->pointTree, a->triangleTree, &b->worldTransform, xf, 2, a->vertices,
                    &b->relativeFrame)) {
        hit = 1;
        float s = 1.0f / q->contactCount;
        q->contact = CollisionVec3(q->contact.x * s, q->contact.y * s, q->contact.z * s);
        q->contact = CollisionTransformPoint(q->contact, xf);
    }
    q->contactOffset = CollisionRotateCols(CollisionVec3(-q->contactOffset.x, -q->contactOffset.y, -q->contactOffset.z), xf);
    q->contactNormal = CollisionRotateCols(CollisionVec3(-q->contactNormal.x, -q->contactNormal.y, -q->contactNormal.z), xf);
    for (int j = 0; j < g_CollisionScratchCount; j++)
        Fn_0042a450(&g_CollisionScratchPoints[j], g_CollisionScratchPoints[j], xf);
    return hit;
}

// 0x004376f0: swept hull A vs model B.  rel = inverse(a+0xc8) * b+0x88, broad phase between
// a's box bounds (a+0x188) and the model bounds, then every enabled element of b is tested
// with HullVsHullSwept(a, element, q) and the contacts are averaged into q->contact.
// The call to 0x004fc9a0 (node position, result stored to a dead local) is kept as decoded.
int CollisionObject::HullVsModelSwept(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->bodyTransform);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->field_0x88);
    int hit = 0;
    if (Fn_00424730(a->triangleTree->center, a->triangleTree->halfExtents, &b->center,
                    &b->halfExtents, &rel, &a->motionTransform)) {
        Vec3 sum(0.0f, 0.0f, 0.0f);
        CollisionVec3 nodePos(0.0f, 0.0f, 0.0f);
        int count = 0;
        // Retail makes this call (0x004fc9a0); nodePos is never read.
        ((CollisionSceneNode*)a->sceneNode)->GetPositionRelativeTo(0, &nodePos);
        for (int i = 0; i < b->elementCount; i++) {
            if (b->elementEnabled[i] && HullVsHullSwept(a, &b->elements[i], q)) {
                sum.x += q->contact.x;
                sum.y += q->contact.y;
                sum.z += q->contact.z;
                hit = 1;
                count++;
            }
        }
        if (hit) {
            float s = 1.0f / count;
            q->contact.x = sum.x * s;
            q->contact.y = sum.y * s;
            q->contact.z = sum.z * s;
        }
        return hit;
    }
    return 0;
}

// 0x00436e50: hull A vs model B, non-swept.  Forwards to HullVsModelSwept when a->field_0x00
// is set.  rel = b+0x88 * inverse(a+0xc8) (inlined 4x4 product in retail), broad phase on the
// box bounds, then each enabled element of b gets the same rotate-into-frame / box test /
// average / rotate-back sequence as HullVsHull, using the out-of-line vector helpers
// (0x0042a4b0 in, 0x0042a510 contact back, 0x0042a450 vectors back, 0x0043c890 divide,
// 0x00428060 accumulate).  The element-node position call 0x004fc9a0 stores to a dead local.
int CollisionObject::HullVsModel(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return HullVsModelSwept(a, b, q);

    Matrix4 inv;
    CollisionInvertRigid(&inv, &a->bodyTransform);
    Matrix4 rel;
    MatrixMultiply(&rel, inv, b->field_0x88);
    int hit = 0;
    if (Fn_00424730(a->triangleTree->center, a->triangleTree->halfExtents, &b->center,
                    &b->halfExtents, &rel, &a->motionTransform)) {
        CollisionContactSum sum;
        CollisionVec3 nodePos(0.0f, 0.0f, 0.0f);
        int count = 0;
        // Retail makes this call (0x004fc9a0); nodePos is never read.
        ((CollisionSceneNode*)a->sceneNode)->GetPositionRelativeTo(0, &nodePos);
        for (int i = 0; i < b->elementCount; i++) {
            CollisionHullBody* e = &b->elements[i];
            // Retail makes this call too, for each element; the result is not read.
            ((CollisionSceneNode*)e->sceneNode)->GetPositionRelativeTo(0, &nodePos);
            if (!b->elementEnabled[i])
                continue;
            const Matrix4* xf = &e->worldTransform;
            Fn_0042a4b0(&q->contactOffset, q->contactOffset, xf);
            Fn_0042a4b0(&q->contactNormal, q->contactNormal, xf);
            q->contact = CollisionVec3(0.0f, 0.0f, 0.0f);
            q->contactCount = 0;
            for (int j = 0; j < g_CollisionScratchCount; j++)
                Fn_0042a4b0(&g_CollisionScratchPoints[j], g_CollisionScratchPoints[j], xf);
            if (Fn_00428950(a->pointTree, e->triangleTree, &a->worldTransform, xf, 2, e->vertices,
                            &a->motionTransform)) {
                CollisionVec3 mean;
                q->contact = *CollisionDivide(&mean, &q->contact, (float)q->contactCount);
                Fn_0042a510(&q->contact, q->contact, xf);
                sum.Accumulate(&q->contact);
                hit = 1;
                count++;
            }
            Fn_0042a450(&q->contactOffset, q->contactOffset, xf);
            Fn_0042a450(&q->contactNormal, q->contactNormal, xf);
            for (int k = 0; k < g_CollisionScratchCount; k++)
                Fn_0042a450(&g_CollisionScratchPoints[k], g_CollisionScratchPoints[k], xf);
        }
        if (hit) {
            CollisionVec3 mean;
            q->contact = *CollisionDivide(&mean, (const CollisionVec3*)&sum, (float)count);
        }
        return hit;
    }
    return 0;
}

// 0x00436100: world-space axis-aligned bounds of this object's local box (Arvo transformed
// AABB): center' = center * M + T, extent_j = sum_i |M[i][j]| * half_i, output min = center'
// - extent and max = center' + extent.  The box and matrix come from the shape payload:
// hull (0): +0x48 and bounds object +0x188; model (1): +0x88 with center/half at +0x18/+0x24;
// mesh (2): +0x08 with bounds object +0x04.  For any other shape type retail falls through
// with uninitialised locals; that is reproduced (locals left unset).
void CollisionObject::GetWorldBounds(CollisionVec3* outMin, CollisionVec3* outMax)
{
    Matrix4 m;
    const CollisionVec3* c;
    const CollisionVec3* h;
    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)shape;
        m = hull->worldTransform;
        c = &hull->triangleTree->center;
        h = &hull->triangleTree->halfExtents;
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)shape;
        m = model->field_0x88;
        c = &model->center;
        h = &model->halfExtents;
        break;
    }
    case 2: {
        CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
        m = mesh->field_0x08;
        c = &mesh->field_0x04->center;
        h = &mesh->field_0x04->halfExtents;
        break;
    }
    }
    CollisionVec3 center = CollisionTransformPoint(*c, &m);
    float ex = CollisionAbs(h->x * m.m[0][0]) + CollisionAbs(h->y * m.m[1][0]) + CollisionAbs(h->z * m.m[2][0]);
    float ey = CollisionAbs(h->x * m.m[0][1]) + CollisionAbs(h->y * m.m[1][1]) + CollisionAbs(h->z * m.m[2][1]);
    float ez = CollisionAbs(h->x * m.m[0][2]) + CollisionAbs(h->y * m.m[1][2]) + CollisionAbs(h->z * m.m[2][2]);
    outMin->x = center.x - ex;
    outMin->y = center.y - ey;
    outMin->z = center.z - ez;
    outMax->x = center.x + ex;
    outMax->y = center.y + ey;
    outMax->z = center.z + ez;
}

// 0x00438c90: this is a static mesh (type 2); test it against a hull (0) or a model (1).
// The mesh's convex geometry (+4) and matrix (+8) go through the box test 0x00428950 with
// mode 1; on a hit the result vec3 at globalResult+8 is rotated by the hull matrix (v * M).
// For a model, 0x004392c0 rejects first and every enabled element is tested.
int CollisionObject::TestMeshAgainst(CollisionObject* other)
{
    CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
    switch (other->shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)other->shape;
        if (Fn_00428950(mesh->field_0x04, hull->triangleTree, &mesh->field_0x08, &hull->worldTransform, 1,
                        hull->vertices, 0)) {
            g_CollisionBoxResult->field_0x08 = CollisionRotateCols(g_CollisionBoxResult->field_0x08, &hull->worldTransform);
            return 1;
        }
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)other->shape;
        int hit = 0;
        if (Fn_004392c0(other)) {
            for (int i = 0; i < model->elementCount; i++) {
                if (model->elementEnabled[i]) {
                    CollisionHullBody* e = &model->elements[i];
                    if (Fn_00428950(mesh->field_0x04, e->triangleTree, &mesh->field_0x08, &e->worldTransform, 1,
                                    e->vertices, 0)) {
                        hit = 1;
                        g_CollisionBoxResult->field_0x08 = CollisionRotateCols(g_CollisionBoxResult->field_0x08, &e->worldTransform);
                    }
                }
            }
        }
        return hit;
    }
    }
    return 0;
}

// 0x00432180 (cdecl): out = to * inverse(from) for rigid transforms (retail inlines the
// transpose-and-negate inverse on a stack copy and then calls 0x00436500).
void CollisionRelativeTransform(Matrix4* out, const Matrix4* from, const Matrix4* to)
{
    Matrix4 inv;
    CollisionInvertRigid(&inv, from);
    CollisionMatrixMultiply(out, &inv, to);
}

// 0x00432260 (cdecl): relative frame of m1 between two other frames.  Expresses m1 in the
// frames m2 and m3 (t = m1 * inverse(frame)) and returns the transform between those two
// results.  Which of the two temporaries is the "from" side is read from the push order of
// the final 0x00432180 call and is tier 2; the geometric meaning is tier 3.
// Retail inlines the first two relative transforms (two copies of the inverse, then
// 0x00436500) and calls 0x00432180 only for the last one, so they are written out here.
void CollisionRelativeFrame(Matrix4* out, const Matrix4* m1, const Matrix4* m2, const Matrix4* m3)
{
    Matrix4 inv;
    Matrix4 t1;
    Matrix4 t2;
    CollisionInvertRigidRelFrame(&inv, m2);
    CollisionMatrixMultiply(&t1, &inv, m1);
    CollisionInvertRigidRelFrame(&inv, m3);
    CollisionMatrixMultiply(&t2, &inv, m1);
    CollisionRelativeTransform(out, &t1, &t2);
}
