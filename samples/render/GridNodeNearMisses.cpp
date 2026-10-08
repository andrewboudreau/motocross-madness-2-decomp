// Near-miss GridNode.cpp candidates (src/reconstructed/GridNode.cpp), kept out of
// src/reconstructed until they match.
//
// 0x00484d70 (leaf lookup, 90 bytes): 81/90. Same instructions; retail forms the table
// index in eax (reusing the node register) and compares gridX through ecx and gridZ
// through edx, VC6 here swaps them. Index locals, operand order and the shape of the
// final test do not change it.
//
// 0x00483910 (corner fetch, 1067 bytes): 1068 bytes, 3.4% equal, all 330 instructions
// present in the same order. Retail keeps the x parameter in its argument slot and a
// second x copy (clamped to 0 or size - 1, masked back into x after the clamped
// lookup) in ebp with a local slot at [esp+0x14]; the level byte stays in eax across
// the loop head. VC6 here register-homes x (ebx) and spills the copy and the level
// byte instead, so the walk loop and every fild temp in the corner blocks use other
// stack slots ([esp+0x2c] for [esp+0x28]). Retail also lays out the no-normals corner
// block right after the normals block's `ret` and the failure loop last; VC6 here
// emits the failure loop before the no-normals block whether the branches share one
// `return 1` or carry their own. Tried: a plain `while (node->level)` walk with x
// clamped in place, the copy as the shifted/clamped/masked variable in each of the
// four combinations, separate returns.
#include "../../src/reconstructed/Griddraw.h"

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

// 0x00483910: walks down to the leaf covering (x, z), clamping to the
// nearest child when the coordinates leave the node, then fetches the
// cell's four corners (0: x, z; 1: x + 1; 2: z + 1; 3: both). Heights
// rescale the stored offset from the block's base; normals rescale the
// packed bytes. Returns 0 (flat, surface 0) when no child covers the cell.
int GridNode::UnknownFunction483910(int x, int z, float* heights, GridVec3* normals,
                                    unsigned char* surface)
{
    GridNode* node = this;
    int xc = x;
    unsigned char level = node->level;
    while (level) {
        GridNode** table = node->children;
        if (!table) {
            goto missing;
        }
        int shift = level * 4;
        int size = 1 << shift;
        int mask = ~(-1 << shift);
        int cx = x >> shift;
        int cz = z >> shift;
        GridNode* child;
        if (cx < 16 && cz < 16) {
            child = table[g_gridRow16[cz] + cx];
            if (child) {
                z &= mask;
                x &= mask;
                goto next;
            }
        }
        if (cx < 0) {
            cx = 0;
            xc = 0;
        }
        if (cz < 0) {
            cz = 0;
            z = 0;
        }
        if (cx >= 16) {
            cx = 15;
            xc = size - 1;
        }
        if (cz >= 16) {
            cz = 15;
            z = size - 1;
        }
        child = table[g_gridRow16[cz] + cx];
        if (!child) {
            goto missing;
        }
        z = mask & z;
        x = mask & xc;
    next:
        node = child;
        level = node->level;
        xc = x;
    }
    {
        GridBaseCell* cell = &node->block->cells[g_gridRow17[z] + x];
        if (normals) {
            float scale = node->field_0x10 * (1.0f / 127.0f);
            normals[0].x = cell->normal[0] * scale;
            normals[0].y = cell->normal[1] * (1.0f / 127.0f);
            normals[0].z = cell->normal[2] * scale;
            heights[0] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[0] = cell->field_0x0;
            }
            cell++;
            normals[1].x = cell->normal[0] * scale;
            normals[1].y = cell->normal[1] * (1.0f / 127.0f);
            normals[1].z = cell->normal[2] * scale;
            heights[1] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[1] = cell->field_0x0;
            }
            cell += 17;
            normals[3].x = cell->normal[0] * scale;
            normals[3].y = cell->normal[1] * (1.0f / 127.0f);
            normals[3].z = cell->normal[2] * scale;
            heights[3] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[3] = cell->field_0x0;
            }
            cell--;
            normals[2].x = cell->normal[0] * scale;
            normals[2].y = cell->normal[1] * (1.0f / 127.0f);
            normals[2].z = cell->normal[2] * scale;
            heights[2] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[2] = cell->field_0x0;
            }
        } else {
            heights[0] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[0] = cell->field_0x0;
            }
            cell++;
            heights[1] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[1] = cell->field_0x0;
            }
            cell += 17;
            heights[3] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[3] = cell->field_0x0;
            }
            cell--;
            heights[2] = ((cell->height - node->block->field_0xd4c) * node->field_0x10 +
                          node->block->field_0xd4c) * node->field_0x14;
            if (surface) {
                surface[2] = cell->field_0x0;
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
        surface[i] = 0;
    }
    return 0;
}
