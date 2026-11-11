"""Terrain::PrepareGeometry, GameObject slot 12 of Terrain (tools/emu_diff.py case).

The body's only float-to-int conversion is the camera focal length (camera +0x198) into
Terrain +0x70, which retail does with its inline FLD/FISTP helper under the current rounding
mode. Every callee (MemTagStack, AgeManager, the PeakHold record, ReadClock, ComputeRatios
and the DrawableGridNode passes) is stubbed at its retail address, so both bodies drive the
same stubs and the call log compares their order, `this` and arguments. ReadClock returns a
pseudo-random increasing clock, TotalSize reports random total/stale sizes on both sides of
the 1 MiB eviction limit, and the focal length is drawn from pixel-sized values, .5
fractions and their float neighbours, plus (every EXTREME_EVERY-th case) out-of-int-range
values and NaN, which retail's `fistp dword` turns into 0x80000000. Only those differ: the
unit keeps the plain FistpFloatToInt (low dword of a 64-bit fistp), because a focal length in
pixels stays inside int and the checked helper lowers the masked score (0.924 to 0.882). Set
EXTREME_EVERY = 0 to compare in-range values only (tests/test_emu_diff.py does).
"""
import math
import struct

RETAIL_VA = 0x00507610
SYMBOL = '?PrepareGeometry@Terrain@@UAEHXZ'
SOURCE = 'src/bringup/Group3_TerrainPrepareGeometry.cpp'

COMPUTE_RATIOS = 0x00507bb0
EXTREME_EVERY = 25
# The unit's file-local statistics stand in for these retail globals (see the source).
BINDINGS = {
    '?ComputeRatios@Terrain@@QAEXXZ': COMPUTE_RATIOS,
    '_s_gridCounter66529c': 0x0066529c,
    '_s_gridCounter68a088': 0x0068a088,
    '_s_gridTextureCountPeak': 0x0068a2f0,
    '_s_gridTextureEstimatePeak': 0x0068a2f4,
}
PROBES = {0x00507836: 'focal length fistp'}

TOGGLE_718 = 0x00574718
TOGGLE_314 = 0x0068a314
MEM_TAG_STACK = 0x0056df04
TEXTURE_COUNT = 0x0068a2ec
TEXTURE_SIZES = 0x0065b65c
SHARED_STATE = 0x0068a090

# (name, retail VA, callee-popped bytes)
CALLEES = [
    ('MemTagStack::Push', 0x004a2d00, 4),
    ('MemTagStack::Pop', 0x004a2d90, 4),
    ('ReadClock', 0x004bfa80, 0),
    ('AgeManager::EvictStale', 0x004011b0, 4),
    ('AgeManager::AdvanceAge', 0x00401040, 0),
    ('PeakHold::Record', 0x004cb6b0, 4),
    ('Terrain::ComputeRatios', COMPUTE_RATIOS, 0),
    ('DrawableGridNode::TestVisibility', 0x00481db0, 8),
    ('DrawableGridNode::PropagateFullMask', 0x004826d0, 0),
    ('DrawableGridNode::RetestChangedVertices', 0x00482760, 0),
    ('DrawableGridNode::TestDroppableVertices', 0x00481580, 0),
    ('DrawableGridNode::CountEdgeDifferences', 0x004813e0, 0),
    ('DrawableGridNode::WalkVisibleFromRoot', 0x0047ef70, 0),
    ('DrawableGridNode::WalkSlot4FromRoot', 0x00481170, 0),
]


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _neighbours(x):
    bits = struct.unpack('<I', struct.pack('<f', x))[0]
    return [struct.unpack('<f', struct.pack('<I', b))[0] for b in (bits - 1, bits, bits + 1)]


def focal_values(rng):
    base = [rng.uniform(0, 2000), rng.uniform(-2000, 2000), rng.randrange(1, 2000) + 0.5,
            -(rng.randrange(0, 2000) + 0.5), rng.randrange(0, 4000) * 0.25, 0.0, -0.0]
    return base


EXTREME = [2147483520.0, 2147483648.0, -2147483648.0, -2147483904.0, 3e9, -3e9, 1e19, -1e19,
           1e30, math.inf, -math.inf, math.nan]


