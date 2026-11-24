// QuadTreeObject.h -- the object base of the collision quadtree (Quadtree.cpp).
//
// Evidence: RTTI .?AVQuadTreeObject@@ (COL 0x0055b038, vtable 0x005511c4,
// two slots: 0x004dc610 `or ax, 0xffff; ret` and 0x00434ce0 `mov eax, 0xf;
// ret 4`). It is the +0 base of CollisionObject (0x00431e70 writes 0x005511c4
// first) and of SoultreeObject (0x004fb2b0). The return types and the one
// stack argument follow those bodies, so a caller through slot 1 (EventManager
// passes the collision quadtree) stays stack-balanced. Slot 0 is defined in
// Quadtree.cpp; slot 1 is the inline body below. Declared on its own so the
// soultree headers can name the base without the collision declarations.
#ifndef BROADPHASE_QUADTREE_OBJECT_H
#define BROADPHASE_QUADTREE_OBJECT_H

class QuadTree;               // broadphase/Quadtree.h

class QuadTreeObject {
public:
    virtual short ComputeSortKey();        // 0x004dc610 (`or ax, 0xffff`; Quadtree.cpp); NextObjectSorted calls it and ignores the result
    virtual unsigned int ComputeQuadTreeCode(QuadTree* tree) { return 15; }  // 0x00434ce0 (ret 4); Vegetation overrides it with the object's quadtree code (0x00456720)

    QuadTreeObject() { queryStamp = 0; objectTypeId = (char)0xff; }
    short queryStamp;  // +0x04 Quadtree query code (NextObject) skips an object whose short at +4 equals the query counter (queryStamp) and otherwise stores the counter there (Quadtree.cpp lines 568/612/676); Insert zeroes it
    short sortKey;                           // +0x06 not touched by the inlined ctor; keeps objectTypeId at +8
    char objectTypeId;  // +0x08 ctor 0x00431e70 stores TypeRegistry::FindTypeId(CollisionObject) (0x00521ea0) here; 0x00438e70 compares [candidate+8] with the cached CollisionObject (0x568610) and Vegetation (0x568611) ids
};

#endif
