// Near-miss Griddraw.cpp candidates, kept out of src/reconstructed until they
// match. See docs/GRIDDRAW.md. They use the headers of
// src/reconstructed/Griddraw.cpp, and src/reconstructed/Griddraw.bindings.json
// already binds every symbol they reference. Scores are matching bytes
// under vc6_o2_mt with relocations resolved (candidate size in brackets).
//
// GridVertexCache::GetVertex (0x0047de90, 1194 bytes; 1069 of 1187): control
// flow, the x87 sequences and all stores match. VC6 keeps x in esi, z in ebp
// and the slot index in edi; retail keeps x in ebp, z in edi and the index
// in esi, and computes the packed (dx << 16) + dz word as
// ((gx * 0xffff + x) << 16) - gz + z. Index expression forms, a split
// declaration and seven spellings of the packed word leave both unchanged. Also
// unchanged or worse: `* 65536`/`* 0x10000` of the x difference, the shifts
// distributed over x and the origin, unsigned short casts, and `|` instead of
// `+` (the `|` form gives z retail's edi but not the x/index registers, 1084).
//
// DrawableGridNode 0x0047e600 (node loader, 1617 bytes; 1091 of 1630): the
// stream reads, both allocations, the bit fields and the child creation
// match. The 0x540 allocation is reached by a `goto` out of the header scan
// (its __LINE__ 687 precedes the small one's 698), which reproduces retail's
// out-of-line block and keeps the header word in the dead stream-argument
// slot. The 0.1 test reads the extent through a reference (an inline
// helper), which gives retail's `fstp extent.y` and block layout (1605 of
// 1617; a direct test keeps extent.y on the x87 stack, 1091). Remaining: the
// extent.x/extent.y products load the cell size first (`fld cell; fmul
// min`), retail the minimum (`fld min; fmul cell`). Not moved by either
// operand order, the sum order in the centre terms, a `cell` local, inline
// multiply helpers (value-first or cell-first), a GridVec3 constructor for
// the centre, or a reordered 0x16c/0x170 sum; nor by 432 centre/extent forms
// (centre product order and parentheses times extent operand order and
// parentheses), all identical. Probes show the order follows the leaves:
// extent.x with +0x168 gives min-first, extent.z with +0x164 cell-first, and
// moving extent.x after the 0.1 test or using +0x18 keeps cell-first.
// A reference to the cell size or the minimum, a terrain or draw-data
// pointer alias, used in either product and either operand order (216
// forms), keeps cell-first too.
// 0..33 dead initialised int locals at the top keep 1605 at every count.
//
// DrawableGridNode 0x00480ad0 (4-cell block draw, 438 bytes; 395 of 438):
// retail keeps the zero start offset in eax from the prologue and compares
// the range pointer and block index against it (`cmp esi,eax`), with the
// draw data in edi and the ranges in esi; VC6 here uses `test` and swaps
// esi/edi. Five placements of the zero initialisers were tried. Also
// unchanged (395): `indexStart = vertexStart`, `ranges != 0`, `!block`, an
// assignment inside the test, a draw-data local, unsigned/long vertexStart
// and all four block locals declared at the top; initialising indexStart
// with vertexStart at the top (four spellings) gives 355, short types and a
// vertexStart assigned only in the else are far worse.
// Comparing ranges with `(GridBlockRange*)vertexStart`, testing
// `data->field_0x17c` first, `ranges = 0` and an assignment in the test, and
// zero-initialised vertexCount/indexStart/indexCount all keep `test`: VC6
// folds a known zero into the compare (probes never produce `cmp r, zero`).
//
// DrawableGridNode 0x00481300 (4-cell block walk, 216 bytes; 114 of 216):
// retail forms the child index as `lea ecx,[edi+ebp]; add ecx,table[quad]`;
// every order of the three terms gives VC6's `mov edx,table[quad];
// add edx,edi; add edx,ebp`, the form retail itself uses in 0x0047ef80. Also
// tried: a row counter times 16, `(j << 4)`, an `int cell = i + j` local
// (with `+=` of the table entry), unsigned counters, `(&children[j + i])[..]`,
// `(children + table[quad])[i + j]`, a local copy of `children` and an inline
// child-lookup helper taking the cell index; all keep VC6's form. Redundant
// parentheses do not change it either (144 bracketings and orders of the
// three terms, with the 0x0047e430 edge index's 8 forms), nor 0x0047e600's
// extent/centre products (512 + 125 forms).
// Also unchanged: a `GridNode* (*)[16]` row view indexed `[r][...]`,
// `(&children[i])[j + cell]`, `(children + i + j)[cell]` and inline child
// lookups taking (cell, i + j), (i + j, cell), (i, j, cell) or (cell, i, j).
//
// DrawableGridNode 0x0047f210 (per-block buffer rebuild, 1576 bytes; 426 of
// 1586): the calls, reallocations, copies and the bookkeeping after the loops
// match in shape. VC6 here gives x edi and the triangle-count copy esi
// (retail: esi and edi), swaps the frame slots of z and indexTotal and of bx
// and the two strength-reduced block offsets, loads minY before maxY for the
// box centre, re-reads indexTotal for the index byte count and stores a dead
// vertexStart * 32 (10 bytes longer). Tried: all 24 declaration orders of
// x, z, indexTotal and bx (no effect), x and bx as one variable (VC6 merges
// them; retail keeps both, so x steps inside the body), three m/n
// initialisations and the origin taken from x/z.
//
// DrawableGridNode 0x00483200 (border vertex walk, 955 bytes; 84 of 956):
// control flow, the tail call VC6 turns into a loop and every call match;
// VC6 gives x ebp and z ebx (retail: ebx and ebp) and re-reads `level` for
// the shift loop where retail reuses al. Tried the shift and count loop
// forms, rx as a variable or inline, the cell comparison forms, an explicit
// node loop (worse) and x reused as the divided column.
// The `z & mask + unit` argument is retail's precedence (`lea edx, [esi +
// ecx] ; and edx, ebp`). Level reuse: an `int` copy of level reloads it, an
// `unsigned char` copy gives retail's `test al, al` / `and eax, 0xff` pair but
// spills the copy into an argument slot; `level > 0`, `0 < level`, a leaf-first
// early return, and down-counting, while, unsigned char and `(int)level` loop
// forms keep the reload. Register swap: rz as a local (worse), the cx/cz and
// px/pz declaration and division orders, px/pz taken from cx/cz, named
// x >> shift / z >> shift, and 0..33 dead initialised locals change nothing.
//
// DrawableGridNode 0x004815e0 (detail update, 1082 bytes; candidate 1083):
// computes the squared near and far distances of the viewer to the node's
// x and z range, the y delta, the distance (FastSqrt), and the two
// 16-bit detail words from (dy² / q + q) * +0x174 * +0x74 with q the
// nearest of the three distances. Control flow, calls and clamps match
// (the 65535 clamps are ternaries; `0xffff` immediates stay immediates
// once they are). Retail keeps the two axis deltas on the x87 stack across
// the three-way branch (fcom on the first, the second loaded before the
// fnstsw, spilled ad hoc in the arms); VC6 here stores both and reloads
// them, which shifts the frame slots, keeps `push edi` in the prologue
// instead of after the early return and tail-merges the early return.
// Tried: locals, block scopes, const/register, repeated expressions, an
// inline helper with reference, pointer or by-value arguments, and an
// accumulating form (which enregisters only the first delta).
// The two 65535 clamps are written `w >= 65535.0f ? 0xffff : ...`, which gives
// retail's branch polarity (64 of 1083; the `<` form 60). Also tried for the
// deltas: no named deltas (expressions throughout), only one of them named,
// the delta assigned inside the test (`(ax = ...) < 0`, comma forms), copies
// of the bounds overwritten by the deltas (the form that made 0x00481b30
// exact), and the far term written before the near term; none keeps both on
// the x87 stack, and small probes never do either.
//
// DrawableGridNode 0x00481de0 (visibility walk, 1750 bytes; candidate
// 1712): tests the node's box (or the four quadrant boxes, recursively,
// down to the child blocks) against the clipper and updates the child
// mask; `a7` is the recursion level, `a8` the quadrant path and the
// child blocks' masks are set to 0x3fffff (all visible), 0x1fffff (leaf)
// or cleared with 0x00464e90. Structure and calls match; the frame is
// the same size. VC6 here keeps `extent` in ebp and `a1` in ebx where
// retail keeps `size` then `a9` in ebx and `a1` in ebp, and lays out the
// subdivision block after the general tests rather than as the
// fall-through of the `size <= 1` branch, so the body differs throughout.
// Retail's order is: the size > 1 tests, the size <= 1 block falling into
// the subdivision, then the visible/corners tests, the all-visible loops,
// the size == 0 child case, a second `visible` test before the a7 switch
// (`childMask |= 1`, `2 << a8`, `0x20 << a8`; cases 1 and 2 return through
// the `a7 != 0` exit). Writing that switch after the if/else under its own
// `if (visible)`, `if (visible && corners == 0) ... else if (visible)`, and
// the size branches swapped (`size <= 1` first) do not move the subdivision
// block ahead of the tests.

