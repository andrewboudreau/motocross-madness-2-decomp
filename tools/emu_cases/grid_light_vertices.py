"""GridVertexSink::LightVertices 0x0049e4a0 and its three workers (tools/emu_diff.py case).

Linked from src/bringup/Group4_GridLighting.cpp. The workers LightAny 0x0049e6b0, LightOne
0x0049f1c0 and LightAmbientPlusOne 0x0049f8d0 are kept from the object (PREFER_OBJECT), so
the candidate run is the whole unit and the retail run is retail's four bodies. The sink
(ECX) holds the light count (+0x2c), light pointers (+0x30), the three per-vertex term
arrays (+0xf8/+0xfc/+0x100) and their capacity (+0x110); a light is type (+0x2c), colour
(+0x54), position (+0x64) and direction (+0x70). Scenes: identity, rigid and non-rigid
matrices; 0 to 64 vertices with unit, scaled and axis-aligned normals (0.6/0.8 ties);
strides 0xc/0x10/0x20 with or without the normals inside the vertex buffer; 0 to 5
lights of every type (1 point, 2 spot, 3 unknown, 4 directional, 6 ambient); the
optional colour array; a term-array capacity below, at and above the count, so retail's
DebugMalloc/DebugFree regrow path runs as retail code.

Group4Vis_ColorByte (src/bringup/Group4_GridLightColor.cpp, a separate unit without
/QIfist) is retail's inlined `fld; fmul 255.0f; call __ftol`; here it is a stub that
computes the same truncation at the run's precision and is not logged, since retail has
no call there.
"""
import random
import struct

RETAIL_VA = 0x0049e4a0
SYMBOL = '?LightVertices@GridVertexSink@@QAEXPAXH00HH@Z'
SOURCE = 'src/bringup/Group4_GridLighting.cpp'
PREFER_OBJECT = {
    '?LightAny@GridVertexSink@@AAEXPBUMatrix4@@HPAX1H@Z',
    '?LightOne@GridVertexSink@@AAEXPBUMatrix4@@HPAX1H@Z',
    '?LightAmbientPlusOne@GridVertexSink@@AAEXPBUMatrix4@@HPAX1H@Z',
}
# The first stub a run creates sits at emu_diff.STUB + 0x100; setup creates the colour stub first.
COLOR_BYTE_STUB = 0x21000100
BINDINGS = {
    '?Group4Vis_ColorByte@@YAHM@Z': COLOR_BYTE_STUB,
    # the game build's link alias: the Vector3 view of retail's Vec3TransformNormal
    '?Vec3TransformNormal@@YAXPAUVector3@@U1@PBUMatrix4@@@Z': 0x0042a450,
}
PROBES = {0x0049e6b0: 'LightAny', 0x0049f1c0: 'LightOne', 0x0049f8d0: 'LightAmbientPlusOne'}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def unit(r):
    while True:
        v = [r.uniform(-1, 1) for _ in range(3)]
        n = sum(c * c for c in v) ** 0.5
        if 0.1 < n <= 1:
            return [c / n for c in v]


