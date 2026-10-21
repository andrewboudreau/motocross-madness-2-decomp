// CollisionObject.cpp -- retail TU D:\aardvark\VC\krusty2\CollisionObject.cpp (see README.md).
//
// Ownership: every function here lies between the first and last CollisionObject.cpp
// __FILE__ references (0x00431dd8..0x0043a124) in link order, and the allocations of the
// shape setters pass that string.  The sections below were separate sample files during
// matching; the section banners keep their original notes.
#include "../../reconstructed/TrackGameGlobal.h"
#include "../../reconstructed/ControlInterface.h"
#include "collision/CollisionObject.h"
#include "collision/CollisionShapeTests.h"
#include "broadphase/Quadtree.h"
#include "math/Math3D.h"
#include "../../reconstructed/MatrixUtil.h"
#include "../../reconstructed/BoundingBoxTreeFunctions.h"
#include "bvh/BoundingBoxTreeQueryFunctions.h"
#include "soultree/SoultreePhysicsObject.h"

// ==============================================================================================
// Section: CollisionObject
// ==============================================================================================
// Collision pipeline as understood (tier 3, from call graph + decoded bodies):
//   SoultreePhysics contacts -> CollisionPoint::UpdateRelativeMotion (0x43a640): builds a
//     contact from position delta, separating direction and tangent frame via
//     cross products; normalizes (0x5087b0), computes penetration/length, and
//     registers points with AddCollisionPoint (0x43a330).
//   0x43aa30 merges the penetrating contacts (mean position, summed+normalised normal,
//     deepest penetration, response terms); 0x43ad80 refreshes contacts (both CollisionPoint.cpp).
//   Broadphase helpers: SphereContainsPointXZ (0x43a1e0, point-in-circle in XZ),
//     BoundingSpheresOverlap (0x43a270). Vector helpers: CollisionRejectFrom
//     (0x43b190, a - proj_b a), CollisionLength (0x435ec0), CollisionDivide (0x43c890).
//   Narrow phase (CollisionShapeTests.cpp, tier 3 names; shape type = field_0x50:
//     0 hull/oriented box, 1 model = array of hulls, 2 static mesh, 3 capsule, 4 sphere):
//     TestAgainst 0x436430 filters and picks TestHull/Model/MeshAgainst (0x438b90/0x438c10/0x438c90);
//     the first two jump-table into HullVs{Hull 0x436720, Model 0x436e50, Capsule 0x437aa0,
//     Sphere 0x4379c0} and ModelVs{Hull 0x437c20, Model 0x438280, Capsule 0x4389b0, Sphere 0x438860};
//     swept variants 0x436af0 / 0x4376f0 / 0x437ef0 / 0x438550 are chosen by the hull swept flag.
//     Hull tests bring the query into the other body's frame, call the box test 0x428950
//     (or 0x429570/0x429890 for sphere/capsule), average contacts and transform them back.
//     GetWorldBounds 0x436100 is an Arvo transformed AABB.
//   CollisionObject owns shapes (ctor 0x431e70, FreeShape 0x432430).

// Retail helpers reached by direct call (addresses are the call targets).
void CollisionHullShape_Free(CollisionHullShape* s);   // 0x00431da0
// Element type of a model's hull array.  Retail allocates it with array new, whose empty
// constructor loop survives as a leftover count in the code, so the class needs a constructor.
struct CollisionHullElement : CollisionHullBody {
    CollisionHullElement() {}
};
struct CollisionModelShapeData {   // type 1: count + array of CollisionHullShape (stride 0x198)
    int hullCount;
    void* field_0x04;
    void* field_0x08;
    char field_0x0c[8];
    CollisionHullElement* hulls;   // hull elements, 0x198 bytes each
};
struct CollisionMeshShapeData {    // type 2
    int field_0x00;
    void* field_0x04;
};
void CollisionModelShape_Free(void* s);                // 0x00431df0
void CollisionMeshShape_Free(void* s);                 // 0x00431e50
// 0x004a30c0 is the plain operator delete(void*) (the target of `delete shape` below as well).

void CollisionHullShape_Free(CollisionHullShape* s) {
    if (s->triangleTree)
        FreeBoxTree(static_cast<QueryTreeNode*>(s->triangleTree));
    if (s->pointTree)
        FreeBoxTree(static_cast<QueryTreeNode*>(s->pointTree));
    if (s->vertices)
        DebugFree(s->vertices, __FILE__, 36);
}

void CollisionModelShape_Free(void* shape) {
    CollisionModelShapeData* s = (CollisionModelShapeData*)shape;
    // Indexing through the reference (not s->hulls[i]) makes VC6 rebuild each
    // hull address as ebx + edx from a fresh hulls load, as retail does.
    CollisionModelShapeData& model = *s;
    for (int i = 0; i < model.hullCount; i++)
        CollisionHullShape_Free((CollisionHullShape*)&model.hulls[i]);
    operator delete(s->hulls);
    operator delete(s->field_0x04);
    operator delete(s->field_0x08);
    operator delete(s);
}

void CollisionMeshShape_Free(void* shape) {
    CollisionMeshShapeData* s = (CollisionMeshShapeData*)shape;
    if (s->field_0x04)
        FreeBoxTree(static_cast<QueryTreeNode*>(s->field_0x04));
    operator delete(s);
}

void CollisionObject::FreeShape() {
    if (shape != 0) {
        switch (shapeType) {
        case 0:
            CollisionHullShape_Free((CollisionHullShape*)shape);
            delete shape;
            break;
        case 1:
            CollisionModelShape_Free(shape);
            break;
        case 2:
            CollisionMeshShape_Free(shape);
            break;
        case 3:
        case 4:
            delete shape;
            break;
        }
    }
    if (contactRecord != 0)
        delete contactRecord;
    shape = 0;
    contactRecord = 0;
}

// Type-id registry (TypeRegistry.cpp): 0x00521ea0 looks a type name up and returns its id.
class TypeRegistry {
public:
    char FindTypeId(const char* name);           // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;             // 0x00575744

static char g_CollisionObjectTypeId = (char)0xff;   // 0x00568610
static char g_VegetationTypeId = (char)0xff;        // 0x00568611

// 0x00431e70
CollisionObject::CollisionObject(int a)
    : GraphicsTest(a)
{
    if (g_CollisionObjectTypeId == (char)0xff)
        g_CollisionObjectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
    if (g_VegetationTypeId == (char)0xff)
        g_VegetationTypeId = g_TypeRegistry->FindTypeId("Vegetation");
    AppendClassName(this);
    QuadTreeObject::objectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
    shapeType = 0;
    shape = 0;
    contactRecord = 0;
    hasContact = 0;
    onHitCallback = 0;
    onHitByCallback = 0;
    debugTreeDepth = 0;
    debugDrawMode = 0;
    quadtreeCell = 15;
    ignoreVegetation = 0;
    collisionEnabled = 1;
    collidable = 1;
    ignoreListMode = 1;
    ignoreList = 0;
    ignoreCount = 0;
    ownerObject = 0;
    ownerType = 0;
    field_0x98 = 0;
    hitObject = 0;
    hitPoint = g_CollisionVec3_5797b0;
    hitNormal = g_CollisionVec3_5797b0;
}

// 0x00431fd0 (deleting) -> 0x00432000 (core)
CollisionObject::~CollisionObject()
{
    if (g_collisionQuadTree && useBroadphase)
        g_collisionQuadTree->Remove(this, quadtreeCell);
    FreeShape();
    if (ignoreList)
        DebugFree((void*)ignoreList, __FILE__, 0x6c);
    if (shape) {
        switch (shapeType) {
        case 0:
            CollisionHullShape_Free((CollisionHullShape*)shape);
            delete shape;
            break;
        case 1:
            CollisionModelShape_Free(shape);
            break;
        case 2:
            CollisionMeshShape_Free(shape);
            break;
        case 3:
        case 4:
            delete shape;
            break;
        }
    }
}

// ==============================================================================================
// Section: CollisionShapeSetup
// ==============================================================================================
// CollisionShapeSetup.cpp -- CollisionObject shape setters that build a payload from a scene
// node, mesh arrays or a .col stream (wave 5).  Names tier 3; offsets/sizes tier 1 (target bytes).
// owner: CollisionObject.cpp (__FILE__ strings at every allocation, lines 0xd8..0x14f / 0x9e8..0x9f0).

// 0x00432720 (ret 0x14): hull shape for a scene node.  `buildPointTree` (arg 2, the only one
// that is tested) adds the point tree; arg 3 is the hull's swept flag; args 4 and 5 are unused
// except that arg 5 is forwarded to the triangle-tree builder.
void CollisionObject::SetHullShape(void* node, int buildPointTree, int swept, int c, int d)
{
    FreeShape();
    CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x116);
    hull->relativeFrame = IdentityMatrix();
    hull->swept = swept;
    shapeType = 0;
    shape = hull;
    hull->sceneNode = node;
    hull->vertices = 0;
    hull->triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0x123);
    BuildModelTriangleTree(reinterpret_cast<BoxTreeNode*>(hull->triangleTree),
                           reinterpret_cast<TreeMatrix4*>(&hull->localTransform),
                           reinterpret_cast<TreeModelSource*>(node),
                           reinterpret_cast<TreeVec3**>(&hull->vertices), 0, d);
    hull->pointTree = 0;
    if (buildPointTree) {
        hull->pointTree = operator new(0x24, __FILE__, 0x128);
        BuildModelPointTree(reinterpret_cast<BoxTreeNode*>(hull->pointTree),
                            reinterpret_cast<TreeMatrix4*>(&hull->localTransform),
                            reinterpret_cast<TreeModelSource*>(node), 0);
    }
}

// 0x004328b0 (ret 0x10): static-mesh hull from vertex/index arrays.  The vertices are copied
// into a block of the object's own (DebugMalloc 12 bytes each, line 0x149), then the triangle
// tree is built from the copy.  The hull's swept flag is 0 and it has no point tree.
void CollisionObject::SetTriangleMeshHullShape(const CollisionVec3* verts, const int* indices, int triCount,
                                  int vertCount)
{
    FreeShape();
    CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x13c);
    hull->relativeFrame = IdentityMatrix();
    hull->swept = 0;
    shapeType = 0;
    shape = hull;
    hull->sceneNode = 0;
    hull->vertices = DebugMalloc(vertCount * sizeof(CollisionVec3), __FILE__, 0x149);
    for (int i = 0; i < vertCount; i++)
        ((CollisionVec3*)hull->vertices)[i] = verts[i];
    hull->triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0x14f);
    BuildTriangleMeshTree(reinterpret_cast<BoxTreeNode*>(hull->triangleTree),
                          reinterpret_cast<TreeMatrix4*>(&hull->localTransform),
                          reinterpret_cast<TreeVec3*>(hull->vertices), indices, triCount, vertCount, 0);
    hull->pointTree = 0;
}

// The .col loader uses the shared 0x134-byte archive stream: constructor
// 0x00460d10 stores this resource-manager pointer at +0x12c, Open returns
// an int at 0x00460f50 (ignored here), and Read uses fread argument order.
#include "../../reconstructed/FileStream.h"
extern UnknownResourceManager* g_ResourceManager; // 0x00572b44

// 0x00439e10 (ret 8): reads the shape type from the stream and builds that payload
// (0: hull 0x198 bytes, 1: model 0x120 bytes), then lets the matching loader fill it.
void CollisionObject::ReadShape(void* node, CollisionFileStream* stream)
{
    FreeShape();
    stream->Read(&shapeType, 4, 1);
    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x9e8);
        hull->sceneNode = node;
        hull->relativeFrame = IdentityMatrix();
        shape = hull;
        ReadHullShape(stream, hull);
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)operator new(0x120, __FILE__, 0x9f0);
        model->field_0x10 = (int)node;
        shape = model;
        ReadModelShape(node, stream, model);
        break;
    }
    }
}

// 0x00432800 (ret 8): shape from a .col file.  Opens the file in "rb" mode and lets
// ReadShape read it; the file object is heap-allocated at line 0x12f.
void CollisionObject::LoadShape(void* node, const char* path)
{
    CollisionFileStream* file = new(__FILE__, 0x12f) CollisionFileStream(g_ResourceManager);
    file->Open(path, "rb", 0);
    ReadShape(node, file);
    delete file;
}

// Scene-graph node as the shape setup sees it (the same SoultreeObject the other areas model;
// local stand-in with just the calls used here, all thiscall).
class CollisionSourceNode {
public:
    char field_0x00[0x13c];
    CollisionSourceNode* parent;   // +0x13c 0x0043a050 climbs it to the root before collecting
};

// Box-tree readers use the shared BVH pointer types at this boundary.
struct BoxTreeNode;
struct TreeVec3;
void ReadTriangleMesh(BoxTreeNode** tree, TreeVec3** vertices, CollisionFileStream* stream);   // 0x0042e0f0
void ReadPointNode(BoxTreeNode** tree, CollisionFileStream* stream);                       // 0x0042e190

