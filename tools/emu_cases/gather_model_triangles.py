"""GatherModelTriangles 0x0042c260 (tools/emu_diff.py case).

The linked body is src/bringup/Group0_GatherModelTriangles.cpp. The model (+0x27c current
LOD, +0x28c LOD table of {meshCount, meshes}) has 0..3 meshes of 0x38 bytes: parts (+0x00
count, +0x04 records of {frame, vertexCount, firstVertex}), +0x0c triangle count, +0x10 and
+0x14 two vertex arrays (0x20-byte vertices, position first) and +0x1c the triangles
(three unsigned shorts). With a frame argument only triangles whose corners all lie in the
last part naming that frame count, and the positions come from +0x14. Positions are drawn
from a small grid (with -0.0 against 0.0 and an occasional NaN) so the merge pass finds
duplicates; parts name the frame, another node or nothing.

Stubbed (thiscall on the model): GetMatrixIn 0x004fca80 (writes a case-random matrix),
SetAxesIn 0x004fc050, SetPositionInFrame 0x004fc740, MarkSubtreeDirty 0x004fdab0,
TransformVertexGroups 0x00440d40 (notes +0x1a4, which must be cleared) and SetMatrixIn
0x004fb8c0 (notes the 64-byte matrix handed back). DebugMalloc 0x004a2e20 / DebugFree
0x004a2e60 note size or pointer and line (the __FILE__ pointer differs per object) and
allocate; the outputs and the allocated buffers' contents are compared.
"""
import random
import struct

RETAIL_VA = 0x0042c260
SYMBOL = '?GatherModelTriangles@@YAXPAVTreeModelSource@@PAVTreeSceneNode@@PAH2PAPAHPAPAUTreeVec3@@@Z'
SOURCE = 'src/bringup/Group0_GatherModelTriangles.cpp'
PROBES = {0x0042c309: 'frame count', 0x0042c6af: 'keep alloc', 0x0042c8ab: 'restore'}
GRID = [0.0, -0.0, 1.0, -1.0, 0.5, 2.0]


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), frame=rng.random() < 0.6,
                   meshes=rng.choice([0, 1, 1, 2, 3]), lod=rng.randrange(2))


def setup(ctx, c):
    r = random.Random(c['seed'])
    model = ctx.alloc(0x2a0, fill=0x5a)
    ctx.pack(model + 0x1a4, 'i', 0x1234)
    frame = ctx.alloc(0x40) if c['frame'] else 0
    other = ctx.alloc(0x40)

    def vertices(count):
        base = ctx.alloc(0x20 * max(count, 1), fill=0x3c)
        for v in range(count):
            p = [r.choice(GRID) for _ in range(3)]
            if r.random() < 0.03:
                p[r.randrange(3)] = float('nan')
            ctx.pack(base + 0x20 * v, '3f', *p)
        return base

    lods = ctx.alloc(16)
    for lod in range(2):
        count = c['meshes'] if lod == c['lod'] else r.randrange(3)
        meshes = ctx.alloc(0x38 * max(count, 1), fill=0x4b)
        for m in range(count):
            mesh = meshes + 0x38 * m
            nverts = r.randrange(3, 12)
            ntris = r.randrange(0, 9)
            main, alt = vertices(nverts), vertices(nverts)
            parts = ctx.alloc(0x14 * 3)
            nparts = r.randrange(0, 4)
            for k in range(nparts):
                first = r.choice([0, 1, r.randrange(nverts)])
                length = r.choice([nverts - first, r.randrange(0, nverts - first + 1)])
                owner = r.choice([frame, frame, other, 0])
                ctx.pack(parts + 0x14 * k, 'IiIii', owner, length, main + 0x20 * first, 0, 0)
            tris = ctx.alloc(6 * max(ntris, 1))
            for t in range(ntris):
                ctx.pack(tris + 6 * t, '3H', *[r.randrange(nverts) for _ in range(3)])
            ctx.pack(mesh, 'iIiiIIiI', nparts, parts, nverts, ntris, main, alt, 0, tris)
        ctx.pack(lods + 8 * lod, 'iI', count, meshes)
    ctx.pack(model + 0x27c, 'i', c['lod'])
    ctx.pack(model + 0x28c, 'I', lods)

    def get_matrix(cx, ecx, a):
        cx.pack(a[1], '16f', *[r.uniform(-2, 2) for _ in range(16)])

    def set_matrix(cx, ecx, a):
        cx.note('SetMatrixIn', cx.read(a[1], 64).hex())

    def transform_groups(cx, ecx, a):
        cx.note('field_0x1a4', cx.unpack(ecx + 0x1a4, 'i')[0])

    def malloc(cx, ecx, a):
        cx.note('DebugMalloc', a[0], a[2])
        return cx.alloc(max(a[0], 4), fill=0xdd)

    def free(cx, ecx, a):
        cx.note('DebugFree', a[0], a[2])

    ctx.stub_at(0x004fca80, 'GetMatrixIn', pop=8, ret=get_matrix)
    ctx.stub_at(0x004fc050, 'SetAxesIn', pop=20)
    ctx.stub_at(0x004fc740, 'SetPositionInFrame', pop=8)
    ctx.stub_at(0x004fdab0, 'MarkSubtreeDirty')
    ctx.stub_at(0x00440d40, 'TransformVertexGroups', pop=4, ret=transform_groups)
    ctx.stub_at(0x004fb8c0, 'SetMatrixIn', pop=8, ret=set_matrix)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(0x004a2e60, 'DebugFree', nargs=3, ret=free, log=False)
    outs = ctx.alloc(16, fill=0x77)
    return dict(args=[model, frame, outs, outs + 4, outs + 8, outs + 12], ret_eax=False)
