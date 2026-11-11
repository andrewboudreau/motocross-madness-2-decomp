"""D3DIMSoultreeShadow slot 30 0x00446f40 (tools/emu_diff.py case).

The caster's current LOD holds a few meshes. The stubbed light-space helpers (0x004a1b00 ortho in
mode 3, 0x004a1a50 projective otherwise) put every vertex on and around the shadow's clip
rectangle, with x and y on and around texel centres (k + 0.5, one ulp either side, random
fractions) and z on both sides of minDepth. Retail's ClipRectangle::Set 0x00431350 and
ClipPolygon 0x00431680 run for real on a ClipRectangle that g_clipRectangle (0x0056852c) points
at. ShadowFillTriangle 0x00461e60 is stubbed and logs the six rounded points; DebugMalloc /
DebugFree log their size or pointer and line; TransformVertexGroups 0x00440d40 is a stub. The scratch
statics (capacity 0x0059ad28, buffer 0x0059ad2c) start empty, too small or large enough. One
case kind fills the 3000-vertex receiver array (0x00581eb8). Compared: the receiver vertices,
vertexCount, the outcodes (0x0057efd4), the scratch statics and the call sequence.
"""
import struct

RETAIL_VA = 0x00446f40
SYMBOL = '?UnknownVirtualSlot30@D3DIMSoultreeShadow@@UAEXXZ'
SOURCE = 'samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp'
# The candidate's file statics live at retail's addresses, so both runs share the state.
BINDINGS = {'_s_shadowPointCapacity': 0x0059ad28, '_s_shadowPoints': 0x0059ad2c}
PROBES = {0x004473d9: 'fill triangle', 0x0044704f: 'DebugMalloc'}

G_CLIP_RECTANGLE = 0x0056852c
G_TEXEL = 0x0057efa4
CAPACITY, POINTS = 0x0059ad28, 0x0059ad2c
ORTHO = 0x004a1b00           # ShadowTransformPointsOrtho, cdecl, 6 args
PROJECTIVE = 0x004a1a50      # ShadowTransformPointsProjective, cdecl, 6 args
FILL = 0x00461e60            # ShadowFillTriangle(shift, buffer, value, points, flag), cdecl
TRANSFORM_GROUPS = 0x00440d40   # D3DIMShadowCasterView::TransformVertexGroups(int), thiscall ret 4
DEBUG_MALLOC = 0x004a2e20    # DebugMalloc(size, file, line), cdecl
DEBUG_FREE = 0x004a2e60      # DebugFree(p, file, line), cdecl


def bump(x, ulps):
    bits = struct.unpack('<i', struct.pack('<f', x))[0]
    if x < 0:
        ulps = -ulps
    return struct.unpack('<f', struct.pack('<i', bits + ulps))[0]


def depth_near(rng, depth):
    return rng.choice([depth, rng.uniform(depth - 1, depth + 1), bump(depth, -1), depth - 0.5, depth + 0.5])


def coord(rng, lo, hi):
    k = rng.choice([rng.randrange(lo - 3, hi + 4), lo, hi, lo + 1, hi - 1])
    kind = rng.randrange(10)
    if kind < 3:
        return k + 0.5
    if kind < 5:
        return bump(k + 0.5, rng.choice([-1, 1, -2, 2]))
    if kind == 5:
        return float(k)
    return k + rng.random()


def cases(rng, n):
    for i in range(n):
        big = i % 25 == 0
        l, r = sorted(rng.randrange(0, 128) for _ in range(2))
        t, b = sorted(rng.randrange(0, 128) for _ in range(2))
        meshes = []
        for _ in range(1 if big else rng.choice([0, 1, 2, 3])):
            vcount = 60 if big else rng.choice([3, 6, 12, 40])
            tris = []
            for _ in range(2400 if big else rng.choice([0, 1, 4, 10, 30])):
                tri = [rng.randrange(vcount) for _ in range(3)]
                if rng.random() < 0.3:
                    tri = [0, 1, 2] if rng.random() < 0.5 else [2, 1, 0]
                tris += tri
            meshes.append((vcount, tris))
        yield dict(rect=(l, t, r, b), update=rng.random() < 0.8, mode=rng.choice([3, 1]), meshes=meshes,
                   lod=rng.randrange(3), shadow_view=big or rng.random() < 0.9,
                   camera_view=big or rng.random() < 0.7, dirty=rng.choice([0, 1]),
                   scratch=rng.choice(['empty', 'small', 'large']), fmt=rng.choice([0x115c, 0x613]),
                   color=rng.getrandbits(32), depth=-1e30 if big else rng.choice([rng.uniform(-2, 2), 0.0, 0.5]),
                   key=rng.randrange(1 << 16), shift=rng.choice([7, 6]), texel=rng.choice([1 / 128, 1 / 64, 0.3]))


