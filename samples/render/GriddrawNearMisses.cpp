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
// declaration and seven spellings of the packed word leave both unchanged.
//
// UnknownFunction47e430 (0x0047e430, 156 bytes; 152 of 156): only the edge
// address differs. Retail loads the row offset first (`mov edx,[ebp];
// add edx,esi`); VC6 here starts from x (`mov edx,esi; add edx,[ebp]`).
// Both operand orders, an index temporary, pointer arithmetic, inline
// helpers, a const or unsigned table and a walking row pointer all give
// VC6's order. 0x00482b40 has the same difference.
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
// the centre, or a reordered 0x16c/0x170 sum.
//
// DrawableGridNode 0x00480ad0 (4-cell block draw, 438 bytes; 395 of 438):
// retail keeps the zero start offset in eax from the prologue and compares
// the range pointer and block index against it (`cmp esi,eax`), with the
// draw data in edi and the ranges in esi; VC6 here uses `test` and swaps
// esi/edi. Five placements of the zero initialisers were tried.
//
// DrawableGridNode 0x00481300 (4-cell block walk, 216 bytes; 114 of 216):
// retail forms the child index as `lea ecx,[edi+ebp]; add ecx,table[quad]`;
// every order of the three terms gives VC6's `mov edx,table[quad];
// add edx,edi; add edx,ebp`, the form retail itself uses in 0x0047ef80.
//
// DrawableGridNode 0x00482b40 (rectangle re-test, 327 bytes; 292 of 327):
// retail computes zEnd before the start row and loads the row offset before
// x (see 0x0047e430).
//
// DrawableGridNode 0x00482dd0 (direction-bit update, 300 bytes; 147 of 333):
// retail shares one copy of the "mark dirty and notify" tail between the
// set and clear paths and places the `!changed` early return just before
// the parent hand-off; VC6 here keeps two copies (they differ in edx/ecx)
// and places the return check inline. An update flag, gotos, an inverted
// 0x80 test and the inverted branch order were tried. With `goto notify`
// from the 0x80 branch into the clear path's notify block every instruction
// matches retail's except that VC6 places the `!changed` return block
// directly after the 0x80 test (retail: after the notify block).
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
// DrawableGridNode 0x00481b30 (block distance, 391 bytes; 139 of 393): retail
// keeps the viewer x and the block size on the x87 stack and copies viewer
// y and z to the frame, then adds the x and z terms before the y test with one
// FastSqrt call per y case. VC6 here keeps a different value on the x87
// stack. Tried four axis-distance spellings, three return forms and all six
// orders of the viewer locals.
//
// DrawableGridNode 0x00483200 (border vertex walk, 955 bytes; 84 of 956):
// control flow, the tail call VC6 turns into a loop and every call match;
// VC6 gives x ebp and z ebx (retail: ebx and ebp) and re-reads `level` for
// the shift loop where retail reuses al. Tried the shift and count loop
// forms, rx as a variable or inline, the cell comparison forms, an explicit
// node loop (worse) and x reused as the divided column.

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
            field_0x2d44 = node->field_0x10;
            field_0x2d48 = node->data->field_0x15c;
            field_0x2d4c = node->data->field_0x158;
            field_0x2d50 = node->data->field_0x160;
            field_0x2d54 = count;
            scale = node->terrain->gridCellSize;
            minY = FLT_MAX;
            maxY = -FLT_MAX;
            field_0x2d64 = node->terrain->lowestQualityOverride;
            field_0x2d68 = node->terrain->field_0xc9c;
            field_0x2d6c = node->terrain->field_0xca0;
            field_0x2d70 = g_gridDrawNormalScale;
            if (node->level == 1)
                field_0x2d70 = g_gridDrawNormalScale * 16.0f;
        }
        vertices[count].x = (float)((originX + x) << shift) * scale;
        vertices[count].z = (float)((originZ + z) << shift) * scale;
        int base = node->block->field_0xd4c;
        vertices[count].y = ((float)(cells[i].height - base) * node->field_0x10 + base) * node->field_0x14 * scale;
        if (vertices[count].y < minY)
            minY = vertices[count].y;
        if (vertices[count].y > maxY)
            maxY = vertices[count].y;
        if (node->data->b2) {
            if (node->field_0x10 < 0.0) {
                vertices[count].nx = -cells[i].normal[0] * (1.0f / 127.0f);
                vertices[count].field_0x10 = cells[i].normal[1] * (1.0f / 127.0f);
                vertices[count].field_0x14 = -cells[i].normal[2] * (1.0f / 127.0f);
            } else {
                vertices[count].nx = cells[i].normal[0] * (1.0f / 127.0f);
                vertices[count].field_0x10 = cells[i].normal[1] * (1.0f / 127.0f);
                vertices[count].field_0x14 = cells[i].normal[2] * (1.0f / 127.0f);
            }
        } else if (field_0x2d64) {
            vertices[count].packed = 0;
            vertices[count].u = ((x - g_gridOriginX) * field_0x2d50 + field_0x2d4c) * 0.9375f + 0.03125f;
            vertices[count].v = ((z - g_gridOriginZ) * field_0x2d50 + field_0x2d48) * 0.9375f + 0.03125f;
            vertices[count].field_0x10 = (x - g_gridOriginX) * field_0x2d70;
            vertices[count].field_0x14 = (z - g_gridOriginZ) * field_0x2d70;
        } else {
            vertices[count].field_0x10 = field_0x2d68;
            vertices[count].field_0x14 = field_0x2d6c;
            vertices[count].u = ((x - g_gridOriginX) * field_0x2d50 + field_0x2d4c) * 0.9375f + 0.03125f;
            vertices[count].v = ((z - g_gridOriginZ) * field_0x2d50 + field_0x2d48) * 0.9375f + 0.03125f;
            vertices[count].packed = ((x - g_gridOriginX) << 16) + (z - g_gridOriginZ);
        }
        slots[i].index = count;
        slots[i].stamp = stamp;
        count++;
    }
    return slots[i].index;
}


