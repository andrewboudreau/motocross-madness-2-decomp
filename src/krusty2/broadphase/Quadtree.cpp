// Quadtree.cpp -- reconstruction of D:\aardvark\VC\krusty2\Quadtree.cpp (broad phase).
#include "Quadtree.h"

extern "C" void qsort(void* base, unsigned int num, unsigned int width,
                      int (*compare)(const void*, const void*));   // 0x00534426

QuadTree* g_pQuadTree;
float g_quadTreeInvScale;

QuadTreeNode* QuadTree::UnknownVirtualSlot1()
{
    return new(__FILE__, 0x7a) QuadTreeNode;
}

QuadTreeNode::QuadTreeNode()
{
    field_0x08 = 0;
    field_0x04 = 0;
    g_pQuadTree->field_0x854++;
    field_0x0c = 0.0f;
    field_0x10 = -3.402823466e+38f;
}

QuadTreeNode::~QuadTreeNode()
{
    QuadTreeItemLink* link = field_0x08;
    if (field_0x04) {
        for (int i = 0; i < 4; i++) {
            if (field_0x04[i])
                delete field_0x04[i];
        }
    }
    if (!(g_pQuadTree->field_0x68 & 2)) {
        while (link) {
            QuadTreeItemLink* next = link->next;
            g_pQuadTree->field_0x60->Free(link);
            link = next;
        }
    }
    g_pQuadTree->field_0x854--;
}

QuadTree::~QuadTree()
{
    g_pQuadTree = this;
    field_0x68 |= 2;
    if (field_0x864)
        operator delete(field_0x864, __FILE__, 0x85);
    if (field_0x5c)
        delete field_0x5c;
    delete field_0x60;
    delete field_0x64;
}

void QuadTree::Reset()
{
    g_pQuadTree = this;
    field_0x854 = 0;
    field_0x858 = -1;
    field_0x68 &= ~2;
    field_0x85c = 0;
    field_0x860 = 0;
    field_0x864 = 0;
    field_0x86c = 0;
    field_0x868 = 0;
    field_0x870 = 0;
}

// Tier 3 semantics: field_0x40..0x4c = world rectangle (x0, x1, z0, z1), field_0x50/0x54
// its extents, field_0x0c = 32768 / max extent (quantization scale), field_0x10[] = cell
// size per depth (0x4000 >> depth) until the cell is no larger than minCell in grid units.
void QuadTree::Init(float x0, float z0, float x1, float z1, float minCell)
{
    field_0x5c = UnknownVirtualSlot1();
    field_0x40 = x0;
    field_0x48 = z0;
    field_0x44 = x1;
    field_0x4c = z1;
    field_0x50 = x1 - x0;
    field_0x54 = z1 - z0;

    field_0x0c = 32768.0f / (field_0x50 > field_0x54 ? field_0x50 : field_0x54);
    g_quadTreeInvScale = 1.0f / field_0x0c;

    int limit = (int)(minCell * field_0x0c);
    int cell = 0x8000;
    field_0x04 = 0;
    do {
        cell >>= 1;
        field_0x10[field_0x04] = cell;
        field_0x04++;
    } while (field_0x04 < 12 && cell > limit);
    field_0x04--;
    field_0x08 = field_0x10[field_0x04];

    field_0x68 &= ~1;
    field_0x6c = 0;
    field_0x74 = 0;
    field_0x70 = 0;
    field_0x78 = 0;
    field_0x80 = 0;
    field_0x64 = new(__FILE__, 0x73) QuadTreePool(8, 0x1000);
    field_0x60 = new(__FILE__, 0x74) QuadTreePool(0x14, 0x1000);
}