#include <float.h>
#include <string.h>

#include "../../src/reconstructed/Griddraw.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"

// Per-TU copies of the shared row tables (see Griddraw.cpp).
static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

extern float g_gridDrawNormalScale;
extern int g_gridQuadCell[16];
extern int g_gridQuadOrder[16];
extern int g_gridOriginX;
extern int g_gridOriginZ;
extern int g_gridExtraBytes;
extern int g_gridDrawMemory;
extern int g_gridDrawMemoryPeak;
extern unsigned short g_gridIndices[0xd8c / 2];
extern GridVertexCache g_gridVertexCache;
float FastSqrt(float x);
extern int g_gridBoxTests;                      // 0x0068a084, box tests this frame

// 0x0047de90: index of vertex (x, z) of the current node in the vertex
// list, appending it on first use this frame. The first vertex of a node
// latches the node's parameters.
int GridVertexCache::GetVertex(int x, int z)
{
    int i = g_gridRow17[z] + x;
    if (slots[i].stamp < stamp) {
        if (count == 0) {
            originX = node->gridX;
            originZ = node->gridZ;
            shift = node->shift;
            cells = node->block->cells;
            reliefScale = node->reliefScale;
            texOriginV = node->data->texOriginV;
            texOriginU = node->data->texOriginU;
            texStep = node->data->texStep;
            field_0x2d54 = count;
            scale = node->terrain->gridCellSize;
            minY = FLT_MAX;
            maxY = -FLT_MAX;
            lowestQualityOverride = node->terrain->lowestQualityOverride;
            unlitColor = node->terrain->field_0xc9c;
            unlitSpecular = node->terrain->field_0xca0;
            detailTexScale = g_gridDrawNormalScale;
            if (node->level == 1)
                detailTexScale = g_gridDrawNormalScale * 16.0f;
        }
        vertices[count].x = (float)((originX + x) << shift) * scale;
        vertices[count].z = (float)((originZ + z) << shift) * scale;
        int base = node->block->baseHeight;
        vertices[count].y = ((float)(cells[i].height - base) * node->reliefScale + base) * node->heightScale * scale;
        if (vertices[count].y < minY)
            minY = vertices[count].y;
        if (vertices[count].y > maxY)
            maxY = vertices[count].y;
        if (node->data->b2) {
            if (node->reliefScale < 0.0) {
                vertices[count].nx = -cells[i].normal[0] * (1.0f / 127.0f);
                vertices[count].ny = cells[i].normal[1] * (1.0f / 127.0f);
                vertices[count].nz = -cells[i].normal[2] * (1.0f / 127.0f);
            } else {
                vertices[count].nx = cells[i].normal[0] * (1.0f / 127.0f);
                vertices[count].ny = cells[i].normal[1] * (1.0f / 127.0f);
                vertices[count].nz = cells[i].normal[2] * (1.0f / 127.0f);
            }
        } else if (lowestQualityOverride) {
            vertices[count].packed = 0;
            vertices[count].u = ((x - g_gridOriginX) * texStep + texOriginU) * 0.9375f + 0.03125f;
            vertices[count].v = ((z - g_gridOriginZ) * texStep + texOriginV) * 0.9375f + 0.03125f;
            vertices[count].ny = (x - g_gridOriginX) * detailTexScale;
            vertices[count].nz = (z - g_gridOriginZ) * detailTexScale;
        } else {
            vertices[count].ny = unlitColor;
            vertices[count].nz = unlitSpecular;
            vertices[count].u = ((x - g_gridOriginX) * texStep + texOriginU) * 0.9375f + 0.03125f;
            vertices[count].v = ((z - g_gridOriginZ) * texStep + texOriginV) * 0.9375f + 0.03125f;
            vertices[count].packed = ((x - g_gridOriginX) << 16) + (z - g_gridOriginZ);
        }
        slots[i].index = count;
        slots[i].stamp = stamp;
        count++;
    }
    return slots[i].index;
}


