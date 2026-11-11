"""QuadTree::Insert 0x004dcac0, adding an object to the cells of its code (tools/emu_diff.py case).

A random partial tree: grown nodes (0x24 bytes, the child array inline at +0x14) with some
children missing, ungrown leaves (0x14 bytes), item lists with full and partly free blocks,
and Y intervals that are either the constructor's (0, -FLT_MAX) or random. The code picks a
depth, a cell and one to four of the flag bits, mostly on cells whose paths already exist,
so the walk creates, grows (root and inner nodes) and reuses nodes, and the Y interval
widening runs with y0 < y1, y0 == y1 and y0 > y1. Code 0xf (the early return) is included.

QuadTreeNode::Grow (0x004ddce0) and QuadTreeNode::AddObject (0x004dd130) run as retail code
in both bodies; their allocators are stubs: DebugRealloc (0x004a2ec0) and
BlockAllocator::Alloc (0x00423fc0) hand out fresh heap blocks. The QuadTree's vtable slot 1
(CreateNode) is a stub that allocates a node as the retail factory and constructor
(0x004dc7e0, 0x004dd070) do; retail's factory sets up an SEH frame through fs:[0], which the
emulator does not map. g_pQuadTree (0x00689b78) is retail's.

The module also holds the tree builders tools/emu_cases/quadtree_remove.py imports.
"""
import struct

RETAIL_VA = 0x004dcac0
SYMBOL = '?Insert@QuadTree@@QAEXPAVQuadTreeObject@@IMM@Z'
SOURCE = 'src/krusty2/broadphase/Quadtree.cpp'
PROBES = {0x004dcd05: 'Grow (root)', 0x004dcd11: 'Grow (inner)', 0x004dcc25: 'CreateNode',
          0x004dcc3f: 'Y interval widened', 0x004dccbb: 'AddObject'}

G_QUADTREE = 0x00689b78
NODE_VTABLE = 0x00557648
DEBUG_REALLOC = 0x004a2ec0
POOL_ALLOC = 0x00423fc0
POOL_FREE = 0x00424050
OPERATOR_DELETE = 0x004a30c0
TREE_SIZE = 0x874
LIVE_NODES = 0x854
OBJECT_COUNT = 0x85c
NEG_FLT_MAX = -3.4028234663852886e38
MAX_DEPTH = 5


def _y_interval(rng):
    if rng.random() < 0.3:
        return (0.0, NEG_FLT_MAX)
    return (rng.uniform(-50, 50), rng.choice([0.0, rng.uniform(0, 30)]))


def random_node(rng, level, max_level, n_objects):
    """('grown' or not, children[4] or None, item blocks, (yc, yh))."""
    blocks = []
    for _ in range(rng.choice([0, 0, 1, 1, 2])):
        blocks.append([rng.choice([None, rng.randrange(n_objects)]) if rng.random() < 0.5
                       else rng.randrange(n_objects) for _ in range(4)])
    children = None
    if level < max_level and rng.random() < 0.75:
        children = [random_node(rng, level + 1, max_level, n_objects) if rng.random() < 0.6 else None
                    for _ in range(4)]
    return dict(children=children, blocks=blocks, y=_y_interval(rng))


def path_indices(cx, cz, depth):
    return [(2 if cx & (0x4000 >> level) else 0) + (1 if cz & (0x4000 >> level) else 0)
            for level in range(depth + 1)]


def cell_origins(code):
    depth = code & 0xf
    x = (code >> 17) & 0x7ff8
    z = (code >> 5) & 0x7ff8
    cell = 0x4000 >> depth
    for i in range(4):
        if code & (0x10 << i):
            yield x + (cell if i & 2 else 0), z + (cell if i & 1 else 0)


