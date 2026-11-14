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
// dx/dz locals shared by the u, ny and packed terms give 744; the operand
// order of the index and origin sums, and ten more packed-word spellings
// (`* 65536` distributed or not, `- gz + z`, `x + -gx`), leave 1069.
//
// DrawableGridNode 0x0047e600 (node loader) is exact and lives in
// src/reconstructed/Griddraw.cpp. Its last difference was the operand order
// of the extent.x/extent.y products (`fld cell; fmul min` against retail's
// `fld min; fmul cell`, 1605 of 1617). Copying the two minimums into float
// locals just before the products makes them the newer leaves (1617; one
// local alone gives 1613 for x, 1609 for y). Forms that did not move it:
// either operand order, the sum order of the centre terms, a `cell` local,
// inline multiply helpers, a GridVec3 constructor for the centre, 432
// centre/extent spellings, references or pointer aliases to either operand
// (216 forms), 0..33 dead locals and 0..63 prepended typedefs.
//
// DrawableGridNode 0x0047f210 (per-block buffer rebuild, 1576 bytes; 426 of
// 1586): the calls, reallocations, copies and the bookkeeping after the loops
// match in shape. VC6 here gives x edi and the triangle-count copy esi
// (retail: esi and edi), swaps the frame slots of z and indexTotal and of bx
// and the two strength-reduced block offsets, loads minY before maxY for the
// box centre, re-reads indexTotal for the index byte count and stores a dead
// vertexStart * 32 (10 bytes longer). Tried: all 24 declaration orders of
// x, z, indexTotal and bx (no effect), x and bx as one variable (VC6 merges
// them; retail keeps both, so x steps inside the body), three
// triangles/prevTriangles initialisations and the origin taken from x/z.
//
// DrawableGridNode 0x004815e0 (detail update, 1082 bytes; candidate 1083):
// computes the squared near and far distances of the viewer to the node's
// x and z range, the y delta, the distance (FastSqrt), and the two
// 16-bit detail words from (dy² / nearest + nearest) * +0x174 * +0x74,
// nearest being the closest of the three distances. Control flow, calls and clamps match
// (the 65535 clamps are ternaries; `0xffff` immediates stay immediates
// once they are). Retail keeps the two axis deltas on the x87 stack across
// the three-way branch (fcom on the first, the second loaded before the
// fnstsw, spilled ad hoc in the arms); VC6 here stores both and reloads
// them, which shifts the frame slots, keeps `push edi` in the prologue
// instead of after the early return and tail-merges the early return.
// Tried: locals, block scopes, const/register, repeated expressions, an
// inline helper with reference, pointer or by-value arguments, and an
// accumulating form (which enregisters only the first delta).
// The two 65535 clamps are written `upperError >= 65535.0f ? 0xffff : ...`, which
// gives retail's branch polarity (64 of 1083; the `<` form 60). Also tried for the
// deltas: no named deltas (expressions throughout), only one of them named,
// the delta assigned inside the test (`(ax = ...) < 0`, comma forms), copies
// of the bounds overwritten by the deltas (the form that made 0x00481b30
// exact), and the far term written before the near term; none keeps both on
// the x87 stack, and small probes never do either. Also inert or worse: the
// second delta assigned in the else-if test (1067 bytes, strict 49), the
// grid/data locals swapped or dropped, the far offset without a local, and
// the early return as `coarse != 0`, a goto past it or with `distance`
// declared first (the return still merges with the final epilogue).
//
// DrawableGridNode 0x00481de0 (visibility walk, 1750 bytes; candidate
// 1712): tests the node's box (or the four quadrant boxes, recursively,
// down to the child blocks) against the clipper and updates the child
// mask; `a7` is the recursion level, `a8` the quadrant path and the
// child blocks' masks are set to 0x3fffff (all visible), 0x1fffff (leaf)
// or cleared with 0x00464e90. Structure and calls match; the frame is
// the same size. VC6 here keeps `extent` in ebp and `screenMatrix` in ebx where
// retail keeps `size` then `a9` in ebx and `screenMatrix` in ebp, and lays out the
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
// Session 25 (masked linked-style 0.675 -> 0.711, strict 143 -> 81/1750):
// - logic fix: on the size == 16 path with data->b0 set retail loads ebx = 1
//   (0x481e8a), stores it to count/childMask/changed and then pushes ebx as
//   the `coarse` argument of every later UpdateLevelOfDetail and recursive
//   call (0x481eec, 0x4821b5, 0x48225a, 0x482335) without reloading it, so
//   the walk continues with coarse = 1 there; the source now says so.
// - the corners test is `if (corners) { loops } else { subdivide }` (retail
//   falls into the loops and jumps back to the subdivision, 0x482144), the
//   depth switch sits under its own `if (visible)` after the if/else, and
//   the parent walk is a do/while (no null test of `this`, 0x482448).
// - ruled out here: center/extent through `const GridVec3&` vs indexed
//   (0.710 vs 0.711), `size <= 1` first (0.563), visible/corners initialised
//   at the declaration instead of the else block (0.631).
// Linked-accuracy audit (masked 0.725 -> 0.719, 500/500 instructions):
// - logic fix: the count test after the four recursive calls compared
//   `depth + 1 == 8`; retail reloads esi from the halved `size` slot at
//   0x4820fe (`mov esi,[esp+0x8c]` with three pushes outstanding) and tests
//   `cmp esi,8` at 0x482370, so the condition is `size == 8 || size == 16`.
// - retail shares one `count == 0 / childMask == 0` block (0x482370) between
//   the all-visible path (reached only for size 16) and the subdivision path
//   (size 8 or 16); the source writes it once after the corners if/else as
//   `(corners == 0 && size == 8) || size == 16`. That is behaviourally the
//   same: the child case reaches it with size 0. Two separate tests scored
//   0.632; ordering `size == 16 ||` first 0.721 (larger body); a split or
//   goto form 0.620-0.638.
// Big-gap pass (73 of 1750 strict, candidate 1712, same frame 0x54): the
// first divergence is at offset 4, register choice. Retail loads `size`
// into ebx, screenMatrix into ebp and center into esi between the pushes
// and compares `cmp ebx, 1`; VC6 here loads size (edx), extent (ebp),
// center (esi) and screenMatrix (ebx) up front and keeps the constant 1
// in ecx (`cmp edx, ecx`), then pushes TestBox's arguments from the
// registers where retail reloads extent from the frame. Unchanged
// (186 of 500 aligned): `count = 1` before `coarse = 1`, the 1s chained
// through coarse (`count = coarse` ..., 69), `visible`/`corners` order
// in the else block, oldMask declared last; `size >= 2` 101 strict by
// position only (185 aligned), oldMask declared first 174 aligned.
// Dropping `coarse = 1` (a probe, not retail's logic) does not free the
// constant register either.
// k-scan (s22): compiled behind 0..63 prepended typedefs, 0x0047de90,
// 0x0047f210, 0x004815e0, 0x00481de0 have the same code at every
// count, so their differences are not a declaration-count effect
// (docs/VC6_OPERAND_ORDER.md section 2).

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


