"""SimulateLookAhead 0x0040eca0, the AI's two-wheel look-ahead simulation (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp), cdecl with
22 arguments (steer, frontVel, rearVel, outVel, outSpeed, center, forward, side, maxTime, dt,
terrain, outAirborne, outImpact, probe, frontSusp, rearSusp, outFrontContact, outRearContact,
path, speedTable, sideOffset, nearOnly). Its callees run as retail code in both runs (the
repository bindings send the candidate's PathSteeringDirection 0x0040e510, PredictPathMotion
0x0040d200, SafeNormalize 0x0040d120, CrossProduct 0x00515600, FastSqrt / FastInvSqrt, the
dot product, the Vec3 constructor and __ftol to retail's addresses) except these stubs:
  - Terrain::QueryGround 0x00507c10 (thiscall, point, normal, 0, 0): a fixed height field
    h(x, z) = a sin(bx) + c cos(dz) + e; it writes the point's y and the normal, as the real
    one does (x and z untouched), and notes the point. No run uses a terrain without a
    grid (retail's QueryGround then writes nothing, and the first step reads unset normals).
  - the probe object (offsets from the retail body: +0x58 hasContact, +0x5c contact record
    with depth +0 and normal +8, +0x78 ignore list, +0x7c ignore count, +0xa4 hit point y):
    SetTransform 0x00435830 notes the translation row, QueryCollisions 0x00438e70 sets a
    random contact (record depth 0..1, a tilted normal).
  - QuerySegmentObstacles 0x00415640 (its own case covers it) notes the segment and its
    arguments, answers from the run's random stream and always writes outPush (zero x/z in
    some runs), outPoint and outDist, as the real one does.
The path (count +0, index +4, t +8, points +0xc, targetSpeed +0x96c, accelLimitScale +0x970,
lookAhead +0x974) runs ahead of the bike with jitter; g_TrackGame +0x2d74 event type (1 and 5
take the velocity-blended aim) and +0x2d80 tree collision. Steering input (no path) covers
z > 0 (rear drive with the speed table), z < 0 (braking) and z = 0. Optional pointers are
null in some runs; the probe is not (the body reads its ignore list unconditionally). maxTime
is positive: with no step, retail's final rear query reads an unset stack vector. dt >= 0.02
and maxTime <= 1.2 keep most runs under 60 steps; path runs move at 2 or more horizontally.
One run in the first 300 (index 130: path mode, maxTime 0.170, dt 0.033) steps past 200M
instructions in both retail and the candidate (QueryGround over 20k times); CLEAN_CASES
stays below it.

Only the 53-bit control word runs. At 0x007f (24-bit) retail and the candidate differ by an
ulp in most runs: VC6 reassociates the inlined three-term dot products (retail sums the
share-step dots x, y, z at 0x00411a44; the candidate's DotProduct expansion sums z, y, x and
a written-out sum z, x, y), which only rounds differently when every add rounds to 24 bits.
"""
import math
import struct

