"""KrustyBike::IsSpeedGainBelowLimit 0x004919a0, slot 70 (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy. Human (+0x734 clear) and AI riders, every
difficulty (1..3) and event type (3 selects the first row of the AI table), speed gains
around the per-mode limit (exact ties, both sides, negative and NaN gains).
"""
import struct

RETAIL_VA = 0x004919a0
SYMBOL = '?IsSpeedGainBelowLimit@KrustyBike@@UAEHM@Z'
SOURCE = 'src/bringup/Group3_KrustyBike.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x004919f5: 'AI arm', 0x00491a44: 'human arm'}

A = [1.15, 1.05, 1.0]
B = [[4.0, 3.25, 2.5], [3.75, 3.25, 2.5]]


def cases(rng, n):
    for _ in range(n):
        ai = rng.choice([0, 1, 2])
        difficulty = rng.choice([1, 2, 3])
        event = rng.choice([0, 1, 2, 3, 3, 4, 5])
        dt = rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1)])
        limit = rng.uniform(0, 50)
        prev = rng.uniform(-30, 30)
        kind = rng.randrange(4)
        if kind == 0:
            gain = rng.uniform(-5, 5)
        else:
            # Near the threshold in the arm that runs.
            row = 1 if event != 3 else 0
            thr = dt * limit / B[row][difficulty - 1] if ai else A[difficulty - 1] * limit * dt
            gain = thr * rng.choice([1.0, 1.0 + 1e-7, 1.0 - 1e-7, 0.999, 1.001])
        if rng.random() < 0.03:
            gain = float('nan')
        yield (ai, difficulty, event, dt, limit, prev, prev + gain)


def setup(ctx, case):
    ai, difficulty, event, dt, limit, prev, speed = case
    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x60c, 'i', difficulty)
    ctx.pack(game + 0x2d74, 'i', event)
    ctx.pack(TRACK_GAME, 'I', game)
    bike = ctx.alloc(0x800, fill=0x5a)
    ctx.pack(bike + 0xb8, 'ff', prev, speed)
    ctx.pack(bike + 0x450, 'f', limit)
    ctx.write(bike + 0x734, struct.pack('<B', ai))
    return dict(ecx=bike, args=[float(dt)])
