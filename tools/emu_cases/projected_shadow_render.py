"""ProjectedShadow::RenderShadow 0x004db8c0 (tools/emu_diff.py case).

The texture lock returns a random 16-bit height map (sometimes 0, the early return); the old
dirty rectangle is random (sometimes empty) so the colorKeyLow clear runs over odd and even
spans and alignments. Each caster part's projected vertices are written by the stubbed
transform helpers (0x004a1b00 ortho in mode 3, 0x004a1a50 projective otherwise) and sit on
and around texel centres and the (1, size-1) inside limits; some triangles are degenerate or
nearly so, to probe the signed-area cull. ShadowFillTriangle 0x00461e60 is stubbed and logs the
six rounded points it is handed. Compared: the cleared texels, clipRect, minDepth, heightMap
and the call sequence (lock, parts, transforms, fills, ReleaseParts, unlock).
"""
import struct

RETAIL_VA = 0x004db8c0
SYMBOL = '?RenderShadow@ProjectedShadow@@QAEXXZ'
SOURCE = 'samples/physics/shadow/ProjectedShadowNearMisses.cpp'
PROBES = {0x004dbb5c: 'x fistp', 0x004dbc7d: 'fill triangle'}

PART_COUNT = 0x00445030      # ShadowCaster::PartCount(int), thiscall ret 4
GET_PART = 0x004433f0        # ShadowCaster::GetPart(...), thiscall ret 0x1c
ORTHO = 0x004a1b00           # ShadowTransformPointsOrtho, cdecl, 6 args
PROJECTIVE = 0x004a1a50      # ShadowTransformPointsProjective, cdecl, 6 args
FILL = 0x00461e60            # ShadowFillTriangle(shift, buffer, value, points, flag), cdecl
RELEASE_PARTS = 0x004fdab0   # ShadowCaster::ReleaseParts(), thiscall


def bump(x, ulps):
    bits = struct.unpack('<i', struct.pack('<f', x))[0]
    if x < 0:
        ulps = -ulps
    return struct.unpack('<f', struct.pack('<i', bits + ulps))[0]


def coord(rng, size):
    k = rng.choice([rng.randrange(2, size - 2)] * 4 + [rng.randrange(0, size + 2), 1, 2, size - 1, size - 2])
    kind = rng.randrange(10)
    if kind < 3:
        return k + 0.5
    if kind < 5:
        return bump(k + 0.5, rng.choice([-1, 1, -2, 2]))
    if kind == 5:
        return float(k)
    if kind == 6:
        return rng.choice([2.5, 1.5, size - 1.5, size - 0.5, 3e9, -3e9, float('nan')])
    return k + rng.random()


def cases(rng, n):
    for _ in range(n):
        shift = rng.choice([7, 7, 6, 5, 4])
        size = 1 << shift
        casters = []
        for _ in range(rng.choice([0, 1, 1, 2, 3])):
            parts = []
            for _ in range(rng.choice([0, 1, 1, 2])):
                count = rng.choice([3, 4, 8, 16])
                parts.append((count, 3 * rng.choice([0, 1, 4, 10])))
            casters.append((parts, rng.choice([-1, -1, 0, 5])))
        if rng.random() < 0.2:
            rect = (rng.randrange(size), rng.randrange(size), 0, 0)   # left > right: no clear
        else:
            l, r = sorted(rng.randrange(size) for _ in range(2))
            t, b = sorted(rng.randrange(size) for _ in range(2))
            rect = (l, t, r, b)
        yield (shift, size, rng.choice([3, 3, 1, 0]), casters, rect, rng.random() < 0.05,
               rng.randrange(1 << 16), rng.getrandbits(32), rng.randrange(1 << 16))