// Tier 3 name: turns a world rectangle (x0, z0)-(x1, z1) into a "locational code" (0xf =
// outside the tree).  Decoded behavior: coordinates are made relative to the tree origin
// (field_0x40/0x48), rejected if outside [0, extent], clamped, quantized with
// field_0x0c (32768 / extent), and the depth d is the deepest level whose cell size still
// contains the rectangle.  Bits 0..3 = depth, 4..7 = flags (0x10 always, 0x20 if z range
// is not a single cell, 0x40 if x range is not, 0x80 if both).  NOTE the retail clamps
// compare against field_0x40/0x44/0x48/0x4c (the world min/max) even though the values are
// already origin-relative; reproduced as decoded.
unsigned int QuadTree::ComputeCode(float x0, float z0, float x1, float z1)
{
    x0 -= field_0x40;
    z0 -= field_0x48;
    x1 -= field_0x40;
    z1 -= field_0x48;

    if (x1 < 0.0f || x0 > field_0x50 || z1 < 0.0f || z0 > field_0x54)
        return 0xf;

    if (x0 < field_0x40)
        x0 = field_0x40;
    if (x1 > field_0x44)
        x1 = field_0x44;
    if (z0 < field_0x48)
        z0 = field_0x48;
    if (z1 > field_0x4c)
        z1 = field_0x4c;

    int ix0 = (int)(x0 * field_0x0c);
    int iz0 = (int)(z0 * field_0x0c);
    int ix1 = (int)(x1 * field_0x0c) + 1;
    int iz1 = (int)(z1 * field_0x0c) + 1;
    int w = ix1 - ix0;
    int h = iz1 - iz0;

    int depth = 0;
    for (; depth <= field_0x04; depth++) {
        int cell = field_0x10[depth];
        if (w >= cell || h >= cell)
            break;
    }
    if (depth > field_0x04)
        depth = field_0x04;

    int size = field_0x10[depth];
    int mask = ~(size - 1);
    ix1 &= mask;
    ix0 &= mask;
    iz0 &= mask;
    iz1 &= mask;
    if ((ix1 - ix0 > size || iz1 - iz0 > size) && depth != 0) {
        mask = ~(size * 2 - 1);
        depth--;
        ix0 &= mask;
        ix1 &= mask;
        iz0 &= mask;
        iz1 &= mask;
    }

    unsigned int flags = 0x10;
    if (iz1 != iz0)
        flags = 0x30;
    if (ix1 != ix0)
        flags |= 0x40;
    if ((flags & 0x60) == 0x60)
        flags |= 0x80;

    if (ix0 >= 0x8000)
        ix0 = 0x7fff;
    if (iz0 >= 0x8000)
        iz0 = 0x7fff;
    return (((ix0 << 12) | iz0) << 5) | flags | depth;
}

// Tier 3: widens this node's Y interval (field_0x0c = centre, field_0x10 = half extent)
// to include [y0, y1]; skipped when y0 >= y1.  Arithmetic: c = (hi + lo) * 0.5,
// h = (hi - lo) * 0.5 (0x4dcb92..0x4dcbae).
inline void QuadTreeNode::ExtendY(float y0, float y1)
{
    if (y0 < y1) {
        float lo = field_0x0c - field_0x10;
        float hi = field_0x10 + field_0x0c;
        if (y0 < lo)
            lo = y0;
        if (y1 > hi)
            hi = y1;
        field_0x0c = (hi + lo) * 0.5f;
        field_0x10 = (hi - lo) * 0.5f;
    }
}

// Same as QuadTreeNode::ExtendY on the root node; retail reloads the root pointer for the
// last store (0x4dce17), which is reproduced by writing the store through field_0x5c.
inline void QuadTree::ExtendRootY(float y0, float y1)
{
    QuadTreeNode* node = field_0x5c;
    if (y0 < y1) {
        float lo = node->field_0x0c - node->field_0x10;
        float hi = node->field_0x10 + node->field_0x0c;
        if (y0 < lo)
            lo = y0;
        if (y1 > hi)
            hi = y1;
        node->field_0x0c = (hi + lo) * 0.5f;
        field_0x5c->field_0x10 = (hi - lo) * 0.5f;
    }
}

