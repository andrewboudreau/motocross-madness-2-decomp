// Griddraw.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\Griddraw.cpp.
// See Griddraw.h and docs/GRIDDRAW.md.

#include "Griddraw.h"

#include <float.h>
#include <string.h>

#include "DebugAlloc.h"
#include "TextureMap.h"

// Per-TU vector constants (tier 2, the shape documented in docs/LZW.md):
// four dynamic initializers (0x004807c0..0x004808fb, listed first in this
// TU's .CRT$XCU entries 0x00566224..0x00566230) build (0,0,0), (1,0,0),
// (0,1,0) and (0,0,1) into 0x006754c0, 0x006754d0, 0x00677900 and
// 0x006754b0. Nothing else in the TU reads them.
struct GridConstVec3 {
    float x, y, z;
    GridConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const GridConstVec3 kVec3Zero = GridConstVec3(0.0f, 0.0f, 0.0f);
static const GridConstVec3 kVec3XAxis = GridConstVec3(1.0f, 0.0f, 0.0f);
static const GridConstVec3 kVec3YAxis = GridConstVec3(0.0f, 1.0f, 0.0f);
static const GridConstVec3 kVec3ZAxis = GridConstVec3(0.0f, 0.0f, 1.0f);

// Row offsets into 16 x 16 and 17 x 17 grids. Tier 2: the same two tables
// precede each of the Grid*.cpp files' own data (0x0056c0c8, 0x0056c174,
// 0x0056c2ac), so they are per-TU statics from a shared header.
static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

// Griddraw's own initialised data (0x0056c200..0x0056c287).
int g_gridLevels = 9;                                   // 0x0056c200
float g_gridDrawNormalScale = 1.0f / 12.0f;             // 0x0056c204
// 0x0056c208: offset of the first cell of each 4 x 4 block in a 16 x 16
// grid, in quadrant (Z) order.
int g_gridQuadCell[16] = {
    0x00, 0x04, 0x40, 0x44, 0x08, 0x0c, 0x48, 0x4c,
    0x80, 0x84, 0xc0, 0xc4, 0x88, 0x8c, 0xc8, 0xcc,
};
// 0x0056c248: Z-order index of each 4 x 4 cell.
int g_gridQuadOrder[16] = {
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15,
};

// Defined with Terrain's data (0x0068a300 / 0x0068a304); tier 3 names.
extern int g_gridDrawMemory;      // bytes held by node vertex buffers
extern int g_gridDrawFrameCount;  // top-level node visits

int g_gridSize = 1 << (g_gridLevels - 1);               // 0x0067a684 ($E 0x0047ddb0)
GridVertexCache g_gridVertexCache;                      // 0x00677910 ($E 0x0047dde0)

GridEdge g_gridEdges[17 * 17];                          // 0x006754e0
int g_gridOriginX;                                      // 0x006754dc (texture origin, set by 0x0047f210)
int g_gridOriginZ;                                      // 0x0067a688
// The triangle index list being built (0x0067a68c; 0xd8c bytes up to the
// next known global, so at most 578 triangles).
unsigned short g_gridIndices[0xd8c / 2];
int g_gridEdgesBuilt;                                   // 0x0067b418
int g_gridExtraBytes;                                   // 0x0067b424 bytes of per-node extra data

// 0x0047de20
void GridVertexCache::Reset(DrawableGridNode* newNode)
{
    node = newNode;
    count = 0;
    minY = FLT_MAX;
    maxY = -FLT_MAX;
    if (++stamp == 0) {
        for (int i = 0; i < 17 * 17; i++) {
            slots[i].stamp = 0;
            slots[i].index = 0;
        }
        stamp++;
    }
    if (node == 0)
        memset(vertices, 0, sizeof(vertices));
}

// 0x0047e340: the two neighbours (x0, z0) and (x1, z1) a vertex at (x, z)
// interpolates between at a given subdivision step.
void UnknownFunction47e340(int x, int z, int h, int v, int maskX, int maskZ, GridEdge* out)
{
    if (!v) {
        if (z & h) {
            out->x0 = x - h;
            out->z0 = z;
            out->x1 = x + h;
            out->dir0 = 4;
            out->z1 = z;
            out->dir1 = 1;
        } else {
            out->x0 = x;
            out->z0 = z - h;
            out->x1 = x;
            out->dir0 = 8;
            out->z1 = z + h;
            out->dir1 = 2;
        }
    } else if (((z - v) & maskZ) == ((x - h) & maskX)) {
        out->x0 = x + h;
        out->z0 = z - v;
        out->x1 = x - h;
        out->dir0 = 4;
        out->z1 = z + v;
        out->dir1 = 2;
    } else {
        out->x0 = x - h;
        out->z0 = z - v;
        out->x1 = x + h;
        out->dir0 = 8;
        out->z1 = z + v;
        out->dir1 = 1;
    }
}

// 0x0047e4d0
static void UnknownFunction47e4d0()
{
    int half = 1;
    int step = 2;
    int rowStep = 1;
    while (half < 16) {
        UnknownFunction47e430(half, 0, step, rowStep);
        rowStep <<= 1;
        UnknownFunction47e430(half, half, step, rowStep);
        half <<= 1;
        step <<= 1;
    }
    for (int z = 0; z <= 16; z++) {
        for (int x = 0; x <= 16; x++) {
            g_gridEdges[g_gridRow17[z] + x].x = x;
            g_gridEdges[g_gridRow17[z] + x].z = z;
        }
    }
}

// 0x0047e540
DrawableGridNode::DrawableGridNode(int a0, int a1, GridNode* parent, GridTerrain* terrain, int a4,
                                   int a5, int a6, int a7, int a8, int a9, int a10)
{
    this->terrain = terrain;
    if (g_gridEdgesBuilt == 0) {
        UnknownFunction47db60();
        UnknownFunction47e4d0();
        for (int i = 0; i < 17 * 17; i++)
            this->terrain->field_0xb8[i] = 0;
        g_gridEdgesBuilt = 1;
    }
    children = 0;
    field_0x30 = 0;
    block = 0;
    childMask = 0;
    data = 0;
    extra = 0;
    this->parent = parent;
}

// 0x0047ec60
DrawableGridNode::~DrawableGridNode()
{
    if (data)
        delete data;
}

// 0x0047ecc0: AgeManager eviction callback (registered by 0x0047f210 and
// 0x0047f840 with the node as owner): frees both vertex buffers and marks
// the node for a rebuild.
int UnknownFunction47ecc0(void* owner)
{
    GridNodeDrawData* d = ((DrawableGridNode*)owner)->data;
    if (d->field_0x134) {
        g_gridDrawMemory -= d->field_0x140;
        operator delete(d->field_0x134, __FILE__, 816);
        d->field_0x134 = 0;
        d->field_0x13c = 0;
        d->field_0x140 = 0;
    }
    if (d->field_0x138) {
        g_gridDrawMemory -= d->field_0x142;
        operator delete(d->field_0x138, __FILE__, 823);
        d->field_0x138 = 0;
        d->field_0x13e = 0;
        d->field_0x142 = 0;
    }
    d->ageEntry.size = 0;
    d->field_0x126 = 0;
    d->field_0x122 = 0;
    d->field_0x12a = 0;
    d->field_0x128 = 0xffff;
    d->field_0x124 = 0xffff;
    d->b0 = 0;
    d->b1 = 1;
    return 1;
}

// 0x0047edb0: releases the vertex buffers of this node (and, when asked, of
// every descendant) and takes its count out of the parent's total.
void DrawableGridNode::UnknownFunction47edb0(int recurse)
{
    if (parent) {
        ((DrawableGridNode*)parent)->data->field_0x12c -= data->field_0x12c + data->field_0x12a;
        if (((DrawableGridNode*)parent)->data->field_0x12c < 0)
            ((DrawableGridNode*)parent)->data->field_0x12c = 0;
    }
    data->field_0x12a = 0;
    data->b1 = 1;
    data->b0 = 0;
    if (recurse && children) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->UnknownFunction47edb0(recurse);
        }
    }
    if (data->field_0x134) {
        g_gridDrawMemory -= data->field_0x140;
        operator delete(data->field_0x134, __FILE__, 858);
        data->field_0x134 = 0;
        data->field_0x13c = 0;
        data->field_0x140 = 0;
    }
    if (data->field_0x138) {
        g_gridDrawMemory -= data->field_0x142;
        operator delete(data->field_0x138, __FILE__, 865);
        data->field_0x138 = 0;
        data->field_0x13e = 0;
        data->field_0x142 = 0;
    }
    if (data->field_0x12c != 0)
        data->field_0x12c = 0;
    data->field_0x126 = 0;
    data->field_0x122 = 0;
    data->field_0x128 = 0xffff;
    data->field_0x124 = 0xffff;
    if (data->ageEntry.size) {
        terrain->field_0xc88->UnknownFunction4010d0(&data->ageEntry);
        data->ageEntry.size = 0;
    }
}

