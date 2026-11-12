"""KrustyBike::UpdateRiderPose 0x0048eea0, slot 102 (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy. Five starting states: crashed (the steer pose),
a trick clip (start, the held / AI hold-window arms, remote and local release, the chain
switch for every trick % 8 and input), the head turn towards the nearest rival (turning back,
turning towards it on both sides with the re-aim near the target, holding), the finish pose,
and the riding pose blend (crossover, lean, pose 16 back-up). Every callee is a stub that logs
its arguments: the Character pose setters 0x004a8bf0 / 0x004a8c50 / 0x004a9050 / SetMotion
0x004a8b40, Fn_0048D990, Get/SetMatrixIn (0x004fca80 writes a matrix, 0x004fb8c0 logs the one
it is handed), and the virtual slots: this 84 (QueryControlInput), 88 (ClassifyInputDirection),
41 (EndMotion), and slot 7 on the rider and on the character subobject at +0x21c.

Trick indices cover chained tricks (16..31) too. Retail indexes s_kbTrickFrame (0x0056d060,
16 entries) without a bound in the hold-window arm, so a chained trick reads the 16 words after
the table in the retail image (s_kbTimeTo60Armed, the KrustyBike TypeDescriptor and two string
literals); the bring-up's 32-entry copy repeats those words, and the candidate reads its own
copy (a file-local static, not bound to retail).
"""
import math
import struct

RETAIL_VA = 0x0048eea0
SYMBOL = '?UpdateRiderPose@KrustyBike@@UAEXM@Z'
SOURCE = 'src/bringup/Group3_KrustyBike.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x0048f968: 'crash steer pose', 0x0048eee9: 'trick start', 0x0048efda: 'trick remote',
          0x0048f061: 'trick chain switch', 0x0048f186: 'AI hold window', 0x0048f1bd: 'player hold window',
          0x0048f247: 'head back done', 0x0048f2e0: 'head + re-aim', 0x0048f4a9: 'head - re-aim',
          0x0048f35d: 'head hold (+)', 0x0048f3bd: 'head clamp +2.7', 0x0048f55d: 'head clamp -2.7',
          0x0048f5d5: 'head hold expired', 0x0048f61f: 'finish pose', 0x0048f66a: 'EndMotion',
          0x0048f723: 'pose 16', 0x0048f799: 'back-up wrap', 0x0048f822: 'rider blend',
          0x0048f856: 'crossover bike pose', 0x0048f8dc: 'bike blend', 0x0048f91e: 'matrix copy'}

TRICK_FRAME = [14, 16, 17, 18, 11, 14, 17, 18, 15, 17, 15, 18, 15, 15, 17, 16,
               # retail's .data after the table (0x0056d0a0..0x0056d0e0)
               1, 0, 0x00558f4c, 0, 0x56413f2e, 0x7375724b, 0x69427974, 0x4040656b,
               0, 0x65646952, 0x726f4d72, 0x6d2e6870, 0x00006662, 0x65746e49, 0x6c6f7072, 0x00657461]


def _f(rng, lo, hi):
    return rng.uniform(lo, hi)