def setup(ctx, case):
    shift, size, mode, casters, rect, lock_fails, key, pair, pixel = case
    rng = ctx.rng
    cells = size * size + 16
    hm = ctx.alloc(2 * cells)
    ctx.write(hm, struct.pack('<%dH' % cells, *[rng.randrange(1 << 16) for _ in range(cells)]))

    lock = ctx.stub('Texture::LockPixels', pop=0xc, ret=0 if lock_fails else hm)
    unlock = ctx.stub('Texture::UnlockPixels', pop=4)
    vt = ctx.vtable([ctx.stub('Texture::vslot%d' % i) for i in range(13)] + [lock, unlock])
    texture = ctx.alloc(0x80, fill=0x5a)
    ctx.pack(texture, 'I', vt)
    ctx.pack(texture + 0x14, 'i', size)

    total = sum(c for parts, _ in casters for c, _ in parts) + 1
    vb = ctx.alloc(12 * total, fill=0xee)
    parts_of, part_data, positions = {}, {}, {}
    array = ctx.alloc(4 * max(len(casters), 1))
    for ci, (parts, flag) in enumerate(casters):
        c = ctx.alloc(0x280, fill=0x3c)
        ctx.pack(c + 0x27c, 'i', flag)
        ctx.pack(array + 4 * ci, 'I', c)
        parts_of[c] = len(parts)
        for pi, (count, index_count) in enumerate(parts):
            pts = []
            for _ in range(count):
                pts.append([coord(rng, size), coord(rng, size),
                            rng.choice([rng.uniform(-10, 10), 0.5, 1.0])])
            if count >= 3 and rng.random() < 0.3:
                # A (nearly) collinear vertex: the signed area is 0 or a rounding residue.
                t = rng.random()
                a, b = pts[0], pts[1]
                pts[2][0] = a[0] + t * (b[0] - a[0])
                pts[2][1] = a[1] + t * (b[1] - a[1])
            src = ctx.alloc(0x20 * count, fill=0x77)
            idx = [rng.randrange(count) for _ in range(index_count)]
            if index_count and rng.random() < 0.5:
                idx[:3] = [0, 1, 2] if rng.random() < 0.5 else [2, 1, 0]
            indices = ctx.alloc(2 * max(index_count, 1), struct.pack('<%dH' % index_count, *idx))
            part_data[(c, pi)] = (src, count, indices, index_count)
            positions[src] = pts

    def part_count(ctx, ecx, a):
        return parts_of[ecx]

    def get_part(ctx, ecx, a):
        src, count, indices, index_count = part_data[(ecx, a[0])]
        ctx.pack(a[1], 'I', src)
        ctx.pack(a[2], 'i', count)
        ctx.pack(a[3], 'I', indices)
        ctx.pack(a[4], 'i', index_count)

    # cdecl helpers: ECX at the call is scratch, so these log their arguments themselves.
    def transform(name):
        def write(ctx, ecx, a):
            ctx.note(name, a)
            for i, p in enumerate(positions[a[1]][:a[3]]):
                ctx.pack(a[0] + 12 * i, '3f', *p)
        return write

    def fill(ctx, ecx, a):
        ctx.note('ShadowFillTriangle', a[0], a[1], a[2] & 0xffff, a[4], ctx.unpack(a[3], '6i'))
    ctx.stub_at(PART_COUNT, 'PartCount', pop=4, ret=part_count)
    ctx.stub_at(GET_PART, 'GetPart', pop=0x1c, ret=get_part)
    ctx.stub_at(ORTHO, 'TransformOrtho', nargs=6, ret=transform('ortho'), log=False)
    ctx.stub_at(PROJECTIVE, 'TransformProjective', nargs=6, ret=transform('projective'), log=False)
    ctx.stub_at(FILL, 'ShadowFillTriangle', nargs=5, ret=fill, log=False)
    ctx.stub_at(RELEASE_PARTS, 'ReleaseParts')

    this = ctx.alloc(0x134, fill=0x11)
    ctx.pack(this + 0x2c, 'Ii', array, len(casters))
    ctx.pack(this + 0x54, 'I', texture)
    ctx.pack(this + 0x68, '4i', *rect)
    ctx.pack(this + 0x88, 'iif', shift, mode, rng.uniform(-5, 5))
    ctx.pack(this + 0x94, 'H', pixel)
    ctx.pack(this + 0xa8, 'I', vb)
    ctx.pack(this + 0xb4, 'HHI', key, 0xcccc, pair)
    return dict(ecx=this, args=[], ret_eax=False)   # void