// 0x0047ef70
int DrawableGridNode::UnknownFunction47ef70()
{
    return UnknownFunction47ef80(0, 0, 16, 0);
}

// 0x0047ef80: walks the visible quadrants of this node (8-cell, then 4-cell
// blocks) down to the child nodes, rebuilds a dirty node's vertex buffer at
// the top-level call and adds the node's count to its parent's total.
// `size` is halved in place; the 8-cell branch restores 16 so the top-level
// rebuild below still runs (retail jumps straight into it from there).
int DrawableGridNode::UnknownFunction47ef80(int x, int z, int size, int quad)
{
    if (childMask == 0)
        return 1;
    if (size == 16)
        g_gridDrawFrameCount++;
    if (children != 0 && !data->b0) {
        size >>= 1;
        if (size == 8) {
            data->field_0x12c = 0;
            if (childMask & 2)
                UnknownFunction47ef80(x, z, 8, 0);
            if (childMask & 4)
                UnknownFunction47ef80(x + 8, z, 8, 4);
            if (childMask & 8)
                UnknownFunction47ef80(x, z + 8, 8, 8);
            if (childMask & 0x10)
                UnknownFunction47ef80(x + 8, z + 8, 8, 12);
            size = 16;
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                UnknownFunction47ef80(x, z, 4, quad);
            if (childMask & (0x40 << quad))
                UnknownFunction47ef80(x + 4, z, 4, quad + 1);
            if (childMask & (0x80 << quad))
                UnknownFunction47ef80(x, z + 4, 4, quad + 2);
            if (childMask & (0x100 << quad))
                UnknownFunction47ef80(x + 4, z + 4, 4, quad + 3);
        } else {
            for (int j = 0; j < 64; j += 16) {
                for (int i = 0; i < 4; i++) {
                    DrawableGridNode* child = (DrawableGridNode*)children[g_gridQuadCell[quad] + i + j];
                    if (child && child->childMask)
                        child->UnknownFunction47ef80(0, 0, 16, 0);
                }
            }
        }
    }
    if (size == 16 && data->b1) {
        int before = data->field_0x12c + data->field_0x12a;
        if (data->b3)
            UnknownFunction47f210();
        else
            UnknownFunction47f840();
        int after = data->field_0x12c + data->field_0x12a;
        if ((before >= 2 && after < 2) || (before < 2 && after >= 2)) {
            if (parent)
                ((DrawableGridNode*)parent)->data->b1 = 1;
        }
        data->b1 = 0;
        if (data->b2)
            terrain->field_0xc40->UnknownFunction49e4a0(terrain->field_0xc44, data->field_0x13c,
                (char*)data->field_0x134 + 0xc, data->field_0x134, 0x20, 0);
    }
    if (parent)
        ((DrawableGridNode*)parent)->data->field_0x12c += data->field_0x12c + data->field_0x12a;
    return 1;
}

