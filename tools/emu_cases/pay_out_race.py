"""PCLastRaceDlg::PayOutRace 0x004d8c40, the pro-circuit race payout (tools/emu_diff.py case).

The linked body is the bring-up src/bringup/Group0_PCLastRaceDlg.cpp. Offsets from the retail
body: g_TrackGame+0x3444 the circuit (series +0x40, feesPaid +0x454, racerCount +0x460, racers
+0x465 stride 0x140 with race stats points/purse/repair/medical/rank/stunt at +0..+0x14, the
season stats at +0x18..+0x2c and money +0x30, entry fees +0x1229, purse multiplier +0x1241,
repair/medical/stunt caps +0x1245/+0x1249/+0x124d, place percentages +0x1251) and
g_TrackGame+0x570 the event manager (result count +0x4c, results inline from +0x50 stride
0x50: racer +0, position +4, points +0x28, repairs +0x34, medical +0x38, stunts +0x3c).
Fees, multipliers, caps and the medical/repair/stunt amounts are random around the cap
crossings (the __min arms), percentages random or tied, points tied often (both bodies run
retail's qsort 0x00534426 with retail's comparator, __ftol and ceil). DebugMalloc 0x004a2e20
is a stub that notes size and line. The ignored `this` is a dummy pointer.
"""
import struct

RETAIL_VA = 0x004d8c40
SYMBOL = '?PayOutRace@PCLastRaceDlg@@QAEXXZ'
SOURCE = 'src/bringup/Group0_PCLastRaceDlg.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x004d8d1f: 'result loop', 0x004d8d52: 'medical cap', 0x004d8ea1: 'race rank sort',
          0x004d8f76: 'season rank sort'}


def cases(rng, n):
    for _ in range(n):
        count = rng.randrange(1, 12)
        results = []
        racers = list(range(count))
        rng.shuffle(racers)
        places = list(range(1, count + 1))
        rng.shuffle(places)
        for j in range(rng.randrange(0, count + 1)):
            results.append(dict(racer=racers[j], pos=places[j], points=rng.choice([0, 1, 5, 10, rng.randrange(30)]),
                                medical=rng.choice([0.0, float(rng.randrange(20)), rng.uniform(0, 20)]),
                                repairs=rng.choice([0.0, float(rng.randrange(20)), rng.uniform(0, 20)]),
                                stunts=rng.choice([0.0, float(rng.randrange(40)), rng.uniform(0, 40)])))
        pct_kind = rng.randrange(4)
        if pct_kind == 0:
            pct = [1.0] * count
        elif pct_kind == 1:
            pct = [float(count - i) for i in range(count)]
        else:
            pct = [rng.uniform(0, 50) for _ in range(count)]
        yield dict(count=count, results=results, pct=pct, series=rng.randrange(6),
                   fees=[rng.choice([0, 500, 1000, 2500, rng.randrange(100000)]) for _ in range(6)],
                   mult=rng.choice([1.0, 2.5, 7.0, rng.uniform(0, 12)]),
                   caps=[rng.choice([0.05, 0.1, 0.25, rng.uniform(0, 1)]) for _ in range(3)],
                   seed=rng.randrange(1 << 30))


def setup(ctx, c):
    import random
    r = random.Random(c['seed'])
    circuit = ctx.alloc(0x1400)
    ctx.pack(circuit + 0x40, 'i', c['series'])
    ctx.pack(circuit + 0x454, 'i', r.randrange(100000))
    ctx.pack(circuit + 0x460, 'i', c['count'])
    for i in range(c['count']):
        base = circuit + 0x465 + 0x140 * i
        ctx.write(base, struct.pack('<13i', *[r.randrange(-50, 5000) for _ in range(13)]))
    ctx.pack(circuit + 0x1229, '6i', *c['fees'])
    ctx.pack(circuit + 0x1241, 'ffff', c['mult'], *c['caps'])
    ctx.pack(circuit + 0x1251, '%df' % c['count'], *c['pct'])
    events = ctx.alloc(0x50 * (len(c['results']) + 2))
    ctx.pack(events + 0x4c, 'i', len(c['results']))
    for j, res in enumerate(c['results']):
        at = events + 0x50 + 0x50 * j
        ctx.write(at, struct.pack('<20f', *[r.uniform(-1, 1) for _ in range(20)]))
        ctx.pack(at, 'ii', res['racer'], res['pos'])
        ctx.pack(at + 0x28, 'i', res['points'])
        ctx.pack(at + 0x34, 'fff', res['repairs'], res['medical'], res['stunts'])
    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x570, 'I', events)
    ctx.pack(game + 0x3444, 'I', circuit)
    ctx.pack(TRACK_GAME, 'I', game)

    def malloc(cx, ecx, a):
        cx.note('DebugMalloc', a[0], a[2])
        return cx.alloc(a[0] + 4, fill=0x99)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    dlg = ctx.alloc(0x100, fill=0x44)
    return dict(ecx=dlg, args=[], ret_eax=False)
