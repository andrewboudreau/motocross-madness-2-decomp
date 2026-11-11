"""TransformPoints 0x004a1b00, strided point transform (tools/emu_diff.py case).

Retail is hand-written x87 code: each coordinate sums ((x*m1 + y*m2) + z*m3) + t on the FPU
stack and rounds once at the store. Cases: random affine matrices and points, strides 12, 16
and 0x20 on either side, a zero source stride (the destination stride is reused), zero
strides on both sides (one point), and in-place transforms (destination == source).
"""
RETAIL_VA = 0x004a1b00
SYMBOL = '?TransformPoints@@YAXPBXPAXPBUMatrix4@@HHH@Z'
SOURCE = 'src/bringup/Group0_Math.cpp'


def cases(rng, n):
    for _ in range(n):
        m = [rng.uniform(-3, 3) for _ in range(16)]
        if rng.random() < 0.3:
            m[12:15] = [rng.uniform(-1e4, 1e4) for _ in range(3)]
        count = rng.choice([0, 1, 2, 5, 17])
        dstride = rng.choice([12, 16, 0x20])
        sstride = rng.choice([0, 12, 16, 0x20, dstride])
        mode = rng.choice(['separate', 'separate', 'inplace', 'zero'])
        if mode == 'inplace':
            sstride = dstride
        if mode == 'zero':
            dstride = sstride = 0
            count = max(count, 1)
        pts = [rng.uniform(-500, 500) for _ in range(3 * max(count, 1))]
        yield (m, count, dstride, sstride, mode, pts)


def setup(ctx, case):
    m, count, dstride, sstride, mode, pts = case
    matrix = ctx.alloc(64)
    ctx.pack(matrix, '16f', *m)
    stride = sstride or dstride or 12
    src = ctx.alloc(stride * max(count, 1) + 16, fill=0x5a)
    for i in range(max(count, 1)):
        ctx.pack(src + i * stride, '3f', *pts[3 * i:3 * i + 3])
    if mode == 'inplace':
        dst = src
    else:
        dst = ctx.alloc((dstride or 12) * max(count, 1) + 16, fill=0x77)
    return dict(args=[dst, src, matrix, count, dstride, sstride], ret_eax=False)
