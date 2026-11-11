"""FollowCamera::FollowCamera(int) 0x00462ee0, the follow camera's constructor (emu_diff case).

The candidate is samples/camera/FollowCameraConstructor.cpp. The object is random bytes. The
base constructor PCCamera::PCCamera 0x004bed80 is a logged stub (thiscall, the flags) that
writes a random fieldOfView +0x16c and eye +0x170, which the body copies; the direct call to
FollowCamera::SetCameraState 0x00466e50 is a logged stub that writes a random state +0x244,
followDistance +0x220, followPitch +0x22c and followYaw +0x234, which the body copies into the
saved/initial fields. The vptr (0x0055282c), the file-scope axes (retail 0x0065b438..; read by
value here) and every field store are compared; the exception frame is pushed and popped
through fs:0 as in retail. Limitation: the axes are uninitialised .bss in both images (zeros
until their dynamic initializer runs), so swapping axis components is not detected here.
"""
import random

RETAIL_VA = 0x00462ee0
SYMBOL = '??0FollowCamera@@QAE@H@Z'
SOURCE = 'samples/camera/FollowCameraConstructor.cpp'


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), flags=rng.choice([0, 1, 2, rng.randrange(1 << 32)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.uc.mem_map(0, 0x1000)
    cam = ctx.alloc(0x400, bytes(r.randrange(256) for _ in range(0x400)))

    def base(cx, e, a):
        cx.pack(e + 0x16c, '4f', *[cx.rng.uniform(-100, 100) for _ in range(4)])
        return e
    ctx.stub_at(0x004bed80, 'PCCamera::PCCamera', pop=4, ret=base)

    def state(cx, e, a):
        cx.pack(e + 0x244, 'i', cx.rng.randrange(8))
        cx.pack(e + 0x220, 'f', cx.rng.uniform(5, 50))
        cx.pack(e + 0x22c, 'f', cx.rng.uniform(-1, 1))
        cx.pack(e + 0x234, 'f', cx.rng.uniform(-3, 3))
    ctx.stub_at(0x00466e50, 'FollowCamera::SetCameraState', pop=4, ret=state)
    return dict(ecx=cam, args=[c['flags']], ret_eax=True)
