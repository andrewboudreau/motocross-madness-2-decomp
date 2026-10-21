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

#include "../collision/CollisionObject.h"   // QuadTreeObject (canonical)

// Debug allocation forms (size/ptr, __FILE__, __LINE__): core/DebugAlloc.h.
#include "../core/DebugAlloc.h"

#include "../../reconstructed/BlockAllocator.h"

#include "QuadTreeDeclarations.h"

#endif
