"""GraphicsTest::DrawCircle 0x0047bd10, the debug circle of line segments (tools/emu_diff.py case).

The linked body is samples/physics/helpers/GraphicsTestNearMisses.cpp. CrossProductCall
0x00515600 and Vec3Normalize 0x005087b0 run natively (retail's). The circle has 0..32 segments
(above 32 both bodies write 2 * segments vertices past the 64-entry array, into the caller's
frame), no axis (the xz plane) or a random, axis-aligned, diagonal or zero axis, random centres
and zero, unit, random and negative radii. The render context's DrawPrimitive (vtable +0x40)
notes the whole vertex buffer it is handed.
"""
import random

RETAIL_VA = 0x0047bd10
SYMBOL = '?DrawCircle@GraphicsTest@@QAEXPBUVec3@@M0H@Z'
SOURCE = 'samples/physics/helpers/GraphicsTestNearMisses.cpp'
PROBES = {0x0047be11: 'axis basis', 0x0047c096: 'draw'}


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def setup(ctx, seed):
    r = random.Random(seed)
    segments = r.choice([0, 1, 2, 3, 7, 8, 16, 31, 32])
    test = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    target = ctx.alloc(0x10)
    vtable = ctx.alloc(0x80)
    ctx.pack(target, 'I', vtable)
    ctx.pack(test + 0x18, 'I', target)

    def draw(cx, ecx, a):           # (type, fvf, vertices, count, flags)
        count = a[3] if a[3] <= 128 else 0
        cx.note('DrawPrimitive', a[0], a[1], a[3], a[4], cx.read(a[2], 32 * count).hex())
        return 0
    ctx.pack(vtable + 0x40, 'I', ctx.stub('DrawPrimitive', pop=20, ret=draw))
    center = ctx.alloc(12)
    ctx.pack(center, '3f', *[r.choice([0.0, r.uniform(-500, 500)]) for _ in range(3)])
    kind = r.choice(['none', 'none', 'random', 'random', 'x', 'y', 'z', 'zero', 'diag'])
    axis = 0
    if kind != 'none':
        axis = ctx.alloc(12)
        if kind == 'random':
            v = [r.uniform(-2, 2) for _ in range(3)]
        elif kind == 'zero':
            v = [0.0, 0.0, 0.0]
        elif kind == 'diag':
            v = [1.0, 1.0, 1.0]
        else:
            v = [float(c == kind) * r.choice([1.0, -1.0, 3.5]) for c in 'xyz']
        ctx.pack(axis, '3f', *v)
    radius = r.choice([0.0, 1.0, r.uniform(0.01, 100), -r.uniform(0.01, 10)])
    return dict(ecx=test, args=[center, radius, axis, segments], ret_eax=False)
