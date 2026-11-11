"""CarProcedural::Tick 0x0042fd80 (tools/emu_diff.py case).

Stubs, each deterministic in its arguments: GameObject::Tick 0x004693d0, FindOffsetAtDistance
0x004308e0 and PositionAtTime 0x00430b10 (smooth functions of their float arguments),
WorldToLocalDirection 0x004fd710 / LocalToWorldDirection 0x004fd5c0 (the case's rotation),
GetPosition 0x004fc970, GetPositionIn 0x004fc9a0, QueryGround 0x00507c10, and the sinks
SetMeshShape 0x00432ab0, SetTransform 0x00435830, QueryCollisions 0x00438e70 (sets the
collider's hasContact from the case), UpdatePlacement 0x00435fb0, SetAxesPtr 0x004fbd70 and
SetPositionInFrame 0x004fc740. The sinks note the exact bits of the vectors and matrices they
are handed, so the steering clamp, the smoothing blends, the collider frame, the wheel axes
and the placement are compared, not only the object's stored fields. CrossProduct,
Vec3Normalize, FastSqrt, FastInvSqrt, OrientationAxesFromAngles (inlined in the candidate)
and the CRT fmod/atan2/sin/cos run as retail code.

Steering directions are drawn around the +-30 degree clamp as well as at random, and the
case covers the first update (sensor mesh, front wheel), trailing and non-trailing cars,
both smoothing modes, sensor and body contacts and non-positive frame times.
"""
import math
import struct

RETAIL_VA = 0x0042fd80
SYMBOL = '?Tick@CarProcedural@@UAEHM@Z'
SOURCE = 'src/bringup/Group2_CarProcedural.cpp'
PROBES = {0x004302a9: 'steering clamp', 0x004305f3: 'clamped low'}


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _fl(bits):
    return struct.unpack('<f', struct.pack('<I', bits & 0xffffffff))[0]


def _rotation(rng):
    a, b = rng.uniform(-math.pi, math.pi), rng.uniform(-0.3, 0.3)
    ca, sa, cb, sb = math.cos(a), math.sin(a), math.cos(b), math.sin(b)
    # yaw a then pitch b, row-major
    return [ca, 0, -sa, sa * sb, cb, ca * sb, sa * cb, -sb, ca * cb]


def cases(rng, n):
    for i in range(n):
        wheels = rng.choice([0, 0, 2, 4, 4, 6])
        steer_kind = rng.choice(['rand', 'edge', 'edge', 'big'])
        if steer_kind == 'edge':
            angle = rng.choice([1, -1]) * (math.pi / 6) + rng.choice([0.0, 1e-7, -1e-7, 1e-6, -1e-6, 1e-4])
        elif steer_kind == 'big':
            angle = rng.uniform(-math.pi, math.pi)
        else:
            angle = rng.uniform(-0.6, 0.6)
        yield dict(
            frame_time=rng.choice([_f32(rng.uniform(0.001, 0.1)), 0.0, -0.01, _f32(rng.uniform(0.001, 0.05))]),
            started=rng.randrange(4) != 0,
            wheels=wheels,
            wheel_z=[_f32(rng.uniform(-3, 3)) for _ in range(wheels)],
            steers=rng.randrange(3) != 0,
            skip=rng.randrange(3) == 0,
            trail=rng.choice([0.0, 0.0, _f32(rng.uniform(1, 40))]),
            path_time=_f32(rng.uniform(0, 60)),
            period=_f32(rng.uniform(5, 90)),
            inv_speed=_f32(rng.uniform(0.5, 2.0)),
            look_ahead=rng.choice([0.1, _f32(rng.uniform(0.01, 0.5))]),
            velocity_scale=rng.choice([10.0, _f32(rng.uniform(1, 20))]),
            back_blend=rng.choice([0.8, _f32(rng.uniform(0, 1))]),
            normal_blend=rng.choice([0.5, _f32(rng.uniform(0, 1))]),
            inv_wheel_radius=_f32(rng.uniform(0.5, 4)),
            spin=_f32(rng.uniform(-7, 7)),
            front_z=_f32(rng.uniform(0, 3)),
            half_height=rng.choice([5.0, _f32(rng.uniform(0.5, 8))]),
            steer_angle=angle,
            path=[rng.uniform(-200, 200) for _ in range(6)],
            rotation=_rotation(rng),
            position=[_f32(rng.uniform(-500, 500)) for _ in range(3)],
            ground=_f32(rng.uniform(-20, 20)),
            normal=[_f32(v) for v in (rng.uniform(-0.3, 0.3), rng.uniform(0.8, 1.0), rng.uniform(-0.3, 0.3))],
            sensor_contact=rng.randrange(3) == 0,
            sensor_offset=_f32(rng.uniform(-1, 2)),
            body_contact=rng.randrange(3) == 0,
            smoothed=[_f32(rng.uniform(-1, 1)) for _ in range(6)],
            offsets=[_f32(rng.uniform(-3, 3)) for _ in range(6)],
            contact=[_f32(rng.uniform(-2, 2)) for _ in range(9)],
            result=rng.randrange(3),
        )


