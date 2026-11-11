"""Terrain::Load 0x005059d0, the .trn terrain file loader (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/krusty2/broadphase/Terrain.cpp (the quarry loader's
UnknownQuarryTerrainObject::Load forward reaches it). The stream (UnknownTextureStream Read
0x00461640, Seek 0x00461340, Tell 0x00461600) is modelled over a synthetic file: the tag
("TRN", other tags, truncated), the version (5 or not), the palette flag, the header words
(field 0xbe8, the cell size as a float, the grid size, an unused word, the block count and
records, the owned-object count and surface records, from negative to a few), then per surface
record width (0x100 or not), height and format; reads past the end leave the destination
alone. The stream may be an inner one (+0x1c, start +0x130). Optional: the stream itself
(null releases), the detail texture name (null picks "testnoise.tga"; TerrainLoadTexture
0x0050a590 may fail, the texture's average colour feeds the unlit colours), the surface
registry (ManagedTexture path), the renderer's softwareRendering flag (quality table pointer
0x0068a32c), the enabled bit (+0x25) and CreatePalette's result (DirectDraw COM slot 5, the
256 palette entries compared). operator new answers each size with a fresh object (fake
vtables for the surfaces, a height field whose level/shift bytes +0x28/+0x29 drive the
inverse cell size) or null; the height field's constructor writes random bounds, so the 16.0
padding and the cell-size scaling run. g_gridSize/g_gridLevels (0x0067a684/0x0056c200) are
compared. fs:[0] is page 0.
"""
import random
import struct

RETAIL_VA = 0x005059d0
SYMBOL = '?Load@Terrain@@QAEPAV1@HPAVTerrainStream@@HPAUTerrainSurfaceDesc@@HHPBDH@Z'
SOURCE = 'src/krusty2/broadphase/Terrain.cpp'
PROBES = {0x00505bcb: 'header ok', 0x00505cf3: 'palette', 0x00505e73: 'blocks',
          0x00505f32: 'managed', 0x00505fcd: 'texture map', 0x005061b1: 'inverse cell',
          0x005061e8: 'fail'}

READ, SEEK, TELL = 0x00461640, 0x00461340, 0x00461600
NEW, CALLOC, MALLOC = 0x004a3010, 0x004a2fc0, 0x004a2e20
LOAD_TEXTURE, GET_COLORS = 0x0050a590, 0x004de270
CTORS = (0x00401000, 0x004ddf40, 0x0047dc20, 0x00510500, 0x004c5f00, 0x0047c880)
GRID_NODE_CTOR = 0x0047c880
TRACK_GAME = 0x0056e26c
# DirectDraw's CreatePalette (slot 5) is a COM __stdcall call (`this` pushed, ECX scratch);
# setup builds its fake vtable first, so the slot sits at a fixed fake address.
CREATE_PALETTE = 0x22000000 + 4 * 5


def _cstr(ctx, at, limit=256):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _read(ctx, rec):
    dst, size, count = rec.args
    s = ctx.data['stream']
    n = max(size * count, 0)
    pos = s['pos']
    chunk = s['file'][pos:pos + n]
    if chunk:
        ctx.write(dst, chunk)
    s['pos'] = pos + len(chunk)
    return len(chunk) // size if size > 0 else 0


def _seek(ctx, rec):
    off, whence = rec.args[0] - ((rec.args[0] & 0x80000000) << 1), rec.args[1]
    s = ctx.data['stream']
    s['pos'] = max(0, (s['pos'] if whence == 1 else 0) + off)
    return 0


def _tell(ctx, rec):
    return ctx.data['stream']['pos']


def _new(ctx, rec):
    size, line = rec.args[0], rec.args[2]
    ctx.note('new', size, line)
    if line in ctx.data['null_lines']:
        return 0
    make = ctx.data['objects'].get(size)
    return make() if make else ctx.alloc(size, fill=0xcd)


