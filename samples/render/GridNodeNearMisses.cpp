// Near-miss GridNode.cpp candidates (src/reconstructed/GridNode.cpp), kept out of
// src/reconstructed until they match.
//
// 0x00484d70 (leaf lookup, 90 bytes): 81/90. Same instructions; retail forms the table
// index in eax (reusing the node register) and compares gridX through ecx and gridZ
// through edx, VC6 here swaps them. Index locals, operand order, pointer walks, the
// shape of the final test, recursive forms, /G6, /O1 and /Oy- (about thirty
// spellings) do not change it. Nor do 82 more forms: `while (table)`,
// `for (;;) ... break` or `while (node->children)` walks (a child local or
// not), row/col/index/shift locals, `*(table + ...)`, redundant parentheses
// in the index or the test, `&&`/`||`/two-if/ternary tests, swapped or
// z-first comparisons and int casts or locals for gridX/gridZ (77/86 at best).
//
// 0x00483910 (cell sampler) is exact and lives in src/reconstructed/GridNode.cpp.
// It closed (cycle 3) from 1040/1067 with two levers: `level` read through a
// `const unsigned char& lv = level;` (VC6 then reuses the leaf-test byte,
// `and eax, 0xff ; shl eax, 2`; a plain byte local got a stack home, an
// `unsigned char lv` assigned in the test, `GridNode& self = *this` and a
// `const GridNode*` alias were worse) and the order of the shift-derived
// locals: `mask` and `one` before `cx`/`cz` (12 of the 24 orders are exact).
//
// 0x00483d40 (GridNode slot 1, 4140 bytes): 96/4189. Readable reconstruction of the
// decoded control flow (grid-line crossings, merge, child recursion, leaf triangles).
// Retail has an ebp frame (`sub esp, 0x43c`) and rounds the cell indices with four
// bare `fistp dword ptr [eax]` (0x4844aa, 0x4844be, 0x484670, 0x484687; the
// `fstp [ebp+8] ; fld [ebp+8] ; mov eax, [ebp+0xc] ; fistp [eax]` pattern of an
// inlined `__asm { fld f ; mov eax, out ; fistp [eax] }` helper) while `__ftol` is
// still called for the other casts (0x483dd2, 0x483f96, 0x484d40, 0x484d57), so
// /QIfist is excluded and the match is out of reach under the no-inline-asm rule;
// `/Oy-` only fakes the frame (still 96/4189) and the x87 scheduling of the crossing
// loops (running y/z kept on the stack) stays different. Written here as casts.
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
GridNode* GridNode::FindLeaf(int x, int z)
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
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
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
                LinePlaneIntersection(from, to, &w0, &w1, &w2, &hit);
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
            LinePlaneIntersection(from, to, &v0, &v1, &v2, &hit);
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
                LinePlaneIntersection(from, to, &w0, &w1, &w2, &hit);
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
