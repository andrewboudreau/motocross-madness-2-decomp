#include <stddef.h>
#define CHECK(name, expression) typedef char name[(expression) ? 1 : -1]

// These two independently compiled views avoid importing another TU's vector
// constants. Both are checked against the same decoded storage boundaries.
#ifdef PHYSICS_VIEW
#include "../../src/krusty2/collision/CollisionShapeTests.h"
CHECK(vector_size, sizeof(CollisionVec3) == 12);
CHECK(matrix_size, sizeof(Matrix4) == 64);
CHECK(matrix_translation, offsetof(Matrix4, _41) == 48);
CHECK(box_size, sizeof(CollisionBoxBounds) == 0x24);
CHECK(box_center, offsetof(CollisionBoxBounds, center) == 4);
CHECK(box_extents, offsetof(CollisionBoxBounds, halfExtents) == 0x10);
CHECK(box_children, offsetof(CollisionBoxBounds, child) == 0x1c);
CHECK(hull_size, sizeof(CollisionHullBody) == 0x198);
CHECK(hull_local, offsetof(CollisionHullBody, localTransform) == 0x88);
CHECK(hull_relative, offsetof(CollisionHullBody, relativeFrame) == 0x148);
CHECK(hull_triangle, offsetof(CollisionHullBody, triangleTree) == 0x188);
CHECK(hull_points, offsetof(CollisionHullBody, pointTree) == 0x18c);
CHECK(hull_vertices, offsetof(CollisionHullBody, vertices) == 0x190);
#else
#include "../../src/reconstructed/BoundingBoxTreeBuild.h"
CHECK(vector_size, sizeof(TreeVec3) == 12);
CHECK(matrix_size, sizeof(TreeMatrix4) == 64);
CHECK(matrix_translation, offsetof(TreeMatrix4, _41) == 48);
CHECK(box_size, sizeof(BoxTreeNode) == 0x24);
CHECK(box_center, offsetof(BoxTreeNode, center) == 4);
CHECK(box_extents, offsetof(BoxTreeNode, halfExtents) == 0x10);
CHECK(box_children, offsetof(BoxTreeNode, child) == 0x1c);
CHECK(model_lod, offsetof(TreeModelSource, currentLod) == 0x27c);
CHECK(model_lods, offsetof(TreeModelSource, lods) == 0x28c);
CHECK(model_vertex_size, sizeof(TreeModelVertex) == 0x20);
CHECK(model_part_size, sizeof(TreeModelPart) == 0x14);
CHECK(model_mesh_size, sizeof(TreeModelMesh) == 0x38);
#endif
