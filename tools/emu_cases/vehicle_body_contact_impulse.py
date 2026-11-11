"""Vehicle::ApplyBodyContactImpulse 0x005265d0, slot 4 (tools/emu_diff.py case).

Offsets from the retail body (tier 1): +0x24 inverse mass, +0x14c restitution, +0x1ac scratch
vector, +0xd8 angular velocity, +0xe4/+0xf0 the two inverse inertias (crash state +0x444
selects the second), +0x3bc model node, +0xbc linear speed, +0xcc world angular velocity.
ContactSolveImpulse and the node's direction helpers run as retail code. velBOrNull is null
in some cases; equal point velocities with a unit velA reach the `len2 == 1` arm.
"""
from tools.emu_cases.resolve_contact_impulse import build_node, node_case, normal, vec

RETAIL_VA = 0x005265d0
SYMBOL = '?ApplyBodyContactImpulse@Vehicle@@UAEXPBUVec3@@PAU2@00HMH00011HPAMM@Z'
SOURCE = 'src/krusty2/vehicle/Vehicle.cpp'
PROBES = {0x00526787: 'solve', 0x005267b8: 'unit speed', 0x005267c2: 'sqrt'}


def cases(rng, n):
    for _ in range(n):
        c = dict(n=normal(rng), nodeA=node_case(rng), nodeB=node_case(rng), crash=rng.choice([0, 0, 2]),
                 invMass=rng.uniform(0.01, 1), e=rng.uniform(0, 1), invMassB=rng.choice([0.0, rng.uniform(0.01, 1)]),
                 with_b=rng.random() < 0.7, scale=rng.choice([1.0, 0.5, rng.uniform(0, 2)]),
                 inertia=[rng.uniform(0.01, 3) for _ in range(3)], shape=[rng.uniform(0.01, 3) for _ in range(3)],
                 iB=[rng.uniform(0.01, 3) for _ in range(3)])
        for k in ('velA', 'wA', 'armA', 'velB', 'wB', 'armB', 'angVel', 'angVelB'):
            c[k] = vec(rng, 5)
        if rng.random() < 0.15:
            # Equal point velocities give a zero impulse, so |velA| stays exactly 1.
            c['velA'] = c['velB'] = [0.0, 1.0, 0.0]
            c['wA'] = c['wB'] = [0.0, 0.0, 0.0]
        yield c


def setup(ctx, c):
    veh = ctx.alloc(0x600, fill=0x41)
    ctx.pack(veh + 0x24, 'f', c['invMass'])
    ctx.pack(veh + 0x14c, 'f', c['e'])
    ctx.pack(veh + 0xd8, '3f', *c['angVel'])
    ctx.pack(veh + 0xe4, '3f', *c['inertia'])
    ctx.pack(veh + 0xf0, '3f', *c['shape'])
    ctx.pack(veh + 0x444, 'i', c['crash'])
    ctx.pack(veh + 0x3bc, 'I', build_node(ctx, c['nodeA']))
    nodeB = build_node(ctx, c['nodeB'])
    p = {}
    for k in ('n', 'velA', 'wA', 'armA', 'velB', 'wB', 'armB', 'iB', 'angVelB'):
        p[k] = ctx.alloc(12)
        ctx.pack(p[k], '3f', *c[k])
    velBOrNull = 0
    if c['with_b']:
        velBOrNull = ctx.alloc(12)
        ctx.pack(velBOrNull, '3f', 1.0, -2.0, 0.5)
    out = ctx.alloc(4, fill=0x77)
    return dict(ecx=veh, args=[p['n'], p['velA'], p['wA'], p['armA'], 7, c['invMassB'], nodeB, p['velB'],
                               p['wB'], p['armB'], p['iB'], p['angVelB'], velBOrNull, out, c['scale']],
                ret_eax=False)