def _calloc(ctx, rec):
    ctx.note('calloc', rec.args[0], rec.args[1], rec.args[3])
    return ctx.alloc(rec.args[0] * rec.args[1] & 0xffff)


def _malloc(ctx, rec):
    ctx.note('malloc', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0] & 0xffff, 4), fill=0xcd)


def _load_texture(ctx, rec):
    ctx.note('TerrainLoadTexture', _cstr(ctx, rec.args[1]), rec.args[0], *rec.args[2:])
    return ctx.data['detail']


def _colors(ctx, rec):
    return ctx.data['colors']


def _this(ctx, rec):
    return rec.ecx


def _grid_node(ctx, rec):
    # The height field's constructor writes the bounds (args 9 and 10).
    r = ctx.data['rng']
    for at in rec.args[9:11]:
        ctx.pack(at, '3f', *(r.choice([r.uniform(-500, 500), 0.0, 3.4e38]) for _ in range(3)))


REPLAY = dict(
    returns={READ: _read, SEEK: _seek, TELL: _tell, NEW: _new, CALLOC: _calloc, MALLOC: _malloc,
             LOAD_TEXTURE: _load_texture, GET_COLORS: _colors, **{c: _this for c in CTORS}},
    effects={GRID_NODE_CTOR: _grid_node},
    pops={NEW: 0, CALLOC: 0, MALLOC: 0, LOAD_TEXTURE: 0, CREATE_PALETTE: 0x14},
    args={NEW: 3, CALLOC: 4, MALLOC: 3, LOAD_TEXTURE: 12, CREATE_PALETTE: 5},
    ecx={NEW: False, CALLOC: False, MALLOC: False, LOAD_TEXTURE: False, CREATE_PALETTE: False},
    pointees={CREATE_PALETTE: [(2, 0x400)]},
    unlogged=(NEW, CALLOC, MALLOC, LOAD_TEXTURE), out_writes=False)


def _word(rng):
    return rng.choice([0, 1, 2, 3, -1, 5, rng.randrange(-4, 9)])