// 0x0047e430
void UnknownFunction47e430(int half, int start, int step, int rowStep)
{
    int z = start;
    int row = 0;
    for (; z <= 16; z += rowStep, row++) {
        int x;
        if (start == 0)
            x = (row & 1) ? 0 : half;
        else
            x = half;
        for (; x <= 16; x += step)
            UnknownFunction47e340(x, z, half, start, step, rowStep, &g_gridEdges[g_gridRow17[z] + x]);
    }
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
DrawableGridNode* DrawableGridNode::UnknownFunction47e600(UnknownTextureStream* stream, int a1,
                                                          DrawableGridNode* parent, int level, int shift,
                                                          int x, int z, int index, float* boundsMin,
                                                          float* boundsMax)
{
    int header;
    int words[16];
    int i;

    stream->UnknownFunction461640(&field_0x10, 4, 1);
    stream->UnknownFunction461640(&field_0x14, 4, 1);
    stream->UnknownFunction461640(&field_0x18, 4, 1);
    stream->UnknownFunction461640(&field_0x1c, 4, 1);
    stream->UnknownFunction461640(&field_0x20, 2, 1);
    this->level = level;
    this->shift = shift;
    gridX = x;
    gridZ = z;
    field_0x2a = index;
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
        if (field_0x18 < boundsMin[1])
            boundsMin[1] = field_0x18;
        if (field_0x1c > boundsMax[1])
            boundsMax[1] = field_0x1c;
    }
    stream->UnknownFunction461640(&blockIndex, 2, 1);
    block = terrain->blocks[blockIndex];
    stream->UnknownFunction461640(&header, 4, 1);
    stream->UnknownFunction461640(words, 4, 16);
    for (i = 0; i < 16; i++) {
        if ((words[i] & 0xfffe) != 0xfffe) {
            data = (GridNodeDrawData*)DebugCalloc(1, 0x540, __FILE__, 687);
            data->field_0x17c = (char*)data + 0x180;
            extra = (GridNodeExtra*)((char*)data->field_0x17c + 0xc0);
            extra->blockRecords = (GridBlockRecord*)((char*)extra + 0x40);
            for (int j = 0; j < 16; j++) {
                extra->blockRecords[j].field_0x00 = words[j];
                extra->blockRecords[j].field_0x04 = 0;
            }
            goto allocated;
        }
    }
    data = (GridNodeDrawData*)DebugCalloc(1, 0x1c0, __FILE__, 698);
    data->field_0x17c = 0;
    extra = (GridNodeExtra*)((char*)data + 0x180);
    extra->blockRecords = 0;
allocated:
    extra->field_0x0c = header;
    extra->field_0x10 = 0;
    data->field_0x134 = 0;
    data->field_0x140 = 0;
    data->field_0x142 = 0;
    data->field_0x13c = 0;
    data->field_0x13e = 0;
    data->field_0x138 = 0;
    data->b1 = 1;
    data->field_0x130 = 0;
    data->field_0x122 = 0xffff;
    data->field_0x126 = 0xffff;
    data->field_0x124 = 0;
    data->field_0x128 = 0;
    data->b0 = 0;
    data->field_0x12a = 0;
    data->field_0x12c = 0;
    data->field_0x174 = 1.0f;
    data->b3 = 0;
    data->b2 = terrain->field_0xc34 & 1;
    stream->UnknownFunction461640(extra, 4, 1);
    if (extra->byteCount) {
        extra->data = DebugMalloc(extra->byteCount, __FILE__, 732);
        g_gridExtraBytes += extra->byteCount;
        stream->UnknownFunction461640(extra->data, extra->byteCount, 1);
    }
    stream->UnknownFunction461640(&field_0x2b, 1, 1);
    stream->UnknownFunction461640(&field_0x2c, 4, 1);
    data->field_0x174 = 1.0f / (field_0x14 * field_0x10);
    if (data->field_0x174 < 0.0f)
        data->field_0x174 = -data->field_0x174;
    memset(data, 0, sizeof(data->field_0x000));
    data->field_0x178 = 0;
    data->field_0x164 = (float)(gridX << this->shift);
    data->field_0x168 = (float)(gridZ << this->shift);
    data->field_0x16c = (float)(16 << this->shift) + data->field_0x164;
    data->field_0x170 = (float)(16 << this->shift) + data->field_0x168;
    center.x = (data->field_0x16c + data->field_0x164) * terrain->gridCellSize * 0.5f;
    center.y = (field_0x1c + field_0x18) * terrain->gridCellSize * 0.5f;
    center.z = (data->field_0x170 + data->field_0x168) * terrain->gridCellSize * 0.5f;
    extent.x = center.x - terrain->gridCellSize * data->field_0x164;
    extent.y = center.y - terrain->gridCellSize * field_0x18;
    if (GridExtentBelow(extent, 0.1f))
        extent = GridVec3(0.1f, 0.1f, 0.1f);
    extent.z = center.z - data->field_0x168 * terrain->gridCellSize;
    if (parent)
        terrain = parent->terrain;
    UnknownVirtualSlot5(stream);
    if (a1 && field_0x2c) {
        field_0x30 = new (__FILE__, 776) int[256];
        children = new (__FILE__, 777) GridNode*[256];
        stream->UnknownFunction461340(terrain->field_0xc38 + field_0x2c, 0, 1);
        stream->UnknownFunction461640(field_0x30, 0x400, 1);
        for (i = 0; i < 256; i++) {
            if (field_0x30[i]) {
                stream->UnknownFunction461340(terrain->field_0xc38 + field_0x30[i], 0, 1);
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
int DrawableGridNode::UnknownFunction480ad0(void* target, int x, int z, int size, int quad, int a5)
{
    int vertexStart = 0;
    GridBlockRange* ranges = (GridBlockRange*)data->field_0x17c;
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
        if (vertexCount != 0 && (data->field_0x178 & (1 << block))) {
            if (terrain->lowestQualityOverride)
                UnknownFunction480fb0(a5, (GridVertex*)data->field_0x134 + vertexStart, vertexCount,
                                      (unsigned short*)data->field_0x138 + indexStart, indexCount,
                                      g_gridQuadOrder[quad]);
            else
                UnknownFunction480c90(a5, (GridVertex*)data->field_0x134 + vertexStart, vertexCount,
                                      (unsigned short*)data->field_0x138 + indexStart, indexCount,
                                      g_gridQuadOrder[quad]);
        }
    }
    if (children && !data->b0) {
        for (int j = 0; j < 64; j += 16) {
            for (int i = 0; i < 4; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridQuadCell[quad] + i + j];
                if (child && child->childMask) {
                    if (child->data->field_0x13c != 0 && !child->data->b3) {
                        if (terrain->lowestQualityOverride)
                            child->UnknownFunction480fb0(a5, (GridVertex*)child->data->field_0x134,
                                                         child->data->field_0x13c,
                                                         (unsigned short*)child->data->field_0x138,
                                                         child->data->field_0x13e, -1);
                        else
                            child->UnknownFunction480c90(a5, (GridVertex*)child->data->field_0x134,
                                                         child->data->field_0x13c,
                                                         (unsigned short*)child->data->field_0x138,
                                                         child->data->field_0x13e, -1);
                    }
                    child->UnknownFunction480940(target, 0, 0, 16, 0, a5);
                }
            }
        }
    }
    return 1;
}


// 0x00481300: one 4-cell block; draws it when it has geometry and
// 0x00481a20 accepts it, then walks the child nodes unless this node's
// buffers stand in for them.
int DrawableGridNode::UnknownFunction481300(int x, int z, int size, int quad)
{
    GridBlockRange* ranges = (GridBlockRange*)data->field_0x17c;
    if (ranges) {
        int block = g_gridQuadOrder[quad];
        int count;
        if (block == 0)
            count = ranges[0].end;
        else
            count = ranges[block].end - ranges[block - 1].end;
        if (count != 0 && UnknownFunction481a20(block))
            UnknownVirtualSlot4(block);
    }
    if (children && !data->b0) {
        for (int j = 0; j < 64; j += 16) {
            for (int i = 0; i < 4; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[i + j + g_gridQuadCell[quad]];
                if (child && child->childMask) {
                    if (child->data->field_0x13c != 0 && !child->data->b3)
                        child->UnknownVirtualSlot4(-1);
                    child->UnknownFunction481180(0, 0, 16, 0);
                }
            }
        }
    }
    return 1;
}


// 0x00482b40: 0x00482a40's test over one subdivision level of a
// rectangle, skipping vertices pinned by a direction bit.
void DrawableGridNode::UnknownFunction482b40(int x0, int z0, int w, int h, int half, int start,
                                             int step, int rowStep)
{
    int row = 0;
    int zEnd = z0 + h;
    for (int z = z0 + start; z <= zEnd; z += rowStep, row++) {
        int x;
        if (start == 0)
            x = (row & 1) ? 0 : half;
        else
            x = half;
        for (x += x0; x <= x0 + w; x += step) {
            int i = g_gridRow17[z] + x;
            if (data->field_0x000[i] & 0xf)
                continue;
            if (!UnknownFunction481cc0(x, z)) {
                data->field_0x000[i] |= 0x40;
                if (data->field_0x000[i] & 0x80)
                    continue;
                data->field_0x000[i] |= 0x80;
            } else {
                data->field_0x000[i] &= ~0x40;
                if (!(data->field_0x000[i] & 0x80))
                    continue;
                if (data->field_0x000[i] & 0x10)
                    continue;
                data->field_0x000[i] &= ~0x80;
            }
            data->b1 = 1;
            UnknownFunction482c90(x, z, data->field_0x000[i] & 0x80);
        }
    }
}


// 0x00482dd0: sets or clears direction bit `dir` of a vertex, re-tests it
// and forwards border vertices to the parent node.
void DrawableGridNode::UnknownFunction482dd0(int x, int z, int flag, int dir, GridBaseCell* origin)
{
    if (x < 0 || z < 0 || x > 16 || z > 16)
        return;
    terrain->field_0x9c++;
    int changed = 0;
    int i = g_gridRow17[z] + x;
    GridBaseCell* cell = &block->cells[i];
    if (flag) {
        if (!(data->field_0x000[i] & (unsigned char)dir)) {
            data->field_0x000[i] |= (unsigned char)dir;
            changed = 1;
        }
        if (!(data->field_0x000[i] & 0x80)) {
            data->field_0x000[i] |= 0x80;
            data->b1 = 1;
            UnknownFunction482c90(x, z, data->field_0x000[i] & 0x80);
        } else if (!changed) {
            return;
        }
    } else {
        data->field_0x000[i] &= ~(unsigned char)dir;
        if (!(data->field_0x000[i] & 0xf) && UnknownFunction482f00(x, z)) {
            data->b1 = 1;
            UnknownFunction482c90(x, z, data->field_0x000[i] & 0x80);
        }
    }
    if (parent && !origin && (x == 0 || x == 16 || z == 0 || z == 16))
        ((DrawableGridNode*)parent)->UnknownFunction4835c0(this, x, z, flag, dir, cell);
}

// 0x0047f210: per-block buffer rebuild (see the note at the top).
int DrawableGridNode::UnknownFunction47f210()
{
    int x;
    int z = 0;
    int indexTotal = 0;
    int before = data->field_0x12a;
    terrain->field_0xa4++;
    int b = 0;
    int vertexTotal = 0;
    int bz;
    float saved = data->field_0x160;
    for (bz = 0; bz < 16; bz += 4) {
        x = 0;
        for (int bx = 0; bx < 16; bx += 4) {
            int closed;
            int n;
            int m;
            int size = UnknownVirtualSlot8(b);
            data->field_0x160 = saved;
            if (size == 0x40)
                data->field_0x160 *= 4.0f;
            g_gridVertexCache.Reset(this);
            g_gridOriginX = bx;
            g_gridOriginZ = bz;
            int vertexStart = vertexTotal;
            int indexStart = indexTotal;
            n = 0;
            m = n;
            n = UnknownFunction47fce0(4, n, x, z + 4, 2, -2, &closed);
            if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                n = UnknownFunction480700(n, x, z + 4, 4, -4);
            m = n;
            x += 4;
            n = UnknownFunction47fce0(4, n, x, z, -2, 2, &closed);
            if (n == m && (data->b0 ? 0 : data->field_0x12c) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                n = UnknownFunction480700(n, x, z, -4, 4);
            vertexTotal += g_gridVertexCache.count;
            indexTotal += n * 3;
            ((GridBlockRange*)data->field_0x17c)[b].centerY = (g_gridVertexCache.maxY + g_gridVertexCache.minY) * 0.5f;
            ((GridBlockRange*)data->field_0x17c)[b].extentY =
                (g_gridVertexCache.maxY - g_gridVertexCache.minY) * terrain->gridCellSize * 0.5f;
            ((GridBlockRange*)data->field_0x17c)[b].end = vertexTotal;
            ((GridBlockRange*)data->field_0x17c)[b].indexEnd = indexTotal;
            extra->blockRecords[b].field_0x1c = 1.0f;
            extra->blockRecords[b].field_0x20 = 0;
            extra->blockRecords[b].field_0x24 = 0;
            if (n != 0) {
                int vertexBytes = vertexTotal * 32;
                int indexBytes = (indexTotal + vertexTotal) * 2;
                if (data->field_0x134 == 0 || vertexBytes > data->field_0x140) {
                    g_gridDrawMemory -= data->field_0x140;
                    void* old = data->field_0x134;
                    int oldSize = data->field_0x140;
                    data->field_0x140 = vertexBytes + 0x80;
                    data->field_0x134 = DebugMalloc(data->field_0x140, __FILE__, 1077);
                    if (data->field_0x134 == 0) {
                        data->field_0x140 = 0;
                        data->field_0x13c = 0;
                        data->field_0x13e = 0;
                        return 0;
                    }
                    if (old) {
                        memcpy(data->field_0x134, old, oldSize);
                        DebugFree(old, __FILE__, 1085);
                    }
                    g_gridDrawMemory += data->field_0x140;
                    if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                        g_gridDrawMemoryPeak = g_gridDrawMemory;
                }
                if (data->field_0x138 == 0 || indexBytes > data->field_0x142) {
                    g_gridDrawMemory -= data->field_0x142;
                    void* old = data->field_0x138;
                    int oldSize = data->field_0x142;
                    data->field_0x142 = indexBytes + 0x20;
                    data->field_0x138 = DebugMalloc(data->field_0x142, __FILE__, 1098);
                    if (data->field_0x138 == 0) {
                        data->field_0x138 = 0;
                        data->field_0x142 = 0;
                        data->field_0x13c = 0;
                        data->field_0x13e = 0;
                        return 0;
                    }
                    if (old) {
                        memcpy(data->field_0x138, old, oldSize);
                        DebugFree(old, __FILE__, 1107);
                    }
                    g_gridDrawMemory += data->field_0x142;
                    if (g_gridDrawMemory > g_gridDrawMemoryPeak)
                        g_gridDrawMemoryPeak = g_gridDrawMemory;
                }
                memcpy((GridVertex*)data->field_0x134 + vertexStart, g_gridVertexCache.vertices,
                       (vertexTotal - vertexStart) * 32);
                memcpy((unsigned short*)data->field_0x138 + indexStart, g_gridIndices,
                       (indexTotal - indexStart) * 2);
            }
            b++;
        }
        z += 4;
    }
    if (data->ageEntry.size == 0)
        terrain->ageManager->Register(&data->ageEntry, UnknownFunction47ecc0, this, 0,
                                                   data->field_0x142 + data->field_0x140);
    else if (data->ageEntry.size != data->field_0x142 + data->field_0x140)
        data->ageEntry.size = data->field_0x142 + data->field_0x140;
    data->field_0x13c = vertexTotal;
    data->field_0x13e = indexTotal;
    data->field_0x12a = indexTotal / 3;
    if ((data->field_0x12a == 0 || before == 0) && data->field_0x12a != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if ((data->field_0x12a == 0 || before == 0) && data->field_0x12a != before && parent)
        ((DrawableGridNode*)parent)->data->b1 = 1;
    if (data->field_0x12a == 0) {
        data->field_0x13c = 0;
        data->field_0x13e = 0;
    }
    data->field_0x160 = saved;
    return 1;
}

// 0x00481b30: distance from the viewer to a 4 x 4 block's box.
float DrawableGridNode::UnknownFunction481b30(int block)
{
    float ex = terrain->viewerX;
    float ey = terrain->viewerY;
    float ez = terrain->viewerZ;
    float size = (data->field_0x16c - data->field_0x164) * 0.25f;
    float minX = (block % 4) * size + data->field_0x164;
    float maxX = minX + size;
    float minZ = (block / 4) * size + data->field_0x168;
    float maxZ = minZ + size;
    float dist;
    float d = ex - minX;
    if (d < 0.0f)
        dist = d * d;
    else if (ex - maxX > 0.0f)
        dist = (ex - maxX) * (ex - maxX);
    else
        dist = 0.0f;
    if (ez - minZ < 0.0f)
        dist += (ez - minZ) * (ez - minZ);
    else if (ez - maxZ > 0.0f)
        dist += (ez - maxZ) * (ez - maxZ);
    else
        dist += 0.0f;
    float below = ey - field_0x18;
    float above = ey - field_0x1c;
    if (below < 0.0f)
        return FastSqrt(below * below + dist);
    if (above > 0.0f)
        return FastSqrt(above * above + dist);
    return FastSqrt(0.0f + dist);
}

// 0x00483200: walks a border vertex down the node tree (VC6 turns the final
// child call into a loop, as retail does).
void DrawableGridNode::UnknownFunction483200(int x, int z, int flag, int dir, GridBaseCell* origin, int minLevel)
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
                UnknownFunction482dd0(x >> shift, z >> shift, flag, dir, origin);
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
                child->UnknownFunction483200(rx, z & mask, flag, dir, origin, minLevel);
            } else {
                if (cz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                    if (child)
                        child->UnknownFunction483200(rx, z & mask, flag, dir, origin, minLevel);
                }
                if (pz >= 16)
                    return;
                child = (DrawableGridNode*)children[g_gridRow16[pz] + cx];
                if (child == 0)
                    return;
                child->UnknownFunction483200(rx, (z & mask) + unit, flag, dir, origin, minLevel);
            }
        } else if (pz == cz) {
            if (cx < 16 && cz < 16) {
                child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                if (child)
                    child->UnknownFunction483200(rx, z & mask, flag, dir, origin, minLevel);
            }
            if (px >= 16 || cz >= 16)
                return;
            child = (DrawableGridNode*)children[g_gridRow16[cz] + px];
            if (child == 0)
                return;
            child->UnknownFunction483200(rx + unit, z & mask, flag, dir, origin, minLevel);
        } else {
            if (cx < 16) {
                if (cz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[cz] + cx];
                    if (child)
                        child->UnknownFunction483200(rx, z & mask, flag, dir, origin, minLevel);
                }
                if (pz < 16) {
                    child = (DrawableGridNode*)children[g_gridRow16[pz] + cx];
                    if (child)
                        child->UnknownFunction483200(rx, z & mask + unit, flag, dir, origin, minLevel);
                }
            }
            if (px >= 16)
                return;
            if (cz < 16) {
                child = (DrawableGridNode*)children[g_gridRow16[cz] + px];
                if (child)
                    child->UnknownFunction483200(rx + unit, z & mask, flag, dir, origin, minLevel);
            }
            if (pz >= 16)
                return;
            child = (DrawableGridNode*)children[g_gridRow16[pz] + px];
            if (child == 0)
                return;
            child->UnknownFunction483200(rx + unit, (z & mask) + unit, flag, dir, origin, minLevel);
        }
        return;
    }
    GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
    if (cell != origin)
        UnknownFunction482dd0(x, z, flag, dir, origin);
}