RETAIL_VA = 0x0040eca0
SYMBOL = '?SimulateLookAhead@@YAHPAUVec3@@000PAM000MMPAVTerrain@@PAE1PAVCollisionObject@@11PAH5PAUUnknownBikeAIPath@@1MH@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
TRACK_GAME = 0x0056e26c
FPCWS = (0x027f,)
PROBES = {0x0040f78a: 'terrain', 0x0040f7ac: 'no terrain', 0x0040f834: 'probe contact', 0x00410a43: 'obstacles',
          0x00410a53: 'obstacle hit', 0x00412c57: 'final ground'}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def cases(rng, n):
    for _ in range(n):
        heading = rng.uniform(-math.pi, math.pi)
        fwd = [math.sin(heading), rng.uniform(-0.2, 0.2), math.cos(heading)]
        norm = math.sqrt(sum(v * v for v in fwd))
        fwd = [v / norm for v in fwd]
        speed = rng.choice([0.0, rng.uniform(1, 15), rng.uniform(15, 60)])
        vel = [fwd[i] * speed + rng.uniform(-2, 2) for i in range(3)]
        center = [rng.uniform(-50, 50), rng.uniform(-1, 4), rng.uniform(-50, 50)]
        mode = rng.choice(['path', 'path', 'path', 'steer', 'none'])
        if mode == 'path' and math.hypot(vel[0], vel[2]) < 2.0:
            vel = [vel[i] + fwd[i] * 3.0 for i in range(3)]
        count = rng.choice([1, 2, 5, 12, 30])
        step = rng.uniform(3, 12)
        bend = rng.uniform(-0.15, 0.15)
        pts, h, p = [], heading, list(center)
        for _ in range(count):
            pts.append([p[0] + rng.uniform(-1, 1), p[1] + rng.uniform(-0.5, 0.5), p[2] + rng.uniform(-1, 1)])
            h += bend
            p = [p[0] + math.sin(h) * step, p[1], p[2] + math.cos(h) * step]
        yield dict(fwd=fwd, vel=vel, rvel=[v + rng.uniform(-1, 1) for v in vel], center=center, mode=mode, pts=pts,
                   index=rng.randrange(0, max(1, count - 1)), t=rng.uniform(0, 1),
                   target=rng.choice([40.0, 1000.0, rng.uniform(0, 80)]), scale=rng.choice([1.0, 1.5, rng.uniform(0, 3)]),
                   look=rng.choice([0.0, 10.0, rng.uniform(0, 40)]),
                   steer=[rng.uniform(-1, 1), rng.uniform(-1, 1), rng.choice([0.0, 1.0, -1.0, rng.uniform(-1, 1)])],
                   max_time=rng.choice([0.04, 0.3, 0.5, 1.0, rng.uniform(0.01, 1.2)]),
                   dt=rng.choice([0.02, 0.033, 0.05, 0.1, rng.uniform(0.02, 0.06)]),
                   terrain=rng.choice(['field', 'field', 'field', None]),
                   field=[rng.uniform(0, 3), rng.uniform(0.02, 0.3), rng.uniform(0, 3), rng.uniform(0.02, 0.3),
                          rng.uniform(-1, 1)],
                   nulls={k: rng.random() < 0.1 for k in ('side', 'speed', 'airborne', 'impact', 'fsusp', 'rsusp',
                                                         'fcontact', 'rcontact', 'table')},
                   contact=rng.choice([0.0, 0.2, 0.5]), hit=rng.choice([0.0, 0.3, 0.7]),
                   event=rng.choice([0, 1, 2, 5]), tree=rng.choice([0, 1]), side_offset=rng.choice([0.0, rng.uniform(-3, 3)]),
                   near=rng.choice([0, 1]), susp=(rng.uniform(0, 1), rng.uniform(0, 1)), seed=rng.randrange(1 << 30))


