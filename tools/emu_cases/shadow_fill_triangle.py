"""ShadowFillTriangle 0x00461e60, the shadow-map triangle rasteriser (tools/emu_diff.py case).

Triangles inside a (1 << shift)-square 16-bit buffer: random, flat-top, flat-bottom,
single-row, single-pixel and thin slivers, in both windings, with and without the corner
plot. The buffer starts filled with 0x77 bytes and is compared whole. Retail returns 1 in
eax, which the void bring-up drops and no caller reads, so eax is not compared.
"""
import struct

RETAIL_VA = 0x00461e60
SYMBOL = '?ShadowFillTriangle@@YAXHPAFFPAHH@Z'
SOURCE = 'src/bringup/Group5_ShadowBridges.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        shift = rng.choice([3, 4, 5, 6])
        size = 1 << shift
        kind = rng.randrange(6)
        pts = [[rng.randrange(size), rng.randrange(size)] for _ in range(3)]
        if kind == 1:      # flat top or bottom
            pts[1][1] = pts[0][1]
        elif kind == 2:    # single row
            pts[1][1] = pts[2][1] = pts[0][1]
        elif kind == 3:    # single pixel
            pts = [list(pts[0]) for _ in range(3)]
        elif kind == 4:    # sliver: two corners within a pixel
            pts[1] = [min(size - 1, pts[0][0] + rng.randrange(2)), min(size - 1, pts[0][1] + rng.randrange(2))]
        value = rng.choice([0, -1, 0x1234, rng.randrange(-0x8000, 0x8000)])
        yield (shift, pts, value, rng.randrange(2))


def setup(ctx, case):
    shift, pts, value, plot = case
    size = 1 << shift
    buffer = ctx.alloc(2 * size * size + 64, fill=0x77)
    points = ctx.alloc(24)
    ctx.pack(points, '6i', *(c for p in pts for c in p))
    return dict(args=[shift, buffer + 32, value, points, plot], ret_eax=False)
