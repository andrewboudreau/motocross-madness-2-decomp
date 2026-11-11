"""KbScoreBoard::ShowValue 0x0048d1e0 (tools/emu_diff.py case).

Stubs: HideDigits 0x0048d540, ClearFlag14c 0x004fdb50, AddChild 0x004fd910, NextKey 0x0048cad0,
GetPosition 0x004fc970 (writes the case's racer position) and FastSqrt 0x00460b50 (returns the
binary32 square root). Every stub call is logged with its `this` and argument, so the digit
models chosen (from both conversions) are compared as well as the stored digits, the
eye-to-frame vector, the distance and the re-size flag.

Points include ties (k + 0.5, which fistp rounds to even), their neighbours and values just
below or above hundreds. Points outside int are not generated: retail's `fistp dword` stores
0x80000000 there and the negative digits then index outside the model tables (retail faults).
"""
import math
import struct

RETAIL_VA = 0x0048d1e0
SYMBOL = '?ShowValue@KbScoreBoard@@QAEXMM@Z'
SOURCE = 'src/bringup/Group4_ScoreBoard.cpp'
PROBES = {0x0048d221: 'points fistp', 0x0048d33a: 'tenths fistp'}


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _next(x, d):
    b = struct.unpack('<i', struct.pack('<f', x))[0]
    return struct.unpack('<f', struct.pack('<i', b + d))[0]


def cases(rng, n):
    for _ in range(n):
        kind = rng.choice(['rand', 'tie', 'tie', 'hundred'])
        if kind == 'rand':
            points = rng.uniform(-99, 120000)
        elif kind == 'tie':
            points = rng.randrange(0, 100000) + 0.5
        elif kind == 'hundred':
            points = rng.randrange(0, 1000) * 100 + rng.choice([-0.5, 0.5, -1.5, 49.5, 50.5, 99.5])
        points = _f32(points)
        if kind in ('tie', 'hundred') and rng.randrange(3) == 0:
            points = _next(points, rng.choice([-1, 1]))
        mult = rng.choice([1.0, 1.05, _next(1.05, -1), _next(1.05, 1), 1.25, 1.35, 1.45, 2.5, 9.95,
                           rng.uniform(1.0, 10.0), rng.randrange(10, 100) / 10 + 0.05])
        eye = [rng.uniform(-500, 500) for _ in range(3)]
        if rng.randrange(8) == 0:
            pos = list(eye)
        else:
            pos = [e + rng.choice([rng.uniform(-50, 50), rng.uniform(-0.5, 0.5)]) for e in eye]
        distance = rng.choice([0.0, rng.uniform(0, 100)])
        yield (points, mult, eye, pos, distance)


def setup(ctx, case):
    points, mult, eye, pos, distance = case
    ctx.stub_at(0x0048d540, 'HideDigits')
    ctx.stub_at(0x004fdb50, 'ClearFlag14c')
    ctx.stub_at(0x004fd910, 'AddChild', pop=4)
    ctx.stub_at(0x0048cad0, 'NextKey')

    def get_position(c, ecx, args):
        c.pack(args[0], '3f', *pos)
    ctx.stub_at(0x004fc970, 'GetPosition', pop=4, ret=get_position)

    def fast_sqrt(c, ecx, args):
        c.note('FastSqrt', args[0])    # cdecl: ECX is not an argument
        value = struct.unpack('<f', struct.pack('<I', args[0]))[0]
        return 0, _f32(math.sqrt(value)) if value >= 0 else float('nan')
    ctx.stub_at(0x00460b50, 'FastSqrt', nargs=1, ret=fast_sqrt, ret_float='f', log=False)

    board = ctx.alloc(0x200, fill=0x66)
    models = [ctx.alloc(0x190, fill=0x10) for _ in range(72)]
    ctx.pack(board + 0x40, '50I', *models[:50])             # digitModels[10][5]
    ctx.pack(board + 0x11c, '2I', models[70], models[71])    # times, decimal point
    ctx.pack(board + 0x124, '20I', *models[50:70])          # fractionDigitModels[10][2]
    ctx.pack(board + 0x3c, 'I', ctx.alloc(0x40))            # baseFrame
    node = ctx.alloc(0x40)
    racer = ctx.alloc(0x400)
    ctx.pack(racer + 0x3bc, 'I', node)
    view = ctx.alloc(0x40)
    ctx.pack(view + 0x38, 'I', racer)
    ctx.pack(board + 0x2c, 'I', view)
    camera = ctx.alloc(0x180)
    ctx.pack(camera + 0x170, '3f', *eye)
    ctx.pack(board + 0x38, 'I', camera)
    ctx.pack(board + 0x1ec, 'f', distance)
    return dict(ecx=board, args=[points, mult], ret_eax=False)
