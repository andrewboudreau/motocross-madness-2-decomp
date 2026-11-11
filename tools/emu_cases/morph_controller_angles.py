"""MorphBastardModifier::ControllerAngles 0x004a3c80, a morph channel's controller rotation as
three angles (tools/emu_diff.py case, call-record/replay mode).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_MorphBastard.cpp). The only
call that leaves the math is SoultreeObject::GetMatrixIn 0x004fca80 on the channel's
controller; it is replayed and writes the controller's world matrix. Every other callee is
pure math and runs as retail code: the dot product 0x0040ae30, the cross product 0x00515600,
the matrix row 0x004a2350, the axis-angle matrix 0x0042de90, the matrix multiply 0x00436500,
FastInvSqrt 0x00460c00, the Vector3 constructor 0x00404e60 and the angle helper 0x004a3be0
(acos through the CRT).

The controller's world matrix times ``inverse`` (passed by value) is the relative rotation
whose angles are stored at +0x60/+0x64/+0x68. Inputs: random rotations, rotations about a
single principal axis with an identity rest pose (rows that stay exactly on an axis, so the
cross products are zero and each block's fallback axis runs), the identity, half turns (the
angle helper's -1 clamp), small angles and non-orthonormal (scaled, sheared) matrices.
"""
import math
import struct

RETAIL_VA = 0x004a3c80
SYMBOL = '?ControllerAngles@MorphBastardModifier@@QAEXPAUMorphBastardChannel@@UMatrix4@@@Z'
SOURCE = 'src/bringup/Group3_MorphBastard.cpp'
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)
GET_MATRIX_IN = 0x004fca80
# The bring-up's out-of-line Vector3 constructor view (link_aliases: Vec3Call 0x00404e60).
BINDINGS = {'??0MbVector3Call@@QAE@MMM@Z': 0x00404e60}
PROBES = {0x004a4253: 'x block: zero cross, unit x axis',
          0x004a42c7: 'x block: normalise',
          0x004a45a5: 'x angle negated',
          0x004a46c9: 'y block: zero cross, unit y axis',
          0x004a487e: 'y angle negated',
          0x004a499b: 'z block: zero cross, unit z axis',
          0x004a4b81: 'z angle negated'}


def _rotation(axis, angle):
    x, y, z = axis
    k = 1.0 / math.sqrt(x * x + y * y + z * z)
    x, y, z = x * k, y * k, z * k
    c, s = math.cos(angle), math.sin(angle)
    t = 1.0 - c
    return [[t * x * x + c, t * x * y + s * z, t * x * z - s * y],
            [t * x * y - s * z, t * y * y + c, t * y * z + s * x],
            [t * x * z + s * y, t * y * z - s * x, t * z * z + c]]


def _principal(which, angle):
    """Exact rotation about one principal axis: that row stays exactly on its axis."""
    c, s = math.cos(angle), math.sin(angle)
    if which == 0:
        return [[1.0, 0.0, 0.0], [0.0, c, s], [0.0, -s, c]]
    if which == 1:
        return [[c, 0.0, -s], [0.0, 1.0, 0.0], [s, 0.0, c]]
    return [[c, s, 0.0], [-s, c, 0.0], [0.0, 0.0, 1.0]]


def _identity():
    return [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]


def _matrix(rows, translation):
    out = []
    for r in rows:
        out += list(r) + [0.0]
    return out + list(translation) + [1.0]


def _random_rotation(rng, kind):
    if kind == 'identity':
        return _identity()
    if kind == 'principal':
        return _principal(rng.randrange(3), rng.choice([rng.uniform(-3.1, 3.1), math.pi, -math.pi / 2]))
    if kind == 'small':
        return _rotation([rng.uniform(-1, 1) for _ in range(3)], rng.uniform(-1e-3, 1e-3))
    if kind == 'half':
        return _rotation([rng.uniform(-1, 1) for _ in range(3)], math.pi)
    rows = _rotation([rng.uniform(-1, 1) for _ in range(3)], rng.uniform(-math.pi, math.pi))
    if kind == 'skewed':
        rows = [[v * rng.uniform(0.8, 1.2) + rng.uniform(-0.05, 0.05) for v in r] for r in rows]
    return rows


def _controller(ctx, rec):
    # GetMatrixIn(SoultreeObject* relativeTo, Matrix4* out): the controller's world matrix.
    ctx.pack(rec.args[1], '16f', *ctx.case['world'])


REPLAY = dict(
    passthrough=(0x0040ae30, 0x00515600, 0x004a2350, 0x0042de90, 0x00436500, 0x00460c00, 0x00460b50,
                 0x00404e60, 0x004a3be0, 0x00535240),
    sizes={'Matrix4': 64},
    effects={GET_MATRIX_IN: _controller},
)


def cases(rng, n):
    kinds = ['random', 'random', 'principal', 'principal', 'identity', 'small', 'half', 'skewed']
    for _ in range(n):
        world_kind = rng.choice(kinds)
        rest_identity = world_kind in ('principal', 'identity') or rng.random() < 0.2
        world = _random_rotation(rng, world_kind)
        rest = _identity() if rest_identity else _random_rotation(rng, 'random')
        # inverse of the rest pose: the transpose of a rotation (with the translation undone)
        move = [rng.uniform(-50, 50) for _ in range(3)]
        rest_move = [rng.uniform(-50, 50) for _ in range(3)]
        inverse_rows = [[rest[j][i] for j in range(3)] for i in range(3)]
        inverse_move = [-sum(rest_move[j] * inverse_rows[j][i] for j in range(3)) for i in range(3)]
        yield dict(
            seed=rng.getrandbits(32),
            axis=rng.randrange(3),
            world=[struct.unpack('<f', struct.pack('<f', v))[0] for v in _matrix(world, move)],
            inverse=_matrix(inverse_rows, inverse_move),
        )


def setup(ctx, c):
    for init in FAST_TABLE_INITS:
        ctx.call(init)
    ctx.case = c
    this = ctx.alloc(0x40)
    controller = ctx.alloc(0x100)
    channel = ctx.alloc(0x6c)
    ctx.pack(channel, 'i', c['axis'])
    ctx.pack(channel + 0x44, 'I', controller)
    ctx.pack(channel + 0x60, '3f', 7.0, 8.0, 9.0)
    return dict(ecx=this, args=[channel, struct.pack('<16f', *c['inverse'])], ret_eax=False)
