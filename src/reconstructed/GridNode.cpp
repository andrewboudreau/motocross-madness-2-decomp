// GridNode.cpp -- the GridNode code after Griddraw.cpp: the destructor
// 0x004838a0 (deleting wrapper 0x004838f0, GridNode slot 0), 0x00483910,
// slot 1 0x00483d40 and the leaf lookup 0x00484d70, ending before
// GUIManager.cpp's initializers (0x00484dd0). RTTI .?AVGridNode@@ (COL
// 0x0055cb08, vtable 0x00553ec0) is tier 1; the class is declared in
// Griddraw.h. There is no __FILE__ literal: the file name is ours (tier 3).
// Its .data is the third copy of the shared row tables (0x0056c2ac), which
// 0x00483910 and 0x00484d70 read. The cell sampler 0x00483910 and the leaf
// lookup 0x00484d70 are exact; slot 1 (segment intersection) is a near miss.

#include "Griddraw.h"

#include "DebugAlloc.h"
#include "MatrixUtil.h"
#include "../bringup/X87FloatToInt.h"
#include <math.h>
#include <stdio.h>
extern "C" void __stdcall OutputDebugStringA(const char*);

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

// Height of a cell corner in world units (the formula of 0x00483910).
static inline float GridCornerHeight(const GridNode* node, const GridBaseCell* cell)
{
    return ((cell->height - node->block->baseHeight) * node->reliefScale + node->block->baseHeight) * node->heightScale;
}

// Cell index of a coordinate midway between two crossings: retail converts
// (a + b) / (2 * cellSize) - 0.5 with a bare fistp (round to nearest even), i.e.
// floor() of the midpoint cell coordinate except at exact ties. Retail's fistp is an
// inlined inline-asm helper writing through an int pointer (ix/iy stay memory-homed);
// this plain (not inline) function keeps that helper boundary as a call, so the size of
// IntersectSegment no longer decides whether VC6 inlines the long X87FloatToInt body.
static void GridRoundToInt(float value, int* out)
{
    *out = X87FloatToInt(value);
}

// Corner-height bounds of a leaf cell, as retail's x87 ternaries.
static inline float GridMin(float a, float b)
{
    return a < b ? a : b;
}

static inline float GridMax(float a, float b)
{
    return a > b ? a : b;
}

