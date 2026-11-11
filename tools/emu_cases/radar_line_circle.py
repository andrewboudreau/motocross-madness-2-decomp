"""RadarOverlay::IntersectLineCircle 0x0051c4f0, the radar's line/circle clip (tools/emu_diff.py case).

Lines through random points against circles they miss, touch (the tangent band
|disc| < 1e-14) and cross, plus degenerate lines (both ends equal). Of two hits retail keeps
the one nearer the fourth argument (the caller's unclamped map position) and writes it
through the fifth, which starts filled with 0x77; the earlier bring-up measured against the
fifth (an uninitialised buffer in DrawTrackOutline), and this case caught it. LineThrough
(0x0051c460) and FastSqrt run as retail code in both bodies. Coordinates use the radar's
scale (tens to thousands of units).
"""
import math

RETAIL_VA = 0x0051c4f0
SYMBOL = '?IntersectLineCircle@RadarOverlay@@QAEHPBM00PAM1@Z'
SOURCE = 'src/bringup/Group3_RadarOverlay.cpp'
PROBES = {0x0051c614: 'tangent', 0x0051c642: 'two hits'}


def cases(rng, n):
    for _ in range(n):
        s = rng.choice([1.0, 50.0, 2000.0])
        a = [rng.uniform(-s, s), rng.uniform(-s, s)]
        b = [rng.uniform(-s, s), rng.uniform(-s, s)]
        kind = rng.randrange(5)
        if kind == 0:
            b = list(a)
        cx, cy = rng.uniform(-s, s), rng.uniform(-s, s)
        r = rng.uniform(0, s)
        if kind in (1, 2):
            # Circle centred near the line: crossing, or tangent when r is the distance.
            t = rng.uniform(-1, 2)
            px, py = a[0] + t * (b[0] - a[0]), a[1] + t * (b[1] - a[1])
            dx, dy = b[0] - a[0], b[1] - a[1]
            ln = math.hypot(dx, dy) or 1.0
            off = rng.uniform(0, s / 2)
            cx, cy = px - dy / ln * off, py + dx / ln * off
            r = off if kind == 2 else off * rng.uniform(1.0, 3.0)
        elif kind == 3:
            a = [float(rng.randrange(-5, 6)), float(rng.randrange(-5, 6))]
            b = [a[0] + rng.choice([0.0, 1.0, 3.0]), a[1] + rng.choice([0.0, 2.0, -1.0])]
            cx, cy, r = float(rng.randrange(-5, 6)), float(rng.randrange(-5, 6)), float(rng.randrange(0, 5))
        reference = rng.choice([list(b), [rng.uniform(-s, s), rng.uniform(-s, s)]])
        yield (a, b, (cx, cy, r), reference)


def setup(ctx, case):
    a, b, circle, reference = case
    pa = ctx.alloc(8)
    ctx.pack(pa, '2f', *a)
    pb = ctx.alloc(8)
    ctx.pack(pb, '2f', *b)
    pc = ctx.alloc(12)
    ctx.pack(pc, '3f', *circle)
    pr = ctx.alloc(8)
    ctx.pack(pr, '2f', *reference)
    out = ctx.alloc(8, fill=0x77)
    this = ctx.alloc(0x100)
    return dict(ecx=this, args=[pa, pb, pc, pr, out])
