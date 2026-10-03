// BoundingBoxTreeQuery.cpp -- small helpers from the run-time box-tree query code at
// 0x004245f0..0x0042ad21.
//
// Ownership is uncertain, so these stay samples.  The range lies in the BoundingBoxTreeBuild.cpp
// link bracket (after bmpfile.cpp's last __FILE__ xref 0x004245dc) but before the partition
// helpers, and none of it references a __FILE__ string.  The query code is called from
// CollisionObject.cpp and CollisionShapeTests, not from the builder.  It may be the end of
// BoundingBoxTreeBuild.cpp's neighbour or a file of its own without a __FILE__ string.  Names
// and argument types are ours (tier 3).
#include "bvh/BoundingBoxTreeBuild.h"

void operator delete(void* p);   // 0x004a30c0

// 0x00578f4c: extra argument the three query entry points below park in a global before calling
// the worker (read by the workers; role unknown).
extern int g_boxTreeQueryArg;

int BoxTreeQueryWorkerA(void* a, void* b, void* c, void* d, void* e, void* f);   // 0x00429570
int BoxTreeQueryWorkerB(void* a, void* b, void* c, void* d, void* e, void* f);   // 0x004298c0
int BoxTreeQueryWorkerC(void* a, void* b, void* c);                              // 0x00429e90

// 0x00429540.
int BoxTreeQueryA(void* a, void* b, void* c, void* d, void* e, void* f, int arg)
{
    g_boxTreeQueryArg = arg;
    return BoxTreeQueryWorkerA(a, b, c, d, e, f);
}

// 0x00429890.
int BoxTreeQueryB(void* a, void* b, void* c, void* d, void* e, void* f, int arg)
{
    g_boxTreeQueryArg = arg;
    return BoxTreeQueryWorkerB(a, b, c, d, e, f);
}

// 0x00429e60.
int BoxTreeQueryC(void* a, void* b, void* c, int arg)
{
    g_boxTreeQueryArg = arg;
    return BoxTreeQueryWorkerC(a, b, c);
}

// 0x0042a160.  Frees a box tree: interior nodes (volume >= 0) free both subtrees first.
void FreeBoxTree(BoxTreeNode* node)
{
    if (node->volume >= 0.0f) {
        FreeBoxTree(node->child[0]);
        FreeBoxTree(node->child[1]);
    }
    operator delete(node);
}

// 0x0042a580.  out = *v * m with the translation row added (the point taken by pointer; compare
// Vec3TransformPoint 0x0042a510, which takes it by value).
void TransformPointPtr(TreeVec3* out, const TreeVec3* v, const TreeMatrix4* m)
{
    out->x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31 + m->_41;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42;
    out->z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33 + m->_43;
}

// 0x0042a5e0.  The inverse of a rigid transform applied to p: subtract the translation row, then
// multiply by the transposed rotation (dot with each row).
void InverseTransformPoint(TreeVec3* out, TreeVec3 p, const TreeMatrix4* m)
{
    float dx = p.x - m->_41;
    float dy = p.y - m->_42;
    float dz = p.z - m->_43;

    out->x = dx * m->_11 + dy * m->_12 + dz * m->_13;
    out->y = dx * m->_21 + dy * m->_22 + dz * m->_23;
    out->z = dx * m->_31 + dy * m->_32 + dz * m->_33;
}
