// Griddraw.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\Griddraw.cpp.
// See Griddraw.h and docs/GRIDDRAW.md.

#include "Griddraw.h"

#include <float.h>
#include <string.h>

#include "DebugAlloc.h"
#include "TextureMap.h"
#include "D3DConstants.h"

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
float FastSqrt(float x);         // 0x00460b50

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
void FindEdgeNeighbours(int x, int z, int h, int v, int maskX, int maskZ, GridEdge* out)
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

// 0x0047e430: one subdivision level of the edge table. Retail loads the row
// offset before x (`mov edx, [ebp]; add edx, esi`), which only a copy of x as
// the inner loop variable reproduces.
void BuildEdgeLevel(int half, int start, int step, int rowStep)
{
    int z = start;
    int row = 0;
    for (; z <= 16; z += rowStep, row++) {
        int x;
        if (start == 0)
            x = (row & 1) ? 0 : half;
        else
            x = half;
        for (int n = x; n <= 16; n += step)
            FindEdgeNeighbours(n, z, half, start, step, rowStep, &g_gridEdges[g_gridRow17[z] + n]);
    }
}

// 0x0047e4d0
static void UnknownFunction47e4d0()
{
    int half = 1;
    int step = 2;
    int rowStep = 1;
    while (half < 16) {
        BuildEdgeLevel(half, 0, step, rowStep);
        rowStep <<= 1;
        BuildEdgeLevel(half, half, step, rowStep);
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
        BuildCurveTables();
        UnknownFunction47e4d0();
        for (int i = 0; i < 17 * 17; i++)
            this->terrain->vertexRetestStamps[i] = 0;
        g_gridEdgesBuilt = 1;
    }
    children = 0;
    childOffsets = 0;
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
int EvictNodeBuffers(void* owner)
{
    GridNodeDrawData* d = ((DrawableGridNode*)owner)->data;
    if (d->vertexBuffer) {
        g_gridDrawMemory -= d->vertexBufferBytes;
        DebugFree(d->vertexBuffer, __FILE__, 816);
        d->vertexBuffer = 0;
        d->vertexCount = 0;
        d->vertexBufferBytes = 0;
    }
    if (d->indexBuffer) {
        g_gridDrawMemory -= d->indexBufferBytes;
        DebugFree(d->indexBuffer, __FILE__, 823);
        d->indexBuffer = 0;
        d->indexCount = 0;
        d->indexBufferBytes = 0;
    }
    d->ageEntry.size = 0;
    d->previousLowerErrorLimit = 0;
    d->lowerErrorLimit = 0;
    d->triangleCount = 0;
    d->previousUpperErrorLimit = 0xffff;
    d->upperErrorLimit = 0xffff;
    d->b0 = 0;
    d->b1 = 1;
    return 1;
}

// 0x0047edb0: releases the vertex buffers of this node (and, when asked, of
// every descendant) and takes its count out of the parent's total.
void DrawableGridNode::ReleaseVertexBuffers(int recurse)
{
    if (parent) {
        ((DrawableGridNode*)parent)->data->childTriangleCount -= data->childTriangleCount + data->triangleCount;
        if (((DrawableGridNode*)parent)->data->childTriangleCount < 0)
            ((DrawableGridNode*)parent)->data->childTriangleCount = 0;
    }
    data->triangleCount = 0;
    data->b1 = 1;
    data->b0 = 0;
    if (recurse && children) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->ReleaseVertexBuffers(recurse);
        }
    }
    if (data->vertexBuffer) {
        g_gridDrawMemory -= data->vertexBufferBytes;
        DebugFree(data->vertexBuffer, __FILE__, 858);
        data->vertexBuffer = 0;
        data->vertexCount = 0;
        data->vertexBufferBytes = 0;
    }
    if (data->indexBuffer) {
        g_gridDrawMemory -= data->indexBufferBytes;
        DebugFree(data->indexBuffer, __FILE__, 865);
        data->indexBuffer = 0;
        data->indexCount = 0;
        data->indexBufferBytes = 0;
    }
    if (data->childTriangleCount != 0)
        data->childTriangleCount = 0;
    data->previousLowerErrorLimit = 0;
    data->lowerErrorLimit = 0;
    data->previousUpperErrorLimit = 0xffff;
    data->upperErrorLimit = 0xffff;
    if (data->ageEntry.size) {
        terrain->ageManager->Unregister(&data->ageEntry);
        data->ageEntry.size = 0;
    }
}

// 0x0047ef70
int DrawableGridNode::WalkVisibleFromRoot()
{
    return WalkVisibleQuadrants(0, 0, 16, 0);
}

// 0x0047ef80: walks the visible quadrants of this node (8-cell, then 4-cell
// blocks) down to the child nodes, rebuilds a dirty node's vertex buffer at
// the top-level call and adds the node's count to its parent's total.
// `size` is halved in place; the 8-cell branch restores 16 so the top-level
// rebuild below still runs (retail jumps straight into it from there).
int DrawableGridNode::WalkVisibleQuadrants(int x, int z, int size, int quad)
{
    if (childMask == 0)
        return 1;
    if (size == 16)
        g_gridDrawFrameCount++;
    if (children != 0 && !data->b0) {
        size >>= 1;
        if (size == 8) {
            data->childTriangleCount = 0;
            if (childMask & 2)
                WalkVisibleQuadrants(x, z, 8, 0);
            if (childMask & 4)
                WalkVisibleQuadrants(x + 8, z, 8, 4);
            if (childMask & 8)
                WalkVisibleQuadrants(x, z + 8, 8, 8);
            if (childMask & 0x10)
                WalkVisibleQuadrants(x + 8, z + 8, 8, 12);
            size = 16;
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                WalkVisibleQuadrants(x, z, 4, quad);
            if (childMask & (0x40 << quad))
                WalkVisibleQuadrants(x + 4, z, 4, quad + 1);
            if (childMask & (0x80 << quad))
                WalkVisibleQuadrants(x, z + 4, 4, quad + 2);
            if (childMask & (0x100 << quad))
                WalkVisibleQuadrants(x + 4, z + 4, 4, quad + 3);
        } else {
            for (int j = 0; j < 64; j += 16) {
                for (int i = 0; i < 4; i++) {
                    DrawableGridNode* child = (DrawableGridNode*)children[g_gridQuadCell[quad] + i + j];
                    if (child && child->childMask)
                        child->WalkVisibleQuadrants(0, 0, 16, 0);
                }
            }
        }
    }
    if (size == 16 && data->b1) {
        int before = data->childTriangleCount + data->triangleCount;
        if (data->b3)
            RebuildBlockBuffers();
        else
            RebuildWholeNodeBuffers();
        int after = data->childTriangleCount + data->triangleCount;
        if ((before >= 2 && after < 2) || (before < 2 && after >= 2)) {
            if (parent)
                ((DrawableGridNode*)parent)->data->b1 = 1;
        }
        data->b1 = 0;
        if (data->b2)
            terrain->vertexSink->LightVertices(terrain->transform, data->vertexCount,
                (char*)data->vertexBuffer + 0xc, data->vertexBuffer, 0x20, 0);
    }
    if (parent)
        ((DrawableGridNode*)parent)->data->childTriangleCount += data->childTriangleCount + data->triangleCount;
    return 1;
}