// Tier 3 name.  Inserts obj into every leaf cell selected by the code's flag bits
// (0x10/0x20/0x40/0x80 = the (x,z), (x,z+cell), (x+cell,z), (x+cell,z+cell) cells at the
// code's depth), creating nodes on the way down and widening their Y interval.
void QuadTree::Insert(QuadTreeObject* obj, unsigned int code, float y0, float y1)
{
    if (code == 0xf)
        return;

    field_0x85c++;
    g_pQuadTree = this;
    obj->field_0x04 = 0;

    int depth = code & 0xf;
    int cell = field_0x10[depth];
    int x = (code >> 17) & 0x7ff8;
    int z = (code >> 5) & 0x7ff8;

    unsigned int bit = 0x10;
    for (int i = 0; i < 4; i++, bit <<= 1) {
        if (!(code & bit))
            continue;

        QuadTreeNode* node = field_0x5c;
        QuadTreeNode* parent = 0;
        ExtendRootY(y0, y1);

        int cx = x + ((i & 2) ? cell : 0);
        int cz = z + ((i & 1) ? cell : 0);
        int mask = 0x4000;
        int parentIndex;
        for (int level = depth + 1; level > 0; level--) {
            int index = ((cx & mask) ? 1 : 0) * 2 + ((cz & mask) ? 1 : 0);
            QuadTreeNode* child;
            if (!node->field_0x04) {
                if (node == field_0x5c)
                    field_0x5c = node = node->Grow();
                else
                    node = node->Grow();
                if (parent)
                    parent->field_0x04[parentIndex] = node;
                child = UnknownVirtualSlot1();
                node->field_0x04[index] = child;
            } else {
                child = node->field_0x04[index];
                if (!child) {
                    child = UnknownVirtualSlot1();
                    node->field_0x04[index] = child;
                }
            }
            child->ExtendY(y0, y1);
            parent = node;
            parentIndex = index;
            node = child;
            mask >>= 1;
        }
        node->AddObject(obj);
    }
}

// Tier 3 name.  Re-walks the existing path of every cell selected by `code` and widens
// the Y interval of each node it passes (no nodes are created).  obj (argument 1) is not
// used by the decoded body.
void QuadTree::UpdateRange(QuadTreeObject* obj, unsigned int code, float y0, float y1)
{
    if (code == 0xf)
        return;
    if (y0 > y1)
        return;

    int depth = code & 0xf;
    int x = (code >> 17) & 0x7ff8;
    int z = (code >> 5) & 0x7ff8;
    int cell = field_0x10[depth];

    unsigned int bit = 0x10;
    for (int i = 0; i < 4; i++, bit <<= 1) {
        if (!(code & bit))
            continue;

        QuadTreeNode* node = field_0x5c;
        ExtendRootY(y0, y1);

        int cx = x + ((i & 2) ? cell : 0);
        int cz = z + ((i & 1) ? cell : 0);
        int mask = 0x4000;
        for (int level = 0; level <= depth; level++, mask >>= 1) {
            QuadTreeNode** kids = node->field_0x04;
            if (!kids)
                break;
            int index = ((cx & mask) ? 1 : 0) * 2 + ((cz & mask) ? 1 : 0);
            node = kids[index];
            node->ExtendY(y0, y1);
        }
    }
}

// Tier 3 name.  Removes obj from every cell selected by `code`; a leaf whose object
// list becomes empty (RemoveObject returns 0) is deleted and unlinked from its parent.
void QuadTree::Remove(QuadTreeObject* obj, unsigned int code)
{
    if (code == 0xf)
        return;

    field_0x85c--;
    g_pQuadTree = this;

    int depth = code & 0xf;
    int x = (code >> 17) & 0x7ff8;
    int z = (code >> 5) & 0x7ff8;
    int cell = field_0x10[depth];

    unsigned int bit = 0x10;
    for (int i = 0; i < 4; i++, bit <<= 1) {
        if (!(code & bit))
            continue;

        QuadTreeNode* node = field_0x5c;
        QuadTreeNode* parent = 0;
        int cx = x + ((i & 2) ? cell : 0);
        int cz = z + ((i & 1) ? cell : 0);
        int mask = 0x4000;
        int index;
        bool found = true;
        for (int level = 0; level <= depth; level++, mask >>= 1) {
            index = ((cx & mask) ? 1 : 0) * 2 + ((cz & mask) ? 1 : 0);
            parent = node;
            QuadTreeNode** kids = node->field_0x04;
            if (!kids || !(node = kids[index])) {
                found = false;
                break;
            }
        }
        if (!found)
            continue;

        if (!node->RemoveObject(obj)) {
            if (node)
                delete node;
            if (parent)
                parent->field_0x04[index] = 0;
        }
    }
}

