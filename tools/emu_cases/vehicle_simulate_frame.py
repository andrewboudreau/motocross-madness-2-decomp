"""Vehicle::SimulateFrame 0x0052a940, slot 49 (tools/emu_diff.py case, call-record/replay mode).

The body orchestrates about 45 virtual slots and 20 direct callees, so every call leaving it is
recorded and replayed (``REPLAY``): both bodies get the same synthetic callee results and the
comparison checks the call sequence (targets, ECX, stack arguments, const pointees) and the
body's own writes.

The object is a KrustyBike (the game's vehicle) with its retail primary vtable 0x00554548, so
virtual calls resolve to the retail overrides. Fields follow VehicleCore.h and
SoultreePhysicsBaseObjectCore.h: floats random, flags and counts small, the pointers the body
dereferences (scene and centre nodes, engine state, smoothers, wheel list) point at seeded blocks.

Declared callee behaviour (the rest is the generic model):
- slot 9 (ComputeFrameSteps) writes 0..3 substeps through its out pointer;
- slot 28 (ResolveSubstepCollisions) usually returns its argument (keeps the loop short);
- slot 39 (ApplyPendingRespawn) returns a placement 15% of the time;
- slot 90 (ClampSubStepsOnCrash) writes 0..2 steps.
"""
import struct

RETAIL_VA = 0x0052a940
SYMBOL = '?SimulateFrame@Vehicle@@UAEXM@Z'
SOURCE = 'src/krusty2/vehicle/Vehicle.cpp'
KB_VT = 0x00554548
KB_VT2 = 0x00554514
PROBES = {}


def _steps_out(ctx, rec):
    ctx.pack(rec.args[1], 'i', rec.rng.choice([0, 1, 1, 2, 3]))


def _clamp_steps(ctx, rec):
    ctx.pack(rec.args[0], 'i', rec.rng.choice([0, 1, 1, 2]))


def _resolve(ctx, rec):
    return rec.args[0] if rec.rng.random() < 0.8 else rec.rng.choice([0, 1, 2])


def _rarely(ctx, rec):
    return rec.rng.choice([1, 2]) if rec.rng.random() < 0.15 else 0


REPLAY = dict(
    effects={('vslot', KB_VT, 9): _steps_out, ('vslot', KB_VT, 90): _clamp_steps},
    returns={('vslot', KB_VT, 28): _resolve, ('vslot', KB_VT, 39): _rarely},
)


def _f(rng, lo=-2.0, hi=2.0):
    return rng.uniform(lo, hi)