def setup(ctx, case):
    rng = ctx.rng
    l, t, r, b = case['rect']
    scale = 1 if case['update'] else 2
    depth = struct.unpack('<f', struct.pack('<f', case['depth']))[0]

    clip = ctx.alloc(0x70, fill=0x99)
    ctx.pack(G_CLIP_RECTANGLE, 'I', clip)
    ctx.pack(G_TEXEL, 'f', case['texel'])
    if case['scratch'] == 'empty':
        ctx.pack(CAPACITY, 'iI', 0, 0)
    else:
        cap = 2 if case['scratch'] == 'small' else 100
        ctx.pack(CAPACITY, 'iI', cap, ctx.alloc(12 * cap, fill=0xbb))

    pixels = ctx.alloc(0x100)
    lock = ctx.stub('Texture::LockPixels', pop=0xc, ret=pixels)
    unlock = ctx.stub('Texture::UnlockPixels', pop=4)
    vt = ctx.vtable([ctx.stub('Texture::vslot%d' % i) for i in range(13)] + [lock, unlock])
    texture = ctx.alloc(0x80, fill=0x5a)
    ctx.pack(texture, 'I', vt)
    ctx.pack(texture + 0x14, 'i', 1 << case['shift'])

    shadow = ctx.alloc(0x134, fill=0x11)
    ctx.pack(shadow + 0x54, 'I', texture)
    ctx.pack(shadow + 0x68, '4i', l, t, r, b)
    ctx.pack(shadow + 0x88, 'iif', case['shift'], case['mode'], depth)
    ctx.pack(shadow + 0xac, 'Ii', case['color'], case['fmt'])
    ctx.pack(shadow + 0xb4, 'H', case['key'])
    ctx.pack(shadow + 0x11c, 'i', int(case['update']))

    positions = {}
    meshes = ctx.alloc(0x38 * max(len(case['meshes']), 1), fill=0x33)
    for k, (vcount, tris) in enumerate(case['meshes']):
        verts = ctx.alloc(0x20 * vcount)
        pts = []
        for v in range(vcount):
            ctx.pack(verts + 0x20 * v, '3f', *(rng.uniform(-100, 100) for _ in range(3)))
            pts.append((coord(rng, l * scale, r * scale), coord(rng, t * scale, b * scale),
                        depth_near(rng, depth)))
        positions[verts] = pts
        triangles = ctx.alloc(2 * max(len(tris), 1), struct.pack('<%dH' % len(tris), *tris))
        ctx.pack(meshes + 0x38 * k + 8, 'iiI', vcount, len(tris) // 3, verts)
        ctx.pack(meshes + 0x38 * k + 0x1c, 'I', triangles)
    lods = ctx.alloc(8 * 3, fill=0x77)
    ctx.pack(lods + 8 * case['lod'], 'iI', len(case['meshes']), meshes)
    caster = ctx.alloc(0x290, fill=0x3c)
    ctx.pack(caster + 0x18c, 'i', case['dirty'])
    ctx.pack(caster + 0x27c, 'i', case['lod'])
    ctx.pack(caster + 0x28c, 'I', lods)

    def transform(name):
        def write(ctx, ecx, a):
            ctx.note(name, a)
            for i, p in enumerate(positions[a[1]][:a[3]]):
                ctx.pack(a[0] + 12 * i, '3f', *p)
        return write

    # cdecl helpers: ECX at the call is scratch, so these log their arguments themselves. The
    # __FILE__ argument is left out: it is the compile path of each object (retail's
    # D:ardvark\VC\krusty2\D3DIMSoultreeShadow.cpp, the near-miss file's path here).
    def fill(ctx, ecx, a):
        ctx.note('ShadowFillTriangle', a[0], a[1], a[2] & 0xffff, a[4], ctx.unpack(a[3], '6i'))

    def malloc(ctx, ecx, a):
        ctx.note('DebugMalloc', a[0], a[2])
        return ctx.alloc(a[0], fill=0xdd)

    def free(ctx, ecx, a):
        ctx.note('DebugFree', a[0], a[2])
    ctx.stub_at(ORTHO, 'TransformOrtho', nargs=6, ret=transform('ortho'), log=False)
    ctx.stub_at(PROJECTIVE, 'TransformProjective', nargs=6, ret=transform('projective'), log=False)
    ctx.stub_at(FILL, 'ShadowFillTriangle', nargs=5, ret=fill, log=False)
    ctx.stub_at(DEBUG_MALLOC, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(DEBUG_FREE, 'DebugFree', nargs=3, ret=free, log=False)
    ctx.stub_at(TRANSFORM_GROUPS, 'TransformVertexGroups', pop=4)

    this = ctx.alloc(0x48, fill=0x66)
    ctx.pack(this + 0x2c, 'IIii', caster, shadow, int(case['shadow_view']), int(case['camera_view']))
    return dict(ecx=this, args=[], ret_eax=False)   # void
