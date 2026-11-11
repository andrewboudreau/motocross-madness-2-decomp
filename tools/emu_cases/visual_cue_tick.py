"""VisualCue::Tick 0x0048b100, the camera-relative cue arrow (tools/emu_diff.py case).

The candidate is samples/race/Krusty3DObjectsNearMisses.cpp (its bindings file binds kVec3Zero
/ kVec3YAxis to retail's 0x0067c308 / 0x0067c328, which the setup fills, since both are
dynamically initialised). Offsets from the retail body: cue +0x18 owner (+0x08 current camera),
+0x2c model, +0x30 isVisible, +0x34 modelScale, +0x38 target pixels, +0x3c model size, +0x40
arcade view, +0x44 view distance, +0x48/+0x4c screen x/y, +0xa8 cue view, +0xac ground terrain,
+0xb0 built field of view, +0xb4/+0xb8 screen offsets, +0xbc cycleCues, +0xc0 current racer,
+0xcc racers[], +0xf8 racer count; arcade view +0xac.. camera matrix (rows 0x10 apart), +0x16c
field of view, +0x170 camera position, +0x198 focal length, +0x1b8 aspect; cue view +0x38 own
racer (+0x10c track point, +0x744 status node with progress +0x44, +0x78c onTrack, +0x7a4
finished), +0x48 race track, +0x50 race camera (+0x3b0 followed bike); g_TrackGame +0x2d74
camera mode (0..5 and out of range), +0x560 baja event (+0x34 view, +0x50 camera, +0x3b4
followed ref with gate +0x7b8; gates at event+0xd8 stride 0x18), +0x2eb4 tag ball, +0x568 tag
event (+0x3c camera with flags byte +0x25 and followed +0x3b4, +0xa8 followed racer with
position +0x0c, +0xdc ball with scene object +0x21c). The field of view equals the built one
in some runs (no rebuild). ArcadeObject::Tick 0x00401800, SetPosition 0x004014f0, SetAxes
0x00401520, Scale 0x004fd340, PointAt 0x00518080, QueryGround 0x00507c10 and GetPosition
0x004fc970 are stubs that note the vectors they are handed (by value, not by address).
"""
import struct

RETAIL_VA = 0x0048b100
SYMBOL = '?Tick@VisualCue@@UAEHM@Z'
SOURCE = 'samples/race/Krusty3DObjectsNearMisses.cpp'
BINDINGS = {'_kVec3Zero': 0x0067c308, '_kVec3YAxis': 0x0067c328}
TRACK_GAME = 0x0056e26c
PROBES = {0x0048b155: 'not own camera', 0x0048b1ac: 'mode 2/3', 0x0048b38a: 'point at',
          0x0048b5fd: 'mode 1/5 gate', 0x0048b67f: 'mode 0', 0x0048b864: 'mode 0 racer', 0x0048bab0: 'tag ball',
          0x0048bb42: 'camera follows', 0x0048bb6b: 'tag racer', 0x0048bb5a: 'hide', 0x0048b223: 'rebuild',
          0x0048bbde: 'plain return'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), tick=rng.random() < 0.93, cycle=rng.choice([0, 1]),
                   mode=rng.choice([0, 1, 2, 3, 4, 5, 0, 2, 4, 6]), same_fov=rng.random() < 0.3,
                   own_camera=rng.random() < 0.9, followed_own=rng.random() < 0.9, own_null=rng.random() < 0.05,
                   finished=rng.choice([0] * 6 + [1, 2]), track=rng.random() < 0.7, on_track=rng.random() < 0.3,
                   racers=rng.choice([0, 1, 1, 2, 3, rng.randrange(12)]), current=rng.choice([-1, 0, 1, 2, rng.randrange(12)]), ref=rng.random() < 0.85,
                   gate=rng.randrange(0, 8), tag_ball=rng.random() < 0.4, flags=rng.randrange(4),
                   tag_followed=rng.choice(['racer', 'racer', 'null', 'same']))


