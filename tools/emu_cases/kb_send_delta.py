"""KrustyBike::SendStateDelta 0x00492ad0, the message 13 delta sender (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy. The bike is filled with small random floats
(so every byte-coded delta, pose byte and flag bit takes random values and the deltas fall on
both sides of the +-30 / +-2 pi limits), then the pointers and the tested values are set:
network and recorder present or not, record or network mode, the short / full interval
timers, the clock gap (< 0x80, < 1000, >= 1000 ms), the event type, stunt mode and the ping
timer. Stubs: ReadClock 0x004bfa80 (returns the state's time plus the chosen gap),
SendFullState (the full message), NetworkInterface::Send 0x004ac830, VCRInterface::QueueRecord
0x004e8720 (both note the message bytes they are handed) and virtual slot 51 (IsSpawnProtected).
__ftol runs as retail code.
"""
import struct

RETAIL_VA = 0x00492ad0
SYMBOL = '?SendStateDelta@KrustyBike@@QAEXMH@Z'
SOURCE = 'src/bringup/Group3_KrustyBike.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x00492b03: 'record', 0x00492b74: 'short interval due', 0x00492c54: 'velocity fits',
          0x00493061: 'clock read', 0x00493081: 'fine count', 0x00493085: 'coarse count',
          0x004931ef: 'Send 13', 0x00493211: 'QueueRecord 13', 0x004932de: 'ping', 0x00493348: 'record ping',
          0x0049337d: 'full (gap)', 0x004933c1: 'full (interval/limit)'}


def cases(rng, n):
    for _ in range(n):
        record = rng.choice([0, 1])
        spread = rng.choice([0.5, 2.0, 2.0, 8.0])
        yield dict(
            record=record, seed=rng.randrange(1 << 30), spread=spread,
            network=rng.choice([0, 1, 1]), recorder=rng.choice([0, 1, 1]),
            frame=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1)]),
            full=rng.uniform(0, 2), short=rng.uniform(0, 0.1), full_record=rng.uniform(0, 2),
            timer=rng.uniform(0, 2), short_timer=rng.uniform(0, 0.12),
            gap=rng.choice([rng.randrange(0x80), rng.randrange(0x80, 1000), rng.randrange(1000, 5000), 0x7f, 0x80, 999, 1000]),
            event=rng.choice([0, 1, 2, 3, 4, 5]), stunt=rng.choice([0, 1]),
            ping=rng.choice([rng.uniform(-0.1, 0.05), rng.uniform(0, 2)]),
            protected=rng.choice([0, 1, 0x100]), due=rng.choice([0, 1]),
        )


def setup(ctx, c):
    import random
    r = random.Random(c['seed'])
    bike = ctx.alloc(0x1700)
    s = c['spread']
    ctx.write(bike, struct.pack('<%df' % (0x1700 // 4), *[r.uniform(-s, s) for _ in range(0x1700 // 4)]))
    state = bike + (0x1390 if c['record'] else 0x1560)

    def clock(cx, ecx, a):
        return (cx.unpack(state + 0x34, 'I')[0] + c['gap']) & 0xffffffff

    def send(cx, ecx, a):
        cx.note('message', cx.read(a[1], a[2]).hex())

    def queue(cx, ecx, a):
        cx.note('message', cx.read(a[2], a[3]).hex())
    ctx.stub_at(0x004bfa80, 'ReadClock', ret=clock)
    ctx.stub_at(0x00492670, 'SendFullState', pop=12)
    ctx.stub_at(0x004ac830, 'NetworkInterface::Send', pop=20, ret=send)
    ctx.stub_at(0x004e8720, 'VCRInterface::QueueRecord', pop=16, ret=queue)
    slots = [ctx.stub('KrustyBike slot %d' % k) for k in range(64)]
    slots[51] = ctx.stub('KrustyBike::IsSpawnProtected', ret=c['protected'])
    ctx.pack(bike, 'I', ctx.vtable(slots))

    game = ctx.alloc(0x3500)
    ctx.pack(game + 8, 'I', ctx.alloc(0x40) if c['network'] else 0)
    ctx.pack(game + 0x2f0, 'f', c['frame'])
    ctx.pack(game + 0x2d74, 'i', c['event'])
    ctx.pack(game + 0x2eb8, 'i', c['stunt'])
    ctx.pack(game + 0x3414, 'fff', c['full'], c['short'], c['full_record'])
    ctx.pack(TRACK_GAME, 'I', game)

    race = ctx.alloc(0x400)
    ctx.pack(race + 0x3fb, 'B', c['due'])
    ctx.pack(bike + 0x740, 'I', race)
    ctx.pack(bike + 0x13fc, 'I', ctx.alloc(0x40) if c['recorder'] else 0)
    ctx.pack(state + 0x68, 'f', c['timer'])
    ctx.pack(bike + 0x15d0, 'f', c['short_timer'])
    ctx.pack(bike + 0x1604, 'f', c['ping'])
    ctx.pack(state + 0x34, 'I', r.randrange(1 << 32))
    return dict(ecx=bike, args=[float(c['frame']), c['record']])
