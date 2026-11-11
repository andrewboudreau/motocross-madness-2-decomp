"""Bike::LoadBike 0x004079c0, the bike loader (tools/emu_diff.py case, call-record/replay mode).

The linked body for 0x004079c0 is KbBikeLoader::Fn_004079C0 (src/bringup/Group1_KbBikeLoader.cpp),
a forwarder that hands its 40 argument dwords unchanged to Bike::LoadBike in
src/krusty2/vehicle/Bike.cpp (identity checked by running the forwarder on marker arguments), so
this case compares that body. Every callee is replayed: LoadVehicle 0x00525e20, the rider
character (D3DIMSoultreeCharacter ctor 0x004455b0 and its load slot), FindByName/FindMotion
(synthetic nodes and motions, sometimes null), the shocks, wheels, tire, steering control,
particle emitters, collision points and the bike's own virtual slots (retail's Bike vtables,
with GameObject reached through the vbtable). operator new is modelled (fresh 0xcd blocks,
size and line noted). The arguments vary the weight (zero, negative, positive), the default
engine flag (the caller's six suspension values or the defaults), the gyro options, rider
names (empty, 15 and 16+ characters), null and present desc/info/terrain pointers, the
collision flag and the Vec3 frames. Synthetic callee floats (wheel positions, node bounds)
drive the float math (wheel base, shock geometry, inertia terms). fs:[0] is page 0.
"""
import random
import struct

RETAIL_VA = 0x004079c0
SYMBOL = ('?LoadBike@Bike@@QAEPAVGameObject@@HPBD00PBUSoultreeLoadDesc@@PBUBikeA5C8@@UVec3@@33PAXPAU'
          'VehicleInputMap@@0HHPAHHHHMMMMMMHH44PAUVehicleAxis@@77PAVSoultreeSlot1f0@@MH@Z')
SOURCE = 'src/krusty2/vehicle/Bike.cpp'
# Math3D.h's axis constants are per-unit statics (bound to many VAs repo-wide); these are Bike.cpp's
# (src/krusty2/vehicle/Bike.bindings.json). setup gives them their initialized values.
AXES = {'_kVec3XAxis': (0x005778b8, (1.0, 0.0, 0.0)), '_kVec3YAxis': (0x005778c8, (0.0, 1.0, 0.0)),
        '_kVec3ZAxis': (0x00577898, (0.0, 0.0, 1.0)), '_kVec3Zero': (0x005778a8, (0.0, 0.0, 0.0))}
BINDINGS = {name: va for name, (va, _) in AXES.items()}
PROBES = {0x004079f5: 'default weight', 0x00407c34: 'rider', 0x00408e49: 'inertia', 0x00408fac: 'orientation'}

NEW, STRNCPY, STRICMP = 0x004a3010, 0x00534b10, 0x00536070
BIKE_VT, CHAR_VT, GAMEOBJ_VT = 0x00550900, 0x005508cc, 0x0055085c


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    size = rec.args[0]
    if size <= 0 or size > 0x100000:
        size = 16
    at = ctx.alloc(size + 0x40, fill=0xcd)
    return at


def _stricmp(ctx, rec):
    def s(a):
        b = ctx.read(a, 128)
        return (b[:b.index(0)] if 0 in b else b).lower()
    a, b = s(rec.args[0]), s(rec.args[1])
    return 0 if a == b else (0xffffffff if a < b else 1)


def _ctor(vptrs, vbase=None):
    # The constructed object gets its retail vtables (and vbtable) so later virtual calls on it
    # resolve to retail's slots in both runs.
    def ctor(ctx, rec):
        for offset, vt in vptrs:
            ctx.pack(rec.ecx + offset, 'I', vt)
        if vbase is not None:
            ctx.pack(rec.ecx + 4, 'I', ctx.alloc(8, struct.pack('<ii', -4, vbase - 4)))
        return rec.ecx
    return ctor


def _add_wheel(ctx, rec):
    # Vehicle::AddWheel 0x00525d30 (modelled: the wheel loops below read wheelList/wheelCount).
    this, wheel = rec.ecx, rec.args[0]
    count, capacity = ctx.unpack(this + 0x544, 'i')[0], ctx.unpack(this + 0x540, 'i')[0]
    if count >= capacity:
        return 0
    ctx.pack(ctx.unpack(this + 0x53c, 'I')[0] + 4 * count, 'I', wheel)
    ctx.pack(this + 0x544, 'i', count + 1)
    return 1


