"""BikeRace::Create 0x00417ed0, the race loader (tracks, start gate, 30-second man, finish arch,
flag girl, counters, VCR tapes, racers, sound, plates) (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group6_BikeRaceCreate.cpp (the quarry loader's
UnknownQuarryRace::Create forward reaches it). Every callee is replayed. The memory the body
walks is "universal": the replay arena is 16-byte cells (the generic fake vptr, then three cell
pointers), and the race object, the scene, its environment, the arguments and g_TrackGame are
objects with that vptr and cell pointers everywhere else, so any pointer chain stays readable and
every virtual call lands on a logged fake slot whose pops and float return come from the retail
call sites. The scene entries (start/finish gates) and the racers carry a virtual base (+4
vbtable, +8 base vptr); the racer slots (+0x3c) carry the network player id and the signed racer
index the multiplayer plates match against. On top of that the case sets what steers the body:
g_TrackGame's event type (0..5 and 7), race mode (0..4), player count, AI and multiplayer AI
counts (0..3), random gates, stunt mode, track name/number, playingReplay, useReplayTape, the
ghost file's first byte, joystickOn, the player indicator, the plate number and the eight racer
slots; KrustyUI's playingDemo; the scene's terrain file (with and without an extension, 0..254
characters). Track::LoadTrack writes random start/finish probes (0x3c bytes each, a 5..60
width at +0x24) and the has-start/has-finish flags; Scene::FindEntryByFileName answers null or
an entry with a small index. sprintf is modelled (%s, %d, %02d) and compared through its text,
D3DRMVectorRotate turns about the axis (kVec3YAxis, set to (0, 1, 0) as its initializer would)
and normalises; strncpy/strrchr run natively; constructors answer `this`. fs:[0] is page 0.
The three objects the body creates through 0x00401310 (the finish arch first) carry a model
width (+0x50) of 5..40. The arch scale (finish width / model width, 0x004187c7) and the flag
girl's and box's offsets built from it are then realistic; with arena cell pointers read as
floats they were around 1e9 and the rotations had a zero axis.
"""
import math
import random
import struct

RETAIL_VA = 0x00417ed0
SYMBOL = ('?Create@BikeRace@@QAEPAV1@PAX0PAVLightManager@@PAUUnknownTextureFormatChoice@@'
          'PAVUnknownBikeRaceProjector@@PAVKrustyBikeCamera@@PAVScene@@PAURaceGateEvent@@'
          'PAVUnknownBikeRaceShadow@@@Z')
SOURCE = 'src/bringup/Group6_BikeRaceCreate.cpp'
PROBES = {0x0041800f: 'tdf from env', 0x004180f3: 'no track file', 0x00418338: 'after load',
          0x00418b06: 'flag girl', 0x00419578: 'AI speeds', 0x00419660: 'joystick',
          0x0041974a: 'replay plates', 0x00419850: 'network plate', 0x004198a7: 'stunt plate',
          0x004198d5: 'AI plates', 0x00419907: 'done'}

ARENA, ARENA_SIZE = 0x50000000, 0x40000
SPRINTF, NEW = 0x00534aaf, 0x004a3010
STRNCPY, STRRCHR = 0x00534b10, 0x00534a60
LOAD_TRACK, FIND_ENTRY = 0x00515ed0, 0x004eaec0
ROTATE = 0x0053304c
CREATE_OBJECT = 0x00401310
TRACK_GAME = 0x0056e26c

_patterns = {}


def _universe(seed, vt):
    """The arena as 16-byte cells: a vptr (the generic fake vtable) and three cell pointers.

    Every 16-aligned arena address, the synthetic pointer results included, is then an object
    whose virtual calls land on the generic fake vtable whatever the slot; the pointers stay in
    the arena's first half, so chains of fields stay mapped.
    """
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
            spec = fmt[i + 1:i + 2]
            width = b''
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


