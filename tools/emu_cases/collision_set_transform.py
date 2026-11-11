"""CollisionObject::SetTransform 0x00435830 (tools/emu_diff.py case).

Places a collision object under a new matrix and refreshes its shape: a hull (six child
matrices and a box tree), a model (0..5 elements with enable flags, each a hull), a mesh,
and the two plain shape types. SoultreeObject::GetMatrixIn (0x004fca80) is replaced in
both runs by a deterministic rigid matrix keyed on the node and seed; every other helper
(CollisionRelativeTransform, MoveBox, UpdateModelBounds, FastSqrt, ...) is retail's. The
fast-math .bss tables are built by running retail's initializers.

Matrices are rigid (Euler angles plus translation) 80% of the time, otherwise sixteen
arbitrary values, so the bounds arithmetic sees non-orthonormal input too. The result is
every byte written to the object and shape graph.
"""
import math
import random
import struct

RETAIL_VA = 0x00435830
SYMBOL = '?SetTransform@CollisionObject@@QAEXPBUMatrix4@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00435b9c: 'hull MoveBox', 0x00435cd8: 'model element GetMatrixIn',
          0x00435e24: 'mesh CollisionLength', 0x00435e96: 'sphere transform'}
FPCWS = (0x027f, 0x007f, 0x037f, 0x0e7f)

GET_MATRIX_IN = 0x004fca80
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)


def rmat(r):
    if r.random() < 0.8:
        a, b, c = (r.uniform(-3.2, 3.2) for _ in range(3))
        ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
        return [cb * cc, cb * sc, -sb, 0,
                sa * sb * cc - ca * sc, sa * sb * sc + ca * cc, sa * cb, 0,
                ca * sb * cc + sa * sc, ca * sb * sc - sa * cc, ca * cb, 0,
                r.uniform(-500, 500), r.uniform(-50, 50), r.uniform(-500, 500), 1]
    return [r.uniform(-4, 4) for _ in range(16)]


def cases(rng, n):
    for i in range(n):
        yield (i, rng.choice([0, 0, 1, 1, 2, 3, 4]))


def _f(*v):
    return struct.pack('<%df' % len(v), *v)


def setup(ctx, case):
    seed, shape_type = case
    r = random.Random(seed)
    for init in FAST_TABLE_INITS:
        ctx.call(init)

    def rv(lo=-10, hi=10):
        return [r.uniform(lo, hi) for _ in range(3)]

    def tree():
        return ctx.alloc(0x40, _f(*[r.uniform(-50, 50) for _ in range(16)]))

    obj = ctx.alloc(0x80, bytes(r.randrange(256) for _ in range(0x40)) + _f(*rv(), r.uniform(0, 9)))
    m = ctx.alloc(0x40, _f(*rmat(r)))
    if shape_type == 0:      # hull
        shape = ctx.alloc(0x198, b'\0' * 8 + b''.join(_f(*rmat(r)) for _ in range(6)))
        ctx.pack(shape + 0x188, 'I', tree())
    elif shape_type == 1:    # model
        n = r.choice([0, 1, 2, 3, 5])
        k = max(n, 1)
        shape = ctx.alloc(0x120)
        elems = ctx.alloc(0x198 * k)
        enabled = ctx.alloc(4 * k, struct.pack('<%di' % k, *[r.choice([0, 1, 1]) for _ in range(k)]))
        hulls = ctx.alloc(4 * k)
        ctx.write(shape, struct.pack('<iIIiiI', n, enabled, hulls, 0, 0, elems) +
                  _f(*rv(), *rv(0, 9), *rv(), *rv(0, 9)) + b''.join(_f(*rmat(r)) for _ in range(3)) +
                  _f(*rv(), *rv()))
        for i in range(n):
            e = elems + 0x198 * i
            t = tree()
            ctx.write(e, struct.pack('<iI', 0, 0x5000 + i) + b''.join(_f(*rmat(r)) for _ in range(6)))
            ctx.pack(e + 0x188, '3I', t, t, t)
    elif shape_type == 2:    # mesh
        t = tree()
        shape = ctx.alloc(0x48, struct.pack('<II', 0, t) + _f(*rmat(r)))
    else:
        shape = ctx.alloc(0x80, _f(*[r.uniform(-20, 20) for _ in range(32)]))
    ctx.pack(obj + 0x50, 'iI', shape_type, shape)

    def get_matrix_in(c, node, args):
        which, out = args
        c.pack(out, '16f', *rmat(random.Random(node * 7 + seed)))
    ctx.stub_at(GET_MATRIX_IN, 'GetMatrixIn', pop=8, ret=get_matrix_in)
    return dict(args=[m], ecx=obj, ret_eax=False)
