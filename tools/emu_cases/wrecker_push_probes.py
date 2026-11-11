"""Wrecker::PushProbesOutOfBox 0x005329e0 (tools/emu_diff.py case).

The linked body is the samples/race/WreckerNearMisses.cpp candidate. Offsets from the
retail body: +0x38 the collision model (+0x100 probe count, +0x104 probes of 0x20 bytes, the
local point at +0 and the node at +0xc), +0x100 the box frame, +0x104 the probe position
array (12-byte stride), +0xe8 box centre, +0xf4 half extents, +0x118 push scale and +0x10c
the impulse written at the end. The rest of the wrecker is random floats.

Stubbed: the probe node's LocalToWorldPoint 0x004fd660 and the box frame's WorldToLocalPoint
0x004fd7f0 (thiscall, out pointer returned); each notes its input point and returns a
case-random point. The box-local points straddle the half extents (and sometimes equal one
or sit at zero) so every clip arm runs, the `hits` mean and the previous-position updates.
The constructor 0x00404e60 and scale 0x005015b0 run as retail code.
"""
import random
import struct

RETAIL_VA = 0x005329e0
SYMBOL = '?PushProbesOutOfBox@Wrecker@@QAEXM@Z'
SOURCE = 'samples/race/WreckerNearMisses.cpp'
PROBES = {0x00532b10: 'probe', 0x00532e39: 'mean'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), count=rng.choice([0, 1, 2, 3, 5, 8]),
                   dt=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    w = ctx.alloc(0x200)
    ctx.write(w, struct.pack('<128f', *[r.uniform(-3, 3) for _ in range(128)]))
    half = [r.uniform(0.2, 3) for _ in range(3)]
    center = [r.uniform(-2, 2) for _ in range(3)]
    ctx.pack(w + 0xe8, '6f', *center, *half)
    model = ctx.alloc(0x110)
    probes = ctx.alloc(0x20 * max(c['count'], 1))
    for i in range(c['count']):
        ctx.pack(probes + 0x20 * i, '3fI', *[r.uniform(-2, 2) for _ in range(3)], ctx.alloc(0x40))
    ctx.pack(model + 0x100, 'iI', c['count'], probes)
    positions = ctx.alloc(12 * max(c['count'], 1), fill=0x55)
    ctx.pack(w + 0x38, 'I', model)
    ctx.pack(w + 0x100, 'II', ctx.alloc(0x40), positions)

    def to_world(cx, ecx, a):
        cx.note('local', cx.read(a[1], 12).hex())
        cx.pack(a[0], '3f', *[cx.rng.uniform(-4, 4) for _ in range(3)])
        return a[0]

    def to_box(cx, ecx, a):
        cx.note('world', cx.read(a[1], 12).hex())
        p = []
        for k in range(3):
            kind = cx.rng.randrange(8)
            if kind == 0:
                v = half[k]
            elif kind == 1:
                v = 0.0
            else:
                v = cx.rng.uniform(-1.2, 1.2) * half[k]
            p.append(v + center[k])
        cx.pack(a[0], '3f', *p)
        return a[0]

    ctx.stub_at(0x004fd660, 'LocalToWorldPoint', pop=8, ret=to_world)
    ctx.stub_at(0x004fd7f0, 'WorldToLocalPoint', pop=8, ret=to_box)
    return dict(ecx=w, args=[float(c['dt'])], ret_eax=False)