// 0x00480700: appends triangle n of a cell corner: (x, z), (x + dx, z) and
// (x, z + dz), wound by whether the step signs agree.
int DrawableGridNode::UnknownFunction480700(int n, int x, int z, int dx, int dz)
{
    int k = n * 3;
    if (dx == dz) {
        g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x, z);
        g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z + dz);
        g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x + dx, z);
    } else {
        g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
        g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x + dx, z);
        g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x, z + dz);
    }
    return n + 1;
}

// 0x00480900
int DrawableGridNode::UnknownFunction480900(void* target)
{
    return UnknownFunction480940(target, 0, 0, 16, 0, 0);
}

// 0x00480920
int DrawableGridNode::UnknownFunction480920(void* target, int a5)
{
    return UnknownFunction480940(target, 0, 0, 16, 0, a5);
}

// 0x00480940: the same quadrant walk as 0x0047ef80; 4-cell blocks go to
// 0x00480ad0.
int DrawableGridNode::UnknownFunction480940(void* target, int x, int z, int size, int quad, int a5)
{
    if (!data->b0 || data->b3) {
        size >>= 1;
        if (size == 8) {
            if (childMask & 2)
                UnknownFunction480940(target, x, z, 8, 0, a5);
            if (childMask & 4)
                UnknownFunction480940(target, x + 8, z, 8, 4, a5);
            if (childMask & 8)
                UnknownFunction480940(target, x, z + 8, 8, 8, a5);
            if (childMask & 0x10)
                UnknownFunction480940(target, x + 8, z + 8, 8, 12, a5);
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                UnknownFunction480ad0(target, x, z, 4, quad, a5);
            if (childMask & (0x40 << quad))
                UnknownFunction480ad0(target, x + 4, z, 4, quad + 1, a5);
            if (childMask & (0x80 << quad))
                UnknownFunction480ad0(target, x, z + 4, 4, quad + 2, a5);
            if (childMask & (0x100 << quad))
                UnknownFunction480ad0(target, x + 4, z + 4, 4, quad + 3, a5);
        }
    }
    return 1;
}

