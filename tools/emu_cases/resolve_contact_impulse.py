"""ResolveContactImpulse 0x00500220, the static-contact impulse (tools/emu_diff.py case).

One scene node (rigid world matrix, valid or stale with a parent) carries the direction
helpers, which run as retail code in both bodies. Approaching and separating velocities, a
zero and nonzero scale (the last argument is a float passed as int) and random inertia.
"""
import struct

from tools.emu_cases.soultree_set_matrix_in import LOCAL, PARENT, VALID, WORLD, NODE, rigid

RETAIL_VA = 0x00500220
SYMBOL = '?ResolveContactImpulse@@YAXMMPAVSoultreeObject@@PBUVec3@@11PAU2@122PAMH@Z'
SOURCE = 'src/krusty2/soultree/SoulTreePhysics.cpp'
PROBES = {0x005002b5: 'approaching', 0x00500386: 'scaled impulse', 0x005003bc: 'unscaled impulse'}


def vec(rng, s):
    return [rng.uniform(-s, s) for _ in range(3)]


def node_case(rng):
    return dict(local=rigid(rng), world=rigid(rng), parent_world=rigid(rng),
                valid=rng.randrange(2), has_parent=rng.randrange(2))


def build_node(ctx, c):
    node = ctx.alloc(NODE)
    ctx.pack(node + LOCAL, '16f', *c['local'])
    ctx.pack(node + WORLD, '16f', *c['world'])
    ctx.pack(node + VALID, 'i', c['valid'])
    if c['has_parent']:
        p = ctx.alloc(NODE)
        ctx.pack(p + LOCAL, '16f', *c['parent_world'])
        ctx.pack(p + WORLD, '16f', *c['parent_world'])
        ctx.pack(p + VALID, 'i', 1)
        ctx.pack(node + PARENT, 'I', p)
    return node


def normal(rng):
    v = vec(rng, 1)
    l = sum(x * x for x in v) ** 0.5 or 1.0
    return [x / l for x in v] if rng.random() < 0.8 else rng.choice([[0.0, 1.0, 0.0], [1.0, 0.0, 0.0]])


def cases(rng, n):
    for _ in range(n):
        scale = rng.choice([0.0, 0.0, 1.0, rng.uniform(0.1, 2.0)])
        yield dict(node=node_case(rng), e=rng.choice([0.0, rng.uniform(0, 1)]), invMass=rng.uniform(0.001, 2),
                   n=normal(rng), rel=vec(rng, 20), arm=vec(rng, 2), inertia=[rng.uniform(0.01, 5) for _ in range(3)],
                   ang=[rng.uniform(0, 2) for _ in range(3)], angVel=vec(rng, 5), vel=vec(rng, 20), scale=scale)


def setup(ctx, c):
    node = build_node(ctx, c['node'])
    ptr = {}
    for k in ('n', 'rel', 'arm', 'inertia', 'ang', 'angVel', 'vel'):
        ptr[k] = ctx.alloc(12)
        ctx.pack(ptr[k], '3f', *c[k])
    out = ctx.alloc(4, fill=0x77)
    bits = struct.unpack('<I', struct.pack('<f', c['scale']))[0]
    return dict(args=[c['e'], c['invMass'], node, ptr['n'], ptr['rel'], ptr['arm'], ptr['inertia'], ptr['ang'],
                      ptr['angVel'], ptr['vel'], out, bits], ret_eax=False)
