"""TireCross 0x00515600, the CollisionVec3 cross product (tools/emu_diff.py case).

src/bringup/Group2_VehicleWheel.cpp forwards it to CrossProductCall, which is bound to the same
retail body; the case checks the forwarding (operand order, the hidden return slot and the
returned pointer). Random, parallel, zero and aliased operands.
"""
import struct

RETAIL_VA = 0x00515600
SYMBOL = '?TireCross@@YA?AUCollisionVec3@@ABU1@0@Z'
SOURCE = 'src/bringup/Group2_VehicleWheel.cpp'


def cases(rng, n):
    for _ in range(n):
        a = [rng.uniform(-50, 50) for _ in range(3)]
        kind = rng.randrange(4)
        if kind == 0:
            b = [rng.uniform(-50, 50) for _ in range(3)]
        elif kind == 1:
            k = rng.uniform(-3, 3)
            b = [k * x for x in a]
        elif kind == 2:
            b = [0.0, 0.0, 0.0]
        else:
            b = None
        yield (a, b)


def setup(ctx, case):
    a, b = case
    pa = ctx.alloc(12)
    ctx.pack(pa, '3f', *a)
    if b is None:
        pb = pa
    else:
        pb = ctx.alloc(12)
        ctx.pack(pb, '3f', *b)
    out = ctx.alloc(12, fill=0x77)
    return dict(args=[out, pa, pb])