// 0x00481170
int DrawableGridNode::UnknownFunction481170()
{
    return UnknownFunction481180(0, 0, 16, 0);
}

// 0x00481180
int DrawableGridNode::UnknownFunction481180(int x, int z, int size, int quad)
{
    if (data->field_0x13c != 0 && childMask != 0 && size == 16 && extra->field_0x08 != 0 && !data->b3)
        UnknownVirtualSlot4(-1);
    if (!data->b0 || data->b3) {
        size >>= 1;
        if (size == 8) {
            data->field_0x178 = 0;
            if (childMask & 2)
                UnknownFunction481180(x, z, 8, 0);
            if (childMask & 4)
                UnknownFunction481180(x + 8, z, 8, 4);
            if (childMask & 8)
                UnknownFunction481180(x, z + 8, 8, 8);
            if (childMask & 0x10)
                UnknownFunction481180(x + 8, z + 8, 8, 12);
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                UnknownFunction481300(x, z, 4, quad);
            if (childMask & (0x40 << quad))
                UnknownFunction481300(x + 4, z, 4, quad + 1);
            if (childMask & (0x80 << quad))
                UnknownFunction481300(x, z + 4, 4, quad + 2);
            if (childMask & (0x100 << quad))
                UnknownFunction481300(x + 4, z + 4, 4, quad + 3);
        }
    }
    return 1;
}

// 0x00481580
void DrawableGridNode::UnknownFunction481580()
{
    if (childMask && children && !data->b0) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->UnknownFunction481580();
        }
    } else if (childMask) {
        for (int z = 0; z <= 16; z++) {
            for (int x = 0; x <= 16; x++)
                UnknownFunction481cc0(x, z);
        }
    }
}