def rotation(r):
    a, b = unit(r), unit(r)
    d = sum(x * y for x, y in zip(a, b))
    b = [y - d * x for x, y in zip(a, b)]
    n = sum(c * c for c in b) ** 0.5
    b = [c / n for c in b]
    return a, b, [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def color_byte(cx, ecx, args):
    from unicorn.x86_const import UC_X86_REG_FPCW
    channel = struct.unpack('<f', struct.pack('<I', args[0]))[0]
    product = channel * 255.0                          # exact in binary64
    if (cx.uc.reg_read(UC_X86_REG_FPCW) >> 8) & 3 == 0:
        product = f32(product)                         # 24-bit precision control
    return int(product) & 0xffffffff                   # __ftol truncates


def setup(ctx, seed):
    assert ctx.stub('Group4Vis_ColorByte', ret=color_byte, nargs=1, log=False) == COLOR_BYTE_STUB
    r = random.Random(seed)
    identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
    kind = r.random()
    if kind < 0.15:
        m = identity
    else:
        a, b, c = rotation(r)
        m = a + [0] + b + [0] + c + [0] + [r.uniform(-500, 500) for _ in range(3)] + [1]
        if kind > 0.9:
            m = [x * r.uniform(0.5, 1.5) for x in m]
    axis = r.random() < 0.25
    if axis:
        m = identity
    count = r.choice([0, 1, 2, 15, 16, 17, 31, 33, 40, 64])
    stride = r.choice([0x20, 0x20, 0x20, 0xc, 0x10])
    shared = stride == 0x20 and r.random() < 0.7
    verts = []
    for _ in range(count):
        n = unit(r)
        if axis:
            n = r.choice([[0, 1, 0], [0, -1, 0], [1, 0, 0], [0.6, 0.8, 0], [0, 0.8, 0.6], [0.5, 0.5, 0.5]])
        if r.random() < 0.1:
            n = [x * r.uniform(0.8, 1.2) for x in n]
        verts.append(([r.uniform(-300, 300) for _ in range(3)], n, r.getrandbits(32)))
    shape = r.random()
    if shape < 0.3:
        types = [r.choice([1, 2, 3, 4, 6])]
    elif shape < 0.6:
        types = [6, r.choice([1, 2, 4, 6])]
        r.shuffle(types)
    elif shape < 0.7:
        types = [r.choice([1, 2, 4]), r.choice([1, 2, 4])]
    else:
        types = [r.choice([1, 2, 4, 6]) for _ in range(r.randint(0, 5))]

    sink = ctx.alloc(0x200, fill=0x5a)
    lights = []
    for t in types:
        col = [r.choice([r.random(), 1.0, 0.0, 0.5, r.uniform(0, 1.3)]) for _ in range(3)]
        d = unit(r)
        if axis:
            d = r.choice([[0, -1, 0], [0, 1, 0], [-0.6, -0.8, 0], [-1, 0, 0]])
            col = [r.choice([0.5, 1.0, 0.25, 0.75, 2.0 / 255, 0.2]) for _ in range(3)]
        e = ctx.alloc(0xb0, fill=0x33)
        ctx.pack(e + 0x2c, 'i', t)
        ctx.pack(e + 0x54, '4f', *col, 1.0)
        ctx.pack(e + 0x64, '3f', *[r.uniform(-600, 600) for _ in range(3)])
        ctx.pack(e + 0x70, '3f', *d)
        lights.append(e)
    ctx.pack(sink + 0x2c, 'i', len(lights))
    if lights:
        ctx.pack(sink + 0x30, '%dI' % len(lights), *lights)
    cap = r.choice([count, count + 5, 64])
    size = 12 * max(cap, 1) + 16
    terms = ctx.alloc(size, bytes((i * 7 + 3) & 0xff for i in range(size)))
    ctx.pack(sink + 0xf8, '3I', terms, terms + 4 * cap, terms + 8 * cap)
    ctx.pack(sink + 0x110, 'ii', cap, 7)

    matrix = ctx.alloc(64)
    ctx.pack(matrix, '16f', *m)
    buf = ctx.alloc(0x20 * count + 0x40)
    for k, (p, n, _) in enumerate(verts):
        ctx.pack(buf + 0x20 * k, '3f3fII', *p, *n, 0xdeadbeef, 0x12345678)
    if shared:
        normals = buf + 0xc
    else:
        normals = ctx.alloc(stride * count + 16)
        for k, (_, n, _) in enumerate(verts):
            ctx.pack(normals + stride * k, '3f', *n)
    colors = 0
    if r.random() < 0.5:
        colors = ctx.alloc(0x20 * count + 16)
        for k, (_, _, c) in enumerate(verts):
            ctx.pack(colors + 0x20 * k + 0x10, 'I', c)
    return dict(ecx=sink, args=[matrix, count, normals, buf, stride, colors], ret_eax=False)
