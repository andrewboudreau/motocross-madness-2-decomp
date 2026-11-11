"""TerrainShadow slot 28 0x005097d0, the terrain shadow's light matrix (tools/emu_diff.py case).

Clip rectangles (+0x6668..+0x6674) wider and narrower than the shadow texture (the early
0 return), half-texel and full-texel scale (+0x98 of the shadow at +0x30), mode 1 (atan2
field of view from camera +0x198, SetLookAt 0x0042e9b0 stubbed and logging the fov, slot
28 stubbed) and other modes (orthographic, lensScale +0xbc). The camera matrices
(+0xac/+0xec) are random; ShadowMatrixIdentity and ShadowMatrixMultiply run as retail
code. Compared: the texel scale +0x677c, lightMatrix (+0xd8 of the shadow) and eax.
"""
import struct

RETAIL_VA = 0x005097d0
SYMBOL = '?UnknownVirtualSlot28@TerrainShadow@@UAEHXZ'
SOURCE = 'samples/physics/shadow/TerrainShadow.cpp'
PROBES = {0x0050985c: 'mode 1', 0x00509909: 'orthographic'}


def cases(rng, n):
    for _ in range(n):
        size = rng.choice([64, 128, 256])
        x0, z0 = rng.uniform(-500, 500), rng.uniform(-500, 500)
        ex = rng.choice([rng.uniform(1, size), rng.uniform(size, 2 * size), float(size)])
        ez = rng.choice([rng.uniform(1, size), rng.uniform(0, 2 * size)])
        cam = [[rng.uniform(-2, 2) for _ in range(16)] for _ in range(2)]
        yield (size, (x0, x0 + ex, z0, z0 + ez), rng.choice([0, 1]), rng.choice([1, 1, 3, 0]),
               rng.uniform(5, 400), rng.uniform(1, 300), cam)


def setup(ctx, case):
    size, clip, half, mode, lens, cam_dist, cam = case

    def look_at(c, ecx, a):
        c.note('fov', c.read(a[4], 4).hex() if a[4] else None)
    ctx.stub_at(0x0042e9b0, 'Camera::SetLookAt', pop=20, ret=look_at)
    camera_vt = [ctx.stub('Camera::vslot%d' % i) for i in range(28)]
    camera_vt.append(ctx.stub('Camera::UnknownVirtualSlot28'))
    camera = ctx.alloc(0x200)
    ctx.pack(camera, 'I', ctx.vtable(camera_vt))
    ctx.pack(camera + 0xac, '16f', *cam[0])
    ctx.pack(camera + 0xec, '16f', *cam[1])
    ctx.pack(camera + 0x198, 'f', cam_dist)
    texture = ctx.alloc(0x40)
    ctx.pack(texture + 0x14, 'i', size)
    shadow = ctx.alloc(0x140, fill=0x77)
    ctx.pack(shadow + 0x54, 'II', texture, camera)
    ctx.pack(shadow + 0x8c, 'i', mode)
    ctx.pack(shadow + 0x98, 'i', half)
    ctx.pack(shadow + 0xbc, 'f', lens)
    this = ctx.alloc(0x6800)
    ctx.pack(this + 0x30, 'I', shadow)
    ctx.pack(this + 0x6668, '4f', *clip)
    return dict(ecx=this, args=[])
