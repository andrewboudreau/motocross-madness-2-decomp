"""EcoSystem::Render3D 0x0045b060, the vegetation draw (tools/emu_diff.py case, call-record/replay
mode).

The linked body is src/bringup/Group4_EcoSystem.cpp. FastInvSqrt 0x00460c00, its estimate
0x00460c70 (tables built by 0x00460ae0/0x00460bb0) and __ftol run natively; every other call is
replayed. The render target (+0x18) has a fake vtable whose GetRenderState (+0x24) writes the
state's value, and whose DrawIndexedPrimitive (+0x3c) notes the vertex and index buffers it is
handed. RadiusForParameter 0x00455ff0 and HeightForParameter 0x00455f90 answer seeded floats.

Inputs: a camera direction (random, axis-aligned or vertical, which zeroes the xz length),
light and ambient colours that saturate or not, a light direction, 0..4 geometry objects and
0..300 billboards (so the 120-billboard batches split), seeded depth keys around the band's
range and the near-distance test, faded and opaque billboards, both detail bands' switches, the
blade renderer and the debug overlay on or off.
"""
import math
import random

RETAIL_VA = 0x0045b060
SYMBOL = '?Render3D@EcoSystem@@UAEHXZ'
SOURCE = 'src/bringup/Group4_EcoSystem.cpp'
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)
TRACK_GAME = 0x0056e26c
MEM_TAGS = 0x0056df04
DETAIL_BANDS = 0x0059af14
ECO_GLOBAL = 0x0059aebc
ZERO_VEC = 0x0059aea0
RADIUS_FOR, HEIGHT_FOR = 0x00455ff0, 0x00455f90
# The source keeps file-local copies of retail's statics; both bodies use retail's.
BINDINGS = {'_kVec3Zero': ZERO_VEC, '_s_debugPage': 0x0056a128, '_s_drawEnabled': 0x0056a12c,
            '_s_geometryRenderTicks': 0x0059aec0, '_s_prepareGeometryPeak': 0x0059aef0,
            '_s_renderPeak': 0x0059aec8, '_s_renderTicks': 0x0059aee4,
            '_s_prepareGeometryTicks': 0x0059aee8, '_s_geometryDrawn': 0x0059af04,
            '_s_billboardsDrawn': 0x0059af08}
PROBES = {0x0045b291: 'right side normalised', 0x0045b6e0: 'geometry drawn',
          0x0045ba36: 'three-face billboard', 0x0045bd71: 'billboards drawn',
          0x0045bdb9: 'debug page allocated'}


def _radius(ctx, rec):
    return 0, rec.rng.choice([0.0, rec.rng.uniform(0.5, 4.0), rec.rng.uniform(4.0, 20.0)])


def _height(ctx, rec):
    return 0, rec.rng.uniform(2.0, 25.0)


REPLAY = dict(returns={RADIUS_FOR: _radius, HEIGHT_FOR: _height},
              passthrough=(0x00460c00, 0x00460c70))


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def _f(r, lo, hi):
    return r.uniform(lo, hi)


