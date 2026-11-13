"""TerrainShadow slot 27 0x00508bc0, the shadow's footprint on the terrain (tools/emu_diff.py case,
call-record/replay mode).

The linked body is samples/physics/shadow/TerrainShadow.cpp. The body resets the grid and screen
rectangles (+0x6658..+0x6674), then casts the light frustum's corner rays at the height field:
in mode 1 (perspective, ProjectedShadow +0x8c) from the light camera's eye to four corners at
look * distance (+0x198) and half the texture size; in other modes from four near corners to
four far corners 10000 along look, scaled by lensScale (+0xbc), walking each ray on by 3 * look
after every hit. Each hit grows the grid rectangle by the hit cell's gridX/gridZ (+0x24/+0x26)
and, through the visibility clipper's ProjectVertices (0x0052f190 on 0x00575a98), the screen
rectangle. The footprint test against the viewer's width and height (+0xc/+0x10) decides the
return; a miss clears cellCount and vertexCount.

CastSegment 0x00506e90 hits 70% of the time (mode 1) or until a 40% miss (the walking rays),
writing a seeded hit point and one of a few cells; ProjectVertices writes a seeded screen
position around the viewport. The out-of-line constructor 0x00404e60, sum 0x00421cb0 and
difference 0x00421d00 run natively. The light camera's vectors are random, axis-aligned or
nearly parallel (look x up near zero). Compared: the rectangles, the counts, the corner and
step points handed to CastSegment and the hits handed to ProjectVertices, and EAX.
"""
import random

RETAIL_VA = 0x00508bc0
SYMBOL = '?IsShadowVisible@TerrainShadow@@UAEHXZ'
SOURCE = 'samples/physics/shadow/TerrainShadow.cpp'
CLIPPER = 0x00575a98
CAST = 0x00506e90
PROJECT = 0x0052f190
PASS = (0x00404e60, 0x00421cb0, 0x00421d00)
PROBES = {0x005090b8: 'mode 1 visible', 0x005090c5: 'mode 2', 0x005096bb: 'ray step',
          0x005097aa: 'mode 2 visible', 0x005097ba: 'not visible'}


def _cast(ctx, rec):
    mode1 = ctx.data['mode'] == 1
    hit = rec.rng.random() < (0.7 if mode1 else 0.6)
    if hit:
        ctx.pack(rec.args[2], '3f', *[rec.rng.uniform(-500.0, 500.0) for _ in range(3)])
        ctx.pack(rec.args[3], 'I', rec.rng.choice(ctx.data['cells']))
    return int(hit)


def _project(ctx, rec):
    w, h = ctx.data['viewport']
    ctx.pack(rec.args[4], '3f', rec.rng.uniform(-40.0, w + 40.0), rec.rng.uniform(-40.0, h + 40.0),
             rec.rng.uniform(0.0, 1.0))
    ctx.pack(rec.args[5], 'i', rec.rng.randrange(16))


REPLAY = dict(
    returns={CAST: _cast, PROJECT: 'void'},
    effects={PROJECT: _project},
    pointees={CAST: [(0, 12), (1, 12)], PROJECT: [(3, 12)]},
    passthrough=PASS,
    out_writes=False,
)


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def _vec(r, kind):
    if kind == 'axis':
        v = [0.0, 0.0, 0.0]
        v[r.randrange(3)] = r.choice([1.0, -1.0])
        return v
    return [r.uniform(-1.0, 1.0) for _ in range(3)]


def setup(ctx, seed):
    r = random.Random(seed)
    mode = r.choice([1, 1, 3, 0, 2])
    ctx.data['mode'] = mode
    w, h = r.choice([(640, 480), (320, 240), (1024, 768), (12, 6)])
    ctx.data['viewport'] = (w, h)
    cells = []
    for _ in range(r.choice([1, 3, 6])):
        c = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
        ctx.pack(c + 0x24, 'HH', r.randrange(0x10000) if r.random() < 0.1 else r.randrange(256),
                 r.randrange(256))
        cells.append(c)
    ctx.data['cells'] = cells

    camera = ctx.alloc(0x200, bytes(r.getrandbits(8) for _ in range(0x200)))
    kind = r.choice(['random', 'random', 'axis'])
    look = _vec(r, kind)
    up = _vec(r, kind)
    if r.random() < 0.1:
        up = [c * r.choice([1.0, -1.0]) for c in look]         # parallel: zero cross product
    ctx.pack(camera + 0x170, '3f', *[r.uniform(-3000.0, 3000.0) for _ in range(3)])
    ctx.pack(camera + 0x17c, '3f', *look)
    ctx.pack(camera + 0x188, '3f', *up)
    ctx.pack(camera + 0x198, 'f', r.uniform(1.0, 500.0))
    texture = ctx.alloc(0x40)
    ctx.pack(texture + 0x14, 'i', r.choice([64, 128, 256]))
    shadow = ctx.alloc(0x140, bytes(r.getrandbits(8) for _ in range(0x140)))
    ctx.pack(shadow + 0x54, 'II', texture, camera)
    ctx.pack(shadow + 0x8c, 'i', mode)
    ctx.pack(shadow + 0xbc, 'f', r.choice([1.0, r.uniform(0.1, 300.0)]))

    viewer = ctx.alloc(0x20)
    ctx.pack(viewer + 8, 'Iii', ctx.alloc(0x130), w, h)
    this = ctx.alloc(0x6800, fill=0x3c)
    ctx.pack(this + 0x18, 'I', viewer)
    ctx.pack(this + 0x2c, 'II', ctx.alloc(0x100), shadow)
    ctx.pack(this + 0x6654, 'i', r.randrange(0x300))
    ctx.pack(this + 0x6678, 'i', r.randrange(0x40))
    ctx.pack(CLIPPER, 'I', ctx.alloc(0x40))
    return dict(ecx=this, args=[])
