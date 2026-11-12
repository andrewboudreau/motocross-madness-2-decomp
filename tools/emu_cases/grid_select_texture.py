"""DrawableGridNodeSharedTextures::SelectTexture 0x0047caa0, the terrain node's texture LOD choice
(tools/emu_diff.py case, call-record/replay mode).

The linked body is samples/render/Grid1NearMisses.cpp. The detail (node size on screen: 16 <<
shift times the terrain's focal length +0x70 over the distance, 10000 at distance 0) picks a stage
against g_grid1DetailDistance (0x0056c034) times 1/16, 1/4, 1 and 4; a root node (level 0) whose
camera moved more than 0.5 (Terrain +0x18 -> +8 -> +0x200) caps the wanted stage at 2. Each stage
falls back to the next coarser one when its texture is missing: 0 the parent's node texture, 1 the
parent's block texture (16 packed records, +0x38 of the extra), 2 the node's own or shared texture
(built through AcquireOwnedObject 0x00505600 and BuildNodeTexture 0x0047d470 when the stage is
the wanted one), 3 one texture per block (built per visible block, TestBlockFrustum 0x00481a20).
The draw data's texture origin and step (+0x158..+0x160), the extra's texture, flags, mapping and
detail, the block records and g_grid1OwnTextureCount (0x0066529c) are compared.

Inputs: distances 0, near and far around every stage threshold; nodes with and without a parent
(whose extra has a shared, own or missing texture and block records), packed texture references
of -1, shared and own; the whole-node and per-block built-texture flags; textures 64, 128 and 256
wide with and without the ManagedTexture remap bit. Every call is replayed; AcquireOwnedObject
answers fresh textures, TestBlockFrustum a seeded 0 or 1.

Inputs leave out one combination: a node without block records whose own texture is not built
yet (own reference, flag 0x10000 clear) while the wanted stage is 3. There stage 3 falls back to
2, stage 2 sets the stage back to the wanted 3 to build nothing, and the retry never ends, in
retail (0x0047cc4e..0x0047cc60 and the default arm) as in the source.
"""
import random
import struct

RETAIL_VA = 0x0047caa0
SYMBOL = '?SelectTexture@DrawableGridNodeSharedTextures@@UAEXM@Z'
SOURCE = 'samples/render/Grid1NearMisses.cpp'
DETAIL_DISTANCE = 0x0056c034
OWN_COUNT = 0x0066529c
ACQUIRE = 0x00505600
TEST_FRUSTUM = 0x00481a20
BUILD_NODE, BUILD_BLOCK = 0x0047d470, 0x0047d780
FORCE_CENTRES, RELEASE_CENTRES = 0x00483040, 0x00483100
REREGISTER = 0x00510670
REGISTER = 0x00401050
BYTES_PER_PIXEL = 0x00511970
PROBES = {0x0047cc83: 'block stage', 0x0047cd31: 'build block texture', 0x0047ce9c: 'parent block',
          0x0047cf29: 'shared texture', 0x0047cf98: 'build node texture', 0x0047d01f: 'own texture'}


def _acquire(ctx, rec):
    return ctx.data['fresh'][rec.occurrence % len(ctx.data['fresh'])]


def _frustum(ctx, rec):
    return int(rec.rng.random() < 0.6)


def _bpp(ctx, rec):
    return rec.rng.choice([1, 2, 3, 4])


REPLAY = dict(
    returns={ACQUIRE: _acquire, TEST_FRUSTUM: _frustum, BUILD_NODE: 'int', BUILD_BLOCK: 'int',
             FORCE_CENTRES: 'int', RELEASE_CENTRES: 'int', REREGISTER: 'void', REGISTER: 'void',
             BYTES_PER_PIXEL: _bpp},
    out_writes=False,
)


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def _texture(ctx, r):
    t = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
    ctx.pack(t + 0x14, 'i', r.choice([0x40, 0x80, 0x100, 0x20]))
    ctx.pack(t + 0x68, 'i', r.choice([0, 1, 3, 2]))
    return t