def setup(ctx, seed):
    r = random.Random(seed)
    for init in FAST_TABLE_INITS:
        ctx.call(init)
    ctx.pack(ZERO_VEC, '3f', 0.0, 0.0, 0.0)

    def draw(cx, rec):          # (type, fvf, vertices, vertexCount, indices, indexCount, flags)
        a = rec.args
        nv = a[3] if 0 <= a[3] <= 1000 else 0
        ni = a[5] if 0 <= a[5] <= 3000 else 0
        cx.note('DrawIndexedPrimitive', a[0], a[1], nv, ni, a[6],
                cx.read(a[2], 32 * nv).hex(), cx.read(a[4], 2 * ni).hex())
        return 0

    def get_state(cx, rec):     # (state, int* out)
        cx.pack(rec.args[1], 'I', 0x100 + rec.args[0])
        return 0

    view_vt = ctx.fake_vtable('render target', 24,
                              pops={7: 12, 8: 12, 9: 8, 15: 28, 19: 0},
                              returns={9: get_state, 15: draw, 7: 'int', 8: 'int', 19: 'void'})
    camera_vt = ctx.fake_vtable('camera', 40, pops={30: 4}, returns={30: 'void'})
    texture_vt = ctx.fake_vtable('texture', 24, returns={19: 'void'})

    camera = ctx.alloc(0x200)
    ctx.pack(camera, 'I', camera_vt)
    kind = r.choice(['random', 'random', 'random', 'x', 'z', 'vertical'])
    if kind == 'random':
        d = [_f(r, -1, 1), _f(r, -0.5, 0.5), _f(r, -1, 1)]
    elif kind == 'vertical':
        d = [0.0, -1.0, 0.0]
    else:
        d = [float(kind == 'x') * r.choice([1.0, -1.0]), 0.0, float(kind == 'z') * r.choice([1.0, -1.0])]
    ctx.pack(camera + 0x17c, '3f', *d)
    ctx.pack(camera + 0x198, 'f', _f(r, 200.0, 800.0))

    view = ctx.alloc(0x80)
    ctx.pack(view, 'I', view_vt)
    ctx.pack(view + 8, 'I', camera)
    ctx.pack(view + 0xc, 'i', r.choice([320, 640, 800, 1024]))
    ctx.pack(view + 0x38, 'i', r.randrange(100000))
    ctx.pack(view + 0x44, 'i', r.randrange(100000))

    eco = ctx.alloc(0x600)
    ctx.pack(eco + 0x18, 'I', view)
    # light colour, ambient, light direction
    ctx.pack(eco + 0x564, '3f', *[_f(r, 0.0, 1.2) for _ in range(3)])
    ctx.pack(eco + 0x570, '3f', *[_f(r, 0.0, 0.6) for _ in range(3)])
    ld = [_f(r, -1, 1), _f(r, -1, 0.2), _f(r, -1, 1)]
    k = 1.0 / math.sqrt(sum(c * c for c in ld))
    ctx.pack(eco + 0x57c, '3f', *[c * k for c in ld])
    ctx.pack(eco + 0x34, 'i', r.randrange(5000))

    # definitions (the object's table at +0x58, indexed by the vegetation byte +0x12)
    definitions = []
    for i in range(4):
        d_ = ctx.alloc(0x200)
        u_left, u_right = sorted([_f(r, 0, 1), _f(r, 0, 1)])
        ctx.pack(d_ + 0x1b8, 'f', u_left)
        ctx.pack(d_ + 0x1bc, 'f', u_right if u_right > u_left else u_left + 0.25)
        for off in (0x1c0, 0x1c4, 0x1c8, 0x1cc, 0x1d0):
            ctx.pack(d_ + off, 'f', _f(r, 0, 1))
        ctx.pack(d_ + 0x1d4, 'f', _f(r, -0.3, 0.1))
        ctx.pack(d_ + 0x1d8, 'i', r.choice([0, 1, 2]))
        tex = ctx.alloc(0x40)
        ctx.pack(tex, 'I', texture_vt)
        ctx.pack(tex + 0x20, 'i', r.choice([1, 2, 3]))
        ctx.pack(d_ + 0x1e0, 'II', tex, tex)
        ctx.pack(eco + 0x58 + 4 * i, 'I', d_)
        definitions.append(d_)

    def vegetation():
        v = ctx.alloc(0x20, bytes(r.getrandbits(8) for _ in range(0x20)))
        ctx.pack(v + 6, 'H', r.choice([0, r.randrange(0x10000), r.randrange(0x800)]))
        ctx.pack(v + 0xc, '3H', *[r.randrange(0x10000) for _ in range(3)])
        ctx.pack(v + 0x12, 'B', r.randrange(4))
        return v

    vegetation_array = vegetation()
    ctx.pack(eco + 0x38, 'I', vegetation_array)
    geometry = r.choice([0, 0, 1, 4])
    glist = ctx.alloc(4 * max(geometry, 1))
    for i in range(geometry):
        ctx.pack(glist + 4 * i, 'I', ctx.alloc(0x40))
    ctx.pack(eco + 0x40, 'I', glist)
    ctx.pack(eco + 0x5a4, 'i', geometry)
    billboards = r.choice([0, 1, 5, 40, 120, 121, 300])
    blist = ctx.alloc(4 * max(billboards, 1))
    for i in range(billboards):
        ctx.pack(blist + 4 * i, 'I', vegetation())
    ctx.pack(eco + 0x3c, 'I', blist)
    ctx.pack(eco + 0x5a0, 'i', billboards)
    ctx.pack(eco + 0x50, 'I', ctx.alloc(120 * 0xc0 + 0x40, fill=0x5a))
    ctx.pack(eco + 0x54, 'I', ctx.alloc(120 * 12 * 2 + 0x40, fill=0x5b))
    ctx.pack(eco + 0x5bc, 'f', r.choice([0.01, 0.05, 0.1]))
    ctx.pack(eco + 0x5c0, 'i', r.randrange(3))

    bands = ctx.alloc(3 * 0x20)
    for i in range(3):
        b = bands + 0x20 * i
        ctx.pack(b + 0xc, 'iii', r.choice([100, 500, 2000]), r.choice([0, 1]), r.choice([0, 1]))
        ctx.pack(b + 0x1c, 'i', r.randrange(8))
    ctx.pack(DETAIL_BANDS, 'I', bands)
    eco_global = ctx.alloc(0x600)
    ctx.pack(eco_global + 0x5a8, 'f', r.choice([0.05, 0.1, 0.25]))
    ctx.pack(ECO_GLOBAL, 'I', eco_global)

    game = ctx.alloc(0x400)
    ctx.pack(game + 0x2d0, 'i', r.choice([0, 1]))
    overlay = 0
    if r.random() < 0.5:
        overlay = ctx.alloc(0x2800)
        ctx.pack(overlay + 0x26c0, 'i', r.randrange(10))
    ctx.pack(game + 0x38, 'I', overlay)
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(MEM_TAGS, 'I', ctx.alloc(0x40))
    return dict(ecx=eco, args=[])
