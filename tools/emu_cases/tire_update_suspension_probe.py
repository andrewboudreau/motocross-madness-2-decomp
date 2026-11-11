"""Tire::UpdateSuspensionProbe 0x00514550 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/tire/Tire.cpp (the VehicleWheel shim jumps to it).
The 0x2c0-byte tire is random floats; the pointers come from the retail body: +0x1bc scene
node, +0x1c0 owner record (first member a node; null in about half the runs, which selects the
a6/a7/a8 axes), +0x58/+0x5c the contact flag and bounds record (scale, box pointer, normal),
+0x178 the surface owner (+0xa4 the per-surface grip tables). The second probe angle is
random or exactly +-pi/2 (the a7 arm); a7 is null in some owner-less runs.

Stubbed, each with case-random outputs drawn from the run's stream and the inputs noted (so
a wrong probe position, axis or matrix shows): GetPositionIn 0x004fc9a0, the ground query
0x00507c10 (moves the point's y, writes a normal from a two-entry pool so the
same-normal arms run, and a surface byte), LocalToWorldDirection 0x004fd5c0,
D3DRMVectorRotate (import thunk 0x0053304c), SetTransform 0x00435830 (notes the frame) and
QueryCollisions 0x00438e70. The cross, dot, inverse square root, frame build 0x00515880,
transform 0x0042a510/0x0042a580 and 0x00515c90 helpers run as retail code.
"""
import math
import random
import struct

RETAIL_VA = 0x00514550
SYMBOL = '?UpdateSuspensionProbe@Tire@@QAEXPAVTireWorld@@PBUCollisionVec3@@MMM11PAVTireNode@@@Z'
SOURCE = 'src/krusty2/tire/Tire.cpp'
# File-static axis constants (Tire.bindings.json); retail builds them at start-up (.bss).
AXES = {'_kVec3ZAxis': (0x0068a3c0, (0.0, 0.0, 1.0)), '_kVec3Zero': (0x0068a3d0, (0.0, 0.0, 0.0)),
        '_kVec3XAxis': (0x0068a3e0, (1.0, 0.0, 0.0)), '_kVec3YAxis': (0x0068a3f0, (0.0, 1.0, 0.0))}
BINDINGS = {k: va for k, (va, _) in AXES.items()}
PROBES = {0x005145b5: 'owner axes', 0x005146d1: 'a8 axes', 0x00514b9b: 'rotate', 0x00514f2f: 'refine',
          0x005153a1: 'contact', 0x005153c4: 'box depth', 0x00515586: 'surface tables'}


def unit(rng):
    v = [rng.uniform(-1, 1) for _ in range(3)]
    l = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / l for x in v]