def cases(rng, n):
    for i in range(n):
        focal = rng.choice(focal_values(rng))
        if rng.random() < 0.5 and focal != 0.0:
            focal = rng.choice(_neighbours(focal))
        if EXTREME_EVERY and i % EXTREME_EVERY == EXTREME_EVERY - 1:
            focal = rng.choice(EXTREME)
        yield dict(
            enabled=rng.random() > 0.05,
            toggle314=rng.random() < 0.3,
            texture_mgr=rng.random() < 0.8,
            mesh_mgr=rng.random() < 0.8,
            texture_sizes=[rng.choice([0, 16, 32, 64, 128, 256, rng.randrange(1, 300)])
                           for _ in range(rng.choice([0, 0, 1, 3, 12]))],
            peaks=(rng.choice([0, 2, 50]), rng.choice([0, 1000, 10 ** 6])),
            draw=(rng.choice([1000, 400]), rng.choice([1000, 400])),
            field84=rng.random() < 0.5,
            cell=_f32(rng.choice([1.0, 4.0, 8.0, 3.0, rng.uniform(0.5, 32.0)])),
            eye=[_f32(rng.uniform(-5000, 5000)) for _ in range(3)],
            focal=focal,
        )


def setup(ctx, case):
    rng = ctx.rng
    ctx.pack(TOGGLE_718, 'i', 1 if case['enabled'] else 0)
    ctx.pack(TOGGLE_314, 'i', 1 if case['toggle314'] else 0)

    tags = ctx.alloc(0x40)
    ctx.pack(MEM_TAG_STACK, 'I', tags)
    clock = {'t': rng.randrange(1 << 30)}

    def read_clock(ctx_, ecx, args):
        clock['t'] = (clock['t'] + rng.randrange(1, 100000)) & 0xffffffff
        return clock['t']

    def total_size(ctx_, ecx, args):
        total = rng.choice([0, 0x80000, 0x200000, rng.randrange(1 << 24)])
        stale = rng.choice([0, 0x100000, 0x100001, rng.randrange(total + 1)])
        ctx_.pack(args[0], 'i', stale)
        return total

    for name, va, pop in CALLEES:
        ret = 0
        if name == 'ReadClock':
            ret = read_clock
        elif name == 'MemTagStack::Push':
            ret = 0x1234
        elif name in ('DrawableGridNode::TestVisibility', 'DrawableGridNode::WalkVisibleFromRoot',
                      'DrawableGridNode::WalkSlot4FromRoot'):
            ret = 1
        ctx.stub_at(va, name, pop=pop, ret=ret)
    # TotalSize logs its argument pointer, which is a frame slot in both bodies.
    ctx.stub_at(0x00401130, 'AgeManager::TotalSize', pop=4, ret=total_size)

    sizes = case['texture_sizes']
    ctx.pack(TEXTURE_COUNT, 'i', len(sizes))
    if sizes:
        ctx.pack(TEXTURE_SIZES, '%di' % len(sizes), *sizes)
    ctx.pack(0x0068a2f0, 'ii', *case['peaks'])

    camera = SHARED_STATE if case['toggle314'] else ctx.alloc(0x220)
    ctx.pack(camera + 0x170, '3f', *case['eye'])
    ctx.pack(camera + 0x198, 'f', case['focal'])
    host = ctx.alloc(0x10)
    ctx.pack(host + 8, 'I', camera)

    this = ctx.alloc(0xd00)
    ctx.write(this, bytes(rng.randrange(256) for _ in range(0x100)))
    ctx.pack(this + 0x18, 'I', host)
    ctx.pack(this + 0x3c, 'I', ctx.alloc(0x140))          # stream
    ctx.pack(this + 0x40, 'f', case['cell'])
    ctx.pack(this + 0x44, 'I', ctx.alloc(0x60))           # height field root
    ctx.pack(this + 0x6c, 'i', case['draw'][0])
    ctx.pack(this + 0x84, 'i', 1 if case['field84'] else 0)
    ctx.pack(this + 0xbec, 'i', case['draw'][1])
    ctx.pack(this + 0xc84, 'I', ctx.alloc(0x14) if case['texture_mgr'] else 0)
    ctx.pack(this + 0xc88, 'I', ctx.alloc(0x14) if case['mesh_mgr'] else 0)
    return dict(ecx=this, args=())
