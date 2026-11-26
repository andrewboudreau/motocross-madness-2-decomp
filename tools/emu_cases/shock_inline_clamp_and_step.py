"""InlineShock::ClampAndStep 0x004fa090 (tools/emu_diff.py case).

The canonical body is src/krusty2/suspension/Shock.cpp. Offsets are retail's (0x004fa090 disassembly): scene node +0x44,
atLimit +0x50, extending +0x54, ratio +0x70, displacement +0x74, step displacement +0x80,
previous length +0x8c, length +0x98, velocity +0xb0, axis +0xc0, minimum length +0xcc.

Random: length below, at and above the minimum, zero length, zero minimum, the previous length
above and below (the clamp to +-maxDelta both ways), dt and maxDelta; the position argument
is written only at the limit. Stubbed: SetPosition 0x004fc660 (position bytes noted).
"""
import random

RETAIL_VA = 0x004fa090
SYMBOL = '?ClampAndStep@InlineShock@@QAEXMMAAUShockVec3@@@Z'
SOURCE = 'src/krusty2/suspension/Shock.cpp'
PROBES = {0x004fa0b1: 'at limit', 0x004fa27b: 'zero length'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    s = ctx.alloc(0xe0, fill=0x3c)
    node = ctx.alloc(0x40, fill=0)
    ctx.pack(s + 0x44, 'I', node)
    ctx.pack(s + 0x50, 'ii', r.choice([0, 1]), r.choice([0, 1]))
    ctx.pack(s + 0x74, '3f', *[r.uniform(-1, 1) for _ in range(3)])
    minimum = r.choice([r.uniform(0.1, 1.0), 0.5, 0.0])
    kind = r.randrange(5)
    length = [r.uniform(0, minimum), minimum, minimum + r.uniform(0, 0.5), 0.0, r.uniform(0, 1)][kind]
    ctx.pack(s + 0x8c, 'f', r.choice([length, r.uniform(0, 1.5), length + r.uniform(-0.01, 0.01)]))
    ctx.pack(s + 0x98, 'f', length)
    ctx.pack(s + 0xc0, '4f', r.uniform(-1, 1), r.uniform(-1, 1), r.uniform(-1, 1), minimum)
    pos = ctx.alloc(12)
    ctx.pack(pos, '3f', *[r.uniform(-5, 5) for _ in range(3)])

    def set_position(cx, ecx, a):
        cx.note('SetPosition', ecx, cx.read(a[0], 12).hex())
        return 0

    ctx.stub_at(0x004fc660, 'SetPosition', pop=4, ret=set_position, log=False)
    md = r.choice([r.uniform(0, 0.2), 0.0, 1e9])
    dt = r.choice([1 / 60.0, r.uniform(0.001, 0.1)])
    return dict(ecx=s, args=[md, dt, pos], ret_eax=False)
