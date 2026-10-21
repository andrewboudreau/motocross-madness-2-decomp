#include <stddef.h>
#ifdef PHYSICS_VIEW
#define SOULTREE_OBJECT_WITH_BASES
#include "../../src/krusty2/math/Math3D.h"
#else
#include "../../src/reconstructed/D3DIMSoulTree.h"
#endif
#define CHECK(name, value) typedef char name[(value) ? 1 : -1]
CHECK(node_size, sizeof(SoultreeObject) == 0x1a4);
CHECK(local_matrix, offsetof(SoultreeObject, localMatrix) == 0xb8);
CHECK(world_valid, offsetof(SoultreeObject, worldValid) == 0x138);
CHECK(parent_node, offsetof(SoultreeObject, parent) == 0x13c);
CHECK(first_child, offsetof(SoultreeObject, firstChild) == 0x140);
CHECK(next_sibling, offsetof(SoultreeObject, nextSibling) == 0x144);
CHECK(parameter_block, offsetof(SoultreeObject, parameterBlock) == 0x1a0);
extern "C" int HierarchyGameBase() {
    return (int)static_cast<GameObject*>(reinterpret_cast<SoultreeObject*>(0x1000)) - 0x1000;
}
extern "C" void DispatchHierarchyLoad(SoultreeObject* node, UnknownTextureStream* stream) {
    node->LoadHierarchy(stream);
}
extern "C" void DispatchHierarchyParameters(SoultreeObject* node) {
    node->LoadHierarchyFromParameters();
}
extern "C" void DispatchHierarchyReset(SoultreeObject* node) {
    node->ResetLocalMatrices();
}
extern "C" void DispatchHierarchyClone(SoultreeObject* node, SoultreeObject* source) {
    node->CloneFrom(source);
}
extern "C" void DispatchHierarchyCopy(SoultreeObject* node, SoultreeObject* source, SoultreeObject* target) {
    node->CopySubtree(source, target);
}
