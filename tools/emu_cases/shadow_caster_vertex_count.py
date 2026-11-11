"""ShadowCaster::GetVertexCount 0x00444fe0 (tools/emu_diff.py case).

Linked from src/bringup/Group1_ShadowCaster.cpp (retail D3DIMSoultreeObject::GetVertexCount).
A null or real LOD table (+0x28c) of 8-byte {surface count, surfaces} records, an LOD count
(+0x274) of -1..6, surface counts of -1..5 and 0x38-byte surfaces whose vertex counts (+8)
are small, negative or near the int limits, so the signed compares and wrap decide.
"""
import random
import struct

RETAIL_VA = 0x00444fe0
SYMBOL = '?GetVertexCount@ShadowCaster@@QAEHXZ'
SOURCE = 'src/bringup/Group1_ShadowCaster.cpp'


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    obj = ctx.alloc(0x2a0, fill=0x5a)
    lods = r.choice([-1, 0, 1, 2, 3, 6])
    ctx.pack(obj + 0x274, 'i', lods)
    if r.random() < 0.1:
        ctx.pack(obj + 0x28c, 'I', 0)
        return dict(ecx=obj)
    records = []
    for _ in range(max(lods, 0)):
        count = r.choice([-1, 0, 1, 2, 5])
        surfaces = ctx.alloc(0x38 * max(count, 1), fill=0x6b)
        for j in range(max(count, 0)):
            ctx.pack(surfaces + 0x38 * j + 8, 'i',
                     r.choice([r.randrange(0, 3000), -r.randrange(1, 100), 0x7fffff00, -0x7fffff00]))
        records += [count & 0xffffffff, surfaces]
    table = ctx.alloc(8 * max(lods, 1), struct.pack('<%dI' % len(records), *records) if records else None)
    ctx.pack(obj + 0x28c, 'I', table)
    return dict(ecx=obj)
