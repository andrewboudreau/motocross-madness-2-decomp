"""Game::Tick 0x004685d0, vtable slot 10 (tools/emu_diff.py case, call-record/replay mode).

The linked body is samples/game/GameTick.cpp, a reviewed functional candidate (not a byte
match): it reads the profiling counter through ReadProfilingCounter (QPC) where retail
executes RDTSC. That difference is by design, so the case leaves the counter call out of the
call log (``unlogged``) and the profiling entry's counter-derived words (start stamp, totals,
peak) out of the write comparison. Everything else is compared: the frame clock and its
0.2 s clamp, frameTime, the peak overwrite with the frame duration, the input/network/
root-object/render/present/release/advance call sequence with the delta's float bits, the
net/tick/prep/screen times and the single-step pause.

The candidate's timer registration statics are bound to retail's guard 0x0065b488 and value
0x0065b4b0 (``BINDINGS``); cases start registered or unregistered (an empty meter, a meter
that already has the "Game::Tick" timer, or a full one).
"""
RETAIL_VA = 0x004685d0
SYMBOL = '?Tick@Game@@UAEHXZ'
SOURCE = 'samples/game/GameTick.cpp'
HIRES_METER = 0x0056c470
GUARD = 0x0065b488
TIMER = 0x0065b4b0
# ReadProfilingCounter has no retail body: it is bound to a spare fake code address and
# replayed there (callee-popped 0) without being logged.
PROFILING_COUNTER = 0x2200ff00
BINDINGS = {'_gameTickTimerRegistered': GUARD, '_gameTickTimer': TIMER,
            '?ReadProfilingCounter@@YA_JXZ': PROFILING_COUNTER}
PROBES = {0x0046873e: 'ReadClock frame', 0x00468735: 'ResetClock', 0x0046879e: 'clamp 0.2',
          0x004687af: 'DispatchInput', 0x004687d0: 'network Update', 0x00468864: 'step one frame'}
ENTRY = 0x48


def _clock(ctx, rec):
    # ReadClock: milliseconds, mostly a frame or a few after the previous frame (both sides
    # of the 0.2 s clamp, wrapping past 2^32 when the previous clock is near the top).
    near = (ctx.previous_clock + rec.rng.choice([0, 1, 16, 17, 33, 199, 200, 201, rec.rng.randrange(1000)])) & 0xffffffff
    return near if rec.rng.random() < 0.8 else rec.rng.randrange(1 << 32)


REPLAY = dict(unlogged=(PROFILING_COUNTER,), pops={PROFILING_COUNTER: 0}, passthrough=('_strncat',),
              returns={'?ReadClock@@YAIXZ': _clock})


def cases(rng, n):
    for _ in range(n):
        yield dict(
            registered=rng.choice([1, 1, 1, 0]),
            meter=rng.choice(['empty', 'found', 'full', 'some']),
            timer=rng.randrange(50),
            previous=rng.choice([0, rng.randrange(1, 1 << 32), 0xfffffff0]),
            frame_clock=rng.randrange(1 << 32),
            suspended=rng.choice([0, 0, 1]),
            control=rng.choice([0, 1]),
            network=rng.choice([0, 1]),
            step=rng.choice([0, 0, 1]),
            paused=rng.choice([0, 1]),
            ms_per_tick=rng.uniform(1e-7, 1e-5),
        )


def setup(ctx, c):
    import random
    rng = random.Random(repr(sorted(c.items())))

    meter = ctx.alloc(0x40 + 50 * ENTRY)
    ctx.pack(meter + 0x30, 'f', c['ms_per_tick'])
    count = {'empty': 0, 'full': 50, 'found': 5, 'some': 3}[c['meter']]
    ctx.pack(meter + 0x34, 'ii', count, -1)
    for i in range(50):
        at = meter + 0x40 + i * ENTRY
        ctx.pack(at, 'II7f', rng.getrandbits(32), rng.getrandbits(8),
                 *[rng.uniform(0, 100) for _ in range(7)])
        if i < count:
            name = b'Game::Tick' if (c['meter'] == 'found' and i == 3) else b'timer%d' % i
            ctx.write(at + 0x24, name + b'\0')
    ctx.pack(HIRES_METER, 'I', meter)
    ctx.pack(GUARD, 'B', c['registered'])
    ctx.pack(TIMER, 'i', c['timer'] if c['registered'] else 0)

    game = ctx.alloc(0x300)
    ctx.pack(game, 'I', ctx.fake_vtable('Game', 16))
    root = ctx.alloc(0x40)
    ctx.pack(root, 'I', ctx.fake_vtable('rootObject', 16, pops={10: 4}))   # slot 10 Tick(float)
    ctx.pack(game + 0x08, 'I', ctx.alloc(0x40) if c['network'] else 0)
    ctx.pack(game + 0x10, 'I', ctx.alloc(0x40))
    ctx.pack(game + 0x14, 'I', ctx.alloc(0x40) if c['control'] else 0)
    ctx.pack(game + 0x1c4, 'ii', c['paused'], c['step'])
    ctx.pack(game + 0x2d4, 'BB', 0, c['suspended'])
    ctx.pack(game + 0x2d8, 'II', c['frame_clock'], c['previous'])
    ctx.previous_clock = c['previous']
    ctx.pack(game + 0x2f4, 'I', root)
    # The counter-derived entry words: start stamp, calls..msThisTick totals and the peak
    # (overwritten from the frame clock afterwards, so the final peak is still compared).
    ignore = []
    for i in range(50):
        at = meter + 0x40 + i * ENTRY
        ignore += [(at, 8), (at + 0x0c, 4), (at + 0x14, 4)]
    return dict(ecx=game, args=[], ignore=tuple(ignore))