CTORS = {0x00525d30: _add_wheel,
         0x004455b0: _ctor([(0, 0x00551530), (0x214, 0x005514c0)], 0x214),      # D3DIMSoultreeCharacter
         0x00512f10: _ctor([(0, 0x00558578), (12, 0x00558508), (184, 0x005584fc)]),   # Tire
         0x004b8a00: _ctor([(0, 0x00555ab8)]), 0x004b8df0: _ctor([(0, 0x00555b34)]),   # dust, dirt chunks
         0x004b9f40: _ctor([(0, 0x00555c9c)]),                                         # steam
         0x004fa700: _ctor([]), 0x004f9ee0: _ctor([]), 0x004a23a0: _ctor([])}          # shocks, moving part


REPLAY = dict(returns={NEW: _new, STRICMP: _stricmp, **CTORS}, pops={NEW: 0, STRICMP: 0}, args={NEW: 3, STRICMP: 2},
              ecx={NEW: False, STRICMP: False}, unlogged=(NEW,), passthrough=(STRNCPY,))


def _floats(r, n, lo=-10.0, hi=10.0):
    return struct.pack('<%df' % n, *[r.choice([0.0, 1.0, r.uniform(lo, hi)]) for _ in range(n)])


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   weight=rng.choice([0.0, -5.0, 230.0, 180.5, rng.uniform(50, 400)]),
                   default_engine=rng.choice([0, 1]),
                   gyros=(rng.choice([0, 1, 2]), rng.choice([0, 1, 2])),
                   name=rng.choice(['', 'Rider', 'x' * 15, 'y' * 16, 'z' * 40]),
                   desc=rng.random() < 0.8, info=rng.random() < 0.7, terrain=rng.random() < 0.7,
                   collision=rng.choice([0, 1]), null_collision=rng.random() < 0.3,
                   suspension=[rng.choice([0.0, 0.5, 1.0, -1.0, rng.uniform(-2, 2)]) for _ in range(6)],
                   rpm=[rng.choice([0, 1000, -1, rng.randrange(20000)]) for _ in range(3)])


POINTER_FIELDS = (0x128, 0x58c, 0x598)
NULLABLE = (0x128,)


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    for va, value in AXES.values():
        ctx.pack(va, '3f', *value)
    size = 0x1800
    this = ctx.alloc(size, _floats(r, size // 4, -5, 5))
    vbtable = ctx.alloc(8, struct.pack('<ii', -4, 0x734))
    ctx.pack(this, 'II', BIKE_VT, vbtable)
    ctx.pack(this + 0x21c, 'I', CHAR_VT)
    ctx.pack(this + 0x738, 'I', GAMEOBJ_VT)
    ctx.pack(this + 0x53c, 'IIi', ctx.alloc(0x20, fill=0), 8, 0)    # wheelList, capacity, count
    # Pointer fields LoadVehicle (replayed) would have set and the body dereferences.
    for offset in POINTER_FIELDS:
        ctx.pack(this + offset, 'I', 0 if (offset in NULLABLE and c['null_collision']) else
                 ctx.alloc(0x400, _floats(r, 0x100, -3, 3)))

    def block(n=0x400):
        return ctx.alloc(n, _floats(r, n // 4, -3, 3))

    def text(s):
        return ctx.alloc(len(s) + 16, s.encode() + b'\0')

    vec = lambda: [struct.unpack('<I', struct.pack('<f', r.uniform(-50, 50)))[0] for _ in range(3)]
    fl = lambda v: struct.unpack('<I', struct.pack('<f', v))[0]
    torque = ctx.alloc(0x100, struct.pack('<64i', *[r.randrange(-10, 400) for _ in range(64)]))
    args = [r.randrange(1, 100),                       # context
            text('Engine1'), text('bikes\\bike.mdl'), text('riders\\rider.mdl'),
            block() if c['desc'] else 0, block(0x200) if c['info'] else 0]
    args += vec() + vec() + vec()
    args += [block() if c['terrain'] else 0, block(), text(c['name']), r.randrange(0, 3), c['default_engine'],
             torque] + [v & 0xffffffff for v in c['rpm']]
    args += [fl(v) for v in c['suspension']]
    args += [c['gyros'][0], c['gyros'][1], block(), block(), block(), block(), block(), block(),
             fl(c['weight']), c['collision']]
    assert len(args) == 40
    return dict(ecx=this, args=args, ret_eax=True)
