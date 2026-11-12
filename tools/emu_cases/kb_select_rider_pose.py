"""KrustyBike::SelectRiderPose 0x0048e3e0, the rider pose selection (tools/emu_diff.py case).

The candidate is samples/physics/krustybike/KrustyBikeSelectRiderPose.cpp, compiled with its
configured /QIfist. Offsets from the retail body: bike +0x108 airborne (byte), +0x444
crashState, +0x47a backingUp (byte), +0x504 controlInput.x, +0x510 prevControlInput.x, +0x520
steerPoseState (0..6, so the default arms of both switches run), +0x5b8 fixed-pose flag,
+0x5f0/+0x5f4 front/rear wheel (+0x150 penetration, +0x260 inContact, +0x27c slipSpeed),
+0x63c crossupScale, +0x640 steer axis (+0 steerValue), +0x644 pose smoother (+0 value, +4
time, +8 ratio, +0xc maxRise, +0x10 maxFall), +0x650 poseIndex, +0x654 poseBlend, +0x658
poseState, +0x65c poseParam, +0x660 poseLeanBlend, +0x6fc crossoverPose, +0x704 smooth time
(FLT_MAX in some runs), +0x734..+0x736 aiKind/isRemote/isGhost. The steer value sits on the
pose boundaries (0.75 - (k + 0.5) / 3.99, +-1 ulp), in range, far out of range, infinite or
NaN (the checked fistp stores 0x80000000). The vtable slots IsRiderPoseLocked (+0x108),
QueryControlInput (+0x150), ClassifyInputDirection (+0x160) and EndMotion (+0xa4) and the
trick starters 0x0048e280 / 0x0048d910 are stubs. Retail's trick-frame table (0x0056d060) is
not read by this body or those two callees.
"""
import struct

RETAIL_VA = 0x0048e3e0
SYMBOL = '?SelectRiderPose@KrustyBike@@UAEXM@Z'
SOURCE = 'samples/physics/krustybike/KrustyBikeSelectRiderPose.cpp'
PROBES = {0x0048e408: 'fixed pose', 0x0048e533: 'head turn', 0x0048e584: 'knocked off', 0x0048e5fc: 'crossover',
          0x0048e65b: 'smoother reset', 0x0048e6cd: 'slip pose', 0x0048e8df: 'crossover switch',
          0x0048ea4a: 'steer switch', 0x0048ee6b: 'exit'}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def ulp(x, k):
    i = struct.unpack('<i', struct.pack('<f', x))[0]
    return struct.unpack('<f', struct.pack('<i', i + k))[0]


def cases(rng, n):
    for _ in range(n):
        k = rng.randrange(-3, 6)
        steer = rng.choice([ulp(f32(0.75 - (k + 0.5) / 3.99), rng.choice([-1, 0, 0, 1])), rng.uniform(-0.5, 1.0),
                            rng.uniform(-0.5, 1.0), rng.choice([1e10, -1e10, float('inf'), float('nan'), 6e8])])
        x = rng.choice([rng.uniform(-1.2, 1.2), rng.uniform(-1.2, 1.2), 0.0, -0.032, 0.032, 0.9, -0.9,
                        rng.choice([-1.0, 1.0]) * rng.uniform(0.85, 1.0)])
        yield dict(seed=rng.randrange(1 << 30), locked=rng.random() < 0.05, fixed=rng.random() < 0.1,
                   airborne=rng.choice([0, 1, 1]), flags=[int(rng.random() < 0.1) for _ in range(3)],
                   crash=rng.choice([0, 0, 0, 1]), steer=steer, x=x, prev=rng.choice([x, rng.uniform(-1.2, 1.2)]),
                   state=rng.choice([0, 1, 1, 2, 3, 4, 5, 5, 6]), crossover=rng.choice([0, 0, 1]),
                   front=rng.choice([rng.uniform(-6, 1), -2.5, -3.0, -1.0]),
                   rear=rng.choice([rng.uniform(-6, 1), -2.5, -1.0, -3.0]), contact=rng.choice([0, 1]),
                   slip=rng.choice([rng.uniform(0, 40), 14.0, 32.0, rng.uniform(10, 18)]), backing=rng.choice([0, 0, 1]),
                   controls={6: rng.random() < 0.3, 7: rng.random() < 0.3, 9: rng.random() < 0.3},
                   direction=rng.choice([-1, 0, 3, 7]), smooth=rng.choice([3.4028234663852886e38, rng.uniform(0.05, 1)]),
                   t=rng.choice([0.0, 0.016, rng.uniform(0, 0.5)]))


def setup(ctx, c):
    import random
    r = random.Random(c['seed'])
    bike = ctx.alloc(0x800)
    ctx.write(bike, struct.pack('<512f', *[r.uniform(-2, 2) for _ in range(512)]))
    vt = ctx.alloc(0x200)
    ctx.pack(bike, 'I', vt)
    for off, name, pop, ret in [(0x108, 'IsRiderPoseLocked', 0, int(c['locked'])),
                                (0x150, 'QueryControlInput', 8, lambda cx, e, a: int(c['controls'].get(a[0], 0))),
                                (0x160, 'ClassifyInputDirection', 0, c['direction'] & 0xffffffff),
                                (0xa4, 'EndMotion', 0, 0)]:
        ctx.pack(vt + off, 'I', ctx.stub(name, pop=pop, ret=ret))
    ctx.stub_at(0x0048e280, 'KrustyBike::LookBackAtNearestRival')
    ctx.stub_at(0x0048d910, 'KrustyBike::StartTrick', pop=4)
    ctx.pack(bike + 0x108, 'B', c['airborne'])
    ctx.pack(bike + 0x444, 'i', c['crash'])
    ctx.pack(bike + 0x47a, 'B', c['backing'])
    ctx.pack(bike + 0x504, 'f', c['x'])
    ctx.pack(bike + 0x510, 'f', c['prev'])
    ctx.pack(bike + 0x520, 'i', c['state'])
    ctx.pack(bike + 0x5b8, 'i', int(c['fixed']))
    front = ctx.alloc(0x280)
    rear = ctx.alloc(0x280)
    ctx.pack(front + 0x150, 'f', c['front'])
    ctx.pack(rear + 0x150, 'f', c['rear'])
    ctx.pack(rear + 0x260, 'i', c['contact'])
    ctx.pack(rear + 0x27c, 'f', c['slip'])
    ctx.pack(bike + 0x5f0, 'II', front, rear)
    axis = ctx.alloc(0x10)
    ctx.pack(axis, 'f', c['steer'])
    smoother = ctx.alloc(0x14)
    ctx.pack(smoother, '5f', r.uniform(-1, 1), r.uniform(0.05, 1), r.uniform(0, 1), r.uniform(0.1, 2), -r.uniform(0.1, 2))
    ctx.pack(bike + 0x63c, 'f', r.choice([1.0, r.uniform(0, 3)]))
    ctx.pack(bike + 0x640, 'II', axis, smoother)
    ctx.pack(bike + 0x6fc, 'i', c['crossover'])
    ctx.pack(bike + 0x704, 'f', c['smooth'])
    ctx.pack(bike + 0x734, '3B', *c['flags'])
    return dict(ecx=bike, args=[float(c['t'])], ret_eax=False)
