"""Scene::Tick 0x004eab00 (tools/emu_diff.py case).

Builds a scene animation table (entries of 0x44 bytes, random sets of 0x10) with characters
laid out as retail reads them:
- vtable at +0;
- vbptr at +4, whose second dword is the offset of the GameObject virtual base (enabled bit at
  base +0x29; Disable and Enable at its slots 4 and 5);
- motionStopped at +0xc;
- currentMotion at +0x18.
Motions hold frameCount (+0xa8), looping (+0xb0), an integer fps (+0xb4) and transitionTime
(+0xbc).

Stubs:
- rand (0x00534c1b) returns the case's draws;
- SetMotion 0x004a8b40, BlendToMotion 0x004a8b80, AdvanceMotion 0x004a6bb0 (returns a frame),
  virtual slot 7, RestartEntry 0x004eb040 and GameObject::Tick 0x004693d0 are logged with
  their arguments.

Edge inputs:
- remaining time on or next to half the transition time;
- random draws whose `rand * (1/32768) * count - 0.5` is a tie.
"""
import struct

RETAIL_VA = 0x004eab00
SYMBOL = '?Tick@Scene@@UAEHM@Z'
SOURCE = 'src/bringup/Group4_SceneTick.cpp'
PROBES = {0x004eadf1: 'random set pick fistp', 0x004eabc9: 'remaining time compare'}

VBOFF = 0x20


def cases(rng, n):
    for _ in range(n):
        motions = []
        for _ in range(rng.randrange(1, 4)):
            fps = rng.choice([15, 25, 30, 60, 7])
            frames = rng.randrange(1, 200)
            # transition chosen so that remaining == transition / 2 for some frame
            left = rng.randrange(0, frames + 1)
            transition = rng.choice([2.0 * left / fps, rng.uniform(0, 2), 2.0 * left / fps + 1e-7,
                                     2.0 * left / fps - 1e-7])
            motions.append((frames, rng.randrange(2), fps, transition, frames - left))
        entries = []
        n_entries = rng.randrange(1, 6)
        for _ in range(n_entries):
            seq = [rng.randrange(1, len(motions) + 1) for _ in range(rng.choice([0, 1, 1, 2, 3]))]
            entries.append(dict(
                flags=rng.choice([0x0c, 0x0e, 0x2c, 0x08, 0x0d, rng.randrange(64)]),
                current_motion=rng.randrange(len(motions)),
                seq=seq, seq_index=rng.randrange(max(len(seq), 1)),
                playing=rng.randrange(len(motions) + 1) - 1,   # -1: no current motion
                stopped=rng.randrange(2), enabled=rng.randrange(2),
                frame=rng.choice([m[4] for m in motions] + [rng.randrange(0, 200)])))
        sets = []
        for _ in range(rng.choice([0, 1, 2, 3])):
            count = rng.choice([0, 1, 2, 3, 5, 7])
            sets.append(dict(entry=rng.randrange(n_entries), count=count,
                             anim=[rng.randrange(n_entries) for _ in range(max(count, 1))],
                             motion=[rng.randrange(len(motions)) for _ in range(max(count, 1))]))
        draws = []
        for _ in range(4):
            c = rng.choice([1, 2, 3, 5, 7])
            k = rng.randrange(2 * c + 1)
            draws.append(min(max(round(k * 32768 / (2 * c)) + rng.choice([-1, 0, 0, 1]), 0), 32767)
                         if rng.randrange(2) else rng.randrange(32768))
        game = (rng.choice([0, 0, 0, 1]), rng.choice([1 / 30, 1 / 60, rng.uniform(0, 0.1)]),
                rng.choice([1.0, 0.5, 2.0, rng.uniform(0, 3)]))
        yield (motions, entries, sets, draws, game, rng.choice([0, 0, 1]), rng.randrange(1, 300))


