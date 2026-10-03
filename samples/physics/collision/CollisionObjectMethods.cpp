// CollisionObjectMethods.cpp -- small CollisionObject members (CollisionObject.cpp TU, tier 3 names).
// Shape setters free the old shape (FreeShape 0x00432430), allocate the payload with the debug
// allocator at CollisionObject.cpp lines 0x15b / 0x16e / 0x183 / 0x18a, then set field_0x50/0x54.
#include "collision/CollisionObject.h"
#include "broadphase/Quadtree.h"
#include "CollisionShapeTests.h"   // Math3D Matrix4


// 0x0042dc90: builds the mesh shape's point block (dest, matrix, points, count); tier 3 signature.
void Fn_0042dc90(void* dest, void* matrix, void* points, int count);

extern "C" void* memset(void* dest, int c, unsigned int count);
#pragma intrinsic(memset)

static inline void SetIdentity(CollisionMatrix4* r)
{
    memset(r, 0, sizeof(CollisionMatrix4));
    r->m[15] = 1.0f;
    r->m[10] = 1.0f;
    r->m[5] = 1.0f;
    r->m[0] = 1.0f;
}

struct CollisionMeshShapeBlock {                 // type 2, 0x48 bytes
    int field_0x00;
    void* field_0x04;                            // 0x24-byte block, filled by 0x0042dc90
    CollisionMatrix4 field_0x08;                 // identity
};

// 0x004329a0
void CollisionObject::SetSphereShape(CollisionVec3 center, float radius)
{
    FreeShape();
    CollisionSphereShape* s = (CollisionSphereShape*)operator new(0x5c, __FILE__, 0x15b);
    shapeType = 4;
    shape = s;
    s->center = center;
    s->radius = radius;
    s->radiusSquared = radius * radius;
    s->radiusScale = 1.0f;
    s->centerScale = 1.0f;
    SetIdentity(&s->transform);
}

// 0x00432a20
void CollisionObject::SetCapsuleShape(CollisionVec3 p0, CollisionVec3 p1, float radius)
{
    FreeShape();
    CollisionCapsuleShape* s = (CollisionCapsuleShape*)operator new(0x68, __FILE__, 0x16e);
    shapeType = 3;
    shape = s;
    s->p0 = p0;
    s->radiusSquared = radius * radius;
    s->radiusScale = 1.0f;
    s->endpointScale = 1.0f;
    s->p1 = p1;
    s->radius = radius;
    SetIdentity(&s->transform);
}

// 0x00432ab0
void CollisionObject::Fn_00432ab0(int count, void* points)
{
    FreeShape();
    CollisionMeshShapeBlock* s = (CollisionMeshShapeBlock*)operator new(0x48, __FILE__, 0x183);
    shapeType = 2;
    shape = s;
    s->field_0x00 = 0;
    s->field_0x04 = operator new(0x24, __FILE__, 0x18a);
    SetIdentity(&s->field_0x08);
    Fn_0042dc90(s->field_0x04, &s->field_0x08, points, count);
}

// 0x00432120: toggles field_0x80; the object is only in the quadtree while it is set.
void CollisionObject::Fn_00432120(int a)
{
    if (g_collisionQuadTree) {
        int wasSet = useBroadphase;
        int enable = a;
        if (wasSet) {
            if (!enable) {
                g_collisionQuadTree->Remove(this, quadtreeCell);
                useBroadphase = enable;
                return;
            }
            Fn_00436080();
        } else if (enable) {
            Fn_00436080();
        }
        useBroadphase = enable;
    } else {
        useBroadphase = a;
    }
}

// 0x00435fb0
void CollisionObject::Fn_00435fb0()
{
    Fn_00435f10();
    if (g_collisionQuadTree && useBroadphase)
        Fn_00436080();
}

// 0x00435fe0
void CollisionObject::Fn_00435fe0()
{
    Fn_00435fb0();
    Fn_00435fb0();
}

// 0x00436080
void CollisionObject::Fn_00436080()
{
    CollisionVec3 maxBounds;
    CollisionVec3 minBounds;
    GetWorldBounds(&minBounds, &maxBounds);
    unsigned int code = g_collisionQuadTree->ComputeCode(minBounds.x, minBounds.z, maxBounds.x, maxBounds.z);
    if (code != (unsigned int)quadtreeCell) {
        g_collisionQuadTree->Remove(this, quadtreeCell);
        quadtreeCell = code;
        g_collisionQuadTree->Insert(this, code, minBounds.y, maxBounds.y);
    }
}

// 0x00439400
void CollisionObject::SetField_0x74(int v)
{
    ignoreListMode = v;
}

// 0x004394d0
void CollisionObject::SetOwner(void* owner, int tag)
{
    ownerObject = owner;
    ownerType = tag;
}

