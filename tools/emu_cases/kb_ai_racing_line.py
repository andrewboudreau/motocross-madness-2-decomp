"""KrustyBike::UpdateAIRacingLine 0x00414370, the AI racing line (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp). The bike,
race, status node, gate and terrain are filled with random floats (so every comparison of
positions, speeds and timers falls either way), then the pointers and tested values are set:
event type 0..5 (free roam, gate races, track races), difficulty, raceMode, hasFinished,
onTrack, a race track or none, null track positions, the trick gates (aiTricksEnabled,
motionActive, airborne, the trick timer against the delay) and a failing DebugCalloc.

The track queries are stubs on the race track (thiscall) with outputs drawn from the run's
random stream, so both bodies see the same answers as long as they ask the same questions:
ClosestPointOnNode 0x00516ca0, DistanceAlong 0x00517da0 (ST0), ShortestPath 0x005179f0 (appends
heap work-list items), MoveAlongPath 0x00517ea0, PointAt 0x00518080, PlaceNear 0x00517340,
SegmentDirection 0x00518130, DistanceToStripEdge 0x00518230 and FreeWorkList 0x00517930.
Also stubbed: DebugCalloc (cdecl, noted without __FILE__), PredictLanding 0x0040e370 (cdecl,
writes the landing time), PickTrick / StartTrick (the trick pick and start) and the virtual
slots 11 (FindPlacement, writes its outputs), 32 (EffectiveLoadWeight, ST0) and 33
(ResetToPlacement, notes the vectors). rand 0x00534c1b is a stub too (the MT CRT's per-thread
seed needs TLS); FastSqrt, FastInvSqrt and SafeNormalize run as retail code.
"""
import random
import struct

RETAIL_VA = 0x00414370
SYMBOL = '?UpdateAIRacingLine@KrustyBike@@QAEHM@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x00414439: 'roam re-pick', 0x00414580: 'roam keep', 0x004146e9: 'gate track',
          0x0041484f: 'gate work list', 0x00414929: 'gate line', 0x004149c0: 'gate distance',
          0x00414dc2: 'track race', 0x00414ed9: 'track line on', 0x00415022: 'respawn',
          0x004150d2: 'track line off', 0x004152b7: 'finished PlaceNear', 0x004152e8: 'strip dir',
          0x00415544: 'PredictLanding', 0x0041559a: 'trick pick', 0x004155d4: 'trick start',
          0x00415612: 'FreeWorkList'}


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.randrange(1 << 30), spread=rng.choice([5.0, 30.0, 200.0]),
            event=rng.choice([0, 0, 1, 2, 2, 3, 3, 4, 5]), difficulty=rng.choice([0, 1, 2, 3, 4]),
            race_mode=rng.choice([0, 0, 1]), finished=rng.choice([0, 0, 1]), on_track=rng.choice([0, 1, 1]),
            track=rng.random() < 0.85, null_pos=rng.choice([None] * 6 + ['progress.a', 'progress.b', 'finish.a', 'finish.b', 'gate']),
            goal=rng.choice([rng.uniform(0, 300), rng.uniform(300, 1000)]),
            tricks=rng.choice([0, 1, 1]), motion=rng.choice([0, 0, 1]), airborne=rng.choice([0, 1, 1]),
            trick_timer=rng.choice([0.0, rng.uniform(0, 40)]), stunt=rng.uniform(0, 100),
            off_track=rng.choice([rng.uniform(0, 4.99), rng.uniform(4.9, 6)]),
            dt=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1)]),
            fail_at=rng.choice([None] * 9 + [1, 2]),
            ok=rng.choice([1.0, 1.0, 1.0, 0.97, 0.9]),
        )


