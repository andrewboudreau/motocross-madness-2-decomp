"""TrackGameViewOwner::PickFollowedRacer 0x004a9d20, the host's followed racer (tools/emu_diff.py case).

The candidate is samples/race/FollowedRacerPicker.cpp. Offsets from the retail body: owner
+0x34 race view (+0x38 local racer, +0x3c racer slots, +0x158 racer count, +0x1a0 replay VCR),
+0xa8 followed racer; racer +0x4a0 outOfRace (0, 1 and 2: only 1 is skipped by the slot walk,
anything nonzero by the count), +0x11bc network player id; g_TrackGame +0x08 network (+0x0c
local player, +0x10 isHost), +0x3334 saveReplay, +0x3428 playingReplay. BikeRace::NextBike
0x004204e0 is a stub walking a per-run list of racers drawn from a small pool (so the bikes
and the slots can disagree); rand 0x00534c1b is a stub (0, 32767 and random values); __ftol
runs as retail code. Network::Send 0x004ac830 and VCRInterface::QueueRecord 0x004e8720 are
stubs that note the message's defined bytes (+4 byte, +8 player id; the rest is
uninitialised in retail).
"""
RETAIL_VA = 0x004a9d20
SYMBOL = '?PickFollowedRacer@TrackGameViewOwner@@QAEXXZ'
SOURCE = 'samples/race/FollowedRacerPicker.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x004a9d7d: 'none racing', 0x004a9d90: 'one racing', 0x004a9da5: 'random pick', 0x004a9dd8: 'skip slot',
          0x004a9e69: 'record', 0x004a9e6e: 'exit'}


def cases(rng, n):
    for _ in range(n):
        pool = rng.randrange(1, 9)
        out = [rng.choice([0, 0, 1, 1, 2]) for _ in range(pool)]
        if rng.random() < 0.2:
            out = [rng.choice([1, 2]) for _ in range(pool)]
        bikes = [rng.randrange(pool) for _ in range(rng.randrange(0, 9))]
        slots = [rng.randrange(pool) for _ in range(rng.randrange(1, 9))]
        if rng.random() < 0.5:
            bikes = list(range(pool))
            slots = list(range(pool))
        yield dict(out=out, bikes=bikes, slots=slots, network=rng.random() < 0.95, host=rng.choice([0, 1, 1, 1, 1]),
                   rand=rng.choice([0, 32767, rng.randrange(32768), rng.randrange(32768)]),
                   vcr=rng.random() < 0.7, save=rng.choice([0, 1, 1]), playing=rng.choice([0, 0, 1]),
                   local=rng.randrange(pool))


def setup(ctx, c):
    racers = []
    for i, o in enumerate(c['out']):
        r = ctx.alloc(0x11c0, fill=0x3c)
        ctx.pack(r + 0x4a0, 'i', o)
        ctx.pack(r + 0x11bc, 'i', 1000 + i)
        racers.append(r)
    slots = ctx.alloc(4 * len(c['slots']))
    ctx.pack(slots, '%dI' % len(c['slots']), *[racers[i] for i in c['slots']])
    view = ctx.alloc(0x200, fill=0x5d)
    ctx.pack(view + 0x38, 'II', racers[c['local']], slots)
    ctx.pack(view + 0x158, 'i', len(c['slots']))
    ctx.pack(view + 0x1a0, 'I', ctx.alloc(0x10) if c['vcr'] else 0)
    owner = ctx.alloc(0xb0, fill=0x6e)
    ctx.pack(owner + 0x34, 'I', view)
    game = ctx.alloc(0x3500)
    network = ctx.alloc(0x20)
    ctx.pack(network + 0x0c, 'ii', 7, c['host'])
    ctx.pack(game + 0x08, 'I', network if c['network'] else 0)
    ctx.pack(game + 0x3334, 'i', c['save'])
    ctx.pack(game + 0x3428, 'i', c['playing'])
    ctx.pack(TRACK_GAME, 'I', game)
    bikes = [racers[i] for i in c['bikes']]

    def next_bike(cx, e, a):           # thiscall (int* iterator)
        i = cx.unpack(a[0], 'i')[0]
        cx.pack(a[0], 'i', i + 1)
        return bikes[i] if 0 <= i < len(bikes) else 0
    ctx.stub_at(0x004204e0, 'BikeRace::NextBike', pop=4, ret=next_bike)
    ctx.stub_at(0x00534c1b, 'rand', ret=c['rand'])

    def message(cx, at):
        return cx.read(at + 4, 1).hex(), cx.read(at + 8, 4).hex()
    ctx.stub_at(0x004ac830, 'Network::Send', pop=20, nargs=5,
                ret=lambda cx, e, a: cx.note('Send', e, a[0], message(cx, a[1]), a[2], a[3], a[4]), log=False)
    ctx.stub_at(0x004e8720, 'VCRInterface::QueueRecord', pop=16, nargs=4,
                ret=lambda cx, e, a: cx.note('QueueRecord', e, a[0], a[1], message(cx, a[2]), a[3]), log=False)
    return dict(ecx=owner, args=[], ret_eax=False)