// Every component of `v` below `limit`. Read through the reference, the
// components are reloaded after their stores (retail 0x0047e600's `fstp
// extent.y` followed by a reload); a direct test keeps extent.y on the x87
// stack.
static inline int GridExtentBelow(const GridVec3& v, float limit)
{
    return v.x < limit && v.y < limit && v.z < limit;
}

// 0x0047e600: reads the node header from the stream, allocates the draw
// data (with per-block records when any of the 16 header words is not an
// 0xfffe/0xffff marker), computes the bounding box and, when asked, creates
// the child nodes through virtual slot 2.
DrawableGridNode* DrawableGridNode::ReadNode(UnknownTextureStream* stream, int a1,
                                             DrawableGridNode* parent, int level, int shift,
                                             int x, int z, int index, float* boundsMin,
                                             float* boundsMax)
{
    int header;
    int words[16];
    int i;

    stream->Read(&reliefScale, 4, 1);
    stream->Read(&heightScale, 4, 1);
    stream->Read(&minHeight, 4, 1);
    stream->Read(&maxHeight, 4, 1);
    stream->Read(&maxError, 2, 1);
    this->level = level;
    this->shift = shift;
    gridX = x;
    gridZ = z;
    childIndex = index;
    if (boundsMin && boundsMax) {
        float fx = (float)x;
        if (fx < boundsMin[0])
            boundsMin[0] = fx;
        float fz = (float)z;
        if (fz < boundsMin[2])
            boundsMin[2] = fz;
        if (fx > boundsMax[0])
            boundsMax[0] = fx;
        if (fz > boundsMax[2])
            boundsMax[2] = fz;
        if (minHeight < boundsMin[1])
            boundsMin[1] = minHeight;
        if (maxHeight > boundsMax[1])
            boundsMax[1] = maxHeight;
    }
    stream->Read(&blockIndex, 2, 1);
    block = terrain->blocks[blockIndex];
    stream->Read(&header, 4, 1);
    stream->Read(words, 4, 16);
    for (i = 0; i < 16; i++) {
        if ((words[i] & 0xfffe) != 0xfffe) {
            data = (GridNodeDrawData*)DebugCalloc(1, 0x540, __FILE__, 687);
            data->blockRanges = (char*)data + 0x180;
            extra = (GridNodeExtra*)((char*)data->blockRanges + 0xc0);
            extra->blockRecords = (GridBlockRecord*)((char*)extra + 0x40);
            for (int j = 0; j < 16; j++) {
                extra->blockRecords[j].textureReference = words[j];
                extra->blockRecords[j].field_0x04 = 0;
            }
            goto allocated;
        }
    }
    data = (GridNodeDrawData*)DebugCalloc(1, 0x1c0, __FILE__, 698);
    data->blockRanges = 0;
    extra = (GridNodeExtra*)((char*)data + 0x180);
    extra->blockRecords = 0;
allocated:
    extra->textureReference = header;
    extra->field_0x10 = 0;
    data->vertexBuffer = 0;
    data->vertexBufferBytes = 0;
    data->indexBufferBytes = 0;
    data->vertexCount = 0;
    data->indexCount = 0;
    data->indexBuffer = 0;
    data->b1 = 1;
    data->field_0x130 = 0;
    data->lowerErrorLimit = 0xffff;
    data->previousLowerErrorLimit = 0xffff;
    data->upperErrorLimit = 0;
    data->previousUpperErrorLimit = 0;
    data->b0 = 0;
    data->triangleCount = 0;
    data->childTriangleCount = 0;
    data->inverseErrorScale = 1.0f;
    data->b3 = 0;
    data->b2 = terrain->useVertexNormals & 1;
    stream->Read(extra, 4, 1);
    if (extra->byteCount) {
        extra->data = DebugMalloc(extra->byteCount, __FILE__, 732);
        g_gridExtraBytes += extra->byteCount;
        stream->Read(extra->data, extra->byteCount, 1);
    }
    stream->Read(&field_0x2b, 1, 1);
    stream->Read(&childListOffset, 4, 1);
    data->inverseErrorScale = 1.0f / (heightScale * reliefScale);
    if (data->inverseErrorScale < 0.0f)
        data->inverseErrorScale = -data->inverseErrorScale;
    memset(data, 0, sizeof(data->vertexFlags));
    data->visibleBlockMask = 0;
    data->gridMinX = (float)(gridX << this->shift);
    data->gridMinZ = (float)(gridZ << this->shift);
    data->gridMaxX = (float)(16 << this->shift) + data->gridMinX;
    data->gridMaxZ = (float)(16 << this->shift) + data->gridMinZ;
    center.x = (data->gridMaxX + data->gridMinX) * terrain->gridCellSize * 0.5f;
    center.y = (maxHeight + minHeight) * terrain->gridCellSize * 0.5f;
    center.z = (data->gridMaxZ + data->gridMinZ) * terrain->gridCellSize * 0.5f;
    extent.x = center.x - terrain->gridCellSize * data->gridMinX;
    extent.y = center.y - terrain->gridCellSize * minHeight;
    if (GridExtentBelow(extent, 0.1f))
        extent = GridVec3(0.1f, 0.1f, 0.1f);
    extent.z = center.z - data->gridMinZ * terrain->gridCellSize;
    if (parent)
        terrain = parent->terrain;
    UnknownVirtualSlot5(stream);
    if (a1 && childListOffset) {
        childOffsets = new (__FILE__, 776) int[256];
        children = new (__FILE__, 777) GridNode*[256];
        stream->Seek(terrain->childTableOffset + childListOffset, 0, 1);
        stream->Read(childOffsets, 0x400, 1);
        for (i = 0; i < 256; i++) {
            if (childOffsets[i]) {
                stream->Seek(terrain->childTableOffset + childOffsets[i], 0, 1);
                children[i] = UnknownVirtualSlot2(stream, a1, this, this->level - 1, this->shift - 4,
                                                  (gridX + i % 16) * 16, (gridZ + i / 16) * 16, i,
                                                  boundsMin, boundsMax);
            } else {
                children[i] = 0;
            }
        }
    }
    return this;
}


