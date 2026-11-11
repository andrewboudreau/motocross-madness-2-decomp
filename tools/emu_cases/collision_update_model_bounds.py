"""CollisionObject::UpdateModelBounds 0x00434eb0 (tools/emu_diff.py case).

A model (collision_model_vs_hull's builder, 1..5 elements, scaled element frames included)
is refitted: the old bounds are saved, each enabled element's box is rotated into the model
frame and the running min/max gives the new centre and half extents. The vector operators,
the point transform 0x0042a510 and the min/max helpers run as retail code. Element 0 is always
enabled: with it disabled both bodies read the uninitialised min/max locals, whose stack
layout differs by construction (retail does the same with garbage).
"""
from tools.emu_cases.collision_model_vs_hull import model_desc, write_model

RETAIL_VA = 0x00434eb0
SYMBOL = '?UpdateModelBounds@CollisionObject@@QAEXXZ'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x004354e6: 'max of later elements'}


def cases(rng, n):
    for _ in range(n):
        d = model_desc(rng, count=rng.randrange(1, 6))
        d['enabled'][0] = 1
        yield d


def setup(ctx, case):
    model = write_model(ctx, case)
    obj = ctx.alloc(0x100, fill=0x5e)
    ctx.pack(obj + 0x50, 'iI', 1, model)
    return dict(ecx=obj)