def cases(rng, n):
    for _ in range(n):
        state = rng.choice(['crash', 'trick', 'trick', 'head', 'head', 'head', 'finish', 'pose', 'pose'])
        idx = rng.randrange(32)
        c = dict(
            state=state,
            dt=rng.choice([1 / 60.0, 1 / 30.0, _f(rng, 0.0, 0.2)]),
            crash=rng.choice([0, 1]) if state == 'crash' else 0,
            pad5b8=1 if state == 'crash' and rng.random() < 0.5 else 0,
            remote=rng.choice([0, 0, 1]), replay=rng.choice([0, 0, 1]), ai=rng.choice([0, 0, 1]),
            steer=(_f(rng, -1, 1), rng.choice([1.0, 0.5, _f(rng, 0.1, 2)])),
            pose_param=_f(rng, 0, 1),
            motion_index=idx, just_started=rng.random() < 0.15,
            input=[rng.choice([0, 1]), rng.choice([0, 1])], direction=rng.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7]),
            clip=_f(rng, 0, 3), hold=_f(rng, 0, 2),
            ai_hold=rng.choice([0.0, 0.0, _f(rng, 0, 0.05), _f(rng, -0.01, 0.5)]),
            held=rng.choice([0, 1]), held_prev=rng.choice([0, 1]), anim_c=rng.choice([0, 1]),
            frame=rng.choice([TRICK_FRAME[idx] + k for k in (-1, 0, 1, 2, 3, 4)] + [rng.randrange(40)]),
            back=rng.random() < 0.3, to_rival=rng.random() < 0.6,
            scaled=rng.choice([0.0, _f(rng, -0.7, 0.7), _f(rng, -0.7, 0.7), 0.3, -0.3]),
            head=_f(rng, -0.8, 0.8), bearing=_f(rng, -2.7, 2.7),
            rival=(_f(rng, -50, 50), _f(rng, -5, 5), _f(rng, -50, 50)),
            position=(_f(rng, -50, 50), 0.0, _f(rng, -50, 50)), yaw=_f(rng, -7, 7),
            finished=2 if state == 'finish' else rng.choice([0, 0, 1, 3]),
            ready=rng.choice([0, 1]), crossover=rng.choice([0, 0, 1]),
            pose_index=rng.choice(list(range(14)) + [16, 16, 10, 0, 1, 2]),
            pose_state=rng.randrange(12), pose_blend=_f(rng, 0, 1), lean_blend=_f(rng, 0, 1),
            backup=rng.choice([_f(rng, 0, 0.45), _f(rng, 0.45, 1), _f(rng, 0.9, 1.0)]),
            backup_rate=_f(rng, 0, 0.2), speed=_f(rng, -30, 30),
            wreck=rng.choice([0, 1, 2, 5]),
        )
        if state == 'trick' and rng.random() < 0.5:
            # The local release with a chain test: no input, no AI hold, pose held.
            c.update(remote=0, replay=0, input=[0, 0], ai_hold=0.0, held=1, anim_c=1)
        if state == 'trick' and rng.random() < 0.3:
            # The hold window of a chained trick, where retail reads past its 16-entry table.
            idx = 16 + rng.randrange(16)
            c.update(motion_index=idx, just_started=False,
                     frame=rng.choice([TRICK_FRAME[idx] + k for k in (-1, 0, 1, 2, 3, 4)] + [rng.randrange(40)]))
            if rng.random() < 0.5:
                c.update(ai=0, remote=0, replay=0, input=[1, rng.choice([0, 1])])
            else:
                c.update(ai_hold=rng.choice([_f(rng, 0, 0.05), _f(rng, 0.05, 0.5)]))
        if state == 'head':
            # The rival's bearing relative to the heading, often beyond the +-2.7 clamp.
            angle = rng.choice([_f(rng, -3.1, 3.1), _f(rng, 2.7, 3.14), _f(rng, -3.14, -2.7)])
            r = _f(rng, 1, 60)
            px, _, pz = c['position']
            c['rival'] = (px + r * math.sin(angle + c['yaw']), _f(rng, -5, 5), pz + r * math.cos(angle + c['yaw']))
        if state == 'head' and rng.random() < 0.5:
            # Near the target so the re-aim runs: head within 0.1 of the scaled bearing.
            c['head'] = c['scaled'] + rng.choice([-1, 1]) * _f(rng, 0, 0.1) - (c['dt'] if c['scaled'] > 0 else -c['dt'])
        yield c


