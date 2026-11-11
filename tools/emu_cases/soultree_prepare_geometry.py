"""D3DIMSoultreeObject::PrepareGeometry 0x00443aa0, the tree's per-frame visibility test
(tools/emu_diff.py case, call-record/replay mode).

The body clears ``visible`` (+0x2c8) and, unless the tree is hidden (+0x140) or has no geometry
(+0x184), refreshes the subtree bounds when dirty (+0x180), fetches the world matrix
(GetMatrixIn 0x004fca80) and the bounds (GetSubtreeBounds 0x004fe850), multiplies the world
matrix by the current camera's matrix (renderContext +0x18 -> camera +8 -> +0xec) and hands
the product to the visibility clipper's box test (0x0052f570 on the clipper at 0x00575a98). A
passing test sets ``visible`` and calls SelectLod 0x00443de0; the base PrepareGeometry 0x00469300
runs last.

Every call is replayed. GetMatrixIn and GetSubtreeBounds write seeded matrices and boxes; the
box test logs the 64-byte product, the centre and the extents and passes 60% of the time. At
24-bit precision each element's sum order shows in the logged product.
"""
import random

RETAIL_VA = 0x00443aa0
SYMBOL = '?PrepareGeometry@D3DIMSoultreeObject@@UAEHXZ'
SOURCE = 'src/bringup/Group5_D3DIMSoulTreeDraw.cpp'
CLIPPER = 0x00575a98
UPDATE_BOUNDS = 0x004fe0f0
GET_MATRIX = 0x004fca80
GET_BOUNDS = 0x004fe850
TEST_BOX = 0x0052f570
SELECT_LOD = 0x00443de0
BASE_PREPARE = 0x00469300
PROBES = {0x00443ae2: 'matrix product', 0x00443daf: 'box test passed'}


def _matrix(rng):
    m = [rng.uniform(-2.0, 2.0) for _ in range(16)]
    if rng.random() < 0.7:          # an affine transform: translation row, last column 0,0,0,1
        m[3] = m[7] = m[11] = 0.0
        m[15] = 1.0
        m[12:15] = [rng.uniform(-3000.0, 3000.0) for _ in range(3)]
    return m


def _get_matrix(ctx, rec):
    ctx.pack(rec.args[1], '16f', *_matrix(rec.rng))


def _get_bounds(ctx, rec):
    ctx.pack(rec.args[0], '3f', *[rec.rng.uniform(-3000.0, 3000.0) for _ in range(3)])
    ctx.pack(rec.args[1], '3f', *[rec.rng.uniform(0.0, 40.0) for _ in range(3)])


def _test_box(ctx, rec):
    return int(rec.rng.random() < 0.6)


REPLAY = dict(
    returns={UPDATE_BOUNDS: 'void', GET_MATRIX: 'void', GET_BOUNDS: 'void', TEST_BOX: _test_box,
             SELECT_LOD: 'void', BASE_PREPARE: 'int'},
    effects={GET_MATRIX: _get_matrix, GET_BOUNDS: _get_bounds},
    pointees={TEST_BOX: [(1, 64), (2, 12), (3, 12)], SELECT_LOD: [(0, 12), (1, 12)]},
    out_writes=False,
)


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def setup(ctx, seed):
    r = random.Random(seed)
    obj = ctx.alloc(0x300, bytes(r.getrandbits(8) for _ in range(0x300)))
    ctx.pack(obj + 0x140, 'I', int(r.random() < 0.1))          # hidden
    ctx.pack(obj + 0x184, 'I', int(r.random() < 0.9))          # has geometry
    ctx.pack(obj + 0x180, 'I', int(r.random() < 0.5))          # subtree bounds dirty
    camera = ctx.alloc(0x130)
    ctx.pack(camera + 0xec, '16f', *_matrix(r))
    render = ctx.alloc(0x10)
    ctx.pack(render + 8, 'I', camera)
    ctx.pack(obj + 0x18, 'I', render)
    clipper = ctx.alloc(0x40)
    ctx.pack(CLIPPER, 'I', clipper)
    return dict(ecx=obj, args=[])
