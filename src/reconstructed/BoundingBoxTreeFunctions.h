#pragma once

// Selected collision-tree entry points from BoundingBoxTreeBuild.cpp.
// Types/layouts are documented in BoundingBoxTreeBuild.h. Forward declarations
// keep its TU-specific vector constants out of physics callers.
struct BoxTreeNode;
struct TreeMatrix4;
struct TreeVec3;
class TreeModelSource;
class TreeSceneNode;

void BuildModelTriangleTree(BoxTreeNode* root, TreeMatrix4* matrix, TreeModelSource* model,
                            TreeVec3** vertices, TreeSceneNode* frame, int unused); // 0x0042c8c0
void BuildTriangleMeshTree(BoxTreeNode* root, TreeMatrix4* matrix, TreeVec3* vertices,
                           const int* indices, int triangleCount, int vertexCount, int unused); // 0x0042cc60
void BuildModelPointTree(BoxTreeNode* root, TreeMatrix4* matrix, TreeModelSource* model,
                         TreeSceneNode* frame); // 0x0042d390
void BuildModelBoxTree(BoxTreeNode* root, const TreeMatrix4* matrix, TreeVec3* boxes,
                       int count); // 0x0042dc90
