"""BikeRace::AimCamera 0x0041eb20, points the race camera at its current target (emu_diff case).

The linked body is the bring-up copy (src/bringup/Group1_BikeRaceCamera.cpp). Offsets from the
retail body: the camera view +0x50 (followingRacer byte +0x390, followedBike +0x3b0, the
inline follow's stores +0x274/+0x384..+0x394/+0x3b4), targetIndex +0xac, targetKind byte +0xb0,
targetRacer +0x14c, targetEntry +0x150, targetCaster +0x154, localRacer +0x38, racerSlots +0x3c,
aiRacers +0x40 (arrays of pointers), raceScene +0x58 (animation table +0xb4: count, 0x44-byte
entries, flags bit 3 keyFramed, character +4 with its enabled bit at +0x29 past the vbtable's
virtual-base offset, object +8 with its enabled bit at +0x25; static casters +0xb8: count and
0x28-byte records with the model at +4), raceShadow +0x5c, g_TrackGame (0x0056e26c) playerCount
+0x18 and bajaEnduroEvent +0x560. A racer has modelNode +0x3bc, riderCharacter +0x5c4 (its
modelNode +0x1a0), a flag record +0x740 (byte +0x18c) and lap gates +0x7b8/+0x7bc; a model node's
+0x27c gates the second show call.

Stubbed and logged (all thiscall): ClearFlag14c 0x004fdb50, the model-node calls 0x00444d40 and
0x00444d00 (one argument), the shadow's ClearCasters 0x004dab90 and AddCaster 0x004dab00, and
SelectLapGates 0x00404df0 (three arguments). Everything else (the follow, the target stores
and the flag tests) runs, so the compared state is the race and view fields, the call
arguments and the returned flag.
"""
import random

RETAIL_VA = 0x0041eb20
SYMBOL = '?AimCamera@BikeRace@@QAEHH@Z'
SOURCE = 'src/bringup/Group1_BikeRaceCamera.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x0041eb54: 'remote racer', 0x0041ed1f: 'local racer', 0x0041eea0: 'AI racer',
          0x0041ecd9: 'lap gates', 0x0041f070: 'caster', 0x0041f123: 'entry character',
          0x0041f162: 'entry object', 0x0041f19a: 'entry disabled', 0x0041f1a3: 'empty table'}


def cases(rng, n):
    for _ in range(n):
        players = rng.choice([1, 1, 2, 4])
        yield dict(seed=rng.randrange(1 << 30), players=players, following=rng.randrange(2),
                   caster=rng.randrange(2), racer=rng.choice([0, rng.randrange(8)]), entry=rng.randrange(0, 4),
                   entries=rng.choice([0, 4, 4, 4]), anim=rng.random() < 0.9,
                   cast=rng.randrange(0, 3), casters=rng.choice([0, 3, 3]), statics=rng.random() < 0.9,
                   shadow=rng.randrange(2), baja=rng.randrange(2))


def setup(ctx, c):
    r = random.Random(c['seed'])
    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x18, 'i', c['players'])
    ctx.pack(TRACK_GAME, 'I', game)
    if c['baja']:
        ctx.pack(game + 0x560, 'I', ctx.alloc(0x40))

    def node():
        p = ctx.alloc(0x280)
        ctx.pack(p + 0x27c, 'I', r.randrange(2))
        return p

    def racer():
        p = ctx.alloc(0x800)
        rider = ctx.alloc(0x1a4)
        ctx.pack(rider + 0x1a0, 'I', node())
        flags = ctx.alloc(0x190)
        ctx.pack(flags + 0x18c, 'B', r.randrange(2))
        ctx.pack(p + 0x3bc, 'I', node())
        ctx.pack(p + 0x5c4, 'I', rider)
        ctx.pack(p + 0x740, 'I', flags)
        ctx.pack(p + 0x7b8, 'ii', r.randrange(-1, 20), r.randrange(-1, 20))
        return p

    race = ctx.alloc(0x200, fill=0x5a)
    view = ctx.alloc(0x400, fill=0x33)
    ctx.pack(race + 0x50, 'I', view)
    ctx.pack(view + 0x390, 'B', c['following'])
    ctx.pack(view + 0x3b0, 'I', racer())
    ctx.pack(race + 0x14c, 'iii', c['racer'], c['entry'], c['cast'])
    ctx.pack(race + 0x38, 'I', racer())
    slots = ctx.alloc(32)
    ctx.pack(slots, '8I', *[racer() for _ in range(8)])
    ai = ctx.alloc(32)
    ctx.pack(ai, '8I', *[racer() for _ in range(8)])
    ctx.pack(race + 0x3c, 'II', slots, ai)
    ctx.pack(race + 0x5c, 'I', ctx.alloc(0x40) if c['shadow'] else 0)

    scene = ctx.alloc(0x100)
    ctx.pack(race + 0x58, 'I', scene)
    if c['anim']:
        table = ctx.alloc(8)
        entries = ctx.alloc(0x44 * 4)
        ctx.pack(table, 'iI', c['entries'], entries)
        for i in range(4):
            e = entries + 0x44 * i
            keyed = r.randrange(2)
            ctx.pack(e, 'B', (8 if keyed else 0) | (0x10 if r.random() < 0.4 else 0))
            enabled = 1 if r.random() < 0.7 else 0
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
        records = ctx.alloc(0x28 * 3)
        for i in range(3):
            ctx.pack(records + 0x28 * i + 4, 'I', ctx.alloc(0x10))
        ctx.pack(statics, 'iI', c['casters'], records)
        ctx.pack(scene + 0xb8, 'I', statics)

    ctx.stub_at(0x004fdb50, 'ModelNode::ClearFlag14c', pop=0)
    ctx.stub_at(0x00444d40, 'ModelNode::Fn_00444D40', pop=4)
    ctx.stub_at(0x00444d00, 'ModelNode::Fn_00444D00', pop=4)
    ctx.stub_at(0x004dab90, 'Shadow::ClearCasters', pop=0)
    ctx.stub_at(0x004dab00, 'Shadow::AddCaster', pop=4)
    ctx.stub_at(0x00404df0, 'BajaEnduroEvent::SelectLapGates', pop=12)
    return dict(ecx=race, args=[c['caster']], ret_eax=True)