// Tier 3 semantics.  Adds obj to the first free slot of this node's item-block list,
// appending a new zeroed block (pool element 0x14) when every block is full.
void QuadTreeNode::AddObject(QuadTreeObject* obj)
{
    QuadTreeItemLink* prev;
    QuadTreeItemLink* link = field_0x08;
    if (link) {
        do {
            for (int i = 0; i < 4; i++) {
                if (!link->objects[i]) {
                    link->objects[i] = obj;
                    return;
                }
            }
            prev = link;
            link = link->next;
        } while (link);
    } else {
        prev = (QuadTreeItemLink*)obj;
    }

    QuadTreeItemLink* fresh = (QuadTreeItemLink*)g_pQuadTree->field_0x60->Alloc();
    fresh->Clear();
    fresh->objects[0] = obj;
    if (!field_0x08)
        field_0x08 = fresh;
    else
        prev->next = fresh;
}

// Tier 3 semantics.  Clears obj out of the item blocks, freeing blocks that become empty.
// Returns nonzero while the node still holds objects or has a child.
int QuadTreeNode::RemoveObject(QuadTreeObject* obj)
{
    QuadTreeItemLink* prev;
    QuadTreeItemLink* link = field_0x08;
    if (link) {
        do {
            int empty = 1;
            int found = 0;
            for (int i = 0; i < 4; i++) {
                QuadTreeObject* o = link->objects[i];
                if (o == obj) {
                    link->objects[i] = 0;
                    found = 1;
                } else if (o) {
                    empty = 0;
                }
            }
            if (empty) {
                if (link == field_0x08)
                    field_0x08 = link->next;
                else
                    prev->next = link->next;
                g_pQuadTree->field_0x60->Free(link);
            }
            if (found)
                break;
            prev = link;
            link = link->next;
        } while (link);
    }
    if (!field_0x08) {
        QuadTreeNode** kids = field_0x04;
        if (!kids || (!kids[0] && !kids[1] && !kids[2] && !kids[3]))
            return 0;
    }
    return 1;
}

// 0x004ddce0: debug-realloc the node to 0x24 bytes so the four child pointers live inline
// right behind it (tier 2), and clear them.
QuadTreeNode* QuadTreeNode::Grow()
{
    QuadTreeNode* node = (QuadTreeNode*)DebugRealloc(this, 0x24, __FILE__, 0x690);
    node->field_0x04 = (QuadTreeNode**)((char*)node + 0x14);
    node->field_0x04[0] = 0;
    node->field_0x04[1] = 0;
    node->field_0x04[2] = 0;
    node->field_0x04[3] = 0;
    return node;
}

// Tier 3 semantics.  Appends to `tail` the cell of every node overlapping the integer
// rectangle (x0, z0)-(x1, z1); a node whose cell (origin nodeX/nodeZ, half size `size`)
// lies completely inside the rectangle contributes its whole subtree via GatherAll.
// Recursion stops (the node itself is appended) once size < minSize.
QuadTreeResultLink* QuadTreeNode::Gather(QuadTreeResultLink* tail, int x0, int z0, int x1, int z1,
                                         int nodeX, int nodeZ, int size, int minSize)
{
    QuadTreeNode* self = this;
    if (size >= minSize) {
        for (int i = 0; i < 4; i++) {
            if (!field_0x04)
                continue;
            QuadTreeNode* child = field_0x04[i];
            if (!child)
                continue;
            int cx = ((i & 2) ? size : 0) + nodeX;
            int cz = ((i & 1) ? size : 0) + nodeZ;
            if (x0 > cx + size || x1 < cx || z0 > cz + size || z1 < cz)
                continue;
            if (cx >= x0 && cx + size <= x1 && cz >= z0 && cz + size <= z1)
                tail = child->GatherAll(tail);
            else
                tail = child->Gather(tail, x0, z0, x1, z1, cx, cz, size >> 1, minSize);
        }
    }
    QuadTreeResultLink* cell = (QuadTreeResultLink*)g_pQuadTree->field_0x64->Alloc();
    cell->item = self;
    cell->next = tail;
    return cell;
}