// 0x00483d40 (GridNode slot 1). Not exact (4140 bytes; retail rounds the four
// midpoint cell indices with an inline-asm fistp helper, which also gives it an
// ebp frame, where GridRoundToInt is a call; the #pragma below gives the frame;
// docs/GRIDDRAW.md forms tried)
// Intersects the segment p0..p1 with the height field: the crossings with
// the node's x and y grid lines, merged along the segment, then per cell
// either the child whose height range the segment reaches or, in a leaf,
// the two cell triangles. A hit is written in parent units; returns 1.
#pragma optimize("y", off)
int GridNode::IntersectSegment(const GridVec3* p0, const GridVec3* p1, GridVec3* out, GridNode** outNode,
                               int* outX, int* outZ)
{
    GridVec3 cross[34];
    GridVec3 xCross[17];
    GridVec3 yCross[17];
    Vector3 hit;
    float dx = p1->x - p0->x;
    float dy = p1->y - p0->y;
    float dz = p1->z - p0->z;
    float cellSize = (float)(1 << shift);
    int nx = 0;
    int ny = 0;
    if (dx != 0.0f) {
        float step;
        if (dx < 0.0f) {
            step = -cellSize;
        } else {
            step = cellSize;
        }
        float zPerX = dz / dx;
        float yPerX = dy / dx;
        float z = p0->z;
        float y = p0->y;
        float x = (float)(int)(p0->x / step) * step;
        if (step < 0.0f) {
            x -= step;
        }
        float yStep = yPerX * step;
        float zStep = zPerX * step;
        if (x == p0->x) {
            xCross[0] = *p0;
            nx = 1;
        } else {
            x += step;
            if ((dx > 0.0f && x <= p1->x) || (dx < 0.0f && x >= p1->x)) {
                xCross[0].x = x;
                float t = x - p0->x;
                nx = 1;
                y += t * yPerX;
                xCross[0].y = y;
                z += t * zPerX;
                xCross[0].z = z;
            }
        }
        if (dx > 0.0f) {
            for (x += step; x <= p1->x; x += step) {
                xCross[nx].x = x;
                y += yStep;
                xCross[nx].y = y;
                z += zStep;
                xCross[nx].z = z;
                nx++;
            }
        } else if (dx < 0.0f) {
            for (x += step; x >= p1->x; x += step) {
                xCross[nx].x = x;
                y += yStep;
                xCross[nx].y = y;
                z += zStep;
                xCross[nx].z = z;
                nx++;
            }
        }
    }
    if (dy != 0.0f) {
        float step;
        if (dy < 0.0f) {
            step = -cellSize;
        } else {
            step = cellSize;
        }
        float zPerY = dz / dy;
        float xPerY = dx / dy;
        float z = p0->z;
        float x = p0->x;
        float y = (float)(int)(p0->y / step) * step;
        if (step < 0.0f) {
            y -= step;
        }
        float xStep = xPerY * step;
        float zStep = zPerY * step;
        if (y == p0->y) {
            yCross[0] = *p0;
            ny = 1;
        } else {
            y += step;
            if ((dy > 0.0f && y <= p1->y) || (dy < 0.0f && y >= p1->y)) {
                float t = y - p0->y;
                ny = 1;
                x += t * xPerY;
                yCross[0].x = x;
                yCross[0].y = y;
                z += t * zPerY;
                yCross[0].z = z;
            }
        }
        if (dy > 0.0f) {
            for (y += step; y <= p1->y; y += step) {
                x += xStep;
                yCross[ny].x = x;
                yCross[ny].y = y;
                z += zStep;
                yCross[ny].z = z;
                ny++;
            }
        } else if (dy < 0.0f) {
            for (y += step; y >= p1->y; y += step) {
                x += xStep;
                yCross[ny].x = x;
                yCross[ny].y = y;
                z += zStep;
                yCross[ny].z = z;
                ny++;
            }
        }
    }
    // Merge the two crossing lists along the segment, dropping shared corners.
    int count = nx + ny;
    int i = 0;
    int j = 0;
    int k = 0;
    if ((xCross[0].x != p0->x || xCross[0].y != p0->y) && (yCross[0].x != p0->x || yCross[0].y != p0->y)) {
        cross[0] = *p0;
        k = 1;
        count++;
    }
    if (dx == 0.0f) {
        for (; k < count; k++, j++) {
            cross[k].x = yCross[j].x;
            cross[k].y = yCross[j].y;
            cross[k].z = yCross[j].z;
        }
    } else {
        for (; k < count; k++) {
            if (i < nx) {
                if (j < ny) {
                    if ((dx > 0.0f && xCross[i].x < yCross[j].x) || (dx < 0.0f && xCross[i].x > yCross[j].x)) {
                        cross[k].x = xCross[i].x;
                        cross[k].y = xCross[i].y;
                        cross[k].z = xCross[i].z;
                        i++;
                        if (cross[k].x == yCross[j].x || cross[k].y == yCross[j].y) {
                            j++;
                            count--;
                        }
                    } else {
                        cross[k].x = yCross[j].x;
                        cross[k].y = yCross[j].y;
                        cross[k].z = yCross[j].z;
                        j++;
                        if (cross[k].x == xCross[i].x || cross[k].y == xCross[i].y) {
                            i++;
                            count--;
                        }
                    }
                } else {
                    if (k != 0 && cross[k - 1].x == xCross[i].x && cross[k - 1].y == xCross[i].y) {
                        count--;
                    } else {
                        cross[k].x = xCross[i].x;
                        cross[k].y = xCross[i].y;
                        cross[k].z = xCross[i].z;
                    }
                    i++;
                }
            } else {
                if (k != 0 && cross[k - 1].x == yCross[j].x && cross[k - 1].y == yCross[j].y) {
                    count--;
                } else {
                    cross[k].x = yCross[j].x;
                    cross[k].y = yCross[j].y;
                    cross[k].z = yCross[j].z;
                }
                j++;
            }
        }
    }
    if (cross[count - 1].x != p1->x || cross[count - 1].y != p1->y) {
        cross[count++] = *p1;
    }
    if (children) {
        for (k = 0; k < count - 1; k++) {
            float twoCells = 2 * cellSize;
            int ix;
            int iy;
            float cy = (cross[k].y + cross[k + 1].y) / twoCells;
            GridRoundToInt((cross[k].x + cross[k + 1].x) / twoCells - 0.5f, &ix);
            GridRoundToInt(cy - 0.5f, &iy);
            if (ix == 16) {
                ix = 15;
            }
            if (iy == 16) {
                iy = 15;
            }
            GridNode* child = children[g_gridRow16[iy] + ix];
            if (child == 0) {
                continue;
            }
            if (cross[k].z < child->minHeight && cross[k + 1].z < child->minHeight) {
                continue;
            }
            if (cross[k].z > child->maxHeight && cross[k + 1].z > child->maxHeight) {
                continue;
            }
            float ox = ix * cellSize;
            float oy = iy * cellSize;
            GridVec3 a;
            GridVec3 b;
            a = GridVec3(cross[k].x - ox, cross[k].y - oy, cross[k].z);
            b = GridVec3(cross[k + 1].x - ox, cross[k + 1].y - oy, cross[k + 1].z);
            if (child->IntersectSegment(&a, &b, out, outNode, outX, outZ)) {
                return 1;
            }
        }
        return 0;
    }
    for (k = 0; k < count - 1; k++) {
        float twoCells = 2 * cellSize;
        int ix;
        int iy;
        float cy = (cross[k].y + cross[k + 1].y) / twoCells;
        GridRoundToInt((cross[k].x + cross[k + 1].x) / twoCells - 0.5f, &ix);
        GridRoundToInt(cy - 0.5f, &iy);
        int cell = g_gridRow17[iy] + ix;
        float h00 = GridCornerHeight(this, &block->cells[cell]);
        float h10 = GridCornerHeight(this, &block->cells[cell + 1]);
        float h01 = GridCornerHeight(this, &block->cells[cell + 17]);
        float h11 = GridCornerHeight(this, &block->cells[cell + 18]);
        float hmin = GridMin(GridMin(h00, h10), GridMin(h01, h11));
        float hmax = GridMax(GridMax(h00, h10), GridMax(h01, h11));
        if (cross[k].z < hmin && cross[k + 1].z < hmin) {
            continue;
        }
        if (cross[k].z > hmax && cross[k + 1].z > hmax) {
            continue;
        }
        float fx = (float)ix;
        float fy = (float)iy;
        float fx1 = fx + 1.0f;
        float fy1 = fy + 1.0f;
        const Vector3* from = (const Vector3*)&cross[k];
        const Vector3* to = (const Vector3*)&cross[k + 1];
        Vector3 v0;
        Vector3 v1;
        Vector3 v2;
        // Each cell is split along one diagonal, alternating like a checkerboard; a
        // hit counts for the triangle whose right-angle corner it is nearer to.
        if ((ix ^ iy) & 1) {
            v0 = Vector3(fx, fy, h00);
            v1 = Vector3(fx1, fy, h10);
            v2 = Vector3(fx, fy1, h01);
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float dx0 = hit.x - fx;
                float dy0 = hit.y - fy;
                float dx1 = hit.x - fx1;
                float dy1 = hit.y - fy1;
                if (dx0 * dx0 + dy0 * dy0 <= dx1 * dx1 + dy1 * dy1) {
                    goto found;
                }
            }
            v0 = Vector3(fx1, fy, h10);
            v1 = Vector3(fx1, fy1, h11);
            v2 = Vector3(fx, fy1, h01);
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float dx0 = hit.x - fx;
                float dy0 = hit.y - fy;
                float dx1 = hit.x - fx1;
                float dy1 = hit.y - fy1;
                if (dx1 * dx1 + dy1 * dy1 <= dx0 * dx0 + dy0 * dy0) {
                    goto found;
                }
            }
        } else {
            v0 = Vector3(fx, fy, h00);
            v1 = Vector3(fx1, fy, h10);
            v2 = Vector3(fx1, fy1, h11);
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float dx0 = hit.x - fx1;
                float dy0 = hit.y - fy;
                float dx1 = hit.x - fx;
                float dy1 = hit.y - fy1;
                if (dx0 * dx0 + dy0 * dy0 <= dx1 * dx1 + dy1 * dy1) {
                    goto found;
                }
            }
            v0 = Vector3(fx, fy, h00);
            v1 = Vector3(fx1, fy1, h11);
            v2 = Vector3(fx, fy1, h01);
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float dx0 = hit.x - fx1;
                float dy0 = hit.y - fy;
                float dx1 = hit.x - fx;
                float dy1 = hit.y - fy1;
                if (dx1 * dx1 + dy1 * dy1 <= dx0 * dx0 + dy0 * dy0) {
                    goto found;
                }
            }
        }
    }
    return 0;

