"""SimulateLookAhead 0x0040eca0, the AI bike's two-wheel look-ahead physics (tools/emu_diff.py
case, call-record/replay mode).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp). Pure math
helpers run as retail code (SafeNormalize, the Vec3 constructor, the dot call, CrossProduct,
FastSqrt, FastInvSqrt, Vec3Magnitude, __ftol), so the values flowing through the body are
real. Every other call is recorded and replayed (``REPLAY``):

- Terrain::QueryGround keeps the x and z the body passes (``inout``) and writes a smooth
  height field's y and normal, so the ground follows the wheels;
- CollisionObject::SetTransform logs the matrix; QueryCollisions sets the probe's contact flag
  (35%) and a contact record (depth, normal);
- PathSteeringDirection and PredictPathMotion write unit aim directions (PredictPathMotion
  fails 4% of the time, which returns 0);
- QuerySegmentObstacles logs both segment ends and returns a hit 50% of the time with a
  horizontal push, an avoid point near the front wheel and a distance.

Inputs follow the one caller (KrustyBike::Fn_00413200 0x00413a0c: maxTime = dt, side =
bodyUp, a path and the class's speed-table row) with variations the body tests: no path
(steering input, the three steer signs), no side vector, no suspension pointers, no terrain,
maxTime spanning a few steps, the event type (1 and 5 blend the velocity into the aim) and the
tree-collision switch.
"""
import math
import struct

RETAIL_VA = 0x0040eca0
SYMBOL = ('?SimulateLookAhead@@YAHPAUVec3@@000PAM000MMPAVTerrain@@PAE1PAVCollisionObject@@11PAH5'
          'PAUUnknownBikeAIPath@@1MH@Z')
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
TRACK_GAME = 0x0056e26c
SPEED_TABLE = 0x00566f74           # g_BikeAISpeedTable_00566f74[3][200]
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)
QUERY_GROUND = 0x00507c10
SET_TRANSFORM = 0x00435830
QUERY_COLLISIONS = 0x00438e70
PATH_STEERING = 0x0040e510
PREDICT_PATH = 0x0040d200
QUERY_OBSTACLES = 0x00415640
PROBES = {0x0040f834: 'probe contact (front)', 0x00410a53: 'obstacle hit',
          0x00410d71: 'event 1/5 aim blend', 0x004112e4: 'path brake stops the wheels',
          0x004115ca: 'steer sign > 0 (rear drive)', 0x00411696: 'steer sign < 0 (brakes)',
          0x0040f2d5: 'PredictPathMotion failed (returns 0)'}
# The linked body groups several inline dot products and Vec3 temporaries in another order than
# retail (VC6's per-function inline budget, see the source notes), so values drift by float32
# ulps, more at 24-bit precision over several steps; logic differences move them far more.
FLOAT_ULPS = 256
FLOAT_ABS = 1e-5


def height(x, z):
    return 2.0 * math.sin(0.13 * x) + 1.5 * math.cos(0.11 * z) + 0.05 * x


def ground_normal(x, z):
    dx = 0.26 * math.cos(0.13 * x) + 0.05
    dz = -0.165 * math.sin(0.11 * z)
    n = (-dx, 1.0, -dz)
    k = 1.0 / math.sqrt(sum(c * c for c in n))
    return [c * k for c in n]


def _query_ground(ctx, rec):
    pos, normal = rec.args[0], rec.args[1]
    x, _, z = ctx.unpack(pos, '3f')
    if not (abs(x) < 1e6 and abs(z) < 1e6):
        return
    ctx.pack(pos + 4, 'f', height(x, z))
    if normal:
        ctx.pack(normal, '3f', *ground_normal(x, z))


def _unit(rng, flat=False):
    v = [rng.uniform(-1, 1), 0.0 if flat else rng.uniform(-0.3, 0.3), rng.uniform(-1, 1)]
    k = 1.0 / math.sqrt(sum(c * c for c in v) or 1.0)
    return [c * k for c in v]


def _aim(ctx, rec):
    ctx.pack(rec.args[0], '3f', *_unit(rec.rng, flat=True))


def _path_steering(ctx, rec):
    return 0 if rec.rng.random() < 0.1 else 1


def _predict(ctx, rec):
    return 0 if rec.rng.random() < 0.04 else 1


def _predict_out(ctx, rec):
    # (Vec3 pos, Vec3 vel, path, Vec3* outDir, maxTime, dt, terrain, side, float*, Vec3*)
    ctx.pack(rec.args[7], '3f', *_unit(rec.rng, flat=True))


def _collisions(ctx, rec):
    probe = rec.ecx
    hit = rec.rng.random() < 0.35
    ctx.pack(probe + 0x58, 'i', int(hit))
    record = ctx.alloc(0x20)
    ctx.pack(record, '5f', rec.rng.uniform(0, 1), 0.0, *_unit(rec.rng))
    ctx.pack(probe + 0x5c, 'I', record)
    return int(hit)


def _obstacles(ctx, rec):
    return 1 if rec.rng.random() < 0.5 else 0


def _obstacles_out(ctx, rec):
    # (Vec3* seg, radius, skipVeg, ignoreCount, ignoreList, push, Vec3* outPush, Vec3* outPoint,
    #  float* outDist, nearOnly)
    a, r = rec.args, rec.rng
    seg = ctx.unpack(a[0], '3f')
    push = [r.uniform(-1, 1), r.uniform(-0.2, 0.2), r.uniform(-1, 1)]
    if r.random() < 0.1:
        push = [0.0, r.uniform(-1, 1), 0.0]
    ctx.pack(a[6], '3f', *push)
    ctx.pack(a[7], '3f', *[seg[i] + r.uniform(-3, 3) for i in range(3)])
    ctx.pack(a[8], 'f', r.choice([1000.0, r.uniform(0, 10)]))


