"""D3DIMSoultreeShadow slot 29 0x00446c30, the caster's camera-view test (tools/emu_diff.py case,
call-record/replay mode).

The linked body is samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp. The body clears
``inCameraView`` (+0x38) and, unless the caster (+0x2c) is hidden (+0x14c) or the shadow did not
see it (``inShadowView`` +0x34), fetches the caster's world matrix (GetMatrixIn 0x004fca80) and
subtree bounds (GetSubtreeBounds 0x004fe850), multiplies the matrix by the render target's camera
matrix (renderContext +0x18 -> camera +8 -> +0xec) and hands the product to the visibility
clipper's box test (0x0052f570 on the clipper at 0x00575a98). A passing test sets inCameraView,
which the body returns.

Every call is replayed. GetMatrixIn and GetSubtreeBounds write seeded matrices and boxes; the box
test logs the 64-byte product, the centre and the extents and passes 60% of the time. At 24-bit
precision each element's sum order shows in the logged product.
"""
import random

RETAIL_VA = 0x00446c30
SYMBOL = '?UnknownVirtualSlot29@D3DIMSoultreeShadow@@UAEHXZ'
SOURCE = 'samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp'
CLIPPER = 0x00575a98
GET_MATRIX = 0x004fca80
GET_BOUNDS = 0x004fe850
TEST_BOX = 0x0052f570
PROBES = {0x00446c5c: 'matrix product', 0x00446f21: 'box test passed'}


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
    returns={GET_MATRIX: 'void', GET_BOUNDS: 'void', TEST_BOX: _test_box},
    effects={GET_MATRIX: _get_matrix, GET_BOUNDS: _get_bounds},
    pointees={TEST_BOX: [(1, 64), (2, 12), (3, 12)]},
    out_writes=False,
)


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def setup(ctx, seed):
    r = random.Random(seed)
    caster = ctx.alloc(0x300, bytes(r.getrandbits(8) for _ in range(0x300)))
    ctx.pack(caster + 0x14c, 'I', int(r.random() < 0.1))       # hidden
    this = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
    ctx.pack(this + 0x2c, 'I', caster)
    ctx.pack(this + 0x34, 'I', int(r.random() < 0.9))          # inShadowView
    camera = ctx.alloc(0x130)
    ctx.pack(camera + 0xec, '16f', *_matrix(r))
    render = ctx.alloc(0x10)
    ctx.pack(render + 8, 'I', camera)
    ctx.pack(this + 0x18, 'I', render)
    clipper = ctx.alloc(0x40)
    ctx.pack(CLIPPER, 'I', clipper)
    return dict(ecx=this, args=[])
