"""VisibilityQuadTreeNode::Traverse 0x0052d610, the back-to-front frustum walk (tools/emu_diff.py case).

Random partial trees (tools/emu_cases/quadtree_insert.py builders; grown nodes with missing
children, item lists present or not, Y intervals random or the constructor's -FLT_MAX half
extent) walked from the root (origin 0, half size 0x4000) with the query cell in each of the
four quadrant orders and on the midlines. g_quadTreeInvScale (0x00689b74) scales the tree to
a world of about 30 to 3300 units.

VisibilityClipper::TestBox (0x0052f570) is a stub that tests the eight corners of the box it
is handed against a per-case set of random planes, as the clipper does with the camera's
planes: it returns 0 when every corner is outside one plane, else 1, and stores the number of
corners inside all planes through its cornersInside argument (8 = fully inside). It notes the
box centre and extent, so every float the walk computes is compared. The nodes' vtable slot 2
(GatherNodesBackToFront, taken for fully inside children) is a logged stub that links one
fresh cell; BlockAllocator::Alloc (0x00423fc0) is a stub. The recursion runs the candidate in
the candidate run (the entry symbol is always the object's own).
"""
import struct

from tools.emu_cases.quadtree_insert import (G_QUADTREE, POOL_ALLOC, TREE_SIZE, random_node, write_objects,
                                             write_tree)
from tools.emu_cases.segment_triangle_query import dot, unit

RETAIL_VA = 0x0052d610
SYMBOL = '?Traverse@VisibilityQuadTreeNode@@QAEPAUQuadTreeResultLink@@HHPAU2@HHHPAUVisibilityCamera@@@Z'
SOURCE = 'samples/physics/visibility/VisibilityQuadTreeTraversal.cpp'
PROBES = {0x0052f570: 'TestBox', 0x0052d718: 'GatherNodes (first arm)', 0x0052d73b: 'recursion (first arm)',
          0x0052e19c: 'recursion (last arm)', 0x0052e1b9: 'item cell'}

INV_SCALE = 0x00689b74
CLIPPER = 0x00575a98
TEST_BOX = 0x0052f570


def cases(rng, n):
    for _ in range(n):
        n_objects = rng.randrange(1, 4)
        root = random_node(rng, 0, rng.randrange(1, 6), n_objects)
        if root['children'] is None:
            root['children'] = [random_node(rng, 1, 4, n_objects) for _ in range(4)]
        scale = rng.choice([1000 / 32768, 0.01, 0.1, 0.001])
        world = 0x8000 * scale
        planes = []
        for _ in range(rng.randrange(1, 6)):
            nrm = unit([rng.uniform(-1, 1), rng.uniform(-0.3, 0.3), rng.uniform(-1, 1)])
            p = [rng.uniform(0, world), rng.uniform(-20, 20), rng.uniform(0, world)]
            planes.append((nrm, -dot(nrm, p)))
        x = rng.choice([rng.randrange(0, 0x8000), 0x4000, rng.choice([0x2000, 0x6000]) + rng.choice([0, -1])])
        z = rng.choice([rng.randrange(0, 0x8000), 0x4000, rng.choice([0x2000, 0x6000]) + rng.choice([0, -1])])
        yield (root, n_objects, scale, planes, x, z, rng.random() < 0.5)


def _test_box(planes):
    def ret(ctx, ecx, args):
        center = ctx.unpack(args[2], '3f')
        extent = ctx.unpack(args[3], '3f')
        ctx.note('box', center, extent, args[4], args[6])
        inside = 0
        outside = [0] * len(planes)
        for k in range(8):
            corner = [center[i] + (extent[i] if k >> i & 1 else -extent[i]) for i in range(3)]
            ok = True
            for j, (nrm, d) in enumerate(planes):
                if dot(nrm, corner) + d < 0:
                    outside[j] += 1
                    ok = False
            inside += ok
        ctx.pack(args[5], 'i', inside)
        return 0 if 8 in outside else 1
    return ret


def _gather(ctx, ecx, args):
    tail = args[2]
    cell = ctx.alloc(8)
    ctx.pack(cell, 'II', 0, ecx)
    if tail:
        ctx.pack(tail, 'I', cell)
    return cell


def _pool_alloc(ctx, ecx, args):
    return ctx.alloc(8, fill=0x7c)


def setup(ctx, case):
    root, n_objects, scale, planes, x, z, with_tail = case
    objects = write_objects(ctx, n_objects)
    tree = ctx.alloc(TREE_SIZE)
    ctx.pack(tree + 0x64, 'I', ctx.alloc(0x40, fill=0x33))
    ctx.pack(G_QUADTREE, 'I', tree)
    gather = ctx.stub('GatherNodesBackToFront', pop=0x18, ret=_gather)
    vtable = ctx.vtable([0, 0, gather])
    node = write_tree(ctx, root, objects, tree, vtable)
    ctx.pack(INV_SCALE, 'f', struct.unpack('<f', struct.pack('<f', scale))[0])
    ctx.pack(CLIPPER, 'I', ctx.alloc(0x40, fill=0x55))
    ctx.stub_at(TEST_BOX, 'TestBox', pop=0x1c, ret=_test_box(planes))
    ctx.stub_at(POOL_ALLOC, 'BlockAllocator::Alloc', ret=_pool_alloc)
    camera = ctx.alloc(0x200, fill=0x66)
    tail = 0
    if with_tail:
        tail = ctx.alloc(8)
        ctx.pack(tail, 'II', 0, 0)
    return dict(args=[x, z, tail, 0, 0, 0x4000, camera], ecx=node)
