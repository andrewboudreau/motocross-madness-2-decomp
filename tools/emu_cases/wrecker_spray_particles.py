"""Wrecker::SprayContactParticles 0x00531740 (tools/emu_diff.py case).

The linked body is the samples/race/WreckerNearMisses.cpp candidate. Offsets from the
retail body: +0x44 wreck state (zero selects +0xbc linearStep, else the rigid body +0x40's
velocity at +0x154), +0x11c the particle manager (+0x2c active count, +0x40 the particle
pointer table, +0x1f98 gravity), +0x124 spray frame (wraps 0x2d -> 0x1d), +0x128 the 256-entry
random table and +0x528 its index (wraps at 256). Speeds straddle the `< 1` early return,
frameTime * 200 gives 0..10 particles, the active count sometimes reaches 1000 mid-spray and
the random index starts near its wrap. Nothing is stubbed: the length, dot, inverse square
root, constructor, scale and add helpers run as retail code.
"""
import random
import struct

RETAIL_VA = 0x00531740
SYMBOL = '?SprayContactParticles@Wrecker@@QAEXUVector3@@00M@Z'
SOURCE = 'samples/race/WreckerNearMisses.cpp'
PROBES = {0x00531800: 'moving', 0x00531a3d: 'spray', 0x00531c21: 'random wrap',
          0x00531d6b: 'frame wrap'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), state=rng.choice([0, 1]),
                   speed=rng.choice([rng.uniform(0, 1.2), rng.uniform(1, 40), 1.0]),
                   dt=rng.choice([1 / 60.0, 1 / 30.0, 0.05, rng.uniform(0, 0.05)]),
                   active=rng.choice([0, 10, 995, 998, 1000]), index=rng.choice([0, 250, 254, 255, 100]),
                   frame=rng.choice([0x1d, 0x2b, 0x2c, 0x10]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    w = ctx.alloc(0x540)
    ctx.write(w, struct.pack('<336f', *[r.uniform(-3, 3) for _ in range(336)]))
    v = [r.uniform(-1, 1) for _ in range(3)]
    l = sum(x * x for x in v) ** 0.5 or 1.0
    v = [x * c['speed'] / l for x in v]
    if r.random() < 0.2:
        v = [0.0, c['speed'], 0.0]                  # a unit direction keeps the `== 1` arms
    body = ctx.alloc(0x160)
    if c['state']:
        ctx.pack(body + 0x154, '3f', *v)
    else:
        ctx.pack(w + 0xbc, '3f', *v)
    ctx.pack(w + 0x40, 'II', body, c['state'])
    manager = ctx.alloc(0x2000)
    ctx.pack(manager + 0x2c, 'i', c['active'])
    for i in range(c['active'], min(c['active'] + 12, 1001)):
        ctx.pack(manager + 0x40 + 4 * i, 'I', ctx.alloc(0x40, fill=0x66))
    ctx.pack(manager + 0x1f98, '3f', *[r.uniform(-10, 10) for _ in range(3)])
    ctx.pack(w + 0x11c, 'I', manager)
    ctx.pack(w + 0x124, 'i', c['frame'])
    ctx.write(w + 0x128, struct.pack('<256f', *[r.random() for _ in range(256)]))
    ctx.pack(w + 0x528, 'i', c['index'])
    ends = [r.uniform(-5, 5) for _ in range(9)]
    return dict(ecx=w, args=[*ends[:6], *[x / 5 for x in ends[6:]], float(c['dt'])], ret_eax=False)
