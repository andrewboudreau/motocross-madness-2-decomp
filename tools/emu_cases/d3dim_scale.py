"""D3DIMSoultreeObject::Scale 0x004fd340, local matrix times a scale (tools/emu_diff.py case).

Random, rotation-like and integer local matrices (+0xb8) scaled by uniform, per-axis,
zero, negative and tiny factors; the whole object is compared after the call.
InvalidateWorldMatrix (0x004fb850) is stubbed at its retail address and logged.
"""
import math

RETAIL_VA = 0x004fd340
SYMBOL = '?Scale@D3DIMSoultreeObject@@QAEXMMM@Z'
SOURCE = 'samples/render/D3DIMScale.cpp'


def cases(rng, n):
    for _ in range(n):
        kind = rng.randrange(3)
        if kind == 0:
            m = [rng.uniform(-100, 100) for _ in range(16)]
        elif kind == 1:
            a, b = rng.uniform(-math.pi, math.pi), rng.uniform(-math.pi, math.pi)
            ca, sa, cb, sb = math.cos(a), math.sin(a), math.cos(b), math.sin(b)
            m = [ca, 0, -sa, 0, sa * sb, cb, ca * sb, 0, sa * cb, -sb, ca * cb, 0,
                 rng.uniform(-500, 500), rng.uniform(-50, 50), rng.uniform(-500, 500), 1]
        else:
            m = [float(rng.randrange(-4, 5)) for _ in range(16)]
        s = rng.choice([1.0, 0.5, 2.0, 0.0, -1.0, 1e-3, rng.uniform(0.01, 10)])
        scale = rng.choice([(s, s, s), (rng.uniform(-3, 3), rng.uniform(0, 3), rng.uniform(-3, 3)), (1.0, 1.0, 1.0)])
        yield (m, scale)


def setup(ctx, case):
    m, scale = case
    ctx.stub_at(0x004fb850, 'SoultreeObject::InvalidateWorldMatrix', pop=0)
    this = ctx.alloc(0x200)
    ctx.pack(this + 0xb8, '16f', *m)
    return dict(ecx=this, args=[float(v) for v in scale], ret_eax=False)