def cases(rng, n):
    for _ in range(n):
        surfaces = rng.choice([0, 0, 1, 2, 5, -1])
        records = [(rng.choice([0x100, 0x100, 0x80, 0x40, 1, 0, -8]), rng.choice([0x100, 0x40]),
                    rng.randrange(0, 8)) for _ in range(max(surfaces, 0))]
        blocks = rng.choice([0, 1, 3, -1])
        yield dict(seed=rng.getrandbits(32),
                   tag=rng.choice([b'TRN\0'] * 6 + [b'TRX\0', b'TRNX', b'trn\0', b'']),
                   version=rng.choice([5] * 6 + [4, 6, -5]),
                   palette=rng.choice([0, 0, 1]), palette_ok=rng.random() < 0.8,
                   header=(rng.getrandbits(32), rng.choice([1.0, 2.5, 0.125, rng.uniform(0.01, 64)]),
                           rng.choice([0x40, 0x80, 0x100, 0, 7]), rng.getrandbits(32)),
                   blocks=(rng.choice([max(blocks, 0), max(blocks, 0) + 2]), blocks),
                   owned=rng.choice([0, 1, 3]), surfaces=surfaces, records=records,
                   truncate=rng.choice([None] * 6 + [rng.randrange(0, 80)]),
                   stream=rng.random() < 0.95, inner=rng.random() < 0.3,
                   name=rng.choice([None, 'detail.tga', '', 'x' * 120]),
                   detail=rng.random() < 0.8, registry=rng.random() < 0.6,
                   software=rng.choice([0, 1, 7]), enabled=rng.random() < 0.5,
                   half=rng.choice([0, 1, 9]), light=rng.choice([0, 1, 0x1234]),
                   quality=rng.choice([0, 1, 2, 9, -1]),
                   null_lines=rng.choice([()] * 8 + [(0x11c,), (0x122,), (0x1e6,), (0x167,)]),   # the surfaces are used unchecked
                   level=rng.choice([0, 1, 2, 3]), shift=rng.choice([4, 6, 8, 12]),
                   colour=rng.choice([0xffffffff, 0, 0x80402010, rng.getrandbits(32)]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    ctx.data['rng'] = r

    def rnd(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    ddraw_vt = ctx.fake_vtable('DirectDraw', 8, returns={5: lambda ctx, rec: 0 if c['palette_ok'] else 0x88760000})
    surface_vt = ctx.fake_vtable('TerrainSurface', 16, pops={5: 0x3c, 8: 0xc})

    body = c['tag'] + struct.pack('<iiI', c['version'], c['palette'], c['header'][0])
    body += struct.pack('<fIIii', c['header'][1], c['header'][2], c['header'][3], *c['blocks'])
    body += b'\xee' * 4 * max(c['blocks'][0], 0)
    body += struct.pack('<ii', c['owned'], c['surfaces']) + b'\xdd' * 4 * c['owned']
    for rec in c['records']:
        body += struct.pack('<iii', *rec)
    lead = 0x30 if c['inner'] else 0
    data = b'\x99' * lead + body
    if c['truncate'] is not None:
        data = data[:lead + c['truncate']]
    ctx.data['stream'] = dict(file=data, pos=0)
    stream = 0
    if c['stream']:
        stream = ctx.alloc(0x140, rnd(0x140))
        ctx.pack(stream + 0x1c, 'I', ctx.alloc(0x10) if c['inner'] else 0)
        ctx.pack(stream + 0x130, 'I', lead)

    def surface(size):
        def make():
            o = ctx.alloc(size, rnd(size))
            ctx.pack(o, 'I', surface_vt)
            return o
        return make

    def grid_node():
        o = ctx.alloc(0x5c, rnd(0x5c))
        ctx.pack(o + 0x28, 'BB', c['level'], c['shift'])
        return o
    ctx.data['objects'] = {0xbc: surface(0xbc), 0x80: surface(0x80), 0x5c: grid_node}
    ctx.data['null_lines'] = c['null_lines']
    ctx.data['colors'] = ctx.alloc(0x300, rnd(0x300))
    detail = 0
    if c['detail']:
        detail = ctx.alloc(0x80, rnd(0x80))
        ctx.pack(detail, 'I', ctx.fake_vtable('DetailTexture', 16, pops={8: 0xc}))
        ctx.pack(detail + 0x3c, 'I', c['colour'])
    ctx.data['detail'] = detail

    track = ctx.alloc(0x600, rnd(0x600))
    ctx.pack(track + 0x2d0, 'i', c['software'])
    ctx.pack(TRACK_GAME, 'I', track)
    ddraw = ctx.alloc(0x20, rnd(0x20))
    ctx.pack(ddraw, 'I', ddraw_vt)
    display = ctx.alloc(0x200, rnd(0x200))
    ctx.pack(display + 0x190, 'I', ddraw)
    host = ctx.alloc(0x100, rnd(0x100))
    ctx.pack(host + 4, 'I', display)

    this = ctx.alloc(0xcc4, rnd(0xcc4))
    ctx.pack(this, 'I', ctx.fake_vtable('Terrain', 16))
    ctx.pack(this + 0x18, 'I', host)
    flags = r.getrandbits(8)
    ctx.pack(this + 0x25, 'B', (flags | 1) if c['enabled'] else (flags & ~1))
    desc = ctx.alloc(0x18, rnd(0x18))
    ctx.pack(desc + 4, 'I', ctx.alloc(0x40, rnd(0x40)) if c['registry'] else 0)
    name = 0
    if c['name'] is not None:
        name = ctx.alloc(0x100, c['name'].encode() + b'\0')
    return dict(ecx=this, args=[host, stream, r.getrandbits(32), desc, c['quality'], c['light'], name,
                                c['half']])
