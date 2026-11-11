"""GridNode::IntersectSegment 0x00483d40 (tools/emu_diff.py case).

A segment against a random sparse grid tree: 1 to 3 levels, child tables of 256 slots at a
random density, leaves sharing six 17x17 height blocks (random, ramp or terraced heights,
with per-node relief/height scales and a min/max that is either real or +-1e9). Children
are called through a fake vtable whose slot 1 is ``ctx.entry``, so the recursion stays in
the retail or the candidate body. LinePlaneIntersection (0x004a1300), _control87 and
__ftol are retail's.

Segment ends sit at random points, on grid lines and corners, or on exact half cells (the
midpoint fistp ties); 35% are lattice segments through exact cell corners (diagonals,
knight moves, axis-parallel), some shifted half a cell; others are vertical or horizontal.
Each optional out pointer is left out for some seeds.

The file-static row tables are bound to retail's copies so that both bodies see the same
memory around them. Only round-to-nearest control words are compared by default. Under
round down, up or chop, the midpoint fistp (0x004844aa, 0x00484670) can round a cell index
out of range. Both bodies then index past the row tables and fault on the same input; only
EAX/ESP at the fault differ. On 500 seeds per mode (``--fpcw 0x67f,0xa7f,0xe7f``), the
status, calls and writes agree in every run. Round to nearest can fault too, rarely:
case 1886 at -n 2000 faults in both bodies the same way at each default control word
(1 of 2000 each, also with the pre-pragma object).
"""
import random
import struct

RETAIL_VA = 0x00483d40
SYMBOL = '?IntersectSegment@GridNode@@UAEHPBUGridVec3@@0PAU2@PAPAV1@PAH3@Z'
SOURCE = 'samples/render/GridNodeNearMisses.cpp'
BINDINGS = {'_g_gridRow16': 0x0056c2ac, '_g_gridRow17': 0x0056c2f0}
PROBES = {0x004845f2: 'child call', 0x004a1300: 'LinePlaneIntersection'}
FPCWS = (0x027f, 0x007f, 0x037f)
MAX_INSNS = 5_000_000

DIRECTIONS = [(1, 1), (1, -1), (-1, 1), (-1, -1), (2, 1), (1, 2), (-2, 1), (3, -1), (1, 0), (0, 1)]


def cases(rng, n):
    for i in range(n):
        yield i


def _build(ctx, r):
    vt = ctx.vtable([0, ctx.entry])
    blocks = []
    for _ in range(6):
        kind = r.randrange(3)
        hs = []
        for i in range(17 * 17):
            if kind == 0:
                h = r.randrange(0, 4000)
            elif kind == 1:
                h = 2000 + (i % 17) * 30 + (i // 17) * 10
            else:
                h = r.choice([1000, 1000, 1200, 3000])
            hs.append(h)
        cells = b''.join(struct.pack('<BBHbbbB', r.randrange(256), 0, h, 0, 127, 0, 0) for h in hs)
        blocks.append(ctx.alloc(0xd50, cells + bytes(r.randrange(256) for _ in range(0xd4c - len(cells))) +
                                struct.pack('<i', r.choice([0, 1000, 2000, 5000]))))

    def node(level, shift, children):
        rel = r.choice([1.0, 0.5, 2.0, -1.0, r.uniform(0.2, 3)])
        hsc = r.choice([0.01, 0.05, r.uniform(0.005, 0.1)])
        mn, mx = sorted([r.uniform(-20, 250), r.uniform(-20, 250)])
        if r.random() < 0.5:
            mn, mx = -1e9, 1e9
        return ctx.alloc(0x34, struct.pack('<IIIIffffHHHHBBBBiI', vt, children, 0, r.choice(blocks), rel, hsc,
                                           mn, mx, 0, 0, r.randrange(0x10000), r.randrange(0x10000),
                                           level, shift, 0, 0, 0, 0))

    def kids(pool, density):
        return ctx.alloc(0x400, struct.pack('<256I', *[r.choice(pool) if r.random() < density else 0
                                                       for _ in range(256)]))
    depth = r.choice([1, 2, 2, 3])
    leaves = [node(depth - 1, 0, 0) for _ in range(10)]
    if depth == 1:
        root = leaves[0]
    elif depth == 2:
        root = node(0, 4, kids(leaves, r.choice([0.3, 0.8, 1.0])))
    else:
        mids = [node(1, 4, kids(leaves, r.choice([0.3, 0.8, 1.0]))) for _ in range(4)]
        root = node(0, 8, kids(mids, r.choice([0.3, 0.8, 1.0])))
    return root, 16 << (4 * (depth - 1))


def setup(ctx, seed):
    r = random.Random(seed)
    root, ext = _build(ctx, r)
    scale = ext / 16.0
    pts = []
    for _ in range(2):
        mode = r.randrange(4)
        if mode == 1:      # on grid lines / corners
            p = [r.randrange(0, 17) * scale, r.choice([r.randrange(0, 17) * scale, r.uniform(0, ext)])]
        elif mode == 2:    # half cells (fistp ties)
            p = [(r.randrange(0, 32) + 0.5) * scale / 2, (r.randrange(0, 32) + 0.5) * scale / 2]
        else:
            p = [r.uniform(0, ext), r.uniform(0, ext)]
        if r.random() < 0.5:
            p.reverse()
        pts.append(p + [r.uniform(-30, 260)])
    if r.random() < 0.35:  # lattice segments: exact corner crossings
        a = r.randrange(0, 17) * scale
        b = r.randrange(0, 17) * scale
        da, db = r.choice(DIRECTIONS)
        k = r.randrange(1, 9)
        pts[0][0], pts[0][1] = a, b
        pts[1][0] = min(max(a + da * k * scale, 0.0), ext)
        pts[1][1] = min(max(b + db * k * scale, 0.0), ext)
        if r.random() < 0.3:
            pts[0][0] += scale / 2
            pts[1][0] = min(pts[1][0] + scale / 2, ext)
    elif r.random() < 0.15:
        pts[1][0] = pts[0][0]
    elif r.random() < 0.15:
        pts[1][1] = pts[0][1]
    if pts[0][0] == pts[1][0] and pts[0][1] == pts[1][1]:
        pts[1][0] = ext - pts[1][0] if pts[1][0] != ext / 2 else 0.0
    io = ctx.alloc(48, struct.pack('<6f', *pts[0], *pts[1]) + b'\xee' * 24)
    return dict(args=[io, io + 12, io + 24, io + 36 if seed % 5 else 0, io + 40 if seed % 7 else 0,
                      io + 44 if seed % 3 else 0], ecx=root)