// 0x0047f840: rebuilds the vertex and index buffers of a node without
// per-block ranges: two diagonal halves of the 16 x 16 cell (the diagonal
// picked by the node's position in its parent), each falling back to one
// big triangle when nothing finer is drawn. Returns 0 when an allocation
// fails.
int DrawableGridNode::RebuildWholeNodeBuffers()
{
    int closed;
    int before = data->triangleCount;
    int n;
    int m;
    terrain->bufferRebuilds++;
    g_gridOriginX = 0;
    g_gridOriginZ = 0;
    g_gridVertexCache.Reset(this);
    if ((childIndex & 0x11) != 0x11 && (childIndex & 0x11) != 0) {
        n = 0;
        m = n;
        n = EmitDiagonalTriangles(8, n, 0, 0, 8, 8, &closed);
        if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->texture == 0)
            n = AppendCornerTriangle(n, 0, 0, 16, 16);
        m = n;
        n = EmitDiagonalTriangles(8, n, 16, 16, -8, -8, &closed);
        if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->texture == 0)
            n = AppendCornerTriangle(n, 16, 16, -16, -16);
    } else {
        n = 0;
        m = n;
        n = EmitDiagonalTriangles(8, n, 0, 16, 8, -8, &closed);
        if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->texture == 0)
            n = AppendCornerTriangle(n, 0, 16, 16, -16);
        m = n;
        n = EmitDiagonalTriangles(8, n, 16, 0, -8, 8, &closed);
        if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
            ((DrawableGridNode*)parent)->extra->texture == 0)
            n = AppendCornerTriangle(n, 16, 0, -16, 16);
    }
    data->triangleCount = n;
    if ((data->triangleCount == 0 || before == 0) && data->triangleCount != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if (data->triangleCount == 0) {
        data->vertexCount = 0;
        data->indexCount = 0;
    } else {
        data->vertexCount = g_gridVertexCache.count;
        data->indexCount = data->triangleCount * 3;
        int vertexBytes = data->vertexCount * 32;
        int indexBytes = (data->indexCount + data->vertexCount) * 2;
        if (data->vertexBuffer == 0 || vertexBytes > data->vertexBufferBytes) {
            g_gridDrawMemory -= data->vertexBufferBytes;
            if (data->vertexBuffer)
                DebugFree(data->vertexBuffer, __FILE__, 1306);
            data->vertexBufferBytes = vertexBytes + 0x80;
            data->vertexBuffer = DebugMalloc(data->vertexBufferBytes, __FILE__, 1309);
            if (data->vertexBuffer == 0) {
                data->vertexBufferBytes = 0;
                data->vertexCount = 0;
                data->indexCount = 0;
                return 0;
            }
            g_gridDrawMemory += data->vertexBufferBytes;
            if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                g_gridDrawMemoryPeak = g_gridDrawMemory;
        }
        if (data->indexBuffer == 0 || indexBytes > data->indexBufferBytes) {
            g_gridDrawMemory -= data->indexBufferBytes;
            if (data->indexBuffer)
                DebugFree(data->indexBuffer, __FILE__, 1324);
            data->indexBufferBytes = indexBytes + 0x20;
            data->indexBuffer = DebugMalloc(data->indexBufferBytes, __FILE__, 1327);
            if (data->indexBuffer == 0) {
                data->indexBuffer = 0;
                data->indexBufferBytes = 0;
                data->vertexCount = 0;
                data->indexCount = 0;
                return 0;
            }
            g_gridDrawMemory += data->indexBufferBytes;
            if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                g_gridDrawMemoryPeak = g_gridDrawMemory;
        }
        memcpy(data->vertexBuffer, g_gridVertexCache.vertices, data->vertexCount * 32);
        memcpy(data->indexBuffer, g_gridIndices, data->indexCount * 2);
        if (data->ageEntry.size == 0)
            terrain->ageManager->Register(&data->ageEntry, EvictNodeBuffers, this, 0,
                                                       data->indexBufferBytes + data->vertexBufferBytes);
        else if (data->ageEntry.size != data->indexBufferBytes + data->vertexBufferBytes)
            data->ageEntry.size = data->indexBufferBytes + data->vertexBufferBytes;
    }
    extra->mappingScale = 1.0f;
    extra->mappingOffsetU = 0.0f;
    extra->mappingOffsetV = 0.0f;
    return 1;
}

