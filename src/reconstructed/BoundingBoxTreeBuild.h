// BoundingBoxTreeBuild.h -- types shared by the bounding-box tree builder
// (D:\aardvark\VC\krusty2\BoundingBoxTreeBuild.cpp, src/reconstructed/BoundingBoxTreeBuild.cpp).
// Header name and all type names are ours (tier 3); the layouts come from the decoded accesses
// noted on each member.  Promoted from src/krusty2/bvh/BoundingBoxTreeBuild.h; the scene-graph
// and model types are local stand-ins seen from this file only.
//
// The trees are the ones CollisionObject's hull/model/mesh payloads hang off (+0x188 triangle
// tree, +0x18c point tree; see collision/CollisionObject.h and the collision samples'
// CollisionTreeNode).  Every node starts with a float: >= 0 marks an interior node and holds
// the volume of its box; -1.0f marks a leaf.
#ifndef BOUNDING_BOX_TREE_BUILD_H
#define BOUNDING_BOX_TREE_BUILD_H

#include "DebugAlloc.h"

// Three consecutive floats; copies are three dword moves (tier 1).  A local type, because
// this TU has none of the per-TU $E vector-constant initialisers that common/Math3D.h adds;
// the inline operators mirror d3dvec.inl like common/Math3D.h's Vec3 (the centre/half-size
// code in the bounds helpers goes through by-value temporaries).
struct TreeVec3 {
    float x, y, z;

    TreeVec3() {}
    TreeVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};