// 0x004824c0: updates the node's level of detail; nodes that stay coarse
// release their children's buffers, finer ones load their child nodes from
// the stream on first use (the same layout 0x0047e600 reads).
void DrawableGridNode::UnknownFunction4824c0(UnknownTextureStream* stream, void* a1, int recurse, int coarse)
{
    float detail = UnknownFunction4815e0(coarse);
    UnknownVirtualSlot3(detail);
    if (field_0x20 <= data->field_0x122 && extra->field_0x08) {
        if (data->b0)
            return;
        data->b0 = 1;
        data->b1 = 1;
        if (children && recurse) {
            for (int i = 0; i < 256; i++) {
                if (children[i])
                    ((DrawableGridNode*)children[i])->UnknownFunction47edb0(1);
            }
        }
        return;
    }
    if (data->b0) {
        data->b0 = 0;
        data->b1 = 1;
    }
    if (field_0x2c == 0)
        return;
    if (children == 0) {
        children = new (__FILE__, 3383) GridNode*[256];
        field_0x30 = new (__FILE__, 3384) int[256];
        stream->UnknownFunction461340(terrain->field_0xc38 + field_0x2c, 0, 1);
        stream->UnknownFunction461640(field_0x30, 0x400, 1);
        for (int i = 0; i < 256; i++) {
            if (field_0x30[i]) {
                stream->UnknownFunction461340(terrain->field_0xc38 + field_0x30[i], 0, 1);
                children[i] = UnknownVirtualSlot2(stream, 0, this, level - 1, shift - 4,
                                                  (gridX + i % 16) * 16, (gridZ + i / 16) * 16, i, 0, 0);
            } else {
                children[i] = 0;
            }
        }
    }
    if (recurse) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->UnknownFunction4824c0(stream, a1, 1, data->b0 | coarse);
        }
    }
}

// 0x004826d0: propagates bit 21 of the mask (all 22 bits set) to the child
// nodes, rebuilds the edge links and resets the index ranges.
void DrawableGridNode::UnknownFunction4826d0()
{
    if (children) {
        for (int i = 0; i < 256; i++) {
            if (children[i]) {
                if (childMask & 0x200000)
                    ((DrawableGridNode*)children[i])->childMask = 0x3fffff;
                ((DrawableGridNode*)children[i])->UnknownFunction4826d0();
            }
        }
    }
    UnknownFunction482ae0();
    data->field_0x130 = terrain->field_0xb4;
    data->field_0x126 = 0;
    data->field_0x122 = 0;
    data->field_0x128 = 0xffff;
    data->field_0x124 = 0xffff;
}

// 0x00482a40: re-tests one vertex and flags it (bit 6: failed the test,
// bit 7: in use) and its neighbours when its state changes.
void DrawableGridNode::UnknownFunction482a40(int x, int z)
{
    terrain->field_0xb0++;
    int i = g_gridRow17[z] + x;
    if (!UnknownFunction481cc0(x, z)) {
        data->field_0x000[i] |= 0x40;
        if (data->field_0x000[i] & 0x80)
            return;
        data->field_0x000[i] |= 0x80;
    } else {
        data->field_0x000[i] &= ~0x40;
        if (data->field_0x000[i] & 0x10)
            return;
        if (!(data->field_0x000[i] & 0x80))
            return;
        data->field_0x000[i] &= ~0x80;
    }
    data->b1 = 1;
    UnknownFunction482c90(x, z, data->field_0x000[i] & 0x80);
}

