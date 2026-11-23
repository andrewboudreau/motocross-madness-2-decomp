"""BikeRace::CreateRacers 0x00419970, the racer loader (tools/emu_diff.py case, call-record/replay mode).

The linked body is in src/reconstructed/BikeRace.cpp (13428 retail bytes): it opens
the rider archive, works out the racer count and start order (replay, single player, pro
circuit, network), then loads the local racer and every AI or remote racer (archive lookups,
engine setup from the garage and the bike-class tables, KrustyBike::Load, textures, skill).

Memory is bike_race_create's "universal" arena (every pointer chain stays readable and every
virtual call lands on a logged fake slot); sprintf, new, the constructors and the vector
rotation reuse that case's models. g_TrackGame (0x0056e26c) carries what steers the body,
at the offsets the retail body reads: +0x18 player count, +0x2158 mode racer count (0..8; the
mode racer slots are 0xf8 bytes from +0x215c), +0x2d70 race mode, +0x2d74 event type,
+0x2d94/+0x2d98 AI and multiplayer AI counts, +0x2da5 network player count, +0x3428/+0x342c
playingReplay/useReplayTape, +0x3444 the pro circuit (+0x460 racer count, +0x464 career
flags, 0x140-byte racers from +0x465), +0x2d7c/+0x570 the event's races and races run, and
for a network race +0x08 the local player, +0x2ebc the start grid. Counts keep the total racer count within the body's
11-entry order arrays. InProCircuitCareer (0x00521cd0), RankByPoints, the archive opener
and AppendChild are modelled from the case (each fails at a chosen call, the ranking is a permutation),
PickRandomRacers and MakeRandomNames fill their arrays, KrustyVCR::GetRacer writes a
terminated recorded racer, and the debug allocators note size and
line, not __FILE__ (the bring-up file name differs from retail's BikeRace.cpp by design);
every other callee is replayed.
"""
import random
import struct

from tools.emu_cases import bike_race_create as base

RETAIL_VA = 0x00419970
SYMBOL = ('?CreateRacers@BikeRace@@QAEHPAXPAVLightManager@@PAUTextureContext@@'
          'PAVTerrain@@PAURaceGateEvent@@@Z')
SOURCE = 'src/reconstructed/BikeRace.cpp'
BINDINGS = dict(base.BINDINGS)
PROBES = {0x00419a57: 'no rider archive', 0x00419b43: 'replay rank', 0x00419bb8: 'pro circuit check',
          0x00419d0f: 'no PC rider', 0x00419f0b: 'load pro circuit racer', 0x0041a36a: 'load pro circuit AI',
          0x0041a540: 'replay racer', 0x0041a650: 'pick random racers', 0x0041a88e: 'load net local',
          0x0041adb2: 'load net player', 0x0041b2aa: 'load net AI', 0x0041b595: 'load local racer',
          0x0041b95a: 'load ghost', 0x0041bb05: 'replay racer 2', 0x0041c0c7: 'load AI',
          0x0041c8b1: 'load net host AI', 0x0041bd6b: 'random skill, event type 0',
          0x0041bdda: 'random skill, other events'}

ARENA = base.ARENA
TRACK_GAME = base.TRACK_GAME
IN_PRO_CIRCUIT = 0x00521cd0
VIEW_CTOR = 0x0048fa60            # BikeRaceKrustyBikeView(int): a constructor, so it returns this

OPEN_ARCHIVE, RANK_BY_POINTS = 0x004e9cd0, 0x00417bc0
PICK_RANDOM, RANDOM_NAMES = 0x0049b0d0, 0x0049b020
GET_RACER, APPEND_CHILD = 0x0049c1e0, 0x00469190
MALLOC, CALLOC, FREE, DELETE_FL = 0x004a2e20, 0x004a2fc0, 0x004a2e60, 0x004a3060
# Virtual calls in this body (fake 'Object' slots): TrackGame slot 0x14 (+0x50) takes two
# arguments, the racer slot 0x32 (+0xc8) three; the virtual base's slot 4 takes none.
SLOT_POPS = {0x14: 8, 0x32: 12}


def _open_archive(ctx, rec):
    k = ctx.data['opens'] = ctx.data.get('opens', 0) + 1
    ctx.note('OpenArchiveFile', base._cstr(ctx, rec.args[1]))
    return 0 if k == ctx.data['case']['open_fail'] else 1


def _append_child(ctx, rec):
    # GameObject::AppendChild: fails (0) only at the case's chosen call, so deep loads run.
    k = ctx.data['appends'] = ctx.data.get('appends', 0) + 1
    return 0 if k == ctx.data['case']['append_fail'] else 1


def _view_ctor(ctx, rec):
    # The view has a virtual base: +0x04 its vbtable (base at +4 from there), +0x08 the base's vptr.
    ctx.pack(rec.ecx + 4, '2I', ctx.data['vbt'], ctx.data['vb'])
    return rec.ecx


