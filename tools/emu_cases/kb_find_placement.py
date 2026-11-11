"""KrustyBike::FindPlacement 0x00491d10, slot 11, the respawn placement (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy. Every event type (0..5 and out of range), every
selected series (GetSelectedSeries 0x00524100 runs as retail code: KbGame +0x578 +0x2800), the
mode 2/3 "copy the stored vectors" conditions one at a time, a zero-length heading, the tag
event's ball / followed racer arms and the respawn-pending arms. PointInPlacementSquare
(0x004b0ac0) and FindObjectPlacement (0x004b0df0) are cdecl stubs that note their arguments
(ECX is not an argument there, so the calls are not logged with it); FastInvSqrt runs as
retail code.
"""
import struct

RETAIL_VA = 0x00491d10
SYMBOL = '?FindPlacement@KrustyBike@@UAEHHPAUVec3@@00PAH@Z'
SOURCE = 'src/bringup/Group3_KrustyBike.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x00491dc6: 'PointInPlacementSquare', 0x00491f5e: 'mode 2/3 copy',
          0x00491edb: 'mode 2/3 zero heading', 0x00491f3b: 'mode 2/3 invsqrt',
          0x00491fbb: 'mode 0', 0x00492010: 'tag ball', 0x0049201f: 'tag self',
          0x00492045: 'tag other', 0x004920d7: 'mode 1/5 pending', 0x00492125: 'mode 1/5 gate',
          0x004921cb: 'default'}


def _vec(rng):
    return [rng.choice([0.0, rng.uniform(-200, 200)]) for _ in range(3)]


def cases(rng, n):
    for _ in range(n):
        position = _vec(rng)
        respawn_position = _vec(rng)
        if rng.random() < 0.15:                      # zero-length mode 2/3 heading
            position[1] = 0.0
            respawn_position[0], respawn_position[2] = position[0], position[2]
        # One of the copy conditions (or none) for mode 2/3.
        cond = rng.choice(['none', 'none', 'race', 'track', 'ai', 'ghost', 'respawn', 'c50'])
        yield dict(
            event=rng.choice([0, 1, 2, 3, 4, 5, 6, -1]),
            series=rng.choice([0, 1, 2, 3, 4, 5, 6]),
            cond=cond,
            respawn=2 if cond == 'respawn' else rng.choice([0, 1]),
            pending=rng.choice([0, 1, 0x80]),
            position=position, respawn_position=respawn_position,
            respawn_heading=_vec(rng), body_forward=_vec(rng),
            hit=rng.choice([0, 1, 0x100, 0x1ff, 0xffffffff]),
            placed=rng.choice([0, 1, 0x12345678]),
            tag=rng.choice(['ball', 'self', 'other']),
            gate=rng.randrange(4),
            scale=rng.uniform(0.01, 4),
        )


def setup(ctx, c):
    def square(cx, ecx, a):
        cx.note('PointInPlacementSquare', *a)
        return c['hit']

    def place(cx, ecx, a):
        a = list(a)
        a[9] &= 0xff                                 # unsigned char argument
        cx.note('FindObjectPlacement', *a)
        return c['placed']
    ctx.stub_at(0x004b0ac0, 'PointInPlacementSquare', nargs=9, ret=square, log=False)
    ctx.stub_at(0x004b0df0, 'FindObjectPlacement', nargs=17, ret=place, log=False)

    bike = ctx.alloc(0x800, fill=0x5a)
    ctx.pack(bike + 0x0c, '3f', *c['position'])
    ctx.pack(bike + 0x88, '3f', *c['body_forward'])
    ctx.pack(bike + 0x109, 'B', c['pending'] & 0xff)
    ctx.pack(bike + 0x10c, '6f', *c['respawn_position'], *c['respawn_heading'])
    ctx.pack(bike + 0x128, 'I', ctx.alloc(0x40))           # collision object
    ctx.pack(bike + 0x1f4, 'I', ctx.alloc(0x40))           # terrain
    ctx.pack(bike + 0x1f8, 'f', c['scale'])
    cond = c['cond']
    ctx.pack(bike + 0x734, 'B', 1 if cond == 'ai' else 0)
    ctx.pack(bike + 0x736, 'B', 1 if cond == 'ghost' else 0)
    ctx.pack(bike + 0x78c, 'I', 1 if cond == 'track' else 0)
    ctx.pack(bike + 0x7b8, 'I', c['gate'])

    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d70, 'iii', 1 if cond == 'race' else 0, c['event'], c['series'])
    ctx.pack(game + 0xc50, 'I', 1 if cond == 'c50' else 0)
    gates = ctx.alloc(24 * 16)
    for k in range(16):
        ctx.pack(gates + 24 * k, '3f', k + 0.5, -k * 2.0, k * 3.25)
    ctx.pack(game + 0x560, 'I', gates)
    tag = ctx.alloc(0x100)
    other = ctx.alloc(0x40)
    ctx.pack(other + 0xc, '3f', 7.0, 8.0, 9.0)
    ball = ctx.alloc(0x40)
    ctx.pack(ball + 0xc, '3f', 1.0, 2.0, 3.0)
    ctx.pack(tag + 0xa8, 'I', bike if c['tag'] == 'self' else other)
    ctx.pack(tag + 0xdc, 'I', ball if c['tag'] == 'ball' else 0)
    ctx.pack(game + 0x568, 'I', tag)
    ctx.pack(TRACK_GAME, 'I', game)

    out_position = ctx.alloc(12, fill=0x77)
    out_forward = ctx.alloc(12, fill=0x78)
    out_up = ctx.alloc(12, fill=0x79)
    out_keep = ctx.alloc(4, fill=0x7a)
    return dict(ecx=bike, args=[c['respawn'], out_position, out_forward, out_up, out_keep])
