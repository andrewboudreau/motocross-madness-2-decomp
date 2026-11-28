"""Vec3Normalize 0x005087b0, the CollisionVec3 pointer-form normaliser (tools/emu_diff.py case).

The game build links the pointer form to src/reconstructed/Math3D.cpp's strict-exact
Vec3Normalize(const Vec3&) through a link alias (the same cdecl arguments: hidden result
pointer, then the source; the result pointer is returned), so the case runs that body.
Random vectors of any scale, exact unit vectors (the `squared == 1` copy), vectors whose
sum of squares rounds to 1, tiny and zero vectors (FastInvSqrt 0x00460c00 runs as retail code in both bodies), and the output aliased
to the input. The returned pointer is compared too.
"""
RETAIL_VA = 0x005087b0
SYMBOL = '?Vec3Normalize@@YA?AUVec3@@ABU1@@Z'
SOURCE = 'src/reconstructed/Math3D.cpp'


def cases(rng, n):
    for _ in range(n):
        kind = rng.randrange(6)
        if kind == 0:
            v = [rng.uniform(-100, 100) for _ in range(3)]
        elif kind == 1:
            v = [0.0, 0.0, 0.0]
            v[rng.randrange(3)] = rng.choice([1.0, -1.0])
        elif kind == 2:
            v = [rng.uniform(-1, 1) for _ in range(3)]
            s = sum(x * x for x in v) ** 0.5 or 1.0
            v = [x / s for x in v]
        elif kind == 3:
            v = [rng.uniform(-1e-3, 1e-3) for _ in range(3)]
        elif kind == 4:
            v = [0.0, 0.0, 0.0]
        else:
            v = [float(rng.randrange(-4, 5)) for _ in range(3)]
        yield (v, rng.random() < 0.3)


def setup(ctx, case):
    v, aliased = case
    src = ctx.alloc(12)
    ctx.pack(src, '3f', *v)
    out = src if aliased else ctx.alloc(12, fill=0x77)
    return dict(args=[out, src])
