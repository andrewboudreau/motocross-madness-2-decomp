// Near-miss GridNode.cpp candidates (src/reconstructed/GridNode.cpp), kept out of
// src/reconstructed until they match.
//
// 0x00484d70 (leaf lookup, 90 bytes): 81/90. Same instructions; retail forms the table
// index in eax (reusing the node register) and compares gridX through ecx and gridZ
// through edx, VC6 here swaps them. Index locals, operand order and the shape of the
// final test do not change it.
//
// 0x00483910 (cell sampler, 1067 bytes): same operations and layout (descent loop as
// VC6's tail-recursion elimination of the child call, the two leaf paths, the default
// fill). Retail keeps x and z in their argument homes across the loop (stores at the
// loop bottom, reload in the fast path) and a copy of x in ebp/[esp+0x14]; VC6 here
// keeps them in ebx/ebp, so the frame is 0xc instead of 0x14 and every register
// differs. Loop forms, a `level` local, `x &= mask` before the call and a single call
// site do not change it.
//
// 0x00483d40 (GridNode slot 1, 4140 bytes): readable reconstruction of the decoded
// control flow (grid-line crossings, merge, child recursion, leaf triangles). Retail
// rounds the cell indices with a bare `fistp` (the `__asm fld/fistp` helper this
// project leaves out), written here as a cast; the x87 scheduling of the crossing
// loops (running y/z kept on the stack) and the frame are not reproduced.
#include "../../src/reconstructed/Griddraw.h"
#include "../../src/reconstructed/MatrixUtil.h"

static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

// 0x00484d70
GridNode* GridNode::UnknownFunction484d70(int x, int z)
{
    GridNode* node = this;
    GridNode** table = children;
    while (table) {
        node = table[g_gridRow16[(z >> node->shift) & 0xf] + ((x >> node->shift) & 0xf)];
        if (!node) {
            return 0;
        }
        table = node->children;
    }
    if (node->gridX != x || node->gridZ != z) {
        return 0;
    }
    return node;
}

// 0x00483910. The block's cells hold the height relative to the block's
// base (+0xd4c) and a signed-byte normal scaled by 1/127 (0x00553ec8, the
// float right after the GridNode vtable). Retail's loop is VC6's
// tail-recursion elimination of the child call.
int GridNode::UnknownFunction483910(int x, int z, float* heights, GridVec3* normals, unsigned char* bytes)
{
    if (level != 0) {
        if (children != 0) {
            int shift = level * 4;
            int cx = x >> shift;
            int cz = z >> shift;
            int mask = ~(-1 << shift);
            int one = 1 << shift;
            GridNode* child;
            if (cx >= 16 || cz >= 16 || (child = children[g_gridRow16[cz] + cx]) == 0) {
                if (cx < 0) {
                    cx = 0;
                    x = 0;
                }
                if (cz < 0) {
                    cz = 0;
                    z = 0;
                }
                if (cx >= 16) {
                    cx = 15;
                    x = one - 1;
                }
                if (cz >= 16) {
                    cz = 15;
                    z = one - 1;
                }
                child = children[g_gridRow16[cz] + cx];
                if (child == 0) {
                    goto missing;
                }
            }
            return child->UnknownFunction483910(x & mask, z & mask, heights, normals, bytes);
        }
    } else {
        GridBaseCell* cell = &block->cells[g_gridRow17[z] + x];
        if (normals) {
            float scale = field_0x10 * (1.0f / 127.0f);
            normals[0].x = cell->normal[0] * scale;
            normals[0].y = cell->normal[1] * (1.0f / 127.0f);
            normals[0].z = cell->normal[2] * scale;
            heights[0] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[0] = cell->field_0x0;
            }
            cell++;
            normals[1].x = cell->normal[0] * scale;
            normals[1].y = cell->normal[1] * (1.0f / 127.0f);
            normals[1].z = cell->normal[2] * scale;
            heights[1] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[1] = cell->field_0x0;
            }
            cell += 17;
            normals[3].x = cell->normal[0] * scale;
            normals[3].y = cell->normal[1] * (1.0f / 127.0f);
            normals[3].z = cell->normal[2] * scale;
            heights[3] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[3] = cell->field_0x0;
            }
            cell--;
            normals[2].x = cell->normal[0] * scale;
            normals[2].y = cell->normal[1] * (1.0f / 127.0f);
            normals[2].z = cell->normal[2] * scale;
            heights[2] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[2] = cell->field_0x0;
            }
        } else {
            heights[0] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[0] = cell->field_0x0;
            }
            cell++;
            heights[1] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[1] = cell->field_0x0;
            }
            cell += 17;
            heights[3] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[3] = cell->field_0x0;
            }
            cell--;
            heights[2] = ((cell->height - block->field_0xd4c) * field_0x10 + block->field_0xd4c) * field_0x14;
            if (bytes) {
                bytes[2] = cell->field_0x0;
            }
        }
        return 1;
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
    return ((cell->height - node->block->field_0xd4c) * node->field_0x10 + node->block->field_0xd4c) * node->field_0x14;
}

