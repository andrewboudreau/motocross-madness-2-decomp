"""Tire::UpdateContactPatch 0x005135f0 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/tire/Tire.cpp (the VehicleWheel shim jumps to it).
The whole 0x2c0-byte tire is random floats; the contact point (CollisionPoint at +0xb8) gets a
unit or random surface normal (+0xe4), the roll direction (+0x230) is unit or random and
reportContactOutputs (+0x264) is set in most runs. minLength straddles the 0.001 threshold,
`ref` is random, along or against the normal (so the rejected tangent is short or zero and
the d <= 0.001 arm runs) or zero. The reject (0x0043b190), normalise (0x005087b0), dot,
square-root and divide helpers run as retail code.
"""
import math

RETAIL_VA = 0x005135f0
SYMBOL = '?UpdateContactPatch@Tire@@QAEXPBUCollisionVec3@@U2@1MPAMPAH@Z'
SOURCE = 'src/krusty2/tire/Tire.cpp'
PROBES = {0x00513735: 'reject', 0x0051374b: 'normalise', 0x005139a3: 'slip return',
          0x00513ae5: 'short d', 0x00513c41: 'short axis'}


def vec(rng, s):
    return [rng.uniform(-s, s) for _ in range(3)]


def unit(rng):
    v = vec(rng, 1)
    l = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / l for x in v]


def cases(rng, n):
    for _ in range(n):
        s = rng.choice([0.5, 3.0, 20.0])
        normal = rng.choice([unit(rng), unit(rng), [0.0, 1.0, 0.0], vec(rng, 2)])
        kind = rng.randrange(5)
        if kind == 0:
            ref = [x * rng.uniform(-3, 3) for x in normal]            # parallel: zero tangent
        elif kind == 1:
            ref = [0.0, 0.0, 0.0]
        else:
            ref = vec(rng, s)
        yield dict(seed=rng.randrange(1 << 30), s=s, normal=normal, ref=ref, axis=vec(rng, s),
                   pos=vec(rng, s), roll=rng.choice([unit(rng), vec(rng, 1)]),
                   min_length=rng.choice([0.0, 0.001, 0.0011, rng.uniform(0, 2)]),
                   report=rng.choice([0, 1, 1, 1]))


def setup(ctx, c):
    import random
    import struct
    r = random.Random(c['seed'])
    tire = ctx.alloc(0x2c0)
    ctx.write(tire, struct.pack('<176f', *[r.uniform(-c['s'], c['s']) for _ in range(176)]))
    ctx.pack(tire + 0xe4, '3f', *c['normal'])
    ctx.pack(tire + 0x230, '3f', *c['roll'])
    ctx.pack(tire + 0x264, 'i', c['report'])
    pos = ctx.alloc(12)
    ctx.pack(pos, '3f', *c['pos'])
    out_value = ctx.alloc(4, fill=0x77)
    out_sign = ctx.alloc(4, fill=0x77)
    return dict(ecx=tire, args=[pos, *map(float, c['axis']), *map(float, c['ref']),
                                float(c['min_length']), out_value, out_sign], ret_eax=False)
