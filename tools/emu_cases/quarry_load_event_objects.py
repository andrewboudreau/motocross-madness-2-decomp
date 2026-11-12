"""BaseQuarryEvent::LoadEventObjects 0x004de590, the event loader (texture budget, audio, camera,
scene, quadtree, lights, shadow, sky, terrain, ecosystem, race, collision ignore lists, podium,
overlays) (tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group3_QuarryLoad.cpp. Memory is "universal" (as
tools/emu_cases/bike_race_create.py): the replay arena is 16-byte cells of the generic fake vptr
and three cell pointers, and the objects the body walks have that vptr and cell pointers
everywhere else. The event itself has its own fake vtable (its slots 27..35 take other arguments
than the other objects' slots of the same number).

The texture budget is driven in full: GlobalMemoryStatus answers a total physical memory around
each threshold (32, 40, 60, 80, 90 MB), the Blade renderer flag, the video memory (the target
display's, or the registry's VideoMemoryMB, -1 or a few MB), the frame-buffer size (bytes per
pixel, buffers, width, height), the platform, driver and device capability bits, the four
estimates (terrain, cube, model textures, race; 0x004e04a0..0x004e0560), the preloaded ecosystem
texture count (0x0059af10), AGP, the forced-quality setting and the registry margins. The scene
(Scene 0x004ea7e0's answer) has 0..4 lights (emitter or none, types 2/4/6/other, the emits,
shadow and lens-flare bits), a fog, an environment whose ecosystem file has or lacks ".est" and
whose cube file is empty or not, 0..3 static models (physics or collision, CountNodes 1 or not)
and 0..3 animation entries (key-framed or procedural cars); NextBike answers 0..3 racers (local or
remote). CountSceneTextures writes the counts, terrain width, cube flag and the ecosystem path;
CountSceneObjects the step count. Archive opens, Scene::Load and BikeRace::Create succeed 97% (the
race 95%) of the time; constructors answer `this`; strstr runs natively; sprintf is modelled and compared
through its text. The progress callback is a cdecl stub (its argument is compared) or null.
"""
import random
import struct

RETAIL_VA = 0x004de590
SYMBOL = '?LoadEventObjects@BaseQuarryEvent@@QAEHP6AXPAH@Z@Z'
SOURCE = 'src/bringup/Group3_QuarryLoad.cpp'
PROBES = {0x004de768: 'blade budget', 0x004de7e0: 'video memory budget', 0x004de910: 'AGP',
          0x004de9b1: 'AGP unforced', 0x004deaf1: 'registry margins', 0x004ded6c: 'cached groups',
          0x004df01f: 'camera created', 0x004df3f3: 'sky', 0x004dfb4f: 'physics caster',
          0x004dfc58: 'key-framed entry', 0x004dfcbf: 'ignore racer', 0x004e0188: 'racer 3D objects',
          0x004e0484: 'exit'}

ARENA, ARENA_SIZE = 0x50000000, 0x40000
TRACK_GAME = 0x0056e26c
PRELOADED = 0x0059af10
MEMORY_STATUS = ('iat', 0x005501c4)
SPRINTF, NEW, STRSTR = 0x00534aaf, 0x004a3010, 0x005362c0
COUNT_TEXTURES, COUNT_OBJECTS = 0x004e9ac0, 0x004e9a10
OPEN_ARCHIVE = 0x004e9cd0
ESTIMATES = (0x004e04a0, 0x004e04e0, 0x004e04f0, 0x004e0560)
BYTES_PER_PIXEL = 0x00511970
SCENE_CREATE, SCENE_LOAD = 0x004ea7e0, 0x004efb20
RACE_CREATE = 0x00417ed0
NEXT_BIKE = 0x004204e0
APPEND_CHILD = 0x00469190
COUNT_NODES = 0x004fda30
CTORS = (0x00401260, 0x00402da0, 0x00417c30, 0x00457250, 0x00460d10, 0x00462620, 0x00462e10,
         0x00462e90, 0x00497cb0, 0x0049e390, 0x004ba390, 0x004da570, 0x004ea660, 0x004fb1e0,
         0x00504490, 0x00505830, 0x00508ae0, 0x0052d340)
