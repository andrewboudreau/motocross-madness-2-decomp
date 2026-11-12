"""EventManager::CreatePodiumScene 0x0045d480, the end-of-event podium (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group1_PodiumScene.cpp. The body decides whether a podium is due
(pro-circuit career: the series' last race; otherwise race mode 2 after the last race, the
showPodiums option and a top-three finish outside network play), closes the race, creates the
podium camera, places the podium from the grid's podium position and direction, from the
event-type-3 node's position and axes, or from the grid spacing (grid row and column from the
racer count), drops it to the ground, then turns the three character offsets and facings, the
camera offset and the look offset about the Y axis by atan2(direction.x, direction.z) and scales
them back to length. It loads up to three winner characters (Winner.mcf or Winnerd.mcf, the
Podium3/4/5 motion by series and rand()), places them, optionally starts the crowd loop and aims
the camera; the vegetation/collision rectangle is 32 x 32 around the podium.

Memory is "universal" (as tools/emu_cases/bike_race_create.py): the replay arena is 16-byte cells
of the generic fake vptr and three cell pointers, and the objects the body walks have that vptr
and cell pointers everywhere else. On top of that the case sets what steers the body:
g_TrackGame's pro circuit (none, or one whose race index is or is not the series' last), race
mode, races, racesRun, showPodiums, network, results[0].position, event type, soundOn and
networkGame; the grid's racer count (odd and even), podium position and direction (zero or not),
the grid spacing, the race view's node owner (null or not) and the lights (null or not); the
result count (0..5). GetSelectedSeries answers 0..5, InProCircuitCareer 0 or 1, the archive opens
fail 10% of the time, NextBike answers racers with places 1..3 or null, rand a value below 32768.
GetPosition/GetAxes and QueryGround write seeded vectors and heights; D3DRMVectorRotate turns
about the axis and normalises (as D3DRM does). sprintf is modelled and compared through its text.
"""
import math
import random
import struct

RETAIL_VA = 0x0045d480
SYMBOL = '?CreatePodiumScene@EventManager@@QAEHXZ'
SOURCE = 'src/bringup/Group1_PodiumScene.cpp'
PROBES = {0x0045d556: 'podium due', 0x0045d64f: 'grid direction', 0x0045d6a7: 'node axes',
          0x0045d6e0: 'grid spacing', 0x0045d9cb: 'outdoor layout', 0x0045dfac: 'character',
          0x0045e189: 'crowd loop', 0x0045e46f: 'camera aimed'}

ARENA, ARENA_SIZE = 0x50000000, 0x40000
TRACK_GAME = 0x0056e26c
SPRINTF, NEW, RAND = 0x00534aaf, 0x004a3010, 0x00534c1b
ROTATE = 0x0053304c
FIND_MODE, FIND_VIEW = 0x0045d2b0, 0x0045d2f0
GET_POSITION, GET_AXES = 0x004fc970, 0x004fc4f0
QUERY_GROUND = 0x00507c10
SERIES = 0x00524100
IN_CAREER = 0x00521cd0
OPEN_ARCHIVE = 0x004e9cd0
NEXT_BIKE = 0x004204e0
CTORS = (0x004bed80, 0x00460d10, 0x004455b0, 0x004bba10)
# The retail statics the body reads and hands out (kVec3YAxis 0x0059af48, kVec3ZAxis 0x0059af18).
BINDINGS = {'_kVec3YAxis': 0x0059af48, '_kVec3ZAxis': 0x0059af18}
# Callee-popped bytes of the virtual slots the body calls (from the retail call sites): slot 8 the
# camera's SetRenderContext (one argument; it answers the camera), 11 the character's load (six),
# 29 LookAt (a Vector3 by value); slot 5 none.
SLOT_POPS = {8: 4, 11: 0x18, 29: 0xc}

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


def _rotate(ctx, rec):
    """D3DRMVectorRotate(out, v, axis, angle): v turned about the axis, normalised."""
    out, v, axis, angle = rec.args
    vx, vy, vz = ctx.unpack(v, '3f')
    ax, ay, az = ctx.unpack(axis, '3f')
    theta = struct.unpack('<f', struct.pack('<I', angle))[0]
    n = math.sqrt(ax * ax + ay * ay + az * az) or 1.0
    ax, ay, az = ax / n, ay / n, az / n
    c, s = (math.cos(theta), math.sin(theta)) if math.isfinite(theta) else (1.0, 0.0)
    d = ax * vx + ay * vy + az * vz
    r = [vx * c + (ay * vz - az * vy) * s + ax * d * (1 - c),
         vy * c + (az * vx - ax * vz) * s + ay * d * (1 - c),
         vz * c + (ax * vy - ay * vx) * s + az * d * (1 - c)]
    m = math.sqrt(sum(x * x for x in r)) if all(math.isfinite(x) for x in r) else 0.0
    ctx.pack(out, '3f', *((x / m if m and math.isfinite(m) else 0.0) for x in r))
    return out


def _this(ctx, rec):
    return rec.ecx


def _vec(rng, scale):
    return [rng.uniform(-scale, scale) for _ in range(3)]


def _get_position(ctx, rec):
    ctx.pack(rec.args[0], '3f', *_vec(rec.rng, 2000.0))


def _get_axes(ctx, rec):
    d = _vec(rec.rng, 1.0) if rec.rng.random() < 0.8 else [0.0, 0.0, rec.rng.choice([1.0, -1.0])]
    ctx.pack(rec.args[0], '3f', *d)
    ctx.pack(rec.args[1], '3f', *_vec(rec.rng, 1.0))


def _query_ground(ctx, rec):
    ctx.pack(rec.args[0] + 4, 'f', rec.rng.uniform(-50.0, 200.0))


