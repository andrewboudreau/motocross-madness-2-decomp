// CollisionShapeSetup.cpp -- CollisionObject shape setters that build a payload from a scene
// node, mesh arrays or a .col stream (wave 5).  Names tier 3; offsets/sizes tier 1 (target bytes).
// owner: CollisionObject.cpp (__FILE__ strings at every allocation, lines 0xd8..0x14f / 0x9e8..0x9f0).
#include "collision/CollisionObject.h"
#include "CollisionShapeTests.h"

// 0x004a1410 (cdecl): fills *out with the identity-like frame matrix and returns it (the
// same function Terrain.h calls GetIdentityMatrix).  Local stand-in.
Matrix4* Fn_004a1410(Matrix4* out);

// Triangle-tree builders (BoundingBoxTreeBuild.cpp).  Local stand-ins with void* in place of
// the bvh types.
void BuildModelTriangleTree(void* root, Matrix4* m, void* model, CollisionVec3** verts,
                            void* frame, int unused);                          // 0x0042c8c0
void BuildTriangleMeshTree(void* root, Matrix4* m, CollisionVec3* verts, const int* indices,
                           int triCount, int vertCount, int unused);           // 0x0042cc60
void BuildPointTree(void* root, Matrix4* m, void* model, void* frame);          // 0x0042d390 (tier 3 name)

// 0x00432720 (ret 0x14): hull shape for a scene node.  `buildPointTree` (arg 2, the only one
// that is tested) adds the point tree; arg 3 is the hull's swept flag; args 4 and 5 are unused
// except that arg 5 is forwarded to the triangle-tree builder.
void CollisionObject::Fn_00432720(void* node, int buildPointTree, int swept, int c, int d)
{
    FreeShape();
    CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x116);
    Matrix4 tmp;
    hull->relativeFrame = *Fn_004a1410(&tmp);
    hull->swept = swept;
    shapeType = 0;
    shape = hull;
    hull->sceneNode = node;
    hull->vertices = 0;
    hull->triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0x123);
    BuildModelTriangleTree(hull->triangleTree, &hull->localTransform, node,
                           (CollisionVec3**)&hull->vertices, 0, d);
    hull->pointTree = 0;
    if (buildPointTree) {
        hull->pointTree = operator new(0x24, __FILE__, 0x128);
        BuildPointTree(hull->pointTree, &hull->localTransform, node, 0);
    }
}

// 0x004328b0 (ret 0x10): static-mesh hull from vertex/index arrays.  The vertices are copied
// into a block of the object's own (DebugMalloc 12 bytes each, line 0x149), then the triangle
// tree is built from the copy.  The hull's swept flag is 0 and it has no point tree.
void CollisionObject::Fn_004328b0(const CollisionVec3* verts, const int* indices, int triCount,
                                  int vertCount)
{
    FreeShape();
    CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x13c);
    Matrix4 tmp;
    hull->relativeFrame = *Fn_004a1410(&tmp);
    hull->swept = 0;
    shapeType = 0;
    shape = hull;
    hull->sceneNode = 0;
    hull->vertices = DebugMalloc(vertCount * sizeof(CollisionVec3), __FILE__, 0x149);
    for (int i = 0; i < vertCount; i++)
        ((CollisionVec3*)hull->vertices)[i] = verts[i];
    hull->triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0x14f);
    BuildTriangleMeshTree(hull->triangleTree, &hull->localTransform, (CollisionVec3*)hull->vertices,
                          indices, triCount, vertCount, 0);
    hull->pointTree = 0;
}

// .col file object: 0x134 bytes, thiscall ctor 0x00460d10 (one global argument, the registry
// at 0x00572b44), destructor 0x00460d60, Open 0x00460f50 (ret 0xc) and the fread-order reader
// 0x00461640 (ptr, size, count).  The same object appears as SoultreeFile / ArchiveFile / TreeFile
// in the other areas; this is a local stand-in with just the members used here.
class CollisionFileStream {
public:
    explicit CollisionFileStream(int registry);                  // 0x00460d10
    ~CollisionFileStream();                                      // 0x00460d60
    void Open(const char* name, const char* mode, int a);        // 0x00460f50
    int Read(void* dst, int size, int count);                    // 0x00461640
    char field_0x00[0x134];
};
extern int g_00572b44;      // the registry passed to the file constructor

