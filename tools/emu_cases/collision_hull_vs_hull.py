"""CollisionObject::HullVsHull 0x00436720 (tools/emu_diff.py case; helpers shared by the collision cases).

Two oriented-box hulls with random rigid transforms (some with a non-unit scale), random query
vectors and 0..6 stale scratch points (0x00579068, count 0x00579014). The oriented-box tree test
0x00428950 is stubbed: it records the three matrices it receives (contents, not addresses),
then reports a hit in about two thirds of the runs, adding a random contact sum and a count of
1..4 to the query and sometimes new scratch points, as the real test does through the result
pointer. Everything else (the forward to HullVsHullSwept, 0x00432260, 0x0042a450, the vector
constructor) runs as retail code in both bodies. Half of the hulls are swept, so the forward to
the retail swept body is covered too.
"""
import math
import struct

RETAIL_VA = 0x00436720
SYMBOL = '?HullVsHull@CollisionObject@@QAEHPAUCollisionHullBody@@0PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x0043674b: "non-swept body", 0x00436741: "swept forward"}

SCRATCH_COUNT = 0x00579014
SCRATCH_POINTS = 0x00579068
BOX_RESULT = 0x00579058
TREE_TREE_QUERY = 0x00428950
NODE_POSITION = 0x004fc9a0      # SoultreeObject::GetPositionIn (thiscall, 2 args)
HULL_SIZE = 0x198


# ------------------------------------------------------------------ shared helpers

def rigid(rng, scale=False, spread=30.0):
    """Row-vector rigid transform (rotation rows, translation in row 3) as 16 floats."""
    a, b, c = (rng.uniform(-math.pi, math.pi) for _ in range(3))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    r = [[cb * cc, cb * sc, -sb],
         [sa * sb * cc - ca * sc, sa * sb * sc + ca * cc, sa * cb],
         [ca * sb * cc + sa * sc, ca * sb * sc - sa * cc, ca * cb]]
    if scale:
        k = rng.choice([0.5, 2.0, rng.uniform(0.8, 1.2)])
        r = [[x * k for x in row] for row in r]
    t = [rng.uniform(-spread, spread) for _ in range(3)]
    return [r[0][0], r[0][1], r[0][2], 0.0, r[1][0], r[1][1], r[1][2], 0.0,
            r[2][0], r[2][1], r[2][2], 0.0, t[0], t[1], t[2], 1.0]


def near(rng, m, spread=1.0):
    """A transform close to m (the previous frame of a moving body)."""
    small = rigid(rng, spread=spread)
    out = list(m)
    for i in (12, 13, 14):
        out[i] += small[i]
    return out


def vec(rng, s=5.0):
    return [rng.uniform(-s, s) for _ in range(3)]


def hull_desc(rng, swept=None, box=None):
    """Picklable description of one hull body."""
    world = rigid(rng, scale=rng.random() < 0.2)
    body = rigid(rng)
    return dict(swept=rng.randrange(2) if swept is None else swept,
                motion=near(rng, rigid(rng, spread=0.0), 0.5), world=world,
                local=rigid(rng), body=body, prev=near(rng, body),
                center=box[0] if box else vec(rng, 3), half=box[1] if box else [rng.uniform(0.1, 4) for _ in range(3)])


def write_hull(ctx, d, node_vtable=None):
    h = ctx.alloc(HULL_SIZE, fill=0x5c)
    node = ctx.alloc(0x40)
    ctx.pack(node, 'I', node_vtable or 0)
    ctx.pack(h, 'iI', d['swept'], node)
    for off, key in ((0x08, 'motion'), (0x48, 'world'), (0x88, 'local'), (0xc8, 'body'), (0x108, 'prev')):
        ctx.pack(h + off, '16f', *d[key])
    tree = ctx.alloc(0x24)
    ctx.pack(tree, 'i3f3f', 0, *d['center'], *d['half'])
    ctx.pack(h + 0x188, 'III', tree, ctx.alloc(0x40), ctx.alloc(0x100))
    return h


def query_desc(rng):
    return dict(offset=vec(rng), normal=vec(rng, 1), contact=vec(rng, 9), fraction=rng.randrange(100),
                count=rng.randrange(5))


def write_query(ctx, d):
    q = ctx.alloc(0x2c)
    ctx.pack(q, '3f3f3fii', *d['offset'], *d['normal'], *d['contact'], d['fraction'], d['count'])
    ctx.pack(BOX_RESULT, 'I', q)
    return q


def write_scratch(ctx, rng, count=None):
    count = rng.randrange(7) if count is None else count
    ctx.pack(SCRATCH_COUNT, 'i', count)
    for i in range(count):
        ctx.pack(SCRATCH_POINTS + 12 * i, '3f', *vec(rng, 2))


def stub_tree_query(ctx, q, hit_rate=0.66):
    """Stub 0x00428950: log the matrix contents, then maybe hit and fill the query."""
    def tree(c, ecx, a):
        for p in (a[2], a[3], a[6]):
            c.note(c.read(p, 64).hex())
        c.note(a[4])
        if c.rng.random() >= hit_rate:
            return 0
        sx, sy, sz, cnt = c.unpack(q + 0x18, '3fi')[0:3] + c.unpack(q + 0x28, 'i')
        add = vec(c.rng, 4)
        k = c.rng.randrange(1, 5)
        c.pack(q + 0x18, '3f', sx + add[0], sy + add[1], sz + add[2])
        c.pack(q + 0x28, 'i', cnt + k)
        if c.rng.random() < 0.3:
            n = c.unpack(SCRATCH_COUNT, 'i')[0]
            c.pack(SCRATCH_POINTS + 12 * n, '3f', *vec(c.rng, 1))
            c.pack(SCRATCH_COUNT, 'i', n + 1)
        return 1
    return ctx.stub_at(TREE_TREE_QUERY, 'TreeTreeQuery', nargs=7, ret=tree)


def stub_node_position(ctx):
    def pos(c, ecx, a):
        c.pack(a[1], '3f', 1.0, 2.0, 3.0)
        return a[1]
    return ctx.stub_at(NODE_POSITION, 'SoultreeObject::GetPositionIn', pop=8, ret=pos)


# ------------------------------------------------------------------ this case

def cases(rng, n):
    for _ in range(n):
        yield (hull_desc(rng), hull_desc(rng), query_desc(rng), rng.randrange(7))


def setup(ctx, case):
    a, b, qd, scratch = case
    ha, hb = write_hull(ctx, a), write_hull(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    obj = ctx.alloc(0x100)
    return dict(ecx=obj, args=[ha, hb, q])
