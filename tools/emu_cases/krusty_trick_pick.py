"""KrustyBike::Fn_0048D780 0x0048d780, the AI trick pick (tools/emu_diff.py case).

rand (0x00534c1b) is stubbed to return the case's draws, so both bodies see the same values.
Draws include products that land exactly on .5 after the `- 0.5` (ties to even) and values
next to them. The function writes *out and, for an AI bike (byte +0x734), the hold time at
+0x152c.
"""

RETAIL_VA = 0x0048d780
SYMBOL = '?Fn_0048D780@KrustyBike@@QAEHPAHMH@Z'
SOURCE = 'samples/physics/bikeai/KrustyBikeTrickPick.cpp'
PROBES = {0x0048d834: 'index fistp helper'}

MIN_TIMES = [2.4000001, 2.3333001, 2.2665999, 2.1333001, 1.9375, 1.8, 1.7647001, 1.7222,
             1.6667, 1.6111, 1.5, 1.3]
COUNTS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 15, 16]


def draw(rng):
    kind = rng.randrange(4)
    if kind == 0:
        return rng.randrange(32768)
    count = rng.choice(COUNTS)
    # rand * count / 32768 an integer or half integer, +-1.
    k = rng.randrange(2 * count + 1)
    r = round(k * 32768 / (2 * count)) + rng.choice([-1, 0, 0, 1])
    return min(max(r, 0), 32767)


def cases(rng, n):
    for _ in range(n):
        base = rng.choice(MIN_TIMES + [3.0, 1.0, 1.2])
        t = base + rng.choice([0.0, 1e-6, -1e-6, rng.uniform(-0.3, 0.3), rng.uniform(0, 3)])
        group = rng.choice([0, 1, 2, 2, 3, -1])
        yield (t, group, rng.randrange(2) * rng.choice([1, 2, 255]), [draw(rng), draw(rng)])


def setup(ctx, case):
    t, group, ai_kind, draws = case
    queue = list(draws)

    def rand(c, ecx, args):
        return queue.pop(0) if queue else 0
    ctx.stub_at(0x00534c1b, 'rand', ret=rand)
    bike = ctx.alloc(0x1600, fill=0x33)
    ctx.pack(bike + 0x734, 'B', ai_kind)
    out = ctx.alloc(4, fill=0x77)
    return dict(ecx=bike, args=[out, t, group])