def setup(ctx, c):
    import random
    r = random.Random(c['seed'])
    own_null = c['own_null'] and not c['cycle']     # cycling cues skip the null check: retail would fault

    def block(size, scale=50.0):
        at = ctx.alloc(size)
        ctx.write(at, struct.pack('<%df' % (size // 4), *[r.uniform(-scale, scale) for _ in range(size // 4)]))
        return at

    ctx.pack(0x0067c308, '3f', 0.0, 0.0, 0.0)
    ctx.pack(0x0067c328, '3f', 0.0, 1.0, 0.0)
    view = block(0x200)
    for row in range(3):
        ctx.pack(view + 0xac + 0x10 * row, '3f', *[r.uniform(-1, 1) for _ in range(3)])
    fov = r.choice([30.0, 60.0, 90.0, r.uniform(5, 150)])
    ctx.pack(view + 0x16c, 'f', fov)
    ctx.pack(view + 0x198, 'f', r.choice([1.0, r.uniform(0.2, 4)]))
    ctx.pack(view + 0x1b8, 'f', r.choice([0.75, r.uniform(0.3, 2)]))

    racer = block(0x800)
    node = block(0x60)
    ctx.pack(racer + 0x744, 'I', node)
    ctx.pack(racer + 0x78c, 'i', c['on_track'])
    ctx.pack(racer + 0x7a4, 'B', c['finished'])
    camera = block(0x400)
    cue_view = block(0x60)
    ctx.pack(cue_view + 0x38, 'I', 0 if own_null else racer)
    ctx.pack(cue_view + 0x48, 'I', block(0x40) if c['track'] else 0)
    ctx.pack(cue_view + 0x50, 'I', camera)
    ctx.pack(camera + 0x3b0, 'I', (0 if own_null else racer) if c['followed_own'] else block(0x40))
    owner = block(0x10)
    ctx.pack(owner + 8, 'I', view if c['own_camera'] else block(0x10))

    cue = block(0x100, 3.0)
    ctx.pack(cue + 0x18, 'I', owner)
    ctx.pack(cue + 0x2c, 'I', block(0x10))
    ctx.pack(cue + 0x30, 'i', r.choice([0, 1]))
    ctx.pack(cue + 0x34, 'fff', r.uniform(0, 3), r.choice([32.0, r.uniform(1, 100)]), r.uniform(0.1, 10))
    ctx.pack(cue + 0x40, 'Ifff', view, r.choice([10.0, r.uniform(1, 40)]), r.uniform(0, 1), r.uniform(0, 1))
    ctx.pack(cue + 0xa8, 'II', cue_view, block(0x10))
    ctx.pack(cue + 0xb0, 'f', fov if c['same_fov'] else r.uniform(5, 150))
    ctx.pack(cue + 0xbc, 'ii', c['cycle'], c['current'])
    count = c['racers']
    ctx.pack(cue + 0xf8, 'i', count)
    for i in range(min(count, 11)):
        ctx.pack(cue + 0xcc + 4 * i, 'I', block(0x20))

    game = block(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['mode'])
    baja = block(0x200)
    ctx.pack(game + 0x560, 'I', baja)
    baja_view = block(0x60)
    baja_camera = block(0x400)
    ctx.pack(baja + 0x34, 'I', baja_view)
    ctx.pack(baja_view + 0x50, 'I', baja_camera)
    ref = block(0x800)
    ctx.pack(ref + 0x7b8, 'i', c['gate'])
    ctx.pack(baja_camera + 0x3b4, 'I', ref if c['ref'] else 0)
    ctx.pack(game + 0x2eb4, 'i', c['tag_ball'])
    tag = block(0x100)
    ctx.pack(game + 0x568, 'I', tag)
    tag_camera = block(0x400)
    ctx.pack(tag + 0x3c, 'I', tag_camera)
    ctx.pack(tag_camera + 0x25, 'B', c['flags'])
    tag_racer = block(0x20)
    ctx.pack(tag + 0xa8, 'I', {'racer': tag_racer, 'null': 0, 'same': tag_racer}[c['tag_followed']])
    ctx.pack(tag_camera + 0x3b4, 'I', tag_racer if c['tag_followed'] == 'same' else block(0x20))
    ctx.pack(tag + 0xdc, 'I', block(0x240))
    ctx.pack(TRACK_GAME, 'I', game)

    def vec(cx, at):
        return cx.read(at, 12).hex()

    ctx.stub_at(0x00401800, 'ArcadeObject::Tick', pop=4, ret=int(c['tick']))
    ctx.stub_at(0x004014f0, 'ArcadeObject::SetPosition', pop=4, log=False,
                ret=lambda cx, e, a: cx.note('SetPosition', e, vec(cx, a[0])))

    def point_at(cx, e, a):            # thiscall (UnknownRunwayTrackPos by value, Vector3* out)
        cx.note('PointAt', e, a[0], a[1], a[2])
        cx.pack(a[3], '3f', *[cx.rng.uniform(-50, 50) for _ in range(3)])
        return 1
    ctx.stub_at(0x00518080, 'UnknownRunwayTrack::PointAt', pop=16, nargs=4, ret=point_at, log=False)

    def query_ground(cx, e, a):
        cx.note('QueryGround', e, vec(cx, a[0]), a[1], a[2], a[3])
        cx.pack(a[0] + 4, 'f', cx.rng.uniform(-5, 5))
    ctx.stub_at(0x00507c10, 'UnknownRunwayTerrain::QueryGround', pop=16, nargs=4, ret=query_ground, log=False)
    ctx.stub_at(0x00401520, 'ArcadeObject::SetAxes', pop=16, nargs=4, log=False,
                ret=lambda cx, e, a: cx.note('SetAxes', e, vec(cx, a[0]), vec(cx, a[1]), a[2], a[3]))
    ctx.stub_at(0x004fd340, 'D3DIMSoultreeObject::Scale', pop=12, nargs=3)

    def get_position(cx, e, a):
        cx.note('GetPosition', e)
        cx.pack(a[0], '3f', *[cx.rng.uniform(-50, 50) for _ in range(3)])
    ctx.stub_at(0x004fc970, 'GetPosition', pop=4, ret=get_position, log=False)
    return dict(ecx=cue, args=[r.uniform(0, 0.1)])
