"""CollisionObject::ModelVsModelSwept 0x00438550 (tools/emu_diff.py case).

Two models, a swept: a's box is grown by its motion and cached, then every element of b runs
the retail ModelVsHullSwept 0x00437ef0; the result is the last hit's contact.
"""
from tools.emu_cases.collision_hull_vs_hull import (query_desc, stub_node_position, stub_tree_query, write_query,
                                                     write_scratch)
from tools.emu_cases.collision_model_vs_hull import model_desc, write_model

RETAIL_VA = 0x00438550
SYMBOL = '?ModelVsModelSwept@CollisionObject@@QAEHPAUCollisionModelBody@@0PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x004387e8: 'element test'}


def cases(rng, n):
    for _ in range(n):
        yield (model_desc(rng, swept=1, count=1 + rng.randrange(3)), model_desc(rng, count=1 + rng.randrange(3)),
               query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ma, mb = write_model(ctx, a), write_model(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ma, mb, q])