inline TreeVec3 operator+(const TreeVec3& a, const TreeVec3& b) { return TreeVec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline TreeVec3 operator-(const TreeVec3& a, const TreeVec3& b) { return TreeVec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline TreeVec3 operator*(const TreeVec3& v, float s) { return TreeVec3(s * v.x, s * v.y, s * v.z); }

// 4x4 row-major matrix, row-vector convention (v' = v * M, translation in row 3); tier 2 from
// the point transform in ComputePointBounds (0x0042aff0): x' = x*_11 + y*_21 + z*_31 + _41.
struct TreeMatrix4 {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

// out = v * m without the translation row (directions: the face normals in 0x0042cc60).
inline void RotateVector(TreeVec3* out, TreeVec3 v, const TreeMatrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33;
}

// out = v * m with the translation row added: the inline form of Vec3TransformPoint
// (0x0042a510, same signature).
inline void TransformPoint(TreeVec3* out, TreeVec3 v, const TreeMatrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// In-place inverse of a rigid transform: transpose the rotation block, then rotate the
// translation back and negate it.  BuildModelBoxTree (0x0042dc90) inlines exactly this body
// (it must go through the pointer: the same statements written on the local matrix directly
// make VC6 order the x87 operands differently); the out-of-line copy at 0x0042e2b0 has the same body (samples/physics/bvh/RigidTransform.cpp).
inline void InvertRigid(TreeMatrix4* m)
{
    float swap;
    swap = m->_12; m->_12 = m->_21; m->_21 = swap;
    swap = m->_13; m->_13 = m->_31; m->_31 = swap;
    swap = m->_23; m->_23 = m->_32; m->_32 = swap;
    TreeVec3 t;
    t.x = -(m->_41 * m->_11 + m->_42 * m->_21 + m->_43 * m->_31);
    t.y = -(m->_41 * m->_12 + m->_42 * m->_22 + m->_43 * m->_32);
    t.z = -(m->_41 * m->_13 + m->_42 * m->_23 + m->_43 * m->_33);
    m->_41 = t.x;
    m->_42 = t.y;
    m->_43 = t.z;
}

// Interior node, 0x24 bytes (DebugMalloc(0x24) at lines 0x24f/0x255/0x4cc/0x4d2/0x52c/0x582).
struct BoxTreeNode {
    float volume;               // +0x00 box volume (output of the bounds helpers); >= 0 = interior
    TreeVec3 center;            // +0x04 box centre
    TreeVec3 halfExtents;       // +0x10 box half size
    BoxTreeNode* child[2];      // +0x1c, +0x20
};

// Triangle leaf, 0x1c bytes (DebugMalloc(0x1c) at lines 0x24d/0x253/0x548).
struct BoxTreeTriangleLeaf {
    float marker;               // +0x00 -1.0f
    unsigned short vertex[3];   // +0x04 vertex indices (narrowed from the build records' ints)
    TreeVec3 normal;            // +0x0c face normal (copied from the build record's +0x0c)
    float planeOffset;          // +0x18 copied from the build record's planeOffset (+0x28)
};

// Box leaf of a model tree, 0x1c bytes (DebugMalloc(0x1c) at lines 0x4ca/0x4d0).
struct BoxTreeBoxLeaf {
    float marker;               // +0x00 -1.0f
    TreeVec3 corner[2];         // +0x04, +0x10 the two corners of the element box
};

// Point leaf, 0x10 bytes (DebugMalloc(0x10) at line 0x59e).
struct BoxTreePointLeaf {
    float marker;               // +0x00 -1.0f
    TreeVec3 point;             // +0x04
};

// Triangle record the builder sorts, 0x2c bytes (rep movsd ecx=0xb).
struct TreeBuildTriangle {
    int vertex[3];              // +0x00 indices into the vertex array
    TreeVec3 normal;            // +0x0c copied into the leaf
    TreeVec3 centroid;          // +0x18 the split tests compare it with the split plane
    int field_0x24;
    float planeOffset;          // +0x28 TriangleNormal's fifth (plane offset) output in
                                // BuildTriangleMeshTree (0x0042cc60); copied into the leaf
};

// The resource stream (TextureMap.h); ReadTriangleMesh/ReadTriangleNode/ReadPointNode read
// through 0x00461640 (buffer, size, count), the fread order.
class UnknownTextureStream;

// 0x004a11e0 (cdecl, outside this file): unit normal of the triangle (a, b, c); the fifth
// argument receives the plane offset (same declaration as broadphase/Terrain.h's
// TerrainTriangleNormal, local stand-in type).
void TriangleNormal(const TreeVec3* a, const TreeVec3* b, const TreeVec3* c, TreeVec3* normal,
                    float* planeOffset);

// Scene-graph node (soultree.cpp's SoultreeObject; see src/krusty2/core/SoultreeObject.h,
// which promoted code may not include).  Local stand-in with only the methods called here.
// Method names follow that header (tier 3).
class TreeSceneNode {
public:
    void SetMatrixIn(TreeSceneNode* frame, TreeMatrix4* m);     // 0x004fb8c0
    void SetAxesIn(TreeSceneNode* frame, const TreeVec3* axisZ, const TreeVec3* axisY,
                   int orthogonalize, int keepZ);                // 0x004fc050
    void SetPositionIn(TreeSceneNode* frame, const TreeVec3* p);  // 0x004fc740
    void GetMatrixIn(TreeSceneNode* frame, TreeMatrix4* out);   // 0x004fca80
    void MarkSubtreeDirty();                                    // 0x004fdab0
};

// Vertex of a render mesh, 0x20 bytes (D3DVERTEX-sized: part ranges are byte offsets divided by
// sizeof); only the position is read here.
struct TreeModelVertex {
    TreeVec3 position;          // +0x00 copied into the point list by 0x0042d390
    float field_0x0c[5];
};

// Vertex range of a mesh that belongs to one frame, 0x14 bytes (stride in 0x0042d390).
struct TreeModelPart {
    TreeSceneNode* frame;       // +0x00 compared with the frame argument of 0x0042d390/0x0042c260
    int vertexCount;            // +0x04 length of the range
    TreeModelVertex* firstVertex;  // +0x08 start of the range inside the mesh's vertices
    int field_0x0c;
    int field_0x10;
};

// Triangle of a render mesh: three 16-bit vertex indices, 6 bytes (stride in 0x0042c260).
struct TreeModelTriangle {
    unsigned short vertex[3];   // +0x00 indices into the mesh's vertices
};

// Mesh of a render model LOD, 0x38 bytes (stride in 0x0042d390 and 0x0042c260).
struct TreeModelMesh {
    int partCount;              // +0x00 entries in parts
    TreeModelPart* parts;       // +0x04
    int vertexCount;            // +0x08 summed for the point list size; loop bound over vertices
    int triangleCount;          // +0x0c summed by 0x0042c260; loop bound over triangles
    TreeModelVertex* vertices;  // +0x10 base of the parts' firstVertex ranges
    TreeModelVertex* field_0x14;  // +0x14 0x0042c260 reads positions from here instead of
                                  // +0x10 when it gathers a single frame's triangles
    int field_0x18;
    TreeModelTriangle* triangles;  // +0x1c read by 0x0042c260
    char pad_0x20[0x38 - 0x20];
};

// LOD entry of a render model, 8 bytes: the meshes of that level.
struct TreeModelLod {
    int meshCount;              // +0x00
    TreeModelMesh* meshes;      // +0x04
};

// Render model (D3DIMSoulTree.CPP; derives from the scene node, see 0x00440d40's this).  Stand-in.
class TreeModelSource : public TreeSceneNode {
public:
    void SelectLod(int lod);    // 0x00440d40: -1 keeps currentLod, then walks the LOD's meshes
    char pad_0x000[0x1a4];
    int field_0x1a4;            // +0x1a4 saved, cleared around SelectLod(-1) and restored by
                                // 0x0042c260
    char pad_0x1a8[0x27c - 0x1a8];
    int currentLod;             // +0x27c index into lods (0x00440d40 stores its argument here)
    char pad_0x280[0x28c - 0x280];
    TreeModelLod* lods;         // +0x28c indexed by currentLod
};

// Vector constants defined midway through BoundingBoxTreeBuild.cpp (0x00579680 zero,
// 0x00579690 x, 0x005796a0 y, 0x00579670 z; $E initializers 0x0042d250..).  Names tier 3.
extern const TreeVec3 kVec3Zero;
extern const TreeVec3 kVec3XAxis;
extern const TreeVec3 kVec3YAxis;
extern const TreeVec3 kVec3ZAxis;

// 0x0042c260: the triangles of `model`'s current LOD (only those whose three corners lie in
// `frame`'s vertex ranges when `frame` is non-null) as a deduplicated vertex list and three
// indices per triangle.
void GatherModelTriangles(TreeModelSource* model, TreeSceneNode* frame, int* triCount,
                          int* vertCount, int** indices, TreeVec3** verts);

// Partition and bounds helpers (0x0042ad30..0x0042b5eb), the first functions of this file.
int SplitTriangles(const TreeBuildTriangle* tris, TreeBuildTriangle* left,
                   TreeBuildTriangle* right, int count, int* leftCount, int* rightCount,
                   float split, int axis);
int SplitBoxes(const TreeVec3* boxes, const TreeVec3* centers, TreeVec3* leftBoxes,
               TreeVec3* rightBoxes, TreeVec3* leftCenters, TreeVec3* rightCenters, int count,
               int* leftCount, int* rightCount, float split, int axis);
void ComputePointBounds(const TreeVec3* points, int count, TreeVec3* center,
                        TreeVec3* halfExtents, float* volume, const TreeMatrix4* m);
void ComputeTriangleBounds(const TreeBuildTriangle* tris, int count, const TreeVec3* verts,
                           TreeVec3* center, TreeVec3* halfExtents, float* volume,
                           const TreeMatrix4* m);
float TriangleBoundsVolume(const TreeBuildTriangle* tris, const TreeVec3* verts, int count);
float PointBoundsVolume(const TreeVec3* points, int count);

#endif
