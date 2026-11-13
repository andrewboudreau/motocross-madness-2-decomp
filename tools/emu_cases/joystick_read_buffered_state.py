"""PCJoystickDevice::ReadBufferedState 0x004c3100, buffered DirectInput joystick events (emu_diff case).

The candidate is samples/inputdevice/JoystickBufferedState.cpp. Offsets from the retail body:
the IDirectInputDevice2 pointer +0x25c (null returns 0 at once), joystickIndex +0x260, the
button records (0x14 bytes from +0x264: state, then the two time pairs the press/release arms
shift at +0x26c/+0x274 and +0x268/+0x270), the device subtype byte +0x3d (4 = gamepad, raw axes
stored by fild qword), axisValues +0x4e4, axisMaximums +0x58c, previousRawAxes +0x5a4,
deadZoneScales +0x5bc (read by ApplyAxisDeadZone 0x004c3090), the spike-filter byte +0x5d5,
directionFlipped (bit 0 of +0x574, axis 1 only) and povValues +0x5d8.

Stubbed (COM and the queue): the device's vtable slot 10 GetDeviceData (stdcall, pop 20) copies
the case's events (16-byte records: offset, data, timeStamp, sequence) and count, answering the
case's HRESULTs in turn (S_OK, DI_BUFFEROVERFLOW, the 'input lost' 0x8007001e / 0x8007000c that
trigger slot 7 Acquire (pop 4, case result) and a re-read, or E_FAIL); the error reporter
0x004bf6a0 is logged without its __FILE__ argument (the sample's own path); the
ControlInterface's QueueEvent 0x0043cea0 (g_TrackGame 0x0056e26c +0x14; button, 2, pressed,
joystickIndex) and JoystickDevice::FeedAxis 0x00489c00 (axis, value, maximum) are logged.
ButtonIndexFromOffset 0x004c2ef0, ApplyAxisDeadZone and __ftol run as retail code. Random:
the event offsets (buttons 0x30..0x4f, the six axes, the four POVs, misaligned and unknown
offsets), their data (press bit 0x80, axis values near the current ones or >10000 away, huge
unsigned values, POV centre 0xffffffff), the subtype, the filter/flip flags, the axis state
(previous raw equal to the current half the time, so the spike filter both holds and passes)
and the button records.
"""
import random
import struct

RETAIL_VA = 0x004c3100
SYMBOL = '?ReadBufferedState@PCJoystickDevice@@QAEHH@Z'
SOURCE = 'samples/inputdevice/JoystickBufferedState.cpp'
TRACK_GAME = 0x0056e26c
LOST = (0x8007001e, 0x8007000c)
PROBES = {0x004c3150: 'input lost', 0x004c3160: 'acquire failed', 0x004c31ec: 'button press',
          0x004c322c: 'button release', 0x004c3292: 'gamepad axis 0', 0x004c3373: 'axis 0',
          0x004c3399: 'spike check', 0x004c3456: 'axis 1 flip test', 0x004c3684: 'feed axis',
          0x004c36a5: 'pov 0', 0x004c36e5: 'done'}


def event(rng, axes):
    k = rng.random()
    if k < 0.3:
        off = 0x30 + rng.randrange(32)
        data = rng.randrange(256) | (rng.randrange(1 << 24) << 8 if rng.random() < 0.2 else 0)
    elif k < 0.75:
        a = rng.randrange(6)
        off = 4 * a
        cur = int(axes[a])
        data = rng.choice([cur + rng.randrange(-200, 200), cur + rng.choice([-1, 1]) * rng.randrange(9000, 40000),
                           rng.randrange(65536), rng.choice([0, 65535, 0xffffffff, 0x80000000])]) & 0xffffffff
    elif k < 0.9:
        off = rng.choice([0x20, 0x24, 0x28, 0x2c])
        data = rng.choice([0xffffffff, rng.randrange(36000)])
    else:
        off = rng.choice([1, 2, 6, 0x13, 0x15, 0x18, 0x1c, 0x22, 0x2d, 0x50, 0x60, 0x400])
        data = rng.randrange(1 << 32)
    return off, data, rng.randrange(1 << 32), rng.randrange(1 << 16)


def cases(rng, n):
    for _ in range(n):
        axes = [rng.choice([float(rng.randrange(65536)), rng.uniform(0, 65535)]) for _ in range(6)]
        first = rng.choice([0, 0, 0, 1, LOST[0], LOST[1], 0x80004005])
        events = [event(rng, axes) for _ in range(rng.choice([0, 1, 4, 16]))]
        yield dict(seed=rng.randrange(1 << 30), device=rng.random() < 0.95, first=first,
                   acquire=rng.choice([0, 1, 0x80070005]), second=rng.choice([0, 0, 0x80004005]),
                   events=events, count=len(events) if rng.random() < 0.9 else rng.randrange(len(events) + 1),
                   gamepad=rng.random() < 0.3, filter=rng.randrange(2), flags=rng.randrange(256),
                   axes=axes, same=[rng.random() < 0.5 for _ in range(6)])


def setup(ctx, c):
    r = random.Random(c['seed'])
    joy = ctx.alloc(0x600, bytes(r.randrange(256) for _ in range(0x600)))
    ctx.pack(joy + 0x3d, 'B', 4 if c['gamepad'] else r.choice([1, 2, 3, 5]))
    ctx.pack(joy + 0x260, 'i', r.randrange(4))
    ctx.pack(joy + 0x4e4, '6f', *c['axes'])
    ctx.pack(joy + 0x58c, '6f', *[r.choice([65535.0, r.uniform(1000, 70000)]) for _ in range(6)])
    prev = [a if s else float(r.randrange(65536)) for a, s in zip(c['axes'], c['same'])]
    ctx.pack(joy + 0x5a4, '6f', *prev)
    ctx.pack(joy + 0x5bc, '6f', *[r.uniform(0.5, 2.0) for _ in range(6)])
    ctx.pack(joy + 0x574, 'B', c['flags'])
    ctx.pack(joy + 0x5d5, 'B', c['filter'])

    results = [c['first'], c['second']]
    calls = [0]

    def get_data(cx, e, a):
        cx.note('GetDeviceData', a[1], a[4], cx.unpack(a[3], 'i')[0])
        res = results[min(calls[0], 1)]
        calls[0] += 1
        if res < 0x80000000:
            data = b''.join(struct.pack('<4I', *ev) for ev in c['events'])
            cx.write(a[2], data)
            cx.pack(a[3], 'i', c['count'])
        return res
    vt = [0] * 12
    vt[10] = ctx.stub('IDirectInputDevice2::GetDeviceData', pop=20, ret=get_data, log=False)
    vt[7] = ctx.stub('IDirectInputDevice2::Acquire', pop=4, ret=c['acquire'])
    dev = ctx.alloc(0x10)
    ctx.pack(dev, 'I', ctx.vtable(vt))
    ctx.pack(joy + 0x25c, 'I', dev if c['device'] else 0)

    def report(cx, e, a):
        cx.note('ReportError', a[0], a[2])
    ctx.stub_at(0x004bf6a0, 'ReportDirectInputError', pop=0, nargs=3, ret=report, log=False)
    ctx.stub_at(0x0043cea0, 'ControlInterface::QueueEvent', pop=16)
    ctx.stub_at(0x00489c00, 'JoystickDevice::FeedAxis', pop=12)
    control = ctx.alloc(0x40)
    game = ctx.alloc(0x40)
    ctx.pack(game + 0x14, 'I', control)
    ctx.pack(TRACK_GAME, 'I', game)
    return dict(ecx=joy, args=[r.randrange(4)], ret_eax=True)