# The canonical constructors the body allocates inline, and VisibilityQuadTree::Setup
# (0x0052d460), which the body calls itself. The BikeRace::SetDetailLevel link forward
# (Group3_QuarryLinkRace.cpp, cdecl, `this` first) stands in for retail's thiscall; the body calls
# it itself, so it is placed at a separate address (0x00423791, never executed) and both shapes
# are compared as one note (race, level).
SETUP = 0x0052d460
SET_DETAIL, SET_DETAIL_FORWARD = 0x00423790, 0x00423791
BINDINGS = {'??0ParticleManager@@QAE@H@Z': 0x004ba390, '??0SoultreeQuadTreeRenderer@@QAE@H@Z': 0x00504490,
            '??0Terrain@@QAE@H@Z': 0x00505830, '??0TerrainShadow@@QAE@H@Z': 0x00508ae0,
            '??0VisibilityQuadTree@@QAE@H@Z': 0x0052d340,
            '?Setup@VisibilityQuadTree@@QAEPAVGameObject@@HMMMMM@Z': SETUP,
            '?Create@FogOn@@QAEPAVGameObject@@PAXPAVFog@@@Z': 0x00486a10,
            '?G3Q_BikeRaceSetDetailLevel@@YAXPAXH@Z': SET_DETAIL_FORWARD}
# Callee-popped bytes of the virtual slots (from the retail call sites). The generic objects:
# slot 20 g_TrackGame's GetRegistryInt (two arguments, an int), 8 SetRenderContext (one; it
# answers the object), 27 ParticleManager's Create (four), 2 Release. The event: 27
# Create3DObjects (one, an int), 32 and 33 (one), 28..31 and 35 none.
OBJECT_POPS = {20: 8, 8: 4, 27: 0x10}
EVENT_POPS = {27: 4, 32: 4, 33: 4}

_patterns = {}


