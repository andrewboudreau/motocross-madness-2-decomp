"""Vec3ScaleCall 0x005015b0, a vector times a scalar (tools/emu_diff.py case).

Shock.cpp calls it as ShockScaleCall, a link alias of Vec3ScaleCall (config/game_build.json);
the case checks the hidden return slot, the vector and the scalar in place and the returned
pointer. Random, zero, negative and huge scalars.
"""
import struct

RETAIL_VA = 0x005015b0
SYMBOL = '?Vec3ScaleCall@@YAPAUVec3@@PAU1@PBU1@M@Z'
SOURCE = 'src/reconstructed/VectorCalls.cpp'


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
