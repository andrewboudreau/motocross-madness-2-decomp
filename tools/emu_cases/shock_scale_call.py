"""ShockScaleCall 0x005015b0, a ShockVec3 times a scalar (tools/emu_diff.py case).

src/bringup/Group4_VehicleShock.cpp forwards it to Vec3ScaleCall, which is bound to the same
retail body; the case checks the forwarding (the hidden return slot, the vector and the scalar
in place, the returned pointer). Random, zero, negative and huge scalars.
"""
import struct

RETAIL_VA = 0x005015b0
SYMBOL = '?ShockScaleCall@@YA?AUShockVec3@@ABU1@M@Z'
SOURCE = 'src/bringup/Group4_VehicleShock.cpp'


def cases(rng, n):
    for _ in range(n):
        v = [rng.uniform(-100, 100) for _ in range(3)]
        s = rng.choice([rng.uniform(-10, 10), 0.0, -1.0, 1e30, rng.uniform(-1e-3, 1e-3)])
        yield (v, s)


def setup(ctx, case):
    v, s = case
    pv = ctx.alloc(12)
    ctx.pack(pv, '3f', *v)
    out = ctx.alloc(12, fill=0x77)
    return dict(args=[out, pv, struct.unpack('<I', struct.pack('<f', s))[0]])