// 0x00439ed0 (ret 8): reads one hull payload: the 3x4 local transform (the w column is set
// to 0,0,0,1 here), the swept flag, then the triangle tree with its vertices and the point
// tree, each behind a presence flag.
void CollisionObject::ReadHullShape(CollisionFileStream* stream, CollisionHullBody* hull)
{
    stream->Read(&hull->localTransform.m[0][0], 4, 1);
    stream->Read(&hull->localTransform.m[0][1], 4, 1);
    stream->Read(&hull->localTransform.m[0][2], 4, 1);
    hull->localTransform.m[0][3] = 0.0f;
    stream->Read(&hull->localTransform.m[1][0], 4, 1);
    stream->Read(&hull->localTransform.m[1][1], 4, 1);
    stream->Read(&hull->localTransform.m[1][2], 4, 1);
    hull->localTransform.m[1][3] = 0.0f;
    stream->Read(&hull->localTransform.m[2][0], 4, 1);
    stream->Read(&hull->localTransform.m[2][1], 4, 1);
    stream->Read(&hull->localTransform.m[2][2], 4, 1);
    hull->localTransform.m[2][3] = 0.0f;
    stream->Read(&hull->localTransform.m[3][0], 4, 1);
    stream->Read(&hull->localTransform.m[3][1], 4, 1);
    stream->Read(&hull->localTransform.m[3][2], 4, 1);
    hull->localTransform.m[3][3] = 1.0f;
    stream->Read(&hull->swept, 4, 1);
    int hasTriangles;
    stream->Read(&hasTriangles, 4, 1);
    if (hasTriangles)
        ReadTriangleMesh((BoxTreeNode**)&hull->triangleTree, (TreeVec3**)&hull->vertices, stream);
    hull->pointTree = 0;
    int hasPoints;
    stream->Read(&hasPoints, 4, 1);
    if (hasPoints)
        ReadPointNode((BoxTreeNode**)&hull->pointTree, stream);
}

// 0x0043a050 (ret 0xc): reads a model payload.  The node's root (followed through +0x13c) is
// expanded into a node list; each element record names its scene node by index into it.
void CollisionObject::ReadModelShape(void* nodeArg, CollisionFileStream* stream, CollisionModelBody* model)
{
    CollisionSourceNode* node = (CollisionSourceNode*)nodeArg;
    while (node->parent)
        node = node->parent;
    int total = reinterpret_cast<SoultreeObject*>(node)->CountNodes();
    CollisionSourceNode** list = (CollisionSourceNode**)operator new(total * 4, __FILE__, 0xa9e);
    int one = 1;
    list[0] = node;
    reinterpret_cast<SoultreeObject*>(node)->CollectDescendants(&one, reinterpret_cast<SoultreeObject**>(list));
    stream->Read(&model->elementCount, 4, 1);
    stream->Read(&model->swept, 4, 1);
    model->elements = new(__FILE__, 0xaa8) CollisionHullElement[model->elementCount];
    model->elementEnabled = (int*)operator new(model->elementCount * 4, __FILE__, 0xaa9);
    model->elementHighlight = (int*)operator new(model->elementCount * 4, __FILE__, 0xaaa);
    for (int i = 0; i < model->elementCount; i++) {
        int index;
        stream->Read(&index, 4, 1);
        model->elements[i].sceneNode = list[index];
        model->elements[i].relativeFrame = IdentityMatrix();
        ReadHullShape(stream, &model->elements[i]);
        model->elementEnabled[i] = 1;
    }
    operator delete(list);
}


// 0x004324b0 (ret 0x14): model shape = one hull per accepted node of the scene subtree.
// arg 2 builds the point trees too, arg 3 is the model's swept flag, arg 4 is the filter passed
// to the node predicate, arg 5 is only forwarded to the triangle-tree builder.
void CollisionObject::SetModelShape(void* nodeArg, int buildPointTrees, int swept, int filter, int d)
{
    CollisionSourceNode* node = (CollisionSourceNode*)nodeArg;
    FreeShape();
    CollisionModelBody* model = (CollisionModelBody*)operator new(0x120, __FILE__, 0xd8);
    shapeType = 1;
    shape = model;
    model->swept = swept;
    int total = reinterpret_cast<SoultreeObject*>(node)->CountNodes();
    CollisionSourceNode** list = (CollisionSourceNode**)operator new(total * 4, __FILE__, 0xe4);
    int one = 1;
    list[0] = node;
    reinterpret_cast<SoultreeObject*>(node)->CollectDescendants(&one, reinterpret_cast<SoultreeObject**>(list));
    model->elementCount = 0;
    for (int i = 0; i < total; i++) {
        if (reinterpret_cast<D3DIMSoultreeObject*>(node)->NodeMovesVertices(reinterpret_cast<SoultreeObject*>(list[i]), filter))
            model->elementCount++;
    }
    model->field_0x10 = (int)node;
    model->elements = new(__FILE__, 0xf3) CollisionHullElement[model->elementCount];
    model->elementEnabled = (int*)operator new(model->elementCount * 4, __FILE__, 0xf4);
    model->elementHighlight = (int*)operator new(model->elementCount * 4, __FILE__, 0xf5);
    int index = 0;
    for (int j = 0; j < total; j++) {
        if (reinterpret_cast<D3DIMSoultreeObject*>(node)->NodeMovesVertices(reinterpret_cast<SoultreeObject*>(list[j]), filter)) {
            model->elements[index].swept = swept;
            model->elementEnabled[index] = 1;
            model->elements[index].triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0xfd);
            model->elements[index].sceneNode = list[j];
            model->elements[index].relativeFrame = IdentityMatrix();
            BuildModelTriangleTree(reinterpret_cast<BoxTreeNode*>(model->elements[index].triangleTree),
                                   reinterpret_cast<TreeMatrix4*>(&model->elements[index].localTransform),
                                   reinterpret_cast<TreeModelSource*>(node),
                                   reinterpret_cast<TreeVec3**>(&model->elements[index].vertices),
                                   reinterpret_cast<TreeSceneNode*>(list[j]), d);
            model->elements[index].pointTree = 0;
            if (buildPointTrees) {
                model->elements[index].pointTree = operator new(0x24, __FILE__, 0x107);
                BuildModelPointTree(reinterpret_cast<BoxTreeNode*>(model->elements[index].pointTree),
                                   reinterpret_cast<TreeMatrix4*>(&model->elements[index].localTransform),
                                   reinterpret_cast<TreeModelSource*>(node),
                                   reinterpret_cast<TreeSceneNode*>(list[j]));
            }
            index++;
        }
    }
}

// ==============================================================================================
// Section: CollisionObjectMethods
// ==============================================================================================
// CollisionObjectMethods.cpp -- small CollisionObject members (CollisionObject.cpp TU, tier 3 names).
// Shape setters free the old shape (FreeShape 0x00432430), allocate the payload with the debug
// allocator at CollisionObject.cpp lines 0x15b / 0x16e / 0x183 / 0x18a, then set field_0x50/0x54.

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
void CollisionObject::SetMeshShape(int count, void* points)
{
    FreeShape();
    CollisionMeshShapeBlock* s = (CollisionMeshShapeBlock*)operator new(0x48, __FILE__, 0x183);
    shapeType = 2;
    shape = s;
    s->field_0x00 = 0;
    s->field_0x04 = operator new(0x24, __FILE__, 0x18a);
    SetIdentity(&s->field_0x08);
    BuildModelBoxTree(reinterpret_cast<BoxTreeNode*>(s->field_0x04),
                      reinterpret_cast<const TreeMatrix4*>(&s->field_0x08),
                      reinterpret_cast<TreeVec3*>(points), count);
}

// 0x00432120: toggles useBroadphase; the object is only in the quadtree while it is set.
void CollisionObject::SetUseBroadphase(int a)
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
            UpdateQuadtreeCell();
        } else if (enable) {
            UpdateQuadtreeCell();
        }
        useBroadphase = enable;
    } else {
        useBroadphase = a;
    }
}

// 0x00435fb0
void CollisionObject::UpdatePlacement()
{
    SyncTransformFromNode();
    if (g_collisionQuadTree && useBroadphase)
        UpdateQuadtreeCell();
}

// 0x00435fe0: UpdatePlacement twice. SetTransform moves the current body
// transform into the previous one, so the second pass leaves no motion
// between them (callers use it after teleporting a body).
void CollisionObject::SnapPlacement()
{
    UpdatePlacement();
    UpdatePlacement();
}

