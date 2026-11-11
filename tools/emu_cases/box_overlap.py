"""BoxOverlap 0x00424ab0, the 15-axis box/box separating-axis test (tools/emu_diff.py case).

The linked body is the samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp copy of
src/krusty2/bvh/BoundingBoxTreeQuery.cpp. Box A (centre, half extents) is tested against box
B given in A's frame through a matrix (identity, an axis permutation or a random rotation,
random translation). The axis table s_axes is bound to its retail address (as for the tree
cases). B's centre offset is random, and every BISECT_EVERY-th case is slid along a random
direction to the two binary32 offsets where retail's answer changes (bisected against retail
at 53-bit precision), so the comparisons sit on their boundary. The out-of-line dot products
run as retail code; nothing is stubbed.
"""
from tools.emu_cases.segment_triangle_query import STATIC_BINDINGS, matrix, retail_flip, rotation, unit

RETAIL_VA = 0x00424ab0
SYMBOL = '?BoxOverlap@@YAHPBUVec3@@0U1@1PBUMatrix4@@@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
BINDINGS = dict(STATIC_BINDINGS)
BISECT_EVERY = 3


def _case(rng):
    return dict(a_center=[rng.uniform(-5, 5) for _ in range(3)],
                a_half=[rng.uniform(0.1, 4) for _ in range(3)],
                b_center=[rng.uniform(-5, 5) for _ in range(3)],
                b_half=[rng.uniform(0.1, 4) for _ in range(3)],
                rot=rotation(rng), trans=[rng.uniform(-3, 3) for _ in range(3)])


def cases(rng, n):
    for k in range(n):
        c = _case(rng)
        if BISECT_EVERY and k % BISECT_EVERY == 0:
            d = unit([rng.uniform(-1, 1) for _ in range(3)])
            base = list(c['b_center'])

            def make(x, c=c, d=d, base=base):
                return dict(c, b_center=[base[i] + x * d[i] for i in range(3)])
            flip = retail_flip(RETAIL_VA, setup, make, -40.0, 0.0)
            if flip:
                for x in flip:
                    yield make(x)
                continue
        yield c


def setup(ctx, c):
    a_center = ctx.alloc(12)
    ctx.pack(a_center, '3f', *c['a_center'])
    a_half = ctx.alloc(12)
    ctx.pack(a_half, '3f', *c['a_half'])
    m = ctx.alloc(64)
    ctx.pack(m, '16f', *matrix(c['rot'], c['trans']))
    return dict(args=[a_center, a_half, *map(float, c['b_center']), *map(float, c['b_half']), m])