// 0x00482ae0: the same subdivision sequence as 0x0047e4d0 over the whole
// node, then one pass with step 16.
void DrawableGridNode::UnknownFunction482ae0()
{
    int half = 1;
    int step = 2;
    int rowStep = 1;
    while (half < 16) {
        UnknownFunction482b40(0, 0, 16, 16, half, 0, step, rowStep);
        rowStep <<= 1;
        UnknownFunction482b40(0, 0, 16, 16, half, half, step, rowStep);
        half <<= 1;
        step <<= 1;
    }
    UnknownFunction482b40(0, 0, 16, 16, 0, 0, 16, 16);
}

// 0x00482c90: passes a vertex change on to the two vertices it depends on;
// the centre vertex depends on two corners of the parent node, picked by
// the node's position in the parent (+0x2a: x in the low nibble, z in the
// high nibble).
void DrawableGridNode::UnknownFunction482c90(int x, int z, int flag)
{
    terrain->field_0xa0++;
    int i = g_gridRow17[z] + x;
    if (x == 8 && z == x) {
        if (parent) {
            if ((field_0x2a & 0x11) != 0x11 && (field_0x2a & 0x11) != 0) {
                ((DrawableGridNode*)parent)->UnknownFunction482dd0(field_0x2a & 0xf, field_0x2a >> 4, flag, 8, 0);
                ((DrawableGridNode*)parent)->UnknownFunction482dd0((field_0x2a & 0xf) + 1, (field_0x2a >> 4) + 1, flag, 1, 0);
            } else {
                ((DrawableGridNode*)parent)->UnknownFunction482dd0(field_0x2a & 0xf, (field_0x2a >> 4) + 1, flag, 2, 0);
                ((DrawableGridNode*)parent)->UnknownFunction482dd0((field_0x2a & 0xf) + 1, field_0x2a >> 4, flag, 4, 0);
            }
        }
    } else {
        if (g_gridEdges[i].x0 != g_gridEdges[i].x1 || g_gridEdges[i].z0 != g_gridEdges[i].z1) {
            UnknownFunction482dd0(g_gridEdges[i].x0, g_gridEdges[i].z0, flag, g_gridEdges[i].dir0, 0);
            UnknownFunction482dd0(g_gridEdges[i].x1, g_gridEdges[i].z1, flag, g_gridEdges[i].dir1, 0);
        }
    }
}

// 0x00482f00: re-tests a vertex that lost its pins; returns 1 when the
// vertex was dropped.
int DrawableGridNode::UnknownFunction482f00(int x, int z)
{
    int i = g_gridRow17[z] + x;
    GridBaseCell* cell = &block->cells[i];
    int result = 0;
    if (childMask && (data->field_0x130 == terrain->field_0xb4 ||
                      terrain->field_0xb8[i] == terrain->field_0x53c)) {
        if (UnknownFunction481cc0(x, z)) {
            data->field_0x000[i] &= ~0x40;
            if ((data->field_0x000[i] & 0x80) && !(data->field_0x000[i] & 0x10)) {
                data->field_0x000[i] &= ~0x80;
                result = 1;
            }
        } else {
            data->field_0x000[i] |= 0xc0;
        }
    } else if (!(data->field_0x000[i] & 0x40)) {
        if (cell->field_0x1 > g_gridBaseCurveInverse[data->field_0x124]) {
            data->field_0x000[i] |= 0xc0;
            result = 1;
        } else if (!(data->field_0x000[i] & 0x10)) {
            data->field_0x000[i] &= ~0x80;
            result = 1;
        }
    } else if (cell->field_0x1 < g_gridBaseCurveInverse[data->field_0x122]) {
        data->field_0x000[i] &= ~0x40;
        if (!(data->field_0x000[i] & 0x10)) {
            data->field_0x000[i] &= ~0x80;
            result = 1;
        }
    }
    return result;
}