// 0x00436080
void CollisionObject::UpdateQuadtreeCell()
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
void CollisionObject::SetIgnoreListMode(int mode)
{
    ignoreListMode = mode;
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

// Primary scene-graph pointers held by the shape payloads (GetMatrixIn = 0x004fca80).

struct HullNodeView { int field_0x00; CollisionSceneNode* node; };      // hull: node at +4
struct ModelNodeView { char pad[0x10]; CollisionSceneNode* node; };     // model: node at +0x10
struct MeshNodeView { CollisionSceneNode* node; };                      // mesh: node at +0

// 0x00435f10
void CollisionObject::SyncTransformFromNode()
{
    Matrix4 m;
    switch (shapeType) {
    case 0:
        reinterpret_cast<SoultreeObject*>(((HullNodeView*)shape)->node)->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 1:
        reinterpret_cast<SoultreeObject*>(((ModelNodeView*)shape)->node)->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 2:
        reinterpret_cast<SoultreeObject*>(((MeshNodeView*)shape)->node)->GetMatrixIn(0, &m);
        SetTransform(&m);
        break;
    case 4:
        SetTransform(0);
        break;
    }
}

// 0x00434970: debug-key handling for the collision-tree overlay, then the GameObject base.
int CollisionObject::OnControlPress(UnknownControlEvent* a, UnknownInputEntry* b)
{
    if (TestInputEvent(10, 0, a, 0x80)) {
        debugTreeDepth--;
        if (debugTreeDepth < 0)
            debugTreeDepth = 0;
    }
    if (TestInputEvent(11, 0, a, 0x80))
        debugTreeDepth++;
    if (TestInputEvent(0x2e, 0, a, 0x80)) {
        debugDrawMode++;
        if (debugDrawMode > 7)
            debugDrawMode = 0;
    }
    return GameObject::OnControlPress(a, b);
}

// 0x00434a10: clears the hit state and (re)initialises the per-object query record that the
// shape tests fill (allocated lazily at field_0x5c; 0x2c bytes for hulls/models, 0x14 for meshes).
int g_CollisionScratchHits;                      // 0x00579060 (cleared here, tier 3 meaning)
void SetCollisionBoxResult(CollisionBoxResult* record);    // sets g_CollisionBoxResult

void CollisionObject::ResetHitState()
{
    hasContact = 0;
    g_CollisionScratchCount = 0;
    g_CollisionScratchHits = 0;
    switch (shapeType) {
    case 0: {
        if (contactRecord == 0)
            contactRecord = operator new(0x2c, __FILE__, 0x34b);
        SetCollisionBoxResult((CollisionBoxResult*)contactRecord);
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
        SetCollisionBoxResult((CollisionBoxResult*)contactRecord);
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
        SetCollisionBoxResult((CollisionBoxResult*)contactRecord);
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
// The components are read into scalars y, x, z (retail's load order) and the square sum is
// z*z + (x*x + y*y), which gives retail's x, y, z multiply order; the plain
// v.x*v.x + v.y*v.y + v.z*v.z loads z, y, x.
static inline Vec3 ContactNormalized(const Vec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float lengthSquared = z * z + (x * x + y * y);
    if (lengthSquared == 1.0f)
        return v;
    float inverseLength = FastInvSqrt(lengthSquared);
    return Vec3(inverseLength * v.x, inverseLength * v.y, inverseLength * v.z);
}

void CollisionObject::StoreHitResult()
{
    if ((unsigned int)shapeType > 1)
        return;
    CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
    hitPoint = q->contact;
    Vec3* sum = (Vec3*)&q->contactNormal;
    *sum = Vec3(0.0f, 0.0f, 0.0f);
    // Vec3::operator+= gives retail's p.x + sum.x, p.y + sum.y but sum.z + p.z
    for (int i = g_CollisionScratchHits; i < g_CollisionScratchCount; i++) {
        *sum += *(Vec3*)&g_CollisionScratchPoints[i];
    }
    Vec3 normal = ContactNormalized(*sum);
    *sum = normal;
    *(Vec3*)&hitNormal = normal;
    g_CollisionScratchHits = g_CollisionScratchCount;
}

// 0x00434cf0: like StoreHitResult for the record only: normalises the summed scratch normals into
// the record, then pushes the record's contact offset out along its own direction by 0.075.
void CollisionObject::StoreHitNormal()
{
    if ((unsigned int)shapeType > 1)
        return;
    CollisionSweepQuery* q = (CollisionSweepQuery*)contactRecord;
    Vec3* sum = (Vec3*)&q->contactNormal;
    *sum = Vec3(0.0f, 0.0f, 0.0f);
    if (g_CollisionScratchCount > 0) {
        for (int i = 0; i < g_CollisionScratchCount; i++) {
            *sum += *(Vec3*)&g_CollisionScratchPoints[i];
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

// 0x004392c0: bounds-only overlap of this object against `other` (used by TestMeshAgainst).
// The second model case sets `boundsB` before it reads `other->shape`; reading the shape
// first swaps ecx and eax for `other` and its shape type across the second switch.
int CollisionObject::TestMeshBounds(CollisionObject* other)
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
        boundsA = &modelBoundsA;
        boundsA->center = model->center;
        boundsA->halfExtents = model->halfExtents;
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
        boundsB = &modelBoundsB;
        CollisionModelBody* model = (CollisionModelBody*)other->shape;
        boundsB->center = model->center;
        boundsB->halfExtents = model->halfExtents;
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
    return TreeBoxOverlap(reinterpret_cast<QueryTreeNode*>(boundsA), reinterpret_cast<QueryTreeNode*>(boundsB),
        xfA, xfB);
}

// ==============================================================================================
// Section: CollisionDebugDraw
// ==============================================================================================
// CollisionDebugDraw.cpp -- debug drawing of the collision bounding-volume trees.
//
// TU ownership: tier 2.  These functions sit together (0x00432b30-0x004341a5) inside the
// CollisionObject.cpp address range.  They are kept in their own sample file only for
// readability.  DrawModel (0x00432b30) and DrawHull (0x00432d30) pick one of the tree
// draws by a mode number.
//
// Evidence (tier 1 unless noted):
//  * All are thiscall members of CollisionObject.  Every draw call goes through
//    `lea reg,[ecx+0xc]`, i.e. the GraphicsTest base (CollisionObject.h).
//  * The tree node is 0x24 bytes: a float at +0 compared with 0.0f (>= 0 means an
//    interior node, < 0 a leaf), a center/half-extent pair at +4/+0x10 (passed to the box
//    draw 0x0047c270), and two children at +0x1c/+0x20, both recursed into.  A triangle
//    leaf has three ushort vertex indices at +4/+6/+8 into a 12-byte vertex array and a
//    normal at +0xc.  A point-tree leaf keeps its point at +4.
//  * The trees are CollisionHullBody::field_0x188 / field_0x18c (CollisionShapeTests.h),
//    so CollisionBoxBounds is this node's box view.
//  * Retail turns the second recursive call into a loop (VC6 tail-recursion
//    elimination).  The source recurses into both children.
//
// Vector math: these functions use the Math3D Vec3.  Its constructor and operator+ are
// the out-of-line COMDATs 0x00404e60 and 0x00421cb0 that DrawTreeNormals calls.
// CollisionVec3 has no operator that constructs through the Vec3 constructor.

// Tree node.  Names are tier 3; offsets are tier 1 (see above).
struct CollisionTreeNode {
    float field_0x00;              // >= 0: interior node (box valid); < 0: leaf
    Vec3 center;                   // +0x04 box center (a point leaf: the point)
    Vec3 halfExtents;              // +0x10
    CollisionTreeNode* child[2];   // +0x1c, +0x20
};

// Triangle leaf view of the same node (field_0x00 < 0).
struct CollisionTreeTriangle {
    float field_0x00;
    unsigned short vertex[3];      // +0x04, indices into the vertex array
    Vec3 normal;                   // +0x0c
};

// Segment leaf view (polyline trees, field_0x00 < 0): the two end points.
struct CollisionTreeSegment {
    float field_0x00;
    Vec3 end[2];                   // +0x04, +0x10
};

// 0x00428db0 (cdecl, 3 args): moves the box (center, half extents) by the second
// matrix in place.  The motion draw calls it between two DrawBox calls.  Tier 3 name.
#include "../../reconstructed/CollisionTransforms.h"

// v * M + translation row.  This has the same signature and result as the out-of-line
// 0x0042a510 (out, v by value, m).  DrawTreeNormals calls that copy for its fourth
// transform, because VC6 stops inlining there (inline budget).  The _RC member names
// matter: the m[r][c] form costs more inline budget, and then the later operator* is
// not inlined either.
inline void DebugTransformPoint(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// The same transform with v by reference: retail reads the source vector in place,
// with no stack copy, in DrawPointTree's first transform.
inline void DebugTransformPointRef(Vec3* out, const Vec3& v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// DrawTreeMotion's motion transform: the components read into scalars and the z row's last
// two products parenthesised give retail's operand order in every row of q[0..2] (with
// v.x/v.y/v.z read directly the same bracketing leaves q[0].z x-first, 1120/1130).
inline void DebugTransformPointQ(Vec3* out, const Vec3& v, const Matrix4* m)
{
    float x = v.x, y = v.y, z = v.z;
    out->x = x * m->_11 + y * m->_21 + z * m->_31 + m->_41;
    out->y = x * m->_12 + y * m->_22 + z * m->_32 + m->_42;
    out->z = x * m->_13 + (y * m->_23 + z * m->_33) + m->_43;
}

// The capsule's two end points (0x00434540).  Retail orders the x87 operands of the two
// transforms differently: the start point's rows load x, y, z products as written (read
// through reference locals), the far end's y and z rows are bracketed x + (y + z) and its
// source comes in by pointer (with it by reference the x row's m->_11 loads after p1.x).
inline void DebugTransformCapsuleStart(Vec3* out, const Vec3& v, const Matrix4* m)
{
    const float& x = v.x;
    const float& y = v.y;
    const float& z = v.z;
    out->x = x * m->_11 + y * m->_21 + z * m->_31 + m->_41;
    out->y = x * m->_12 + y * m->_22 + z * m->_32 + m->_42;
    out->z = x * m->_13 + y * m->_23 + z * m->_33 + m->_43;
}

inline void DebugTransformCapsuleEnd(Vec3* out, const Vec3* p, const Matrix4* m)
{
    const Vec3& v = *p;
    const float& x = v.x;
    const float& y = v.y;
    const float& z = v.z;
    out->x = x * m->_11 + y * m->_21 + z * m->_31 + m->_41;
    out->y = x * m->_12 + (y * m->_22 + z * m->_32) + m->_42;
    out->z = x * m->_13 + (y * m->_23 + z * m->_33) + m->_43;
}

// v * M3x3 (no translation), like 0x0042a450.
inline void DebugRotateVector(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// 0x00433240 (ret 0x10).  Draws the tree `depth` levels down: at depth 0 a box node is
// drawn red and a triangle leaf blue.  Leaves above that depth are skipped.
void CollisionObject::DrawTree(CollisionTreeNode* node, int depth, const Matrix4* xf,
                               const Vec3* verts)
{
    if (depth == 0) {
        if (node->field_0x00 >= 0.0f) {
            SetDrawColor(0xff, 0, 0);
            DrawBox(&node->center, &node->halfExtents, xf);
        } else {
            SetDrawColor(0, 0, 0xff);
            const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
            Vec3 p[3];
            DebugTransformPoint(&p[0], verts[tri->vertex[0]], xf);
            DebugTransformPoint(&p[1], verts[tri->vertex[1]], xf);
            DebugTransformPoint(&p[2], verts[tri->vertex[2]], xf);
            DrawLine(&p[0], &p[1]);
            DrawLine(&p[1], &p[2]);
            DrawLine(&p[2], &p[0]);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawTree(node->child[0], depth - 1, xf, verts);
        DrawTree(node->child[1], depth - 1, xf, verts);
    }
}

// 0x004334c0 (ret 0x14).  Like DrawTree, but also shows where `motion` moves each node.
// A box is drawn, moved by 0x00428db0 and drawn again.  A triangle is drawn and each
// corner is joined (in red) to its image under `motion`.
void CollisionObject::DrawTreeMotion(CollisionTreeNode* node, int depth, const Matrix4* xf,
                                     const Matrix4* motion, const Vec3* verts)
{
    if (depth == 0) {
        if (node->field_0x00 < 0.0f) {
            SetDrawColor(0, 0, 0xff);
        } else {
            SetDrawColor(0xff, 0, 0);
        }
        Vec3 center = node->center;
        Vec3 halfExtents = node->halfExtents;
        if (node->field_0x00 >= 0.0f) {
            DrawBox(&center, &halfExtents, xf);
            MoveBox(&center, &halfExtents, motion);
            DrawBox(&center, &halfExtents, xf);
        } else {
            SetDrawColor(0, 0, 0xff);
            const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
            Vec3 p[3];
            Vec3 q[3];
            DebugTransformPoint(&p[0], verts[tri->vertex[0]], xf);
            DebugTransformPoint(&p[1], verts[tri->vertex[1]], xf);
            DebugTransformPoint(&p[2], verts[tri->vertex[2]], xf);
            // q reads p in place (no stack copy), hence the by-reference transform.
            DebugTransformPointQ(&q[0], p[0], motion);
            DebugTransformPointQ(&q[1], p[1], motion);
            DebugTransformPointQ(&q[2], p[2], motion);
            DrawLine(&p[0], &p[1]);
            DrawLine(&p[1], &p[2]);
            DrawLine(&p[2], &p[0]);
            SetDrawColor(0xff, 0, 0);
            DrawLine(&p[0], &q[0]);
            DrawLine(&p[1], &q[1]);
            DrawLine(&p[2], &q[2]);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawTreeMotion(node->child[0], depth - 1, xf, motion, verts);
        DrawTreeMotion(node->child[1], depth - 1, xf, motion, verts);
    }
}

// 0x00433930 (ret 0xc).  Draws every leaf point of a point tree in green.  With a motion
// matrix, each point is drawn with its moved image and a line between them.
void CollisionObject::DrawPointTree(CollisionTreeNode* node, const Matrix4* xf,
                                    const Matrix4* motion)
{
    if (node->field_0x00 >= 0.0f) {
        DrawPointTree(node->child[0], xf, motion);
        DrawPointTree(node->child[1], xf, motion);
    } else {
        Vec3 p = node->center;
        if (motion) {
            Vec3 q;
            DebugTransformPointRef(&q, p, motion);
            DebugTransformPoint(&p, node->center, xf);
            DebugTransformPoint(&q, q, xf);
            SetDrawColor(0, 0xff, 0);
            DrawLine(&p, &q);
            DrawMarker(&p, 0.05f);
            DrawMarker(&q, 0.05f);
        } else {
            SetDrawColor(0, 0xff, 0);
            DebugTransformPoint(&p, p, xf);
            DrawMarker(&p, 0.25f);
        }
    }
}

// 0x00433be0 (ret 0xc).  For every triangle leaf: its corners as green markers, its edges
// in blue, and a red line of length 0.1 from its centroid along its normal.
void CollisionObject::DrawTreeNormals(CollisionTreeNode* node, const Vec3* verts,
                                      const Matrix4* xf)
{
    if (node->field_0x00 >= 0.0f) {
        DrawTreeNormals(node->child[0], verts, xf);
        DrawTreeNormals(node->child[1], verts, xf);
    } else {
        const CollisionTreeTriangle* tri = (const CollisionTreeTriangle*)node;
        Vec3 a = verts[tri->vertex[0]];
        Vec3 b = verts[tri->vertex[1]];
        Vec3 c = verts[tri->vertex[2]];
        Vec3 center = (a + b + c) * (1.0f / 3.0f);
        Vec3 n = tri->normal;
        DebugRotateVector(&n, n, xf);
        SetDrawColor(0, 0xff, 0);
        DebugTransformPoint(&a, a, xf);
        DebugTransformPoint(&b, b, xf);
        DebugTransformPoint(&c, c, xf);
        DebugTransformPoint(&center, center, xf);
        n = center + n * 0.1f;   // retail reuses the normal's stack slot for the tip
        DrawMarker(&a, 0.1f);
        DrawMarker(&b, 0.1f);
        DrawMarker(&c, 0.1f);
        SetDrawColor(0, 0, 0xff);
        DrawLine(&a, &b);
        DrawLine(&b, &c);
        DrawLine(&c, &a);
        SetDrawColor(0xff, 0, 0);
        DrawLine(&center, &n);
    }
}

// 0x00434040 (ret 0xc).  Like DrawTree, for a point tree: at depth 0 a box node is drawn
// red and a leaf point as a magenta marker.
void CollisionObject::DrawBoxTree(CollisionTreeNode* node, int depth, const Matrix4* xf)
{
    if (depth == 0) {
        if (node->field_0x00 < 0.0f) {
            SetDrawColor(0, 0, 0xff);
        } else {
            SetDrawColor(0xff, 0, 0);
        }
        if (node->field_0x00 >= 0.0f) {
            DrawBox(&node->center, &node->halfExtents, xf);
        } else {
            SetDrawColor(0xff, 0, 0xff);
            Vec3 p = node->center;
            DebugTransformPoint(&p, p, xf);
            DrawMarker(&p, 0.25f);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawBoxTree(node->child[0], depth - 1, xf);
        DrawBoxTree(node->child[1], depth - 1, xf);
    }
}

// 0x004341b0 (ret 8).  Draws every leaf of a polyline (type 2) tree: a leaf holds a segment
// (its two end points at +0x04 / +0x10), drawn as two magenta markers joined by a green line.
// VC6 turns the second recursive call into the loop retail shows.
void CollisionObject::DrawSegmentTree(CollisionTreeNode* node, const Matrix4* xf)
{
    if (node->field_0x00 >= 0.0f) {
        DrawSegmentTree(node->child[0], xf);
        DrawSegmentTree(node->child[1], xf);
    } else {
        SetDrawColor(0xff, 0, 0xff);
        Vec3 p, q;
        const Vec3* ends = &((const CollisionTreeSegment*)node)->end[0];
        DebugTransformPoint(&p, ends[0], xf);
        DebugTransformPoint(&q, ends[1], xf);
        DrawMarker(&p, 0.25f);
        DrawMarker(&q, 0.25f);
        SetDrawColor(0, 0xff, 0);
        DrawLine(&p, &q);
    }
}

// 0x00434340 (ret 0xc).  DrawBoxTree for a polyline tree: at depth 0 a box node is drawn red
// and a segment leaf as in DrawSegmentTree.
void CollisionObject::DrawSegmentTreeLevel(CollisionTreeNode* node, int depth, const Matrix4* xf)
{
    if (depth == 0) {
        if (node->field_0x00 < 0.0f) {
            SetDrawColor(0, 0, 0xff);
        } else {
            SetDrawColor(0xff, 0, 0);
        }
        if (node->field_0x00 >= 0.0f) {
            DrawBox(&node->center, &node->halfExtents, xf);
        } else {
            SetDrawColor(0xff, 0, 0xff);
            Vec3 p, q;
            const Vec3* ends = &((const CollisionTreeSegment*)node)->end[0];
            DebugTransformPoint(&p, ends[0], xf);
            DebugTransformPoint(&q, ends[1], xf);
            DrawMarker(&p, 0.25f);
            DrawMarker(&q, 0.25f);
            SetDrawColor(0, 0xff, 0);
            DrawLine(&p, &q);
        }
    } else if (node->field_0x00 >= 0.0f) {
        DrawSegmentTreeLevel(node->child[0], depth - 1, xf);
        DrawSegmentTreeLevel(node->child[1], depth - 1, xf);
    }
}

// Normalise as slot 14 emits it: |v|^2 summed (y*y + x*x) + z*z, exactly 1.0f returns v,
// and the result goes through the out-of-line constructor 0x00404e60 (CollisionVec3's).
inline CollisionVec3 DebugNormalized(const CollisionVec3& v)
{
    float lengthSquared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (lengthSquared == 1.0f)
        return v;
    float inverseLength = FastInvSqrt(lengthSquared);
    return CollisionVec3(v.x * inverseLength, v.y * inverseLength, v.z * inverseLength);
}

// Device behind GameObject::renderContext as slot 14 drives it (render-state style calls with
// small enum arguments); tier 3 stand-in, only the slots used here.
class CollisionDebugDevice {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(int a, int b, int c);   // +0x1c
    virtual void UnknownVirtualSlot8(int a, int b, int c);   // +0x20
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10(int a, int b);         // +0x28
};

// Math3D views of the sphere / capsule payloads (same layouts as CollisionSphereShape and
// CollisionCapsuleShape in CollisionObject.h).
struct CollisionSphereView {
    Vec3 center;
    float radius;
    float radiusSquared;
    float radiusScale;      // +0x14
    float centerScale;      // +0x18
    Matrix4 transform;      // +0x1c
};
struct CollisionCapsuleView {
    Vec3 p0;
    Vec3 p1;
    float radius;           // +0x18
    float radiusSquared;
    float radiusScale;
    float endpointScale;
    Matrix4 transform;      // +0x28
};

// 0x00434540 (slot 14 of the GraphicsTest table, this = the GraphicsTest subobject): the
// collision debug overlay.  debugDrawMode 0 draws nothing, 5 adds the translucent bounding
// sphere; hulls and models draw their trees, a polyline its segment tree, a sphere a wire
// sphere and a capsule its two end spheres joined by ten rings along the axis.
// The capsule's end points go through DebugTransformCapsuleStart/End (see there); with one
// shared transform for both ends retail's operand orders are out of reach (1054-1058).
// The ring step is d / 10.0f: VC6 turns it into a multiply by the reciprocal constant
// 0x005511cc, a separate pool entry from the literal 0.1f at 0x005507d0.
int CollisionObject::UnknownVirtualSlot14()
{
    ((CollisionDebugDevice*)renderContext)->UnknownVirtualSlot8(0x1b, 0, 0);
    ((CollisionDebugDevice*)renderContext)->UnknownVirtualSlot10(7, 0);
    ((CollisionDebugDevice*)renderContext)->UnknownVirtualSlot7(0, 1, 1);
    Vec3 center;
    if (shape && debugDrawMode) {
        if (debugDrawMode == 5) {
            SetDrawRGBA(0, 0xff, 0xff, 0x80);
            BeginAlphaBlend();
            DrawSphere((const Vec3*)&this->boundCenter, boundRadius, 8);
        }
        switch (shapeType) {
        case 0:
            DrawHull((CollisionHullBody*)shape, debugTreeDepth, debugDrawMode);
            break;
        case 1:
            DrawModel((CollisionModelBody*)shape, debugTreeDepth, debugDrawMode);
            break;
        case 2:
            DrawSegmentTreeLevel((CollisionTreeNode*)((CollisionMeshBody*)shape)->field_0x04,
                                 debugTreeDepth, &((CollisionMeshBody*)shape)->field_0x08);
            DrawSegmentTree((CollisionTreeNode*)((CollisionMeshBody*)shape)->field_0x04,
                            &((CollisionMeshBody*)shape)->field_0x08);
            break;
        case 4: {
            SetDrawColor(0xff, 0xff, 0xff);
            CollisionSphereView* sphere = (CollisionSphereView*)shape;
            center = sphere->center * sphere->centerScale;
            DebugTransformPoint(&center, center, &sphere->transform);
            DrawSphere(&center, sphere->radius * sphere->radiusScale, 8);
            break;
        }
        case 3: {
            SetDrawColor(0xff, 0xff, 0xff);
            CollisionCapsuleView* capsule = (CollisionCapsuleView*)shape;
            Vec3 ends[2];
            DebugTransformCapsuleStart(&ends[0], capsule->p0, &capsule->transform);
            DebugTransformCapsuleEnd(&ends[1], &capsule->p1, &capsule->transform);
            DrawSphere(&ends[0], capsule->radius, 0x10);
            DrawSphere(&ends[1], ((CollisionCapsuleView*)shape)->radius, 0x10);
            Vec3 d = ends[1] - ends[0];
            CollisionVec3 axis = DebugNormalized(*(CollisionVec3*)&d);
            Vec3 step = d / 10.0f;
            for (int i = 0; i < 10; i++) {
                Vec3 ring = ends[0] + step * (float)i;
                DrawCircle(&ring, ((CollisionCapsuleView*)shape)->radius, (Vec3*)&axis, 0x20);
            }
            break;
        }
        }
    }
    field_0x98 = 0;
    return GameObject::UnknownVirtualSlot14();
}

// 0x00432d30 (ret 0xc).  Draws one hull's trees; `mode` (1..7) picks the view.  Mode 4
// also draws the tree under field_0x88 * field_0x108.
void CollisionObject::DrawHull(CollisionHullBody* hull, int depth, int mode)
{
    if (mode == 1) {
        DrawTree((CollisionTreeNode*)hull->triangleTree, depth, &hull->worldTransform,
                 (const Vec3*)hull->vertices);
    } else if (mode == 2) {
        if (hull->pointTree) {
            DrawBoxTree((CollisionTreeNode*)hull->pointTree, depth, &hull->worldTransform);
        }
    } else if (mode == 3) {
        DrawTreeMotion((CollisionTreeNode*)hull->triangleTree, depth, &hull->worldTransform,
                       &hull->motionTransform, (const Vec3*)hull->vertices);
    } else if (mode == 4) {
        DrawTreeMotion((CollisionTreeNode*)hull->triangleTree, depth, &hull->worldTransform,
                       &hull->motionTransform, (const Vec3*)hull->vertices);
        // m = field_0x88 * field_0x108 (row vectors), written out here.  Retail's term
        // order comes out only with the product in this function through local
        // references declared after m; an inline helper, or m declared last, gives a
        // different FPU order.
        Matrix4 m;
        const Matrix4& a = hull->localTransform;
        const Matrix4& b = hull->prevBodyTransform;
        m._11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
        m._12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
        m._13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
        m._14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
        m._21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
        m._22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
        m._23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
        m._24 = a._21 * b._14 + a._22 * b._24 + a._23 * b._34 + a._24 * b._44;
        m._31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
        m._32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
        m._33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
        m._34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
        m._41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
        m._42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
        m._43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
        m._44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
        DrawTree((CollisionTreeNode*)hull->triangleTree, depth, &m,
                 (const Vec3*)hull->vertices);
    } else if (mode == 5) {
        if (hull->pointTree) {
            DrawPointTree((CollisionTreeNode*)hull->pointTree, &hull->worldTransform,
                          &hull->motionTransform);
        }
    } else if (mode == 6) {
        if (hull->pointTree) {
            DrawPointTree((CollisionTreeNode*)hull->pointTree, &hull->worldTransform,
                          &hull->relativeFrame);
        }
    } else if (mode == 7) {
        DrawTreeNormals((CollisionTreeNode*)hull->triangleTree, (const Vec3*)hull->vertices,
                        &hull->worldTransform);
    }
}

// 0x00432b30 (ret 0xc).  Draws a model's bounds in green, then each element hull.  Mode 1
// draws each element's point-tree box, colored by the two per-element flag arrays.
void CollisionObject::DrawModel(CollisionModelBody* model, int depth, int mode)
{
    SetDrawColor(0, 0xff, 0);
    DrawBox((const Vec3*)&model->center, (const Vec3*)&model->halfExtents, &model->field_0x88);
    for (int i = 0; i < model->elementCount; i++) {
        if (mode == 1) {
            if (model->elements[i].pointTree) {
                SetDrawColor(0, 0, 0xff);
                if (!model->elementEnabled[i]) {
                    SetDrawColor(0xff, 0, 0);
                }
                if (model->elementHighlight[i]) {
                    SetDrawColor(0, 0xff, 0);
                }
                CollisionTreeNode* box = (CollisionTreeNode*)model->elements[i].pointTree;
                DrawBox(&box->center, &box->halfExtents, &model->elements[i].worldTransform);
            }
        } else if (mode == 2) {
            if (model->elements[i].pointTree) {
                DrawTree((CollisionTreeNode*)model->elements[i].triangleTree, depth, &model->elements[i].worldTransform,
                         (const Vec3*)model->elements[i].vertices);
            }
        } else if (mode == 3) {
            if (model->elements[i].pointTree) {
                DrawBoxTree((CollisionTreeNode*)model->elements[i].pointTree, depth, &model->elements[i].worldTransform);
            }
        } else if (mode == 4) {
            DrawTreeMotion((CollisionTreeNode*)model->elements[i].triangleTree, depth, &model->elements[i].worldTransform,
                           &model->elements[i].motionTransform, (const Vec3*)model->elements[i].vertices);
        } else if (mode == 5) {
            if (model->elements[i].pointTree) {
                DrawPointTree((CollisionTreeNode*)model->elements[i].pointTree, &model->elements[i].worldTransform,
                              &model->elements[i].motionTransform);
            }
        } else if (mode == 6) {
            if (model->elements[i].pointTree) {
                DrawPointTree((CollisionTreeNode*)model->elements[i].pointTree, &model->elements[i].worldTransform, 0);
            }
        } else if (mode == 7) {
            DrawTreeNormals((CollisionTreeNode*)model->elements[i].triangleTree,
                            (const Vec3*)model->elements[i].vertices, &model->elements[i].worldTransform);
        }
    }
}

// ==============================================================================================
// Section: CollisionShapeTests
// ==============================================================================================
// CollisionShapeTests.cpp -- narrow-phase shape tests between CollisionObject payloads.
//
// Translation-unit ownership is PROVISIONAL (tier 3): the file split is chosen for
// readability.  Retail has no source string in 0x00436100-0x004389b0 (nearest __FILE__ is
// CollisionPoint.cpp at 0x0043a346), so these functions may belong to
// CollisionObject.cpp, CollisionCharacter.cpp or a separate shape-test unit.

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

// Collision queries over the broad phase (0x00438e70).
// Vegetation patch as 0x00438e70 sees a quadtree object whose objectTypeId is Vegetation's:
// cell coordinates at +0x0c / +0x10 and thiscall accessors (tier 3 stand-in).
class CollisionVegetation : public QuadTreeObject {
public:
    float GetRadius();                        // 0x00457230
    int GetObjectCount();                     // 0x00457080
    CollisionObject* GetObject(int index);    // 0x004570a0 (ret 4)
    unsigned short cellX;                     // +0x0c
    unsigned short field_0x0e;
    unsigned short cellZ;                     // +0x10
};
struct CollisionVegetationGrid {   // object at *0x0059aebc (tier 3): a view of EcoSystem (g_EcoSystem)
    char field_0x00[0x5a8];
    float cellSize;                // +0x5a8 world size of one vegetation cell
};
extern CollisionVegetationGrid* g_vegetationGrid;   // 0x0059aebc
struct CollisionGameContext {      // object at *0x0056e26c (tier 3; root of the object tree at +0x34)
    char field_0x00[0x34];
    GameObject* root;              // +0x34
};
// gameobj.cpp's iterator (src/reconstructed/GameObjectIterator.h; local stand-in, 0x94 bytes).
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter);   // 0x00469950
    ~GameObjectIterator();                                                  // 0x00469a40
    GameObject* Next();                                                     // 0x00469a50
private:
    char field_0x00[0x94];
};
struct CollisionBoundingSphere;
int SphereContainsPointXZ(const CollisionBoundingSphere* sphere, const Vec3* p, float r);   // 0x0043a1e0
int BoundingSpheresOverlap(const CollisionBoundingSphere* a, const CollisionBoundingSphere* b);   // 0x0043a270

// The hit bookkeeping 0x00438e70 repeats after every successful TestAgainst(hitObject).
inline void CollisionObject::ReportHit()
{
    hasContact = 1;
    StoreHitResult();
    hitObject->hitPoint = hitPoint;
    hitObject->hitNormal = hitNormal;
    if (onHitCallback)
        onHitCallback(this, hitObject);
    if (hitObject->onHitByCallback)
        hitObject->onHitByCallback(hitObject, this);
}

// 0x00438e70: tests this object against everything near it.  With the global collision
// quadtree (and useBroadphase) the candidates are the quadtree objects overlapping the world
// bounds: collision objects (bounding spheres first) and vegetation patches (their objects,
// after an XZ circle test); without it every CollisionObject of the game-object tree.
// Returns hasContact.
int CollisionObject::QueryCollisions()
{
    if (!collisionEnabled) {
        hasContact = 0;
        return 0;
    }
    ResetHitState();
    if (g_collisionQuadTree && useBroadphase) {
        field_0x98 = 1;
        CollisionVec3 maxBounds;
        CollisionVec3 minBounds;
        GetWorldBounds(&minBounds, &maxBounds);
        g_collisionQuadTree->BeginQuery(minBounds.x, minBounds.z, maxBounds.x, maxBounds.z);
        for (QuadTreeObject* o = g_collisionQuadTree->NextObject(); o; o = g_collisionQuadTree->NextObject()) {
            if (o->objectTypeId == g_CollisionObjectTypeId) {
                hitObject = (CollisionObject*)o;
                hitObject->field_0x98 = 1;
                if (BoundingSpheresOverlap((CollisionBoundingSphere*)this, (CollisionBoundingSphere*)hitObject)
                    && TestAgainst(hitObject))
                    ReportHit();
            } else if (!ignoreVegetation && o->objectTypeId == g_VegetationTypeId) {
                CollisionVegetation* vegetation = (CollisionVegetation*)o;
                Vec3 center;
                center.x = vegetation->cellX * g_vegetationGrid->cellSize;
                center.z = vegetation->cellZ * g_vegetationGrid->cellSize;
                float radius = vegetation->GetRadius();   // unused: retail calls it and pops the result
                if (SphereContainsPointXZ((CollisionBoundingSphere*)this, &center, vegetation->GetRadius())) {
                    for (int i = 0; i < vegetation->GetObjectCount(); i++) {
                        hitObject = vegetation->GetObject(i);
                        hitObject->field_0x98 = 1;
                        if (TestAgainst(hitObject))
                            ReportHit();
                    }
                }
            }
        }
        g_collisionQuadTree->EndQuery();
    } else {
        field_0x98 = 1;
        GameObjectIterator* it = new(__FILE__, 0x7da) GameObjectIterator(reinterpret_cast<CollisionGameContext*>(g_TrackGame)->root, 1, "");
        // Retail null-checks the CollisionObject* and then the GameObject* again (three tests
        // per step), which is this round trip through the derived pointer type.
        GameObject* object;
        while ((object = (CollisionObject*)it->Next()) != 0) {
            hitObject = dynamic_cast<CollisionObject*>(object);
            if (hitObject) {
                hitObject->field_0x98 = 1;
                if (TestAgainst(hitObject))
                    ReportHit();
            }
        }
        delete it;
    }
    if (hasContact)
        StoreHitNormal();
    return hasContact;
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
    if ((other->isEnabled) == 0)
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
    case 4:
        return HullVsSphere(mine, (CollisionSphereShape*)other->shape);
    case 3:
        return HullVsCapsule(mine, (CollisionCapsuleShape*)other->shape);
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
    case 4:
        return ModelVsSphere(mine, (CollisionSphereShape*)other->shape);
    case 3:
        return ModelVsCapsule(mine, (CollisionCapsuleShape*)other->shape);
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
    return SphereTreeQuery(reinterpret_cast<const Vec3*>(&world), r, rr,
        reinterpret_cast<QueryTreeNode*>(hull->pointTree), &hull->worldTransform, 0);
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
    return CapsuleTreeQueryWithVertices(reinterpret_cast<const Vec3*>(ends), r, r * r,
        reinterpret_cast<QueryTreeNode*>(hull->pointTree), &hull->worldTransform, 0,
        reinterpret_cast<Vec3*>(hull->vertices));
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
    if (BoxSphereOverlap(reinterpret_cast<const Vec3*>(&model->center),
        reinterpret_cast<const Vec3*>(&model->halfExtents), reinterpret_cast<const Vec3&>(world), r, rr,
        &model->field_0x88)) {
        for (int i = 0; i < model->elementCount; i++) {
            if (SphereTreeQuery(reinterpret_cast<const Vec3*>(&world), r, rr,
                reinterpret_cast<QueryTreeNode*>(model->elements[i].pointTree),
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
    if (BoxCapsuleOverlap(&model->center, &model->halfExtents, ends, r, rr, &model->field_0x88)) {
        for (int i = 0; i < model->elementCount; i++) {
            if (CapsuleTreeQueryWithVertices(reinterpret_cast<const Vec3*>(ends), r, rr,
                reinterpret_cast<QueryTreeNode*>(model->elements[i].pointTree),
                &model->elements[i].worldTransform, 0, reinterpret_cast<Vec3*>(model->elements[i].vertices)))
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

// The query's contact position as a Math3D Vec3 (inline constructor and operators; the
// model tests below accumulate and average it with them).
static inline Vec3& QueryContact(CollisionSweepQuery* q)
{
    return *(Vec3*)&q->contact;
}

// 0x00437c20: model vs hull.  Forwards to ModelVsHullSwept when model->swept is set;
// otherwise rel = inverse(model+0x88) * hull+0x48 and each enabled element is tested
// against the hull with the box-box test 0x00436720; the contact is the mean of the hits.
int CollisionObject::ModelVsHull(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return ModelVsHullSwept(a, b, q);

    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigid(&rel, &a->field_0x88);
    MatrixMultiply(&rel, rel, b->worldTransform);
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(a->center), reinterpret_cast<const Vec3&>(a->halfExtents),
        reinterpret_cast<const Vec3*>(&b->triangleTree->center),
        reinterpret_cast<const Vec3*>(&b->triangleTree->halfExtents), &rel, &a->field_0xc8)) {
        Vec3 sum;
        sum = Vec3(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < a->elementCount; i++) {
            if (a->elementEnabled[i]) {
                CollisionHullBody* e = &a->elements[i];
                if (HullVsHull(e, b, q)) {
                    sum += QueryContact(q);
                    hit = 1;
                    count++;
                }
            }
        }
        if (hit) {
            Vec3 mean;
            float s;
            mean.x = sum.x * (s = 1.0f / count);
            mean.y = sum.y * s;
            mean.z = sum.z * s;
            QueryContact(q) = mean;
            // Retail scales x as fld st(0); fmul sum.x and y/z component
            // first: s assigned inside the x product gives that order
            // (sum * (1.0f / count), a Vec3 ctor, member operators and a
            // by-value helper all scale x component first).  The copy
            // registers (retail eax/ecx/edx) still differ.
        }
        return hit;
    }
    return 0;
}

// 0x00437ef0: swept model vs hull.  Like ModelVsHull, but the relative frame uses the hull's
// +0xc8 matrix, a copy of the model box (centre, half extents) is grown in place by
// 0x004290d0 and cached in the model at +0x108/+0x114, and each enabled element is tested
// with the swept hull test 0x00436af0.
int CollisionObject::ModelVsHullSwept(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigid(&rel, &a->field_0x88);
    MatrixMultiply(&rel, rel, b->bodyTransform);
    CollisionVec3 center = a->center;
    CollisionVec3 half = a->halfExtents;
    GrowBoxByTransformedBox(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&half),
        reinterpret_cast<const Vec3&>(a->field_0x30), reinterpret_cast<const Vec3&>(a->field_0x3c),
        &a->field_0xc8);
    a->sweptCenter = center;
    a->sweptHalfExtents = half;
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(center), reinterpret_cast<const Vec3&>(half),
        reinterpret_cast<const Vec3*>(&b->triangleTree->center),
        reinterpret_cast<const Vec3*>(&b->triangleTree->halfExtents), &rel, &a->field_0xc8)) {
        Vec3 sum;
        sum = Vec3(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < a->elementCount; i++) {
            if (a->elementEnabled[i]) {
                CollisionHullBody* e = &a->elements[i];
                if (HullVsHullSwept(e, b, q)) {
                    sum += QueryContact(q);
                    hit = 1;
                    count++;
                }
            }
        }
        if (hit) {
            Vec3 mean;
            float s;
            mean.x = sum.x * (s = 1.0f / count);
            mean.y = sum.y * s;
            mean.z = sum.z * s;
            QueryContact(q) = mean;
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

    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigid(&rel, &a->field_0x88);
    MatrixMultiply(&rel, rel, b->field_0x88);
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(a->center), reinterpret_cast<const Vec3&>(a->halfExtents),
        reinterpret_cast<const Vec3*>(&b->center), reinterpret_cast<const Vec3*>(&b->halfExtents), &rel,
        &a->field_0xc8)) {
        Vec3 sum;
        sum = Vec3(0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int i = 0; i < b->elementCount; i++) {
            CollisionHullBody* e = &b->elements[i];
            if (ModelVsHull(a, e, q)) {
                sum += QueryContact(q);
                hit = 1;
                count++;
            }
        }
        if (hit) {
            Vec3 mean;
            float s;
            mean.x = sum.x * (s = 1.0f / count);
            mean.y = sum.y * s;
            mean.z = sum.z * s;
            QueryContact(q) = mean;
        }
        return hit;
    }
    return 0;
}

// 0x00438550: swept model vs model: as ModelVsModel with a's swept bounds (0x004290d0) and
// ModelVsHullSwept per element of b, except that the result is the last element's contact
// (copied out on each hit and back at the end), not the mean.
// The rigid inverse as ModelVsModelSwept (0x00438550) expands it: the x translation term
// pairs differ from CollisionInvertRigid, y follows CollisionInvertRigidRelFrame.
static inline void CollisionInvertRigidModel(Matrix4* out, const Matrix4* src)
{
    *out = *src;
    float t;
    t = out->m[0][1]; out->m[0][1] = out->m[1][0]; out->m[1][0] = t;
    t = out->m[0][2]; out->m[0][2] = out->m[2][0]; out->m[2][0] = t;
    t = out->m[1][2]; out->m[1][2] = out->m[2][1]; out->m[2][1] = t;
    CollisionVec3 p;
    p.x = -((out->m[3][2] * out->m[2][0] + out->m[3][0] * out->m[0][0]) + out->m[3][1] * out->m[1][0]);
    p.y = -((out->m[3][1] * out->m[1][1] + out->m[3][0] * out->m[0][1]) + out->m[3][2] * out->m[2][1]);
    p.z = -((out->m[3][2] * out->m[2][2] + out->m[3][1] * out->m[1][2]) + out->m[3][0] * out->m[0][2]);
    out->m[3][0] = p.x;
    out->m[3][1] = p.y;
    out->m[3][2] = p.z;
}
int CollisionObject::ModelVsModelSwept(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigidModel(&rel, &a->field_0x88);
    MatrixMultiply(&rel, rel, b->field_0x88);
    CollisionVec3 center = a->center;
    CollisionVec3 half = a->halfExtents;
    GrowBoxByTransformedBox(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&half),
        reinterpret_cast<const Vec3&>(a->field_0x30), reinterpret_cast<const Vec3&>(a->field_0x3c),
        &a->field_0xc8);
    a->sweptCenter = center;
    a->sweptHalfExtents = half;
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(center), reinterpret_cast<const Vec3&>(half),
        reinterpret_cast<const Vec3*>(&b->center), reinterpret_cast<const Vec3*>(&b->halfExtents), &rel,
        &a->field_0xc8)) {
        Vec3 last;
        last = Vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < b->elementCount; i++) {
            CollisionHullBody* e = &b->elements[i];
            if (ModelVsHullSwept(a, e, q)) {
                hit = 1;
                last = QueryContact(q);
            }
        }
        if (hit)
            QueryContact(q) = last;
        return hit;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Hull vs hull.  Both call the oriented-box test 0x00428950, which leaves contact points in
// the global scratch list (g_CollisionScratchPoints[0 .. g_CollisionScratchCount)) and a hit
// count in query->contactCount, then bring the results back into world space.
// ---------------------------------------------------------------------------

// Inline forms of the out-of-line helpers 0x0042a4b0 (M3x3 * v), 0x0042a450 (v * M3x3) and
// 0x0042a510 (v * M + T),
// written as BoundingBoxTreeQuery.cpp writes them; the hull tests expand them in place.
inline void CollisionRotateRowsTo(CollisionVec3* out, CollisionVec3 v, const Matrix4* m)
{
    out->x = v.x * m->m[0][0] + v.y * m->m[0][1] + v.z * m->m[0][2];
    out->y = v.x * m->m[1][0] + v.y * m->m[1][1] + v.z * m->m[1][2];
    out->z = v.x * m->m[2][0] + v.y * m->m[2][1] + v.z * m->m[2][2];
}

inline void CollisionRotateColsTo(CollisionVec3* out, CollisionVec3 v, const Matrix4* m)
{
    out->x = v.x * m->m[0][0] + v.y * m->m[1][0] + v.z * m->m[2][0];
    out->y = v.x * m->m[0][1] + v.y * m->m[1][1] + v.z * m->m[2][1];
    out->z = v.x * m->m[0][2] + v.y * m->m[1][2] + v.z * m->m[2][2];
}

inline void CollisionTransformPointTo(CollisionVec3* out, CollisionVec3 v, const Matrix4* m)
{
    out->x = v.x * m->m[0][0] + v.y * m->m[1][0] + v.z * m->m[2][0] + m->m[3][0];
    out->y = v.x * m->m[0][1] + v.y * m->m[1][1] + v.z * m->m[2][1] + m->m[3][1];
    out->z = v.x * m->m[0][2] + v.y * m->m[1][2] + v.z * m->m[2][2] + m->m[3][2];
}

// The scratch loop's form of CollisionRotateRowsTo: its y and z rows add the x and z terms
// first, which keeps retail's x, z, y product order in the loop (HullVsHull 0x00436720).
inline void CollisionRotateRowsGrouped(CollisionVec3* out, CollisionVec3 v, const Matrix4* m)
{
    out->x = v.x * m->m[0][0] + v.y * m->m[0][1] + v.z * m->m[0][2];
    out->y = v.y * m->m[1][1] + (v.z * m->m[1][2] + v.x * m->m[1][0]);
    out->z = v.y * m->m[2][1] + (v.z * m->m[2][2] + v.x * m->m[2][0]);
}

// Negation and scaling through the out-of-line constructor 0x00404e60, as retail calls it.
inline CollisionVec3 operator-(const CollisionVec3& v)
{
    return CollisionVec3(-v.x, -v.y, -v.z);
}

inline CollisionVec3 CollisionScaled(const CollisionVec3& v, float s)
{
    return CollisionVec3(v.x * s, v.y * s, v.z * s);
}

// M3x3 * -v in place; the negated components stay on the x87 stack (no constructor call).
inline void CollisionRotateRowsNegated(CollisionVec3* v, const Matrix4* m)
{
    float x = -v->x;
    float y = -v->y;
    float z = -v->z;
    v->x = z * m->m[0][2] + y * m->m[0][1] + x * m->m[0][0];
    v->y = z * m->m[1][2] + y * m->m[1][1] + x * m->m[1][0];
    v->z = z * m->m[2][2] + y * m->m[2][1] + x * m->m[2][0];
}

// v * s with the three products computed before the result is filled; VC6 then spills the
// z product and stores x and y after it, as retail does (docs/VC6_OPERAND_ORDER.md 1.3).
inline CollisionVec3 CollisionScaledProducts(const CollisionVec3& v, float s)
{
    float x = v.x * s;
    float y = v.y * s;
    float z = v.z * s;
    CollisionVec3 r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
}

// 0x00436720: hull A vs hull B, non-swept.  Forwards to HullVsHullSwept when a->swept is
// set.  The query vectors and the scratch points are first rotated into B's frame, the
// box test runs with mode 2, and on a hit the contact point is averaged over contactCount
// and transformed back; the vectors/points are always rotated back afterwards.
// Near miss (936/962, same size, frame and registers as retail).  The product order of each
// inlined rotation follows the leaf age of its matrix operand: the first rotation reads
// &b->worldTransform directly (retail z, y, x), the scratch loop reads it through xf after
// xf's first use (retail x, z, y, with its y and z rows grouped x + z first).  Left: the
// second forward rotation's x row sums x, z, y (retail z, y, x); no term order or grouping
// of that row fixes it without breaking the loop.
int CollisionObject::HullVsHull(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    if (a->swept)
        return HullVsHullSwept(a, b, q);

    CollisionRotateRowsTo(&q->contactOffset, q->contactOffset, &b->worldTransform);
    const Matrix4* xf = &b->worldTransform;
    CollisionRotateRowsTo(&q->contactNormal, q->contactNormal, xf);
    CollisionVec3 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    q->contact = zero;
    q->contactCount = 0;
    for (int i = 0; i < g_CollisionScratchCount; i++)
        CollisionRotateRowsGrouped(&g_CollisionScratchPoints[i], g_CollisionScratchPoints[i], xf);

    int hit = TreeTreeQuery(reinterpret_cast<QueryTreeNode*>(a->pointTree),
        reinterpret_cast<QueryTreeNode*>(b->triangleTree), &a->worldTransform, &b->worldTransform, 2,
        reinterpret_cast<Vec3*>(b->vertices), &a->motionTransform);
    if (hit) {
        float s = 1.0f / q->contactCount;
        q->contact = CollisionScaledProducts(q->contact, s);
        CollisionTransformPointTo(&q->contact, q->contact, &b->worldTransform);
    }
    CollisionRotateColsTo(&q->contactOffset, q->contactOffset, &b->worldTransform);
    CollisionRotateColsTo(&q->contactNormal, q->contactNormal, &b->worldTransform);
    for (int j = 0; j < g_CollisionScratchCount; j++)
        Vec3TransformNormal(reinterpret_cast<Vec3*>(&g_CollisionScratchPoints[j]),
            reinterpret_cast<const Vec3&>(g_CollisionScratchPoints[j]), &b->worldTransform);
    return hit;
}

// 0x00436af0: swept hull test (a is the moving hull).  The stale scratch points are rotated
// by a's matrix, the query vectors are negated and rotated into a's frame, the relative
// frame of b over a's two history matrices is built with 0x00432260 into b+0x148, and the
// box test runs with b as the first shape.  On a hit the contact is averaged and moved by
// a's matrix; the query vectors and scratch points are always rotated back.
// Near miss (809/852, same size, frame 0x18 and registers as retail).  As in HullVsHull the
// rotations' product orders follow whether they read the matrix through xf or directly, and
// contactCount is cleared after the relative frame is built.  Left: the zero-vector stores
// and the contactCount store are scheduled into the neighbouring code in another order
// (statement placement sweeps do not move them).
int CollisionObject::HullVsHullSwept(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q)
{
    for (int i = 0; i < g_CollisionScratchCount; i++)
        CollisionRotateRowsTo(&g_CollisionScratchPoints[i], g_CollisionScratchPoints[i], &a->worldTransform);
    const Matrix4* xf = &a->worldTransform;
    CollisionRotateRowsNegated(&q->contactOffset, xf);
    CollisionRotateRowsNegated(&q->contactNormal, &a->worldTransform);
    CollisionVec3 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    q->contact = zero;
    CollisionRelativeFrame(&b->relativeFrame, &b->worldTransform, &a->bodyTransform, &a->prevBodyTransform);

    q->contactCount = 0;
    int hit;
    if (!TreeTreeQuery(reinterpret_cast<QueryTreeNode*>(b->pointTree),
        reinterpret_cast<QueryTreeNode*>(a->triangleTree), &b->worldTransform, xf, 2,
        reinterpret_cast<Vec3*>(a->vertices), &b->relativeFrame))
        hit = 0;
    else {
        hit = 1;
        float s = 1.0f / q->contactCount;
        q->contact = CollisionScaled(q->contact, s);
        CollisionTransformPointTo(&q->contact, q->contact, &a->worldTransform);
    }
    Vec3TransformNormal(reinterpret_cast<Vec3*>(&q->contactOffset),
        reinterpret_cast<const Vec3&>(-q->contactOffset), &a->worldTransform);
    Vec3TransformNormal(reinterpret_cast<Vec3*>(&q->contactNormal),
        reinterpret_cast<const Vec3&>(-q->contactNormal), &a->worldTransform);
    for (int j = 0; j < g_CollisionScratchCount; j++)
        Vec3TransformNormal(reinterpret_cast<Vec3*>(&g_CollisionScratchPoints[j]),
            reinterpret_cast<const Vec3&>(g_CollisionScratchPoints[j]), &a->worldTransform);
    return hit;
}

// 0x004376f0: swept hull A vs model B.  rel = inverse(a+0xc8) * b+0x88, broad phase between
// a's box bounds (a+0x188) and the model bounds, then every enabled element of b is tested
// with HullVsHullSwept(a, element, q) and the contacts are averaged into q->contact.
// The call to 0x004fc9a0 (node position, result stored to a dead local) is kept as decoded.
int CollisionObject::HullVsModelSwept(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q)
{
    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigid(&rel, &a->bodyTransform);
    MatrixMultiply(&rel, rel, b->field_0x88);
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(a->triangleTree->center),
        reinterpret_cast<const Vec3&>(a->triangleTree->halfExtents), reinterpret_cast<const Vec3*>(&b->center),
        reinterpret_cast<const Vec3*>(&b->halfExtents), &rel, &a->motionTransform)) {
        Vec3 sum;
        sum = Vec3(0.0f, 0.0f, 0.0f);
        CollisionVec3 nodePos;
        int count = 0;
        // Retail makes this call (0x004fc9a0); nodePos is never read.
        reinterpret_cast<SoultreeObject*>(a->sceneNode)->GetPositionIn(
            0, reinterpret_cast<Vec3*>(&nodePos));
        for (int i = 0; i < b->elementCount; i++) {
            if (b->elementEnabled[i]) {
                CollisionHullBody* e = &b->elements[i];
                if (HullVsHullSwept(a, e, q)) {
                    sum += QueryContact(q);
                    hit = 1;
                    count++;
                }
            }
        }
        if (hit) {
            Vec3 mean;
            float s;
            mean.x = sum.x * (s = 1.0f / count);
            mean.y = sum.y * s;
            mean.z = sum.z * s;
            QueryContact(q) = mean;
        }
        return hit;
    }
    return 0;
}

// The 4x4 product of MatrixMultiply (0x0042a1a0) expanded inline with its by-value
// operands (same text as the out-of-line body; *out = b * a in the row-vector convention).
// HullVsModel (0x00436e50) inlines it; the operand aliasing the output is copied first.
static inline void MatrixMultiplyInline(Matrix4* out, Matrix4 a, Matrix4 b)
{
    out->_11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
    out->_12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
    out->_13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
    out->_14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
    out->_21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
    out->_22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
    out->_23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
    out->_24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
    out->_31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
    out->_32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
    out->_33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
    out->_34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
    out->_41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
    out->_42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
    out->_43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
    out->_44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
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

    int hit = 0;
    Matrix4 rel;
    CollisionInvertRigidRelFrame(&rel, &a->bodyTransform);
    MatrixMultiplyInline(&rel, rel, b->field_0x88);
    CollisionBoxBounds* tree = a->triangleTree;
    if (SweptBoxOverlap(reinterpret_cast<const Vec3&>(tree->center),
        reinterpret_cast<const Vec3&>(tree->halfExtents), reinterpret_cast<const Vec3*>(&b->center),
        reinterpret_cast<const Vec3*>(&b->halfExtents), &rel, &a->motionTransform)) {
        Vec3 sum = Vec3(0.0f, 0.0f, 0.0f);
        CollisionVec3 nodePos;
        int count = 0;
        // Retail makes this call (0x004fc9a0); nodePos is never read.
        reinterpret_cast<SoultreeObject*>(a->sceneNode)->GetPositionIn(
            0, reinterpret_cast<Vec3*>(&nodePos));
        for (int i = 0; i < b->elementCount; i++) {
            // Retail makes this call too, for each element; the result is not read.
            CollisionVec3 elementPos;
            reinterpret_cast<SoultreeObject*>(b->elements[i].sceneNode)->GetPositionIn(
            0, reinterpret_cast<Vec3*>(&elementPos));
            if (!b->elementEnabled[i])
                continue;
            CollisionHullBody* e = &b->elements[i];
            const Matrix4* xf = &e->worldTransform;
            CollisionRotateRowsTo(&q->contactOffset, q->contactOffset, xf);
            CollisionRotateRowsTo(&q->contactNormal, q->contactNormal, xf);
            q->contact = CollisionVec3(0.0f, 0.0f, 0.0f);
            q->contactCount = 0;
            for (int j = 0; j < g_CollisionScratchCount; j++)
                CollisionRotateRowsTo(&g_CollisionScratchPoints[j], g_CollisionScratchPoints[j], xf);
            if (TreeTreeQuery(reinterpret_cast<QueryTreeNode*>(a->pointTree),
                reinterpret_cast<QueryTreeNode*>(e->triangleTree), &a->worldTransform, xf, 2,
                reinterpret_cast<Vec3*>(e->vertices), &a->motionTransform)) {
                CollisionVec3 mean;
                q->contact = *CollisionDivide(&mean, &q->contact, (float)q->contactCount);
                CollisionTransformPointTo(&q->contact, q->contact, xf);
                sum += *(Vec3*)&q->contact;
                hit = 1;
                count++;
            }
            CollisionRotateColsTo(&q->contactOffset, q->contactOffset, xf);
            CollisionRotateColsTo(&q->contactNormal, q->contactNormal, xf);
            for (int k = 0; k < g_CollisionScratchCount; k++)
                CollisionRotateColsTo(&g_CollisionScratchPoints[k], g_CollisionScratchPoints[k], xf);
        }
        if (hit) {
            CollisionVec3 mean;
            q->contact = *CollisionDivide(&mean, (const CollisionVec3*)&sum, (float)count);
        }
        return hit;
    }
    return 0;
}

inline void CollisionDiagonal(float d[3][3], float x, float y, float z)
{
    d[0][0] = x; d[0][1] = 0.0f; d[0][2] = 0.0f;
    d[1][0] = 0.0f; d[1][1] = y; d[1][2] = 0.0f;
    d[2][0] = 0.0f; d[2][1] = 0.0f; d[2][2] = z;
}

// r = d * M (upper 3x3), fully unrolled.
inline void CollisionMul3x3(float r[3][3], const float d[3][3], const Matrix4& m)
{
    r[0][0] = d[0][0] * m.m[0][0] + d[0][1] * m.m[1][0] + d[0][2] * m.m[2][0];
    r[0][1] = d[0][0] * m.m[0][1] + d[0][1] * m.m[1][1] + d[0][2] * m.m[2][1];
    r[0][2] = d[0][0] * m.m[0][2] + d[0][1] * m.m[1][2] + d[0][2] * m.m[2][2];
    r[1][0] = d[1][0] * m.m[0][0] + d[1][1] * m.m[1][0] + d[1][2] * m.m[2][0];
    r[1][1] = d[1][0] * m.m[0][1] + d[1][1] * m.m[1][1] + d[1][2] * m.m[2][1];
    r[1][2] = d[1][0] * m.m[0][2] + d[1][1] * m.m[1][2] + d[1][2] * m.m[2][2];
    r[2][0] = d[2][0] * m.m[0][0] + d[2][1] * m.m[1][0] + d[2][2] * m.m[2][0];
    r[2][1] = d[2][0] * m.m[0][1] + d[2][1] * m.m[1][1] + d[2][2] * m.m[2][1];
    r[2][2] = d[2][0] * m.m[0][2] + d[2][1] * m.m[1][2] + d[2][2] * m.m[2][2];
}

// 0x00436100: world-space axis-aligned bounds of this object's local box (Arvo transformed
// AABB): center' = center * M + T, extent_j = sum_i |M[i][j]| * half_i, output min = center'
// - extent and max = center' + extent.  The box and matrix come from the shape payload:
// hull (0): +0x48 and bounds object +0x188; model (1): +0x88 with center/half at +0x18/+0x24;
// mesh (2): +0x08 with bounds object +0x04.  For any other shape type retail falls through
// with uninitialised locals; that is reproduced (locals left unset).
// Retail forms diag(half) as a 3x3 with its zeros as stack temporaries and multiplies it
// by M fully unrolled.  The center is one sum per component written x, y, z, translation
// (CollisionTransformPoint); VC6 emits the z product first.  Accumulating it one term per
// statement gives the same products but schedules the diagonal's zero stores and the pop
// edi/esi pair elsewhere (727).  798/804: two of the zero temporaries trade stack slots.
void CollisionObject::GetWorldBounds(CollisionVec3* outMin, CollisionVec3* outMax)
{
    Matrix4 m;
    const CollisionVec3* c;
    const CollisionVec3* h;
    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)shape;
        c = &hull->triangleTree->center;
        h = &hull->triangleTree->halfExtents;
        m = hull->worldTransform;
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)shape;
        c = &model->center;
        h = &model->halfExtents;
        m = model->field_0x88;
        break;
    }
    case 2: {
        CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
        c = &mesh->field_0x04->center;
        h = &mesh->field_0x04->halfExtents;
        m = mesh->field_0x08;
        break;
    }
    }
    float hm[3][3];
    CollisionDiagonal(hm, h->x, h->y, h->z);
    CollisionVec3 center = CollisionTransformPoint(*c, &m);
    float r[3][3];
    CollisionMul3x3(r, hm, m);
    float ex = CollisionAbs(r[0][0]) + CollisionAbs(r[1][0]) + CollisionAbs(r[2][0]);
    float ey = CollisionAbs(r[0][1]) + CollisionAbs(r[1][1]) + CollisionAbs(r[2][1]);
    float ez = CollisionAbs(r[0][2]) + CollisionAbs(r[1][2]) + CollisionAbs(r[2][2]);
    outMin->x = center.x - ex;
    outMin->y = center.y - ey;
    outMin->z = center.z - ez;
    outMax->x = center.x + ex;
    outMax->y = center.y + ey;
    outMax->z = center.z + ez;
}

// v * M3x3 written back through `out` from a by-value copy of v (retail copies the vector to
// the stack before the three rows; the x row's terms are emitted x, y, z).
inline void CollisionRotateColsInPlace(CollisionVec3* out, CollisionVec3 v, const Matrix4* m)
{
    out->x = v.x * m->m[0][0] + v.y * m->m[1][0] + v.z * m->m[2][0];
    out->y = v.z * m->m[2][1] + v.y * m->m[1][1] + v.x * m->m[0][1];
    out->z = v.z * m->m[2][2] + v.y * m->m[1][2] + v.x * m->m[0][2];
}

// 0x00438c90: this is a static mesh (type 2); test it against a hull (0) or a model (1).
// The mesh's convex geometry (+4) and matrix (+8) go through the box test 0x00428950 with
// mode 1; on a hit the result vec3 at globalResult+8 is rotated by the hull matrix (v * M).
// For a model, 0x004392c0 rejects first and every enabled element is tested.  Retail forms
// the element address before reading its enabled flag.
int CollisionObject::TestMeshAgainst(CollisionObject* other)
{
    CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
    switch (other->shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)other->shape;
        if (TreeTreeQuery(reinterpret_cast<QueryTreeNode*>(mesh->field_0x04),
            reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &mesh->field_0x08, &hull->worldTransform, 1,
            reinterpret_cast<Vec3*>(hull->vertices), 0)) {
            CollisionRotateColsInPlace(&reinterpret_cast<CollisionBoxResult*>(g_CollisionBoxResult)->field_0x08,
                reinterpret_cast<CollisionBoxResult*>(g_CollisionBoxResult)->field_0x08, &hull->worldTransform);
            return 1;
        }
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)other->shape;
        int hit = 0;
        if (TestMeshBounds(other)) {
            for (int i = 0; i < model->elementCount; i++) {
                CollisionHullBody* e = &model->elements[i];
                if (model->elementEnabled[i]) {
                    if (TreeTreeQuery(reinterpret_cast<QueryTreeNode*>(mesh->field_0x04),
                        reinterpret_cast<QueryTreeNode*>(e->triangleTree), &mesh->field_0x08, &e->worldTransform,
                        1, reinterpret_cast<Vec3*>(e->vertices), 0)) {
                        hit = 1;
                        CollisionRotateColsInPlace(&reinterpret_cast<CollisionBoxResult*>(g_CollisionBoxResult)->field_0x08,
                            reinterpret_cast<CollisionBoxResult*>(g_CollisionBoxResult)->field_0x08, &e->worldTransform);
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
// Near miss (441/458): one store (mov [esp+0x30], eax) of the second inverse copy is
// scheduled later.  No gain from an inline CollisionRelativeTransform (inlines all three),
// three calls, swapped inverse variants, two inverse locals, block scopes, declaration
// orders, t2 first or reordered rows in the second copy.  Also no gain (18 forms) from a
// separate helper for the second copy with reversed or reordered swaps, split swap temps,
// a Matrix4& alias, scalar or direct translation stores, or a by-value return.
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

// ==============================================================================================
// Section: small helpers
// ==============================================================================================
// Vec3 with an inline constructor (retail builds the result with the arguments evaluated
// z, y, x and stored x, y, z, so the values stay on the x87 stack until the stores).
struct CollisionInlineVec3 {
    float x, y, z;
    CollisionInlineVec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

// 0x00435610 (cdecl, returns the vector through the hidden first argument): componentwise
// minimum of a and b.  Called from the bounds code at 0x00435499 (inside SetTransform) with
// the result fed to the Vec3 operator+.
// owner: bracket only.
CollisionInlineVec3 CollisionVec3Min(const CollisionVec3& a, const CollisionVec3& b)
{
    return CollisionInlineVec3(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);
}

// 0x00435680: componentwise maximum (retail's test ah,0x41 form is `a > b`).
// owner: bracket only.
CollisionInlineVec3 CollisionVec3Max(const CollisionVec3& a, const CollisionVec3& b)
{
    return CollisionInlineVec3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
}

// 0x00435ec0 (cdecl, owner: CollisionObject.cpp bracket): vector length; returns 1.0f without a square root when |v|^2 == 1.
float CollisionLength(const CollisionVec3* v)
{
    // Written out rather than CollisionDot(*v, *v): the inline form schedules z*z first.
    float s = (v->x * v->x + v->y * v->y) + v->z * v->z;
    return s == 1.0f ? 1.0f : FastSqrt(s);
}

// 0x00439e00 (cdecl): installs the scratch result record the box tests write into
// (g_CollisionBoxResult, 0x00579058, read by HullVsModel and friends).
// owner: bracket only (between the 0x00439xxx shape tests and the __FILE__ xrefs of CollisionObject.cpp).
void SetCollisionBoxResult(CollisionBoxResult* result)
{
    g_CollisionBoxResult = reinterpret_cast<QueryHit*>(result);
}

// 0x004394f0 (cdecl; ObjectPicker.cpp 0x004b0a46): whether a sphere (world centre, r, r*r)
// touches an enabled object's triangles: a hull directly, a model after its bounds test,
// element by element.  Shape 2 (and any other) reports no contact; retail keeps an empty
// case 2 in the dispatch (the second `dec ecx`).
int SphereTouchesObject(const CollisionVec3* center, float radius, float radiusSq, CollisionObject* object)
{
    if (!(object->isEnabled))
        return 0;
    switch (object->shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)object->shape;
        return SphereTreeQueryWithVertices(reinterpret_cast<const Vec3*>(center), radius, radiusSq,
            reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &hull->worldTransform, 1,
            reinterpret_cast<Vec3*>(hull->vertices));
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)object->shape;
        if (BoxSphereOverlap(reinterpret_cast<const Vec3*>(&model->center),
            reinterpret_cast<const Vec3*>(&model->halfExtents), reinterpret_cast<const Vec3&>(*center), radius,
            radiusSq, &model->field_0x88)) {
            for (int i = 0; i < model->elementCount; i++) {
                CollisionHullBody* hull = &model->elements[i];
                if (SphereTreeQueryWithVertices(reinterpret_cast<const Vec3*>(center), radius, radiusSq,
                    reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &hull->worldTransform, 1,
                    reinterpret_cast<Vec3*>(hull->vertices)))
                    return 1;
            }
        }
        return 0;
    }
    case 2:
        return 0;
    }
    return 0;
}


// ==============================================================================================
// Section: transforms and the segment query (0x00434eb0, 0x00435830, 0x00439820)
// ==============================================================================================
// In-place rigid inverse as 0x00434eb0 and 0x00439820 expand it on a stack copy (transpose,
// then the negated translation dot products in the order of CollisionInvertRigidRelFrame).
// 0x0042e2b0 is the out-of-line copy that 0x00439820's model case calls (cdecl, one argument).
static inline void CollisionInvertRigidInPlace(Matrix4* m)
{
    float t;
    t = m->_12; m->_12 = m->_21; m->_21 = t;
    t = m->_13; m->_13 = m->_31; m->_31 = t;
    t = m->_23; m->_23 = m->_32; m->_32 = t;
    CollisionVec3 p;
    p.x = -((m->_43 * m->_31 + m->_42 * m->_21) + m->_41 * m->_11);
    p.y = -((m->_42 * m->_22 + m->_41 * m->_12) + m->_43 * m->_32);
    p.z = -((m->_43 * m->_33 + m->_42 * m->_23) + m->_41 * m->_13);
    m->_41 = p.x;
    m->_42 = p.y;
    m->_43 = p.z;
}

// BoundingBoxTreeQuery.cpp entry points the segment query uses (bvh/BoundingBoxTreeQuery.h
// declares them on its own node types; the tree pointer is opaque here).
int SegmentBoxOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 p0, Vec3 p1,
                      const Matrix4* m);                                             // 0x004253b0
// Call views (see math/Math3D.h): the Vec3 constructor 0x00404e60 inside the expanded
// operators of 0x00439820.
struct CollisionVec3Call : Vec3 {
    CollisionVec3Call(float x_, float y_, float z_);
};

// Complete 0x00404e60 constructor behavior for this provisional call view.
CollisionVec3Call::CollisionVec3Call(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

static inline Vec3 SubtractCtorCall(const Vec3& a, const Vec3& b)
{
    return CollisionVec3Call(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline Vec3 AddCtorCall(const Vec3& a, const Vec3& b)
{
    return CollisionVec3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline Vec3 ScaleCtorCall(const Vec3& v, float s)
{
    return CollisionVec3Call(v.x * s, v.y * s, v.z * s);
}

// Vector by value times the 3x3 block / the full transform (row-vector convention), as the
// bounds code expands them; 0x0042a510 is the out-of-line copy of the point form.
static inline void CollisionRotateInline(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// 0x00434eb0: refit the model bounds.  Every enabled element's box (tree root: center and
// half extents) is rotated into the model frame; the half extents go axis by axis so the
// projected extent is the sum of the absolute components.  The first element starts the
// running min/max, the others extend it (CollisionVec3Min/Max out of line).  Retail expands the
// Min argument's operator- but calls its Vec3 constructor (0x00404e60), and calls operator+
// (0x00421cb0) for the Max argument: the inline budget runs out there.  The named `lo`, the
// `continue`, the toModel pointer and the guarded do-while spend retail's budget
// (VC6_INLINE_BUDGET).  Retail clears i before the loop guard, which a for loop gives, but a
// for loop compiles its entry test as `cmp [ebx], 0` where retail loads the count.
void CollisionObject::UpdateModelBounds()
{
    CollisionModelBody* model = (CollisionModelBody*)shape;
    model->field_0x30 = model->center;
    model->field_0x3c = model->halfExtents;
    Matrix4 inv = model->field_0x88;
    CollisionInvertRigidInPlace(&inv);
    const Matrix4* toModel = &inv;
    Vec3 minBound;
    Vec3 maxBound;
    if (model->elementCount > 0) {
        int i = 0;
        do {
            if (!model->elementEnabled[i])
                continue;
            CollisionHullBody* e = &model->elements[i];
            Vec3 ax = Vec3(e->triangleTree->halfExtents.x, 0.0f, 0.0f);
            Vec3 ay = Vec3(0.0f, e->triangleTree->halfExtents.y, 0.0f);
            Vec3 az = Vec3(0.0f, 0.0f, e->triangleTree->halfExtents.z);
            CollisionRotateInline(&ax, ax, &e->worldTransform);
            CollisionRotateInline(&ax, ax, toModel);
            CollisionRotateInline(&ay, ay, &e->worldTransform);
            CollisionRotateInline(&ay, ay, toModel);
            CollisionRotateInline(&az, az, &e->worldTransform);
            CollisionRotateInline(&az, az, toModel);
            Vec3 extent;
            extent.x = CollisionAbs(ax.x) + CollisionAbs(ay.x) + CollisionAbs(az.x);
            extent.y = CollisionAbs(ax.y) + CollisionAbs(ay.y) + CollisionAbs(az.y);
            extent.z = CollisionAbs(ax.z) + CollisionAbs(ay.z) + CollisionAbs(az.z);
            Vec3 center;
            DebugTransformPoint(&center, *(Vec3*)&e->triangleTree->center, &e->worldTransform);
            DebugTransformPoint(&center, center, toModel);
            if (i != 0) {
                Vec3 lo = center - extent;
                *(CollisionInlineVec3*)&minBound = CollisionVec3Min(*(CollisionVec3*)&minBound, *(CollisionVec3*)&lo);
                *(CollisionInlineVec3*)&maxBound = CollisionVec3Max(*(CollisionVec3*)&maxBound, *(CollisionVec3*)&(center + extent));
            } else {
                minBound = center - extent;
                maxBound = center + extent;
            }
        } while (++i < model->elementCount);
    }
    *(Vec3*)&model->center = (minBound + maxBound) * 0.5f;
    *(Vec3*)&model->halfExtents = (maxBound - minBound) * 0.5f;
}

// The 4x4 product of CollisionMatrixMultiply (0x00436500) as SetTransform's hull case expands it
// (the model case calls the function).
// Term order dead end (about 40 forms): retail's rows 1-3 share one per-element order and
// row 0 has its own, consistent with leaf ages fixed in row 0.  Element pointers (float views
// of out/a/b), references, swapped operands or terms, direct expansion in SetTransform, a
// hull-body helper, reassigned row pointers and the five bracketings with or without
// parenthesised products leave every row different; m[i][j] indexing or per-row pointers
// are dearer and make the point transform a call (per-row pointers reproduce row 1 only).
static inline void CollisionMatrixMultiplyInline(Matrix4* out, const Matrix4* a, const Matrix4* b)
{
    out->_11 = b->_11 * a->_11 + b->_12 * a->_21 + b->_13 * a->_31 + b->_14 * a->_41;
    out->_12 = b->_11 * a->_12 + b->_12 * a->_22 + b->_13 * a->_32 + b->_14 * a->_42;
    out->_13 = b->_11 * a->_13 + b->_12 * a->_23 + b->_13 * a->_33 + b->_14 * a->_43;
    out->_14 = b->_11 * a->_14 + b->_12 * a->_24 + b->_13 * a->_34 + b->_14 * a->_44;
    out->_21 = b->_21 * a->_11 + b->_22 * a->_21 + b->_23 * a->_31 + b->_24 * a->_41;
    out->_22 = b->_21 * a->_12 + b->_22 * a->_22 + b->_23 * a->_32 + b->_24 * a->_42;
    out->_23 = b->_21 * a->_13 + b->_22 * a->_23 + b->_23 * a->_33 + b->_24 * a->_43;
    out->_24 = b->_21 * a->_14 + b->_22 * a->_24 + b->_23 * a->_34 + b->_24 * a->_44;
    out->_31 = b->_31 * a->_11 + b->_32 * a->_21 + b->_33 * a->_31 + b->_34 * a->_41;
    out->_32 = b->_31 * a->_12 + b->_32 * a->_22 + b->_33 * a->_32 + b->_34 * a->_42;
    out->_33 = b->_31 * a->_13 + b->_32 * a->_23 + b->_33 * a->_33 + b->_34 * a->_43;
    out->_34 = b->_31 * a->_14 + b->_32 * a->_24 + b->_33 * a->_34 + b->_34 * a->_44;
    out->_41 = b->_41 * a->_11 + b->_42 * a->_21 + b->_43 * a->_31 + b->_44 * a->_41;
    out->_42 = b->_41 * a->_12 + b->_42 * a->_22 + b->_43 * a->_32 + b->_44 * a->_42;
    out->_43 = b->_41 * a->_13 + b->_42 * a->_23 + b->_43 * a->_33 + b->_44 * a->_43;
    out->_44 = b->_41 * a->_14 + b->_42 * a->_24 + b->_43 * a->_34 + b->_44 * a->_44;
}

// v * M + translation as the hull case of 0x00435830 expands it (the sum of the three
// products first, then the translation).
static inline void CollisionTransformPointInline(Vec3* out, const Vec3& v, const Matrix4* m)
{
    out->x = (v.x * m->_11 + v.y * m->_21 + v.z * m->_31) + m->_41;
    out->y = (v.x * m->_12 + v.y * m->_22 + v.z * m->_32) + m->_42;
    out->z = (v.x * m->_13 + v.y * m->_23 + v.z * m->_33) + m->_43;
}

// Length of a vector as 0x00435830 expands it: the dot product is called out of line
// (0x0040ae30) at both expanded sites; 0x00435ec0 (CollisionLength) is the mesh case's call.
static inline float CollisionLengthInline(const Vec3& v)
{
    float s = Vec3DotCall(&v, &v);
    return s == 1.0f ? 1.0f : FastSqrt(s);
}

// 0x00435830 (thiscall, ret 4): places the shape with the new body matrix m.
//   hull:   the previous body matrix is kept, worldTransform = localTransform * m (inline
//           product), motionTransform = prev * inverse(m); the bounding sphere is the tree
//           root's box moved by the motion and transformed by the body matrix.
//   model:  field_0x48 keeps the old matrix, field_0xc8 = old * inverse(m); every element
//           gets its scene node's world matrix and the same three matrices; then the
//           bounds are refitted (UpdateModelBounds) and the sphere comes from them.
//   mesh:   the matrix is stored; the sphere is the tree root box.
//   sphere: the sphere is the shape's own centre and radius through its transform.
//   capsule: nothing.
// Only the hull case expands the point transform; the other cases call 0x0042a510 (inline budget).
// The hull case names the previous and current body matrices: retail keeps &prev in edx across
// the product for the CollisionRelativeTransform argument and spills &body (163 -> 902).
// Model loop (about 25 forms): retail holds model in ebx at the loop head (copied in the latch,
// so the empty-loop exit needs no copy).  Reading model->elements into a block-local first
// gives the ebx base and the stack-kept 0x198 offset but enters the loop with a jmp and copies
// after it (863, d192; `e = model->elements; e += i` 882, d180); e++, char* or byte-offset
// strides, while/do forms and named prev/body element pointers score lower.
void CollisionObject::SetTransform(const Matrix4* m)
{
    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)shape;
        CollisionVec3* boundCenter = (CollisionVec3*)&this->boundCenter;   // +0x40, the bounding-sphere centre
        Matrix4* prev = &hull->prevBodyTransform;
        Matrix4* body = &hull->bodyTransform;
        *prev = *body;
        *body = *m;
        CollisionMatrixMultiplyInline(&hull->worldTransform, m, &hull->localTransform);
        CollisionRelativeTransform(&hull->motionTransform, m, prev);
        *boundCenter = hull->triangleTree->center;
        Vec3 half = *(Vec3*)&hull->triangleTree->halfExtents;
        MoveBox((Vec3*)boundCenter, &half, &hull->motionTransform);
        boundRadius = CollisionLengthInline(half);
        Vec3 c = *(Vec3*)boundCenter;
        CollisionTransformPointInline((Vec3*)boundCenter, c, body);
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)shape;
        model->field_0x48 = model->field_0x88;
        model->field_0x88 = *m;
        CollisionRelativeTransform(&model->field_0xc8, m, &model->field_0x48);
        for (int i = 0; i < model->elementCount; i++) {
            CollisionHullBody* e = &model->elements[i];
            e->prevBodyTransform = e->bodyTransform;
            reinterpret_cast<SoultreeObject*>(e->sceneNode)->GetMatrixIn(0, &e->bodyTransform);
            CollisionMatrixMultiply(&e->worldTransform, &e->bodyTransform, &e->localTransform);
            CollisionRelativeTransform(&e->motionTransform, &e->bodyTransform, &e->prevBodyTransform);
        }
        UpdateModelBounds();
        CollisionVec3* boundCenter = (CollisionVec3*)&this->boundCenter;
        *boundCenter = model->center;
        Vec3 half = *(Vec3*)&model->halfExtents;
        MoveBox((Vec3*)boundCenter, &half, &model->field_0xc8);
        boundRadius = CollisionLengthInline(half);
        Vec3TransformPoint(reinterpret_cast<Vec3*>(boundCenter), reinterpret_cast<const Vec3&>(*boundCenter),
            &model->field_0x88);
        break;
    }
    case 2: {
        CollisionMeshBody* mesh = (CollisionMeshBody*)shape;
        mesh->field_0x08 = *m;
        CollisionVec3* boundCenter = (CollisionVec3*)&this->boundCenter;
        *boundCenter = mesh->field_0x04->center;
        CollisionVec3 half = mesh->field_0x04->halfExtents;
        boundRadius = CollisionLength(&half);
        Vec3TransformPoint(reinterpret_cast<Vec3*>(boundCenter), reinterpret_cast<const Vec3&>(*boundCenter),
            &mesh->field_0x08);
        break;
    }
    case 3:
        break;
    case 4: {
        CollisionSphereShape* sphere = (CollisionSphereShape*)shape;
        CollisionVec3* boundCenter = (CollisionVec3*)&this->boundCenter;
        *boundCenter = sphere->center;
        boundRadius = sphere->radius;
        Vec3TransformPoint(reinterpret_cast<Vec3*>(boundCenter), reinterpret_cast<const Vec3&>(*boundCenter),
            (const Matrix4*)&sphere->transform);
        break;
    }
    }
}

// 0x00439820 (cdecl; the segment version of SphereTouchesObject 0x004394f0): the segment
// ends[0]..ends[1] against the object.  Rejects when the closest point of the segment to
// the bounding-sphere centre lies outside the sphere; otherwise runs the segment tree query
// (0x00429e60) on the hull, or on every enabled element of the model after the model bounds
// test (0x004253b0), with the hit record installed as the box result.  On a hit *outPoint
// is the point at the hit fraction and *outNormal the record's normal rotated into world.
// By-value views of the out-of-line Vec3 operators the pre-test calls (each result is a
// compiler temporary): subtract 0x00421d00, add 0x00421cb0, dot 0x0040ae30, length 0x00435ec0.
Vec3 SegmentSubtract(const Vec3& a, const Vec3& b);
Vec3 SegmentAdd(const Vec3& a, const Vec3& b);
// Distance from center to the closest point of the segment p[0]..p[1].  Retail's frame
// (0xa8, each operator result in its own slot) comes from this helper being inlined.
// Taking the const reference address retains each returned temporary through the
// full expression while joining the existing pointer-view dot/length providers.
static inline float SegmentPointDistance(const Vec3* p, const Vec3& center)
{
    Vec3 d = SegmentSubtract(p[1], p[0]);
    float t = Vec3DotCall(&d, &static_cast<const Vec3&>(SegmentSubtract(center, p[0]))) / Vec3DotCall(&d, &d);
    if (t >= 1.0f)
        t = 1.0f;
    else if (t <= 0.0f)
        t = 0.0f;
    Vec3 scaled = CollisionVec3Call(d.x * t, d.y * t, d.z * t);
    return CollisionLength(reinterpret_cast<const CollisionVec3*>(&static_cast<const Vec3&>(SegmentSubtract(center, SegmentAdd(scaled, p[0])))));
}
int SegmentTouchesObject(const CollisionVec3* ends, CollisionObject* object, CollisionVec3* outPoint,
                         CollisionVec3* outNormal)
{
    if (object->isEnabled) {
        const Vec3* p = (const Vec3*)ends;
        if (SegmentPointDistance(p, *(const Vec3*)&object->boundCenter) < object->boundRadius) {
            switch (object->shapeType) {
            case 0: {
                CollisionHullBody* hull = (CollisionHullBody*)object->shape;
                CollisionBoxResult hit;
                hit.point = 0;
                *(Vec3*)&hit.field_0x08 = Vec3(0.0f, 0.0f, 0.0f);
                hit.fraction = 1.0f;
                SetCollisionBoxResult(&hit);
                Matrix4 inv = hull->worldTransform;
                CollisionInvertRigidInPlace(&inv);
                int result = SegmentTreeQueryWithVertices(p, reinterpret_cast<QueryTreeNode*>(hull->triangleTree), &inv,
                                                          (Vec3*)hull->vertices);
                if (result) {
                    Vec3 d = SubtractCtorCall(p[1], p[0]);
                    Vec3 scaled = ScaleCtorCall(d, hit.fraction);
                    *(Vec3*)outPoint = AddCtorCall(scaled, p[0]);
                    CollisionRotateColsTo(outNormal, hit.field_0x08, &hull->worldTransform);
                }
                return result;
            }
            case 1: {
                CollisionModelBody* model = (CollisionModelBody*)object->shape;
                CollisionBoxResult hit;
                hit.point = 0;
                *(Vec3*)&hit.field_0x08 = Vec3(0.0f, 0.0f, 0.0f);
                hit.fraction = 1.0f;
                SetCollisionBoxResult(&hit);
                Matrix4 inv = model->field_0x88;
                InvertRigidTransform(&inv);
                if (SegmentBoxOverlap((Vec3*)&model->center, (Vec3*)&model->halfExtents, p[0], p[1], &inv)) {
                    for (int i = 0; i < model->elementCount; i++) {
                        CollisionHullBody* e = &model->elements[i];
                        inv = e->worldTransform;
                        InvertRigidTransform(&inv);
                        if (SegmentTreeQueryWithVertices(p, reinterpret_cast<QueryTreeNode*>(e->triangleTree),
                                &inv, (Vec3*)e->vertices)) {
                            Vec3 d = SubtractCtorCall(p[1], p[0]);
                            Vec3 scaled = ScaleCtorCall(d, hit.fraction);
                            *(Vec3*)outPoint = SegmentAdd(scaled, p[0]);
                            Vec3TransformNormal(reinterpret_cast<Vec3*>(outNormal),
                                reinterpret_cast<const Vec3&>(hit.field_0x08), &e->worldTransform);
                            return 1;
                        }
                    }
                }
                return 0;
            }
            case 2:   // retail tests it (the lone dec eax) and falls into the return 0
                return 0;
            }
        }
    }
    return 0;
}

// 0x004320f0: three collision flags followed by a nonvirtual call to
// GameObject::SetRenderContext on the secondary base at +0xc. The context
// is passed through unchanged; its pointed-to type remains opaque.
void CollisionObject::Configure(void* context, int broadphase, int enabled, int collidableValue)
{
    useBroadphase = broadphase;
    collisionEnabled = enabled;
    collidable = collidableValue;
    GameObject::SetRenderContext(context);
}