def _get_racer(ctx, rec):
    # KrustyVCR::GetRacer(int* id, char* ai, int index, char* name, char* bikeFile, char* riderFile,
    # char* bike, char* rider, int* engineSize, int* fourStroke, int* plate): the recorded racer,
    # with terminated strings (name 0x10 bytes, the rest 0x40 or more).
    a, k = rec.args, rec.args[2]
    ctx.pack(a[0], 'i', k + 1)
    ctx.pack(a[1], 'B', k & 1)
    for at, text in ((a[3], b'Rec%d' % k), (a[4], b'Res/bike%d.mcf' % k), (a[5], b'Res/rider%d.mcf' % k),
                     (a[6], b'bike%d' % k), (a[7], b'rider%d' % k)):
        ctx.write(at, text + bytes(1))
    ctx.pack(a[8], 'i', (125, 250, 500)[k % 3])
    ctx.pack(a[9], 'i', k & 1)
    ctx.pack(a[10], 'i', 10 + k)


def _rank(ctx, rec):
    # RankByPoints(int ranking[11]): a permutation of 1..11 (the body searches it for i + 1).
    ctx.pack(rec.args[0], '11i', *ctx.data['case']['ranking'])


def _pick_random(ctx, rec):
    # PickRandomRacers(int* picks, int count, int* used): small table indexes.
    count = max(0, min(rec.args[1], 11))
    ctx.pack(rec.args[0], '%di' % count, *[(3 * i + 1) % 7 for i in range(count)])
    if rec.args[2]:
        ctx.pack(rec.args[2], '%di' % count, *[i % 5 for i in range(count)])


def _random_names(ctx, rec):
    count = max(0, min(rec.args[1], 11))
    names = ctx.data.setdefault('names', [ctx.alloc(16, b'Racer %d\0' % i) for i in range(11)])
    ctx.pack(rec.args[0], '%dI' % count, *names[:count])


def _malloc(ctx, rec):
    ctx.note('DebugMalloc', rec.args[0], rec.args[2])
    return ctx.alloc(max(min(rec.args[0], 0x10000), 16), fill=0xcd)


def _calloc(ctx, rec):
    ctx.note('DebugCalloc', rec.args[0], rec.args[1], rec.args[3])
    return ctx.alloc(max(min(rec.args[0] * rec.args[1], 0x10000), 16))


def _free(ctx, rec):
    ctx.note('DebugFree', rec.args[0], rec.args[2])


REPLAY = dict(base.REPLAY)
REPLAY['returns'] = dict(base.REPLAY['returns'])
REPLAY['returns'].update({IN_PRO_CIRCUIT: lambda ctx, rec: ctx.data['case']['pro'],
                          OPEN_ARCHIVE: _open_archive, APPEND_CHILD: _append_child, VIEW_CTOR: _view_ctor, MALLOC: _malloc, CALLOC: _calloc, FREE: _free})
REPLAY['effects'] = {GET_RACER: _get_racer, RANK_BY_POINTS: _rank, PICK_RANDOM: _pick_random, RANDOM_NAMES: _random_names}
REPLAY['pops'] = {**base.REPLAY['pops'], MALLOC: 0, CALLOC: 0, FREE: 0}
REPLAY['args'] = {**base.REPLAY['args'], MALLOC: 3, CALLOC: 4, FREE: 3}
REPLAY['ecx'] = {**base.REPLAY['ecx'], MALLOC: False, CALLOC: False, FREE: False}
# The allocators pass __FILE__, which names the bring-up file here and BikeRace.cpp in retail;
# their models note size and line instead.
REPLAY['unlogged'] = tuple(base.REPLAY['unlogged']) + (MALLOC, CALLOC, FREE, DELETE_FL)


