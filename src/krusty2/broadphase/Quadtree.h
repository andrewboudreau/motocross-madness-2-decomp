// Quadtree.h -- collision broad phase: QuadTree / QuadTreeNode (Quadtree.cpp).
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

#include "collision/CollisionObject.h"   // QuadTreeObject (canonical)

// Debug allocation forms (size/ptr, __FILE__, __LINE__): core/DebugAlloc.h.
#include "core/DebugAlloc.h"

#include "../../reconstructed/BlockAllocator.h"

class QuadTreeNode;

// One block of a node's object list: next link + 4 object slots (pool element 0x14).
struct QuadTreeItemLink {
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

    QuadTreeNode** children;       // +0x04 child array [4]
    QuadTreeItemLink* itemList;    // +0x08 object list head
    float yCenter;                // +0x0c ctor 0
    float yHalfExtent;                // +0x10 ctor -FLT_MAX (0xff7fffff)
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
    int maxDepth;  // +0x04 Init loop count-1 of the cell table; ComputeCode clamps depth to it; IsValidCode compares depth with it
    int leafCellSize;  // +0x08 Init: = cellSizes[maxDepth]; passed as minSize to root Gather in BeginQuery
    float quantScale;  // +0x0c Init: 32768 / max(extentX, extentZ); multiplies world coords into the 15-bit grid
    int cellSizes[12];  // +0x10 Init: cellSizes[d] = 0x4000 >> d; indexed by depth in Insert/Remove/ComputeCode
    float worldMinX;                // +0x40 world rectangle min x (Init arg 1)
    float worldMaxX;                // +0x44 max x (Init arg 3)
    float worldMinZ;                // +0x48 min z (Init arg 2)
    float worldMaxZ;                // +0x4c max z (Init arg 4)
    float extentX;                // +0x50 x extent
    float extentZ;                // +0x54 z extent
    int field_0x58;
    QuadTreeNode* rootNode;        // +0x5c root node (dtor calls its deleting dtor)
    BlockAllocator* itemPool;        // +0x60 pool of QuadTreeItemLink (0x14 elements)
    BlockAllocator* resultPool;        // +0x64 pool of QuadTreeResultLink (8 byte elements)
    unsigned int stateFlags;         // +0x68 bit 1 (2) = tree is being torn down
    unsigned short queryStamp;  // +0x6c ++ in BeginQuery/RestartQuery; compared with QuadTreeObject stamp (maxDepth) to return each object once
    unsigned short field_0x6e;
    QuadTreeResultLink* queryListHead;  // +0x70 BeginQuery stores the Gather result; RestartQuery copies it to the cursor
    QuadTreeResultLink* queryCursor;  // +0x74 current result cell iterated by NextObject/NextObjectSorted
    int itemSlotIndex;  // +0x78 0..3 index into the current item block, reset to 0 per block
    QuadTreeItemLink* itemCursor;  // +0x7c current QuadTreeItemLink block of the cell being iterated
    int sortedCount;  // +0x80 number of entries in the sorted stack sortedStack; popped with --
    QuadTreeObject* sortedStack[500]; // +0x84 sorted-query object stack (0x1f4 entries max)
    int liveNodeCount;                 // +0x854 live node count (QuadTreeNode ctor++/dtor--)
    int field_0x858;
    int objectCount;  // +0x85c Insert ++ / Remove -- (once per call, not per cell)
    int field_0x860;
    QuadTreeObject** resultArray;    // +0x864 result array, freed with debug delete (line 0x85) by the dtor
    int field_0x868;
    int resultCount;                 // +0x86c result count
    int resultCapacity;                 // +0x870 result capacity
};

extern float g_quadTreeInvScale;     // 0x00689b74: 1 / field_0x0c set by Init
extern QuadTree* g_pQuadTree;        // 0x00689b78: the "current" tree

#endif