def make_code(rng, depth):
    cell = 0x4000 >> depth
    # Cells on a coarse grid so that different cases share paths; x + cell may reach 0x8000.
    x = rng.randrange(0, 0x8000 // cell) * cell & 0x7ff8
    z = rng.randrange(0, 0x8000 // cell) * cell & 0x7ff8
    flags = rng.choice([0x10, 0x20, 0x40, 0x80, 0x30, 0x50, 0xf0, rng.randrange(0x10, 0x100, 0x10)])
    return (x << 17) | (z << 5) | flags | depth


def write_tree(ctx, node, objects, tree, vtable=NODE_VTABLE):
    """Writes a node (recursively) and returns its address."""
    grown = node['children'] is not None
    at = ctx.alloc(0x24 if grown else 0x14)
    head = 0
    for block in reversed(node['blocks']):
        link = ctx.alloc(0x14)
        ctx.pack(link, '5I', head, *[objects[o] if o is not None else 0 for o in block])
        head = link
    kids = 0
    if grown:
        kids = at + 0x14
        ptrs = [write_tree(ctx, c, objects, tree, vtable) if c else 0 for c in node['children']]
        ctx.pack(kids, '4I', *ptrs)
    ctx.pack(at, 'III2f', vtable, kids, head, *node['y'])
    count = ctx.unpack(tree + LIVE_NODES, 'i')[0]
    ctx.pack(tree + LIVE_NODES, 'i', count + 1)
    return at


def _create_node(ctx, ecx, args):
    node = ctx.alloc(0x14, fill=0x5a)
    ctx.pack(node, 'III2f', NODE_VTABLE, 0, 0, 0.0, NEG_FLT_MAX)
    tree = ctx.unpack(G_QUADTREE, 'I')[0]
    count = ctx.unpack(tree + LIVE_NODES, 'i')[0]
    ctx.pack(tree + LIVE_NODES, 'i', count + 1)
    return node


def _realloc(ctx, ecx, args):
    old, size = args[0], args[1]
    new = ctx.alloc(size, fill=0x6b)
    ctx.write(new, ctx.read(old, 0x14))
    return new


def _pool_alloc(ctx, ecx, args):
    return ctx.alloc(0x14, fill=0x7c)


def write_quadtree(ctx, root, objects, other_tree=False):
    """The QuadTree object (vtable slot 1 = CreateNode stub), its pools and the tree."""
    tree = ctx.alloc(TREE_SIZE, fill=0)
    create = ctx.stub('CreateNode', ret=_create_node)
    ctx.pack(tree, 'I', ctx.vtable([0, create]))
    ctx.pack(tree + 4, 'i', MAX_DEPTH)
    ctx.pack(tree + 0x10, '12i', *[0x4000 >> d for d in range(12)])
    ctx.pack(tree + 0x60, 'II', ctx.alloc(0x40, fill=0x22), ctx.alloc(0x40, fill=0x33))
    ctx.pack(tree + OBJECT_COUNT, 'i', ctx.rng.randrange(0, 50))
    ctx.pack(tree + 0x5c, 'I', write_tree(ctx, root, objects, tree))
    # g_pQuadTree may still point at another tree; Insert/Remove set it to this one.
    ctx.pack(G_QUADTREE, 'I', ctx.alloc(TREE_SIZE, fill=0) if other_tree else tree)
    ctx.stub_at(DEBUG_REALLOC, 'DebugRealloc', nargs=4, ret=_realloc)
    ctx.stub_at(POOL_ALLOC, 'BlockAllocator::Alloc', ret=_pool_alloc)
    ctx.stub_at(POOL_FREE, 'BlockAllocator::Free', pop=4)
    ctx.stub_at(OPERATOR_DELETE, 'operator delete', nargs=1)
    return tree


def write_objects(ctx, n):
    out = []
    for _ in range(n):
        obj = ctx.alloc(0x10, fill=0x44)
        ctx.pack(obj + 4, 'h', ctx.rng.randrange(-100, 100))
        out.append(obj)
    return out


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def cases(rng, n):
    for _ in range(n):
        n_objects = rng.randrange(1, 9)
        root = random_node(rng, 0, rng.randrange(0, MAX_DEPTH + 1), n_objects)
        if rng.random() < 0.15:
            root['children'] = None
        depth = rng.randrange(0, MAX_DEPTH + 1)
        code = 0xf if rng.random() < 0.03 else make_code(rng, depth)
        y0 = rng.uniform(-80, 80)
        kind = rng.randrange(6)
        if kind == 0:
            y1 = y0
        elif kind == 1:
            y1 = y0 - rng.uniform(0, 10)
        else:
            y1 = y0 + rng.choice([rng.uniform(0, 40), rng.uniform(0, 1e-3)])
        new_object = rng.random() < 0.7
        yield (root, n_objects, code, _f32(y0), _f32(y1), new_object, rng.random() < 0.2)


def setup(ctx, case):
    root, n_objects, code, y0, y1, new_object, other_tree = case
    objects = write_objects(ctx, n_objects)
    tree = write_quadtree(ctx, root, objects, other_tree)
    obj = write_objects(ctx, 1)[0] if new_object else objects[0]
    return dict(args=[obj, code, y0, y1], ecx=tree, ret_eax=False)  # void
