"""ChatOverlay::CollectRacers 0x0051d730, the overlay's racer name tags (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group3_RadarOverlay.cpp. Offsets from the retail body: the
overlay's target +0x18, overlay texture +0x2c, chat view +0x12c, cue rectangle +0x150 (left,
top), name line right +0x160, racers +0x16c (eleven), racer count +0x198, name tags +0x2d8 and
their rectangles +0x30c (16 bytes; right at +8); g_TrackGame (0x0056e26c) +0x2d74 the event
type (0 adds the fixed extra tag). NextBike 0x004204e0 answers 0..10 racers and then null;
`new` 0x004a3010 answers a fresh object with a fake vtable or (5%) null; the NameOverlay
constructor 0x00518d80 answers `this`, Create 0x00518e30 `this` or (10%) null; AppendChild,
SetFixedPosition 0x00519080 (its Vector3 compared), the tag's slot 4 (Disable) and
RedrawNameTags 0x0051e910 are logged; fs:[0] is page 0. The cue rectangle's ints straddle the
float-exact range.
"""
import random

RETAIL_VA = 0x0051d730
SYMBOL = '?CollectRacers@ChatOverlay@@QAEHPAUUnknownChatView@@@Z'
SOURCE = 'src/bringup/Group3_RadarOverlay.cpp'
PROBES = {0x0051d7bc: 'tag loop', 0x0051d84a: 'extra tag', 0x0051d8e6: 'fixed tag', 0x0051d7ee: 'new failed'}

TRACK_GAME = 0x0056e26c
NEXT_BIKE, NEW, CTOR, CREATE = 0x004204e0, 0x004a3010, 0x00518d80, 0x00518e30
TAG_POPS = {4: 0}


def _next_bike(ctx, rec):
    racers = ctx.data['racers']
    k = ctx.data['bike_index']
    ctx.data['bike_index'] = k + 1
    return racers[k] if k < len(racers) else 0


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    if rec.rng.random() < 0.05:
        return 0
    tag = ctx.alloc(0x180, fill=0x5a)
    ctx.pack(tag, 'I', ctx.data['tag_vt'])
    return tag


def _this(ctx, rec):
    return rec.ecx


def _create(ctx, rec):
    return rec.ecx if rec.rng.random() < 0.9 else 0


REPLAY = dict(returns={NEXT_BIKE: _next_bike, NEW: _new, CTOR: _this, CREATE: _create},
              pops={NEW: 0}, args={NEW: 3}, ecx={NEW: False}, unlogged=(NEW,))


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), racers=rng.choice([0, 1, 2, 5, 10]),
                   event_type=rng.choice([0, 0, 1, 2]),
                   cue=(rng.choice([0, -5, 123, 16777217, -16777219, 0x7fffffff]),
                        rng.choice([0, 7, -300, 33554435])))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)               # fs:[0] (the EH frame's link) is page 0
    r = random.Random(c['seed'])

    def noise(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    ctx.data['tag_vt'] = ctx.fake_vtable('NameOverlay', 8, pops=TAG_POPS)
    ctx.data['racers'] = [ctx.alloc(0x40, noise(0x40)) for _ in range(c['racers'])]
    ctx.data['bike_index'] = 0
    game = ctx.alloc(0x3000, noise(0x3000))
    ctx.pack(game + 0x2d74, 'i', c['event_type'])
    ctx.pack(TRACK_GAME, 'I', game)
    overlay = ctx.alloc(0x400, noise(0x400))
    ctx.pack(overlay + 0x150, 'ii', *c['cue'])
    view = ctx.alloc(0x100, noise(0x100))
    return dict(ecx=overlay, args=[view])