def setup(ctx, c):
    def note_matrix(cx, ecx, a):
        cx.note('SetMatrixIn', cx.read(a[1], 64).hex())

    def get_matrix(cx, ecx, a):
        cx.pack(a[1], '16f', *[float(k) + 0.25 for k in range(16)])

    ctx.stub_at(0x0048d990, 'Fn_0048D990', pop=4)
    ctx.stub_at(0x004a8b40, 'Character::SetMotion', pop=4)
    ctx.stub_at(0x004a8bf0, 'Character::ApplyMotionAtPhase', pop=8)
    ctx.stub_at(0x004a8c50, 'Character::ApplyMotionBlendAtPhase', pop=16)
    ctx.stub_at(0x004a9050, 'Character::ApplyFourMotionBlendAtPhase', pop=28)
    ctx.stub_at(0x004fca80, 'SoultreeObject::GetMatrixIn', pop=8, ret=get_matrix)
    ctx.stub_at(0x004fb8c0, 'SoultreeObject::SetMatrixIn', pop=8, ret=note_matrix)

    inputs = list(c['input'])

    def query(cx, ecx, a):
        return inputs[0] if a[0] == 6 else inputs[1]
    slots = [ctx.stub('KrustyBike slot %d' % k) for k in range(96)]
    slots[84] = ctx.stub('KrustyBike::QueryControlInput', pop=8, ret=query)
    slots[88] = ctx.stub('KrustyBike::ClassifyInputDirection', ret=c['direction'])
    slots[41] = ctx.stub('KrustyBike::EndMotion')
    character_slots = [ctx.stub('Character@+0x21c slot %d' % k) for k in range(8)]
    character_slots[7] = ctx.stub('Character@+0x21c slot 7', pop=12)
    rider_slots = [ctx.stub('rider slot %d' % k) for k in range(8)]
    rider_slots[7] = ctx.stub('rider slot 7', pop=12)

    game = ctx.alloc(0x3500)
    ctx.pack(game + 0x3428, 'i', c['replay'])
    ctx.pack(TRACK_GAME, 'I', game)

    bike = ctx.alloc(0x1600, fill=0x5a)
    ctx.pack(bike, 'I', ctx.vtable(slots))
    ctx.pack(bike + 0x0c, '3f', *c['position'])
    ctx.pack(bike + 0x50, 'f', c['yaw'])
    ctx.pack(bike + 0x21c, 'I', ctx.vtable(character_slots))
    frame = ctx.alloc(16)
    ctx.pack(frame, 'i', c['frame'])
    ctx.pack(bike + 0x238, 'I', frame)
    model = ctx.alloc(0x40)
    ctx.pack(bike + 0x3bc, 'I', model)
    state = c['state']
    ctx.pack(bike + 0x430, 'BBBb', 1 if state in ('trick', 'head') else 0, 0x33,
             1 if c['just_started'] else 0, c['motion_index'] if state == 'trick' else -1)
    ctx.pack(bike + 0x434, 'f', c['speed'])
    ctx.pack(bike + 0x444, 'i', c['crash'])
    steer = ctx.alloc(16)
    ctx.pack(steer + 4, 'ff', *c['steer'])
    ctx.pack(bike + 0x47c, 'I', steer)
    ctx.pack(bike + 0x5b8, 'i', c['pad5b8'])
    rider = ctx.alloc(0x200, fill=0x11)
    ctx.pack(rider, 'I', ctx.vtable(rider_slots))
    ctx.pack(rider + 0xc, 'i', c['ready'])
    ctx.pack(rider + 0x1a0, 'I', ctx.alloc(0x40))
    ctx.pack(bike + 0x5c4, 'I', rider)
    wrecker = ctx.alloc(0x80)
    ctx.pack(wrecker + 0x44, 'i', c['wreck'])
    ctx.pack(bike + 0x604, 'I', wrecker)
    ctx.pack(bike + 0x650, 'ifiifff', c['pose_index'], c['pose_blend'], c['pose_state'],
             0, c['lean_blend'], c['backup'], c['backup_rate'])
    ctx.pack(bike + 0x65c, 'f', c['pose_param'])
    ctx.pack(bike + 0x660, 'f', c['lean_blend'])
    ctx.pack(bike + 0x66c, '18i', *[0x1000 + k for k in range(18)])
    ctx.pack(bike + 0x6b4, '18i', *[0x2000 + k for k in range(18)])
    ctx.pack(bike + 0x6fc, 'i', c['crossover'])
    ctx.pack(bike + 0x734, 'BB', c['ai'], c['remote'])
    ctx.pack(bike + 0x7a4, 'B', c['finished'])
    ctx.pack(bike + 0x14a0, '16i', *[c['anim_c'] * (0x3000 + k) for k in range(16)])
    ctx.pack(bike + 0x1520, 'ffff', c['clip'], 0.0, c['hold'], c['ai_hold'])
    ctx.pack(bike + 0x153c, 'BBB', c['held'], 0, c['held_prev'])
    ctx.pack(bike + 0x154d, 'BB', 1 if c['to_rival'] else 0, 1 if c['back'] else 0)
    ctx.pack(bike + 0x1550, 'fff', c['scaled'], c['head'], c['bearing'])
    rival = ctx.alloc(0x40)
    ctx.pack(rival + 0xc, '3f', *c['rival'])
    ctx.pack(bike + 0x155c, 'I', rival)
    return dict(ecx=bike, args=[float(c['dt'])])