def setup(ctx, case):
    motions, entries, sets, draws, (paused, frame_time, speed), hold, frame_ret = case
    queue = list(draws)
    ctx.stub_at(0x00534c1b, 'rand', ret=lambda c, e, a: queue.pop(0) if queue else 0)
    ctx.stub_at(0x004a8b40, 'SetMotion', pop=4)
    ctx.stub_at(0x004a8b80, 'BlendToMotion', pop=8)
    ctx.stub_at(0x004a6bb0, 'AdvanceMotion', pop=12, ret=frame_ret)
    ctx.stub_at(0x004eb040, 'RestartEntry', pop=16)
    ctx.stub_at(0x004693d0, 'GameObject::Tick', pop=4, ret=0x77)
    slot7 = ctx.stub('Character::slot7', pop=12, ret=frame_ret + 1)
    char_vt = ctx.vtable([ctx.stub('Character::vslot%d' % i) for i in range(7)] + [slot7])
    disable = ctx.stub('GameObject::Disable')
    enable = ctx.stub('GameObject::Enable')
    base_vt = ctx.vtable([ctx.stub('GameObject::vslot%d' % i) for i in range(4)] + [disable, enable])
    vbtable = ctx.alloc(8)
    ctx.pack(vbtable, '2i', -4, VBOFF)

    game = ctx.alloc(0x400, fill=0x11)
    ctx.pack(game + 0x1c4, 'I', paused)
    ctx.pack(game + 0x2e0, 'f', speed)
    ctx.pack(game + 0x2f0, 'f', frame_time)
    ctx.pack(0x0056e26c, 'I', game)

    motion_ptrs = []
    for frames, looping, fps, transition, _ in motions:
        m = ctx.alloc(0xc0, fill=0x2d)
        ctx.pack(m + 0xa8, 'i', frames)
        ctx.pack(m + 0xb0, 'I', looping)
        ctx.pack(m + 0xb4, 'i', fps)
        ctx.pack(m + 0xbc, 'f', transition)
        motion_ptrs.append(m)
    motion_array = ctx.alloc(4 * len(motion_ptrs))
    ctx.pack(motion_array, '%dI' % len(motion_ptrs), *motion_ptrs)

    table_entries = ctx.alloc(0x44 * len(entries), fill=0x5e)
    for i, e in enumerate(entries):
        ch = ctx.alloc(0x80, fill=0x4c)
        ctx.pack(ch, 'II', char_vt, vbtable)
        ctx.pack(ch + 0xc, 'I', e['stopped'])
        ctx.pack(ch + 0x18, 'I', motion_ptrs[e['playing']] if e['playing'] >= 0 else 0)
        ctx.pack(ch + 4 + VBOFF, 'I', base_vt)
        ctx.pack(ch + 4 + VBOFF + 0x29, 'B', e['enabled'] | 0x40)
        seq = ctx.alloc(max(len(e['seq']), 1))
        if e['seq']:
            ctx.write(seq, bytes(e['seq']))
        at = table_entries + 0x44 * i
        ctx.pack(at, 'B', e['flags'])
        ctx.pack(at + 4, 'I', ch)
        ctx.pack(at + 0x18, 'i', e['frame'])
        ctx.pack(at + 0x1c, 'i', len(motions))
        ctx.pack(at + 0x20, 'I', motion_array)
        ctx.pack(at + 0x24, 'bbbB', e['current_motion'], len(e['seq']), e['seq_index'], 0)
        ctx.pack(at + 0x28, 'I', seq)

    set_array = ctx.alloc(0x10 * max(len(sets), 1), fill=0x3e)
    for j, s in enumerate(sets):
        anim = ctx.alloc(len(s['anim']), bytes(s['anim']))
        mot = ctx.alloc(len(s['motion']), bytes(s['motion']))
        ctx.pack(set_array + 0x10 * j, 'bxxxIII', s['count'], table_entries + 0x44 * s['entry'], anim, mot)

    table = ctx.alloc(0x10)
    ctx.pack(table, 'iIiI', len(entries), table_entries, len(sets), set_array)
    scene = ctx.alloc(0x800, fill=0x5c)
    ctx.pack(scene + 0xb4, 'I', table)
    ctx.pack(scene + 0x7c0, 'I', hold)
    return dict(ecx=scene, args=[0.125])
