"""SoultreeObject::SetAxesIn 0x004fc050 (tools/emu_diff.py case).

The node tree of soultree_set_matrix_in with random axis pairs: unit, scaled, nearly parallel
and exactly unit-length (the normalise helper's `|v|^2 == 1` shortcut), with and without
orthogonalisation, keeping z or y. Vec3Normalize, the cross product, the matrix products and
the world-matrix helpers run as retail code in both bodies.
"""
from tools.emu_cases.soultree_set_matrix_in import build_tree, tree_case

RETAIL_VA = 0x004fc050
SYMBOL = '?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z'
SOURCE = 'samples/physics/helpers/SoultreeMatrix.cpp'
PROBES = {0x004fc133: 'orthogonalize', 0x004fc29d: 'frame == this', 0x004fc2f4: 'frame product', 0x004fc3d6: 'parent branch'}


def axis(rng):
    kind = rng.randrange(4)
    if kind == 0:
        return rng.choice([[0.0, 0.0, 1.0], [0.0, 1.0, 0.0], [1.0, 0.0, 0.0], [0.0, -1.0, 0.0]])
    if kind == 1:
        return [rng.uniform(-1, 1) for _ in range(3)]
    return [rng.uniform(-50, 50) for _ in range(3)]


def cases(rng, n):
    for _ in range(n):
        c = tree_case(rng)
        z, y = axis(rng), axis(rng)
        if rng.random() < 0.15:          # nearly parallel axes
            y = [v + rng.uniform(-1e-3, 1e-3) for v in z]
        c['axes'] = (z, y, rng.randrange(2), rng.randrange(2))
        yield c


def setup(ctx, case):
    me, frame = build_tree(ctx, case)
    z, y, ortho, keep = case['axes']
    pz, py = ctx.alloc(12), ctx.alloc(12)
    ctx.pack(pz, '3f', *z)
    ctx.pack(py, '3f', *y)
    return dict(ecx=me, args=[frame, pz, py, ortho, keep], ret_eax=False)
