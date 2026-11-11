"""CollisionRelativeFrame 0x00432260 (tools/emu_diff.py case).

m1 expressed in two rigid frames m2 and m3, then the transform between the two results. The
matrix product 0x00436500 and the relative transform 0x00432180 run as retail code. Frames are
rigid (some scaled, and some m3 close to m2 as a body's previous frame); the output is the
64-byte matrix.
"""
from tools.emu_cases.collision_hull_vs_hull import near, rigid

RETAIL_VA = 0x00432260
SYMBOL = '?CollisionRelativeFrame@@YAXPAUMatrix4@@PBU1@11@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'


def cases(rng, n):
    for _ in range(n):
        m2 = rigid(rng, scale=rng.random() < 0.2)
        m3 = near(rng, m2) if rng.random() < 0.5 else rigid(rng)
        yield (rigid(rng, scale=rng.random() < 0.2), m2, m3)


def setup(ctx, case):
    ptrs = []
    for m in case:
        p = ctx.alloc(64)
        ctx.pack(p, '16f', *m)
        ptrs.append(p)
    out = ctx.alloc(64, fill=0x5c)
    return dict(args=[out] + ptrs)
