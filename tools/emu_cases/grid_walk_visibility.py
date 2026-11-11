"""DrawableGridNode::WalkVisibilityQuadrants 0x00481de0 (tools/emu_diff.py case).

Linked from samples/render/GriddrawNearMisses.cpp. A root node (layout from
src/reconstructed/Griddraw.h: children +0x04, parent +0x08, terrain +0x34, data +0x38,
childMask +0x40, centre +0x44, extent +0x50; data flags b0/b1 at +0x121) with a 16 x 16
child table at a random density. Children carry a random childMask (0, full 0x1fffff or
0x3fffff, or garbage), a parent and, for some, their own sparse child table, so the
`child->WalkVisibilityQuadrants(..., 16, 0, 0, ...)` recursion runs (it calls the body under
test, retail or candidate). Entry is the root walk (size 16, depth 0) or a quadrant walk
(size 8/4, depth 1/2, random path).

Stubbed, with arguments and pointed-to boxes noted: VisibilityClipper::TestBox 0x0052f570
(thiscall on g_visibilityClipper 0x00575a98; returns visible from the run's random stream
and writes 8 or another corner count), UpdateLevelOfDetail 0x004824c0 (sets or clears the
node's data b0) and the shared empty 0x00464e90. g_gridBoxTests is retail's global.
"""
import random
import struct

RETAIL_VA = 0x00481de0
SYMBOL = '?WalkVisibilityQuadrants@DrawableGridNode@@QAEHPAX0PAM1HHHHHH@Z'
SOURCE = 'samples/render/GriddrawNearMisses.cpp'
CLIPPER = 0x00575a98
PROBES = {0x00481e76: 'size16 LOD', 0x0048202e: 'quadrant recursion', 0x004821ba: 'all-corners LOD',
          0x00482271: 'child walk', 0x00482311: 'leaf TestBox', 0x00482366: 'leaf release',
          0x004823f8: 'invisible release', 0x00482474: 'tail release'}
MAX_INSNS = 5_000_000


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), density=rng.choice([0.05, 0.3, 1.0]),
                   visible=rng.choice([0.2, 0.6, 0.9]), corners8=rng.choice([0.0, 0.2, 0.5]),
                   lod=rng.choice([0.0, 0.5, 1.0]), entry=rng.choice(['root'] * 4 + ['quad8', 'quad4']),
                   coarse=rng.randrange(2), mask=rng.choice([0, 1, 0x1fffff, 0x3fffff, 0x12345]))


def mask(r):
    return r.choice([0, 0, 0x1fffff, 0x3fffff, r.randrange(1 << 22)])


def setup(ctx, c):
    r = random.Random(c['seed'])
    terrain = ctx.alloc(0x100, fill=0x3c)
    renderer = ctx.alloc(0x20, fill=0x3d)
    ctx.pack(terrain + 0x18, 'I', renderer)
    ctx.pack(renderer + 0x08, 'I', 0x0ca3e7a0)

    def data_block():
        d = ctx.alloc(0x180, fill=0)
        ctx.pack(d + 0x121, 'B', r.randrange(256))
        return d

    def node_bytes(children, parent, m, center, extent):
        b = bytearray(b'\x4e' * 0x60)
        struct.pack_into('<III', b, 4, children, parent, 0)
        struct.pack_into('<III', b, 0x34, terrain, data_block(), 0)
        struct.pack_into('<i3f3f', b, 0x40, m, *center, *extent)
        return b

    def vec():
        return [r.uniform(-500, 500) for _ in range(3)]

    def ext():
        return [r.uniform(1, 200) for _ in range(3)]

    root = ctx.alloc(0x60)
    slots = [0] * 256
    for i in range(256):
        if r.random() < c['density']:
            grand = 0
            if r.random() < 0.08:
                gslots = [0] * 256
                child_at = ctx.heap        # allocated right below; parent of grandchildren
                for k in range(256):
                    if r.random() < 0.03:
                        gslots[k] = ctx.alloc(0x60, node_bytes(0, 0, mask(r), vec(), ext()))
                grand = ctx.alloc(0x400, struct.pack('<256I', *gslots))
            child = ctx.alloc(0x60, node_bytes(grand, root, mask(r), vec(), ext()))
            if grand:
                for k in range(256):
                    g = ctx.unpack(grand + 4 * k, 'I')[0]
                    if g:
                        ctx.pack(g + 8, 'I', child)
            slots[i] = child
    table = ctx.alloc(0x400, struct.pack('<256I', *slots))
    parent = ctx.alloc(0x60, node_bytes(0, 0, mask(r), vec(), ext()))
    ctx.write(root, node_bytes(table, parent if r.random() < 0.5 else 0, c['mask'], vec(), ext()))

    def test_box(cx, ecx, a):         # (camera, matrix, center, extent, 0, int* corners, int* other)
        cx.note('TestBox', cx.read(a[2], 12).hex(), cx.read(a[3], 12).hex())
        visible = 1 if cx.rng.random() < c['visible'] else 0
        for p in (a[5], a[6]):
            if p:
                cx.pack(p, 'i', 8 if cx.rng.random() < c['corners8'] else cx.rng.choice([0, 3, 7]))
        return visible

    def lod(cx, ecx, a):              # (stream, a1, flag, coarse)
        d = cx.unpack(ecx + 0x38, 'I')[0]
        flags = cx.unpack(d + 0x121, 'B')[0]
        flags = (flags | 1) if cx.rng.random() < c['lod'] else (flags & ~1)
        cx.pack(d + 0x121, 'B', flags & 0xff)
        return 0

    ctx.stub_at(0x0052f570, 'TestBox', pop=28, ret=test_box)
    ctx.stub_at(0x004824c0, 'UpdateLevelOfDetail', pop=16, ret=lod)
    ctx.stub_at(0x00464e90, 'Release464e90')
    clipper = ctx.alloc(0x40, fill=0x11)
    ctx.pack(CLIPPER, 'I', clipper)

    center = ctx.alloc(12)
    extent = ctx.alloc(12)
    ctx.pack(center, '3f', *vec())
    ctx.pack(extent, '3f', *ext())
    stream, a1 = 0x5e000000, ctx.alloc(64, fill=0x3f)
    if c['entry'] == 'root':
        x = z = 0
        size, depth, path = 16, 0, 0
    else:
        size = 8 if c['entry'] == 'quad8' else 4
        depth = 1 if size == 8 else 2
        x, z = r.randrange(0, 17 - size, size), r.randrange(0, 17 - size, size)
        path = r.randrange(4) if depth == 1 else r.randrange(16)
    return dict(ecx=root, args=[stream, a1, center, extent, x, z, size, depth, path, c['coarse']])