found:
    // The hit must lie within the segment's box, else the walk ends.
    if ((hit.x < p0->x && hit.x < p1->x) || (hit.x > p0->x && hit.x > p1->x)) {
        return 0;
    }
    if ((hit.y < p0->y && hit.y < p1->y) || (hit.y > p0->y && hit.y > p1->y)) {
        return 0;
    }
    if ((hit.z < p0->z && hit.z < p1->z) || (hit.z > p0->z && hit.z > p1->z)) {
        return 0;
    }
    out->x = (float)(gridX << (level * 4)) + hit.x;
    out->y = (float)(gridZ << (level * 4)) + hit.y;
    out->z = hit.z;
    if (outNode) {
        *outNode = this;
    }
    if (outX) {
        *outX = (int)(hit.x + 0.5);
    }
    if (outZ) {
        *outZ = (int)(hit.y + 0.5);
    }
    return 1;
}
#pragma optimize("", on)

// 0x00484d70: the leaf holding grid cell (x, z), or 0 when that branch is
// missing or the leaf is another cell. Written, like GetCellCorners, as a
// tail call into the child: retail's loop is VC6's tail-recursion
// elimination. The child slot is read twice through the index (test, then
// call), which gives retail's eax index and ecx/edx compare registers; with
// a child local VC6 reloads `children` and builds another loop, and the
// explicit walks of the old sample swap the registers (81/90).
GridNode* GridNode::FindLeaf(int x, int z)
{
    if (children) {
        int index = g_gridRow16[(z >> shift) & 0xf] + ((x >> shift) & 0xf);
        if (!children[index])
            return 0;
        return children[index]->FindLeaf(x, z);
    }
    if (gridX != x || gridZ != z)
        return 0;
    return this;
}
