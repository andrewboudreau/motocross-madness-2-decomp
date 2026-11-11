"""FollowCamera::ProcessCameraInput 0x00463a30, slot 45: keyboard/joystick camera controls.

The candidate is samples/camera/FollowCameraControls.cpp. The camera block is random floats;
offsets from the retail body: the view matrix's first column +0xac/+0xbc/+0xcc, eye +0x170,
view direction +0x17c, followDistance +0x220 (around the 7 and 50 thresholds), followPitch
+0x22c (near the +-1.55 clamp), followYaw +0x234 (near +-pi), cameraState +0x244, overrideActive
+0x268, isMovingTarget +0x26c, isSettled +0x274, cachedTarget +0x2a8 and targetDistance +0x2c0
(around the 0 and 6.5 thresholds). g_TrackGame (0x0056e26c) +0x14 is the ControlInterface:
slot 3 IsDeviceButtonDown(control, device, mask, 0) is a logged stub answering from the case's
pressed-key set (keys 0x48/0x50/0x4b/0x4d/0x4a/0x0c/0x4e/0x0d on device 0, buttons 2/3 on
device 2); its active joystick +0x0c (present or null) has slot 4 (index, &x, &y), a logged
stub returning the case's flag and axes (zeros included). The compared state is the camera
fields, the key queries and the returned flag.
"""
import random
import struct

RETAIL_VA = 0x00463a30
SYMBOL = '?ProcessCameraInput@FollowCamera@@UAE_N_NM@Z'
SOURCE = 'samples/camera/FollowCameraControls.cpp'
TRACK_GAME = 0x0056e26c
KEYS = [0x48, 0x50, 0x4b, 0x4d, 0x4a, 0x0c, 0x4e, 0x0d]
PROBES = {0x00463a63: 'move target', 0x00463d1b: 'target joystick', 0x00463ee6: 'not target',
          0x004641dc: 'state 5 joystick', 0x004647a6: 'orbit joystick', 0x004649f7: 'inactive'}


def cases(rng, n):
    for _ in range(n):
        p = rng.choice([0.1, 0.4, 0.8])
        yield dict(seed=rng.randrange(1 << 30), active=rng.random() < 0.9,
                   override=rng.random() < 0.3, moving=rng.randrange(2),
                   state=rng.choice([0, 1, 2, 3, 4, 5, 5, 6, 7]),
                   keys=[k for k in KEYS if rng.random() < p], buttons=[b for b in (2, 3) if rng.random() < p],
                   joystick=rng.random() < 0.6, stick=rng.random() < 0.6,
                   axes=rng.choice([(0.0, 0.0), (0.0, rng.uniform(-1, 1)), (rng.uniform(-1, 1), 0.0),
                                    (rng.uniform(-1, 1), rng.uniform(-1, 1))]),
                   distance=rng.choice([7.0, 7.1, 49.9, 50.0, rng.uniform(0, 100)]),
                   pitch=rng.choice([1.54, -1.54, rng.uniform(-1.6, 1.6)]),
                   yaw=rng.choice([3.14, -3.14, rng.uniform(-3.2, 3.2)]),
                   target=rng.choice([0.0, 6.5, -1.0, rng.uniform(0, 20)]),
                   dt=rng.choice([0.016, 0.05, rng.uniform(0, 0.5)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    cam = ctx.alloc(0x400)
    ctx.write(cam, struct.pack('<256f', *[r.uniform(-50, 50) for _ in range(256)]))
    ctx.pack(cam + 0x220, 'f', c['distance'])
    ctx.pack(cam + 0x22c, 'f', c['pitch'])
    ctx.pack(cam + 0x234, 'f', c['yaw'])
    ctx.pack(cam + 0x244, 'i', c['state'])
    ctx.pack(cam + 0x268, 'II', int(c['override']), c['moving'])
    ctx.pack(cam + 0x274, 'B', r.randrange(2))
    ctx.pack(cam + 0x2c0, 'f', c['target'])

    def button(cx, e, a):
        if a[1] == 0:
            return int(a[0] in c['keys'])
        return int(a[0] in c['buttons'])

    def stick(cx, e, a):
        if c['stick']:
            cx.pack(a[1], 'f', c['axes'][0])
            cx.pack(a[2], 'f', c['axes'][1])
        return int(c['stick'])

    control = ctx.alloc(0x20)
    ctx.pack(control, 'I', ctx.vtable([0, 0, 0, ctx.stub('ControlInterface::IsDeviceButtonDown', pop=16, ret=button)]))
    if c['joystick']:
        joystick = ctx.alloc(0x10)
        ctx.pack(joystick, 'I', ctx.vtable([0, 0, 0, 0, ctx.stub('Joystick::UnknownVirtualSlot4', pop=12, ret=stick)]))
        ctx.pack(control + 0xc, 'I', joystick)
    game = ctx.alloc(0x200)
    ctx.pack(game + 0x14, 'I', control)
    ctx.pack(TRACK_GAME, 'I', game)
    return dict(ecx=cam, args=[int(c['active']), c['dt']], ret_eax=True)