// 0x004815e0: the detail update (see the note at the top).
float DrawableGridNode::UnknownFunction4815e0(int coarse)
{
    float nearX;
    float farX;
    float nearZ;
    float farZ;
    GridTerrain* t = terrain;
    GridNodeDrawData* d = data;
    float ax = t->viewerX - d->field_0x164;
    float bx = t->viewerX - d->field_0x16c;
    if (ax < 0.0f) {
        nearX = ax * ax;
        farX = bx * bx;
    } else if (bx > 0.0f) {
        nearX = bx * bx;
        farX = ax * ax;
    } else {
        nearX = 0.0f;
        float f = ((t->viewerX + t->viewerX < d->field_0x164 + d->field_0x16c) ? d->field_0x16c : d->field_0x164) - t->viewerX;
        farX = f * f;
    }
    float az = t->viewerZ - d->field_0x168;
    float bz = t->viewerZ - d->field_0x170;
    if (az < 0.0f) {
        nearZ = az * az;
        farZ = bz * bz;
    } else if (bz > 0.0f) {
        nearZ = bz * bz;
        farZ = az * az;
    } else {
        nearZ = 0.0f;
        float f = ((t->viewerZ + t->viewerZ < d->field_0x170 + d->field_0x168) ? d->field_0x170 : d->field_0x168) - t->viewerZ;
        farZ = f * f;
    }
    float near2 = nearZ + nearX;
    float far2 = farZ + farX;
    float dyLo = t->viewerY - field_0x18;
    float dyHi = t->viewerY - field_0x1c;
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
    terrain->field_0x98++;
    data->field_0x126 = data->field_0x122;
    data->field_0x128 = data->field_0x124;
    if (terrain->field_0x6c == 0) {
        data->field_0x124 = 0;
    } else if (near2 == 0.0f) {
        data->field_0x124 = 0xffff;
    } else if (far2 == 0.0f) {
        data->field_0x124 = 0xffff;
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
        float v = (dySq / q + q) * data->field_0x174 * terrain->field_0x74;
        data->field_0x122 = (v < 65535.0f) ? (unsigned short)(int)v : 0xffff;
        if (dyLo < 0.0f)
            dyLo = -dyLo;
        if (dyHi < 0.0f)
            dyHi = -dyHi;
        float m = (dyLo > dyHi) ? dyLo : dyHi;
        m = m * m;
        float ratioNear = (m + near2) / nearDist;
        float ratioFar = (m + far2) / farDist;
        float w = ((ratioNear > ratioFar) ? ratioNear : ratioFar) * (data->field_0x174 * terrain->field_0x74);
        data->field_0x124 = (w < 65535.0f) ? (unsigned short)(int)w : 0xffff;
        return distance;
    }
    float q;
    if (dySq < near2)
        q = FastSqrt(near2);
    else if (dySq > far2)
        q = FastSqrt(far2);
    else
        q = dy;
    data->field_0x122 = (unsigned short)(int)((dySq / q + q) * data->field_0x174 * terrain->field_0x74);
    if (data->field_0x122 > 0xffff)
        data->field_0x122 = 0xffff;
    return distance;
}

