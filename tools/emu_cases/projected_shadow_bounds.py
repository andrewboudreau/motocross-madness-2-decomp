"""ProjectedShadow::ComputeBounds 0x004db0d0, light-space caster bounds (tools/emu_diff.py case).

Zero to four casters (array +0x2c, count +0x30) whose GetBounds (0x004fe850, stubbed)
returns random centres and half extents, carried through random, rotation-like and
scaled world matrices (+0xf8); then mode 3 (orthographic, texture size +0x14 of +0x54),
mode 1 (point light at +0x44: lightDir +0x38, atan2 field of view, normalisation, unit
with random light positions) and other modes (bounds only). The camera (+0x58) SetLookAt
(0x0042e9b0) and virtual slots 28/29 are stubbed and log their inputs, including the
pointed-to eye and fov; its matrices +0xac/+0xec are random. Retail ShadowMatrixIdentity,
ShadowMatrixMultiply, the Vec3 constructor, FastInvSqrt and atan2 run in both bodies.
Compared: centre +0xcc, half extents +0xc0, lensScale +0xbc, lightDir and lightMatrix +0xd8.
A null caster array returns at once.
"""
import math
import struct

RETAIL_VA = 0x004db0d0
SYMBOL = '?ComputeBounds@ProjectedShadow@@QAEHXZ'
SOURCE = 'samples/physics/shadow/ProjectedShadowNearMisses.cpp'
PROBES = {0x004db5a7: 'mode 3', 0x004db6d1: 'mode 1', 0x004db7f5: 'mode 1 normalise'}


def rotation(rng):
    a, b, c = (rng.uniform(-math.pi, math.pi) for _ in range(3))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    return [[ca * cc + sa * sb * sc, cb * sc, -sa * cc + ca * sb * sc],
            [-ca * sc + sa * sb * cc, cb * cc, sa * sc + ca * sb * cc],
            [sa * cb, -sb, ca * cb]]


def matrix(rng):
    kind = rng.randrange(3)
    if kind == 0:
        rows = [[rng.uniform(-3, 3) for _ in range(3)] for _ in range(3)]
    elif kind == 1:
        rows = rotation(rng)
    else:
        s = rng.uniform(0.1, 4)
        rows = [[s * v for v in r] for r in rotation(rng)]
    t = [rng.uniform(-800, 800), rng.uniform(-30, 80), rng.uniform(-800, 800)]
    return [*rows[0], 0.0, *rows[1], 0.0, *rows[2], 0.0, *t, 1.0]


def cases(rng, n):
    for _ in range(n):
        casters = []
        for _ in range(rng.choice([0, 1, 1, 2, 4])):
            casters.append(([rng.uniform(-5, 5) for _ in range(3)],
                            [rng.choice([0.0, rng.uniform(0, 4)]) for _ in range(3)], matrix(rng)))
        mode = rng.choice([3, 3, 1, 1, 0])
        light_dir = [rng.uniform(-1, 1) for _ in range(3)]
        light_pos = [rng.uniform(-900, 900), rng.uniform(50, 400), rng.uniform(-900, 900)]
        cam = [[rng.uniform(-2, 2) for _ in range(16)] for _ in range(2)]
        yield (casters, mode, light_dir, light_pos, rng.choice([64, 128, 256]), cam, rng.random() < 0.05)


def setup(ctx, case):
    casters, mode, light_dir, light_pos, size, cam, null_array = case

    def get_bounds(c, ecx, a):
        centre, ext, _ = data[ecx]
        c.pack(a[0], '3f', *centre)
        c.pack(a[1], '3f', *ext)
    data = {}
    ctx.stub_at(0x004fe850, 'ShadowCaster::GetBounds', pop=8, ret=get_bounds)

    def look_at(c, ecx, a):
        c.note('eye', c.read(a[0], 12).hex(), c.read(a[4], 4).hex() if a[4] else None)
    ctx.stub_at(0x0042e9b0, 'Camera::SetLookAt', pop=20, ret=look_at)

    array = ctx.alloc(4 * max(len(casters), 1))
    for i, (centre, ext, world) in enumerate(casters):
        k = ctx.alloc(0x200, fill=0x3c)
        ctx.pack(k + 0xf8, '16f', *world)
        ctx.pack(array + 4 * i, 'I', k)
        data[k] = (centre, ext, world)

    camera_vt = [ctx.stub('Camera::vslot%d' % i) for i in range(28)]
    camera_vt.append(ctx.stub('Camera::UnknownVirtualSlot28'))
    camera_vt.append(ctx.stub('Camera::UnknownVirtualSlot29', pop=12))
    camera = ctx.alloc(0x140)
    ctx.pack(camera, 'I', ctx.vtable(camera_vt))
    ctx.pack(camera + 0xac, '16f', *cam[0])
    ctx.pack(camera + 0xec, '16f', *cam[1])
    texture = ctx.alloc(0x40)
    ctx.pack(texture + 0x14, 'i', size)

    this = ctx.alloc(0x140, fill=0x77)
    ctx.pack(this + 0x2c, 'Ii', 0 if null_array else array, len(casters))
    ctx.pack(this + 0x38, '3f', *light_dir)
    ctx.pack(this + 0x44, '3f', *light_pos)
    ctx.pack(this + 0x54, 'II', texture, camera)
    ctx.pack(this + 0x8c, 'i', mode)
    return dict(ecx=this, args=[])
