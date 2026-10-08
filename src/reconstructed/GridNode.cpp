// GridNode.cpp -- the GridNode code after Griddraw.cpp: the destructor
// 0x004838a0 (deleting wrapper 0x004838f0, GridNode slot 0), 0x00483910,
// slot 1 0x00483d40 and the leaf lookup 0x00484d70, ending before
// GUIManager.cpp's initializers (0x00484dd0). RTTI .?AVGridNode@@ (COL
// 0x0055cb08, vtable 0x00553ec0) is tier 1; the class is declared in
// Griddraw.h. There is no __FILE__ literal: the file name is ours (tier 3).
// Its .data is the third copy of the shared row tables (0x0056c2ac), which
// 0x00483910 and 0x00484d70 read. Slot 1 is not reconstructed; 0x00483910
// (corner fetch) and 0x00484d70 are near misses
// (samples/render/GridNodeNearMisses.cpp).

#include "Griddraw.h"

#include "DebugAlloc.h"

// Row offsets into 16 x 16 and 17 x 17 grids (this TU's copy, 0x0056c2ac and
// 0x0056c2f0; see Griddraw.cpp).
static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

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
