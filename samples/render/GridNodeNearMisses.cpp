// Near-miss GridNode.cpp candidates (src/reconstructed/GridNode.cpp), kept out of
// src/reconstructed until they match.
//
// 0x00484d70 (leaf lookup, 90 bytes): 81/90. Same instructions; retail forms the table
// index in eax (reusing the node register) and compares gridX through ecx and gridZ
// through edx, VC6 here swaps them. Index locals, operand order and the shape of the
// final test do not change it.
#include "../../src/reconstructed/Griddraw.h"

static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
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