def cases(rng, n):
    for _ in range(n):
        c = dict(
            seed=rng.getrandbits(32),
            frame=rng.choice([1 / 60.0, 1 / 30.0, _f(rng, 0.0, 0.1)]),
            crash=rng.choice([0, 0, 0, 1]),
            prev_crash=rng.choice([0, 1]),
            point_count=rng.randrange(0, 5),
            touching=rng.choice([0, 0, 1, 2]),
            points_touching=rng.choice([0, 1]),
            wheels=rng.choice([0, 1, 2, 2]),
            wheel_count=2,
            contact_total=rng.choice([0, 1, 2, 3]),
            wheel_point=rng.choice([0, 1]),
            any_wheel=rng.choice([0, 1]),
            airborne=rng.choice([0, 1]),
            landing=rng.choice([0, 1]),
            spawn_timer=rng.choice([0.0, 0.0, _f(rng, 0, 0.05), _f(rng, 0, 2)]),
            speed=rng.choice([0.0, 0.0005, _f(rng, 0, 60), _f(rng, 0, 5), float('inf')]),
            prev_speed=rng.choice([0.0, _f(rng, 0, 60), _f(rng, 0, 5)]),
            same_weight=rng.random() < 0.5,
            step=rng.choice([1 / 60.0, 1 / 120.0, _f(rng, 0.001, 0.05)]),
            remainder=rng.choice([0.0, 0.0, 0.00005, _f(rng, 0, 0.02)]),
            tc=(_f(rng, 0.01, 1), _f(rng, 0.01, 1)),
            gear_timer=rng.choice([0.0, _f(rng, 0, 0.1)]),
        )
        yield c


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])

    def block(size, fill_floats=True):
        at = ctx.alloc(size)
        if fill_floats:
            ctx.pack(at, '%df' % (size // 4), *[rng.uniform(-2, 2) for _ in range(size // 4)])
        return at

    size = 0x1800
    bike = block(size)
    ctx.pack(bike, 'I', KB_VT)
    ctx.pack(bike + 0x21c, 'I', KB_VT2)
    scene = block(0x200)
    ctx.pack(scene, 'I', ctx.fake_vtable('sceneNode', 64))
    center = block(0x200)
    ctx.pack(center, 'I', ctx.fake_vtable('centerNode', 64))
    ctx.pack(bike + 0x08, 'I', scene)
    ctx.pack(bike + 0x218, 'I', center)
    ctx.pack(bike + 0x3bc, 'I', scene)       # D3DIMSoultreeCharacter::modelNode
    ctx.pack(bike + 0x42c, 'I', scene)
    ctx.pack(bike + 0xb8, 'ff', c['prev_speed'], c['speed'])
    ctx.pack(bike + 0x108, 'BBB', c['airborne'], 0, 0)
    ctx.pack(bike + 0x124, 'I', block(0x100))
    ctx.pack(bike + 0x128, 'I', block(0x100))
    ctx.pack(bike + 0x12c, 'I', block(0x40))
    ctx.pack(bike + 0x130, 'i', c['point_count'])
    ctx.pack(bike + 0x138, 'B', 0)
    step = c['step']
    ctx.pack(bike + 0x13c, 'fff', step, 1.0 / step, c['frame'])
    base = _f(rng, 100, 300)
    ctx.pack(bike + 0x150, 'ff', base, base if c['same_weight'] else base + _f(rng, 1, 50))
    ctx.pack(bike + 0x1c4, 'iii', rng.choice([0, 1]), 8, c['touching'])
    ctx.pack(bike + 0x1d0, 'B', c['points_touching'])
    ctx.pack(bike + 0x1d4, 'Iii', block(0x200), 4, 2)
    ctx.pack(bike + 0x1e0, 'fffi', c['remainder'], step, step, 4)
    ctx.pack(bike + 0x1f0, 'II', block(0x100), block(0x100))
    ctx.pack(bike + 0x200, 'Iii', block(0x40), 0, 0)
    ctx.pack(bike + 0x20c, 'BBBB', 0, 1, 0, 1)
    ctx.pack(bike + 0x210, 'i', 1)
    # Vehicle
    ctx.pack(bike + 0x440, 'iii', 1, c['crash'], 0)
    ctx.pack(bike + 0x460, 'ii', 0, 0)
    ctx.pack(bike + 0x468, 'II', block(0x100), block(0x40))
    ctx.pack(bike + 0x478, 'BBBB', 0, 0, 0, 0)
    steer = block(0x100)
    ctx.pack(steer, 'I', ctx.fake_vtable('steerState', 32))
    engine = block(0x1e4)
    ctx.pack(engine + 0x8, 'if', 0, c['gear_timer'])
    ctx.pack(bike + 0x47c, 'IIi', steer, engine, 0)
    ctx.pack(bike + 0x4a0, 'i', 0)
    ctx.pack(bike + 0x4a8, 'i', c['wheels'])
    ctx.pack(bike + 0x4f0, 'if', 0, c['spawn_timer'])
    ctx.pack(bike + 0x4f8, 'III', block(0x40), block(0x40), block(0x40))
    wheels = [block(0x200) for _ in range(c['wheel_count'])]
    wl = ctx.alloc(4 * len(wheels), struct.pack('<%dI' % len(wheels), *wheels))
    ctx.pack(bike + 0x53c, 'Iii', wl, len(wheels), len(wheels))
    ctx.pack(bike + 0x548, 'IIi', wheels[0], wheels[-1], 0)
    ctx.pack(bike + 0x554, 'Iii', block(0x20), 0, 0)
    ctx.pack(bike + 0x560, 'Iiii', block(0x20), 0, 0, 0)
    ctx.pack(bike + 0x570, 'i', c['contact_total'])
    for off, tc in ((0x58c, c['tc'][0]), (0x598, c['tc'][1])):
        sm = ctx.alloc(0x14)
        ctx.pack(sm, '5f', _f(rng), tc, _f(rng, 0, 1), 3.4e38, -3.4e38)
        ctx.pack(bike + off, 'I', sm)
    ctx.pack(bike + 0x59c, 'iiii', c['landing'], c['wheel_point'], c['any_wheel'], 0)
    ctx.pack(bike + 0x5ac, 'I', block(0x100))
    ctx.pack(bike + 0x5b4, 'i', c['prev_crash'])
    return dict(ecx=bike, args=[float(c['frame'])])
