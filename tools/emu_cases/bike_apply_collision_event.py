"""Bike::ApplyCollisionEvent 0x00405190 (Bike slot 38; tools/emu_diff.py case).

The bike's collision response: per event kind it gathers the other party's mass, inertia,
velocity and contact arm, then hands them to the two impulse solvers (Bike slots 3 and 4),
updates the other party, and reports to UpdateCrashState (slot 100). Kind 100 also scores
the hit angle with atan2 and may link the two riders; kinds 100, 0..1, 0x66 and 0x6a fire
the spark emitter.

Offsets read from the retail body (tier 1): this +0xc position, +0x50 yaw, +0x64 velocity,
+0xb8/+0xbc speeds, +0xcc world angular velocity, +0x124 particle manager (enabled bit
+0x25), +0x128 contact set (hit point +0xa0, normal +0xac), +0x1e4 step time, +0x5ac spark
emitter (+0x60 burst flag, +0x78 launch velocity), +0x608 linked vehicle. Event +0x60 is the
other party. Vehicle party: +0x8 scene node, +0x3bc node word, +0x444 crash state.
Kind 0x3ea party: +0x5c kind info (+0x24 fraction), +0xc4 body (slot 43, +0x234 node).
Kind 0x69 object: +0x34, +0x40; kind 0x2711 object: +0x1a0, +0x224.

Every virtual and the three helpers (SetPosition 0x004b9dc0, LocalToWorldPoint 0x004fd660,
LocalToWorldDirection 0x004fd5c0) are stubs. Each stub logs the values behind its pointer
arguments, not the pointers, since the two builds lay out the frame differently. The
solvers write an impulse derived from those values, so a wrong input also changes the
impulse later passed to UpdateCrashState. Objects are filled with random floats, then the
pointer and flag fields are set.

Kinds 0x65 and 0x3e8 pass the per-axis scale uninitialised in retail (the source keeps that
as provisional); those values are logged separately as 'uninit scale' so a difference there
is visible as such.
"""
import struct

RETAIL_VA = 0x00405190
SYMBOL = '?ApplyCollisionEvent@Bike@@UAEXHHPAX@Z'
SOURCE = 'src/krusty2/vehicle/Bike.cpp'
FPCWS = (0x027f, 0x007f, 0x037f)

KINDS = (100, 0x68, 0x65, 0, 1, 0x66, 0x6a, 0x69, 0x3e8, 0x3e9, 0x3ea, 0x2711, 2, 0x67, 0x3eb, 5000)
PROBES = {0x004052e9: 'kind 100 atan2', 0x004052aa: 'link other', 0x004052c5: 'link this',
          0x00405392: 'kind 100 sparks', 0x004055b9: 'kind 0x3e8 static', 0x0040579b: 'body solver',
          0x00405a09: 'kind 0x3ea point', 0x00405a9a: 'other direction', 0x00405b90: 'static solver',
          0x00405bb9: 'rest clamp', 0x00405c37: 'other crash'}

SLOT_STATIC, SLOT_BODY, SLOT_WORLD_ANG, SLOT_CAN_CRASH, SLOT_CRASH = 3, 4, 43, 52, 100


def fvec(rng, scale):
    return [rng.uniform(-scale, scale) for _ in range(3)]


def cases(rng, n):
    for k in range(n):
        kind = KINDS[k % len(KINDS)] if k < 4 * len(KINDS) else rng.choice(KINDS)
        pos = fvec(rng, 50)
        yield dict(
            kind=kind, notify=rng.choice([0, 1, 1]), seed=rng.getrandbits(32),
            pos=pos, other_pos=[p + rng.choice([0.0, 2.0, -2.0, rng.uniform(-3, 3)]) * (i == 1) +
                                (rng.uniform(-5, 5) if i != 1 else 0.0) for i, p in enumerate(pos)],
            yaw=rng.uniform(-6.3, 6.3), particles=rng.choice([0, 1, 2]), crash=rng.choice([0, 1]),
            can_crash=rng.choice([0, 1]),
            speeds=[rng.choice([0.0, 0.0005, 0.05, rng.uniform(-1, 1), rng.uniform(0, 30)]) for _ in range(2)],
            fraction=rng.choice([0.0, 1.0, rng.uniform(0, 1)]), world_ang=fvec(rng, 5))


def _bits(ctx, addr):
    return ctx.unpack(addr, '3I') if addr else None


def _impulse_from(values):
    h = 0x811c9dc5
    for v in values:
        for w in (v or (0,)):
            h = ((h ^ w) * 0x01000193) & 0xffffffff
    return (h % 20000) / 100.0 - 100.0


