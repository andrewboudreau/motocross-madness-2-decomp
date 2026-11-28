"""VisibilityClipper::TestBox 0x0052f570 (tools/emu_diff.py case).

Boxes against perspective and random matrices and an exact unit clip matrix whose half-unit
boxes put the centre and corners exactly on the clip planes; some straddle the eye plane (w near zero or
negative), where the projected corners leave int and the checked conversion must store
0x80000000 as retail's 32-bit fistp does. The function has no calls; it writes the eight
clip-space corners to 0x0068abc8, the screen rectangle and the inside count.
"""
import math

RETAIL_VA = 0x0052f570
SYMBOL = '?TestBox@VisibilityClipper@@QAEHPAVCamera@@PBM11PAH22@Z'
SOURCE = 'samples/physics/visibility/VisibilityTestBox.cpp'
BINDINGS = {'_s_boxCorners': 0x0068abc8}
PROBES = {0x0052f92a: 'first corner x fistp', 0x0052f9b8: 'corner loop fistp'}


def perspective(rng):
    fov = rng.uniform(0.6, 1.6)
    near, far = rng.uniform(0.1, 2.0), rng.uniform(100, 5000)
    f = 1 / math.tan(fov / 2)
    a = rng.uniform(0.6, 1.0)
    # Row-vector clip space with 0 <= x, y, z <= w (D3D-style, offset into [0, w]).
    m = [0.5 * f * a, 0, 0, 0,
         0, 0.5 * f, 0, 0,
         0.5, 0.5, far / (far - near), 1,
         0, 0, -near * far / (far - near), 0]
    # A random camera transform in front: rotate about y and translate.
    t = rng.uniform(-math.pi, math.pi)
    c, s = math.cos(t), math.sin(t)
    view = [c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0,
            rng.uniform(-50, 50), rng.uniform(-10, 10), rng.uniform(-50, 50), 1]
    return [sum(view[r * 4 + k] * m[k * 4 + col] for k in range(4)) for r in range(4) for col in range(4)]


def cases(rng, n):
    for i in range(n):
        kind = rng.choice(['persp', 'persp', 'persp', 'random', 'eye', 'edge'])
        if kind == 'random':
            m = [rng.uniform(-2, 2) for _ in range(16)]
        elif kind == 'edge':
            # clip space 0 <= x, y, z <= w = 1 exactly, so half-unit centres and extents
            # put the centre and corners on the planes (the < 0.0 tests at equality)
            m = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
        else:
            m = perspective(rng)
        if kind == 'eye':
            # Put the box centre close to the eye plane of the matrix (w ~ 0).
            center = [rng.uniform(-5, 5), rng.uniform(-5, 5), rng.uniform(-5, 5)]
            w = m[3] * center[0] + m[7] * center[1] + m[11] * center[2] + m[15]
            if abs(m[11]) > 1e-3:
                center[2] -= (w - rng.choice([0.0, 1e-6, -1e-6, 1e-3])) / m[11]
        elif kind == 'edge':
            # mostly inside, one coordinate on a plane (the centre trivial-accept tests at
            # equality), sometimes just outside
            center = [rng.choice([0.0, 0.25, 0.5, 1.0, -0.5, 1.5]) for _ in range(3)]
            center[rng.randrange(3)] = rng.choice([0.0, 1.0])
        else:
            center = [rng.uniform(-200, 200), rng.uniform(-30, 30), rng.uniform(-200, 200)]
        extent = [rng.choice([rng.uniform(0, 2), rng.uniform(0, 40), 0.0]) for _ in range(3)]
        if kind == 'edge':
            extent = [rng.choice([0.0, 0.5, 1.0, 0.25]) for _ in range(3)]
        width, height = rng.choice([(640, 480), (800, 600), (512, 384), (rng.randrange(1, 2048), rng.randrange(1, 2048))])
        mode = rng.randrange(4)  # bit 0: screenRect, bit 1: cornersInside
        if kind == 'edge' and rng.random() < 0.5:
            mode = 0              # the centre test runs only without either output
        yield (m, center, extent, width, height, rng.randrange(-50, 50), rng.randrange(-50, 50), mode)


def setup(ctx, case):
    m, center, extent, width, height, ox, oy, mode = case
    camera = ctx.alloc(0x200, fill=0x5a)
    ctx.pack(camera + 0x1a0, 'ii', ox, oy)
    ctx.pack(camera + 0x1a8, 'II', width, height)
    matrix = ctx.alloc(64)
    ctx.pack(matrix, '16f', *m)
    c = ctx.alloc(12)
    ctx.pack(c, '3f', *center)
    e = ctx.alloc(12)
    ctx.pack(e, '3f', *extent)
    rect = ctx.alloc(16, fill=0x77) if mode & 1 else 0
    inside = ctx.alloc(4, fill=0x77) if mode & 2 else 0
    clipper = ctx.alloc(0x40)
    return dict(ecx=clipper, args=[camera, matrix, c, e, rect, inside, 0])
