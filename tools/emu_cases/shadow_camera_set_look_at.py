"""ShadowCameraProxy::SetLookAt 0x0042e9b0, eye/forward/up/fov setter (tools/emu_diff.py case).

Retail normalises each direction with lengthSq = (x*x + y*y) + z*z, compares it unrounded with
1.0 (copy when equal) and otherwise scales by FastInvSqrt (0x00460c00, run as retail code via
the binding). Cases: every argument independently null or set, unit axes and near-unit vectors
(the exact-1.0 branch and its neighbours), random magnitudes, and a zero fov (no lens store,
counter bump only).
"""
RETAIL_VA = 0x0042e9b0
SYMBOL = '?SetLookAt@ShadowCameraProxy@@QAEXPBUShadowVec3@@HHHPAM@Z'
SOURCE = 'src/bringup/Group2_ShadowCameraLookAt.cpp'

UNIT = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, -1.0), (0.6, 0.8, 0.0), (0.0, 0.28, 0.96)]


def _vec(rng):
    kind = rng.random()
    if kind < 0.25:
        return rng.choice(UNIT)
    if kind < 0.4:
        x, y, z = rng.choice(UNIT)
        e = rng.uniform(-1e-6, 1e-6)
        return (x + e, y - e, z + e)
    s = rng.choice([1e-3, 1.0, 50.0, 4e3])
    return tuple(rng.uniform(-s, s) for _ in range(3))


def cases(rng, n):
    for _ in range(n):
        eye = tuple(rng.uniform(-1e3, 1e3) for _ in range(3)) if rng.random() < 0.7 else None
        a = _vec(rng) if rng.random() < 0.8 else None
        b = _vec(rng) if rng.random() < 0.8 else None
        c = rng.getrandbits(32) if rng.random() < 0.5 else None
        fov = rng.choice([0.0, rng.uniform(0.2, 3.0)]) if rng.random() < 0.6 else None
        yield (eye, a, b, c, fov, rng.getrandbits(31))


def setup(ctx, case):
    eye, a, b, c, fov, counter = case
    context = ctx.alloc(0x20)
    ctx.pack(context + 0x14, 'I', counter)
    camera = ctx.alloc(0x200, fill=0x3c)
    ctx.pack(camera + 0x18, 'I', context)

    def vec_arg(v):
        if v is None:
            return 0
        p = ctx.alloc(12)
        ctx.pack(p, '3f', *v)
        return p

    pc = 0
    if c is not None:
        pc = ctx.alloc(4)
        ctx.pack(pc, 'I', c)
    pf = 0
    if fov is not None:
        pf = ctx.alloc(4)
        ctx.pack(pf, 'f', fov)
    return dict(ecx=camera, args=[vec_arg(eye), vec_arg(a), vec_arg(b), pc, pf], ret_eax=False)