def make_vtable(ctx, label, case):
    def static_solver(c, ecx, a):
        normal, vel, arm, scale, kind, zero, out = a
        vals = [_bits(c, normal), _bits(c, vel), _bits(c, arm)]
        c.note('static solver inputs', ecx, tuple(vals), kind, zero)
        c.note('uninit scale' if kind in (0x3e8, 0x65) else 'scale', _bits(c, scale))
        c.pack(out, 'f', _impulse_from(vals + [(kind,)]))
        return 0

    def body_solver(c, ecx, a):
        vals = [_bits(c, a[i]) for i in (0, 1, 2, 3)]
        other = [_bits(c, a[i]) for i in (7, 8, 9, 10, 11)]
        c.note('body solver inputs', ecx, tuple(vals), a[4], a[5], a[6], tuple(other), a[12], a[14])
        c.pack(a[13], 'f', _impulse_from(vals + other + [(a[4], a[5], a[14])]))
        return 0

    def world_ang(c, ecx, a):
        c.pack(a[0], '3f', *case['world_ang'])
        return a[0]

    entries = []
    for k in range(101):
        if k == SLOT_STATIC:
            entries.append(ctx.stub('%s slot 3 static solver' % label, pop=28, ret=static_solver, log=False))
        elif k == SLOT_BODY:
            entries.append(ctx.stub('%s slot 4 body solver' % label, pop=60, ret=body_solver, log=False))
        elif k == SLOT_WORLD_ANG:
            entries.append(ctx.stub('%s slot 43 world angular velocity' % label, pop=4, ret=world_ang, log=False))
        elif k == SLOT_CAN_CRASH:
            entries.append(ctx.stub('%s slot 52 CanCrash' % label, ret=case['can_crash']))
        elif k == SLOT_CRASH:
            entries.append(ctx.stub('%s slot 100 UpdateCrashState' % label, pop=12))
        else:
            entries.append(ctx.stub('%s unexpected slot %d' % (label, k)))
    return ctx.vtable(entries)


def float_block(ctx, size, rng, scale=20.0):
    return ctx.alloc(size, struct.pack('<%df' % (size // 4), *(rng.uniform(-scale, scale) for _ in range(size // 4))))


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])

    def node_transform(name):
        def ret(cx, ecx, a):
            out, src = a
            v = cx.unpack(src, '3f')
            cx.note(name, ecx, cx.unpack(src, '3I'))
            k = (ecx >> 4) % 7
            cx.pack(out, '3f', v[1] * 0.5 + k, v[2] - k, v[0] * 2.0 + 0.25)
            return out
        return ret

    ctx.stub_at(0x004b9dc0, 'SparkParticleEmitter::SetPosition', pop=12)
    ctx.stub_at(0x004fd660, 'SoultreeObject::LocalToWorldPoint', pop=8, ret=node_transform('point'), log=False)
    ctx.stub_at(0x004fd5c0, 'SoultreeObject::LocalToWorldDirection', pop=8, ret=node_transform('direction'),
                log=False)

    bike = float_block(ctx, 0x700, rng)
    ctx.pack(bike, 'I', make_vtable(ctx, 'bike', c))
    ctx.pack(bike + 0xc, '3f', *c['pos'])
    ctx.pack(bike + 0x50, 'f', c['yaw'])
    ctx.pack(bike + 0xb8, '2f', *c['speeds'])
    if c['particles']:
        pm = ctx.alloc(0x40)
        ctx.pack(pm + 0x25, 'B', 1 if c['particles'] == 2 else 0)
        ctx.pack(bike + 0x124, 'I', pm)
    else:
        ctx.pack(bike + 0x124, 'I', 0)
    contacts = float_block(ctx, 0xc0, rng, 50.0)
    ctx.pack(bike + 0x128, 'I', contacts)
    ctx.pack(bike + 0x5ac, 'I', float_block(ctx, 0x100, rng))
    ctx.pack(bike + 0x608, 'I', 0)

    kind = c['kind']
    event = float_block(ctx, 0x80, rng)
    if kind in (100, 0x68):
        other = float_block(ctx, 0x700, rng)
        ctx.pack(other, 'I', make_vtable(ctx, 'other', c))
        ctx.pack(other + 0xc, '3f', *c['other_pos'])
        ctx.pack(other + 0x8, 'I', ctx.alloc(0x40))
        ctx.pack(other + 0x444, 'I', c['crash'])
        ctx.pack(other + 0x608, 'I', 0)
        party = other
    elif kind == 0x3ea:
        party = float_block(ctx, 0xd0, rng)
        info = float_block(ctx, 0x40, rng)
        ctx.pack(info + 0x24, 'f', c['fraction'])
        body = float_block(ctx, 0x240, rng)
        ctx.pack(body, 'I', make_vtable(ctx, 'party body', c))
        ctx.pack(body + 0x234, 'I', ctx.alloc(0x40))
        ctx.pack(party + 0x5c, 'I', info)
        ctx.pack(party + 0xc4, 'I', body)
    else:
        party = float_block(ctx, 0x240, rng)
    ctx.pack(event + 0x60, 'I', party)
    return dict(ecx=bike, args=[c['notify'], kind, event], ret_float=False, ret_eax=False)