// 0x00439410
void CollisionObject::AddIgnoredOwner(void* owner)
{
    int i;
    for (i = 0; i < ignoreCount; i++) {
        if (((void**)ignoreList)[i] == owner)
            return;
    }
    for (i = 0; i < ignoreCount; i++) {
        if (((void**)ignoreList)[i] == 0) {
            ((void**)ignoreList)[i] = owner;
            return;
        }
    }
    ignoreList = (int)DebugRealloc((void*)ignoreList, (ignoreCount + 1) * 4, __FILE__, 0x843);
    ((void**)ignoreList)[ignoreCount] = owner;
    ignoreCount++;
}

// 0x00439490
void CollisionObject::RemoveIgnoredOwner(void* owner)
{
    for (int i = 0; i < ignoreCount; i++) {
        if (((void**)ignoreList)[i] == owner) {
            ((void**)ignoreList)[i] = 0;
            return;
        }
    }
}

struct HullPositionView { char pad[0xf8]; CollisionVec3 position; };
struct ModelPositionView { char pad[0xb8]; CollisionVec3 position; };
struct MeshPositionView { char pad[0x38]; CollisionVec3 position; };

// 0x00436000
void CollisionObject::GetShapePosition(CollisionVec3* out)
{
    switch (shapeType) {
    case 0:
        {
            HullPositionView* v = (HullPositionView*)shape;
            out->x = v->position.x;
            out->y = v->position.y;
            out->z = v->position.z;
        }
        break;
    case 1:
        {
            ModelPositionView* v = (ModelPositionView*)shape;
            out->x = v->position.x;
            out->y = v->position.y;
            out->z = v->position.z;
        }
        break;
    case 2:
        {
            MeshPositionView* v = (MeshPositionView*)shape;
            out->x = v->position.x;
            out->y = v->position.y;
            out->z = v->position.z;
        }
        break;
    }
}

// Scene-graph node held by the shape payloads (CollisionSceneNode::GetMatrixIn = 0x004fca80).

struct HullNodeView { int field_0x00; CollisionSceneNode* node; };      // hull: node at +4
struct ModelNodeView { char pad[0x10]; CollisionSceneNode* node; };     // model: node at +0x10
struct MeshNodeView { CollisionSceneNode* node; };                      // mesh: node at +0

// 0x00435f10
void CollisionObject::Fn_00435f10()
{
    Matrix4 m;
    switch (shapeType) {
    case 0:
        ((HullNodeView*)shape)->node->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 1:
        ((ModelNodeView*)shape)->node->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 2:
        ((MeshNodeView*)shape)->node->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 4:
        SetTransform(0);
        break;
    }
}

// 0x0043caa0 (cdecl): true when the input event (arg 3: two ints, code and sub code) matches
// (code, sub) and the input owner accepts the mask; tier 3 signature.
int Fn_0043caa0(int code, int sub, void* event, int mask);

// 0x00434970: debug-key handling for the collision-tree overlay, then the GameObject base.
int CollisionObject::GameObjectVirtualSlot23(int a, int b)
{
    if (Fn_0043caa0(10, 0, (void*)a, 0x80)) {
        debugTreeDepth--;
        if (debugTreeDepth < 0)
            debugTreeDepth = 0;
    }
    if (Fn_0043caa0(11, 0, (void*)a, 0x80))
        debugTreeDepth++;
    if (Fn_0043caa0(0x2e, 0, (void*)a, 0x80)) {
        debugDrawMode++;
        if (debugDrawMode > 7)
            debugDrawMode = 0;
    }
    return GameObject::GameObjectVirtualSlot23(a, b);
}

// 0x00434a10: clears the hit state and (re)initialises the per-object query record that the
// shape tests fill (allocated lazily at field_0x5c; 0x2c bytes for hulls/models, 0x14 for meshes).
int g_CollisionScratchHits;                      // 0x00579060 (cleared here, tier 3 meaning)
void Fn_00439e00(CollisionBoxResult* record);    // sets g_CollisionBoxResult

void CollisionObject::Fn_00434a10()
{
    hasContact = 0;
    g_CollisionScratchCount = 0;
    g_CollisionScratchHits = 0;
    switch (shapeType) {
    case 0: {
        if (contactRecord == 0)
            contactRecord = operator new(0x2c, __FILE__, 0x34b);
        Fn_00439e00((CollisionBoxResult*)contactRecord);
        CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
        *(Vec3*)&q->contactOffset = Vec3(0.0f, 0.0f, 0.0f);
        *(Vec3*)&q->contactNormal = Vec3(0.0f, 0.0f, 0.0f);
        q->contactCount = 0;
        *(float*)&q->fraction = 1.0f;
        break;
    }
    case 1: {
        if (contactRecord == 0)
            contactRecord = operator new(0x2c, __FILE__, 0x35a);
        Fn_00439e00((CollisionBoxResult*)contactRecord);
        CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
        *(Vec3*)&q->contactOffset = Vec3(0.0f, 0.0f, 0.0f);
        *(Vec3*)&q->contactNormal = Vec3(0.0f, 0.0f, 0.0f);
        q->contactCount = 0;
        *(float*)&q->fraction = 1.0f;
        break;
    }
    case 2: {
        if (contactRecord == 0)
            contactRecord = operator new(0x14, __FILE__, 0x369);
        Fn_00439e00((CollisionBoxResult*)contactRecord);
        int* r = (int*)contactRecord;
        r[1] = 0;
        *(float*)&r[0] = 1.0f;
        break;
    }
    }
}