def _load_track(ctx, rec):
    c = ctx.data['case']
    r = rec.rng
    for at in rec.args[1:3]:     # the start and finish probes (0x3c bytes; the width at +0x24)
        values = [r.uniform(-300, 300) for _ in range(15)]
        values[9] = r.uniform(5, 60)
        ctx.pack(at, '15f', *values)
    ctx.pack(rec.args[3], 'i', c['has_start'])
    ctx.pack(rec.args[4], 'i', c['has_finish'])
    return c['track_ok']


def _find_entry(ctx, rec):
    ctx.note('FindEntryByFileName', _cstr(ctx, rec.args[0]))
    ctx.pack(rec.args[1], 'i', rec.rng.randrange(4))
    return 0 if rec.rng.random() < 0.25 else ctx.data['entries'].pop()


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


def _create_object(ctx, rec):
    created = ctx.data['created']
    return created[rec.occurrence % len(created)]


CTORS = (0x00401260, 0x00417500, 0x004455b0, 0x00460d10, 0x00469db0, 0x0048bca0, 0x0048c2b0,
         0x004e1fb0, 0x004e77f0, 0x00524a50)
# The retail statics the body reads (0x00418757) or hands out by address (SetAxesPtr 0x00418559).
BINDINGS = {'_kVec3Zero': 0x00578e60, '_kVec3YAxis': 0x00578e80}
# Callee-popped bytes and return kinds of the virtual slots the body calls (from the retail call
# sites): 0x2c the character's Create (6 arguments), 0x1c a three-argument motion call, 0xb4 a
# float getter, 0xc8 its two-argument setter, 0x11c and slot 5 (the joystick's SetAutoCenter)
# one argument; the others none.
SLOT_POPS = {0xb: 0x18, 7: 0xc, 0x2d: 4, 0x32: 8, 0x47: 4, 5: 4}

