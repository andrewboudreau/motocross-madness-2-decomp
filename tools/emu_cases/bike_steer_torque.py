"""Bike::Method_0x0040a520, the steering-torque integrator (tools/emu_diff.py case).

Offsets from the retail body (tier 1): Bike +0x108 airborne byte, +0x188 force accumulator,
+0x5f0 front wheel, +0x5f8/+0x5fc the two bar contacts, +0x610 gains, +0x61c steer torque,
+0x13c step time, +0x3bc model node, +0x47c steer state (angle at +4), +0x42c pose node.
Wheel +0xcc position, +0x120 load vector, +0x16c state word, +0x1c0 owner (axis at +0xc),
+0x200 node position, +0x20c up axis, +0x260 in contact, +0x274 radius. Contact +0x14 point,
+0x2c axis, +0xa4 active, +0xb4 state. The steer state's AddAngle (0x00504ec0) and SetAngle
(0x00504e20) are stubbed and logged; the vector helpers, the node's WorldToLocalDirection
and the scale-assign (0x0040ae00) run as retail code.

Both left-contact arms store point - nodePosition into `offset` and project `base` on the
contact axis (retail frame +0x20 and +0x38); left and right contacts are each active in
about a third of the cases so every arm is reached.
"""
from tools.emu_cases.resolve_contact_impulse import build_node, node_case, normal, vec

RETAIL_VA = 0x0040a520
SYMBOL = '?Method_0x0040a520@Bike@@QAEMXZ'
SOURCE = 'src/krusty2/vehicle/Bike.cpp'
PROBES = {0x0040a586: 'airborne', 0x0040a683: 'wheel contact', 0x0040a740: 'wheel + left bar',
          0x0040a986: 'left bar only', 0x0040aa8a: 'right bar test', 0x0040ac02: 'local torque',
          0x0040ad3c: 'add angle', 0x0040ada6: 'clamp angle'}


def contact(rng):
    return dict(active=rng.choice([0, 0, 1]), point=vec(rng, 3), axis=normal(rng))


def cases(rng, n):
    for _ in range(n):
        yield dict(air=rng.choice([0, 0, 0, 1]), force=vec(rng, 200), contact=rng.choice([0, 1, 1]),
                   pos=vec(rng, 3), nodepos=vec(rng, 3), load=vec(rng, 100), owner_axis=normal(rng),
                   up=normal(rng), radius=rng.uniform(0.2, 1.5), left=contact(rng), right=contact(rng),
                   gains=[rng.uniform(0, 0.05) for _ in range(3)],
                   torque=rng.choice([[0.0, 0.0, 0.0], [0.0, 0.0, 1.0], vec(rng, 0.5), vec(rng, 20)]),
                   step=rng.choice([1 / 60, rng.uniform(0.001, 0.05)]),
                   angle=rng.choice([0.0, 0.78, -0.78, rng.uniform(-0.8, 0.8), 0.9]), node=node_case(rng))


def setup(ctx, c):
    ctx.stub_at(0x00504ec0, 'SteerState::AddAngle', pop=8)
    ctx.stub_at(0x00504e20, 'SteerState::SetAngle', pop=8)
    bike = ctx.alloc(0x640, fill=0x3c)
    ctx.pack(bike + 0x108, 'B', c['air'])
    ctx.pack(bike + 0x188, '3f', *c['force'])
    ctx.pack(bike + 0x610, '6f', *c['gains'], *c['torque'])
    ctx.pack(bike + 0x13c, 'f', c['step'])
    ctx.pack(bike + 0x3bc, 'I', build_node(ctx, c['node']))
    steer = ctx.alloc(0x40)
    ctx.pack(steer + 4, 'f', c['angle'])
    ctx.pack(bike + 0x47c, 'I', steer)
    ctx.pack(bike + 0x42c, 'I', 0x00badd0e)
    wheel = ctx.alloc(0x280, fill=0x3c)
    owner = ctx.alloc(0x20)
    ctx.pack(owner + 0xc, '3f', *c['owner_axis'])
    ctx.pack(wheel + 0xcc, '3f', *c['pos'])
    ctx.pack(wheel + 0x120, '3f', *c['load'])
    ctx.pack(wheel + 0x1c0, 'I', owner)
    ctx.pack(wheel + 0x200, '3f', *c['nodepos'])
    ctx.pack(wheel + 0x20c, '3f', *c['up'])
    ctx.pack(wheel + 0x260, 'i', c['contact'])
    ctx.pack(wheel + 0x274, 'f', c['radius'])
    ctx.pack(bike + 0x5f0, 'I', wheel)
    for off, k in ((0x5f8, 'left'), (0x5fc, 'right')):
        p = ctx.alloc(0xc0, fill=0x3c)
        ctx.pack(p + 0x14, '3f', *c[k]['point'])
        ctx.pack(p + 0x2c, '3f', *c[k]['axis'])
        ctx.pack(p + 0xa4, 'i', c[k]['active'])
        ctx.pack(bike + off, 'I', p)
    return dict(ecx=bike, args=[], ret_float=True, ret_eax=False)