QuadTreeResultLink* QuadTreeNode::GatherAll(QuadTreeResultLink* tail)
{
    QuadTreeNode* self = this;
    for (int i = 0; i < 4; i++) {
        if (field_0x04 && field_0x04[i])
            tail = field_0x04[i]->GatherAll(tail);
    }
    QuadTreeResultLink* cell = (QuadTreeResultLink*)g_pQuadTree->field_0x64->Alloc();
    cell->item = self;
    cell->next = tail;
    return cell;
}

// Tier 3 name.  Starts an AABB query over the world rectangle (x0, z0)-(x1, z1): builds the
// list of overlapping nodes in field_0x70 and arms the iteration state.  Returns 0 when the
// rectangle misses the tree or a query is already active (flag bit 0).
int QuadTree::BeginQuery(float x0, float z0, float x1, float z1)
{
    x0 -= field_0x40;
    x1 -= field_0x40;
    z0 -= field_0x48;
    z1 -= field_0x48;
    if (x1 < x0) {
        float t = x1;
        x1 = x0;
        x0 = t;
    }
    if (z1 < z0) {
        float t = z1;
        z1 = z0;
        z0 = t;
    }
    if (x1 < 0.0f || x0 > field_0x50 || z1 < 0.0f || z0 > field_0x54)
        return 0;
    if (field_0x68 & 1)
        return 0;

    field_0x6c++;
    field_0x864 = 0;
    field_0x68 |= 1;
    g_pQuadTree = this;

    int ix0 = (int)(x0 * field_0x0c);
    int iz0 = (int)(z0 * field_0x0c);
    int ix1 = (int)(x1 * field_0x0c);
    int iz1 = (int)(z1 * field_0x0c);
    field_0x74 = 0;
    field_0x7c = 0;
    field_0x78 = 0;
    field_0x80 = 0;
    if (field_0x5c)
        field_0x74 = field_0x5c->Gather(0, ix0, iz0, ix1, iz1, 0, 0, 0x4000, field_0x08);
    field_0x70 = field_0x74;
    return 1;
}

// Tier 3 name.  Iterates the objects of the nodes found by BeginQuery, returning each object
// once (the per-query stamp in QuadTreeObject::field_0x04 is compared with field_0x6c).
QuadTreeObject* QuadTree::NextObject()
{
    if (!(field_0x68 & 1))
        return 0;
    if (field_0x864 != 0)
        return 0;
    g_pQuadTree = this;
    while (field_0x74) {
        if (!field_0x7c) {
            field_0x7c = field_0x74->item->field_0x08;
            if (!field_0x7c) {
                field_0x74 = field_0x74->next;
                continue;
            }
        }
        for (;;) {
            while (field_0x78 < 4) {
                QuadTreeObject* obj = field_0x7c->objects[field_0x78++];
                if (obj && obj->field_0x04 != (short)field_0x6c) {
                    obj->field_0x04 = field_0x6c;
                    return obj;
                }
            }
            field_0x78 = 0;
            field_0x7c = field_0x7c->next;
            if (!field_0x7c)
                break;
        }
        field_0x74 = field_0x74->next;
    }
    return 0;
}

// qsort comparator for NextObjectSorted: orders objects by QuadTreeObject::field_0x06.
static int CompareObjectKey(const void* a, const void* b)
{
    int d = (unsigned short)(*(QuadTreeObject**)a)->field_0x06 -
            (unsigned short)(*(QuadTreeObject**)b)->field_0x06;
    if (d < 0)
        return -1;
    return d != 0;
}

// Tier 3 name.  Like NextObject, but gathers the objects of one node at a time into the
// sorted stack field_0x84 (sorted by field_0x06) and pops them from the back.
QuadTreeObject* QuadTree::NextObjectSorted()
{
    if (!(field_0x68 & 1))
        return 0;
    if (field_0x80)
        return field_0x84[--field_0x80];
    g_pQuadTree = this;
    if (field_0x80 != 0)
        return 0;
    while (field_0x74) {
        if (!field_0x7c) {
            field_0x7c = field_0x74->item->field_0x08;
        }
        if (field_0x7c) {
            do {
                while (field_0x78 < 4) {
                    QuadTreeObject* obj = field_0x7c->objects[field_0x78++];
                    if (obj && obj->field_0x04 != (short)field_0x6c) {
                        obj->field_0x04 = field_0x6c;
                        if (field_0x80 < 500) {
                            field_0x84[field_0x80] = obj;
                            field_0x80++;
                            obj->UnknownVirtualSlot0();
                        }
                    }
                }
                field_0x78 = 0;
                field_0x7c = field_0x7c->next;
            } while (field_0x7c);
            int n = field_0x80;
            if (n < 0 || n > 1) {
                if (n == 2) {
                    QuadTreeObject* a = field_0x84[0];
                    QuadTreeObject* b = field_0x84[1];
                    if ((unsigned short)a->field_0x06 > (unsigned short)b->field_0x06) {
                        field_0x84[0] = b;
                        field_0x84[1] = a;
                    }
                } else {
                    qsort(field_0x84, n, 4, CompareObjectKey);
                }
            }
        }
        field_0x74 = field_0x74->next;
        if (field_0x80)
            return field_0x84[--field_0x80];
    }
    return 0;
}