def setup(ctx, c):
    r = random.Random(c['seed'])
    s = c['spread']

    def fill(addr, size, lo=None):
        ctx.write(addr, struct.pack('<%df' % (size // 4), *[r.uniform(-s, s) for _ in range(size // 4)]))

    ok = c['ok']

    def yes(cx):
        return 1 if cx.rng.random() < ok else 0

    def raw3(cx, at):
        cx.pack(at, 'IIf', 0x51000000 + cx.rng.randrange(16) * 0x10, 0x52000000 + cx.rng.randrange(16) * 0x30,
                cx.rng.random())

    def vec(cx, at):
        cx.pack(at, '3f', *[cx.rng.uniform(-s, s) for _ in range(3)])

    def closest(cx, ecx, a):          # (Vec3 pos, int node, KbRaw3* out, float* dist)
        raw3(cx, a[4])
        return yes(cx)

    def distance(cx, ecx, a):         # (KbRaw3 a, KbRaw3 b) -> ST0
        return 0, cx.rng.choice([cx.rng.uniform(-s, s), 0.0, cx.rng.uniform(0, s)])

    def shortest(cx, ecx, a):         # (KbRaw3 a, KbRaw3 b, KbTrackItem** list, float* dist)
        tail = a[6]
        while cx.unpack(tail, 'I')[0]:
            tail = cx.unpack(tail, 'I')[0] + 0xc
        for _ in range(cx.rng.choice([0, 1, 2])):
            item = cx.alloc(0x10)
            cx.pack(item, 'II', 0, 0x53000000 + cx.rng.randrange(16) * 0x18)
            cx.pack(tail, 'I', item)
            tail = item + 0xc
        cx.pack(a[7], 'f', cx.rng.uniform(0, s))
        return yes(cx)

    def move(cx, ecx, a):             # (KbRaw3 from, KbRaw3* out, KbTrackItem** list, float d, char)
        raw3(cx, a[3])
        return yes(cx)

    def point(cx, ecx, a):            # (KbRaw3 pos, Vec3* out)
        vec(cx, a[3])
        return yes(cx)

    def segment_direction(cx, ecx, a):  # (int seg, Vec3* out)
        vec(cx, a[1])
        return yes(cx)

    def strip_edge(cx, ecx, a):       # (Vec3 pos, int seg, float* out, int side)
        cx.pack(a[4], 'f', cx.rng.uniform(0, 10))
        return yes(cx)

    def free_list(cx, ecx, a):        # (KbTrackItem** list, int)
        items, p = [], cx.unpack(a[0], 'I')[0]
        while p and len(items) < 16:
            items.append(p)
            p = cx.unpack(p + 0xc, 'I')[0]
        cx.note('FreeWorkList', *items)
        return 1

    count = [0]

    def calloc(cx, ecx, a):
        count[0] += 1
        cx.note('calloc', a[0], a[1], a[3])
        if c['fail_at'] is not None and count[0] == c['fail_at']:
            return 0
        return cx.alloc(a[0] * a[1])

    def landing(cx, ecx, a):          # cdecl (Vec3, Vec3, float* time, Vec3*, Vec3*, Terrain*)
        cx.note('PredictLanding', *a)
        cx.pack(a[6], 'f', cx.rng.uniform(-0.5, 5))
        return 1

    def pick(cx, ecx, a):             # (int* out, float t, int)
        cx.pack(a[0], 'i', cx.rng.randrange(16))
        return 1 if cx.rng.random() < 0.4 else 0

    def find_placement(cx, ecx, a):   # slot 11 (int, Vec3*, Vec3*, Vec3*, int*)
        for p in a[1:4]:
            vec(cx, p)
        cx.pack(a[4], 'i', cx.rng.randrange(2))
        return 1

    def reset(cx, ecx, a):            # slot 33 (Vec3*, Vec3*, Vec3*, Vec3*, int, float)
        cx.note('ResetToPlacement', *[cx.read(p, 12).hex() for p in a[:4]], a[4], a[5])

    ctx.stub_at(0x00516ca0, 'ClosestPointOnNode', pop=24, ret=closest)
    ctx.stub_at(0x00517da0, 'DistanceAlong', pop=24, ret=distance, ret_float='f')
    ctx.stub_at(0x005179f0, 'ShortestPath', pop=32, ret=shortest)
    ctx.stub_at(0x00517ea0, 'MoveAlongPath', pop=28, ret=move)
    ctx.stub_at(0x00518080, 'PointAt', pop=16, ret=point)
    ctx.stub_at(0x00517340, 'PlaceNear', pop=40, ret=lambda cx, e, a: yes(cx))
    ctx.stub_at(0x00518130, 'SegmentDirection', pop=8, ret=segment_direction)
    ctx.stub_at(0x00518230, 'DistanceToStripEdge', pop=24, ret=strip_edge)
    ctx.stub_at(0x00517930, 'FreeWorkList', pop=8, ret=free_list)
    ctx.stub_at(0x004a2fc0, 'DebugCalloc', nargs=4, ret=calloc, log=False)
    ctx.stub_at(0x0040e370, 'PredictLanding', nargs=10, ret=landing, log=False)
    ctx.stub_at(0x00534c1b, 'rand', ret=lambda cx, e, a: cx.rng.randrange(0x8000), log=False)
    ctx.stub_at(0x0048d780, 'PickTrick', pop=12, ret=pick)
    ctx.stub_at(0x0048d910, 'StartTrick', pop=4)
    slots = [ctx.stub('KrustyBike slot %d' % k) for k in range(64)]
    slots[11] = ctx.stub('KrustyBike::FindPlacement', pop=20, ret=find_placement)
    slots[32] = ctx.stub('KrustyBike::EffectiveLoadWeight', ret=(0, 1.75), ret_float='f')
    slots[33] = ctx.stub('KrustyBike::ResetToPlacement', pop=24, ret=reset)

    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x60c, 'i', c['difficulty'])
    ctx.pack(game + 0x2d70, 'ii', c['race_mode'], c['event'])
    ctx.pack(TRACK_GAME, 'I', game)

    bike = ctx.alloc(0x1700)
    fill(bike, 0x1700)
    ctx.pack(bike, 'I', ctx.vtable(slots))
    terrain = ctx.alloc(0x80)
    fill(terrain, 0x80)
    ctx.pack(terrain + 0x40, 'f', r.choice([1.0, 0.5, r.uniform(0.1, 3)]))
    ctx.pack(bike + 0x1f4, 'I', terrain)
    ctx.pack(bike + 0x108, 'B', c['airborne'])
    ctx.pack(bike + 0x430, 'B', c['motion'])
    ctx.pack(bike + 0x78c, 'i', c['on_track'])
    ctx.pack(bike + 0x7a4, 'B', c['finished'])
    ctx.pack(bike + 0x7dc, 'f', c['trick_timer'])
    ctx.pack(bike + 0x11b4, 'f', c['off_track'])
    ctx.pack(bike + 0x15dc, 'f', c['stunt'])
    ctx.pack(bike + 0x15e4, 'B', c['tricks'])

    race = ctx.alloc(0x200)
    fill(race, 0x200)
    node = ctx.alloc(0x80)
    fill(node, 0x80)
    gate = ctx.alloc(0x80)
    fill(gate, 0x80)
    track = ctx.alloc(0x40)
    ctx.pack(track, 'I', 0x50000000)
    ctx.pack(race + 0x48, 'I', track if c['track'] else 0)
    ctx.pack(race + 0x134, 'IIf', 0x51000100, 0x52000100, r.random())
    ctx.pack(node + 0x08, 'f', c['goal'])
    ctx.pack(node + 0x34, 'I', gate)
    ctx.pack(node + 0x38, 'IIf', 0x51000200, 0x52000200, r.random())
    ctx.pack(node + 0x44, 'IIf', 0x51000300, 0x52000300, r.random())
    ctx.pack(gate + 0x2c, 'IIf', 0x51000400, 0x52000400, r.random())
    blank = {'progress.a': node + 0x44, 'progress.b': node + 0x48, 'finish.a': race + 0x134,
             'finish.b': race + 0x138, 'gate': gate + 0x2c}.get(c['null_pos'])
    if blank:
        ctx.pack(blank, 'I', 0)
    ctx.pack(bike + 0x740, 'II', race, node)
    return dict(ecx=bike, args=[float(c['dt'])])