def setup(ctx, case):
    c = case
    rot = c['rotation']
    path = c['path']

    def vec_at(t):
        # A smooth path whose direction at t is steered by the case's angle.
        return [_f32(path[0] + path[3] * math.sin(t * 0.37) + 7 * t * math.sin(c['steer_angle'])),
                _f32(path[1] + 0.5 * path[4] * math.cos(t * 0.21)),
                _f32(path[2] + 3 * t * math.cos(c['steer_angle']) + path[5] * 0.01 * t * t)]

    def rotate(v, inverse):
        if inverse:
            return [_f32(rot[0] * v[0] + rot[3] * v[1] + rot[6] * v[2]),
                    _f32(rot[1] * v[0] + rot[4] * v[1] + rot[7] * v[2]),
                    _f32(rot[2] * v[0] + rot[5] * v[1] + rot[8] * v[2])]
        return [_f32(rot[0] * v[0] + rot[1] * v[1] + rot[2] * v[2]),
                _f32(rot[3] * v[0] + rot[4] * v[1] + rot[5] * v[2]),
                _f32(rot[6] * v[0] + rot[7] * v[1] + rot[8] * v[2])]

    ctx.stub_at(0x004693d0, 'GameObject::Tick', pop=4, ret=c['result'])

    def find_offset(cx, ecx, args):
        t, d = _fl(args[0]), _fl(args[1])
        return 0, _f32(0.05 * d + 0.001 * math.sin(t))
    ctx.stub_at(0x004308e0, 'FindOffsetAtDistance', pop=8, ret=find_offset, ret_float='f')

    def position_at(cx, ecx, args):
        cx.pack(args[0], '3f', *vec_at(_fl(args[1])))
        return args[0]
    ctx.stub_at(0x00430b10, 'PositionAtTime', pop=8, ret=position_at)

    def direction(inverse):
        def f(cx, ecx, args):
            cx.note('direction', cx.read(args[1], 12))
            cx.pack(args[0], '3f', *rotate(cx.unpack(args[1], '3f'), inverse))
            return args[0]
        return f
    ctx.stub_at(0x004fd710, 'WorldToLocalDirection', pop=8, ret=direction(False))
    ctx.stub_at(0x004fd5c0, 'LocalToWorldDirection', pop=8, ret=direction(True))

    def get_position(cx, ecx, args):
        z = c['wheel_z'][cx.data.setdefault('wheel', 0) % max(1, c['wheels'])] if c['wheels'] else 0.0
        cx.data['wheel'] += 1
        cx.pack(args[0], '3f', 0.5, 0.25, z)
        return args[0]
    ctx.stub_at(0x004fc970, 'GetPosition', pop=4, ret=get_position)

    def get_position_in(cx, ecx, args):
        cx.pack(args[1], '3f', *c['position'])
    ctx.stub_at(0x004fc9a0, 'GetPositionIn', pop=8, ret=get_position_in)

    def query_ground(cx, ecx, args):
        x, y, z = cx.unpack(args[0], '3f')
        cx.pack(args[0], '3f', x, c['ground'], z)
        cx.pack(args[1], '3f', *c['normal'])
    ctx.stub_at(0x00507c10, 'QueryGround', pop=16, ret=query_ground)

    def mesh(cx, ecx, args):
        cx.note('mesh', cx.read(args[1], 24))
    ctx.stub_at(0x00432ab0, 'SetMeshShape', pop=8, ret=mesh)

    def transform(cx, ecx, args):
        cx.note('transform', cx.read(args[0], 64))
    ctx.stub_at(0x00435830, 'SetTransform', pop=4, ret=transform)

    def query(cx, ecx, args):
        hit = int(c['sensor_contact'] if ecx == sensor else c['body_contact'])
        cx.pack(ecx + 0x58, 'i', hit)
        return hit
    ctx.stub_at(0x00438e70, 'QueryCollisions', ret=query)
    ctx.stub_at(0x00435fb0, 'UpdatePlacement')

    def axes(cx, ecx, args):
        cx.note('axes', cx.read(args[0], 12), cx.read(args[1], 12))
    ctx.stub_at(0x004fbd70, 'SetAxesPtr', pop=16, ret=axes)

    def place(cx, ecx, args):
        cx.note('place', cx.read(args[1], 12))
    ctx.stub_at(0x004fc740, 'SetPositionInFrame', pop=8, ret=place)

    car = ctx.alloc(0x240, fill=0x3c)
    model = ctx.alloc(0x300, fill=0x11)
    sensor = ctx.alloc(0xc0, fill=0x22)
    body = ctx.alloc(0xc0, fill=0x33)
    terrain = ctx.alloc(0x40, fill=0x44)
    for collider in (sensor, body):
        record = ctx.alloc(0x30)
        ctx.pack(record, '9f', *c['contact'])
        if collider == sensor:
            ctx.pack(record, 'f', c['sensor_offset'])
        shape = ctx.alloc(0x110)
        ctx.pack(shape + 0xf8, '3f', 1.5, -2.25, 3.75)
        ctx.pack(collider + 0x54, 'IiI', shape, 0, record)
    wheels = [ctx.alloc(0x300, fill=0x55) for _ in range(c['wheels'])]
    wheel_list = ctx.alloc(4 * max(1, len(wheels)))
    if wheels:
        ctx.pack(wheel_list, '%dI' % len(wheels), *wheels)
    keys = ctx.alloc(16 * 3)
    ctx.pack(keys, '4f4f4f', 0.0, 0, 0, 0, c['period'] / 2, 0, 0, 0, c['period'], 0, 0, 0)

    ctx.pack(car + 0x34, '3I', model, sensor, body)
    ctx.pack(car + 0x50, 'iiii', int(c['steers']), int(c['started']), 0, int(c['skip']))
    ctx.pack(car + 0x60, 'I', c['wheels'])
    ctx.pack(car + 0x6c, 'I', 3)
    ctx.pack(car + 0x174, 'f', c['inv_speed'])
    ctx.pack(car + 0x184, '5f', c['inv_wheel_radius'], c['half_height'], c['spin'], 0.125, c['path_time'])
    ctx.pack(car + 0x198, '6f', c['front_z'], c['trail'], 0.0, c['look_ahead'], c['velocity_scale'],
             c['back_blend'])
    ctx.pack(car + 0x1b0, 'fI', c['normal_blend'], keys)
    ctx.pack(car + 0x1d0, '6f', *c['offsets'])
    ctx.pack(car + 0x1f4, '6f', *c['smoothed'])
    ctx.pack(car + 0x20c, '2I', terrain, wheel_list)
    return dict(ecx=car, args=[c['frame_time']])