void QuadTree::RestartQuery()
{
    field_0x74 = field_0x70;
    field_0x78 = 0;
    field_0x6c++;
    field_0x80 = 0;
}

void QuadTree::EndQuery()
{
    if (field_0x68 & 1) {
        field_0x64->Reset();
        field_0x70 = 0;
        field_0x74 = 0;
        field_0x78 = 0;
        field_0x80 = 0;
        field_0x68 &= ~1;
    }
}

// Tier 3 name.  Appends the not-yet-stamped objects of an item-block chain to the result
// array field_0x864 (capacity field_0x870, count field_0x86c).
void QuadTree::CollectItems(QuadTreeItemLink* link)
{
    for (; link; link = link->next) {
        QuadTreeObject** slot = link->objects;
        for (int i = 0; i < 4; i++) {
            if (field_0x86c >= field_0x870)
                return;
            QuadTreeObject* obj = *slot++;
            if (obj && obj->field_0x04 != (short)field_0x6c) {
                obj->field_0x04 = field_0x6c;
                field_0x864[field_0x86c] = obj;
                field_0x86c++;
            }
        }
    }
}

// Tier 3 name: a code is valid when its depth is the deepest level and exactly one flag bit
// (0x10, 0x20, 0x40 or 0x80) is set.
int QuadTree::IsValidCode(unsigned int code)
{
    int depth = code & 0xf;
    int flags = code & 0xf0;
    if (depth == field_0x04 && (flags == 0x10 || flags == 0x20 || flags == 0x40 || flags == 0x80))
        return 1;
    return 0;
}

// Slots 1/2: back-to-front walk relative to the point (x0, z0).  Child k sits at
// (nodeX or nodeX+size, nodeZ or nodeZ+size): index = 2 * xBit + zBit (same as Insert).
// The child that contains the point is visited last (tier 2, decoded from the four
// unrolled orders 0-1-2-3 / 1-0-3-2 / 2-3-0-1 / 3-2-1-0).
void QuadTreeNode::UnknownVirtualSlot1(int x0, int z0, int nodeX, int nodeZ, int size)
{
    int midX = nodeX + size;
    int midZ = nodeZ + size;
    size >>= 1;
    if (field_0x04) {
        if (x0 >= midX) {
            if (z0 >= midZ) {
                if (field_0x04[0]) field_0x04[0]->UnknownVirtualSlot1(x0, z0, nodeX, nodeZ, size);
                if (field_0x04[1]) field_0x04[1]->UnknownVirtualSlot1(x0, z0, nodeX, midZ, size);
                if (field_0x04[2]) field_0x04[2]->UnknownVirtualSlot1(x0, z0, midX, nodeZ, size);
                if (field_0x04[3]) field_0x04[3]->UnknownVirtualSlot1(x0, z0, midX, midZ, size);
            } else {
                if (field_0x04[1]) field_0x04[1]->UnknownVirtualSlot1(x0, z0, nodeX, midZ, size);
                if (field_0x04[0]) field_0x04[0]->UnknownVirtualSlot1(x0, z0, nodeX, nodeZ, size);
                if (field_0x04[3]) field_0x04[3]->UnknownVirtualSlot1(x0, z0, midX, midZ, size);
                if (field_0x04[2]) field_0x04[2]->UnknownVirtualSlot1(x0, z0, midX, nodeZ, size);
            }
        } else if (z0 >= midZ) {
            if (field_0x04[2]) field_0x04[2]->UnknownVirtualSlot1(x0, z0, midX, nodeZ, size);
            if (field_0x04[3]) field_0x04[3]->UnknownVirtualSlot1(x0, z0, midX, midZ, size);
            if (field_0x04[0]) field_0x04[0]->UnknownVirtualSlot1(x0, z0, nodeX, nodeZ, size);
            if (field_0x04[1]) field_0x04[1]->UnknownVirtualSlot1(x0, z0, nodeX, midZ, size);
        } else {
            if (field_0x04[3]) field_0x04[3]->UnknownVirtualSlot1(x0, z0, midX, midZ, size);
            if (field_0x04[2]) field_0x04[2]->UnknownVirtualSlot1(x0, z0, midX, nodeZ, size);
            if (field_0x04[1]) field_0x04[1]->UnknownVirtualSlot1(x0, z0, nodeX, midZ, size);
            if (field_0x04[0]) field_0x04[0]->UnknownVirtualSlot1(x0, z0, nodeX, nodeZ, size);
        }
    }
    g_pQuadTree->CollectItems(field_0x08);
}

