"""SoultreeObject::SetMatrixIn 0x004fb8c0 (tools/emu_diff.py case).

A small node tree (grandparent -> parent -> this -> two children, plus a separate frame node)
with random rigid and general matrices and random worldValid flags. The frame is this node,
none, the parent or the separate node; the parent link is present or not. UpdateWorldMatrix,
InvalidateWorldMatrix, SetLocalMatrix and the matrix products run as retail code in both
bodies, so the case compares the stored local matrix, the in-place world-space *m, the world
matrices the helpers refresh and the invalidation flags.
"""
import math

RETAIL_VA = 0x004fb8c0
SYMBOL = '?SetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z'
SOURCE = 'samples/physics/helpers/SoultreeMatrix.cpp'
PROBES = {0x004fb8e4: 'frame == this', 0x004fb907: 'frame world', 0x004fbc7f: 'parent branch'}

NODE = 0x1a4
LOCAL, WORLD, VALID, PARENT, CHILD, NEXT, PREV = 0xb8, 0xf8, 0x138, 0x13c, 0x140, 0x144, 0x148


def rigid(rng):
    """A random rotation (row-vector convention) with a translation."""
    a, b, c = (rng.uniform(-math.pi, math.pi) for _ in range(3))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    rz = [[ca, sa, 0], [-sa, ca, 0], [0, 0, 1]]
    ry = [[cb, 0, -sb], [0, 1, 0], [sb, 0, cb]]
    rx = [[1, 0, 0], [0, cc, sc], [0, -sc, cc]]

    def mul(p, q):
        return [[sum(p[i][k] * q[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
    r = mul(mul(rz, ry), rx)
    t = [rng.uniform(-100, 100) for _ in range(3)]
    return [r[0][0], r[0][1], r[0][2], 0.0, r[1][0], r[1][1], r[1][2], 0.0,
            r[2][0], r[2][1], r[2][2], 0.0, t[0], t[1], t[2], 1.0]


def matrix(rng):
    kind = rng.randrange(4)
    if kind == 0:
        return [rng.uniform(-3, 3) for _ in range(16)]
    if kind == 1:
        m = rigid(rng)
        s = rng.uniform(0.2, 4)
        return [x * s if i < 12 else x for i, x in enumerate(m)]
    return rigid(rng)


def tree_case(rng):
    return dict(mats=[(matrix(rng), matrix(rng)) for _ in range(6)],
                valid=[rng.randrange(2) for _ in range(6)],
                has_parent=rng.random() < 0.75, has_grand=rng.random() < 0.5,
                frame=rng.choice(['self', 'none', 'parent', 'other', 'other']))


def build_tree(ctx, case):
    """Nodes: 0 grandparent, 1 parent, 2 this, 3 and 4 children, 5 separate frame."""
    nodes = [ctx.alloc(NODE, fill=0) for _ in range(6)]
    for i, n in enumerate(nodes):
        local, world = case['mats'][i]
        ctx.pack(n + LOCAL, '16f', *local)
        ctx.pack(n + WORLD, '16f', *world)
        ctx.pack(n + VALID, 'i', case['valid'][i])
    g, p, me, c1, c2, other = nodes
    if case['has_grand']:
        ctx.pack(p + PARENT, 'I', g)
        ctx.pack(g + CHILD, 'I', p)
    if case['has_parent']:
        ctx.pack(me + PARENT, 'I', p)
        ctx.pack(p + CHILD, 'I', me)
    ctx.pack(me + CHILD, 'I', c1)
    ctx.pack(c1 + PARENT, 'I', me)
    ctx.pack(c2 + PARENT, 'I', me)
    ctx.pack(c1 + NEXT, 'I', c2)
    ctx.pack(c2 + PREV, 'I', c1)
    frame = {'self': me, 'none': 0, 'parent': p if case['has_parent'] else 0, 'other': other}[case['frame']]
    return me, frame


def cases(rng, n):
    for _ in range(n):
        c = tree_case(rng)
        c['m'] = matrix(rng)
        yield c


def setup(ctx, case):
    me, frame = build_tree(ctx, case)
    m = ctx.alloc(64)
    ctx.pack(m, '16f', *case['m'])
    return dict(ecx=me, args=[frame, m], ret_eax=False)
