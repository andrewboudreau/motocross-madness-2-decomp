"""CollisionObject::FreeShape 0x00432430 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/collision/CollisionObject.cpp. With a shape (+0x54)
the shape type (+0x50, jump table 0x00432494) picks the release: 0 frees the hull's arrays
and then the hull, 1 and 2 hand the shape to the model/mesh free, 3 and 4 delete it, other
types leak it. The contact record (+0x5c) is deleted when present and both pointers are
cleared. Types run over 0..6 and -1; the shape and the record are each absent in some runs.

Stubbed: CollisionHullShape_Free 0x00431da0, CollisionModelShape_Free 0x00431df0,
CollisionMeshShape_Free 0x00431e50 and operator delete 0x004a30c0 (all cdecl, the pointer
logged), so the call order is compared.
"""
RETAIL_VA = 0x00432430
SYMBOL = '?FreeShape@CollisionObject@@QAEXXZ'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x0043244c: 'hull', 0x00432460: 'model', 0x00432468: 'mesh', 0x00432470: 'delete',
          0x00432480: 'record'}


def cases(rng, n):
    for _ in range(n):
        yield dict(kind=rng.choice([0, 1, 2, 3, 4, 5, 6, -1]), shape=rng.random() < 0.85,
                   record=rng.random() < 0.6)


def setup(ctx, c):
    me = ctx.alloc(0x80, fill=0x33)
    ctx.pack(me + 0x50, 'iI', c['kind'], ctx.alloc(0x40) if c['shape'] else 0)
    ctx.pack(me + 0x5c, 'I', ctx.alloc(0x40) if c['record'] else 0)
    for va, name in ((0x00431da0, 'CollisionHullShape_Free'), (0x00431df0, 'CollisionModelShape_Free'),
                     (0x00431e50, 'CollisionMeshShape_Free'), (0x004a30c0, 'operator delete')):
        ctx.stub_at(va, name, nargs=1)
    return dict(ecx=me, ret_eax=False)
