#include <stddef.h>
#define CHECK(name, expression) typedef char name[(expression) ? 1 : -1]

// Compile the actual local source-node view and canonical query/scene types.
#ifdef QUERY_VIEW
#include "../../src/krusty2/bvh/BoundingBoxTreeQuery.h"
CHECK(query_size, sizeof(QueryTreeNode) == 0x24);
CHECK(query_marker, offsetof(QueryTreeNode, volume) == 0);
CHECK(query_children, offsetof(QueryTreeNode, child) == 0x1c);
#else
#include "../../src/krusty2/collision/CollisionObject.cpp"
CHECK(local_parent, offsetof(CollisionSourceNode, parent) == 0x13c);
CHECK(scene_parent, offsetof(SoultreeObject, parent) == 0x13c);
CHECK(model_size, sizeof(D3DIMSoultreeObject) == 0x2d8);
extern "C" int CollisionGameObjectOffset() {
    return (int)static_cast<GameObject*>(reinterpret_cast<CollisionObject*>(0x1000)) - 0x1000;
}
#endif
