"""Vehicle::UpdateLeanFromNormal 0x005286a0 (tools/emu_diff.py case).

Offsets from the retail body (tier 1): +0x544 wheel count, +0x4ac/+0x4b0 lean angle and
cosine, +0x48/+0x3c the fallback roll values, +0x1ac/+0x1b8 the two scratch vectors, +0x4cc
the side axis. GetAverageGroundNormal (0x00528400) is stubbed: it writes a case-chosen
normal and returns its argument. The dot, scale, subtract, normalise and inverse square root
helpers and asin run as retail code. The lean angle goes through asin and the hardware fcos,
so a value rounded to binary32 before the cosine would show here. Normals parallel to the
forward axis take the zero-perpendicular fallback; unit-length crosses take the `== 1` arms.
"""
import math

RETAIL_VA = 0x005286a0
SYMBOL = '?UpdateLeanFromNormal@Vehicle@@UAEXPAUVec3@@H0@Z'
SOURCE = 'src/krusty2/vehicle/Vehicle.cpp'
PROBES = {0x005286dc: 'average normal', 0x0052898a: 'zero perpendicular', 0x00528857: 'side normalise',
          0x0052893f: 'sqrt arm', 0x00528979: 'cos'}


def unit(rng):
    v = [rng.uniform(-1, 1) for _ in range(3)]
    l = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / l for x in v]


def cases(rng, n):
    for _ in range(n):
        fwd = rng.choice([unit(rng), [0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [rng.uniform(-3, 3) for _ in range(3)]])
        kind = rng.randrange(5)
        if kind == 0:
            up = [0.0, 1.0, 0.0]
        elif kind == 1:                       # parallel to forward: zero perpendicular
            up = [x * rng.choice([1.0, -2.0]) for x in fwd]
        else:
            up = unit(rng)
        side = rng.choice([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], unit(rng), [rng.uniform(-2, 2) for _ in range(3)]])
        yield dict(wheels=rng.choice([0, 1, 2, 2, 2]), fwd=fwd, up=up, side=side, use_up=rng.randrange(2),
                   cos=rng.randrange(2), saved=(rng.uniform(-1, 1), rng.uniform(-1, 1)))


def setup(ctx, c):
    def avg(cx, ecx, a):
        cx.pack(a[0], '3f', *c['up'])
        return a[0]
    ctx.stub_at(0x00528400, 'Vehicle::GetAverageGroundNormal', pop=4, ret=avg)
    veh = ctx.alloc(0x600, fill=0x41)
    ctx.pack(veh + 0x544, 'i', c['wheels'])
    ctx.pack(veh + 0x48, 'f', c['saved'][0])
    ctx.pack(veh + 0x3c, 'f', c['saved'][1])
    ctx.pack(veh + 0x4cc, '3f', *c['side'])
    fwd = ctx.alloc(12)
    ctx.pack(fwd, '3f', *c['fwd'])
    up = 0
    if c['use_up']:
        up = ctx.alloc(12)
        ctx.pack(up, '3f', *c['up'])
    return dict(ecx=veh, args=[fwd, c['cos'], up], ret_eax=False)
