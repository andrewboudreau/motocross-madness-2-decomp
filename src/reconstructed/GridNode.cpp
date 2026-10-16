// GridNode.cpp -- the GridNode code after Griddraw.cpp: the destructor
// 0x004838a0 (deleting wrapper 0x004838f0, GridNode slot 0), 0x00483910,
// slot 1 0x00483d40 and the leaf lookup 0x00484d70, ending before
// GUIManager.cpp's initializers (0x00484dd0). RTTI .?AVGridNode@@ (COL
// 0x0055cb08, vtable 0x00553ec0) is tier 1; the class is declared in
// Griddraw.h. There is no __FILE__ literal: the file name is ours (tier 3).
// Its .data is the third copy of the shared row tables (0x0056c2ac), which
// 0x00483910 and 0x00484d70 read. The cell sampler 0x00483910 is exact; slot 1
// (segment intersection) and 0x00484d70 are near misses
// (samples/render/GridNodeNearMisses.cpp).

#include "Griddraw.h"

#include "DebugAlloc.h"
#include "MatrixUtil.h"

// The third copy of the shared row tables (0x0056c2ac, 0x0056c2f0).
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
    if (childOffsets) {
        delete childOffsets;
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

// 0x00483910. The block's cells hold the height relative to the block's
// base (+0xd4c) and a signed-byte normal scaled by 1/127 (0x00553ec8, the
// float right after the GridNode vtable). Retail's loop is VC6's
// tail-recursion elimination of the child call; the clamped copies of x and
// z (xx, zz) are what retail keeps in ebp/[esp+0x14] and in z's argument home.
int GridNode::GetCellCorners(int x, int z, float* heights, GridVec3* normals, unsigned char* bytes)
{
    int xx = x;
    int zz = z;
    const unsigned char& lv = level;
    if (lv == 0) {
        GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
        if (normals) {
            float scale = reliefScale * (1.0f / 127.0f);
            normals[0].x = cell->normal[0] * scale;
            normals[0].y = cell->normal[1] * (1.0f / 127.0f);
            normals[0].z = cell->normal[2] * scale;
            heights[0] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[0] = cell->surface;
            }
            cell++;
            normals[1].x = cell->normal[0] * scale;
            normals[1].y = cell->normal[1] * (1.0f / 127.0f);
            normals[1].z = cell->normal[2] * scale;
            heights[1] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[1] = cell->surface;
            }
            cell += 17;
            normals[3].x = cell->normal[0] * scale;
            normals[3].y = cell->normal[1] * (1.0f / 127.0f);
            normals[3].z = cell->normal[2] * scale;
            heights[3] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[3] = cell->surface;
            }
            cell--;
            normals[2].x = cell->normal[0] * scale;
            normals[2].y = cell->normal[1] * (1.0f / 127.0f);
            normals[2].z = cell->normal[2] * scale;
            heights[2] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[2] = cell->surface;
            }
        } else {
            heights[0] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[0] = cell->surface;
            }
            cell++;
            heights[1] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[1] = cell->surface;
            }
            cell += 17;
            heights[3] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[3] = cell->surface;
            }
            cell--;
            heights[2] = ((cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
            if (bytes) {
                bytes[2] = cell->surface;
            }
        }
        return 1;
    }
    if (children != 0) {
        int shift = lv * 4;
        int mask = ~(-1 << shift);
        int one = 1 << shift;
        int cx = x >> shift;
        int cz = z >> shift;
        GridNode* child;
        if (cx < 16 && cz < 16 && (child = children[g_gridRow16[cz] + cx]) != 0) {
            return child->GetCellCorners(x & mask, z & mask, heights, normals, bytes);
        }
        if (cx < 0) {
            cx = 0;
            xx = 0;
        }
        if (cz < 0) {
            cz = 0;
            zz = 0;
        }
        if (cx >= 16) {
            cx = 15;
            xx = one - 1;
        }
        if (cz >= 16) {
            cz = 15;
            zz = one - 1;
        }
        child = children[g_gridRow16[cz] + cx];
        if (child == 0) {
            goto missing;
        }
        return child->GetCellCorners(xx & mask, zz & mask, heights, normals, bytes);
    }
missing:
    for (int i = 0; i < 4; i++) {
        if (normals) {
            normals[i] = GridVec3(0.0f, 1.0f, 0.0f);
        }
        heights[i] = 0.0f;
        bytes[i] = 0;
    }
    return 0;
}