def cases(rng, n):
    for _ in range(n):
        players = rng.choice([1, 1, 1, 2, 3, 4])
        ai = rng.choice([0, 1, 3, 7])
        mp_ai = rng.choice([0, 1, 2]) if players > 1 else 0    # multiplayer AI only in a network race
        yield dict(seed=rng.getrandbits(32), players=players, ai=ai, mp_ai=mp_ai,
                   net=rng.choice([0, 0, 2, 3]), race_mode=rng.choice([0, 1, 1, 2, 3, 4]),
                   event=rng.choice([0, 0, 1, 2, 3, 5]), replay=rng.choice([0, 0, 1]), tape=rng.choice([0, 1]),
                   mode_racers=rng.randint(0, 8),
                   pro=rng.choice([0, 0, 1]) if players == 1 else 0,    # the pro circuit is single player
                   pro_racers=rng.randint(1, 11), career=rng.choice([0, 2]),
                   engines=[rng.choice([80, 125, 250, 500, 0]) for _ in range(11)],
                   open_fail=rng.choice([0] * 6 + [1, 2, 3, rng.randint(4, 12)]),
                   append_fail=rng.choice([0] * 6 + [1, 2, rng.randint(3, 9)]),
                   ranking=rng.sample(range(1, 12), 11),
                   races=rng.choice([0, 0, 1, 3]), races_run=rng.choice([0, 2]),
                   local_id=rng.choice([1, 1, 1, 9]),
                   ai_hosts=[rng.choice([1, 1, 2, 3, 4]) for _ in range(4)], shuffle=rng.getrandbits(32))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    ctx.data['case'] = c
    vt = ctx.fake_vtable('Object', 0x80, pops=SLOT_POPS, returns={0x2d: 'float'})
    vb = ctx.fake_vtable('VirtualBase', 8)
    vbt = ctx.alloc(8, struct.pack('<iI', -4, 4))
    ctx.data.update(vbt=vbt, vb=vb)
    ctx.write(ARENA, base._universe(c['seed'], vt))
    r = random.Random(c['seed'])

    def universal(size, virtual_base=False):
        at = ctx.alloc(size)
        words = [vt] + [ARENA + 16 * r.randrange(0x2000) for _ in range(size // 4 - 1)]
        if virtual_base:
            words[1:3] = [vbt, vb]
        ctx.write(at, struct.pack('<%dI' % len(words), *words))
        return at

    ctx.data['entries'] = [universal(0x400, True) for _ in range(2)]
    ctx.data['created'] = [universal(0x100) for _ in range(3)]
    game = universal(0x3600)
    ctx.pack(game + 0x18, 'i', c['players'])
    ctx.pack(game + 0x2158, 'i', c['mode_racers'])
    ctx.pack(game + 0x2d70, 'ii', c['race_mode'], c['event'])
    ctx.pack(game + 0x2d94, 'ii', c['ai'], c['mp_ai'])
    ctx.pack(game + 0x2da5, 'B', c['net'])
    ctx.pack(game + 0x3428, 'ii', c['replay'], c['tape'])
    circuit = universal(0x465 + 11 * 0x140 + 0x20)
    ctx.pack(circuit + 0x460, 'iB', c['pro_racers'], c['career'])
    for k in range(11):
        at = circuit + 0x465 + 0x140 * k
        for off, text in ((0x38, b'bike%d.mcf' % k), (0x78, b'Bike %d' % k), (0xb8, b'rider%d' % k),
                          (0xf8, b'Racer %d' % k)):
            ctx.write(at + off, text + b'\0')
        ctx.pack(at + 0x138, 'ii', c['engines'][k], k & 1)
    ctx.pack(game + 0x3444, 'I', circuit if c['pro'] else 0)
    # The front end (+0x56c): +0x48 the manufacturers (200 bytes, +0x40 model file), +0x50 the
    # bikes (0x94 bytes, +0x00 manufacturer index); PickRandomRacers picks bikes 0..6.
    ui = universal(0x800)
    makers = universal(200 * 4)
    for k in range(4):
        ctx.write(makers + 200 * k + 0x40, b'maker%d.mcf\0' % k)
    bikes = universal(0x94 * 8)
    for k in range(8):
        ctx.pack(bikes + 0x94 * k, 'i', (k * 3) % 4)
    ctx.pack(ui + 0x48, 'I', makers)
    ctx.pack(ui + 0x50, 'I', bikes)
    ctx.pack(game + 0x56c, 'I', ui)
    # Races so far: +0x2d7c the event's race count, the event manager's (+0x570) racesRun (+0x48).
    ctx.pack(game + 0x2d7c, 'i', c['races'])
    events = universal(0x1000)      # +0x444: 0x50-byte event AI records
    ctx.pack(events + 0x48, 'B', c['races_run'])
    ctx.pack(game + 0x570, 'I', events)
    # A network race: the network (+0x08) local player id (+0x0c); the start grid (+0x2ebc,
    # 8-byte entries: +0 network player id, +5 AI flag) and the mode racer slots (+0x215c,
    # 0xf8 bytes: +0xd4 network player id, +0xd8 racer index) list the same racers, players
    # 1..playerCount and their hosts' AI racers. A local id of 9 is on no grid entry.
    network = universal(0x40)
    ctx.pack(network + 0xc, 'i', c['local_id'])
    ctx.pack(game + 0x8, 'I', network)
    grid = [(p, 0, 0) for p in range(1, c['players'] + 1)]
    for k in range(c['mp_ai']):
        host = min(c['ai_hosts'][k], c['players'])
        grid.append((host, 1, 1 + sum(1 for g in grid if g[0] == host and g[1])))
    grid = grid[:8]
    slots = list(grid)
    random.Random(c['shuffle']).shuffle(slots)
    for i, (pid, is_ai, _) in enumerate(grid):
        ctx.pack(game + 0x2ebc + 8 * i, 'i', pid)
        ctx.pack(game + 0x2ebc + 8 * i + 5, 'B', is_ai)
    if not c['replay'] and c['players'] > 1:
        ctx.pack(game + 0x2158, 'i', len(slots))
        for j, (pid, _, index) in enumerate(slots):
            ctx.pack(game + 0x215c + 0xf8 * j + 0xd4, 'ii', pid, index)
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(BINDINGS['_kVec3YAxis'], '3f', 0.0, 1.0, 0.0)

    this = universal(0x400)
    owner = universal(0x100)
    args = [owner] + [universal(0x100) for _ in range(4)]
    return dict(ecx=this, args=args, ret_eax=True)