// 0x0047fce0: the diagonal counterpart of 0x00480200; *closed reports whether
// vertex (x + dx, z + dz) is in use.
int DrawableGridNode::EmitDiagonalTriangles(int level, int n, int x, int z, int dx, int dz, int* closed)
{
    int a;
    int b;
    level--;
    x += dx;
    z += dz;
    if (!(data->vertexFlags[g_gridRow17[z] + x] & 0x80)) {
        *closed = 0;
        return n;
    }
    if (level != 0) {
        n = EmitEdgeTriangles(level, n, x, z, -dx, 0, &a);
        n = EmitEdgeTriangles(level, n, x, z, 0, -dz, &b);
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
void DrawableGridNode::EmitLeafEdgeFan(int x, int z, int dx, int dz, int n)
{
    int k = n * 3;
    int i = g_gridRow17[z] + x;
    int f;
    if (dx == 0) {
        if (dz == 1) {
            f = block->cells[i - 18].field_0x7_b0 && (data->vertexFlags[i - 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - 1, z - f);
            f = (1 - block->cells[i - 17].field_0x7_b0) && (data->vertexFlags[i - 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x + 1, z - f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z - 1);
        } else {
            f = block->cells[i].field_0x7_b0 && (data->vertexFlags[i + 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z + 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x + 1, z + f);
            f = (1 - block->cells[i - 1].field_0x7_b0) && (data->vertexFlags[i + 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x - 1, z + f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z + 1);
        }
    } else {
        if (dx == 1) {
            f = block->cells[i - 18].field_0x7_b0 && (data->vertexFlags[i - 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z - 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x - 1, z - f);
            f = (1 - block->cells[i - 1].field_0x7_b0) && (data->vertexFlags[i + 16] & 0x80);
            g_gridIndices[k + 3] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 4] = g_gridVertexCache.GetVertex(x - 1, z + f);
            g_gridIndices[k + 5] = g_gridVertexCache.GetVertex(x, z + 1);
        } else {
            f = block->cells[i].field_0x7_b0 && (data->vertexFlags[i + 18] & 0x80);
            g_gridIndices[k] = g_gridVertexCache.GetVertex(x, z);
            g_gridIndices[k + 1] = g_gridVertexCache.GetVertex(x, z + 1);
            g_gridIndices[k + 2] = g_gridVertexCache.GetVertex(x + 1, z + f);
            f = (1 - block->cells[i - 17].field_0x7_b0) && (data->vertexFlags[i - 16] & 0x80);
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
    return !(c && c->data->triangleCount + (c->data->b0 ? 0 : c->data->childTriangleCount) == 0 && c->childMask);
}

// 0x00480200: the triangles on one edge of a cell, split at the edge midpoint
// while subdivision levels remain (0x0047fce0 handles the diagonal halves).
// Leaf nodes without children emit the full fan through 0x0047fe70.
int DrawableGridNode::EmitEdgeTriangles(int level, int n, int x, int z, int dx, int dz, int* closed)
{
    int a;
    int b;
    level--;
    if (dx == 0)
        z += dz;
    else
        x += dx;
    if (!(data->vertexFlags[g_gridRow17[z] + x] & 0x80)) {
        *closed = 0;
        return n;
    }
    if (level != 0) {
        if (dx == 0) {
            int h = dz < 0 ? -dz >> 1 : -(dz >> 1);
            n = EmitDiagonalTriangles(level, n, x, z, h, h, &a);
            n = EmitDiagonalTriangles(level, n, x, z, -h, h, &b);
        } else {
            int h = dx < 0 ? -dx >> 1 : -(dx >> 1);
            n = EmitDiagonalTriangles(level, n, x, z, h, h, &a);
            n = EmitDiagonalTriangles(level, n, x, z, h, -h, &b);
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
        EmitLeafEdgeFan(x, z, dx, dz, n);
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
int DrawableGridNode::AppendCornerTriangle(int n, int x, int z, int dx, int dz)
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

// 0x00480900: draws the whole grid, DrawVisibleQuadrants(target, 0, 0, 16, 0, 0)
int DrawableGridNode::DrawVisible(void* target)
{
    return DrawVisibleQuadrants(target, 0, 0, 16, 0, 0);
}

// 0x00480920: the same walk with the detail-pass argument (Terrain slot 14 passes its detail texture)
int DrawableGridNode::DrawVisibleWithDetail(void* target, int a5)
{
    return DrawVisibleQuadrants(target, 0, 0, 16, 0, a5);
}

// 0x00480940: the same quadrant walk as 0x0047ef80; 4-cell blocks go to
// 0x00480ad0.
int DrawableGridNode::DrawVisibleQuadrants(void* target, int x, int z, int size, int quad, int a5)
{
    if (!data->b0 || data->b3) {
        size >>= 1;
        if (size == 8) {
            if (childMask & 2)
                DrawVisibleQuadrants(target, x, z, 8, 0, a5);
            if (childMask & 4)
                DrawVisibleQuadrants(target, x + 8, z, 8, 4, a5);
            if (childMask & 8)
                DrawVisibleQuadrants(target, x, z + 8, 8, 8, a5);
            if (childMask & 0x10)
                DrawVisibleQuadrants(target, x + 8, z + 8, 8, 12, a5);
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                DrawVisibleBlock(target, x, z, 4, quad, a5);
            if (childMask & (0x40 << quad))
                DrawVisibleBlock(target, x + 4, z, 4, quad + 1, a5);
            if (childMask & (0x80 << quad))
                DrawVisibleBlock(target, x, z + 4, 4, quad + 2, a5);
            if (childMask & (0x100 << quad))
                DrawVisibleBlock(target, x + 4, z + 4, 4, quad + 3, a5);
        }
    }
    return 1;
}

// 0x00480c90: submits a node's or one block's vertex range on the unlit
// path. With a detail texture (Terrain+0xc3c detailTexture) and `a5` clear the
// base pass gets flat colours (white close up, the terrain's unlit colours
// further away); with `a5` set, close-up ranges get detail texture coordinates
// from the packed offsets. Without a detail texture only the base pass runs.
void DrawableGridNode::SubmitUnlitRange(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                                            int indexCount, int block)
{
    float detail;
    if (block == -1)
        detail = extra->detail;
    else
        detail = extra->blockRecords[block].detail;
    if (g_gridGameSettings->softwareRenderer) {
        if (terrain->qualityParamB < detail)
            terrain->renderer->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 1, 0);
        else
            terrain->renderer->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    }
    if (terrain->detailTexture) {
        if (a5) {
            if (terrain->qualityParamA < detail) {
                float scale = g_gridDrawNormalScale;
                if (level)
                    scale = g_gridDrawNormalScale * 16.0f;
                for (int i = 0; i < vertexCount; i++) {
                    unsigned int packed = vertices[i].packed;
                    float* uv = &vertices[i].ny;
                    uv[0] = (packed >> 16) * scale;
                    uv[1] = (packed & 0xffff) * scale;
                }
                if (!terrain->renderer->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, 0x222, vertices, vertexCount, indices, indexCount, 0))
                    return;
            }
        } else {
            terrain->vertexLighter.FogVertices(vertices, vertexCount);
            if (block >= 0)
                UnknownVirtualSlot7(block);
            else
                UnknownVirtualSlot6();
            if (terrain->updateTextureCoordinates && extra->texture && (extra->texture->mapFlags & 1)) {
                if (block >= 0) {
                    GridBlockRecord* r = &extra->blockRecords[block];
                    extra->texture->RemapTexCoordsIfMoved(&r->mappingScale, &r->mappingOffsetU, &r->mappingOffsetV,
                                                             &vertices->u, &vertices->v, vertexCount, 0x20);
                } else {
                    extra->texture->RemapTexCoordsIfMoved(&extra->mappingScale, &extra->mappingOffsetU,
                                                             &extra->mappingOffsetV, &vertices->u, &vertices->v,
                                                             vertexCount, 0x20);
                }
            }
            if (terrain->qualityParamA < detail) {
                for (int i = 0; i < vertexCount; i++) {
                    vertices[i].color = -1;
                    vertices[i].specular = 0;
                }
                if (!terrain->renderer->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertices, vertexCount, indices, indexCount, 0))
                    return;
            } else {
                int color = terrain->unlitColor;
                int specular = terrain->unlitSpecular;
                for (int i = 0; i < vertexCount; i++) {
                    vertices[i].color = color;
                    vertices[i].specular = specular;
                }
                if (!terrain->renderer->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertices, vertexCount, indices, indexCount, 0))
                    return;
            }
        }
    } else {
        terrain->vertexLighter.FogVertices(vertices, vertexCount);
        if (block >= 0)
            UnknownVirtualSlot7(block);
        else
            UnknownVirtualSlot6();
        if (terrain->updateTextureCoordinates && extra->texture && (extra->texture->mapFlags & 1)) {
            if (block >= 0) {
                GridBlockRecord* r = &extra->blockRecords[block];
                extra->texture->RemapTexCoordsIfMoved(&r->mappingScale, &r->mappingOffsetU, &r->mappingOffsetV,
                                                         &vertices->u, &vertices->v, vertexCount, 0x20);
            } else {
                extra->texture->RemapTexCoordsIfMoved(&extra->mappingScale, &extra->mappingOffsetU, &extra->mappingOffsetV,
                                                         &vertices->u, &vertices->v, vertexCount, 0x20);
            }
        }
        terrain->ageManager->MarkUsed(&data->ageEntry);
        if (!terrain->renderer->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertices, vertexCount, indices, indexCount, 0))
            return;
    }
    terrain->indicesDrawn += indexCount;
    terrain->verticesDrawn += vertexCount;
}

// 0x00480fb0: submits a node's (block == -1) or one block's lit vertex range:
// selects the texture through slot 6 or 7, lets the managed texture update
// the texture coordinates, switches the close-up stage states on or off by
// the detail limit, marks the buffers used and draws.
void DrawableGridNode::SubmitLitRange(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                                            int indexCount, int block)
{
    if (g_gridGameSettings->softwareRenderer || !a5)
        return;
    float detail;
    if (block == -1)
        detail = extra->detail;
    else
        detail = extra->blockRecords[block].detail;
    if (block >= 0)
        UnknownVirtualSlot7(block);
    else
        UnknownVirtualSlot6();
    if (terrain->updateTextureCoordinates && extra->texture && (extra->texture->mapFlags & 1)) {
        if (block >= 0) {
            GridBlockRecord* r = &extra->blockRecords[block];
            extra->texture->RemapTexCoordsIfMoved(&r->mappingScale, &r->mappingOffsetU, &r->mappingOffsetV, &vertices->u,
                                                     &vertices->v, vertexCount, 0x20);
        } else {
            extra->texture->RemapTexCoordsIfMoved(&extra->mappingScale, &extra->mappingOffsetU, &extra->mappingOffsetV,
                                                     &vertices->u, &vertices->v, vertexCount, 0x20);
        }
    }
    if (terrain->qualityParamA < detail) {
        if (terrain->closeUpStateSet == 0) {
            terrain->renderer->UnknownVirtualSlot7(1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
            terrain->renderer->UnknownVirtualSlot7(1, D3DTSS_MIPFILTER, D3DTFP_NONE);
            terrain->closeUpStateSet = 1;
        }
    } else if (terrain->closeUpStateSet != 0) {
        terrain->renderer->UnknownVirtualSlot7(1, D3DTSS_MINFILTER, g_gridGameSettings->minFilter);
        terrain->renderer->UnknownVirtualSlot7(1, D3DTSS_MIPFILTER, g_gridGameSettings->mipFilter);
        terrain->closeUpStateSet = 0;
    }
    terrain->ageManager->MarkUsed(&data->ageEntry);
    if (terrain->renderer->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, 0x222, vertices, vertexCount, indices, indexCount, 0)) {
        terrain->indicesDrawn += indexCount;
        terrain->verticesDrawn += vertexCount;
    }
}

// 0x00481170
int DrawableGridNode::WalkSlot4FromRoot()
{
    return WalkSlot4Quadrants(0, 0, 16, 0);
}

// 0x00481180
int DrawableGridNode::WalkSlot4Quadrants(int x, int z, int size, int quad)
{
    if (data->vertexCount != 0 && childMask != 0 && size == 16 && extra->texture != 0 && !data->b3)
        UnknownVirtualSlot4(-1);
    if (!data->b0 || data->b3) {
        size >>= 1;
        if (size == 8) {
            data->visibleBlockMask = 0;
            if (childMask & 2)
                WalkSlot4Quadrants(x, z, 8, 0);
            if (childMask & 4)
                WalkSlot4Quadrants(x + 8, z, 8, 4);
            if (childMask & 8)
                WalkSlot4Quadrants(x, z + 8, 8, 8);
            if (childMask & 0x10)
                WalkSlot4Quadrants(x + 8, z + 8, 8, 12);
        } else if (size == 4) {
            if (childMask & (0x20 << quad))
                WalkSlot4Block(x, z, 4, quad);
            if (childMask & (0x40 << quad))
                WalkSlot4Block(x + 4, z, 4, quad + 1);
            if (childMask & (0x80 << quad))
                WalkSlot4Block(x, z + 4, 4, quad + 2);
            if (childMask & (0x100 << quad))
                WalkSlot4Block(x + 4, z + 4, 4, quad + 3);
        }
    }
    return 1;
}

// 0x004813e0: compares the shared edges of each child with its right and
// lower neighbour (height, curve index and vertex bit 7) and counts the
// differences, which nothing reads. Retail recurses into children[row]
// once per column inside the scan, then into every child.
void DrawableGridNode::CountEdgeDifferences()
{
    int mismatches = 0;
    if (childMask && children && !data->b0) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                DrawableGridNode* node = (DrawableGridNode*)children[g_gridRow16[i] + j];
                DrawableGridNode* right = (DrawableGridNode*)children[g_gridRow16[i] + j + 1];
                DrawableGridNode* below = (DrawableGridNode*)children[g_gridRow16[i + 1] + j];
                if (node) {
                    if (right && node->childMask && right->childMask) {
                        for (int k = 0; k < 16; k++) {
                            int cell = g_gridRow17[k];
                            if (node->block->cells[cell + 16].height != right->block->cells[cell].height ||
                                node->block->cells[cell + 16].errorIndex != right->block->cells[cell].errorIndex ||
                                (node->data->vertexFlags[cell + 16] ^ right->data->vertexFlags[cell]) & 0x80)
                                mismatches++;
                        }
                    }
                    if (below && node->childMask && below->childMask) {
                        for (int k = 0; k < 15; k++) {
                            if (node->block->cells[g_gridRow17[16] + k].height != below->block->cells[k].height ||
                                node->block->cells[g_gridRow17[16] + k].errorIndex != below->block->cells[k].errorIndex ||
                                (node->data->vertexFlags[g_gridRow17[16] + k] ^ below->data->vertexFlags[k]) & 0x80)
                                mismatches++;
                        }
                    }
                }
                if (children[i])
                    ((DrawableGridNode*)children[i])->CountEdgeDifferences();
            }
        }
    }
    if (children) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->CountEdgeDifferences();
        }
    }
}

// 0x00481580
void DrawableGridNode::TestDroppableVertices()
{
    if (childMask && children && !data->b0) {
        for (int i = 0; i < 256; i++) {
            if (children[i])
                ((DrawableGridNode*)children[i])->TestDroppableVertices();
        }
    } else if (childMask) {
        for (int z = 0; z <= 16; z++) {
            for (int x = 0; x <= 16; x++)
                CanDropVertex(x, z);
        }
    }
}

// 0x004824c0: updates the node's level of detail; nodes that stay coarse
// release their children's buffers, finer ones load their child nodes from
// the stream on first use (the same layout 0x0047e600 reads).
void DrawableGridNode::UpdateLevelOfDetail(UnknownTextureStream* stream, void* a1, int recurse, int coarse)
{
    float detail = ComputeViewerDetail(coarse);
    UnknownVirtualSlot3(detail);
    if (maxError <= data->lowerErrorLimit && extra->texture) {
        if (data->b0)
            return;
        data->b0 = 1;
        data->b1 = 1;
        if (children && recurse) {
            for (int i = 0; i < 256; i++) {
                if (children[i])
                    ((DrawableGridNode*)children[i])->ReleaseVertexBuffers(1);
            }
        }
        return;
    }
    if (data->b0) {
        data->b0 = 0;
        data->b1 = 1;
    }
    if (childListOffset == 0)
        return;
    if (children == 0) {
        children = new (__FILE__, 3383) GridNode*[256];
        childOffsets = new (__FILE__, 3384) int[256];
        stream->Seek(terrain->childTableOffset + childListOffset, 0, 1);
        stream->Read(childOffsets, 0x400, 1);
        for (int i = 0; i < 256; i++) {
            if (childOffsets[i]) {
                stream->Seek(terrain->childTableOffset + childOffsets[i], 0, 1);
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
                ((DrawableGridNode*)children[i])->UpdateLevelOfDetail(stream, a1, 1, data->b0 | coarse);
        }
    }
}

// 0x004826d0: propagates bit 21 of the mask (all 22 bits set) to the child
// nodes, rebuilds the edge links and resets the index ranges.
void DrawableGridNode::PropagateFullMask()
{
    if (children) {
        for (int i = 0; i < 256; i++) {
            if (children[i]) {
                if (childMask & 0x200000)
                    ((DrawableGridNode*)children[i])->childMask = 0x3fffff;
                ((DrawableGridNode*)children[i])->PropagateFullMask();
            }
        }
    }
    TestNodeVertices();
    data->field_0x130 = terrain->field_0xb4;
    data->previousLowerErrorLimit = 0;
    data->lowerErrorLimit = 0;
    data->previousUpperErrorLimit = 0xffff;
    data->upperErrorLimit = 0xffff;
}

// 0x00482760: re-tests the vertices whose curve error lies between the
// node's old and new detail limits (GridBaseBlock keeps the vertices sorted by
// curve value, +0x908 indexes the sorted list): newly coarse ones are
// released, newly fine ones pinned, the rest of the band re-tested.
void DrawableGridNode::RetestChangedVertices()
{
    if (childMask == 0)
        return;
    if (children && !data->b0) {
        for (int i = 0; i < 256; i++) {
            if (children[i]) {
                if (childMask == 0x200000)
                    ((DrawableGridNode*)children[i])->childMask = 0x3fffff;
                ((DrawableGridNode*)children[i])->RetestChangedVertices();
            }
        }
    }
    terrain->retestStamp++;
    int lo = g_gridBaseCurveInverse[data->lowerErrorLimit];
    int cur = g_gridBaseCurveInverse[data->previousLowerErrorLimit];
    int hi = g_gridBaseCurveInverse[data->upperErrorLimit];
    if (hi != 0xff)
        hi++;
    int hiOld = g_gridBaseCurveInverse[data->previousUpperErrorLimit];
    if (hiOld != 0xff)
        hiOld++;
    int k;
    for (k = block->firstVertexByError[cur]; k < 17 * 17 && k < block->firstVertexByError[lo]; k++) {
        int i = block->verticesByError[k];
        data->vertexFlags[i] &= ~0x40;
        if (!(data->vertexFlags[i] & 0xf)) {
            terrain->verticesReleased++;
            if (!(data->vertexFlags[i] & 0x10) && (data->vertexFlags[i] & 0x80)) {
                data->vertexFlags[i] &= ~0x80;
                data->b1 = 1;
                PropagateVertexChange(g_gridEdges[i].x, g_gridEdges[i].z, 0);
            }
        }
    }
    if (hiOld > hi) {
        for (k = block->firstVertexByError[hi]; k < 17 * 17 && k <= block->firstVertexByError[hiOld]; k++) {
            int i = block->verticesByError[k];
            data->vertexFlags[i] |= 0x40;
            if (!(data->vertexFlags[i] & 0xf)) {
                terrain->verticesPinned++;
                if (!(data->vertexFlags[i] & 0x80)) {
                    data->vertexFlags[i] |= 0x80;
                    data->b1 = 1;
                    PropagateVertexChange(g_gridEdges[i].x, g_gridEdges[i].z, 0x80);
                }
            }
        }
    }
    for (k = block->firstVertexByError[lo]; k < 17 * 17 && k < block->firstVertexByError[hi]; k++) {
        int i = block->verticesByError[k];
        if (data->vertexFlags[i] & 0xf)
            terrain->vertexRetestStamps[i] = terrain->retestStamp;
        else
            TestVertex(g_gridEdges[i].x, g_gridEdges[i].z);
    }
    data->field_0x130 = terrain->field_0xb4;
}

// 0x00482a40: re-tests one vertex and flags it (bit 6: failed the test,
// bit 7: in use) and its neighbours when its state changes.
void DrawableGridNode::TestVertex(int x, int z)
{
    terrain->vertexTests++;
    int i = g_gridRow17[z] + x;
    if (!CanDropVertex(x, z)) {
        data->vertexFlags[i] |= 0x40;
        if (data->vertexFlags[i] & 0x80)
            return;
        data->vertexFlags[i] |= 0x80;
    } else {
        data->vertexFlags[i] &= ~0x40;
        if (data->vertexFlags[i] & 0x10)
            return;
        if (!(data->vertexFlags[i] & 0x80))
            return;
        data->vertexFlags[i] &= ~0x80;
    }
    data->b1 = 1;
    PropagateVertexChange(x, z, data->vertexFlags[i] & 0x80);
}

// 0x00482ae0: the same subdivision sequence as 0x0047e4d0 over the whole
// node, then one pass with step 16.
void DrawableGridNode::TestNodeVertices()
{
    int half = 1;
    int step = 2;
    int rowStep = 1;
    while (half < 16) {
        TestRectangleLevel(0, 0, 16, 16, half, 0, step, rowStep);
        rowStep <<= 1;
        TestRectangleLevel(0, 0, 16, 16, half, half, step, rowStep);
        half <<= 1;
        step <<= 1;
    }
    TestRectangleLevel(0, 0, 16, 16, 0, 0, 16, 16);
}

// 0x00482b40: 0x00482a40's test over one subdivision level of a
// rectangle, skipping vertices pinned by a direction bit. Retail loads the
// row offset before x, which the row pointer and the inner loop's own copy
// of x reproduce.
void DrawableGridNode::TestRectangleLevel(int x0, int z0, int w, int h, int half, int start,
                                             int step, int rowStep)
{
    int row = 0;
    int z = z0 + start;
    int zEnd = z0 + h;
    for (; z <= zEnd; z += rowStep, row++) {
        int x;
        if (start == 0)
            x = (row & 1) ? 0 : half;
        else
            x = half;
        int* r = &g_gridRow17[z];
        for (int n = x0 + x; n <= x0 + w; n += step) {
            int i = *r + n;
            if (data->vertexFlags[i] & 0xf)
                continue;
            if (!CanDropVertex(n, z)) {
                data->vertexFlags[i] |= 0x40;
                if (data->vertexFlags[i] & 0x80)
                    continue;
                data->vertexFlags[i] |= 0x80;
            } else {
                data->vertexFlags[i] &= ~0x40;
                if (!(data->vertexFlags[i] & 0x80))
                    continue;
                if (data->vertexFlags[i] & 0x10)
                    continue;
                data->vertexFlags[i] &= ~0x80;
            }
            data->b1 = 1;
            PropagateVertexChange(n, z, data->vertexFlags[i] & 0x80);
        }
    }
}

// 0x00482c90: passes a vertex change on to the two vertices it depends on;
// the centre vertex depends on two corners of the parent node, picked by
// the node's position in the parent (+0x2a: x in the low nibble, z in the
// high nibble).
void DrawableGridNode::PropagateVertexChange(int x, int z, int flag)
{
    terrain->vertexChangePropagations++;
    int i = g_gridRow17[z] + x;
    if (x == 8 && z == x) {
        if (parent) {
            if ((childIndex & 0x11) != 0x11 && (childIndex & 0x11) != 0) {
                ((DrawableGridNode*)parent)->SetVertexDirection(childIndex & 0xf, childIndex >> 4, flag, 8, 0);
                ((DrawableGridNode*)parent)->SetVertexDirection((childIndex & 0xf) + 1, (childIndex >> 4) + 1, flag, 1, 0);
            } else {
                ((DrawableGridNode*)parent)->SetVertexDirection(childIndex & 0xf, (childIndex >> 4) + 1, flag, 2, 0);
                ((DrawableGridNode*)parent)->SetVertexDirection((childIndex & 0xf) + 1, childIndex >> 4, flag, 4, 0);
            }
        }
    } else {
        if (g_gridEdges[i].x0 != g_gridEdges[i].x1 || g_gridEdges[i].z0 != g_gridEdges[i].z1) {
            SetVertexDirection(g_gridEdges[i].x0, g_gridEdges[i].z0, flag, g_gridEdges[i].dir0, 0);
            SetVertexDirection(g_gridEdges[i].x1, g_gridEdges[i].z1, flag, g_gridEdges[i].dir1, 0);
        }
    }
}

// 0x00482dd0: sets or clears direction bit `dir` of a vertex, re-tests it
// and forwards border vertices to the parent node. Both paths set one
// `notify` flag; VC6 then threads it into a single notify block and places
// the `!changed` return after it, as retail does (gotos or two notify blocks
// place the return check inline).
void DrawableGridNode::SetVertexDirection(int x, int z, int flag, int dir, GridBaseCell* origin)
{
    if (x < 0 || z < 0 || x > 16 || z > 16)
        return;
    terrain->vertexDirectionSets++;
    int changed = 0;
    int i = g_gridRow17[z] + x;
    GridBaseCell* cell = &block->cells[i];
    int notify = 0;
    if (flag) {
        if (!(data->vertexFlags[i] & (unsigned char)dir)) {
            data->vertexFlags[i] |= (unsigned char)dir;
            changed = 1;
        }
        if (!(data->vertexFlags[i] & 0x80)) {
            data->vertexFlags[i] |= 0x80;
            notify = 1;
        }
    } else {
        data->vertexFlags[i] &= ~(unsigned char)dir;
        if (!(data->vertexFlags[i] & 0xf) && RetestUnpinnedVertex(x, z))
            notify = 1;
    }
    if (notify) {
        data->b1 = 1;
        PropagateVertexChange(x, z, data->vertexFlags[i] & 0x80);
    } else if (flag && !changed)
        return;
    if (parent && !origin && (x == 0 || x == 16 || z == 0 || z == 16))
        ((DrawableGridNode*)parent)->PropagateBorderChange(this, x, z, flag, dir, cell);
}

// 0x00482f00: re-tests a vertex that lost its pins; returns 1 when the
// vertex was dropped.
int DrawableGridNode::RetestUnpinnedVertex(int x, int z)
{
    int i = g_gridRow17[z] + x;
    GridBaseCell* cell = &block->cells[i];
    int result = 0;
    if (childMask && (data->field_0x130 == terrain->field_0xb4 ||
                      terrain->vertexRetestStamps[i] == terrain->retestStamp)) {
        if (CanDropVertex(x, z)) {
            data->vertexFlags[i] &= ~0x40;
            if ((data->vertexFlags[i] & 0x80) && !(data->vertexFlags[i] & 0x10)) {
                data->vertexFlags[i] &= ~0x80;
                result = 1;
            }
        } else {
            data->vertexFlags[i] |= 0xc0;
        }
    } else if (!(data->vertexFlags[i] & 0x40)) {
        if (cell->errorIndex > g_gridBaseCurveInverse[data->upperErrorLimit]) {
            data->vertexFlags[i] |= 0xc0;
            result = 1;
        } else if (!(data->vertexFlags[i] & 0x10)) {
            data->vertexFlags[i] &= ~0x80;
            result = 1;
        }
    } else if (cell->errorIndex < g_gridBaseCurveInverse[data->lowerErrorLimit]) {
        data->vertexFlags[i] &= ~0x40;
        if (!(data->vertexFlags[i] & 0x10)) {
            data->vertexFlags[i] &= ~0x80;
            result = 1;
        }
    }
    return result;
}

// 0x00483040: forces the sixteen block centres (2 + 4i, 2 + 4j) in (bit 4
// and bit 7) and marks the node.
int DrawableGridNode::ForceBlockCentres()
{
    if (data->b3)
        return 1;
    int i = 2 * 17 + 2;
    for (int bz = 0; bz < 4; bz++, i += 4 * 17 - 16) {
        for (int bx = 0; bx < 4; bx++, i += 4) {
            data->vertexFlags[i] |= 0x10;
            if (!(data->vertexFlags[i] & 0x80)) {
                data->vertexFlags[i] |= 0x80;
                PropagateVertexChange(g_gridEdges[i].x, g_gridEdges[i].z, 0x80);
            }
        }
    }
    data->b3 = 1;
    data->b1 = 1;
    data->visibleBlockMask = 0;
    return 1;
}

// 0x00483100: undoes 0x00483040 for the centres nothing else needs.
int DrawableGridNode::ReleaseBlockCentres()
{
    if (data->b3) {
        int i = 2 * 17 + 2;
        for (int bz = 0; bz < 4; bz++, i += 4 * 17 - 16) {
            for (int bx = 0; bx < 4; bx++, i += 4) {
                if (data->vertexFlags[i] & 0x10) {
                    data->vertexFlags[i] &= ~0x10;
                    if (!(data->vertexFlags[i] & 0x4f)) {
                        int x = g_gridEdges[i].x;
                        int z = g_gridEdges[i].z;
                        if (RetestUnpinnedVertex(x, z))
                            PropagateVertexChange(x, z, 0);
                    }
                }
            }
        }
        data->b3 = 0;
        data->b1 = 1;
        for (int k = 0; k < 16; k++) {
            ((GridBlockRange*)data->blockRanges)[k].end = 0;
            ((GridBlockRange*)data->blockRanges)[k].indexEnd = 0;
        }
    }
    return 1;
}

// 0x004835c0: a vertex on the border of `child` changed; passes the change to
// the neighbouring children of this node, or for children on this node's own
// border, down from the root node through 0x00483200.
void DrawableGridNode::PropagateBorderChange(DrawableGridNode* child, int x, int z, int flag, int dir,
                                            GridBaseCell* cell)
{
    DrawableGridNode* n;
    if ((child->childIndex & 0xf) != 0 && (child->childIndex & 0xf) != 0xf &&
        (child->childIndex & 0xf0) != 0 && (child->childIndex & 0xf0) != 0xf0) {
        if (x == 0) {
            n = (DrawableGridNode*)children[child->childIndex - 1];
            if (n)
                n->SetVertexDirection(16, z, flag, dir, cell);
            if (z == 0) {
                n = (DrawableGridNode*)children[child->childIndex - 17];
                if (n)
                    n->SetVertexDirection(16, 16, flag, dir, cell);
                n = (DrawableGridNode*)children[child->childIndex - 16];
                if (n)
                    n->SetVertexDirection(0, 16, flag, dir, cell);
            } else if (z == 16) {
                n = (DrawableGridNode*)children[child->childIndex + 15];
                if (n)
                    n->SetVertexDirection(16, 0, flag, dir, cell);
                n = (DrawableGridNode*)children[child->childIndex + 16];
                if (n)
                    n->SetVertexDirection(0, 0, flag, dir, cell);
            }
        } else if (x == 16) {
            n = (DrawableGridNode*)children[child->childIndex + 1];
            if (n)
                n->SetVertexDirection(0, z, flag, dir, cell);
            if (z == 0) {
                n = (DrawableGridNode*)children[child->childIndex - 15];
                if (n)
                    n->SetVertexDirection(0, 16, flag, dir, cell);
                n = (DrawableGridNode*)children[child->childIndex - 16];
                if (n)
                    n->SetVertexDirection(16, 16, flag, dir, cell);
            } else if (z == 16) {
                n = (DrawableGridNode*)children[child->childIndex + 17];
                if (n)
                    n->SetVertexDirection(0, 0, flag, dir, cell);
                n = (DrawableGridNode*)children[child->childIndex + 16];
                if (n)
                    n->SetVertexDirection(16, 0, flag, dir, cell);
            }
        } else if (z == 0) {
            n = (DrawableGridNode*)children[child->childIndex - 16];
            if (n)
                n->SetVertexDirection(x, 16, flag, dir, cell);
        } else if (z == 16) {
            n = (DrawableGridNode*)children[child->childIndex + 16];
            if (n)
                n->SetVertexDirection(x, 0, flag, dir, cell);
        }
    } else {
        GridNode* root = this;
        if (root) {
            while (root->parent)
                root = root->parent;
            int shift = 0;
            for (int i = 0; i < level - 1; i++)
                shift += 4;
            ((DrawableGridNode*)root)->WalkBorderVertex((child->gridX + x) << shift,
                                                        (child->gridZ + z) << shift, flag, dir, cell,
                                                        child->level);
        }
    }
}

// 0x00481a20: frustum test of 4 x 4 block `block` (Z order); records the
// result in bit `block` of the draw data's +0x178 mask.
int DrawableGridNode::TestBlockFrustum(int block)
{
    float boxExtent[3];
    float boxCenter[3];
    float size = (data->gridMaxX - data->gridMinX) * terrain->gridCellSize * 0.25f;
    float half = size * 0.5f;
    boxExtent[0] = half;
    boxExtent[2] = half;
    boxExtent[1] = ((GridBlockRange*)data->blockRanges)[block].extentY;
    boxCenter[0] = (block % 4) * size + data->gridMinX * terrain->gridCellSize + half;
    boxCenter[2] = (block / 4) * size + data->gridMinZ * terrain->gridCellSize + half;
    boxCenter[1] = ((GridBlockRange*)data->blockRanges)[block].centerY;
    int visible = g_visibilityClipper->TestBox(terrain->camera, terrain->camera->matrix,
                                               boxCenter, boxExtent, 0, 0, 0);
    if (visible)
        data->visibleBlockMask |= 1 << block;
    else
        data->visibleBlockMask &= ~(1 << block);
    return visible;
}

// 0x00481b30: distance from the viewer to a 4 x 4 block's box.
float DrawableGridNode::BlockDistance(int block)
{
    float ex = terrain->viewerX;
    float ey = terrain->viewerY;
    float ez = terrain->viewerZ;
    float size = (data->gridMaxX - data->gridMinX) * 0.25f;
    float minX = (block % 4) * size + data->gridMinX;
    float maxX = minX + size;
    float minZ = (block / 4) * size + data->gridMinZ;
    float maxZ = minZ + size;
    // Squared distances from the viewer to the block's x and z ranges; each
    // bound is replaced by the viewer's offset from it. (With separate delta
    // locals VC6 folds the single-use maxX into minX + size; retail stores it.)
    float distX;
    if ((minX = ex - minX) < 0.0f)
        distX = minX * minX;
    else if ((maxX = ex - maxX) > 0.0f)
        distX = maxX * maxX;
    else
        distX = 0.0f;
    float distZ;
    if ((minZ = ez - minZ) < 0.0f)
        distZ = minZ * minZ;
    else if ((maxZ = ez - maxZ) > 0.0f)
        distZ = maxZ * maxZ;
    else
        distZ = 0.0f;
    float dist = distX + distZ;
    float distY;
    float below = ey - minHeight;
    float above = ey - maxHeight;
    if (below < 0.0f)
        distY = below * below;
    else if (above > 0.0f)
        distY = above * above;
    else
        distY = 0.0f;
    return FastSqrt(distY + dist);
}

// 0x00481db0: the root call of the visibility walk 0x00481de0 with the
// node's bounding box (+0x44 centre, +0x50 half extents).
int DrawableGridNode::TestVisibility(void* a0, void* a1)
{
    return UnknownFunction481de0(a0, a1, &center.x, &extent.x, 0, 0, 16, 0, 0, 0);
}

// 0x00481cc0: screen-space error test of vertex (x, z): returns 1 when the
// vertex's curve error, seen from the viewer, is small enough to drop it.
int DrawableGridNode::CanDropVertex(int x, int z)
{
    GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
    float error = (float)g_gridBaseCurve[cell->errorIndex] * heightScale * reliefScale;
    float dx = terrain->viewerX - (float)((gridX + x) << shift);
    float dz = terrain->viewerZ - (float)((gridZ + z) << shift);
    float dy = terrain->viewerY -
               ((float)(cell->height - block->baseHeight) * reliefScale + block->baseHeight) * heightScale;
    float horizontal = dx * dx + dz * dz;
    float distance = dy * dy + horizontal;
    float limit = distance * terrain->drawDistanceRatioSquared * distance;
    terrain->vertexErrorTests++;
    if (horizontal * error * error <= limit)
        return 1;
    return 0;
}