// 0x00481de0: the visibility walk (see the note at the top).
int DrawableGridNode::UnknownFunction481de0(void* a0, void* a1, float* center, float* extent, int a4, int a5,
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
                UnknownFunction4824c0((UnknownTextureStream*)a0, a1, 0, a9);
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
                count += UnknownFunction481de0(a0, a1, &c1.x, &childExtent.x, a4, a5, size, a7 + 1, a8 * 4, a9);
                count += UnknownFunction481de0(a0, a1, &c2.x, &childExtent.x, a4 + size, a5, size, a7 + 1, a8 * 4 + 1, a9);
                count += UnknownFunction481de0(a0, a1, &c3.x, &childExtent.x, a4, a5 + size, size, a7 + 1, a8 * 4 + 2, a9);
                count += UnknownFunction481de0(a0, a1, &c4.x, &childExtent.x, a4 + size, a5 + size, size, a7 + 1, a8 * 4 + 3, a9);
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
                        count += child->UnknownFunction481de0(a0, a1, &child->center.x, &child->extent.x, 0, 0, 16, 0, 0, a9);
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
                            child->UnknownFunction4824c0((UnknownTextureStream*)a0, a1, 0, a9);
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
                        child->UnknownFunction4824c0((UnknownTextureStream*)a0, a1, 1, a9);
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