def _reference(r, textures):
    kind = r.choice(['none', 'none', 'shared', 'own'])
    if kind == 'none':
        index = -1
    else:
        index = r.randrange(textures)
    return ((index & 0x7fff) << 1) | int(kind == 'own') | (r.getrandbits(16) << 16)


def _extra(ctx, r, textures):
    extra = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(extra + 8, 'I', _texture(ctx, r))
    ctx.pack(extra + 0xc, 'I', _reference(r, textures))
    ctx.pack(extra + 0x10, 'I', _texture(ctx, r))
    blocks = 0
    if r.random() < 0.6:
        blocks = ctx.alloc(16 * 0x2c, bytes(r.getrandbits(8) for _ in range(16 * 0x2c)))
        for b in range(16):
            ctx.pack(blocks + 0x2c * b, 'I', _reference(r, textures))
            ctx.pack(blocks + 0x2c * b + 4, 'I', _texture(ctx, r))
    ctx.pack(extra + 0x38, 'I', blocks)
    flags = r.choice([0, 0x10000, r.getrandbits(17), 0xffff])
    reference = ctx.unpack(extra + 0xc, 'I')[0]
    if not blocks and reference & 1 and (reference & 0xfffe) != 0xfffe:
        flags |= 0x10000        # see the docstring: retail's retry would never end
    ctx.pack(extra + 0x3c, 'I', flags)
    return extra


def setup(ctx, seed):
    r = random.Random(seed)
    textures = 8
    detail_distance = r.choice([100.0, 300.0, r.uniform(10.0, 1000.0)])
    ctx.pack(DETAIL_DISTANCE, 'f', detail_distance)
    ctx.pack(OWN_COUNT, 'i', r.randrange(100))
    ctx.data['fresh'] = [_texture(ctx, r) for _ in range(20)]

    camera = ctx.alloc(0x210)
    ctx.pack(camera + 0x200, 'f', r.choice([0.0, 0.5, 0.75, 3.0]))
    host = ctx.alloc(0x10)
    ctx.pack(host + 8, 'I', camera)
    terrain = ctx.alloc(0xd00, bytes(r.getrandbits(8) for _ in range(0xd00)))
    ctx.pack(terrain + 0x18, 'I', host)
    focal = r.choice([256, 320, 500, 1000])
    ctx.pack(terrain + 0x70, 'i', focal)
    for i in range(textures):
        ctx.pack(terrain + 0x544 + 4 * i, 'I', _texture(ctx, r))
    ctx.pack(terrain + 0xc24, 'i', r.randrange(16))
    ctx.pack(terrain + 0xc84, 'I', ctx.alloc(0x40))

    def node(parent):
        n = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
        ctx.pack(n + 8, 'I', parent)
        ctx.pack(n + 0x28, 'BBB', r.choice([0, 0, 1, 2]), r.choice([0, 1, 2, 4]), r.randrange(256))
        ctx.pack(n + 0x34, 'I', terrain)
        data = ctx.alloc(0x300, bytes(r.getrandbits(8) for _ in range(0x300)))
        extra = _extra(ctx, r, textures)
        ctx.pack(n + 0x38, 'II', data, extra)
        return n

    parent = node(0) if r.random() < 0.7 else 0
    this = node(parent)

    # Distances around each stage threshold: detail = (16 << shift) * focal / distance.
    shift = ctx.unpack(this + 0x29, 'B')[0]
    size = (16 << shift) * focal
    thresholds = [detail_distance * k for k in (0.0625, 0.25, 1.0, 4.0)]
    t = r.choice(thresholds)
    distance = r.choice([0.0, size / t, size / t * r.uniform(0.9, 1.1), r.uniform(1.0, 1e5)])
    return dict(ecx=this, args=[struct.unpack('<f', struct.pack('<f', distance))[0]])
