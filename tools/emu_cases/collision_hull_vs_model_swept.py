"""CollisionObject::HullVsModelSwept 0x004376f0 (tools/emu_diff.py case).

A swept hull against a model: broad phase 0x00424730 as retail code, then each enabled element
runs the retail swept hull test 0x00436af0 over the logging tree-test stub, and the contacts
are averaged. The scene-node position call 0x004fc9a0 is stubbed (its result is dead).
"""
from tools.emu_cases.collision_hull_vs_hull import (query_desc, stub_node_position, stub_tree_query, write_hull,
                                                     write_query, write_scratch)
from tools.emu_cases.collision_model_vs_hull import close_hull, model_desc, write_model

RETAIL_VA = 0x004376f0
SYMBOL = '?HullVsModelSwept@CollisionObject@@QAEHPAUCollisionHullBody@@PAUCollisionModelBody@@PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00437900: 'element test'}


def cases(rng, n):
    for _ in range(n):
        yield (close_hull(rng, swept=1), model_desc(rng), query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ha, mb = write_hull(ctx, a), write_model(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ha, mb, q])
