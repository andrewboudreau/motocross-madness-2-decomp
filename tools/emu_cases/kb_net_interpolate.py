"""KrustyBike::Fn_00493660 0x00493660, the remote/replay bike's state interpolation
(tools/emu_diff.py case, call-record/replay mode).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBike.cpp). Every call
leaving it is recorded and replayed (``REPLAY``): ReadClock, the orientation helpers, the node
transforms, the wreck calls, the trick starts, SelectLapGates and the virtual slots return the
same synthetic results to both bodies. FastSqrt and FastInvSqrtEstimate run as retail code.

The bike (primary vtable 0x00554548) is random floats with the four received states
(states[] at +0x11c8, newest first, ascending times, mostly nonzero positions), the interval
bookkeeping, the pose latches and the race fields set from the case. The network tuning
globals (g_kbInterpolate, g_kbLatencyHiding, g_kbRateLimiting, g_kbAllowWarping, ...) are
written into the retail image, so both bodies read the same switches; g_TrackGame points at
a game block with the frame time, replay flag, race mode, event type and an optional Baja
enduro track.
"""
import struct

RETAIL_VA = 0x00493660
SYMBOL = '?Fn_00493660@KrustyBike@@QAEXMH@Z'
SOURCE = 'src/bringup/Group3_KrustyBike.cpp'
KB_VT = 0x00554548
KB_VT2 = 0x00554514
TRACK_GAME = 0x0056e26c
# g_kbLatencyHiding .. g_kbInterpolate (0x0056cb3c..0x0056cb60) and g_kbUseLatencyThreshold
G_LATENCY_HIDING = 0x0056cb3c
G_USE_LATENCY_THRESHOLD = 0x0067c3a0
PROBES = {0x00493c0c: 'clock extrapolation', 0x00493f07: 'velocity rate limit',
          0x00494211: 'position rate limit', 0x004947be: 'crossover pose axes',
          0x00494998: 'wheel roll', 0x00495005: 'lap gates (state)', 0x004951bd: 'tumble',
          0x004951d1: 'animated wreck', 0x00495208: 'end wreck', 0x00495276: 'trick start',
          0x0049562c: 'trick chain', 0x0049575d: 'collision placement'}



def _motion_complete(ctx, rec):
    return 1 if rec.rng.random() < 0.2 else 0