// 0x00483040: forces the sixteen block centres (2 + 4i, 2 + 4j) in (bit 4
// and bit 7) and marks the node.
int DrawableGridNode::UnknownFunction483040()
{
    if (data->b3)
        return 1;
    int i = 2 * 17 + 2;
    for (int bz = 0; bz < 4; bz++, i += 4 * 17 - 16) {
        for (int bx = 0; bx < 4; bx++, i += 4) {
            data->field_0x000[i] |= 0x10;
            if (!(data->field_0x000[i] & 0x80)) {
                data->field_0x000[i] |= 0x80;
                UnknownFunction482c90(g_gridEdges[i].x, g_gridEdges[i].z, 0x80);
            }
        }
    }
    data->b3 = 1;
    data->b1 = 1;
    data->field_0x178 = 0;
    return 1;
}

// 0x00483100: undoes 0x00483040 for the centres nothing else needs.
int DrawableGridNode::UnknownFunction483100()
{
    if (data->b3) {
        int i = 2 * 17 + 2;
        for (int bz = 0; bz < 4; bz++, i += 4 * 17 - 16) {
            for (int bx = 0; bx < 4; bx++, i += 4) {
                if (data->field_0x000[i] & 0x10) {
                    data->field_0x000[i] &= ~0x10;
                    if (!(data->field_0x000[i] & 0x4f)) {
                        int x = g_gridEdges[i].x;
                        int z = g_gridEdges[i].z;
                        if (UnknownFunction482f00(x, z))
                            UnknownFunction482c90(x, z, 0);
                    }
                }
            }
        }
        data->b3 = 0;
        data->b1 = 1;
        for (int k = 0; k < 16; k++) {
            ((GridBlockRange*)data->field_0x17c)[k].end = 0;
            ((GridBlockRange*)data->field_0x17c)[k].indexEnd = 0;
        }
    }
    return 1;
}

// 0x00481a20: frustum test of 4 x 4 block `block` (Z order); records the
// result in bit `block` of the draw data's +0x178 mask.
int DrawableGridNode::UnknownFunction481a20(int block)
{
    float boxExtent[3];
    float boxCenter[3];
    float size = (data->field_0x16c - data->field_0x164) * terrain->gridCellSize * 0.25f;
    float half = size * 0.5f;
    boxExtent[0] = half;
    boxExtent[2] = half;
    boxExtent[1] = ((GridBlockRange*)data->field_0x17c)[block].extentY;
    boxCenter[0] = (block % 4) * size + data->field_0x164 * terrain->gridCellSize + half;
    boxCenter[2] = (block / 4) * size + data->field_0x168 * terrain->gridCellSize + half;
    boxCenter[1] = ((GridBlockRange*)data->field_0x17c)[block].centerY;
    int visible = g_visibilityClipper->TestBox(terrain->field_0xc8c, terrain->field_0xc8c->matrix,
                                               boxCenter, boxExtent, 0, 0, 0);
    if (visible)
        data->field_0x178 |= 1 << block;
    else
        data->field_0x178 &= ~(1 << block);
    return visible;
}

// 0x00481db0: the root call of the visibility walk 0x00481de0 with the
// node's bounding box (+0x44 centre, +0x50 half extents).
int DrawableGridNode::UnknownFunction481db0(void* a0, void* a1)
{
    return UnknownFunction481de0(a0, a1, &center.x, &extent.x, 0, 0, 16, 0, 0, 0);
}

// 0x00481cc0: screen-space error test of vertex (x, z): returns 1 when the
// vertex's curve error, seen from the viewer, is small enough to drop it.
int DrawableGridNode::UnknownFunction481cc0(int x, int z)
{
    GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
    float error = (float)g_gridBaseCurve[cell->field_0x1] * field_0x14 * field_0x10;
    float dx = terrain->field_0x60 - (float)((gridX + x) << shift);
    float dz = terrain->field_0x68 - (float)((gridZ + z) << shift);
    float dy = terrain->field_0x64 -
               ((float)(cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
    float horizontal = dx * dx + dz * dz;
    float distance = dy * dy + horizontal;
    float limit = distance * terrain->field_0x78 * distance;
    terrain->field_0x90++;
    if (horizontal * error * error <= limit)
        return 1;
    return 0;
}