def setup(ctx, c):
    a, b, cc, d, e = c['field']

    def height(x, z):
        y = f32(a * math.sin(b * x) + cc * math.cos(d * z) + e)
        nx, nz = -a * b * math.cos(b * x), cc * d * math.sin(d * z)
        n = math.sqrt(nx * nx + 1 + nz * nz)
        return y, (nx / n, 1 / n, nz / n)

    def query_ground(cx, ecx, args):
        point, normal = args[0], args[1]
        x, _, z = cx.unpack(point, '3f')
        cx.note('QueryGround', cx.read(point, 12).hex(), normal != 0, args[2], args[3])
        if c['terrain'] != 'field' or not (math.isfinite(x) and math.isfinite(z)):
            return
        y, nrm = height(x, z)
        cx.pack(point + 4, 'f', y)
        if normal:
            cx.pack(normal, '3f', *nrm)
    terrain = ctx.alloc(0x100, fill=0x21)
    ctx.stub_at(0x00507c10, 'Terrain::QueryGround', pop=16, nargs=4, ret=query_ground, log=False)

    probe = ctx.alloc(0x100, fill=0x00)
    record = ctx.alloc(0x20)
    ignore = ctx.alloc(8)
    ctx.pack(ignore, 'II', 0x1234, 0x5678)
    ctx.pack(probe + 0x5c, 'I', record)
    ctx.pack(probe + 0x78, 'Ii', ignore, 2)

    def set_transform(cx, ecx, args):
        cx.note('SetTransform', ecx, cx.read(args[0], 64).hex())
    ctx.stub_at(0x00435830, 'CollisionObject::SetTransform', pop=4, ret=set_transform, log=False)

    def query_collisions(cx, ecx, args):
        hit = cx.rng.random() < c['contact']
        cx.pack(ecx + 0x58, 'i', int(hit))
        if hit:
            nx, nz = cx.rng.uniform(-0.5, 0.5), cx.rng.uniform(-0.5, 0.5)
            n = math.sqrt(nx * nx + 1 + nz * nz)
            cx.pack(record, '5f', cx.rng.uniform(0, 1), 0.0, nx / n, 1 / n, nz / n)
    ctx.stub_at(0x00438e70, 'CollisionObject::QueryCollisions', ret=query_collisions)

    def obstacles(cx, ecx, args):     # cdecl (seg, radius, skipVeg, ignoreCount, ignoreList, push, outPush,
        cx.note('QuerySegmentObstacles', cx.read(args[0], 24).hex(), *args[1:6], args[9])   # outPoint, outDist, near)
        hit = cx.rng.random() < c['hit']
        push = [cx.rng.uniform(-1, 1), 0.0, cx.rng.uniform(-1, 1)] if hit and cx.rng.random() < 0.8 else [0.0] * 3
        seg = struct.unpack('<6f', cx.read(args[0], 24))
        cx.pack(args[6], '3f', *push)
        cx.pack(args[7], '3f', *[cx.rng.uniform(-1, 1) * 0.5 for _ in range(3)])
        cx.pack(args[8], 'f', cx.rng.uniform(0, 20) if hit else 1e30)
        return int(hit)
    ctx.stub_at(0x00415640, 'QuerySegmentObstacles', nargs=10, ret=obstacles, log=False)

    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['event'])
    ctx.pack(game + 0x2d80, 'i', c['tree'])
    ctx.pack(TRACK_GAME, 'I', game)

    path = ctx.alloc(0x978, fill=0x00)
    ctx.pack(path, 'iif', len(c['pts']), c['index'], c['t'])
    for i, p in enumerate(c['pts']):
        ctx.pack(path + 0xc + 12 * i, '3f', *p)
    ctx.pack(path + 0x96c, 'fff', c['target'], c['scale'], c['look'])
    table = ctx.alloc(4 * 4096)
    ctx.pack(table, '4096f', *[f32(300.0 - 0.5 * i if i < 200 else 1.0) for i in range(4096)])

    def vec(v, fill=0x00):
        p = ctx.alloc(12, fill=fill)
        ctx.pack(p, '3f', *v)
        return p
    nulls = c['nulls']
    steer = vec(c['steer'])
    front_vel, rear_vel = vec(c['vel']), vec(c['rvel'])
    out_vel = ctx.alloc(12, fill=0x55)
    out_speed = 0 if nulls['speed'] else ctx.alloc(4, fill=0x55)
    center, forward = vec(c['center']), vec(c['fwd'])
    side = 0 if nulls['side'] else vec([0.0, 1.0, 0.0])
    airborne = 0 if nulls['airborne'] else ctx.alloc(4, fill=0x55)
    impact = 0 if nulls['impact'] else ctx.alloc(4)
    fsusp = 0 if nulls['fsusp'] else ctx.alloc(4)
    rsusp = 0 if nulls['rsusp'] else ctx.alloc(4)
    if fsusp:
        ctx.pack(fsusp, 'f', c['susp'][0])
    if rsusp:
        ctx.pack(rsusp, 'f', c['susp'][1])
    fcontact = 0 if nulls['fcontact'] else ctx.alloc(4, fill=0x55)
    rcontact = 0 if nulls['rcontact'] else ctx.alloc(4, fill=0x55)
    return dict(args=[steer if c['mode'] == 'steer' else 0, front_vel, rear_vel, out_vel, out_speed, center, forward,
                      side, float(c['max_time']), float(c['dt']), 0 if c['terrain'] is None else terrain, airborne,
                      impact, probe, fsusp, rsusp, fcontact, rcontact, path if c['mode'] == 'path' else 0,
                      0 if nulls['table'] else table, float(c['side_offset']), c['near']])