// 0x00434bb0: after a hull/model test, publishes the hit point (record contact) and the summed,
// normalised scratch contact normals from this test's scratch entries.
// Inline normalise as retail emits it: lengths of exactly 1.0f skip the reciprocal square root.
static inline Vec3 ContactNormalized(const Vec3& v)
{
    float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (lengthSquared == 1.0f)
        return v;
    float inverseLength = FastInvSqrt(lengthSquared);
    return Vec3(inverseLength * v.x, inverseLength * v.y, inverseLength * v.z);
}

void CollisionObject::Fn_00434bb0()
{
    if ((unsigned int)shapeType > 1)
        return;
    CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
    hitPoint = q->contact;
    Vec3* sum = (Vec3*)&q->contactNormal;
    *sum = Vec3(0.0f, 0.0f, 0.0f);
    for (int i = g_CollisionScratchHits; i < g_CollisionScratchCount; i++) {
        sum->x += g_CollisionScratchPoints[i].x;
        sum->y += g_CollisionScratchPoints[i].y;
        sum->z += g_CollisionScratchPoints[i].z;
    }
    Vec3 normal = ContactNormalized(*sum);
    *sum = normal;
    *(Vec3*)&hitNormal = normal;
    g_CollisionScratchHits = g_CollisionScratchCount;
}

// 0x00434cf0: like Fn_00434bb0 for the record only: normalises the summed scratch normals into
// the record, then pushes the record's contact offset out along its own direction by 0.075.
void CollisionObject::Fn_00434cf0()
{
    if ((unsigned int)shapeType > 1)
        return;
    CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
    Vec3* sum = (Vec3*)&q->contactNormal;
    *sum = Vec3(0.0f, 0.0f, 0.0f);
    if (g_CollisionScratchCount > 0) {
        for (int i = 0; i < g_CollisionScratchCount; i++) {
            sum->x += g_CollisionScratchPoints[i].x;
            sum->y += g_CollisionScratchPoints[i].y;
            sum->z += g_CollisionScratchPoints[i].z;
        }
        *sum = ContactNormalized(*sum);
    }
    Vec3* offset = (Vec3*)&q->contactOffset;
    Vec3 direction = ContactNormalized(*offset);
    Vec3 push = Vec3(direction.x * 0.075f, direction.y * 0.075f, direction.z * 0.075f);
    offset->x += push.x;
    offset->y += push.y;
    offset->z += push.z;
}

// 0x00428c20: broad-phase overlap of two bounds boxes under their transforms (tier 3 signature
// from the push order at 0x004393e8: boundsA, boundsB, xfA, xfB).
int Fn_00428c20(CollisionBoxBounds* boundsA, CollisionBoxBounds* boundsB, const Matrix4* xfA,
                const Matrix4* xfB);

// 0x004392c0: bounds-only overlap of this object against `other` (used by TestMeshAgainst).
int CollisionObject::Fn_004392c0(CollisionObject* other)
{
    CollisionBoxBounds modelBoundsA;
    CollisionBoxBounds modelBoundsB;
    CollisionBoxBounds* boundsA;
    const Matrix4* xfA;
    CollisionBoxBounds* boundsB;
    const Matrix4* xfB;

    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)shape;
        boundsA = hull->triangleTree;
        xfA = &hull->bodyTransform;
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)shape;
        modelBoundsA.center = model->center;
        modelBoundsA.halfExtents = model->halfExtents;
        boundsA = &modelBoundsA;
        xfA = &model->field_0x88;
        break;
    }
    case 2: {
        CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
        boundsA = mesh->field_0x04;
        xfA = &mesh->field_0x08;
        break;
    }
    }
    switch (other->shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)other->shape;
        boundsB = hull->triangleTree;
        xfB = &hull->bodyTransform;
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)other->shape;
        modelBoundsB.center = model->center;
        modelBoundsB.halfExtents = model->halfExtents;
        boundsB = &modelBoundsB;
        xfB = &model->field_0x88;
        break;
    }
    case 2: {
        CollisionMeshBody* mesh = (CollisionMeshBody*)other->shape;
        boundsB = mesh->field_0x04;
        xfB = &mesh->field_0x08;
        break;
    }
    }
    return Fn_00428c20(boundsA, boundsB, xfA, xfB);
}