def _universe(seed, vt):
    key = (seed, vt)
    if key not in _patterns:
        r = random.Random('universe|%d' % seed)
        _patterns.clear()
        words = []
        for _ in range(ARENA_SIZE // 16):
            words += [vt] + [ARENA + 16 * r.randrange(0x2000) for _ in range(3)]
        _patterns[key] = struct.pack('<%dI' % len(words), *words)
    return _patterns[key]


def _cstr(ctx, at, limit=0x200):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _sprintf(ctx, rec):
    out, fmt = rec.args[0], _cstr(ctx, rec.args[1])
    rest = list(ctx.unpack(rec.esp + 8, '6I'))
    text, i = b'', 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%':
            j = i + 1
            while fmt[j:j + 1].isdigit():
                j += 1
            width, spec = fmt[i + 1:j], fmt[j:j + 1]
            value = rest.pop(0)
            if spec == b's':
                text += _cstr(ctx, value)
            else:
                v = value - (1 << 32) if value & 0x80000000 else value
                text += (('%' + width.decode() + 'd') % v).encode()
            i = j + 1
        else:
            text += fmt[i:i + 1]
            i += 1
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', fmt, text)
    return len(text)


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ARENA + 16 * rec.rng.randrange(0x2000)


def _this(ctx, rec):
    return rec.ecx


def _memory_status(ctx, rec):
    ctx.pack(rec.args[0] + 8, 'I', ctx.data['case']['phys'])


def _count_textures(ctx, rec):
    c = ctx.data['case']
    ctx.pack(rec.args[0], '4I', *[rec.rng.randrange(40) for _ in range(4)])
    ctx.pack(rec.args[1], 'f', c['terrain_width'])
    ctx.pack(rec.args[2], 'i', c['cube'])
    ctx.write(rec.args[3], c['preload'].encode() + b'\0')


def _count_objects(ctx, rec):
    ctx.pack(rec.args[0], 'i', rec.rng.randrange(1, 300))
    return rec.rng.choice([0, 1, 1])


def _registry(ctx, rec):
    name = _cstr(ctx, rec.args[0])
    c = ctx.data['case']
    if name.endswith(b'VideoMemoryMB'):
        return c['registry_mb'] & 0xffffffff
    if name == b'MarginPercentage':
        return c['margin_percent']
    if name == b'MinMarginKBytes':
        return c['margin_kb']
    return rec.args[1]


def _estimate(ctx, rec):
    return ctx.data['case']['estimates'][[0x004e04a0, 0x004e04e0, 0x004e04f0, 0x004e0560].index(rec.target)]


def _mostly(p):
    def f(ctx, rec):
        return int(rec.rng.random() < p)
    return f


def _bpp(ctx, rec):
    return ctx.data['case']['bpp']


def _scene(ctx, rec):
    return ctx.data['scene']


def _race(ctx, rec):
    return rec.ecx if rec.rng.random() < 0.95 else 0


def _next_bike(ctx, rec):
    racers = ctx.data['racers']
    k = ctx.data['bike_index']
    ctx.data['bike_index'] = (k + 1) % (len(racers) + 1)
    return racers[k] if k < len(racers) else 0


def _count_nodes(ctx, rec):
    return rec.rng.choice([1, 1, 2])


def _set_detail(ctx, rec):
    ctx.note('SetDetailLevel', rec.ecx, rec.args[0])


def _set_detail_forward(ctx, rec):
    ctx.note('SetDetailLevel', rec.args[0], rec.args[1])


REPLAY = dict(returns={SPRINTF: _sprintf, NEW: _new, MEMORY_STATUS: 'void', COUNT_TEXTURES: 'void',
                       COUNT_OBJECTS: _count_objects, OPEN_ARCHIVE: _mostly(0.97),
                       **{e: _estimate for e in ESTIMATES}, BYTES_PER_PIXEL: _bpp,
                       SCENE_CREATE: _scene, SCENE_LOAD: _mostly(0.97), RACE_CREATE: _race,
                       APPEND_CHILD: _mostly(0.95),
                       NEXT_BIKE: _next_bike, COUNT_NODES: _count_nodes,
                       SET_DETAIL: _set_detail, SET_DETAIL_FORWARD: _set_detail_forward,
                       **{c: _this for c in CTORS}},
              effects={MEMORY_STATUS: _memory_status, COUNT_TEXTURES: _count_textures},
              pops={SPRINTF: 0, NEW: 0, MEMORY_STATUS: 4, SET_DETAIL: 4, SET_DETAIL_FORWARD: 0},
              args={SPRINTF: 2, NEW: 3, MEMORY_STATUS: 1, SET_DETAIL: 1, SET_DETAIL_FORWARD: 2},
              ecx={SPRINTF: False, NEW: False, MEMORY_STATUS: False, SET_DETAIL: True,
                   SET_DETAIL_FORWARD: False},
              unlogged=(SPRINTF, NEW, SET_DETAIL, SET_DETAIL_FORWARD), passthrough=(STRSTR,))

MB = 1 << 20


def cases(rng, n):
    for _ in range(n):
        phys = rng.choice([32, 40, 60, 80, 90, 128]) * MB + rng.choice([-1, 0, 1, 0x123456])
        yield dict(seed=rng.getrandbits(32), phys=phys & 0xffffffff,
                   blade=rng.choice([0, 0, 0, 1]), agp=rng.choice([0, 1]), forced=rng.choice([0, 1]),
                   video=rng.choice([4, 8, 16, 32]) * MB, registry_mb=rng.choice([-1, -1, 2, 8, 16]),
                   bpp=rng.choice([2, 4]), buffers=rng.choice([0, 1, 2]),
                   size=rng.choice([(640, 480), (800, 600), (1024, 768)]),
                   platform=rng.choice([1, 2]), driver_caps=rng.choice([0, 0x400]),
                   device_caps=rng.choice([0, 0x4000]),
                   estimates=[rng.randrange(0, 12 * MB) for _ in range(4)], preloaded=rng.randrange(20),
                   margin_percent=rng.choice([0, 10, 50]), margin_kb=rng.choice([0, 0x200, 0x4000]),
                   terrain_width=rng.choice([64.0, 128.0, 256.0]), cube=rng.choice([0, 1]),
                   preload=rng.choice(['', 'eco\\quarry.est', 'eco\\quarry.esb']),
                   eco_file=rng.choice(['', 'q.est', 'q.esb']), cube_file=rng.choice(['', 'sky.cub']),
                   lights=[(rng.choice([0, 1, 1]), rng.choice([2, 4, 6, 1]), rng.randrange(32))
                           for _ in range(rng.randrange(5))],
                   casters=[(rng.choice([0, 1]), rng.choice([0, 1])) for _ in range(rng.randrange(4))],
                   entries=[rng.choice([0, 1]) for _ in range(rng.randrange(4))],
                   racers=[rng.choice([0, 0, 1]) for _ in range(rng.randrange(4))],
                   progress=rng.choice([0, 1, 1]), thirty=rng.randrange(-1, 3), gate=rng.randrange(-1, 3),
                   settings=[rng.randrange(3) for _ in range(5)])


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    ctx.data['case'] = c
    vt = ctx.fake_vtable('Object', 0x40, pops=OBJECT_POPS, returns={20: _registry, 8: _this})
    event_vt = ctx.fake_vtable('Event', 0x40, pops=EVENT_POPS)
    # The progress callback is cdecl void(int*): the first call passes the step count (a local,
    # compared by value), the others null. ECX at its calls is scratch.
    def _progress(cx, e, a):
        cx.note('progress', cx.unpack(a[0], 'i')[0] if a[0] else None)
    progress = ctx.stub('progress', pop=0, nargs=1, ret=_progress, log=False) if c['progress'] else 0
    ctx.write(ARENA, _universe(c['seed'], vt))
    r = random.Random(c['seed'])

    def universal(size):
        at = ctx.alloc(size)
        words = [vt] + [ARENA + 16 * r.randrange(0x2000) for _ in range(size // 4 - 1)]
        ctx.write(at, struct.pack('<%dI' % len(words), *words))
        return at

    game = universal(0x3600)
    ctx.pack(game + 0x2d0, 'i', c['blade'])
    ctx.pack(game + 0x434, 'i', c['platform'])
    display = universal(0xa00)
    ctx.pack(display + 0x9f0, 'i', c['agp'])
    ctx.pack(game + 0xc, 'I', display)
    ctx.pack(game + 0x1ed4, '5i', c['forced'], *c['settings'][1:])
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(PRELOADED, 'i', c['preloaded'])

    target_display = universal(0x600)
    ctx.pack(target_display + 0x54, 'i', c['video'])
    ctx.write(target_display + 0x4bc, b'{guid-text}\0')
    ctx.pack(target_display + 0x1b8, 'I', c['driver_caps'])
    target = universal(0x200)
    ctx.pack(target + 4, 'I', target_display)
    w, h = c['size']
    ctx.pack(target + 0xc, 'iii', w, h, c['buffers'])
    ctx.pack(target + 0x164, 'I', c['device_caps'])

    env = universal(0x420)
    ctx.write(env, c['eco_file'].encode() + b'\0')
    ctx.write(env + 0x104, b'quarry.trn\0')
    ctx.write(env + 0x208, c['cube_file'].encode() + b'\0')
    ctx.write(env + 0x30c, b'particle.tga\0')
    ctx.write(env + 0x34c, b'detail.tga\0')
    ctx.pack(env + 0x38c, '4f', 1.0, 1.25, 128.0, 96.0)
    fog = ctx.alloc(0x10, struct.pack('<Iff', 0x8080c0, 1.0, 0.5))
    lights = ctx.alloc(0x8c * max(1, len(c['lights'])))
    for k, (emitter, kind, bits) in enumerate(c['lights']):
        at = lights + 0x8c * k
        ctx.pack(at, 'Ii', universal(0x80) if emitter else 0, kind)
        ctx.pack(at + 0xc, '3f', *[r.uniform(-100.0, 100.0) for _ in range(3)])
        ctx.pack(at + 0x28, '3f', *[r.uniform(-1.0, 1.0) for _ in range(3)])
        ctx.pack(at + 0x3c, 'B', bits)
    casters = ctx.alloc(0x28 * max(1, len(c['casters'])))
    for k, (physics, collision) in enumerate(c['casters']):
        at = casters + 0x28 * k
        ctx.pack(at, 'BxxxIII', physics, universal(0x100), universal(0x200),
                 universal(0x40) if collision else 0)
    static_models = ctx.alloc(8, struct.pack('<iI', len(c['casters']), casters))
    entries = ctx.alloc(0x44 * max(1, len(c['entries'])))
    for k, keyed in enumerate(c['entries']):
        ctx.pack(entries + 0x44 * k, 'BxxxII', 8 if keyed else 0, universal(0x300), universal(0x240))
    table = ctx.alloc(16, struct.pack('<iIiI', len(c['entries']), entries, 0, 0))
    scene = universal(0x700)
    ctx.pack(scene + 0xa4, 'IiIIII', env, len(c['lights']), lights, fog,
             table if c['entries'] else 0, static_models if c['casters'] else 0)
    ctx.data['scene'] = scene

    racers = []
    for remote in c['racers']:
        racer = universal(0x1500)
        ctx.pack(racer + 0x735, 'B', remote)
        racers.append(racer)
    ctx.data['racers'] = racers
    ctx.data['bike_index'] = 0

    this = universal(0x200)
    ctx.pack(this, 'I', event_vt)
    ctx.pack(this + 0x18, 'I', target)
    return dict(ecx=this, args=[progress])
