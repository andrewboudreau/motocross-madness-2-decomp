"""BikeRace::NextCameraTarget 0x0041f1d0, the spectator camera's next/previous target.

The linked body is the bring-up copy (src/bringup/Group1_BikeRaceCamera.cpp). Offsets from the
retail body: g_TrackGame (0x0056e26c) +0x3430 uiInteractionBlocked, +0x18 playerCount, +0x3424
network AI count, +0x2d70 raceMode (4 = one other), +0x2d94 aiRacerCount; the race's camera
view +0x50 (overrideActive +0x268, cameraState +0x244 (7 tested), targetCharacter +0x384,
targetCar +0x388, shadowCaster +0x38c, followingRacer byte +0x390); targetRacer +0x14c,
targetEntry +0x150, targetCaster +0x154; localRacer +0x38, racerSlots +0x3c and aiRacers +0x40
(arrays of pointers, outOfRace at racer +0x4a0); raceScene +0x58 with the animation table
+0xb4 (count, 0x44-byte entries: flags byte bit 3 keyFramed, bit 4 excluded; a key-framed
character's enabled bit sits at +0x29 past its virtual base, reached through the vbtable at
character +4; an object's at +0x25) and the static casters +0xb8 (count).

AimCamera 0x0041eb20 is a stub (thiscall, logs its flag, returns 0 or 1 from the run's
stream), so the compared state is the target indices, followingRacer and the AimCamera calls.
Player counts 1..4, AI counts 0..7, random out-of-race racers, hidden or disabled entries
(including all of them, which ends the walk at the start index) and empty tables all occur.
The walks only stop at an active slot or back at the start index, so (as in the game) a start
index outside the range loops forever unless some slot is active: such cases keep racer 0
(the local racer and the first slot) in the race and entry 0 visible.
"""
import random

RETAIL_VA = 0x0041f1d0
SYMBOL = '?NextCameraTarget@BikeRace@@QAEXHHH@Z'
SOURCE = 'src/bringup/Group1_BikeRaceCamera.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x0041f244: 'aim on switch', 0x0041f2a4: 'racer walk', 0x0041f348: 'racer wrap',
          0x0041f38b: 'entry walk', 0x0041f422: 'entry wrap', 0x0041f4c8: 'caster step',
          0x0041f531: 'no casters'}


def cases(rng, n):
    for _ in range(n):
        players = rng.choice([1, 1, 1, 2, 4])
        yield dict(seed=rng.randrange(1 << 30), blocked=rng.random() < 0.05, players=players,
                   net_ai=rng.randrange(0, 4), race_mode=rng.choice([0, 4]), ai=rng.choice([0, 1, 3, 7]),
                   override=rng.random() < 0.1, state=rng.choice([0, 3, 7, 7]) if rng.random() < 0.3 else 1,
                   following=rng.randrange(2), forward=rng.randrange(2), racers=rng.randrange(2),
                   objects=rng.randrange(2), aim=rng.randrange(2), entries=rng.choice([0, 1, 3, 6]),
                   casters=rng.choice([0, 1, 2, 5]), anim_table=rng.random() < 0.9,
                   statics=rng.random() < 0.9, target=rng.randrange(-1, 9), entry=rng.randrange(-1, 7),
                   caster=rng.randrange(-1, 6), shadow=rng.randrange(2), tchar=rng.random() < 0.2,
                   tcar=rng.random() < 0.2)


def setup(ctx, c):
    r = random.Random(c['seed'])
    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x3430, 'I', int(c['blocked']))
    ctx.pack(game + 0x18, 'i', c['players'])
    ctx.pack(game + 0x3424, 'i', c['net_ai'])
    ctx.pack(game + 0x2d70, 'i', c['race_mode'])
    ctx.pack(game + 0x2d94, 'i', c['ai'])
    ctx.pack(TRACK_GAME, 'I', game)

    race = ctx.alloc(0x200)
    view = ctx.alloc(0x400)
    ctx.pack(race + 0x50, 'I', view)
    ctx.pack(view + 0x268, 'I', int(c['override']))
    ctx.pack(view + 0x244, 'i', c['state'])
    ctx.pack(view + 0x384, 'II', int(c['tchar']), int(c['tcar']))
    ctx.pack(view + 0x38c, 'I', c['shadow'])
    ctx.pack(view + 0x390, 'B', c['following'])
    ctx.pack(race + 0x14c, 'iii', c['target'], c['entry'], c['caster'])

    others = c['net_ai'] if c['players'] > 1 else 1 if c['race_mode'] == 4 else c['ai']
    racer_escape = not 0 <= c['target'] < c['players'] + others

    def racer(active=False):
        p = ctx.alloc(0x4a4)
        ctx.pack(p + 0x4a0, 'I', 1 if r.random() < 0.6 and not active else 0)
        return p
    slots = [racer(racer_escape and i == 0) for i in range(16)]
    local = racer(racer_escape)
    ctx.pack(race + 0x38, 'I', local)
    arr = ctx.alloc(64)
    ctx.pack(arr, '16I', *slots[:16])
    ai_arr = ctx.alloc(64)
    ctx.pack(ai_arr, '16I', *[racer() for _ in range(16)])
    ctx.pack(race + 0x3c, 'II', arr, ai_arr)

    scene = ctx.alloc(0x100)
    ctx.pack(race + 0x58, 'I', scene)
    if c['anim_table']:
        table = ctx.alloc(8)
        entries = ctx.alloc(0x44 * max(c['entries'], 1))
        ctx.pack(table, 'iI', c['entries'], entries)
        escape = not 0 <= c['entry'] < c['entries']
        for i in range(c['entries']):
            e = entries + 0x44 * i
            keyed = r.randrange(2)
            visible = escape and i == 0
            ctx.pack(e, 'B', (8 if keyed else 0) | (0x10 if r.random() < 0.4 and not visible else 0))
            enabled = 1 if r.random() < 0.75 or visible else 0
            if keyed:
                off = r.choice([0x10, 0x20])
                ch = ctx.alloc(0x80)
                vbt = ctx.alloc(8)
                ctx.pack(vbt, 'ii', 0, off)
                ctx.pack(ch + 4, 'I', vbt)
                ctx.pack(ch + off + 0x29, 'B', enabled | 0x40)
                ctx.pack(e + 4, 'I', ch)
            else:
                ob = ctx.alloc(0x40)
                ctx.pack(ob + 0x25, 'B', enabled | 0x80)
                ctx.pack(e + 8, 'I', ob)
        ctx.pack(scene + 0xb4, 'I', table)
    if c['statics']:
        statics = ctx.alloc(8)
        ctx.pack(statics, 'i', c['casters'])
        ctx.pack(scene + 0xb8, 'I', statics)

    ctx.stub_at(0x0041eb20, 'BikeRace::AimCamera', pop=4, ret=lambda cx, e, a: c['aim'] if cx.rng.random() < 0.5 else cx.rng.randrange(2))
    return dict(ecx=race, args=[c['forward'], c['racers'], c['objects']], ret_eax=False)
