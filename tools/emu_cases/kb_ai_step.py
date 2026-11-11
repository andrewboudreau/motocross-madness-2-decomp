"""KrustyBike::Fn_00413200 0x00413200, the AI bike's physics step (tools/emu_diff.py case,
call-record/replay mode).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp). Every call
leaving it is recorded and replayed (``REPLAY``): the look-ahead SimulateLookAhead 0x0040eca0,
PredictLanding 0x0040e370, the wheel shocks, the node transforms and about 25 virtual slots all
return the same synthetic results to both bodies, and the comparison checks the call sequence
(targets, ECX, stack arguments, const pointees) and the body's own writes.

The bike (primary vtable 0x00554548, so virtual calls resolve to the retail overrides) is random
floats with the fields the body tests set from the case: aiKind (player or AI branch), airborne,
crashState, hasFinished, spawnProtectTimer, the speeds against prevSpeed, contactTime, bodyUp.y
and the collision type. Race, wheels, wheel shocks, steer axis, collision object and nodes point
at seeded blocks.

Declared callee behaviour (the rest is the generic model):
- slot 39 (ApplyPendingRespawn) returns a placement 15% of the time;
- slot 90 (ClampSubStepsOnCrash) writes 0..2 steps;
- PredictLanding writes a landing time (sometimes <= 0) and a normal;
- SimulateLookAhead writes the airborne flag, impact and the two suspension travels.
"""
import struct

RETAIL_VA = 0x00413200
SYMBOL = '?Fn_00413200@KrustyBike@@QAEHM@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
KB_VT = 0x00554548
KB_VT2 = 0x00554514
PREDICT_LANDING = 0x0040e370
LOOK_AHEAD = 0x0040eca0
PROBES = {0x00413370: 'respawn placement', 0x004133b4: 'place wheels', 0x0041343a: 'player brake ramp',
          0x004134f0: 'AI PredictLanding', 0x00413ea9: 'crashed control', 0x00413eb9: 'air control',
          0x00414033: 'crash trigger'}


def _rarely(ctx, rec):
    return rec.rng.choice([1, 2]) if rec.rng.random() < 0.15 else 0


def _clamp_steps(ctx, rec):
    ctx.pack(rec.args[0], 'i', rec.rng.choice([0, 1, 1, 2]))


def _landing(ctx, rec):
    # (Vec3 pos, Vec3 vel, float* time, int, Vec3* normal, Terrain*)
    ctx.pack(rec.args[6], 'f', rec.rng.choice([-0.5, 0.0, rec.rng.uniform(0, 3)]))
    ctx.pack(rec.args[8], '3f', *[rec.rng.uniform(-1, 1) for _ in range(3)])


def _look_ahead(ctx, rec):
    # args: steer, front, rear, vel, 0, pos, fwd, up, dt, dt, terrain, &airborne, &impact,
    # probe, &frontTravel, &rearTravel, &frontContact, &rearContact, path, speed, offset, near
    a = rec.args
    ctx.pack(a[11], 'B', rec.rng.choice([0, 1]))
    ctx.pack(a[12], 'f', rec.rng.choice([0.0, rec.rng.uniform(-150, 150)]))
    ctx.pack(a[14], 'f', rec.rng.uniform(-0.5, 0.5))
    ctx.pack(a[15], 'f', rec.rng.uniform(-0.5, 0.5))
    for p in (a[1], a[2], a[3], a[5]):
        ctx.pack(p, '3f', *[rec.rng.uniform(-30, 30) for _ in range(3)])


