"""WriteTga 0x00512990, the TGA screenshot/texture writer (tools/emu_diff.py case,
call-record/replay mode).

The linked body is samples/render/TgafileNearMisses.cpp. fopen 0x00534ceb answers a handle or
NULL, fwrite 0x005350b7 notes (bytes, size, count) and answers the full count or, at a chosen
call, a short count, so every header field and early exit is compared; fclose 0x00534c3d is
logged. Images are 24, 32, 16 (555 when greenMask is 0x3e0, else 565) or other bit depths, with
zero, negative and odd widths and heights and packed or padded strides; the 32-bit branch swaps
red and blue in place, which the write comparison covers.
"""
import random

RETAIL_VA = 0x00512990
SYMBOL = '?WriteTga@@YAHPAUTgaFile@@IHH@Z'
SOURCE = 'samples/render/TgafileNearMisses.cpp'
PROBES = {0x00512af0: '24', 0x00512bb6: '32', 0x00512cab: '555', 0x00512d1c: '565', 0x00512dba: 'fail'}

FOPEN, FWRITE, FCLOSE = 0x00534ceb, 0x005350b7, 0x00534c3d
HANDLE = 0x0bad0f11


def _cstr(ctx, at, limit=0x110):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _fopen(ctx, rec):
    ctx.note('fopen', _cstr(ctx, rec.args[0]), _cstr(ctx, rec.args[1]))
    return HANDLE if ctx.data['open'] else 0


def _fwrite(ctx, rec):
    data, size, count, stream = rec.args[:4]
    n = size * count
    ctx.note('fwrite', size, count, stream, bytes(ctx.read(data, n)) if 0 < n <= 0x4000 else n)
    k = ctx.data['calls']
    ctx.data['calls'] = k + 1
    if k == ctx.data['fail_at']:
        return max(count - 1, 0) if count else 1
    return count


def _fclose(ctx, rec):
    ctx.note('fclose', rec.args[0])
    return 0


returns = {FOPEN: _fopen, FWRITE: _fwrite, FCLOSE: _fclose}
pops = {FOPEN: 0, FWRITE: 0, FCLOSE: 0}
args = {FOPEN: 2, FWRITE: 4, FCLOSE: 1}
ecx = {FOPEN: False, FWRITE: False, FCLOSE: False}
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=(FOPEN, FWRITE, FCLOSE),
              log_pointees=False)


def cases(rng, n):
    for _ in range(n):
        bpp = rng.choice([24, 24, 32, 32, 16, 16, 15, 8, 0])
        width = rng.choice([0, 1, 2, 3, 5, 8, -1, -3])
        height = rng.choice([0, 1, 2, 4, -2])
        bytes_pp = {24: 3, 32: 4}.get(bpp, 2)
        stride = rng.choice([0, 0, max(width, 0) * bytes_pp, max(width, 0) * bytes_pp + bytes_pp,
                             max(width, 0) * bytes_pp + 1, 7])
        # calls: 12 header writes, then the rows; fail somewhere or never
        fail_at = rng.choice([None, None, None, 0, 1, 5, 11, 12, 13, 15])
        yield dict(seed=rng.getrandbits(32), bpp=bpp, width=width, height=height, stride=stride,
                   green=rng.choice([0x3e0, 0x7e0, 0]), descriptor=rng.choice([0, 0x20, 0x28, 0x1ff]),
                   open=rng.random() < 0.9, fail_at=fail_at,
                   name=rng.choice(['shot.tga', 'a' * 0x103, '']))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data.update(open=c['open'], fail_at=c['fail_at'], calls=0)
    rows = max(c['height'], 0)
    span = max(c['stride'], max(c['width'], 0) * 4)
    size = 0x40 + rows * span * 2
    bits = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))
    tga = ctx.alloc(0x130, bytes(r.getrandbits(8) for _ in range(0x130)))
    ctx.pack(tga + 0xe, 'h', c['width'])
    ctx.pack(tga + 0x10, 'h', c['height'])
    ctx.pack(tga + 0x12, 'B', c['bpp'])
    ctx.pack(tga + 0x14, 'I', bits)
    ctx.write(tga + 0x1c, c['name'].encode() + b'\0')
    return dict(args=[tga, c['stride'], c['green'], c['descriptor']])