def cases(rng, n):
    for _ in range(n):
        owner = rng.random() < 0.5
        a7 = owner or rng.random() < 0.7
        angle = rng.choice([rng.uniform(-3, 3), 1.5707964, -1.5707964, 0.0]) if a7 else rng.uniform(-3, 3)
        yield dict(seed=rng.randrange(1 << 30), owner=owner, a7=a7, angle=angle,
                   radius_scale=rng.uniform(-1, 2), yaw=rng.uniform(-4, 4),
                   contact=rng.choice([0, 1]), scale=rng.choice([rng.uniform(0, 1), rng.uniform(-1, 2)]),
                   surface=rng.random() < 0.7, same=rng.random() < 0.4,
                   threshold=rng.choice([0.0, rng.uniform(-1, 1)]))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def rvec(s=2.0):
        return [r.uniform(-s, s) for _ in range(3)]

    for va, value in AXES.values():
        ctx.pack(va, '3f', *value)
    tire = ctx.alloc(0x2c0)
    ctx.write(tire, struct.pack('<176f', *[r.uniform(-2, 2) for _ in range(176)]))
    ctx.pack(tire + 0x154, 'f', c['threshold'])
    ctx.pack(tire + 0x274, 'f', r.uniform(0.2, 1.5))
    node = ctx.alloc(0x40)
    ctx.pack(tire + 0x1bc, 'I', node)
    if c['owner']:
        rec = ctx.alloc(8)
        ctx.pack(rec, 'I', ctx.alloc(0x40))
        ctx.pack(tire + 0x1c0, 'I', rec)
    else:
        ctx.pack(tire + 0x1c0, 'I', 0)
    bounds = ctx.alloc(0x20)
    box = ctx.alloc(0x18)
    ctx.pack(box, '6f', *rvec(), *rvec())
    ctx.pack(bounds, 'fI3f', c['scale'], box, *unit(r))
    ctx.pack(tire + 0x58, 'II', c['contact'], bounds)
    if c['surface']:
        owner = ctx.alloc(0xa8)
        tables = ctx.alloc(0x420)
        ctx.write(tables, struct.pack('<264f', *[r.uniform(0, 2) for _ in range(264)]))
        ctx.pack(owner + 0xa4, 'I', tables)
        ctx.pack(tire + 0x178, 'I', owner)
    else:
        ctx.pack(tire + 0x178, 'I', 0)

    pool = [unit(r), unit(r)]

    def position_in(cx, ecx, a):        # (parent, out)
        cx.pack(a[1], '3f', *[cx.rng.uniform(-5, 5) for _ in range(3)])

    def ground(cx, ecx, a):             # (pos, normal, flags, byte*)
        cx.note('ground at', cx.read(a[0], 12).hex())
        x, y, z = cx.unpack(a[0], '3f')
        cx.pack(a[0], '3f', x, y + cx.rng.uniform(-0.5, 0.5), z)
        n = pool[0] if c['same'] else cx.rng.choice(pool + [unit(cx.rng)])
        cx.pack(a[1], '3f', *n)
        cx.pack(a[3], 'B', cx.rng.randrange(256))
        return 1

    def to_world(cx, ecx, a):           # (out, in) -> out
        cx.note('direction of', cx.read(a[1], 12).hex())
        cx.pack(a[0], '3f', *unit(cx.rng))
        return a[0]

    def rotate(cx, ecx, a):             # stdcall (result, vector, axis, theta) -> result
        cx.note('rotate', cx.read(a[1], 12).hex(), cx.read(a[2], 12).hex(), a[3])
        cx.pack(a[0], '3f', *unit(cx.rng))
        return a[0]

    def set_transform(cx, ecx, a):
        cx.note('frame', cx.read(a[0], 64).hex())

    ctx.stub_at(0x004fc9a0, 'GetPositionIn', pop=8, ret=position_in)
    ctx.stub_at(0x00507c10, 'QueryGround', pop=16, ret=ground)
    ctx.stub_at(0x004fd5c0, 'LocalToWorldDirection', pop=8, ret=to_world)
    ctx.stub_at(0x0053304c, 'D3DRMVectorRotate', pop=16, ret=rotate)
    ctx.stub_at(0x00435830, 'SetTransform', pop=4, ret=set_transform)
    ctx.stub_at(0x00438e70, 'QueryCollisions')

    world = ctx.alloc(0x40)
    velocity = ctx.alloc(12)
    ctx.pack(velocity, '3f', *rvec(10))
    a6 = ctx.alloc(12)
    ctx.pack(a6, '3f', *rc(r))
    a7 = 0
    if c['a7']:
        a7 = ctx.alloc(12)
        ctx.pack(a7, '3f', *rc(r))
    a8 = ctx.alloc(0x40)
    return dict(ecx=tire, args=[world, velocity, float(c['angle']), float(c['radius_scale']),
                                float(c['yaw']), a6, a7, a8], ret_eax=False)


def rc(r):
    return r.choice([unit(r), [r.uniform(-2, 2) for _ in range(3)], [0.0, 1.0, 0.0]])
