// Near-miss GridNode.cpp candidates (src/reconstructed/GridNode.cpp), kept out of
// src/reconstructed until they match.
//
// 0x00484d70 (leaf lookup) is exact and lives in src/reconstructed/GridNode.cpp:
// a tail call into the child through an index local, the child slot read
// twice (session 18). The ~110 walk spellings recorded before stayed at 81/90.
//
// 0x00483910 (cell sampler) is exact and lives in src/reconstructed/GridNode.cpp.
// It closed (cycle 3) from 1040/1067 with two levers: `level` read through a
// `const unsigned char& lv = level;` (VC6 then reuses the leaf-test byte,
// `and eax, 0xff ; shl eax, 2`; a plain byte local got a stack home, an
// `unsigned char lv` assigned in the test, `GridNode& self = *this` and a
// `const GridNode*` alias were worse) and the order of the shift-derived
// locals: `mask` and `one` before `cx`/`cz` (12 of the 24 orders are exact).
//
// 0x00483d40 (GridNode slot 1, 4140 bytes): strict 68/4140 (masked linked-style 0.812,
// 1349/1387 insns; was 0.602). Readable reconstruction of the decoded control flow
// (grid-line crossings, merge, child recursion, leaf triangles).
// Retail has an ebp frame (`sub esp, 0x43c`) and rounds the cell indices with four
// bare `fistp dword ptr [eax]` (0x4844aa, 0x4844be, 0x484670, 0x484687; the
// `fstp [ebp+8] ; fld [ebp+8] ; mov eax, [ebp+0xc] ; fistp [eax]` pattern of an
// inlined `__asm { fld f ; mov eax, out ; fistp [eax] }` helper) while `__ftol` is
// still called for the other casts (0x483dd2, 0x483f96, 0x484d40, 0x484d57), so
// /QIfist is excluded and the match is out of reach under the no-inline-asm rule.
// GridRoundToInt rounds them with src/bringup/X87FloatToInt.h (fistp's ties-to-even).
// Retail has no range test on the rounded cell indices (only the child loop's
// `== 16 -> 15` clamp, 0x4844c3/0x4844d3).
// Differential emulation against retail (unicorn: random 1- to 3-level grid trees, lattice,
// tie and axis-aligned segments, every x87 rounding and precision mode) finds no behaviour
// difference in 6000 cases; see docs/GAME_BUILD.md (bodies checked by emulation).
//
// Logic fixes from the retail branches (session 25):
// - merge dedupe: after taking an x crossing, retail drops the next y crossing when
//   x OR y is equal (0x4842a5/0x484305 jump on the x compare alone); it was `&&`.
//   The tail arms keep `&& k != 0`.
// - leaf distance tests are `A <= B` (`test ah, 1 ; je found`), not `!(B < A)`, and
//   the height max is a `>` ternary (`GridMax`): both differ only for NaN inputs.
// Shape levers that moved the masked score: the crossing loops split by the sign of
// dx/dy with `step` set by an if/else, member-wise stores with `nx = 1` placed between
// them, i/j/k at function scope, the y midpoint computed before the x rounding,
// `goto found` with the label after the loop's `return 0`, `v0 = Vector3(...)`
// temporaries, nested GridMin/GridMax (retail evaluates Min(h01, h11) first).
// Ruled out: `twoCells` spelled cellSize + cellSize / * 2 / 2.0f (identical); a
// GridCopy(dst, src) pointer helper (one strength-reduced pointer instead of the
// per-member ones retail has); a struct-copy loop (becomes rep movsd); an inline
// rounding helper (VC6 inlines the long X87FloatToInt body at some sites). `/Oy-`
// gives retail-like register allocation (0.884 masked) but only fakes the frame and is
// not adopted. The remaining diffs are the ebp-frame allocation (`this`/p1 reloaded
// from memory, `fld st(0) ; fcomp`), the fistp sites as calls, and the order of the
// final out writes.
#include "../../src/reconstructed/Griddraw.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/bringup/X87FloatToInt.h"
#include <math.h>
#include <stdio.h>
extern "C" void __stdcall OutputDebugStringA(const char*);

static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

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

// 0x00483d40
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
        GridBaseCell* cell = &block->cells[g_gridRow17[iy] + ix];
        float h00 = GridCornerHeight(this, cell);
        float h10 = GridCornerHeight(this, cell + 1);
        float h01 = GridCornerHeight(this, cell + 17);
        float h11 = GridCornerHeight(this, cell + 18);
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
