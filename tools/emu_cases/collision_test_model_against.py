"""CollisionObject::TestModelAgainst 0x00438c10 (tools/emu_diff.py case).

The linked body is canonical src/krusty2/collision/CollisionObject.cpp. Same dispatch as
TestHullAgainst (collision_test_hull_against.py) with `this` a model: jump table 0x00438c70
over the other object's shape type, the hull and model tests also taking the contact record.

Stubbed: ModelVsHull 0x00437c20, ModelVsModel 0x00438280, ModelVsSphere 0x00438860 and
ModelVsCapsule 0x004389b0, each returning a case-random value.
"""
from tools.emu_cases.collision_test_hull_against import cases, dispatch_setup  # noqa: F401

RETAIL_VA = 0x00438c10
SYMBOL = '?TestModelAgainst@CollisionObject@@QAEHPAV1@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00438c27: 'hull', 0x00438c39: 'model', 0x00438c4b: 'sphere', 0x00438c59: 'capsule',
          0x00438c67: 'default'}
TESTS = {0: (0x00437c20, 'ModelVsHull', 12), 1: (0x00438280, 'ModelVsModel', 12),
         4: (0x00438860, 'ModelVsSphere', 8), 3: (0x004389b0, 'ModelVsCapsule', 8)}


def setup(ctx, c):
    return dispatch_setup(ctx, c, TESTS)
