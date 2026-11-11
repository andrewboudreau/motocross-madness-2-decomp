"""BikeRace::WriteRelativePositions 0x004210f0, the start-grid slot (tools/emu_diff.py case).

The linked body is the bring-up copy (src/bringup/Group1_BikeRaceCamera.cpp). The race is
random floats; from the retail body: gridReady +0x188 (byte), the cached grid (first slot
+0x164, step +0x170, direction +0x17c), raceTrack +0x48, raceTerrain +0x4c (gridCellSize
+0x40), raceScene +0x58 (default start position +0x70, direction +0x7c), hasStartProbe +0x144
with the probe centre +0xcc and direction +0xd8, racerCount +0x158, the event type at
g_TrackGame (0x0056e26c) +0x2d74 (0..5 and an out-of-table 7). The gate owner (fourth
argument, events 1 and 5) has gateCount +0xac, 0x18-byte gate records from +0xd8 and entry
pointers from +0x420 (width +0x50). Track::SegmentDirection 0x00518130 is a stub that writes a
random direction; the track's start node, first segment and its next links are present or
null. The dot, inverse square root and scale helpers run as retail code. Default start
directions include unit vectors, so the `== 1` shortcut runs.

Fixed here: Normalize summed the length through a TU-local dot ((z + y) + x) and scaled by the
unrounded FastInvSqrt result; retail calls the dot 0x0040ae30 ((y + x) + z) and the scale
0x005015b0 with the inverse rounded to float (2 of 100 differed at 24 bits, 22 of 300 at 53).
Also fixed (53-bit only, 11 of 300): VC6 expanded the Vector3 constructor at the grid sites
(the 4.66 offset in event 0, the event 2/3 tail), keeping a product in a register where retail
calls the out-of-line constructor 0x00404e60 and so rounds it to float. The grid sites now
construct through it as retail does (ScaleCall/AddCall/SubCall; event 4 also calls the scale
0x005015b0 and the out-of-line operator+/- 0x00421cb0/0x00421d00). Both control words compare.
"""
import math
import random
import struct

RETAIL_VA = 0x004210f0
SYMBOL = '?WriteRelativePositions@BikeRace@@QAEXPAUVector3@@0PAXH@Z'
SOURCE = 'src/bringup/Group1_BikeRaceCamera.cpp'
TRACK_GAME = 0x0056e26c
# Event 4's out-of-line Vector3 operator+ and operator- (retail code in both runs).
BINDINGS = {'?BikeRaceAddCall@@YA?AUVector3@@ABU1@0@Z': 0x00421cb0,
            '?BikeRaceSubtractCall@@YA?AUVector3@@ABU1@0@Z': 0x00421d00}
PROBES = {0x00421112: 'cached', 0x004211a9: 'events 2/3', 0x004212bb: 'segment direction',
          0x004214d0: 'event 0', 0x0042174c: 'events 1/5', 0x00421a59: 'event 4',
          0x00421c84: 'grid ready'}


def unit(rng):
    v = [rng.uniform(-1, 1) for _ in range(3)]
    l = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / l for x in v]


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), ready=rng.choice([0, 0, 0, 1]),
                   event=rng.choice([0, 1, 2, 3, 4, 5, 7]), count=rng.choice([0, -1, 1, 2, 5, 11]),
                   probe=rng.randrange(2), racers=rng.choice([0, 1, 7, 15]),
                   links=rng.choice([0, 1, 2, 3, 4, 4, 4]), gates=rng.randrange(1, 12),
                   unit=rng.randrange(2), width=rng.choice([1.0, 30.0, rng.uniform(0, 200)]))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def floats(addr, size, s=100.0):
        ctx.write(addr, struct.pack('<%df' % (size // 4), *[r.uniform(-s, s) for _ in range(size // 4)]))

    race = ctx.alloc(0x200)
    floats(race, 0x200)
    ctx.pack(race + 0x188, 'B', c['ready'])
    ctx.pack(race + 0x144, 'I', c['probe'])
    ctx.pack(race + 0x158, 'i', c['racers'])
    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['event'])
    ctx.pack(TRACK_GAME, 'I', game)
    scene = ctx.alloc(0x100)
    floats(scene, 0x100)
    if c['unit']:
        ctx.pack(scene + 0x7c, '3f', *unit(r))
    ctx.pack(race + 0x58, 'I', scene)
    terrain = ctx.alloc(0x80)
    floats(terrain, 0x80, 10.0)
    ctx.pack(race + 0x4c, 'I', terrain)
    track = ctx.alloc(0x40)
    node = ctx.alloc(0x40)
    segs = [ctx.alloc(0x40) for _ in range(3)]
    ctx.pack(track, 'I', node if c['links'] >= 1 else 0)
    ctx.pack(node + 8, 'I', segs[0] if c['links'] >= 2 else 0)
    ctx.pack(segs[0] + 0x2c, 'I', segs[1] if c['links'] >= 3 else 0)
    ctx.pack(segs[1] + 0x2c, 'I', segs[2] if c['links'] >= 4 else 0)
    ctx.pack(race + 0x48, 'I', track)

    def direction(cx, ecx, a):
        cx.pack(a[1], '3f', *[cx.rng.uniform(-1, 1) for _ in range(3)])
    ctx.stub_at(0x00518130, 'Track::SegmentDirection', pop=8, ret=direction)

    owner = ctx.alloc(0x500)
    floats(owner, 0x500)
    ctx.pack(owner + 0xac, 'i', c['gates'])
    entry = ctx.alloc(0x80)
    floats(entry, 0x80)
    ctx.pack(entry + 0x50, 'f', c['width'])
    ctx.pack(owner + 0x41c + 4 * c['gates'], 'I', entry)

    a = ctx.alloc(12, fill=0x77)
    b = ctx.alloc(12, fill=0x77)
    return dict(ecx=race, args=[a, b, owner, c['count']], ret_eax=False)