QuadTreeResultLink* QuadTreeNode::UnknownVirtualSlot2(int x0, int z0, QuadTreeResultLink* tail,
                                                      int nodeX, int nodeZ, int size)
{
    int midX = nodeX + size;
    int midZ = nodeZ + size;
    size >>= 1;
    if (field_0x04) {
        if (x0 >= midX) {
            if (z0 >= midZ) {
                if (field_0x04[0]) tail = field_0x04[0]->UnknownVirtualSlot2(x0, z0, tail, nodeX, nodeZ, size);
                if (field_0x04[1]) tail = field_0x04[1]->UnknownVirtualSlot2(x0, z0, tail, nodeX, midZ, size);
                if (field_0x04[2]) tail = field_0x04[2]->UnknownVirtualSlot2(x0, z0, tail, midX, nodeZ, size);
                if (field_0x04[3]) tail = field_0x04[3]->UnknownVirtualSlot2(x0, z0, tail, midX, midZ, size);
            } else {
                if (field_0x04[1]) tail = field_0x04[1]->UnknownVirtualSlot2(x0, z0, tail, nodeX, midZ, size);
                if (field_0x04[0]) tail = field_0x04[0]->UnknownVirtualSlot2(x0, z0, tail, nodeX, nodeZ, size);
                if (field_0x04[3]) tail = field_0x04[3]->UnknownVirtualSlot2(x0, z0, tail, midX, midZ, size);
                if (field_0x04[2]) tail = field_0x04[2]->UnknownVirtualSlot2(x0, z0, tail, midX, nodeZ, size);
            }
        } else if (z0 >= midZ) {
            if (field_0x04[2]) tail = field_0x04[2]->UnknownVirtualSlot2(x0, z0, tail, midX, nodeZ, size);
            if (field_0x04[3]) tail = field_0x04[3]->UnknownVirtualSlot2(x0, z0, tail, midX, midZ, size);
            if (field_0x04[0]) tail = field_0x04[0]->UnknownVirtualSlot2(x0, z0, tail, nodeX, nodeZ, size);
            if (field_0x04[1]) tail = field_0x04[1]->UnknownVirtualSlot2(x0, z0, tail, nodeX, midZ, size);
        } else {
            if (field_0x04[3]) tail = field_0x04[3]->UnknownVirtualSlot2(x0, z0, tail, midX, midZ, size);
            if (field_0x04[2]) tail = field_0x04[2]->UnknownVirtualSlot2(x0, z0, tail, midX, nodeZ, size);
            if (field_0x04[1]) tail = field_0x04[1]->UnknownVirtualSlot2(x0, z0, tail, nodeX, midZ, size);
            if (field_0x04[0]) tail = field_0x04[0]->UnknownVirtualSlot2(x0, z0, tail, nodeX, nodeZ, size);
        }
    }
    if (field_0x08) {
        QuadTreeResultLink* cell = (QuadTreeResultLink*)g_pQuadTree->field_0x64->Alloc();
        if (tail)
            tail->next = cell;
        cell->next = 0;
        cell->item = this;
        return cell;
    }
    return tail;
}