REPLAY = dict(returns={SPRINTF: _sprintf, NEW: _new, LOAD_TRACK: _load_track, FIND_ENTRY: _find_entry,
                       ROTATE: _rotate, CREATE_OBJECT: _create_object,
                       **{c: _this for c in CTORS}},
              pops={SPRINTF: 0, NEW: 0, ROTATE: 16}, args={SPRINTF: 2, NEW: 3, ROTATE: 4},
              ecx={SPRINTF: False, NEW: False, ROTATE: False},
              unlogged=(SPRINTF, NEW), passthrough=(STRNCPY, STRRCHR))


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   event=rng.choice([0, 1, 1, 2, 2, 3, 3, 3, 4, 5, 7]),
                   race_mode=rng.choice([0, 1, 1, 2, 3, 4]),
                   players=rng.choice([1, 1, 1, 2, 4]), ai=rng.choice([0, 1, 3]), mp_ai=rng.choice([0, 1, 2]),
                   random_gates=rng.choice([0, 1]), stunt=rng.choice([0, 1]),
                   track_number=rng.choice([0, 7, 99, 255]), track_name=rng.choice(['', 'Quarry', 'x' * 40]),
                   replay=rng.choice([0, 0, 1]), tape=rng.choice([0, 1, 1]), ghost=rng.choice([0, 0x41]),
                   joystick=rng.choice([0, 1, 2]), indicator=rng.choice([0, 1]), plate=rng.randrange(-5, 100),
                   demo=rng.choice([0, 1]),
                   terrain=rng.choice(['tracks\\quarry.trn', 'a.b.c', 'x' * 250 + '.trn', 'q.' + 'y' * 270, '.']),
                   has_start=rng.choice([0, 1, 1]), has_finish=rng.choice([0, 1, 1]), track_ok=rng.choice([0, 1, 1]),
                   slots=[(rng.randrange(4), rng.randrange(4), rng.randrange(100)) for _ in range(8)],
                   racer_ids=[(rng.randrange(4), rng.choice([0, 1, 2, 3, -1])) for _ in range(8)])


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    ctx.data['case'] = c
    vt = ctx.fake_vtable('Object', 0x80, pops=SLOT_POPS, returns={0x2d: 'float'})
    vb = ctx.fake_vtable('VirtualBase', 8)
    vbt = ctx.alloc(8, struct.pack('<iI', -4, 4))    # the virtual base sits at +8
    ctx.write(ARENA, _universe(c['seed'], vt))
    r = random.Random(c['seed'])

    def universal(size, virtual_base=False):
        """An object: the generic vptr, then arena cell pointers (+4 vbtable, +8 base vptr)."""
        at = ctx.alloc(size)
        words = [vt] + [ARENA + 16 * r.randrange(0x2000) for _ in range(size // 4 - 1)]
        if virtual_base:
            words[1:3] = [vbt, vb]
        ctx.write(at, struct.pack('<%dI' % len(words), *words))
        return at

    # The scene entries (start/finish gates) and the racers have a virtual base.
    ctx.data['entries'] = [universal(0x400, True) for _ in range(2)]
    # The objects 0x00401310 creates, with their model width.
    ctx.data['created'] = []
    for _ in range(3):
        o = universal(0x100)
        ctx.pack(o + 0x50, 'f', r.uniform(5, 40))
        ctx.data['created'].append(o)
    game = universal(0x3600)
    ctx.pack(game + 0x18, 'i', c['players'])
    ui = universal(0x600)
    ctx.pack(ui + 0x4a8, 'i', c['demo'])
    ctx.pack(game + 0x56c, 'I', ui)
    ctx.pack(game + 0x1000, 'i', c['joystick'])
    ctx.pack(game + 0xc48, 'i', c['indicator'])
    ctx.pack(game + 0x2144, 'i', c['plate'])
    for k, (pid, index, plate) in enumerate(c['slots']):
        slot = game + 0x215c + 0xf8 * k
        ctx.pack(slot + 0xc0, 'i', plate)
        ctx.pack(slot + 0xd4, 'ii', pid, index)
    ctx.pack(game + 0x2b64, 'B', c['ghost'])
    ctx.pack(game + 0x2d70, 'ii', c['race_mode'], c['event'])
    ctx.pack(game + 0x2d94, 'ii', c['ai'], c['mp_ai'])
    ctx.pack(game + 0x2d9c, 'B', c['random_gates'])
    ctx.pack(game + 0x2da4, 'B', c['track_number'])
    ctx.write(game + 0x2da6, c['track_name'].encode() + b'\0')
    ctx.pack(game + 0x2eb8, 'i', c['stunt'])
    ctx.pack(game + 0x3428, 'ii', c['replay'], c['tape'])
    ctx.pack(TRACK_GAME, 'I', game)
    # The rotation axis (kVec3YAxis is set by a dynamic initializer, so the image holds zeros).
    ctx.pack(BINDINGS['_kVec3YAxis'], '3f', 0.0, 1.0, 0.0)

    env = universal(0x400)
    ctx.write(env + 0x104, c['terrain'].encode()[:0x103] + b'\0')
    scene = universal(0x200)
    ctx.pack(scene + 0xa4, 'I', env)
    this = universal(0x400)
    racers = ctx.alloc(0x20, struct.pack('<8I', *(universal(0x800, True) for _ in range(8))))
    ctx.pack(this + 0x38, 'I', universal(0x800, True))    # the local racer
    ctx.pack(this + 0x40, 'I', racers)                     # the AI racers
    # The racer slots, matched against g_TrackGame's racers by network player id (+0x11bc) and
    # racer index (+0x11c0, a signed byte) when painting the multiplayer plates.
    slots = []
    for pid, index in c['racer_ids']:
        o = universal(0x1200, True)
        ctx.pack(o + 0x11bc, 'i', pid)
        ctx.pack(o + 0x11c0, 'b', index)
        slots.append(o)
    ctx.pack(this + 0x3c, 'I', ctx.alloc(0x20, struct.pack('<8I', *slots)))
    owner = universal(0x100)
    args = [owner] + [universal(0x100) for _ in range(5)] + [scene] + [universal(0x100) for _ in range(2)]
    return dict(ecx=this, args=args)
