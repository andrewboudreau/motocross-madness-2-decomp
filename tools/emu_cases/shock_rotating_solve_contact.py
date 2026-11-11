"""RotatingShock::SolveContact 0x004fac60 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/suspension/Shock.cpp (TireAttachB::SolveContact in
Group4_VehicleShock.cpp jumps to it). The shock's 0x100 bytes are random floats; from the
retail body, +0x44 is the scene node and +0xe0 the swing axis selector (jump table of 4
entries; out-of-range values leave the swing uninitialised, so the case keeps 0..3). The
lengths (+0x90 cleared length, +0xc8 arm length, +0xc4/+0x98..) are positive in most runs so
the limit clamp and both delta signs run. A zero normal, or a normal along a y swing axis (the swing then has no y), takes the
`dist == 0` early return.

Stubbed: GetPositionIn 0x004fc9a0 (writes a case-random anchor) and the two node calls the
retail ClampAndStep 0x004fa8b0 makes, SetMatrixLike 0x004fca30 and Rotate 0x004fceb0 (the
rotation axis and angle are logged). ClampAndStep, the cross/scale/length/project helpers
and _finite run as retail code.
"""
import random
import struct

RETAIL_VA = 0x004fac60
SYMBOL = '?SolveContact@RotatingShock@@QAEXMPBUShockVec3@@00000MPAMPAH@Z'
SOURCE = 'src/krusty2/suspension/Shock.cpp'
PROBES = {0x004fae44: 'dist zero', 0x004fae73: 'zero, side below -1', 0x004faf10: 'contact', 0x004faf8e: 'free return',
          0x004fafbb: 'loaded return'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), axis=rng.randrange(4), normal=rng.choice(['random'] * 10 + ['zero', 'along axis', 'along axis']),
                   dt=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1)]),
                   limit=rng.choice([0.0, rng.uniform(-1, 1), rng.uniform(-0.05, 0.05)]))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def vec(s=2.0):
        p = ctx.alloc(12)
        ctx.pack(p, '3f', *[r.uniform(-s, s) for _ in range(3)])
        return p

    shock = ctx.alloc(0x100)
    ctx.write(shock, struct.pack('<64f', *[r.uniform(-2, 2) for _ in range(64)]))
    for off in (0x90, 0xc4, 0xc8):
        ctx.pack(shock + off, 'f', r.choice([r.uniform(0.05, 2), r.uniform(-1, 2)]))
    ctx.pack(shock + 0x44, 'I', ctx.alloc(0x40))
    ctx.pack(shock + 0xe0, 'i', c['axis'])

    def position_in(cx, ecx, a):
        cx.pack(a[1], '3f', *[cx.rng.uniform(-2, 2) for _ in range(3)])

    def rotate(cx, ecx, a):           # (ShockVec3 axis by value, float angle)
        return 0

    ctx.stub_at(0x004fc9a0, 'GetPositionIn', pop=8, ret=position_in)
    ctx.stub_at(0x004fca30, 'SetMatrixLike', pop=4)
    ctx.stub_at(0x004fceb0, 'Rotate', pop=16, ret=rotate)
    axis_a, axis_b, point = vec(), vec(), vec(3)
    normal = ctx.alloc(12)
    if c['normal'] == 'zero':
        ctx.pack(normal, '3f', 0.0, 0.0, 0.0)
    elif c['normal'] == 'along axis':          # swing has no y, so its dot is exactly zero
        ctx.pack(axis_a, '3f', 0.0, 1.0, 0.0)
        ctx.pack(axis_b, '3f', 0.0, -2.0, 0.0)
        ctx.pack(normal, '3f', 0.0, r.uniform(-1, 1), 0.0)
    else:
        ctx.pack(normal, '3f', *[r.uniform(-1, 1) for _ in range(3)])
    off_a, off_b = vec(5), vec(5)
    load = ctx.alloc(4, fill=0x77)
    active = ctx.alloc(4, fill=0x77)
    return dict(ecx=shock, args=[float(c['dt']), axis_a, axis_b, point, normal, off_a, off_b,
                                 float(c['limit']), load, active], ret_eax=False)
