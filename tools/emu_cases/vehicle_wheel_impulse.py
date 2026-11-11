"""Vehicle::Method_00527A20 0x00527a20, wheel drive and contact impulses (tools/emu_diff.py case).

Offsets from the retail body (tier 1): Vehicle +0x53c wheel list, +0x544 count, +0x444 crash
state, +0x440 direction flag, +0x13c step time, +0x158 body mass. Wheel +0x260 in contact,
+0x2a0 drive strength, +0xe4 ground normal, +0xf0 relative position, +0xfc applied share,
+0x120/+0x12c load vector/load, +0xb8 contact point (virtual slot 1), +0x130 friction force,
+0x13c friction magnitude, +0x148/+0x14c tangent/spin speed. The per-wheel drive
(0x00514170) and the contact point's slot 1 are stubbed and logged; Vec3 * float
(0x005015b0) runs as retail code. Values cover both impulse arms, a zero budget, a
nonpositive time scale, loads on both sides of zero and *speed above and below the budget.
"""
RETAIL_VA = 0x00527a20
SYMBOL = '?Method_00527A20@Vehicle@@QAEXPAMPAUVec3@@1@Z'
SOURCE = 'src/krusty2/vehicle/Vehicle.cpp'
PROBES = {0x00527ac9: 'drive', 0x00527b46: 'load >= 0', 0x00527ba1: 'load < 0',
          0x00527c73: 'scaled impulse', 0x00527cb7: 'unscaled impulse'}


def vec(rng, s):
    return [rng.uniform(-s, s) for _ in range(3)]


def wheel(rng):
    return dict(contact=rng.choice([0, 1, 1, 1]), drive=rng.choice([0.0, 0.0, rng.uniform(0, 3)]),
                normal=vec(rng, 1), rel=vec(rng, 2), share=vec(rng, 50), force=vec(rng, 100),
                mag=rng.choice([0.0, -1.0, rng.uniform(0, 500), rng.uniform(0, 5)]),
                tangent=rng.choice([0.0, rng.uniform(-5, 20)]), spin=rng.choice([0.0, rng.uniform(-5, 20)]))


def cases(rng, n):
    for _ in range(n):
        yield dict(wheels=[wheel(rng) for _ in range(rng.choice([0, 1, 2, 2, 4]))],
                   crash=rng.choice([0, 0, 1]), forward=rng.randrange(2),
                   step=rng.choice([1 / 60, 1 / 30, rng.uniform(0.001, 0.05)]),
                   mass=rng.uniform(1, 20), speed=rng.choice([0.0, -1.0, rng.uniform(0, 30), 1e6]),
                   torque=vec(rng, 10), f=vec(rng, 10))


def setup(ctx, c):
    ctx.stub_at(0x00514170, 'VehicleWheel::ApplyDrive', pop=0x1c)
    slot1 = ctx.stub('contactPoint.slot1', pop=0)
    cvt = ctx.vtable([slot1] * 4)
    veh = ctx.alloc(0x600)
    wheels = []
    for w in c['wheels']:
        p = ctx.alloc(0x2b0, fill=0)
        ctx.pack(p + 0x260, 'i', w['contact'])
        ctx.pack(p + 0x2a0, 'f', w['drive'])
        ctx.pack(p + 0xe4, '3f', *w['normal'])
        ctx.pack(p + 0xf0, '3f', *w['rel'])
        ctx.pack(p + 0xfc, '3f', *w['share'])
        ctx.pack(p + 0xb8, 'I', cvt)
        ctx.pack(p + 0x130, '3f', *w['force'])
        ctx.pack(p + 0x13c, 'f', w['mag'])
        ctx.pack(p + 0x148, 'ff', w['tangent'], w['spin'])
        wheels.append(p)
    lst = ctx.alloc(4 * max(len(wheels), 1))
    for i, p in enumerate(wheels):
        ctx.pack(lst + 4 * i, 'I', p)
    ctx.pack(veh + 0x53c, 'I', lst)
    ctx.pack(veh + 0x544, 'i', len(wheels))
    ctx.pack(veh + 0x444, 'i', c['crash'])
    ctx.pack(veh + 0x440, 'i', c['forward'])
    ctx.pack(veh + 0x13c, 'f', c['step'])
    ctx.pack(veh + 0x158, 'f', c['mass'])
    speed, torque, force = ctx.alloc(4), ctx.alloc(12), ctx.alloc(12)
    ctx.pack(speed, 'f', c['speed'])
    ctx.pack(torque, '3f', *c['torque'])
    ctx.pack(force, '3f', *c['f'])
    return dict(ecx=veh, args=[speed, torque, force], ret_eax=False)
