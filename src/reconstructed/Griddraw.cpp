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
extern int g_gridDrawMemoryPeak;  // 0x0068a2fc, high-water mark of g_gridDrawMemory

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

// 0x0047f840: rebuilds the vertex and index buffers of a node without
// per-block ranges: two diagonal halves of the 16 x 16 cell (the diagonal
// picked by the node's position in its parent), each falling back to one
// big triangle when nothing finer is drawn. Returns 0 when an allocation
// fails.
int DrawableGridNode::UnknownFunction47f840()
{
    int closed;
    int before = data->field_0x12a;
    int n;
    int m;
    terrain->field_0xa4++;
    g_gridOriginX = 0;
    g_gridOriginZ = 0;
    g_gridVertexCache.Reset(this);
    if ((field_0x2a & 0x11) != 0x11 && (field_0x2a & 0x11) != 0) {
        n = 0;
        m = n;
        n = UnknownFunction47fce0(8, n, 0, 0, 8, 8, &closed);
        if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->field_0x08 == 0)
            n = UnknownFunction480700(n, 0, 0, 16, 16);
        m = n;
        n = UnknownFunction47fce0(8, n, 16, 16, -8, -8, &closed);
        if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->field_0x08 == 0)
            n = UnknownFunction480700(n, 16, 16, -16, -16);
    } else {
        n = 0;
        m = n;
        n = UnknownFunction47fce0(8, n, 0, 16, 8, -8, &closed);
        if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->field_0x08 == 0)
            n = UnknownFunction480700(n, 0, 16, 16, -16);
        m = n;
        n = UnknownFunction47fce0(8, n, 16, 0, -8, 8, &closed);
        if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->field_0x08 == 0)
            n = UnknownFunction480700(n, 16, 0, -16, 16);
    }
    data->field_0x12a = n;
    if ((data->field_0x12a == 0 || before == 0) && data->field_0x12a != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if (data->field_0x12a == 0) {
        data->field_0x13c = 0;
        data->field_0x13e = 0;
    } else {
        data->field_0x13c = g_gridVertexCache.count;
        data->field_0x13e = data->field_0x12a * 3;
        int vertexBytes = data->field_0x13c * 32;
        int indexBytes = (data->field_0x13e + data->field_0x13c) * 2;
        if (data->field_0x134 == 0 || vertexBytes > data->field_0x140) {
            g_gridDrawMemory -= data->field_0x140;
            if (data->field_0x134)
                operator delete(data->field_0x134, __FILE__, 1306);
            data->field_0x140 = vertexBytes + 0x80;
            data->field_0x134 = DebugMalloc(data->field_0x140, __FILE__, 1309);
            if (data->field_0x134 == 0) {
                data->field_0x140 = 0;
                data->field_0x13c = 0;
                data->field_0x13e = 0;
                return 0;
            }
            g_gridDrawMemory += data->field_0x140;
            if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                g_gridDrawMemoryPeak = g_gridDrawMemory;
        }
        if (data->field_0x138 == 0 || indexBytes > data->field_0x142) {
            g_gridDrawMemory -= data->field_0x142;
            if (data->field_0x138)
                operator delete(data->field_0x138, __FILE__, 1324);
            data->field_0x142 = indexBytes + 0x20;
            data->field_0x138 = DebugMalloc(data->field_0x142, __FILE__, 1327);
            if (data->field_0x138 == 0) {
                data->field_0x138 = 0;
                data->field_0x142 = 0;
                data->field_0x13c = 0;
                data->field_0x13e = 0;
                return 0;
            }
            g_gridDrawMemory += data->field_0x142;
            if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                g_gridDrawMemoryPeak = g_gridDrawMemory;
        }
        memcpy(data->field_0x134, g_gridVertexCache.vertices, data->field_0x13c * 32);
        memcpy(data->field_0x138, g_gridIndices, data->field_0x13e * 2);
        if (data->ageEntry.size == 0)
            terrain->field_0xc88->UnknownFunction401050(&data->ageEntry, UnknownFunction47ecc0, this, 0,
                                                       data->field_0x142 + data->field_0x140);
        else if (data->ageEntry.size != data->field_0x142 + data->field_0x140)
            data->ageEntry.size = data->field_0x142 + data->field_0x140;
    }
    extra->field_0x28 = 1.0f;
    extra->field_0x2c = 0.0f;
    extra->field_0x30 = 0.0f;
    return 1;
}

