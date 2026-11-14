"""BuildStatusList 0x004e5d00, the race status list (tools/emu_diff.py case).

The linked body is the Group 6 bring-up copy (src/bringup/Group6_BuildStatusList.cpp), cdecl
(list, view). Offsets from the retail body: view +0x38 local racer, +0x3c racer slots, +0x40
AI racers, +0xc8 gate list, +0x158 racer count; racer +0x0c position, +0x744 status node;
status node 0x54 bytes (+0 creation order, +4 racer, +0x28 position, +0x34 next gate, +0x50
next); g_TrackGame+0x18 player count, +0x2d70 race mode, +0x2d94 AI racer count. Racers are
drawn from a small pool, so the AI and slot arrays repeat each other and the local racer
(HasStatusNode 0x004e5c70 runs as retail code and skips them). DebugCalloc 0x004a2fc0 is a stub
that notes count, size and line (not __FILE__) and fails on a chosen call; a null list and a
null slot array are included.
"""
import struct

RETAIL_VA = 0x004e5d00
SYMBOL = '?BuildStatusList@@YAHPAPAURaceStatusNode@@PAUEventBikeRaceView@@@Z'
SOURCE = 'src/bringup/Group6_BuildStatusList.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x004e5d9e: 'AI loop', 0x004e5dd3: 'AI node', 0x004e5e5e: 'slot loop', 0x004e5e90: 'slot node',
          0x004e5d3b: 'failed'}


def cases(rng, n):
    for _ in range(n):
        pool = rng.randrange(1, 9)
        yield dict(pool=pool, ai=[rng.randrange(pool) for _ in range(rng.randrange(0, 8))],
                   slots=[rng.randrange(pool) for _ in range(rng.randrange(0, 9))], local=rng.randrange(pool),
                   players=rng.choice([1, 1, 2]), race_mode=rng.choice([0, 1, 4]), null_slots=rng.random() < 0.1,
                   null_list=rng.random() < 0.05, fail_at=rng.choice([None] * 8 + [1, 2, 3, 5]),
                   ai_count_extra=rng.choice([0, 0, -1]))


def setup(ctx, c):
    racers = []
    for _ in range(c['pool']):
        r = ctx.alloc(0x800, fill=0x5a)
        ctx.pack(r + 0x0c, '3f', *[ctx.rng.uniform(-100, 100) for _ in range(3)])
        racers.append(r)
    ai = ctx.alloc(4 * max(1, len(c['ai'])))
    if c['ai']:
        ctx.pack(ai, '%dI' % len(c['ai']), *[racers[i] for i in c['ai']])
    slots = ctx.alloc(4 * max(1, len(c['slots'])))
    if c['slots']:
        ctx.pack(slots, '%dI' % len(c['slots']), *[racers[i] for i in c['slots']])
    view = ctx.alloc(0x200, fill=0x6b)
    ctx.pack(view + 0x38, 'III', racers[c['local']], 0 if c['null_slots'] else slots, ai)
    ctx.pack(view + 0xc8, 'I', 0x47a7e000)
    ctx.pack(view + 0x158, 'i', len(c['slots']))
    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x18, 'i', c['players'])
    ctx.pack(game + 0x2d70, 'i', c['race_mode'])
    ctx.pack(game + 0x2d94, 'i', max(0, len(c['ai']) + c['ai_count_extra']))
    ctx.pack(TRACK_GAME, 'I', game)
    count = [0]

    def calloc(cx, ecx, a):
        count[0] += 1
        cx.note('calloc', a[0], a[1], a[3])
        if c['fail_at'] == count[0]:
            return 0
        return cx.alloc(a[0] * a[1])          # calloc: zeroed
    ctx.stub_at(0x004a2fc0, 'DebugCalloc', nargs=4, ret=calloc, log=False)
    lst = ctx.alloc(4, fill=0x77)
    return dict(args=[0 if c['null_list'] else lst, view])
