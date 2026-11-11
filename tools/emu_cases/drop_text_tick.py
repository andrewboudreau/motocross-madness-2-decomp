"""DropTextOverlay::Tick 0x0051b070 (tools/emu_diff.py case).

GameObject::Tick (0x004693d0) is stubbed. g_TrackGame (0x0056e26c) points at a block whose
frame time (+0x2f0) is the case's. Retail stores the new elapsed time with `fst` and keeps
the unrounded sum for the drop, so drops that land on or next to whole screen lines (the
`- 0.5` then sits on a tie) and tiny dropSeconds (out of int range, infinity at zero) are
included. Writes compared: elapsed (+0x30), visible (+0x34) and textY (+0x44).
"""

RETAIL_VA = 0x0051b070
SYMBOL = '?Tick@DropTextOverlay@@UAEHM@Z'
SOURCE = 'src/bringup/Group4_DropTextOverlayTick.cpp'
PROBES = {0x0051b0c3: 'drop fistp'}


def cases(rng, n):
    for _ in range(n):
        height = rng.choice([480, 600, 384, 768, rng.randrange(1, 2000)])
        seconds = rng.choice([rng.uniform(0.2, 4.0), 1.0, 2.0, 0.5, 3.0, 1e-3, 1e-9, 0.0, -1.0])
        frame = rng.choice([1 / 30, 1 / 60, 0.05, rng.uniform(0, 0.1)])
        kind = rng.randrange(3)
        if kind == 0:
            elapsed = rng.uniform(0, 5)
        else:
            # elapsed + frame lands near drop lines k (or k + 0.5)
            k = rng.randrange(0, height + 2) + (0.5 if kind == 2 else 0.0)
            elapsed = k * seconds / height - frame if height else 0.0
            elapsed += rng.choice([0.0, 1e-7, -1e-7, 1e-5])
        visible = rng.choice([0, 1, 1, 1])
        y_offset = rng.choice([0, -200, 40, rng.randrange(-1000, 1000)])
        yield (height, seconds, frame, elapsed, visible, y_offset)


def setup(ctx, case):
    height, seconds, frame, elapsed, visible, y_offset = case
    ctx.stub_at(0x004693d0, 'GameObject::Tick', pop=4)
    game = ctx.alloc(0x400, fill=0x11)
    ctx.pack(game + 0x2f0, 'f', frame)
    ctx.pack(0x0056e26c, 'I', game)
    target = ctx.alloc(0x40, fill=0x22)
    ctx.pack(target + 0x10, 'i', height)
    overlay = ctx.alloc(0x80, fill=0x44)
    ctx.pack(overlay + 0x18, 'I', target)
    ctx.pack(overlay + 0x30, 'f', elapsed)
    ctx.pack(overlay + 0x38, 'f', seconds)
    ctx.pack(overlay + 0x34, 'I', visible)
    ctx.pack(overlay + 0x40, 'i', y_offset)
    return dict(ecx=overlay, args=[0.25])