REPLAY = dict(
    returns={('vslot', KB_VT, 39): _rarely},
    effects={('vslot', KB_VT, 90): _clamp_steps, PREDICT_LANDING: _landing, LOOK_AHEAD: _look_ahead},
    # the look-ahead's stack inputs: steer, the airborne flag and the impact accumulator
    pointees={LOOK_AHEAD: (0, 11, 12)},
)


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.getrandbits(32),
            dt=rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0.001, 0.1)]),
            ai=rng.choice([0, 1, 1, 1]),
            airborne=rng.choice([0, 1]),
            crash=rng.choice([0, 0, 0, 1]),
            finished=rng.choice([0, 0, 1]),
            racing=rng.choice([0, 1, 1]),
            spawn_timer=rng.choice([0.0, 0.0, rng.uniform(0, 0.05), rng.uniform(0, 2)]),
            speed=rng.uniform(0, 60),
            speed_jump=rng.choice([0.0, 0.0, rng.uniform(-60, 60)]),
            target_speed=rng.uniform(0, 80),
            contact=rng.choice([0, 1]),
            contact_time=rng.choice([0.0, rng.uniform(0, 0.6)]),
            collision_type=rng.choice([0, 1000, 100, 101, 68]),
            up_y=rng.choice([1.0, rng.uniform(-1, 1)]),
            height=rng.choice([0.0, rng.uniform(-50, 50), 20000.0, -20000.0]),
            control=(rng.uniform(-1.5, 1.5), rng.uniform(-1.5, 1.5)),
            normal=[rng.uniform(-1, 1) for _ in range(3)],
            engine_class=rng.randrange(3),
            front_contact=rng.choice([0, 1]),
            rear_contact=rng.choice([0, 1]),
        )


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])

    def block(size, lo=-2.0, hi=2.0):
        at = ctx.alloc(size)
        ctx.pack(at, '%df' % (size // 4), *[rng.uniform(lo, hi) for _ in range(size // 4)])
        return at

    bike = block(0x1700)
    ctx.pack(bike, 'I', KB_VT)
    ctx.pack(bike + 0x21c, 'I', KB_VT2)
    scene = block(0x200)
    ctx.pack(scene, 'I', ctx.fake_vtable('sceneNode', 64))
    center = block(0x200)
    ctx.pack(center, 'I', ctx.fake_vtable('centerNode', 64))
    ctx.pack(bike + 0x08, 'I', scene)
    ctx.pack(bike + 0x10, 'f', c['height'])
    ctx.pack(bike + 0x98, 'f', c['up_y'])
    ctx.pack(bike + 0xb8, 'ff', c['speed'] + c['speed_jump'], c['speed'])
    ctx.pack(bike + 0x108, 'B', c['airborne'])
    collision = block(0xb8)
    ctx.pack(collision + 0x58, 'i', c['contact'])
    ctx.pack(bike + 0x124, 'I', block(0x100))
    ctx.pack(bike + 0x128, 'I', collision)
    ctx.pack(bike + 0x12c, 'I', block(0x40))
    ctx.pack(bike + 0x134, 'i', c['collision_type'])
    ctx.pack(bike + 0x1f4, 'I', block(0x80))
    ctx.pack(bike + 0x218, 'I', center)
    ctx.pack(bike + 0x3bc, 'I', scene)       # D3DIMSoultreeCharacter::modelNode
    ctx.pack(bike + 0x42c, 'I', scene)
    ctx.pack(bike + 0x440, 'iii', 1, c['crash'], 0)
    steer = block(0x100)
    ctx.pack(steer, 'I', ctx.fake_vtable('steerState', 32))
    engine = block(0x1e4)
    ctx.pack(bike + 0x47c, 'II', steer, engine)
    ctx.pack(bike + 0x4f4, 'f', c['spawn_timer'])
    ctx.pack(bike + 0x4f8, 'I', block(0x40))
    ctx.pack(bike + 0x504, 'ff', *c['control'])
    sm = ctx.alloc(0x14)
    ctx.pack(sm, '5f', rng.uniform(-2, 2), rng.uniform(0.01, 1), rng.uniform(0, 1), 3.4e38, -3.4e38)
    ctx.pack(bike + 0x58c, 'I', sm)
    sm = ctx.alloc(0x14)
    ctx.pack(sm, '5f', rng.uniform(-2, 2), rng.uniform(0.01, 1), rng.uniform(0, 1), 3.4e38, -3.4e38)
    ctx.pack(bike + 0x598, 'I', sm)
    wheels = []
    for flag in (c['front_contact'], c['rear_contact']):
        w = block(0x300)
        ctx.pack(w + 0x260, 'i', flag)
        ctx.pack(w + 0x274, 'f', rng.uniform(0.5, 2))
        ctx.pack(w + 0x2ac, 'II', block(0x100), block(0x100))
        wheels.append(w)
    ctx.pack(bike + 0x5f0, 'II', *wheels)
    ctx.pack(bike + 0x604, 'I', block(0x80))
    ctx.pack(bike + 0x640, 'I', block(0x40))
    race = block(0x200)
    ctx.pack(race + 0x4c, 'I', block(0x80))
    ctx.pack(race + 0x18a, 'B', c['racing'])
    ctx.pack(bike + 0x734, 'B', c['ai'])
    ctx.pack(bike + 0x740, 'I', race)
    ctx.pack(bike + 0x778, 'f', 100.0)
    ctx.pack(bike + 0x79c, 'i', c['engine_class'])
    ctx.pack(bike + 0x7a4, 'B', c['finished'])
    ctx.pack(bike + 0x814, 'f', c['contact_time'])
    ctx.pack(bike + 0x828, '3f', *c['normal'])
    ctx.pack(bike + 0x96c, 'f', c['target_speed'])
    ctx.pack(bike + 0x15e0, 'I', block(0xb8))
    ctx.pack(bike + 0x11b0, 'i', rng.choice([0, 1]))
    return dict(ecx=bike, args=[float(c['dt'])])
