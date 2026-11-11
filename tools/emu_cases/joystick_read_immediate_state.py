"""PCJoystickDevice::ReadImmediateState 0x004c3790, polled DirectInput joystick state (emu_diff case).

The candidate is samples/inputdevice/JoystickImmediateState.cpp. Offsets from the retail body:
the IDirectInputDevice2 pointer +0x25c (null returns 0), joystickIndex +0x260, the 32 button
records (0x14 bytes from +0x264: state, the release arm clears +4/+0xc, the press arm +8/+0x10),
the device subtype byte +0x3d (4 = gamepad: no axes fed), axisValues +0x4e4, axisMaximums
+0x58c, directionFlipped (bit 0 of +0x574, axis 1 only) and povValues +0x5d8. The 0x50-byte
DIJOYSTATE (axes at +0, POVs at +0x20, buttons at +0x30) is filled by the stubbed vtable slot 9
GetDeviceState (stdcall: this, 0x50, &state; pop 12), which answers the case's HRESULT (S_OK,
S_FALSE or a failure). Logged stubs: JoystickDevice::FeedAxis 0x00489c00 (axis, value,
maximum) and the ControlInterface's QueueEvent 0x0043cea0 (g_TrackGame 0x0056e26c +0x14;
button, 2, pressed, joystickIndex). Random: the axes (signed, small to huge), the button bytes
(press bit 0x80 plus noise), the previous button states (0/1, now and then other values), the
subtype, the flip flags and the button times.
"""
import random
import struct

RETAIL_VA = 0x004c3790
SYMBOL = '?ReadImmediateState@PCJoystickDevice@@QAEHH@Z'
SOURCE = 'samples/inputdevice/JoystickImmediateState.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x004c37be: 'state failed', 0x004c37d5: 'feed axes', 0x004c38e1: 'release',
          0x004c38fd: 'press', 0x004c3926: 'povs'}


def cases(rng, n):
    for _ in range(n):
        p = rng.choice([0.1, 0.5, 0.9])
        yield dict(seed=rng.randrange(1 << 30), device=rng.random() < 0.95,
                   result=rng.choice([0, 0, 0, 1, 0x80004005, 0x8007001e]), gamepad=rng.random() < 0.3,
                   flags=rng.randrange(256),
                   axes=[rng.choice([rng.randrange(65536), rng.randrange(-(1 << 31), 1 << 31), 0, 32767])
                         for _ in range(6)],
                   buttons=[(0x80 if rng.random() < p else 0) | rng.choice([0, 0, rng.randrange(128)])
                            for _ in range(32)],
                   previous=[rng.choice([0, 1, 0, 1, rng.randrange(-3, 4)]) for _ in range(32)])


def setup(ctx, c):
    r = random.Random(c['seed'])
    joy = ctx.alloc(0x600, bytes(r.randrange(256) for _ in range(0x600)))
    ctx.pack(joy + 0x3d, 'B', 4 if c['gamepad'] else r.choice([1, 2, 3, 5]))
    ctx.pack(joy + 0x260, 'i', r.randrange(4))
    for i, prev in enumerate(c['previous']):
        ctx.pack(joy + 0x264 + 0x14 * i, 'i', prev)
    ctx.pack(joy + 0x58c, '6f', *[r.choice([65535.0, r.uniform(1000, 70000)]) for _ in range(6)])
    ctx.pack(joy + 0x574, 'B', c['flags'])

    def get_state(cx, e, a):
        cx.note('GetDeviceState', a[1])
        if c['result'] < 0x80000000:
            pov = [r.choice([0xffffffff, r.randrange(36000)]) for _ in range(4)]
            cx.write(a[2], struct.pack('<6i2i4I32B', *(c['axes'] + [r.randrange(65536), r.randrange(65536)]
                                                      + pov + c['buttons'])))
        return c['result']
    vt = [0] * 12
    vt[9] = ctx.stub('IDirectInputDevice2::GetDeviceState', pop=12, ret=get_state, log=False)
    dev = ctx.alloc(0x10)
    ctx.pack(dev, 'I', ctx.vtable(vt))
    ctx.pack(joy + 0x25c, 'I', dev if c['device'] else 0)

    ctx.stub_at(0x0043cea0, 'ControlInterface::QueueEvent', pop=16)
    ctx.stub_at(0x00489c00, 'JoystickDevice::FeedAxis', pop=12)
    control = ctx.alloc(0x40)
    game = ctx.alloc(0x40)
    ctx.pack(game + 0x14, 'I', control)
    ctx.pack(TRACK_GAME, 'I', game)
    return dict(ecx=joy, args=[r.randrange(4)], ret_eax=True)
