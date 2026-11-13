"""TerrainShadow slot 30 0x00509aa0 (tools/emu_diff.py case).

A few terrain cells, each with a mesh (sometimes without vertices, sometimes in 16 sections),
are transformed by the stubbed light-space helpers (0x004a1b00 ortho in mode 3, 0x004a1a50
projective otherwise) to points on and around the shadow's clip rectangle, with x and y on
and around texel centres (k + 0.5, one ulp either side, random fractions) and z on both sides
of minDepth. Retail's ClipRectangle::Set 0x00431350 and ClipPolygon 0x00431680 run for real
on a ClipRectangle that g_clipRectangle (0x0056852c) points at, so clipped corners land on the
rectangle's edges (k - 0.5 is a tie there). ShadowFillTriangle 0x00461e60 is stubbed and logs
the six rounded points; AgeManager::MarkUsed 0x00401250 and the texture lock/unlock are stubs.
One case kind fills the 0x300-vertex receiver array. Compared: the receiver vertices and
vertexCount, the clip rectangle object, and the call sequence.
"""
import struct

RETAIL_VA = 0x00509aa0
SYMBOL = '?BuildShadowVertices@TerrainShadow@@UAEXXZ'
SOURCE = 'samples/physics/shadow/TerrainShadow.cpp'
PROBES = {0x00509fd0: 'fill triangle', 0x0050a14e: 'MarkUsed', 0x0050a134: 'vertex array full'}

G_CLIP_RECTANGLE = 0x0056852c
ORTHO = 0x004a1b00           # ShadowTransformPointsOrtho, cdecl, 6 args
PROJECTIVE = 0x004a1a50      # ShadowTransformPointsProjective, cdecl, 6 args
FILL = 0x00461e60            # ShadowFillTriangle(shift, buffer, value, points, flag), cdecl
MARK_USED = 0x00401250       # AgeManager::MarkUsed(AgeEntry*), thiscall ret 4


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


def mesh_case(rng, big=False):
    if not big and rng.random() < 0.08:
        return None                               # mesh->vertices == 0
    vcount = rng.choice([3, 6, 12, 40]) if not big else 60
    sectioned = not big and rng.random() < 0.3
    if sectioned:
        sections, end = [], 0
        for _ in range(16):
            end += 3 * rng.choice([0, 0, 1, 2, 4])
            sections.append((rng.randrange(vcount // 2 + 1), end))
        tri_count = end // 3
        span = vcount - vcount // 2
    else:
        sections = None
        tri_count = 600 if big else rng.choice([1, 4, 10, 30])
        span = vcount
    tris = []
    for _ in range(tri_count):
        t = [rng.randrange(span) for _ in range(3)]
        if rng.random() < 0.3:
            t = [min(k, span - 1) for k in ([0, 1, 2] if rng.random() < 0.5 else [2, 1, 0])]
        tris += t
    return (vcount, sections, tris, 0x08 if sectioned else rng.choice([0, 0xf7]))


def cases(rng, n):
    for i in range(n):
        big = i % 25 == 0
        l, r = sorted(rng.randrange(0, 128) for _ in range(2))
        t, b = sorted(rng.randrange(0, 128) for _ in range(2))
        cells = [(mesh_case(rng, big), 1 if big else rng.choice([0, 1, 1]))
                 for _ in range(1 if big else rng.choice([0, 1, 2, 3]))]
        yield dict(rect=(l, t, r, b), update=rng.random() < 0.8, mode=rng.choice([3, 1]),
                   cells=cells, big=big, fmt=rng.choice([0x115c, 0x613]), color=rng.getrandbits(32),
                   depth=rng.choice([rng.uniform(-2, 2), 0.0, -1e30 if big else 0.5]),
                   key=rng.randrange(1 << 16), shift=rng.choice([7, 6]), texel=rng.choice([1 / 128, 1 / 64, 0.3]))


def setup(ctx, case):
    rng = ctx.rng
    l, t, r, b = case['rect']
    scale = 1 if case['update'] else 2
    depth = struct.unpack('<f', struct.pack('<f', case['depth']))[0]

    clip = ctx.alloc(0x70, fill=0x99)
    ctx.pack(G_CLIP_RECTANGLE, 'I', clip)
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

    age_manager = ctx.alloc(0x40, fill=0x44)
    terrain = ctx.alloc(0xd00, fill=0x22)
    ctx.pack(terrain + 0xc88, 'I', age_manager)

    positions = {}
    keys = []
    for mesh_desc, emit in case['cells']:
        mesh = ctx.alloc(0x180, fill=0x33)
        cell = ctx.alloc(0x44, fill=0x44)
        ctx.pack(cell + 0x38, 'I', mesh)
        ctx.pack(cell + 0x40, 'i', emit)
        keys.append(cell)
        if mesh_desc is None:
            ctx.pack(mesh + 0x134, 'I', 0)
            continue
        vcount, sections, tris, flags = mesh_desc
        ctx.pack(mesh + 0x121, 'B', flags)
        verts = ctx.alloc(0x20 * vcount)
        pts = []
        for v in range(vcount):
            ctx.pack(verts + 0x20 * v, '3f', *(rng.uniform(-100, 100) for _ in range(3)))
            pts.append((coord(rng, l * scale, r * scale), coord(rng, t * scale, b * scale),
                        depth_near(rng, depth)))
        positions[verts] = pts
        indices = ctx.alloc(2 * max(len(tris), 1), struct.pack('<%dH' % len(tris), *tris))
        ctx.pack(mesh + 0x134, 'IIhh', verts, indices, vcount, len(tris))
        if sections:
            rec = ctx.alloc(0xc * 16, fill=0x55)
            for k, (base, end) in enumerate(sections):
                ctx.pack(rec + 0xc * k, 'hh', base, end)
            ctx.pack(mesh + 0x17c, 'I', rec)
        else:
            ctx.pack(mesh + 0x17c, 'I', 0)

    def transform(name):
        def write(ctx, ecx, a):
            ctx.note(name, a[1:])
            for i, p in enumerate(positions[a[1]][:a[3]]):
                ctx.pack(a[0] + 12 * i, '3f', *p)
        return write

    # cdecl helpers: ECX at the call is scratch, so these log their arguments themselves.
    def fill(ctx, ecx, a):
        ctx.note('ShadowFillTriangle', a[0], a[1], a[2] & 0xffff, a[4], ctx.unpack(a[3], '6i'))
    ctx.stub_at(ORTHO, 'TransformOrtho', nargs=6, ret=transform('ortho'), log=False)
    ctx.stub_at(PROJECTIVE, 'TransformProjective', nargs=6, ret=transform('projective'), log=False)
    ctx.stub_at(FILL, 'ShadowFillTriangle', nargs=5, ret=fill, log=False)
    ctx.stub_at(MARK_USED, 'AgeManager::MarkUsed', pop=4)

    this = ctx.alloc(0x6780, fill=0x66)
    ctx.pack(this + 0x2c, 'II', terrain, shadow)
    ctx.pack(this + 0x6678, 'i', len(keys))
    for k, cell in enumerate(keys):
        ctx.pack(this + 0x667c + 4 * k, 'I', cell)
    ctx.pack(this + 0x677c, 'f', case['texel'])
    return dict(ecx=this, args=[], ret_eax=False)   # void