// 0x0047fce0: the diagonal counterpart of 0x00480200; *closed reports whether
// vertex (x + dx, z + dz) is in use.
int DrawableGridNode::UnknownFunction47fce0(int level, int n, int x, int z, int dx, int dz, int* closed)
{
    int a;
    int b;
    level--;
    x += dx;
    z += dz;
    if (!(data->field_0x000[g_gridRow17[z] + x] & 0x80)) {
        *closed = 0;
        return n;
    }
    if (level != 0) {
        n = UnknownFunction480200(level, n, x, z, -dx, 0, &a);
        n = UnknownFunction480200(level, n, x, z, 0, -dz, &b);
    } else {
        if (children) {
            *closed = 1;
            return n;
        }
        a = b = 0;
    }
    if (!a) {
        int k = n * 3;
        g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
        g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x - dx, z - dx);
        g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - dx, z + dx);
        n++;
    }
    if (!b) {
        int k = n * 3;
        g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
        g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x + dz, z - dz);
        g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - dz, z - dz);
        n++;
    }
    *closed = 1;
    return n;
}

// 0x0047fe70: the four triangles around a leaf edge vertex; bit 0 of a
// cell's +7 byte picks the diagonal (written `1 - bit` as retail computes it).
void DrawableGridNode::UnknownFunction47fe70(int x, int z, int dx, int dz, int n)
{
    int k = n * 3;
    int i = g_gridRow17[z] + x;
    int f;
    if (dx == 0) {
        if (dz == 1) {
            f = block->cells[i - 18].field_0x7_b0 && (data->field_0x000[i - 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - 1, z - f);
            f = (1 - block->cells[i - 17].field_0x7_b0) && (data->field_0x000[i - 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x + 1, z - f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z - 1);
        } else {
            f = block->cells[i].field_0x7_b0 && (data->field_0x000[i + 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z + 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x + 1, z + f);
            f = (1 - block->cells[i - 1].field_0x7_b0) && (data->field_0x000[i + 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x - 1, z + f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z + 1);
        }
    } else {
        if (dx == 1) {
            f = block->cells[i - 18].field_0x7_b0 && (data->field_0x000[i - 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - 1, z - f);
            f = (1 - block->cells[i - 1].field_0x7_b0) && (data->field_0x000[i + 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x - 1, z + f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z + 1);
        } else {
            f = block->cells[i].field_0x7_b0 && (data->field_0x000[i + 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z + 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x + 1, z + f);
            f = (1 - block->cells[i - 17].field_0x7_b0) && (data->field_0x000[i - 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x + 1, z - f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z - 1);
        }
    }
}

// 0x00480200 helper (inlined): a neighbouring child closes the shared edge
// unless it exists, draws no triangles of its own and is not empty.
static inline int Blocked(DrawableGridNode* c)
{
    return !(c && c->data->field_0x12a + (c->data->b0 ? 0 : c->data->field_0x12c) == 0 && c->childMask);
}

// 0x00480200: the triangles on one edge of a cell, split at the edge midpoint
// while subdivision levels remain (0x0047fce0 handles the diagonal halves).
// Leaf nodes without children emit the full fan through 0x0047fe70.
int DrawableGridNode::UnknownFunction480200(int level, int n, int x, int z, int dx, int dz, int* closed)
{
    int a;
    int b;
    level--;
    if (dx == 0)
        z += dz;
    else
        x += dx;
    if (!(data->field_0x000[g_gridRow17[z] + x] & 0x80)) {
        *closed = 0;
        return n;
    }
    if (level != 0) {
        if (dx == 0) {
            int h = dz < 0 ? -dz >> 1 : -(dz >> 1);
            n = UnknownFunction47fce0(level, n, x, z, h, h, &a);
            n = UnknownFunction47fce0(level, n, x, z, -h, h, &b);
        } else {
            int h = dx < 0 ? -dx >> 1 : -(dx >> 1);
            n = UnknownFunction47fce0(level, n, x, z, h, h, &a);
            n = UnknownFunction47fce0(level, n, x, z, h, -h, &b);
        }
    } else if (children != 0) {
        if (data->b0) {
            a = b = 0;
        } else {
            if (dx == 0) {
                if (dz > 0) {
                    a = Blocked((DrawableGridNode*)children[g_gridRow16[z - 1] + x - 1]);
                    b = Blocked((DrawableGridNode*)children[g_gridRow16[z - 1] + x]);
                } else {
                    a = Blocked((DrawableGridNode*)children[g_gridRow16[z] + x]);
                    b = Blocked((DrawableGridNode*)children[g_gridRow16[z] + x - 1]);
                }
            } else if (dx > 0) {
                a = Blocked((DrawableGridNode*)children[g_gridRow16[z - 1] + x - 1]);
                b = Blocked((DrawableGridNode*)children[g_gridRow16[z] + x - 1]);
            } else {
                a = Blocked((DrawableGridNode*)children[g_gridRow16[z] + x]);
                b = Blocked((DrawableGridNode*)children[g_gridRow16[z - 1] + x]);
            }
        }
    } else {
        UnknownFunction47fe70(x, z, dx, dz, n);
        *closed = 1;
        return n + 2;
    }
    if (!a) {
        int k = n * 3;
        if (dx == 0) {
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - dz);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - dz, z);
        } else {
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - dx);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - dx, z);
        }
        n++;
    }
    if (!b) {
        int k = n * 3;
        if (dx == 0) {
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x + dz, z);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x, z - dz);
        } else {
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x - dx, z);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x, z + dx);
        }
        n++;
    }
    *closed = 1;
    return n;
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

// 0x00480c90: submits a node's or one block's vertex range on the unlit
// path. With lighting off (Terrain+0xc3c) the vertices get flat colours (white
// close up, the terrain's unlit colours further away); with it on and `a5`
// set, close-up ranges get texture coordinates from the packed offsets.
void DrawableGridNode::UnknownFunction480c90(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                                            int indexCount, int block)
{
    float detail;
    if (block == -1)
        detail = extra->field_0x34;
    else
        detail = extra->field_0x38[block].field_0x28;
    if (g_gridGameSettings->field_0x2d0) {
        if (terrain->field_0xca8 < detail)
            terrain->field_0x18->UnknownVirtualSlot8(4, 1, 0);
        else
            terrain->field_0x18->UnknownVirtualSlot8(4, 0, 0);
    }
    if (terrain->field_0xc3c) {
        if (a5) {
            if (terrain->field_0xca4 < detail) {
                float scale = g_gridDrawNormalScale;
                if (level)
                    scale = g_gridDrawNormalScale * 16.0f;
                for (int i = 0; i < vertexCount; i++) {
                    unsigned int packed = vertices[i].packed;
                    float* uv = &vertices[i].field_0x10;
                    uv[0] = (packed >> 16) * scale;
                    uv[1] = (packed & 0xffff) * scale;
                }
                if (!terrain->field_0x18->UnknownVirtualSlot15(4, 0x222, vertices, vertexCount, indices, indexCount, 0))
                    return;
            }
        } else {
            terrain->field_0x2c.UnknownFunction484f10(vertices, vertexCount);
            if (block >= 0)
                UnknownVirtualSlot7(block);
            else
                UnknownVirtualSlot6();
            if (terrain->field_0x38 && extra->field_0x08 && (extra->field_0x08->field_0x68 & 1)) {
                if (block >= 0) {
                    GridBlockRecord* r = &extra->field_0x38[block];
                    extra->field_0x08->UnknownFunction510910(&r->field_0x1c, &r->field_0x20, &r->field_0x24,
                                                             &vertices->u, &vertices->v, vertexCount, 0x20);
                } else {
                    extra->field_0x08->UnknownFunction510910(&extra->field_0x28, &extra->field_0x2c,
                                                             &extra->field_0x30, &vertices->u, &vertices->v,
                                                             vertexCount, 0x20);
                }
            }
            if (terrain->field_0xca4 < detail) {
                for (int i = 0; i < vertexCount; i++) {
                    vertices[i].color = -1;
                    vertices[i].specular = 0;
                }
                if (!terrain->field_0x18->UnknownVirtualSlot15(4, 0x1e2, vertices, vertexCount, indices, indexCount, 0))
                    return;
            } else {
                int color = terrain->unlitColor;
                int specular = terrain->unlitSpecular;
                for (int i = 0; i < vertexCount; i++) {
                    vertices[i].color = color;
                    vertices[i].specular = specular;
                }
                if (!terrain->field_0x18->UnknownVirtualSlot15(4, 0x1e2, vertices, vertexCount, indices, indexCount, 0))
                    return;
            }
        }
    } else {
        terrain->field_0x2c.UnknownFunction484f10(vertices, vertexCount);
        if (block >= 0)
            UnknownVirtualSlot7(block);
        else
            UnknownVirtualSlot6();
        if (terrain->field_0x38 && extra->field_0x08 && (extra->field_0x08->field_0x68 & 1)) {
            if (block >= 0) {
                GridBlockRecord* r = &extra->field_0x38[block];
                extra->field_0x08->UnknownFunction510910(&r->field_0x1c, &r->field_0x20, &r->field_0x24,
                                                         &vertices->u, &vertices->v, vertexCount, 0x20);
            } else {
                extra->field_0x08->UnknownFunction510910(&extra->field_0x28, &extra->field_0x2c, &extra->field_0x30,
                                                         &vertices->u, &vertices->v, vertexCount, 0x20);
            }
        }
        terrain->field_0xc88->UnknownFunction401250(&data->ageEntry);
        if (!terrain->field_0x18->UnknownVirtualSlot15(4, 0x1e2, vertices, vertexCount, indices, indexCount, 0))
            return;
    }
    terrain->field_0x88 += indexCount;
    terrain->field_0x8c += vertexCount;
}

// 0x00480fb0: submits a node's (block == -1) or one block's lit vertex range:
// selects the texture through slot 6 or 7, lets the managed texture update
// the texture coordinates, switches the close-up stage states on or off by
// the detail limit, marks the buffers used and draws.
void DrawableGridNode::UnknownFunction480fb0(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                                            int indexCount, int block)
{
    if (g_gridGameSettings->field_0x2d0 || !a5)
        return;
    float detail;
    if (block == -1)
        detail = extra->field_0x34;
    else
        detail = extra->field_0x38[block].field_0x28;
    if (block >= 0)
        UnknownVirtualSlot7(block);
    else
        UnknownVirtualSlot6();
    if (terrain->field_0x38 && extra->field_0x08 && (extra->field_0x08->field_0x68 & 1)) {
        if (block >= 0) {
            GridBlockRecord* r = &extra->field_0x38[block];
            extra->field_0x08->UnknownFunction510910(&r->field_0x1c, &r->field_0x20, &r->field_0x24, &vertices->u,
                                                     &vertices->v, vertexCount, 0x20);
        } else {
            extra->field_0x08->UnknownFunction510910(&extra->field_0x28, &extra->field_0x2c, &extra->field_0x30,
                                                     &vertices->u, &vertices->v, vertexCount, 0x20);
        }
    }
    if (terrain->field_0xca4 < detail) {
        if (terrain->field_0xcb4 == 0) {
            terrain->field_0x18->UnknownVirtualSlot7(1, 0x11, 2);
            terrain->field_0x18->UnknownVirtualSlot7(1, 0x12, 1);
            terrain->field_0xcb4 = 1;
        }
    } else if (terrain->field_0xcb4 != 0) {
        terrain->field_0x18->UnknownVirtualSlot7(1, 0x11, g_gridGameSettings->field_0x550);
        terrain->field_0x18->UnknownVirtualSlot7(1, 0x12, g_gridGameSettings->field_0x554);
        terrain->field_0xcb4 = 0;
    }
    terrain->field_0xc88->UnknownFunction401250(&data->ageEntry);
    if (terrain->field_0x18->UnknownVirtualSlot15(4, 0x222, vertices, vertexCount, indices, indexCount, 0)) {
        terrain->field_0x88 += indexCount;
        terrain->field_0x8c += vertexCount;
    }
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

// 0x00482760: re-tests the vertices whose curve error lies between the
// node's old and new detail limits (GridBaseBlock keeps the vertices sorted by
// curve value, +0x908 indexes the sorted list): newly coarse ones are
// released, newly fine ones pinned, the rest of the band re-tested.
void DrawableGridNode::UnknownFunction482760()
{
    if (childMask == 0)
        return;
    if (children && !data->b0) {
        for (int i = 0; i < 256; i++) {
            if (children[i]) {
                if (childMask == 0x200000)
                    ((DrawableGridNode*)children[i])->childMask = 0x3fffff;
                ((DrawableGridNode*)children[i])->UnknownFunction482760();
            }
        }
    }
    terrain->field_0x53c++;
    int lo = g_gridBaseCurveInverse[data->field_0x122];
    int cur = g_gridBaseCurveInverse[data->field_0x126];
    int hi = g_gridBaseCurveInverse[data->field_0x124];
    if (hi != 0xff)
        hi++;
    int hiOld = g_gridBaseCurveInverse[data->field_0x128];
    if (hiOld != 0xff)
        hiOld++;
    int k;
    for (k = block->field_0x908[cur]; k < 17 * 17 && k < block->field_0x908[lo]; k++) {
        int i = block->field_0xb08[k];
        data->field_0x000[i] &= ~0x40;
        if (!(data->field_0x000[i] & 0xf)) {
            terrain->field_0xac++;
            if (!(data->field_0x000[i] & 0x10) && (data->field_0x000[i] & 0x80)) {
                data->field_0x000[i] &= ~0x80;
                data->b1 = 1;
                UnknownFunction482c90(g_gridEdges[i].x, g_gridEdges[i].z, 0);
            }
        }
    }
    if (hiOld > hi) {
        for (k = block->field_0x908[hi]; k < 17 * 17 && k <= block->field_0x908[hiOld]; k++) {
            int i = block->field_0xb08[k];
            data->field_0x000[i] |= 0x40;
            if (!(data->field_0x000[i] & 0xf)) {
                terrain->field_0xa8++;
                if (!(data->field_0x000[i] & 0x80)) {
                    data->field_0x000[i] |= 0x80;
                    data->b1 = 1;
                    UnknownFunction482c90(g_gridEdges[i].x, g_gridEdges[i].z, 0x80);
                }
            }
        }
    }
    for (k = block->field_0x908[lo]; k < 17 * 17 && k < block->field_0x908[hi]; k++) {
        int i = block->field_0xb08[k];
        if (data->field_0x000[i] & 0xf)
            terrain->field_0xb8[i] = terrain->field_0x53c;
        else
            UnknownFunction482a40(g_gridEdges[i].x, g_gridEdges[i].z);
    }
    data->field_0x130 = terrain->field_0xb4;
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

// 0x004835c0: a vertex on the border of `child` changed; passes the change to
// the neighbouring children of this node, or for children on this node's own
// border, down from the root node through 0x00483200.
void DrawableGridNode::UnknownFunction4835c0(DrawableGridNode* child, int x, int z, int flag, int dir,
                                            GridBaseCell* cell)
{
    DrawableGridNode* n;
    if ((child->field_0x2a & 0xf) != 0 && (child->field_0x2a & 0xf) != 0xf &&
        (child->field_0x2a & 0xf0) != 0 && (child->field_0x2a & 0xf0) != 0xf0) {
        if (x == 0) {
            n = (DrawableGridNode*)children[child->field_0x2a - 1];
            if (n)
                n->UnknownFunction482dd0(16, z, flag, dir, cell);
            if (z == 0) {
                n = (DrawableGridNode*)children[child->field_0x2a - 17];
                if (n)
                    n->UnknownFunction482dd0(16, 16, flag, dir, cell);
                n = (DrawableGridNode*)children[child->field_0x2a - 16];
                if (n)
                    n->UnknownFunction482dd0(0, 16, flag, dir, cell);
            } else if (z == 16) {
                n = (DrawableGridNode*)children[child->field_0x2a + 15];
                if (n)
                    n->UnknownFunction482dd0(16, 0, flag, dir, cell);
                n = (DrawableGridNode*)children[child->field_0x2a + 16];
                if (n)
                    n->UnknownFunction482dd0(0, 0, flag, dir, cell);
            }
        } else if (x == 16) {
            n = (DrawableGridNode*)children[child->field_0x2a + 1];
            if (n)
                n->UnknownFunction482dd0(0, z, flag, dir, cell);
            if (z == 0) {
                n = (DrawableGridNode*)children[child->field_0x2a - 15];
                if (n)
                    n->UnknownFunction482dd0(0, 16, flag, dir, cell);
                n = (DrawableGridNode*)children[child->field_0x2a - 16];
                if (n)
                    n->UnknownFunction482dd0(16, 16, flag, dir, cell);
            } else if (z == 16) {
                n = (DrawableGridNode*)children[child->field_0x2a + 17];
                if (n)
                    n->UnknownFunction482dd0(0, 0, flag, dir, cell);
                n = (DrawableGridNode*)children[child->field_0x2a + 16];
                if (n)
                    n->UnknownFunction482dd0(16, 0, flag, dir, cell);
            }
        } else if (z == 0) {
            n = (DrawableGridNode*)children[child->field_0x2a - 16];
            if (n)
                n->UnknownFunction482dd0(x, 16, flag, dir, cell);
        } else if (z == 16) {
            n = (DrawableGridNode*)children[child->field_0x2a + 16];
            if (n)
                n->UnknownFunction482dd0(x, 0, flag, dir, cell);
        }
    } else {
        GridNode* root = this;
        if (root) {
            while (root->parent)
                root = root->parent;
            int shift = 0;
            for (int i = 0; i < level - 1; i++)
                shift += 4;
            ((DrawableGridNode*)root)->UnknownFunction483200((child->gridX + x) << shift,
                                                             (child->gridZ + z) << shift, flag, dir, cell,
                                                             child->level);
        }
    }
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