REPLAY = dict(
    passthrough=(0x00404e60, 0x0040d120, 0x0040ae30, 0x00515600, 0x00460b50, 0x00460c00, 0x00413190,
                 0x00413160, 0x00421cb0, 0x00421d00, 0x00428090, 0x005015b0),
    sizes={'Matrix4': 64, 'TerrainVec3': 12},
    inout={QUERY_GROUND: (0,), QUERY_OBSTACLES: ((0, 24),)},
    returns={PATH_STEERING: _path_steering, PREDICT_PATH: _predict, QUERY_COLLISIONS: _collisions,
             QUERY_OBSTACLES: _obstacles},
    effects={QUERY_GROUND: _query_ground, PATH_STEERING: _aim, PREDICT_PATH: _predict_out,
             QUERY_OBSTACLES: _obstacles_out},
)


def cases(rng, n):
    for _ in range(n):
        dt = rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0.005, 0.08)])
        yield dict(
            seed=rng.getrandbits(32),
            dt=dt,
            steps=rng.choice([1, 1, 1, 2, 3, 0.5, 2.5]),
            path=rng.random() < 0.6,
            steer=rng.random() < 0.9,
            steer_z=rng.choice([0.0, 1.0, -1.0, rng.uniform(-1, 1)]),
            side=rng.random() < 0.85,
            front_susp=rng.random() < 0.9,
            rear_susp=rng.random() < 0.9,
            terrain=rng.random() < 0.9,
            impact=rng.random() < 0.9,
            outs=rng.random() < 0.8,
            speed_table=rng.random() < 0.85,
            table_row=rng.randrange(3),
            event=rng.choice([0, 0, 1, 2, 3, 5]),
            trees=rng.choice([0, 1]),
            lift=rng.choice([rng.uniform(-0.6, 0.6), rng.uniform(0.4, 3.0), rng.uniform(-0.2, 0.2)]),
            speed=rng.choice([rng.uniform(0, 5), rng.uniform(5, 60), 0.0]),
            target_speed=rng.choice([rng.uniform(0, 80), 1000.0, 40.0, -rng.uniform(0.1, 5)]),
            near_only=rng.choice([0, 1]),
        )


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])
    for init in FAST_TABLE_INITS:
        ctx.call(init)

    def vec(values):
        at = ctx.alloc(12)
        ctx.pack(at, '3f', *values)
        return at

    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['event'])
    ctx.pack(game + 0x2d80, 'i', c['trees'])
    ctx.pack(TRACK_GAME, 'I', game)

    fwd = _unit(rng)
    cx, cz = rng.uniform(-200, 200), rng.uniform(-200, 200)
    center = [cx, height(cx, cz) + c['lift'], cz]
    # side is the body's up vector: up, tilted a little
    up = [rng.uniform(-0.3, 0.3), 1.0, rng.uniform(-0.3, 0.3)]
    k = 1.0 / math.sqrt(sum(x * x for x in up))
    up = [x * k for x in up]
    vel = [f * c['speed'] + rng.uniform(-2, 2) for f in fwd]
    front_vel = [v + rng.uniform(-1, 1) for v in vel]
    rear_vel = [v + rng.uniform(-1, 1) for v in vel]

    path = 0
    if c['path']:
        path = ctx.alloc(0x978)
        ctx.pack(path, 'iif', 4, 0, 0.0)
        ctx.pack(path + 0x96c, 'fff', c['target_speed'], rng.choice([1.5, rng.uniform(0.5, 2)]), 20.0)
    steer = vec([rng.uniform(-1, 1), rng.uniform(-1, 1), c['steer_z']]) if c['steer'] else 0
    probe = ctx.alloc(0x100)
    ctx.pack(probe, '64f', *[rng.uniform(-2, 2) for _ in range(64)])
    ctx.pack(probe + 0x58, 'iI', 0, 0)
    ignore = ctx.alloc(16)
    ctx.pack(probe + 0x78, 'iI', rng.randrange(3), ignore)
    terrain = ctx.alloc(0x40) if c['terrain'] else 0
    impact = ctx.alloc(4) if c['impact'] else 0
    if impact:
        ctx.pack(impact, 'f', rng.choice([0.0, rng.uniform(0, 20)]))
    susp_f = ctx.alloc(4) if c['front_susp'] else 0
    susp_r = ctx.alloc(4) if c['rear_susp'] else 0
    for at in (susp_f, susp_r):
        if at:
            ctx.pack(at, 'f', rng.choice([0.0, 1.0, rng.uniform(0, 1)]))
    outs = [ctx.alloc(4) if c['outs'] else 0 for _ in range(4)]   # outSpeed, airborne, contacts
    table = SPEED_TABLE + 800 * c['table_row'] if c['speed_table'] else 0
    dt = c['dt']
    args = [steer, vec(front_vel), vec(rear_vel), vec([0.0, 0.0, 0.0]), outs[0], vec(center), vec(fwd),
            vec(up) if c['side'] else 0, float(dt * c['steps']), float(dt), terrain, outs[1], impact,
            probe, susp_f, susp_r, outs[2], outs[3], path, table, rng.uniform(-3, 3), c['near_only']]
    return dict(args=args)
