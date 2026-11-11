"""ConstraintMethodCollisionModel::ApplyContactImpulse 0x0043bdb0 (tools/emu_diff.py case).

ConBody layout from ConstraintTypes.h (velocity +0x154, previous velocity +0x160, angular
velocity +0x16c, mass +0x178, inertia and inverse inertia matrices +0x190/+0x1d0, centre of
mass +0x228, node +0x234) and the model's fields (+0xb8 restitution, +0xc0 dt, +0xc4 body,
+0xcc contact point, +0xf0 damping, +0xf4 impulse loss, +0xf8 max impulse, +0xfc max speed).
Virtual slot 43 of each body is retail's GetWorldAngularVelocity (0x004cbed0); the node
helpers and vector operators run as retail code. With and without the other body; max
impulse and max speed zero (unlimited) or small enough to clamp.
"""
from tools.emu_cases.resolve_contact_impulse import build_node, node_case, normal, vec

RETAIL_VA = 0x0043bdb0
SYMBOL = '?ApplyContactImpulse@ConstraintMethodCollisionModel@@QAEXPAVConBody@@MUConVec3@@11@Z'
SOURCE = 'samples/physics/constraint/ConstraintMethodCollisionModelNearMisses.cpp'
PROBES = {0x0043c0b2: 'other body', 0x0043c47a: 'max impulse test', 0x0043c6f5: 'speed clamp'}


def inertia(rng):
    m = [0.0] * 16
    for i in range(3):
        m[i * 5] = rng.uniform(0.1, 5)
    if rng.random() < 0.5:
        for i in range(12):
            if i % 5:
                m[i] = rng.uniform(-0.3, 0.3)
    m[15] = 1.0
    if rng.random() < 0.3:                 # translation row (termB reads it)
        m[12:15] = vec(rng, 0.5)
    return m


def body_case(rng):
    return dict(node=node_case(rng), vel=vec(rng, 20), prev=vec(rng, 20), w=vec(rng, 3),
                mass=rng.uniform(0.5, 50), I=inertia(rng), invI=inertia(rng), com=vec(rng, 1))


def cases(rng, n):
    for _ in range(n):
        yield dict(a=body_case(rng), b=body_case(rng), with_b=rng.random() < 0.6,
                   e=rng.uniform(0, 1), dt=rng.choice([1 / 60, rng.uniform(0, 0.1)]), damp=rng.uniform(0, 2),
                   loss=rng.choice([0.0, rng.uniform(0, 0.5)]),
                   maxJ=rng.choice([0.0, rng.uniform(0.01, 5), 1e6]), maxV=rng.choice([0.0, rng.uniform(0.5, 10), 1e6]),
                   t=rng.choice([0.0, 1.0, rng.random()]), offset=vec(rng, 0.5), point=vec(rng, 5), n=normal(rng))


def build_body(ctx, b):
    vt = ctx.vtable([0] * 43 + [0x004cbed0] + [0] * 4)
    body = ctx.alloc(0x240, fill=0x3c)
    ctx.pack(body, 'I', vt)
    ctx.pack(body + 0x154, '9f', *b['vel'], *b['prev'], *b['w'])
    ctx.pack(body + 0x178, 'ff', b['mass'], 1 / b['mass'])
    ctx.pack(body + 0x190, '16f', *b['I'])
    ctx.pack(body + 0x1d0, '16f', *b['invI'])
    ctx.pack(body + 0x228, '3f', *b['com'])
    ctx.pack(body + 0x234, 'I', build_node(ctx, b['node']))
    return body


def setup(ctx, c):
    model = ctx.alloc(0x110, fill=0x3c)
    ctx.pack(model + 0xb8, 'f', c['e'])
    ctx.pack(model + 0xc0, 'f', c['dt'])
    ctx.pack(model + 0xc4, 'I', build_body(ctx, c['a']))
    ctx.pack(model + 0xf0, '4f', c['damp'], c['loss'], c['maxJ'], c['maxV'])
    other = build_body(ctx, c['b']) if c['with_b'] else 0
    return dict(ecx=model, args=[other, c['t'], *c['offset'], *c['point'], *c['n']], ret_eax=False)