# slot 42 (CheckMotionComplete) is usually 0 so the trick start and chain tests run.
REPLAY = dict(inline=('?FastSqrt@@YAMM@Z', '?FastInvSqrtEstimate@@YAMM@Z'),
              returns={('vslot', KB_VT, 42): _motion_complete})


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.getrandbits(32),
            dt=rng.choice([1 / 60.0, 1 / 30.0]),
            a=rng.choice([0, 1]),
            frame=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0.001, 0.1), 0.0]),
            replay=rng.choice([0, 0, 0, 1]),
            race_mode=rng.choice([0, 1, 2, 3, 4, 4]),
            event=rng.choice([0, 1, 3, 4]),
            baja=rng.choice([0, 1]),
            gates=rng.randrange(1, 8),
            zero_pos=rng.random() < 0.08,
            same_received=rng.random() < 0.3,
            latch=rng.choice([0, 1]),
            elapsed=rng.choice([0.0, rng.uniform(-0.05, 0.3)]),
            length=rng.choice([0.0, rng.uniform(0.01, 0.2)]),
            times=sorted((rng.randrange(1000, 100000) for _ in range(4)), reverse=True),
            replay_time=rng.uniform(0, 120),
            last_record=rng.choice([0, -2]),
            paused=rng.choice([0, 0, 1]),
            switches=[rng.choice([0, 1]) for _ in range(10)],
            use_latency=rng.choice([0, 1]),
            crash_prev=rng.choice([0, 0, 1]),
            motion=rng.choice([0, 0, 1]),
            motion_index=rng.choice([0, 1, 5, -1]),
            crossover=rng.choice([0, 0, 1]),
            ghost=rng.choice([0, 0, 1]),
            finished=rng.choice([0, 0, 1]),
            stall=rng.choice([0.0, rng.uniform(0, 2)]),
            followed=rng.choice([0, 1]),
        )


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])

    def block(size, lo=-2.0, hi=2.0):
        at = ctx.alloc(size)
        ctx.pack(at, '%df' % (size // 4), *[rng.uniform(lo, hi) for _ in range(size // 4)])
        return at

    def node(label):
        at = block(0x200)
        ctx.pack(at, 'I', ctx.fake_vtable(label, 16))
        return at

    s = c['switches']
    ctx.pack(G_LATENCY_HIDING, '4i', *s[:4])                 # latency hiding .. extrap limit flag
    ctx.pack(G_LATENCY_HIDING + 0x10, 'fff', rng.uniform(0, 0.3), rng.uniform(0, 0.3), rng.uniform(0, 3))
    ctx.pack(G_LATENCY_HIDING + 0x1c, '3i', *s[4:7])         # time received, allow negative, interpolate
    ctx.pack(G_USE_LATENCY_THRESHOLD, 'i', c['use_latency'])

    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x2f0, 'f', c['frame'])
    if c['baja']:
        track = block(0x100)
        ctx.pack(track + 0xac, 'i', c['gates'])
        ctx.pack(game + 0x560, 'I', track)
    ctx.pack(game + 0x2d70, 'ii', c['race_mode'], c['event'])
    ctx.pack(game + 0x3428, 'i', c['replay'])
    ctx.pack(TRACK_GAME, 'I', game)

    bike = block(0x1700)
    ctx.pack(bike, 'I', KB_VT)
    ctx.pack(bike + 0x21c, 'I', KB_VT2)
    ctx.pack(bike + 0x08, 'I', node('sceneNode'))
    ctx.pack(bike + 0x128, 'I', block(0xb8))
    ctx.pack(bike + 0x20c, 'BB', 0, 0)
    ctx.pack(bike + 0x218, 'I', node('centerNode'))
    ctx.pack(bike + 0x3bc, 'I', node('modelNode'))
    ctx.pack(bike + 0x42c, 'I', node('poseNode'))
    ctx.pack(bike + 0x430, 'BBBb', c['motion'], 0, 0, c['motion_index'])
    ctx.pack(bike + 0x440, 'ii', 1, c['crash_prev'])
    ctx.pack(bike + 0x4a8, 'i', rng.choice([0, 1, 2]))
    ctx.pack(bike + 0x544, 'i', 2)
    rider = block(0x200)
    ctx.pack(rider + 0x1a0, 'I', node('riderNode'))
    ctx.pack(bike + 0x5c4, 'I', rider)
    wheels = []
    for _ in range(2):
        w = block(0x300)
        ctx.pack(w + 0x2ac, 'II', node('secondaryAux'), node('primaryAux'))
        wheels.append(w)
    ctx.pack(bike + 0x5f0, 'II', *wheels)
    ctx.pack(bike + 0x604, 'I', block(0x80))
    steer = block(0x40)
    ctx.pack(steer + 4, 'f', rng.uniform(0.01, 1))
    ctx.pack(bike + 0x640, 'I', steer)
    ctx.pack(bike + 0x650, 'ii', rng.randrange(14), rng.randrange(12))
    ctx.pack(bike + 0x6fc, 'i', c['crossover'])
    ctx.pack(bike + 0x734, 'BBB', 1, 0, c['ghost'])
    race = block(0x400)
    camera = block(0x400)
    ctx.pack(camera + 0x3b4, 'I', bike if c['followed'] else 0)
    ctx.pack(race + 0x50, 'I', camera)
    ctx.pack(race + 0x1b8, 'f', c['replay_time'])
    ctx.pack(race + 0x1e4, 'i', c['last_record'])
    ctx.pack(race + 0x3f8, 'B', c['paused'])
    ctx.pack(bike + 0x740, 'I', race)
    ctx.pack(bike + 0x7a0, 'h', rng.randrange(4))
    ctx.pack(bike + 0x7a4, 'BB', c['finished'], rng.choice([0, 1]))
    ctx.pack(bike + 0x7b8, 'ii', rng.randrange(c['gates']), rng.randrange(c['gates']))
    ctx.pack(bike + 0x11c4, 'f', rng.uniform(-50, 50))
    ctx.pack(bike + 0x138c, 'f', c['stall'])
    ctx.pack(bike + 0x1400, '4i', rng.choice([0, 1]), rng.choice([0, 1]), rng.randrange(256), rng.randrange(256))
    ctx.pack(bike + 0x153c, 'BBBB', 0, 0, 0, rng.choice([0, 1]))

    # The four received states, newest first; times in ms against a replay clock in seconds.
    states = []
    # Replay stamps straddle the clock less the 0.17 s / 0.27 s lag: the newest 0..0.4 s
    # back, each older one 40..120 ms before the next.
    stamp = c['replay_time'] * 1000.0 - rng.uniform(0, 400)
    for i, t in enumerate(c['times']):
        at = bike + 0x11d8 + i * 0x60
        ctx.pack(at, 'BBhhh', 0, rng.randrange(8), rng.randrange(200), rng.randrange(4), rng.randrange(20))
        pos = [rng.uniform(-100, 100) for _ in range(3)]
        if c['zero_pos'] and i == (3 if c['replay'] else 2):
            pos[rng.randrange(3)] = 0.0
        ctx.pack(at + 0x08, '3f', *pos)
        ctx.pack(at + 0x14, '3f', *[rng.uniform(-3.2, 3.2) for _ in range(3)])
        ctx.pack(at + 0x20, 'fiff', rng.uniform(0, 90), 0, rng.uniform(0, 90), rng.uniform(0, 30))
        ctx.pack(at + 0x30, '6f', *[rng.uniform(-20, 20) for _ in range(6)])
        ctx.pack(at + 0x48, '4B', rng.randrange(256), rng.randrange(256), rng.randrange(256), rng.randrange(256))
        ctx.pack(at + 0x4c, 'I', int(max(0.0, stamp)) if c['replay'] else t)
        stamp -= rng.uniform(40, 120)
        # motion (6 bits, 17 without one, 0x12.. a chained trick), then crashDirection:3,
        # crashed:1 (usually the bike's current crash state), flag4..flag7, flag8, flag9
        # (a running motion usually gets its chained trick, so the chain test runs)
        chain = 0x11 + (c['motion_index'] & 0x3f)
        motion = rng.choice([0, rng.randrange(1, 0x11), 0x11, chain, rng.randrange(64)]
                            + [chain] * (3 * c['motion']))
        crashed = c['crash_prev'] if rng.random() < 0.7 else 1 - c['crash_prev']
        flags = (rng.randrange(256) & ~0x08) | (crashed << 3)
        ctx.pack(at + 0x50, '4B', motion, flags, rng.randrange(256), rng.randrange(c['gates']))
        ctx.pack(at + 0x54, 'B', 0)
        ctx.pack(at + 0x58, 'Ii', t + rng.randrange(100), rng.choice([0, 1]))
        states.append(at)
    ctx.pack(bike + 0x11c8, '4I', *states)
    received = struct.unpack('<I', bytes(ctx.read(states[0] + 0x58, 4)))[0]
    ctx.pack(bike + 0x15ec, 'ffiI', c['elapsed'], c['length'], c['latch'],
             received if c['same_received'] else received + 1)
    ctx.pack(bike + 0x15fc, 'II', states[1], states[2])
    return dict(ecx=bike, args=[float(c['dt']), c['a']])
