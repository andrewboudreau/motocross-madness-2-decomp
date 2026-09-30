// QuadTree.h -- collision broad phase: QuadTree / QuadTreeNode (Quadtree.cpp).
//
// Evidence (tier 1 unless noted; analysis/*.json + target bytes):
//  * RTTI .?AVQuadTree@@ (vtable 0x0055763c, 2 slots) and .?AVQuadTreeNode@@
//    (vtable 0x00557648, 3 slots); both roots (vtable_overrides relation "root"),
//    primary vtable only (object_offset 0).
//  * Scalar deleting dtors: QuadTree 0x004dda90 -> core 0x004dc840,
//    QuadTreeNode 0x004dd0a0 -> core 0x004dd0c0 (deleting_destructors.json).
//  * QuadTree has no out-of-line ctor: its only vptr write is inlined at 0x0052d35f.
//  * Source: D:\aardvark\VC\krusty2\Quadtree.cpp (__FILE__ 0x00572040; lines 0x73,
//    0x74, 0x7a, 0x85, 0x690 at the xrefs in the node factory / dtor / Grow).
//  * operator new(0x14, __FILE__, 0x7a) in the node factory 0x004dc7e0, so
//    sizeof(QuadTreeNode) == 0x14.  Grow (0x004ddce0) reallocs a node to 0x24 bytes
//    (4 child pointers inline, tier 2).
//
// Semantics are tier 2/3; names other than field_0xNN / UnknownVirtualSlotN are tier 3
// and justified in the .cpp.
#ifndef BROADPHASE_QUADTREE_H
#define BROADPHASE_QUADTREE_H

#include "../collision/CollisionObject.h"   // QuadTreeObject (canonical)

// Debug allocation forms (size/ptr, __FILE__, __LINE__): 0x004a3010 / 0x004a2e60.
void* operator new(unsigned int size, const char* file, int line);
void operator delete(void* p, const char* file, int line);
void* DebugRealloc(void* p, unsigned int size, const char* file, int line);   // 0x004a2ec0 (cdecl)

// Fixed-size block pool (retail 0x00423f70 ctor(elemSize, count), 0x00423fb0 dtor,
// 0x00423fc0 Alloc, 0x00424050 Free, 0x00424110 Reset; object size 0x28 from the
// operator new at 0x4dc751).  Owned by another TU: PROVISIONAL declaration only.
class QuadTreePool {
public:
    QuadTreePool(unsigned int elemSize, unsigned int count);
    ~QuadTreePool();
    void* Alloc();
    void Free(void* p);
    void Reset();
private:
    char field_0x00[0x28];
};

class QuadTreeNode;

// One block of a node's object list: next link + 4 object slots (pool element 0x14).
struct QuadTreeItemLink {
    void Clear() {
        next = 0;
        objects[0] = 0;
        objects[1] = 0;
        objects[2] = 0;
        objects[3] = 0;
    }
    QuadTreeItemLink* next;
    QuadTreeObject* objects[4];
};


// Query result cell (pool element 8).
struct QuadTreeResultLink {
    QuadTreeResultLink* next;
    QuadTreeNode* item;
};

class QuadTreeNode {
public:
    virtual ~QuadTreeNode();                                     // 0x004dd0a0 -> core 0x004dd0c0
    // Slots 1/2 (also overridden by VisibilityQuadTreeNode, vtable_overrides.json) walk the
    // node's four children back to front relative to the point (x0, z0); the child order is
    // decoded in the .cpp (tier 2).  Arguments: query point, node origin, node half size.
    virtual void UnknownVirtualSlot1(int x0, int z0, int nodeX, int nodeZ, int size);   // 0x004ddab0 (ret 0x14)
    virtual QuadTreeResultLink* UnknownVirtualSlot2(int x0, int z0, QuadTreeResultLink* tail,
                                                    int nodeX, int nodeZ, int size);    // 0x004dd7a0 (ret 0x18)

