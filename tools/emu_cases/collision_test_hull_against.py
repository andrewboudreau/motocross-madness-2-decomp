"""CollisionObject::TestHullAgainst 0x00438b90 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/collision/CollisionObject.cpp. `this` is a hull
object; the other object's shape type (+0x50) picks the pair test through a five-entry jump
table (0 hull, 1 model, 2 none, 3 capsule, 4 sphere; anything above 4 returns 0). The hull
and model tests also get this object's contact record (+0x5c). The other type runs over
0..6 and the two -1/0x80000000 sentinels so the unsigned `ja` default is covered.

Stubbed: the four pair tests (thiscall on this object, shape pointers logged), each
returning a case-random value that must come back in EAX.
"""
RETAIL_VA = 0x00438b90
SYMBOL = '?TestHullAgainst@CollisionObject@@QAEHPAV1@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00438ba7: 'hull', 0x00438bb9: 'model', 0x00438bcb: 'sphere', 0x00438bd9: 'capsule',
          0x00438be7: 'default'}
# (VA, name, popped bytes) by other shape type
TESTS = {0: (0x00436720, 'HullVsHull', 12), 1: (0x00436e50, 'HullVsModel', 12),
         4: (0x004379c0, 'HullVsSphere', 8), 3: (0x00437aa0, 'HullVsCapsule', 8)}
TYPES = [0, 1, 2, 3, 4, 5, 6, -1, -0x80000000]


def cases(rng, n):
    for _ in range(n):
        yield dict(kind=rng.choice(TYPES), mine=rng.choice([0, 0x1000]), ret=rng.choice([0, 1, 7]))


def dispatch_setup(ctx, c, tests):
    me = ctx.alloc(0x80, fill=0x33)
    other = ctx.alloc(0x80, fill=0x44)
    ctx.pack(me + 0x54, 'I', ctx.alloc(0x40) + c['mine'] if c['mine'] else 0)
    ctx.pack(me + 0x5c, 'I', ctx.alloc(0x40))
    ctx.pack(other + 0x50, 'iI', c['kind'], ctx.alloc(0x40))
    for va, name, pop in tests.values():
        ctx.stub_at(va, name, pop=pop, ret=c['ret'])
    return dict(ecx=me, args=[other])


def setup(ctx, c):
    return dispatch_setup(ctx, c, TESTS)
