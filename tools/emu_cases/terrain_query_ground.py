"""Terrain::QueryGround 0x00507c10 (tools/emu_diff.py case).

The ground height, smooth normal and surface kind under a point. Terrain::GetCellCorners
(0x00483910) is replaced in both runs by a deterministic fill keyed on the case seed and the
cell indices; TriangleNormal, Vec3DotCall, FastInvSqrt, _control87 and __ftol are retail's.
The fast-math .bss tables are built by running retail's initializers, and kVec3YAxis
(0x0068a058) is written as (0, 1, 0).

Inputs cover exact half-cell coordinates (fistp ties under every rounding mode), points on
and beside the cell diagonal (which triangle is chosen), NaN/inf, -0, denormals, values at
and beyond the int range, and flat/sloped/random corner normals. A seed with ``seed % 4 ==
1`` gives (0, 1, 0) corner normals, ``0`` gives small integer heights.
"""
import random

RETAIL_VA = 0x00507c10
SYMBOL = '?QueryGround@Terrain@@QAEXPAUTerrainVec3@@0HPAE@Z'
SOURCE = 'src/krusty2/broadphase/Terrain.cpp'
# A file-static with a constructor: the object's copy is zero until its dynamic initializer
# runs, so both bodies read retail's, which setup fills.
BINDINGS = {'_kVec3YAxis': 0x0068a058}
PROBES = {0x00483910: 'GetCellCorners'}
# Nearest (53- and 24-bit precision), then down, up and chop: the cell index is a fistp.
FPCWS = (0x027f, 0x007f, 0x037f, 0x067f, 0x0a7f, 0x0e7f)

GET_CELL_CORNERS = 0x00483910
VEC3_Y_AXIS = 0x0068a058
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)
SPECIALS = [float('nan'), float('inf'), -float('inf'), -0.0, 0.5, 1.5, 2.5, 1e-45, 2147483647.0,
            2147483648.0, -2147483648.0, 4294967296.0, 0.4999999, 65535.5]


def cases(rng, n):
    for i in range(n):
        cell = rng.choice([1.0, 0.5, 2.0, rng.uniform(0.1, 8)])
        inv = rng.choice([1.0 / cell, 1.0, rng.uniform(0.05, 4)])
        kind = rng.randrange(7)
        if kind == 0:
            pos = [rng.uniform(-5, 300) for _ in range(3)]
        elif kind == 1:    # exact half-integer grid coordinates (rounding ties)
            pos = [(rng.randrange(0, 200) + rng.choice([0.5, 1.0, 0.0])) / inv, rng.uniform(-9, 9),
                   (rng.randrange(0, 200) + rng.choice([0.5, 1.0, 0.0])) / inv]
        elif kind == 2:    # on and beside the cell diagonal
            a = rng.uniform(0, 100)
            pos = [a, rng.uniform(-9, 9), a + rng.choice([0, 1e-4, -1e-4, 1, 0.5])]
        elif kind == 3:    # small / near zero
            pos = [rng.uniform(-1, 2), 0.0, rng.uniform(-1, 2)]
        elif kind == 4:    # beyond int
            pos = [rng.choice([3e9, -3e9, 1e20, 2.2e9]), 1.0, rng.uniform(0, 50)]
        elif kind == 5:
            pos = [rng.uniform(0, 1e6), 1.0, rng.uniform(0, 1e6)]
        else:              # NaN/inf, -0, denormal, int-range edges
            pos = [rng.choice(SPECIALS) / inv, rng.uniform(-9, 9),
                   rng.choice(SPECIALS + [rng.uniform(0, 99)]) / inv]
            if rng.random() < 0.5:
                pos[0], pos[2] = pos[2], pos[0]
        yield (i, pos, cell, inv, rng.random() < 0.8, rng.choice([0, 1, 2]), rng.random() < 0.7)


def setup(ctx, case):
    seed, pos, cell, inv, has_normal, flat, has_surface = case
    for init in FAST_TABLE_INITS:
        ctx.call(init)
    ctx.pack(VEC3_Y_AXIS, '3f', 0.0, 1.0, 0.0)

    def get_cell_corners(c, ecx, args):
        ix, iz, heights, normals, kinds = args
        r = random.Random(seed * 1000003 + (ix & 0xffff) * 65537 + (iz & 0xffff))
        mode = seed % 4
        c.pack(heights, '4f', *[r.uniform(-50, 200) if mode else r.choice([0.0, 1.0, 2.5]) for _ in range(4)])
        if normals:
            ns = [r.uniform(-1, 1) for _ in range(12)]
            c.pack(normals, '12f', *([0.0, 1.0, 0.0] * 4 if mode == 1 else ns))
        c.write(kinds, bytes(r.randrange(256) for _ in range(4)))
        return 1
    ctx.stub_at(GET_CELL_CORNERS, 'GetCellCorners', pop=0x14, ret=get_cell_corners)

    terrain = ctx.alloc(0xc40)
    ctx.pack(terrain + 0x40, 'fI', cell, ctx.alloc(0x40))    # cell size, cell data
    ctx.pack(terrain + 0xc30, 'f', inv)                      # inverse cell size
    p = ctx.alloc(12)
    ctx.pack(p, '3f', *pos)
    normal = ctx.alloc(12)
    ctx.pack(normal, '3I', 0x7fc0dead, 0x7fc0dead, 0x7fc0dead)
    surface = ctx.alloc(1, b'\xee')
    return dict(args=[p, normal if has_normal else 0, flat, surface if has_surface else 0],
                ecx=terrain, ret_eax=False)