// 0x00480ad0: draws one 4-cell block from this node's buffers when it has
// geometry and passed the frustum test, then walks the child nodes.
int DrawableGridNode::DrawVisibleBlock(void* target, int x, int z, int size, int quad, int a5)
{
    int vertexStart = 0;
    GridBlockRange* ranges = (GridBlockRange*)data->blockRanges;
    if (ranges) {
        int block = g_gridQuadOrder[quad];
        int vertexCount;
        int indexStart;
        int indexCount;
        if (block == 0) {
            vertexCount = ranges[0].end;
            indexCount = ranges[0].indexEnd;
            indexStart = 0;
        } else {
            vertexStart = ranges[block - 1].end;
            indexStart = ranges[block - 1].indexEnd;
            vertexCount = ranges[block].end - vertexStart;
            indexCount = ranges[block].indexEnd - indexStart;
        }
        if (vertexCount != 0 && (data->visibleBlockMask & (1 << block))) {
            if (terrain->lowestQualityOverride)
                SubmitLitRange(a5, (GridVertex*)data->vertexBuffer + vertexStart, vertexCount,
                                      (unsigned short*)data->indexBuffer + indexStart, indexCount,
                                      g_gridQuadOrder[quad]);
            else
                SubmitUnlitRange(a5, (GridVertex*)data->vertexBuffer + vertexStart, vertexCount,
                                      (unsigned short*)data->indexBuffer + indexStart, indexCount,
                                      g_gridQuadOrder[quad]);
        }
    }
    if (children && !data->b0) {
        for (int j = 0; j < 64; j += 16) {
            for (int i = 0; i < 4; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridQuadCell[quad] + i + j];
                if (child && child->childMask) {
                    if (child->data->vertexCount != 0 && !child->data->b3) {
                        if (terrain->lowestQualityOverride)
                            child->SubmitLitRange(a5, (GridVertex*)child->data->vertexBuffer,
                                                         child->data->vertexCount,
                                                         (unsigned short*)child->data->indexBuffer,
                                                         child->data->indexCount, -1);
                        else
                            child->SubmitUnlitRange(a5, (GridVertex*)child->data->vertexBuffer,
                                                         child->data->vertexCount,
                                                         (unsigned short*)child->data->indexBuffer,
                                                         child->data->indexCount, -1);
                    }
                    child->DrawVisibleQuadrants(target, 0, 0, 16, 0, a5);
                }
            }
        }
    }
    return 1;
}


