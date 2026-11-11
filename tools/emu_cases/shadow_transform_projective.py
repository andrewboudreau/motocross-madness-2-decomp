"""ShadowTransformPointsProjective 0x004a1a50, strided projective transform (tools/emu_diff.py case).

Linked from src/bringup/Group5_ShadowBridges.cpp. Each point is divided by its w row
(fdivr by 1.0) and the three coordinate rows are scaled by that reciprocal, all on the x87
stack; retail sums each row in its own order (w: (z y) x + t; x: (y z) x + t; y and z:
(y x) z + t). Cases: random projective matrices (w row near zero in some), random points,
strides 12, 16 and 0x20, a zero source stride (destination stride reused), counts 0..17.
"""
RETAIL_VA = 0x004a1a50
SYMBOL = '?ShadowTransformPointsProjective@@YAXPAUShadowVec3@@PBXPBUShadowMatrix@@HHH@Z'
SOURCE = 'src/bringup/Group5_ShadowBridges.cpp'


def cases(rng, n):
    for _ in range(n):
        m = [rng.uniform(-3, 3) for _ in range(16)]
        if rng.random() < 0.3:
            m[12:15] = [rng.uniform(-1e3, 1e3) for _ in range(3)]
        if rng.random() < 0.3:
            m[3], m[7], m[11], m[15] = 0.0, 0.0, 0.0, 1.0     # affine
        count = rng.choice([0, 1, 2, 5, 17])
        dstride = rng.choice([12, 16, 0x20])
        sstride = rng.choice([0, 12, 16, 0x20, dstride])
        pts = [rng.uniform(-500, 500) for _ in range(3 * max(count, 1))]
        yield (m, count, dstride, sstride, pts)


def setup(ctx, case):
    m, count, dstride, sstride, pts = case
    matrix = ctx.alloc(64)
    ctx.pack(matrix, '16f', *m)
    stride = sstride or dstride
    src = ctx.alloc(stride * max(count, 1) + 16, fill=0x5a)
    for i in range(max(count, 1)):
        ctx.pack(src + i * stride, '3f', *pts[3 * i:3 * i + 3])
    dst = ctx.alloc(dstride * max(count, 1) + 16, fill=0x77)
    return dict(args=[dst, src, matrix, count, dstride, sstride], ret_eax=False)