    QuadTreeNode();                                              // 0x004dd070
    void AddObject(QuadTreeObject* obj);                         // 0x004dd130 (ret 4)
    int RemoveObject(QuadTreeObject* obj);                       // 0x004dd1b0 (ret 4)
    QuadTreeResultLink* Gather(QuadTreeResultLink* tail, int x0, int z0, int x1, int z1,
                               int nodeX, int nodeZ, int size, int minSize);   // 0x004dd3e0 (ret 0x24)
    QuadTreeResultLink* GatherAll(QuadTreeResultLink* tail);     // 0x004dd4f0 (ret 4)
    QuadTreeNode* Grow();                                        // 0x004ddce0
    void ExtendY(float y0, float y1);                            // inlined at 0x4dcb58 / 0x4dcc3f

    QuadTreeNode** field_0x04;       // child array [4]
    QuadTreeItemLink* field_0x08;    // object list head
    float field_0x0c;                // ctor 0
    float field_0x10;                // ctor -FLT_MAX (0xff7fffff)
};

class QuadTree {
public:
    virtual ~QuadTree();                                         // 0x004dda90 -> core 0x004dc840
    virtual QuadTreeNode* UnknownVirtualSlot1();                 // 0x004dc7e0 node factory

    void Reset();                                                // 0x004dc620
    unsigned int ComputeCode(float x0, float z0, float x1, float z1); // 0x004dc8c0 (ret 0x10)
    void Insert(QuadTreeObject* obj, unsigned int code, float y0, float y1); // 0x004dcac0 (ret 0x10)
    void UpdateRange(QuadTreeObject* obj, unsigned int code, float y0, float y1); // 0x004dcd40 (ret 0x10)
    void Remove(QuadTreeObject* obj, unsigned int code);         // 0x004dcf20 (ret 8)
    void ExtendRootY(float y0, float y1);                        // inlined at 0x4dcb58 / 0x4dcdca
    int BeginQuery(float x0, float z0, float x1, float z1);      // 0x004dd270 (ret 0x10)
    QuadTreeObject* NextObject();                                // 0x004dd540
    QuadTreeObject* NextObjectSorted();                          // 0x004dd600
    void RestartQuery();                                         // 0x004dd750
    void EndQuery();                                             // 0x004dd770
    void CollectItems(QuadTreeItemLink* link);                   // 0x004ddd20 (ret 4)
    int IsValidCode(unsigned int code);                          // 0x004ddd90 (ret 4)
    void Init(float x0, float z0, float x1, float z1, float minCell); // 0x004dc670 (ret 0x14)

    // Layout (tier 2: offsets seen in the bodies; names tier 3 where commented).
    int field_0x04;
    int field_0x08;
    float field_0x0c;
    int field_0x10[12];
    float field_0x40;                // world rectangle min x (Init arg 1)
    float field_0x44;                // max x (Init arg 3)
    float field_0x48;                // min z (Init arg 2)
    float field_0x4c;                // max z (Init arg 4)
    float field_0x50;                // x extent
    float field_0x54;                // z extent
    int field_0x58;
    QuadTreeNode* field_0x5c;        // root node (dtor calls its deleting dtor)
    QuadTreePool* field_0x60;        // pool of QuadTreeItemLink (0x14 elements)
    QuadTreePool* field_0x64;        // pool of QuadTreeResultLink (8 byte elements)
    unsigned int field_0x68;         // bit 1 (2) = tree is being torn down
    unsigned short field_0x6c;
    unsigned short field_0x6e;
    QuadTreeResultLink* field_0x70;
    QuadTreeResultLink* field_0x74;
    int field_0x78;
    QuadTreeItemLink* field_0x7c;
    int field_0x80;
    QuadTreeObject* field_0x84[500]; // sorted-query object stack (0x1f4 entries max)
    int field_0x854;                 // live node count (QuadTreeNode ctor++/dtor--)
    int field_0x858;
    int field_0x85c;
    int field_0x860;
    QuadTreeObject** field_0x864;    // result array, freed with debug delete (line 0x85) by the dtor
    int field_0x868;
    int field_0x86c;                 // result count
    int field_0x870;                 // result capacity
};

extern float g_quadTreeInvScale;     // 0x00689b74: 1 / field_0x0c set by Init
extern QuadTree* g_pQuadTree;        // 0x00689b78: the "current" tree

#endif