def _series(ctx, rec):
    return ctx.data['case']['series'][rec.occurrence % 4]


def _in_career(ctx, rec):
    return ctx.data['case']['career']


def _open(ctx, rec):
    return int(rec.rng.random() >= 0.1)


def _next_bike(ctx, rec):
    return rec.rng.choice(ctx.data['racers'] + [0])


def _rand(ctx, rec):
    return rec.rng.randrange(32768)


REPLAY = dict(returns={SPRINTF: _sprintf, NEW: _new, ROTATE: _rotate, RAND: _rand,
                       FIND_MODE: lambda ctx, rec: ctx.data['owner'],
                       FIND_VIEW: lambda ctx, rec: ctx.data['view'],
                       GET_POSITION: 'void', GET_AXES: 'void', QUERY_GROUND: 'void',
                       SERIES: _series, IN_CAREER: _in_career, OPEN_ARCHIVE: _open,
                       NEXT_BIKE: _next_bike, **{c: _this for c in CTORS}},
              effects={GET_POSITION: _get_position, GET_AXES: _get_axes, QUERY_GROUND: _query_ground},
              pops={SPRINTF: 0, NEW: 0, ROTATE: 16, RAND: 0}, args={SPRINTF: 2, NEW: 3, ROTATE: 4, RAND: 0},
              ecx={SPRINTF: False, NEW: False, ROTATE: False, RAND: False},
              unlogged=(SPRINTF, NEW))


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   circuit=rng.choice(['none', 'none', 'last', 'other']),
                   race_mode=rng.choice([0, 1, 2, 2]), races=rng.randrange(4), races_run=rng.randrange(5),
                   show=rng.choice([0, 1, 1, 1]), network=rng.choice([0, 0, 1]),
                   position=rng.choice([1, 2, 3, 4]), event=rng.choice([0, 1, 3, 3]),
                   sound=rng.choice([0, 1]), net_game=rng.choice([0, 1]),
                   racer_count=rng.choice([1.0, 2.0, 5.0, 8.0, 11.0]),
                   direction=rng.choice(['zero', 'zero', 'zero', 'random', 'axis']),
                   spacing=rng.uniform(0.5, 20.0), node=rng.choice([0, 1, 1, 1]), lights=rng.choice([0, 1, 1]),
                   result_count=rng.choice([0, 1, 1, 2, 3, 3, 5]), career=rng.choice([0, 1]),
                   series=[rng.randrange(6) for _ in range(4)])


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    ctx.data['case'] = c
    vt = ctx.fake_vtable('Object', 0x40, pops=SLOT_POPS, returns={8: _this})
    ctx.write(ARENA, _universe(c['seed'], vt))
    r = random.Random(c['seed'])

    def universal(size):
        at = ctx.alloc(size)
        words = [vt] + [ARENA + 16 * r.randrange(0x2000) for _ in range(size // 4 - 1)]
        ctx.write(at, struct.pack('<%dI' % len(words), *words))
        return at

    game = universal(0x3500)
    ctx.pack(game + 8, 'i', c['network'])
    ctx.pack(game + 0xc3c, 'i', c['show'])
    ctx.pack(game + 0xfa0, 'i', c['sound'])
    ctx.pack(game + 0x291c, 'i', c['net_game'])
    ctx.pack(game + 0x2d70, 'ii', c['race_mode'], c['event'])
    ctx.pack(game + 0x2d7c, 'i', c['races'])
    circuit = 0
    if c['circuit'] != 'none':
        circuit = universal(0x1400)
        series = r.randrange(4)
        track_count, bonus = r.randrange(1, 10), r.choice([0, 0, r.randrange(1, 10)])
        last = bonus - 1 if bonus else track_count
        ctx.pack(circuit + 0x40, 'ii', series, last if c['circuit'] == 'last' else last + 1)
        ctx.pack(circuit + 0x1289 + 16 * series, 'ii', track_count, bonus)
    ctx.pack(game + 0x3444, 'I', circuit)
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(BINDINGS['_kVec3YAxis'], '3f', 0.0, 1.0, 0.0)
    ctx.pack(BINDINGS['_kVec3ZAxis'], '3f', 0.0, 0.0, 1.0)

    model = universal(0x400)
    ctx.pack(model + 0x398, 'f', c['racer_count'])
    grid = universal(0x100)
    ctx.pack(grid + 0x8c, '3f', *_vec(r, 3000.0))
    direction = {'zero': [0.0, 0.0, 0.0], 'random': _vec(r, 1.0),
                 'axis': [0.0, 0.0, -1.0]}[c['direction']]
    ctx.pack(grid + 0x98, '3f', *direction)
    ctx.pack(grid + 0xa4, 'I', model)
    track = universal(0x80)
    ctx.pack(track + 0x40, 'f', c['spacing'])
    race_view = universal(0x100)
    ctx.pack(race_view + 0x4c, 'I', track)
    ctx.pack(race_view + 0x64, 'I', universal(0x200) if c['node'] else 0)
    owner = universal(0x100)
    ctx.pack(owner + 0x2c, 'I', grid)
    ctx.pack(owner + 0x34, 'I', race_view)
    if not c['lights']:
        ctx.pack(owner + 0x7c, 'I', 0)
    ctx.data['owner'] = owner
    ctx.data['view'] = universal(0x100)
    racers = []
    for _ in range(4):
        racer = universal(0x800)
        ctx.pack(racer + 0x784, 'i', r.randrange(1, 4))
        racers.append(racer)
    ctx.data['racers'] = racers

    this = universal(0x500)
    ctx.pack(this + 0x48, 'B', c['races_run'])
    ctx.pack(this + 0x4c, 'i', c['result_count'])
    ctx.pack(this + 0x54, 'i', c['position'])
    return dict(ecx=this, args=[])
