"""DrawableGridNodeSharedTextures::BuildNodeTexture 0x0047d470, the node-wide terrain texture
built from run-length tile rows (tools/emu_diff.py case).

The linked body is samples/render/Grid1NearMisses.cpp. The node's extra (+0x3c) holds the run
table (+4) and the packed own bit and index (+0xc: index -1 or own clear return 0). Each run is a
(tile, length - 1) byte pair; the runs of a row sum to 256 pixels, as the terrain files have
them, with lengths from 1 to 256 so runs fit inside one tile, start or end mid-tile and span
several tiles. Terrain (+0x34) textures[id] (+0x544) answer LockLevel(16) (slot +0x40) with a
16 x 16 tile of random pixels; the target texture answers LockSurface (+0x34) with its bits and
is 256 wide (written in place) or 128/64/32 wide (staged in the 0x006652a0 buffer, then
Downsample 0x004d1b90 is logged). PixelFormatBytesPerPixel 0x00511970 answers 1 to 4 (3 and 4
only for 256-wide textures: the staging buffer holds 0x20000 bytes). The staging buffer is
present or allocated through DebugMalloc 0x004a2e20 (noted by size and line: the __FILE__ tag is
the unit's own). Every byte written is compared.
"""
import random

RETAIL_VA = 0x0047d470
SYMBOL = '?BuildNodeTexture@DrawableGridNodeSharedTextures@@QAEHPAVManagedTexture@@@Z'
SOURCE = 'samples/render/Grid1NearMisses.cpp'
FPCWS = (0x027f,)   # integer only
PROBES = {0x0047d495: 'malloc', 0x0047d574: 'new tile', 0x0047d5bf: 'one span', 0x0047d603: 'head',
          0x0047d664: 'whole tile', 0x0047d697: 'tail', 0x0047d729: 'downsample', 0x0047d772: 'none'}

BUFFER = 0x006652a0
BINDINGS = {'_g_grid1TextureBuffer': BUFFER}
BPP, MALLOC, DOWNSAMPLE = 0x00511970, 0x004a2e20, 0x004d1b90


def _row(r, style):
    lengths = []
    left = 256
    while left:
        if style == 'short':
            n = r.randint(1, 5)
        elif style == 'long':
            n = r.choice([16, 32, 17, 31, 47, 100, 256])
        elif style == 'aligned':
            n = 16 * r.randint(1, 4)
        else:
            n = r.choice([1, 2, 3, 7, 15, 16, 17, 20, 33, 64, 255])
        n = min(n, left)
        lengths.append(n)
        left -= n
    return lengths


def cases(rng, n):
    for _ in range(n):
        width = rng.choice([0x100, 0x100, 0x80, 0x40, 0x20])
        bpp = rng.choice([1, 2, 2, 3, 4] if width == 0x100 else [1, 2, 2])
        yield dict(seed=rng.getrandbits(32), width=width, bpp=bpp,
                   style=rng.choice(['short', 'long', 'aligned', 'mixed', 'mixed']),
                   tiles=rng.choice([1, 2, 5, 40, 256]),
                   index=rng.choice([0, 3, 100, 100, -1]), own=rng.random() < 0.92,
                   staged=rng.random() < 0.7)


def setup(ctx, c):
    r = random.Random(c['seed'])
    bpp, width = c['bpp'], c['width']
    runs = bytearray()
    for _ in range(256):
        for n in _row(r, c['style']):
            runs += bytes([r.randrange(c['tiles']), n - 1])
    index = c['index']
    data = ctx.alloc(len(runs) + 2 * max(index, 0) + 16)
    ctx.write(data + 2 * max(index, 0), runs)

    staging = ctx.alloc(0x20000, fill=0x5a)
    ctx.pack(BUFFER, 'I', staging if c['staged'] else 0)
    def malloc(cx, ecx, args):   # the __FILE__ tag is the unit's own: note size and line
        cx.note('DebugMalloc', args[0], args[2])
        return staging
    ctx.stub_at(MALLOC, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(BPP, 'PixelFormatBytesPerPixel', nargs=1, ret=bpp)
    ctx.stub_at(DOWNSAMPLE, 'Downsample', nargs=10, ret=1)

    def lock_level(cx, ecx, args):
        return cx.data['tile_bits'][ecx]
    tile_vt = ctx.vtable([ctx.stub('tile slot %d' % k) for k in range(16)]
                         + [ctx.stub('LockLevel', pop=4, ret=lock_level),
                            ctx.stub('UnlockLevel', pop=4, ret=1)] + [ctx.stub('tile slot 18')])
    terrain = ctx.alloc(0xc90, fill=0x33)
    ctx.data['tile_bits'] = {}
    for k in range(256):
        tex = ctx.alloc(0x30)
        ctx.pack(tex, 'I', tile_vt)
        bits = ctx.alloc(16 * 16 * bpp, bytes(r.getrandbits(8) for _ in range(16 * 16 * bpp)))
        ctx.data['tile_bits'][tex] = bits
        ctx.pack(terrain + 0x544 + 4 * k, 'I', tex)

    out = ctx.alloc(width * width * bpp + 16, fill=0x77)
    tex_vt = ctx.vtable([ctx.stub('texture slot %d' % k) for k in range(13)]
                        + [ctx.stub('LockSurface', pop=12, ret=out),
                           ctx.stub('UnlockSurface', pop=4, ret=1),
                           ctx.stub('BuildMipLevels', pop=4, ret=1)] + [ctx.stub('texture slot 16')])
    texture = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
    ctx.pack(texture, 'I', tex_vt)
    ctx.pack(texture + 0x14, 'i', width)

    extra = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(extra + 4, 'I', data)
    packed = (r.getrandbits(16) << 16) | ((index & 0x7fff) << 1) | int(c['own'])
    ctx.pack(extra + 0xc, 'I', packed)
    node = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
    ctx.pack(node + 0x34, 'I', terrain)
    ctx.pack(node + 0x3c, 'I', extra)
    return dict(ecx=node, args=[texture])
