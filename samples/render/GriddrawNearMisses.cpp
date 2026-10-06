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
// slot. Retail multiplies the extent terms as `fld min; fmul cell` and
// stores extent.y with fstp before the 0.1 test; VC6 here loads the cell
// size first and keeps extent.y on the x87 stack, which adds a pop block and
// moves the out-of-line allocation earlier.
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
// 0x80 test and the inverted branch order were tried.

#include <float.h>
#include <string.h>

#include "../../src/reconstructed/Griddraw.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"

// Per-TU copy of the shared row table (see Griddraw.cpp).
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
            field_0x2d64 = node->terrain->field_0xcb0;
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
    stream->UnknownFunction461640(&field_0x22, 2, 1);
    block = terrain->blocks[field_0x22];
    stream->UnknownFunction461640(&header, 4, 1);
    stream->UnknownFunction461640(words, 4, 16);
    for (i = 0; i < 16; i++) {
        if ((words[i] & 0xfffe) != 0xfffe) {
            data = (GridNodeDrawData*)DebugCalloc(1, 0x540, __FILE__, 687);
            data->field_0x17c = (char*)data + 0x180;
            extra = (GridNodeExtra*)((char*)data->field_0x17c + 0xc0);
            extra->field_0x38 = (GridBlockRecord*)((char*)extra + 0x40);
            for (int j = 0; j < 16; j++) {
                extra->field_0x38[j].field_0x00 = words[j];
                extra->field_0x38[j].field_0x04 = 0;
            }
            goto allocated;
        }
    }
    data = (GridNodeDrawData*)DebugCalloc(1, 0x1c0, __FILE__, 698);
    data->field_0x17c = 0;
    extra = (GridNodeExtra*)((char*)data + 0x180);
    extra->field_0x38 = 0;
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
    if (extra->field_0x00) {
        extra->field_0x04 = DebugMalloc(extra->field_0x00, __FILE__, 732);
        g_gridExtraBytes += extra->field_0x00;
        stream->UnknownFunction461640(extra->field_0x04, extra->field_0x00, 1);
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
    if (extent.x < 0.1f && extent.y < 0.1f && extent.z < 0.1f)
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
            if (terrain->field_0xcb0)
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
                        if (terrain->field_0xcb0)
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
void DrawableGridNode::UnknownFunction482dd0(int x, int z, int flag, int dir, int noParent)
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
    if (parent && !noParent && (x == 0 || x == 16 || z == 0 || z == 16))
        ((DrawableGridNode*)parent)->UnknownFunction4835c0(this, x, z, flag, dir, cell);
}