// 0x00481300: one 4-cell block; draws it when it has geometry and
// 0x00481a20 accepts it, then walks the child nodes unless this node's
// buffers stand in for them.
int DrawableGridNode::WalkSlot4Block(int x, int z, int size, int quad)
{
    GridBlockRange* ranges = (GridBlockRange*)data->blockRanges;
    if (ranges) {
        int block = g_gridQuadOrder[quad];
        int count;
        if (block == 0)
            count = ranges[0].end;
        else
            count = ranges[block].end - ranges[block - 1].end;
        if (count != 0 && TestBlockFrustum(block))
            UnknownVirtualSlot4(block);
    }
    if (children && !data->b0) {
        for (int j = 0; j < 64; j += 16) {
            for (int i = 0; i < 4; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[i + j + g_gridQuadCell[quad]];
                if (child && child->childMask) {
                    if (child->data->vertexCount != 0 && !child->data->b3)
                        child->UnknownVirtualSlot4(-1);
                    child->WalkSlot4Quadrants(0, 0, 16, 0);
                }
            }
        }
    }
    return 1;
}


// 0x0047f210: per-block buffer rebuild (see the note at the top).
int DrawableGridNode::RebuildBlockBuffers()
{
    int x;
    int z = 0;
    int indexTotal = 0;
    int before = data->triangleCount;
    terrain->bufferRebuilds++;
    int b = 0;
    int vertexTotal = 0;
    int bz;
    float saved = data->texStep;
    for (bz = 0; bz < 16; bz += 4) {
        x = 0;
        for (int bx = 0; bx < 16; bx += 4) {
            int closed;
            int n;
            int m;
            int size = UnknownVirtualSlot8(b);
            data->texStep = saved;
            if (size == 0x40)
                data->texStep *= 4.0f;
            g_gridVertexCache.Reset(this);
            g_gridOriginX = bx;
            g_gridOriginZ = bz;
            int vertexStart = vertexTotal;
            int indexStart = indexTotal;
            n = 0;
            m = n;
            n = EmitDiagonalTriangles(4, n, x, z + 4, 2, -2, &closed);
            if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                n = AppendCornerTriangle(n, x, z + 4, 4, -4);
            m = n;
            x += 4;
            n = EmitDiagonalTriangles(4, n, x, z, -2, 2, &closed);
            if (n == m && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                n = AppendCornerTriangle(n, x, z, -4, 4);
            vertexTotal += g_gridVertexCache.count;
            indexTotal += n * 3;
            ((GridBlockRange*)data->blockRanges)[b].centerY = (g_gridVertexCache.maxY + g_gridVertexCache.minY) * 0.5f;
            ((GridBlockRange*)data->blockRanges)[b].extentY =
                (g_gridVertexCache.maxY - g_gridVertexCache.minY) * terrain->gridCellSize * 0.5f;
            ((GridBlockRange*)data->blockRanges)[b].end = vertexTotal;
            ((GridBlockRange*)data->blockRanges)[b].indexEnd = indexTotal;
            extra->blockRecords[b].mappingScale = 1.0f;
            extra->blockRecords[b].mappingOffsetU = 0;
            extra->blockRecords[b].mappingOffsetV = 0;
            if (n != 0) {
                int vertexBytes = vertexTotal * 32;
                int indexBytes = (indexTotal + vertexTotal) * 2;
                if (data->vertexBuffer == 0 || vertexBytes > data->vertexBufferBytes) {
                    g_gridDrawMemory -= data->vertexBufferBytes;
                    void* old = data->vertexBuffer;
                    int oldSize = data->vertexBufferBytes;
                    data->vertexBufferBytes = vertexBytes + 0x80;
                    data->vertexBuffer = DebugMalloc(data->vertexBufferBytes, __FILE__, 1077);
                    if (data->vertexBuffer == 0) {
                        data->vertexBufferBytes = 0;
                        data->vertexCount = 0;
                        data->indexCount = 0;
                        return 0;
                    }
                    if (old) {
                        memcpy(data->vertexBuffer, old, oldSize);
                        DebugFree(old, __FILE__, 1085);
                    }
                    g_gridDrawMemory += data->vertexBufferBytes;
                    if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                        g_gridDrawMemoryPeak = g_gridDrawMemory;
                }
                if (data->indexBuffer == 0 || indexBytes > data->indexBufferBytes) {
                    g_gridDrawMemory -= data->indexBufferBytes;
                    void* old = data->indexBuffer;
                    int oldSize = data->indexBufferBytes;
                    data->indexBufferBytes = indexBytes + 0x20;
                    data->indexBuffer = DebugMalloc(data->indexBufferBytes, __FILE__, 1098);
                    if (data->indexBuffer == 0) {
                        data->indexBuffer = 0;
                        data->indexBufferBytes = 0;
                        data->vertexCount = 0;
                        data->indexCount = 0;
                        return 0;
                    }
                    if (old) {
                        memcpy(data->indexBuffer, old, oldSize);
                        DebugFree(old, __FILE__, 1107);
                    }
                    g_gridDrawMemory += data->indexBufferBytes;
                    if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                        g_gridDrawMemoryPeak = g_gridDrawMemory;
                }
                memcpy((GridVertex*)data->vertexBuffer + vertexStart, g_gridVertexCache.vertices,
                       (vertexTotal - vertexStart) * 32);
                memcpy((unsigned short*)data->indexBuffer + indexStart, g_gridIndices,
                       (indexTotal - indexStart) * 2);
            }
            b++;
        }
        z += 4;
    }
    if (data->ageEntry.size == 0)
        terrain->ageManager->Register(&data->ageEntry, EvictNodeBuffers, this, 0,
                                                   data->indexBufferBytes + data->vertexBufferBytes);
    else if (data->ageEntry.size != data->indexBufferBytes + data->vertexBufferBytes)
        data->ageEntry.size = data->indexBufferBytes + data->vertexBufferBytes;
    data->vertexCount = vertexTotal;
    data->indexCount = indexTotal;
    data->triangleCount = indexTotal / 3;
    if ((data->triangleCount == 0 || before == 0) && data->triangleCount != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if ((data->triangleCount == 0 || before == 0) && data->triangleCount != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if (data->triangleCount == 0) {
        data->vertexCount = 0;
        data->indexCount = 0;
    }
    data->texStep = saved;
    return 1;
}


// 0x00483200: walks a border vertex down the node tree (VC6 turns the final
// child call into a loop, as retail does).
void DrawableGridNode::WalkBorderVertex(int x, int z, int flag, int dir, GridBaseCell* origin, int minLevel)
{
    if (level != 0) {
        int shift = 0;
        for (int i = 0; i < level; i++)
            shift += 4;
        int unit = 1 << shift;
        int mask = unit - 1;
        int rx = x & mask;
        if (rx == 0 && (z & mask) == 0) {
            GridBaseCell* cell = &block->cells[g_gridRow17[z >> shift] + (x >> shift)];
            if (cell != origin)
                SetVertexDirection(x >> shift, z >> shift, flag, dir, origin);
        }
        if (children == 0 || level <= minLevel)
            return;
        int cx = x;
        int cz = z;
        int px = x - 1;
        int pz = z - 1;
        int j = level;
        while (j--) {
            cx /= 16;
            cz /= 16;
            px /= 16;
            pz /= 16;
        }
        DrawableGridNode* child;
        if (px == cx) {
            if (cx >= 16)
                return;
            if (pz == cz) {
                if (cz >= 16)
                    return;
                child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                if (child == 0)
                    return;
                child->WalkBorderVertex(rx, z & mask, flag, dir, origin, minLevel);
            } else {
                if (cz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                    if (child)
                        child->WalkBorderVertex(rx, z & mask, flag, dir, origin, minLevel);
                }
                if (pz >= 16)
                    return;
                child = (DrawableGridNode*)children[g_gridRow16[pz] + cx];
                if (child == 0)
                    return;
                child->WalkBorderVertex(rx, (z & mask) + unit, flag, dir, origin, minLevel);
            }
        } else if (pz == cz) {
            if (cx < 16 && cz < 16) {
                child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                if (child)
                    child->WalkBorderVertex(rx, z & mask, flag, dir, origin, minLevel);
            }
            if (px >= 16 || cz >= 16)
                return;
            child = (DrawableGridNode*)children[g_gridRow16[cz] + px];
            if (child == 0)
                return;
            child->WalkBorderVertex(rx + unit, z & mask, flag, dir, origin, minLevel);
        } else {
            if (cx < 16) {
                if (cz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                    if (child)
                        child->WalkBorderVertex(rx, z & mask, flag, dir, origin, minLevel);
                }
                if (pz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[pz] + cx];
                    if (child)
                        child->WalkBorderVertex(rx, z & mask + unit, flag, dir, origin, minLevel);
                }
            }
            if (px >= 16)
                return;
            if (cz < 16) {
                child = (DrawableGridNode*)children[g_gridRow16[cz] + px];
                if (child)
                    child->WalkBorderVertex(rx + unit, z & mask, flag, dir, origin, minLevel);
            }
            if (pz >= 16)
                return;
            child = (DrawableGridNode*)children[g_gridRow16[pz] + px];
            if (child == 0)
                return;
            child->WalkBorderVertex(rx + unit, (z & mask) + unit, flag, dir, origin, minLevel);
        }
        return;
    }
    GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
    if (cell != origin)
        SetVertexDirection(x, z, flag, dir, origin);
}

// 0x004815e0: the detail update (see the note at the top).
float DrawableGridNode::ComputeViewerDetail(int coarse)
{
    float nearX;
    float farX;
    float nearZ;
    float farZ;
    GridTerrain* t = terrain;
    GridNodeDrawData* d = data;
    float ax = t->viewerX - d->gridMinX;
    float bx = t->viewerX - d->gridMaxX;
    if (ax < 0.0f) {
        nearX = ax * ax;
        farX = bx * bx;
    } else if (bx > 0.0f) {
        nearX = bx * bx;
        farX = ax * ax;
    } else {
        nearX = 0.0f;
        float f = ((t->viewerX + t->viewerX < d->gridMinX + d->gridMaxX) ? d->gridMaxX : d->gridMinX) - t->viewerX;
        farX = f * f;
    }
    float az = t->viewerZ - d->gridMinZ;
    float bz = t->viewerZ - d->gridMaxZ;
    if (az < 0.0f) {
        nearZ = az * az;
        farZ = bz * bz;
    } else if (bz > 0.0f) {
        nearZ = bz * bz;
        farZ = az * az;
    } else {
        nearZ = 0.0f;
        float f = ((t->viewerZ + t->viewerZ < d->gridMaxZ + d->gridMinZ) ? d->gridMaxZ : d->gridMinZ) - t->viewerZ;
        farZ = f * f;
    }
    float near2 = nearZ + nearX;
    float far2 = farZ + farX;
    float dyLo = t->viewerY - minHeight;
    float dyHi = t->viewerY - maxHeight;
    float dy;
    float dySq;
    if (dyLo < 0.0f) {
        dy = dyLo;
        dySq = dyLo * dyLo;
    } else if (dyHi > 0.0f) {
        dy = dyHi;
        dySq = dyHi * dyHi;
    } else {
        dySq = 0.0f;
        dy = 0.0f;
    }
    float distance = FastSqrt(dySq + near2);
    if (coarse)
        return distance;
    terrain->detailUpdates++;
    data->previousLowerErrorLimit = data->lowerErrorLimit;
    data->previousUpperErrorLimit = data->upperErrorLimit;
    if (terrain->appliedDrawDistance == 0) {
        data->upperErrorLimit = 0;
    } else if (near2 == 0.0f) {
        data->upperErrorLimit = 0xffff;
    } else if (far2 == 0.0f) {
        data->upperErrorLimit = 0xffff;
    } else {
        float nearDist = FastSqrt(near2);
        float farDist = FastSqrt(far2);
        float q;
        if (dySq < near2)
            q = nearDist;
        else if (dySq > far2)
            q = farDist;
        else
            q = dy;
        float v = (dySq / q + q) * data->inverseErrorScale * terrain->drawDistanceRatio;
        data->lowerErrorLimit = (v >= 65535.0f) ? 0xffff : (unsigned short)(int)v;
        if (dyLo < 0.0f)
            dyLo = -dyLo;
        if (dyHi < 0.0f)
            dyHi = -dyHi;
        float m = (dyLo > dyHi) ? dyLo : dyHi;
        m = m * m;
        float ratioNear = (m + near2) / nearDist;
        float ratioFar = (m + far2) / farDist;
        float w = ((ratioNear > ratioFar) ? ratioNear : ratioFar) * (data->inverseErrorScale * terrain->drawDistanceRatio);
        data->upperErrorLimit = (w >= 65535.0f) ? 0xffff : (unsigned short)(int)w;
        return distance;
    }
    float q;
    if (dySq < near2)
        q = FastSqrt(near2);
    else if (dySq > far2)
        q = FastSqrt(far2);
    else
        q = dy;
    data->lowerErrorLimit = (unsigned short)(int)((dySq / q + q) * data->inverseErrorScale * terrain->drawDistanceRatio);
    if (data->lowerErrorLimit > 0xffff)
        data->lowerErrorLimit = 0xffff;
    return distance;
}

// 0x00481de0: the visibility walk (see the note at the top).
int DrawableGridNode::WalkVisibilityQuadrants(void* a0, void* a1, float* center, float* extent, int a4, int a5,
                                            int size, int a7, int a8, int a9)
{
    int count = 0;
    int changed = 0;
    int oldMask = childMask;
    int visible;
    int corners;
    int i;
    int j;
    if (size > 1) {
        g_gridBoxTests++;
        visible = g_visibilityClipper->TestBox(terrain->renderer->camera, (float*)a1, center, extent, 0, &corners, 0);
        if (corners != 8)
            corners = 0;
        if (size == 16) {
            if (visible) {
                UpdateLevelOfDetail((UnknownTextureStream*)a0, a1, 0, a9);
                if (data->b0) {
                    count = 1;
                    if (childMask == 0) {
                        childMask = 1;
                        changed = 1;
                    }
                } else {
                    childMask = 0;
                }
            } else {
                childMask = 0;
            }
        }
    } else {
        visible = 1;
        corners = 0;
    }
    if (visible) {
        if (corners == 0) {
            size >>= 1;
            if (size) {
                const GridVec3& e = *(const GridVec3*)extent;
                const GridVec3& c = *(const GridVec3*)center;
                GridVec3 childExtent = GridVec3(e.x * 0.5f, e.y, e.z * 0.5f);
                GridVec3 c1 = GridVec3(c.x - childExtent.x, c.y, c.z - childExtent.z);
                GridVec3 c2 = GridVec3(childExtent.x + c.x, c.y, c.z - childExtent.z);
                GridVec3 c3 = GridVec3(c.x - childExtent.x, c.y, childExtent.z + c.z);
                GridVec3 c4 = GridVec3(childExtent.x + c.x, c.y, childExtent.z + c.z);
                count += WalkVisibilityQuadrants(a0, a1, &c1.x, &childExtent.x, a4, a5, size, a7 + 1, a8 * 4, a9);
                count += WalkVisibilityQuadrants(a0, a1, &c2.x, &childExtent.x, a4 + size, a5, size, a7 + 1, a8 * 4 + 1, a9);
                count += WalkVisibilityQuadrants(a0, a1, &c3.x, &childExtent.x, a4, a5 + size, size, a7 + 1, a8 * 4 + 2, a9);
                count += WalkVisibilityQuadrants(a0, a1, &c4.x, &childExtent.x, a4 + size, a5 + size, size, a7 + 1, a8 * 4 + 3, a9);
                if (a7 + 1 == 8 || size == 16) {
                    if (count == 0)
                        childMask = 0;
                    else if (childMask == 0)
                        changed = 1;
                }
            } else {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[a5] + a4];
                if (child) {
                    if (child->children) {
                        count += child->WalkVisibilityQuadrants(a0, a1, &child->center.x, &child->extent.x, 0, 0, 16, 0, 0, a9);
                        child->data->b1 = 1;
                    } else {
                        int dummy;
                        g_gridBoxTests++;
                        if (g_visibilityClipper->TestBox(terrain->renderer->camera, (float*)a1, &child->center.x,
                                                         &child->extent.x, 0, 0, &dummy)) {
                            if (child->childMask != 0x1fffff) {
                                child->childMask = 0x1fffff;
                                changed = 1;
                            }
                            child->UpdateLevelOfDetail((UnknownTextureStream*)a0, a1, 0, a9);
                            count++;
                        } else if (child->childMask) {
                            child->childMask = 0;
                            changed = 1;
                            child->UnknownFunction464e90();
                        }
                    }
                }
            }
        } else {
            for (j = 0; j < size; j++) {
                for (i = 0; i < size; i++) {
                    DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[a5 + j] + i + a4];
                    if (child) {
                        if (child->childMask != 0x3fffff) {
                            child->childMask = 0x3fffff;
                            changed = 1;
                            if (child->children)
                                child->data->b1 = 1;
                        }
                        child->UpdateLevelOfDetail((UnknownTextureStream*)a0, a1, 1, a9);
                        count++;
                    }
                }
            }
            switch (a7) {
            case 0:
                childMask = 0x1fffff;
                break;
            case 1:
                childMask |= 0x1e0 << (a8 * 4);
                break;
            }
            if (size == 16) {
                if (count == 0)
                    childMask = 0;
                else if (childMask == 0)
                    changed = 1;
            }
        }
        switch (a7) {
        case 0:
            childMask |= 1;
            goto finish;
        case 1:
            childMask |= 2 << a8;
            return count;
        case 2:
            childMask |= 0x20 << a8;
            return count;
        }
    } else {
        for (j = 0; j < size; j++) {
            for (i = 0; i < size; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[a5 + j] + i + a4];
                if (child && child->childMask) {
                    child->childMask = 0;
                    changed = 1;
                    child->UnknownFunction464e90();
                }
            }
        }
    }
    if (a7 != 0)
        return count;
finish:
    if (count == 0x100)
        childMask |= 0x200000;
    if (changed || oldMask != childMask) {
        for (DrawableGridNode* node = this; node; node = (DrawableGridNode*)node->parent)
            node->data->b1 = 1;
    }
    if (oldMask != 0 && childMask == 0)
        UnknownFunction464e90();
    return count;
}
