"""ContactSolveImpulse 0x005004a0, the body/body contact impulse (tools/emu_diff.py case).

Two scene nodes carry the direction helpers (retail code in both bodies). Random contact
geometry, masses, inertia and restitution; velB is null in some cases (the B update is
skipped). The impulse, both bodies' accumulators and *outJ are compared.
"""
from tools.emu_cases.resolve_contact_impulse import build_node, node_case, normal, vec

RETAIL_VA = 0x005004a0
SYMBOL = '?ContactSolveImpulse@@YAXMPBUVec3@@MPAVSoultreeObject@@000PAU1@2M100022PAM@Z'
SOURCE = 'samples/physics/contact/ContactImpulse.cpp'
PROBES = {0x0050084a: 'body B update'}


def cases(rng, n):
    for _ in range(n):
        c = dict(e=rng.choice([0.0, rng.uniform(0, 1)]), n=normal(rng),
                 invA=rng.uniform(0.001, 2), invB=rng.choice([0.0, rng.uniform(0.001, 2)]),
                 nodeA=node_case(rng), nodeB=node_case(rng), with_b=rng.random() < 0.7)
        for k in ('dispA', 'dispB'):
            c[k] = vec(rng, 10)
        for k in ('rA', 'rB'):
            c[k] = vec(rng, 2)
        for k in ('iA', 'iB'):
            c[k] = [rng.uniform(0.01, 5) for _ in range(3)]
        for k in ('wA', 'vA', 'wB', 'vB'):
            c[k] = vec(rng, 10)
        yield c


def setup(ctx, c):
    a, b = build_node(ctx, c['nodeA']), build_node(ctx, c['nodeB'])
    p = {}
    for k in ('n', 'dispA', 'dispB', 'rA', 'rB', 'iA', 'iB', 'wA', 'vA', 'wB', 'vB'):
        p[k] = ctx.alloc(12)
        ctx.pack(p[k], '3f', *c[k])
    out = ctx.alloc(4, fill=0x77)
    return dict(args=[c['e'], p['n'], c['invA'], a, p['dispA'], p['rA'], p['iA'], p['wA'], p['vA'],
                      c['invB'], b, p['dispB'], p['rB'], p['iB'], p['wB'], p['vB'] if c['with_b'] else 0, out],
                ret_eax=False)
