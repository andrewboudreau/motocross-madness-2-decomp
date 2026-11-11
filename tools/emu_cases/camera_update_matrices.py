"""Camera::UpdateMatrices 0x0042ee30, slot 28 of every camera (tools/emu_diff.py case).

The camera is random floats; the frame (eye +0x170, direction +0x17c, up +0x188, roll +0x194),
the field of view +0x16c (degrees, including tiny and near-180 values), near/far +0x1bc/+0x1c0
and the unsigned viewport size +0x1a8/+0x1ac (including widths above 2^31, which retail loads
with a 64-bit fild) are set from the case. ViewMatrix 0x004a1500, ProjectionMatrix 0x004a1460,
IdentityMatrix 0x004a1410 and MatrixMult 0x004a1860 run as retail code, so the compared state is
the view (+0x6c), projection (+0xac), the two screen products (+0xec, +0x12c) and the focal
scale +0x198 (inline fptan).
"""
import math
import random
import struct

RETAIL_VA = 0x0042ee30
SYMBOL = '?UpdateMatrices@Camera@@UAEXXZ'
SOURCE = 'samples/camera/CameraNearMisses.cpp'


def unit(rng):
    v = [rng.uniform(-1, 1) for _ in range(3)]
    l = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / l for x in v]


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), eye=[rng.uniform(-500, 500) for _ in range(3)],
                   dir=unit(rng), up=rng.choice([[0.0, 1.0, 0.0], unit(rng)]), roll=rng.uniform(-3.2, 3.2),
                   fov=rng.choice([60.0, 45.0, rng.uniform(1, 170), rng.uniform(0.01, 1)]),
                   near=rng.choice([0.1, 1.0, rng.uniform(0.01, 10)]), far=rng.choice([1000.0, rng.uniform(20, 5000)]),
                   w=rng.choice([640, 800, 1024, rng.randrange(1, 4096), 0x80000000 + rng.randrange(1 << 20)]),
                   h=rng.choice([480, 600, 768, rng.randrange(1, 4096)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    cam = ctx.alloc(0x200)
    ctx.write(cam, struct.pack('<128f', *[r.uniform(-10, 10) for _ in range(128)]))
    ctx.pack(cam + 0x16c, 'f', c['fov'])
    ctx.pack(cam + 0x170, '3f', *c['eye'])
    ctx.pack(cam + 0x17c, '3f', *c['dir'])
    ctx.pack(cam + 0x188, '3f', *c['up'])
    ctx.pack(cam + 0x194, 'f', c['roll'])
    ctx.pack(cam + 0x1a8, 'II', c['w'], c['h'])
    ctx.pack(cam + 0x1bc, 'ff', c['near'], c['far'])
    return dict(ecx=cam, args=[], ret_eax=False)