// 0x00439e10 (ret 8): reads the shape type from the stream and builds that payload
// (0: hull 0x198 bytes, 1: model 0x120 bytes), then lets the matching loader fill it.
void CollisionObject::Fn_00439e10(void* node, CollisionFileStream* stream)
{
    FreeShape();
    stream->Read(&shapeType, 4, 1);
    switch (shapeType) {
    case 0: {
        CollisionHullBody* hull = (CollisionHullBody*)operator new(0x198, __FILE__, 0x9e8);
        hull->sceneNode = node;
        Matrix4 tmp;
        hull->relativeFrame = *Fn_004a1410(&tmp);
        shape = hull;
        Fn_00439ed0(stream, hull);
        break;
    }
    case 1: {
        CollisionModelBody* model = (CollisionModelBody*)operator new(0x120, __FILE__, 0x9f0);
        model->field_0x10 = (int)node;
        shape = model;
        Fn_0043a050(node, stream, model);
        break;
    }
    }
}

// 0x00432800 (ret 8): shape from a .col file.  Opens the file in "rb" mode and lets
// Fn_00439e10 read it; the file object is heap-allocated at line 0x12f.
void CollisionObject::Fn_00432800(void* node, const char* path)
{
    CollisionFileStream* file = new(__FILE__, 0x12f) CollisionFileStream(g_00572b44);
    file->Open(path, "rb", 0);
    Fn_00439e10(node, file);
    delete file;
}

// Scene-graph node as the shape setup sees it (the same SoultreeObject the other areas model;
// local stand-in with just the calls used here, all thiscall).
class CollisionSourceNode {
public:
    int CountNodes();                                        // 0x004fda30
    void CollectNodes(int* count, CollisionSourceNode** list);   // 0x004fda60 (ret 8)
    int Accepts(CollisionSourceNode* child, int filter);     // 0x00445060 (ret 8), tier 3 name
};

// Element type of a model's hull array.  Retail allocates it with array new, whose empty
// constructor loop survives as a leftover count in the code, so the class needs a constructor.
struct CollisionHullElement : CollisionHullBody {
    CollisionHullElement() {}
};

// 0x004324b0 (ret 0x14): model shape = one hull per accepted node of the scene subtree.
// arg 2 builds the point trees too, arg 3 is the model's swept flag, arg 4 is the filter passed
// to the node predicate, arg 5 is only forwarded to the triangle-tree builder.
void CollisionObject::Fn_004324b0(void* nodeArg, int buildPointTrees, int swept, int filter, int d)
{
    CollisionSourceNode* node = (CollisionSourceNode*)nodeArg;
    FreeShape();
    CollisionModelBody* model = (CollisionModelBody*)operator new(0x120, __FILE__, 0xd8);
    shapeType = 1;
    shape = model;
    model->swept = swept;
    int total = node->CountNodes();
    CollisionSourceNode** list = (CollisionSourceNode**)operator new(total * 4, __FILE__, 0xe4);
    int one = 1;
    list[0] = node;
    node->CollectNodes(&one, list);
    model->elementCount = 0;
    for (int i = 0; i < total; i++) {
        if (node->Accepts(list[i], filter))
            model->elementCount++;
    }
    model->field_0x10 = (int)node;
    model->elements = new(__FILE__, 0xf3) CollisionHullElement[model->elementCount];
    model->elementEnabled = (int*)operator new(model->elementCount * 4, __FILE__, 0xf4);
    model->elementHighlight = (int*)operator new(model->elementCount * 4, __FILE__, 0xf5);
    int index = 0;
    for (int j = 0; j < total; j++) {
        if (node->Accepts(list[j], filter)) {
            model->elements[index].swept = swept;
            model->elementEnabled[index] = 1;
            model->elements[index].triangleTree = (CollisionBoxBounds*)operator new(0x24, __FILE__, 0xfd);
            model->elements[index].sceneNode = list[j];
            Matrix4 tmp;
            model->elements[index].relativeFrame = *Fn_004a1410(&tmp);
            BuildModelTriangleTree(model->elements[index].triangleTree, &model->elements[index].localTransform,
                                   node, (CollisionVec3**)&model->elements[index].vertices, list[j], d);
            model->elements[index].pointTree = 0;
            if (buildPointTrees) {
                model->elements[index].pointTree = operator new(0x24, __FILE__, 0x107);
                BuildPointTree(model->elements[index].pointTree, &model->elements[index].localTransform,
                               node, list[j]);
            }
            index++;
        }
    }
}
