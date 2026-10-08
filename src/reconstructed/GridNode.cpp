// GridNode.cpp -- the GridNode code after Griddraw.cpp: the destructor
// 0x004838a0 (deleting wrapper 0x004838f0, GridNode slot 0), 0x00483910,
// slot 1 0x00483d40 and the leaf lookup 0x00484d70, ending before
// GUIManager.cpp's initializers (0x00484dd0). RTTI .?AVGridNode@@ (COL
// 0x0055cb08, vtable 0x00553ec0) is tier 1; the class is declared in
// Griddraw.h. There is no __FILE__ literal: the file name is ours (tier 3).
// Its .data is the third copy of the shared row tables (0x0056c2ac), which
// 0x00483910 and 0x00484d70 read. 0x00483910 (cell sampler), slot 1 (segment
// intersection) and 0x00484d70 are near misses (samples/render/GridNodeNearMisses.cpp).

#include "Griddraw.h"

#include "DebugAlloc.h"


// 0x004838a0
GridNode::~GridNode()
{
    if (field_0x30) {
        delete field_0x30;
    }
    if (children) {
        for (int i = 0; i < 256; i++) {
            if (children[i]) {
                delete children[i];
            }
        }
        delete children;
    }
}