// 0x0047f210: per-block buffer rebuild (see the note at the top).
int DrawableGridNode::RebuildBlockBuffers()
{
    int x;
    int z = 0;
    int indexTotal = 0;
    int before = data->triangleCount;
    terrain->bufferRebuilds++;
    int block = 0;
    int vertexTotal = 0;
    int bz;
    float saved = data->texStep;
    for (bz = 0; bz < 16; bz += 4) {
        x = 0;
        for (int bx = 0; bx < 16; bx += 4) {
            int closed;
            int triangles;
            int prevTriangles;
            int size = GetBlockTextureSize(block);
            data->texStep = saved;
            if (size == 0x40)
                data->texStep *= 4.0f;
            g_gridVertexCache.Reset(this);
            g_gridOriginX = bx;
            g_gridOriginZ = bz;
            int vertexStart = vertexTotal;
            int indexStart = indexTotal;
            triangles = 0;
            prevTriangles = triangles;
            triangles = EmitDiagonalTriangles(4, triangles, x, z + 4, 2, -2, &closed);
            if (triangles == prevTriangles && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                triangles = AppendCornerTriangle(triangles, x, z + 4, 4, -4);
            prevTriangles = triangles;
            x += 4;
            triangles = EmitDiagonalTriangles(4, triangles, x, z, -2, 2, &closed);
            if (triangles == prevTriangles && (data->b0 ? 0 : data->childTriangleCount) == 0 && parent &&
                ((DrawableGridNode*)parent)->extra->texture == 0)
                triangles = AppendCornerTriangle(triangles, x, z, -4, 4);
            vertexTotal += g_gridVertexCache.count;
            indexTotal += triangles * 3;
            ((GridBlockRange*)data->blockRanges)[block].centerY =
                (g_gridVertexCache.maxY + g_gridVertexCache.minY) * 0.5f;
            ((GridBlockRange*)data->blockRanges)[block].extentY =
                (g_gridVertexCache.maxY - g_gridVertexCache.minY) * terrain->gridCellSize * 0.5f;
            ((GridBlockRange*)data->blockRanges)[block].end = vertexTotal;
            ((GridBlockRange*)data->blockRanges)[block].indexEnd = indexTotal;
            extra->blockRecords[block].mappingScale = 1.0f;
            extra->blockRecords[block].mappingOffsetU = 0;
            extra->blockRecords[block].mappingOffsetV = 0;
            if (triangles != 0) {
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
            block++;
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


// 0x004815e0: the detail update (see the note at the top).
float DrawableGridNode::ComputeViewerDetail(int coarse)
{
    float nearX;
    float farX;
    float nearZ;
    float farZ;
    GridTerrain* grid = terrain;
    GridNodeDrawData* drawData = data;
    float ax = grid->viewerX - drawData->gridMinX;
    float bx = grid->viewerX - drawData->gridMaxX;
    if (ax < 0.0f) {
        nearX = ax * ax;
        farX = bx * bx;
    } else if (bx > 0.0f) {
        nearX = bx * bx;
        farX = ax * ax;
    } else {
        nearX = 0.0f;
        float farOffset =
            ((grid->viewerX + grid->viewerX < drawData->gridMinX + drawData->gridMaxX)
                 ? drawData->gridMaxX : drawData->gridMinX) - grid->viewerX;
        farX = farOffset * farOffset;
    }
    float az = grid->viewerZ - drawData->gridMinZ;
    float bz = grid->viewerZ - drawData->gridMaxZ;
    if (az < 0.0f) {
        nearZ = az * az;
        farZ = bz * bz;
    } else if (bz > 0.0f) {
        nearZ = bz * bz;
        farZ = az * az;
    } else {
        nearZ = 0.0f;
        float farOffset =
            ((grid->viewerZ + grid->viewerZ < drawData->gridMaxZ + drawData->gridMinZ)
                 ? drawData->gridMaxZ : drawData->gridMinZ) - grid->viewerZ;
        farZ = farOffset * farOffset;
    }
    float near2 = nearZ + nearX;
    float far2 = farZ + farX;
    float dyLo = grid->viewerY - minHeight;
    float dyHi = grid->viewerY - maxHeight;
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
        float nearest;
        if (dySq < near2)
            nearest = nearDist;
        else if (dySq > far2)
            nearest = farDist;
        else
            nearest = dy;
        float lowerError = (dySq / nearest + nearest) * data->inverseErrorScale * terrain->drawDistanceRatio;
        data->lowerErrorLimit = (lowerError >= 65535.0f) ? 0xffff : (unsigned short)(int)lowerError;
        if (dyLo < 0.0f)
            dyLo = -dyLo;
        if (dyHi < 0.0f)
            dyHi = -dyHi;
        float maxDy = (dyLo > dyHi) ? dyLo : dyHi;
        maxDy = maxDy * maxDy;
        float ratioNear = (maxDy + near2) / nearDist;
        float ratioFar = (maxDy + far2) / farDist;
        float upperError = ((ratioNear > ratioFar) ? ratioNear : ratioFar)
            * (data->inverseErrorScale * terrain->drawDistanceRatio);
        data->upperErrorLimit = (upperError >= 65535.0f) ? 0xffff : (unsigned short)(int)upperError;
        return distance;
    }
    float nearest;
    if (dySq < near2)
        nearest = FastSqrt(near2);
    else if (dySq > far2)
        nearest = FastSqrt(far2);
    else
        nearest = dy;
    data->lowerErrorLimit =
        (unsigned short)(int)((dySq / nearest + nearest) * data->inverseErrorScale
                              * terrain->drawDistanceRatio);
    if (data->lowerErrorLimit > 0xffff)
        data->lowerErrorLimit = 0xffff;
    return distance;
}

// 0x00481de0: the visibility walk (see the note at the top).
int DrawableGridNode::WalkVisibilityQuadrants(void* stream, void* screenMatrix, float* center, float* extent, int x, int z,
                                            int size, int depth, int path, int coarse)
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
        visible = reinterpret_cast<GridVisibilityClipper*>(g_visibilityClipper)->TestBox(terrain->renderer->camera, (float*)screenMatrix, center, extent, 0, &corners, 0);
        if (corners != 8)
            corners = 0;
        if (size == 16) {
            if (visible) {
                UpdateLevelOfDetail((FileStream*)stream, screenMatrix, 0, coarse);
                if (data->b0) {
                    coarse = 1;
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
        if (corners != 0) {
            for (j = 0; j < size; j++) {
                for (i = 0; i < size; i++) {
                    DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[z + j] + i + x];
                    if (child) {
                        if (child->childMask != 0x3fffff) {
                            child->childMask = 0x3fffff;
                            changed = 1;
                            if (child->children)
                                child->data->b1 = 1;
                        }
                        child->UpdateLevelOfDetail((FileStream*)stream, screenMatrix, 1, coarse);
                        count++;
                    }
                }
            }
            switch (depth) {
            case 0:
                childMask = 0x1fffff;
                break;
            case 1:
                childMask |= 0x1e0 << (path * 4);
                break;
            }
        } else {
            size >>= 1;
            if (size) {
                GridVec3 childExtent = GridVec3(extent[0] * 0.5f, extent[1], extent[2] * 0.5f);
                GridVec3 c1 = GridVec3(center[0] - childExtent.x, center[1], center[2] - childExtent.z);
                GridVec3 c2 = GridVec3(childExtent.x + center[0], center[1], center[2] - childExtent.z);
                GridVec3 c3 = GridVec3(center[0] - childExtent.x, center[1], childExtent.z + center[2]);
                GridVec3 c4 = GridVec3(childExtent.x + center[0], center[1], childExtent.z + center[2]);
                count += WalkVisibilityQuadrants(stream, screenMatrix, &c1.x, &childExtent.x, x, z, size, depth + 1, path * 4, coarse);
                count += WalkVisibilityQuadrants(stream, screenMatrix, &c2.x, &childExtent.x, x + size, z, size, depth + 1, path * 4 + 1, coarse);
                count += WalkVisibilityQuadrants(stream, screenMatrix, &c3.x, &childExtent.x, x, z + size, size, depth + 1, path * 4 + 2, coarse);
                count += WalkVisibilityQuadrants(stream, screenMatrix, &c4.x, &childExtent.x, x + size, z + size, size, depth + 1, path * 4 + 3, coarse);
            } else {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[z] + x];
                if (child) {
                    if (child->children) {
                        count += child->WalkVisibilityQuadrants(stream, screenMatrix, &child->center.x, &child->extent.x, 0, 0, 16, 0, 0, coarse);
                        child->data->b1 = 1;
                    } else {
                        int dummy;
                        g_gridBoxTests++;
                        if (reinterpret_cast<GridVisibilityClipper*>(g_visibilityClipper)->TestBox(terrain->renderer->camera, (float*)screenMatrix, &child->center.x,
                                                         &child->extent.x, 0, 0, &dummy)) {
                            if (child->childMask != 0x1fffff) {
                                child->childMask = 0x1fffff;
                                changed = 1;
                            }
                            child->UpdateLevelOfDetail((FileStream*)stream, screenMatrix, 0, coarse);
                            count++;
                        } else if (child->childMask) {
                            child->childMask = 0;
                            changed = 1;
                            child->UnknownFunction464e90();
                        }
                    }
                }
            }
        }
        if ((corners == 0 && size == 8) || size == 16) {
            if (count == 0)
                childMask = 0;
            else if (childMask == 0)
                changed = 1;
        }
    } else {
        for (j = 0; j < size; j++) {
            for (i = 0; i < size; i++) {
                DrawableGridNode* child = (DrawableGridNode*)children[g_gridRow16[z + j] + i + x];
                if (child && child->childMask) {
                    child->childMask = 0;
                    changed = 1;
                    child->UnknownFunction464e90();
                }
            }
        }
    }
    if (visible) {
        switch (depth) {
        case 0:
            childMask |= 1;
            break;
        case 1:
            childMask |= 2 << path;
            return count;
        case 2:
            childMask |= 0x20 << path;
            return count;
        }
    }
    if (depth != 0)
        return count;
    if (count == 0x100)
        childMask |= 0x200000;
    if (changed || oldMask != childMask) {
        DrawableGridNode* node = this;
        do {
            node->data->b1 = 1;
            node = (DrawableGridNode*)node->parent;
        } while (node);
    }
    if (oldMask != 0 && childMask == 0)
        UnknownFunction464e90();
    return count;
}