// Cell index of a coordinate midway between two crossings: retail converts
// (a + b) / (2 * cellSize) - 0.5 with a bare fistp (round to nearest).
static inline int GridMidCell(float a, float b, float twoCells)
{
    return (int)((a + b) / twoCells - 0.5f);
}

// Crossing of the segment with one grid-line family: `coord` is the stepped
// coordinate, the other two follow it linearly.
static inline int GridCrossingAccepted(float coord, float delta, float end)
{
    if (delta > 0.0f && coord <= end) {
        return 1;
    }
    return delta < 0.0f && coord >= end;
}

// 0x00483d40
int GridNode::UnknownVirtualSlot1(const GridVec3* p0, const GridVec3* p1, GridVec3* out, GridNode** outNode,
                                  int* outX, int* outZ)
{
    GridVec3 cross[34];
    GridVec3 xCross[17];
    GridVec3 yCross[17];
    float dx = p1->x - p0->x;
    float dy = p1->y - p0->y;
    float dz = p1->z - p0->z;
    float cellSize = (float)(1 << shift);
    int nx = 0;
    int ny = 0;
    if (dx != 0.0f) {
        float step = dx < 0.0f ? -cellSize : cellSize;
        float zPerX = dz / dx;
        float yPerX = dy / dx;
        float y = p0->y;
        float z = p0->z;
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
            if (GridCrossingAccepted(x, dx, p1->x)) {
                float t = x - p0->x;
                y += t * yPerX;
                z += t * zPerX;
                xCross[0] = GridVec3(x, y, z);
                nx = 1;
            }
        }
        for (x += step; GridCrossingAccepted(x, dx, p1->x); x += step) {
            y += yStep;
            z += zStep;
            xCross[nx++] = GridVec3(x, y, z);
        }
    }
    if (dy != 0.0f) {
        float step = dy < 0.0f ? -cellSize : cellSize;
        float zPerY = dz / dy;
        float xPerY = dx / dy;
        float x = p0->x;
        float z = p0->z;
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
            if (GridCrossingAccepted(y, dy, p1->y)) {
                float t = y - p0->y;
                x += t * xPerY;
                z += t * zPerY;
                yCross[0] = GridVec3(x, y, z);
                ny = 1;
            }
        }
        for (y += step; GridCrossingAccepted(y, dy, p1->y); y += step) {
            x += xStep;
            z += zStep;
            yCross[ny++] = GridVec3(x, y, z);
        }
    }
    // Merge the two crossing lists along the segment, dropping shared corners.
    int count = nx + ny;
    int k = 0;
    if ((xCross[0].x != p0->x || xCross[0].y != p0->y) && (yCross[0].x != p0->x || yCross[0].y != p0->y)) {
        cross[0] = *p0;
        k = 1;
        count++;
    }
    if (dx == 0.0f) {
        for (int j = 0; k < count; k++, j++) {
            cross[k] = yCross[j];
        }
    } else {
        int i = 0;
        int j = 0;
        for (; k < count; k++) {
            if (i < nx) {
                if (j < ny) {
                    if ((dx > 0.0f && xCross[i].x < yCross[j].x) || (dx < 0.0f && xCross[i].x > yCross[j].x)) {
                        cross[k] = xCross[i++];
                        if (cross[k].x == yCross[j].x && cross[k].y == yCross[j].y) {
                            j++;
                            count--;
                        }
                    } else {
                        cross[k] = yCross[j++];
                        if (cross[k].x == xCross[i].x && cross[k].y == xCross[i].y) {
                            i++;
                            count--;
                        }
                    }
                } else {
                    if (k != 0 && cross[k - 1].x == xCross[i].x && cross[k - 1].y == xCross[i].y) {
                        count--;
                    } else {
                        cross[k] = xCross[i];
                    }
                    i++;
                }
            } else {
                if (k != 0 && cross[k - 1].x == yCross[j].x && cross[k - 1].y == yCross[j].y) {
                    count--;
                } else {
                    cross[k] = yCross[j];
                }
                j++;
            }
        }
    }
    if (cross[count - 1].x != p1->x || cross[count - 1].y != p1->y) {
        cross[count++] = *p1;
    }
    if (children) {
        float twoCells = cellSize + cellSize;
        for (k = 0; k < count - 1; k++) {
            int ix = GridMidCell(cross[k].x, cross[k + 1].x, twoCells);
            int iy = GridMidCell(cross[k].y, cross[k + 1].y, twoCells);
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
            if (cross[k].z < child->field_0x18 && cross[k + 1].z < child->field_0x18) {
                continue;
            }
            if (cross[k].z > child->field_0x1c && cross[k + 1].z > child->field_0x1c) {
                continue;
            }
            float ox = ix * cellSize;
            float oy = iy * cellSize;
            GridVec3 a(cross[k].x - ox, cross[k].y - oy, cross[k].z);
            GridVec3 b(cross[k + 1].x - ox, cross[k + 1].y - oy, cross[k + 1].z);
            if (child->UnknownVirtualSlot1(&a, &b, out, outNode, outX, outZ)) {
                return 1;
            }
        }
        return 0;
    }
    float twoCells = cellSize + cellSize;
    for (k = 0; k < count - 1; k++) {
        int ix = GridMidCell(cross[k].x, cross[k + 1].x, twoCells);
        int iy = GridMidCell(cross[k].y, cross[k + 1].y, twoCells);
        GridBaseCell* cell = &block->cells[g_gridRow17[iy] + ix];
        float h00 = GridCornerHeight(this, cell);
        float h10 = GridCornerHeight(this, cell + 1);
        float h01 = GridCornerHeight(this, cell + 17);
        float h11 = GridCornerHeight(this, cell + 18);
        float lo = h01 < h11 ? h01 : h11;
        float hmin = h00 < h10 ? h00 : h10;
        if (!(hmin < lo)) {
            hmin = lo;
        }
        float hi = h01 <= h11 ? h11 : h01;
        float hmax = h00 <= h10 ? h10 : h00;
        if (!(hmax > hi)) {
            hmax = hi;
        }
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
        Vector3 hit;
        const Vector3* from = (const Vector3*)&cross[k];
        const Vector3* to = (const Vector3*)&cross[k + 1];
        int found = 0;
        if ((ix ^ iy) & 1) {
            Vector3 v0(fx, fy, h00);
            Vector3 v1(fx1, fy, h10);
            Vector3 v2(fx, fy1, h01);
            UnknownFunction4a1300(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float a = hit.x - fx;
                float b = hit.y - fy;
                float c = hit.x - fx1;
                float d = hit.y - fy1;
                if (!(c * c + d * d < a * a + b * b)) {
                    found = 1;
                }
            }
            if (!found) {
                Vector3 w0(fx1, fy, h10);
                Vector3 w1(fx1, fy1, h11);
                Vector3 w2(fx, fy1, h01);
                UnknownFunction4a1300(from, to, &w0, &w1, &w2, &hit);
                if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                    float a = hit.x - fx;
                    float b = hit.y - fy;
                    float c = hit.x - fx1;
                    float d = hit.y - fy1;
                    if (!(a * a + b * b < c * c + d * d)) {
                        found = 1;
                    }
                }
            }
        } else {
            Vector3 v0(fx, fy, h00);
            Vector3 v1(fx1, fy, h10);
            Vector3 v2(fx1, fy1, h11);
            UnknownFunction4a1300(from, to, &v0, &v1, &v2, &hit);
            if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                float a = hit.x - fx1;
                float b = hit.y - fy;
                float c = hit.x - fx;
                float d = hit.y - fy1;
                if (!(c * c + d * d < a * a + b * b)) {
                    found = 1;
                }
            }
            if (!found) {
                Vector3 w0(fx, fy, h00);
                Vector3 w1(fx1, fy1, h11);
                Vector3 w2(fx, fy1, h01);
                UnknownFunction4a1300(from, to, &w0, &w1, &w2, &hit);
                if (hit.x <= fx1 && hit.x >= fx && hit.y <= fy1 && hit.y >= fy) {
                    float a = hit.x - fx1;
                    float b = hit.y - fy;
                    float c = hit.x - fx;
                    float d = hit.y - fy1;
                    if (!(a * a + b * b < c * c + d * d)) {
                        found = 1;
                    }
                }
            }
        }
        if (!found) {
            continue;
        }
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
        out->z = hit.z;
        out->y = (float)(gridZ << (level * 4)) + hit.y;
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
    return 0;
}
