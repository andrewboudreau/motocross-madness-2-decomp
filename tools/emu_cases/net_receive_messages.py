"""ReceiveMessages 0x004af8e0, the DirectPlay receive loop (tools/emu_diff.py case).

Linked from samples/net/NetThreadNearMisses.cpp. g_TrackGame (0x0056e26c) +8 is the network
interface (null in some runs), its +4 the IDirectPlay4A (null in some runs) and +0x48 the
critical section. The receive buffer and its size are retail's statics 0x006886d8 and
0x006886dc (bound), seeded with a buffer or null and a random size.

Stubbed and noted by their stack arguments (ECX is scratch at these calls, so it is not
compared): Enter/LeaveCriticalSection (their IAT slots 0x0055025c/0x00550284 point
at stubs), IDirectPlay4A::Receive (vtable +0x64, stdcall, 5 arguments) which plays a random
script per run: DPERR_BUFFERTOOSMALL with a new size, messages of 0..40 bytes copied into
the passed buffer with random from/to ids (a message larger than the offered size, or a
null buffer, gets DPERR_BUFFERTOOSMALL and is delivered on the next call, as DirectPlay
does), positive results, and a final error;
DebugMalloc 0x004a2e20 (returns a fresh block or null) and DebugFree 0x004a2e60, noted by
size, pointer and line only (the __FILE__ pointers differ by build); HandleReceivedMessage
0x004afa90 (cdecl, bound for the candidate too), noted with the message bytes it is given.
"""
import random
import struct

RETAIL_VA = 0x004af8e0
SYMBOL = '?ReceiveMessages@@YAJI@Z'
SOURCE = 'samples/net/NetThreadNearMisses.cpp'
TRACK_GAME = 0x0056e26c
RECEIVE_BUFFER, RECEIVE_SIZE = 0x006886d8, 0x006886dc
ENTER_CS_IAT, LEAVE_CS_IAT = 0x0055025c, 0x00550284
BINDINGS = {
    '_s_ReceiveBuffer': RECEIVE_BUFFER,
    '_g_UnknownGlobal6886dc': RECEIVE_SIZE,
    '?HandleReceivedMessage@@YAXPAEIHHI@Z': 0x004afa90,
}
PROBES = {0x004af93b: 'invalid interface', 0x004af99b: 'free old buffer',
          0x004af9d4: 'regrown', 0x004afa06: 'handle message', 0x004afa13: 'leave loop'}
BUFFER_TOO_SMALL = 0x8877001e


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    game = ctx.alloc(0x20, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    net = 0
    if r.random() > 0.05:
        net = ctx.alloc(0x200, fill=0x4e)
    ctx.pack(game + 8, 'I', net)
    capacity = r.choice([0, 0, 16, 64])
    buf = ctx.alloc(max(capacity, 1), fill=0x99) if capacity or r.random() < 0.3 else 0
    ctx.pack(RECEIVE_BUFFER, 'II', buf, capacity)

    script = []
    for _ in range(r.randrange(0, 8)):
        k = r.random()
        if k < 0.2:
            script.append(('small', r.choice([4, 32, 64, 200])))
        elif k < 0.3:
            script.append(('ok', r.randrange(1, 100)))
        else:
            script.append(('msg', r.choice([0, 1, 3, 4, 5, 12, 40]), r.randrange(1 << 32),
                           r.randrange(1 << 32), r.choice([0, 0, 1, 0x7fffffff])))
    script.append(('end', r.choice([0x887700be, 0x80004005, 0x88770082])))
    state = dict(step=0)

    def receive(cx, ecx, a):        # (this, from*, to*, flags, buffer, size*); ECX is scratch
        offered = cx.unpack(a[5], 'I')[0]
        cx.note('Receive', a[0], a[3], a[4], offered)
        step = script[min(state['step'], len(script) - 1)]
        state['step'] += 1
        size_at = a[5]
        if step[0] == 'small':
            cx.pack(size_at, 'I', step[1])
            return BUFFER_TOO_SMALL
        if step[0] == 'end':
            return step[1]
        if step[0] == 'ok':
            return 0
        _, size, src, dst, result = step
        if size and (not a[4] or size > offered):
            state['step'] -= 1          # DirectPlay keeps the message and reports its size
            cx.pack(size_at, 'I', size)
            return BUFFER_TOO_SMALL
        cx.pack(a[1], 'II', src, 0)
        cx.pack(a[2], 'I', dst)
        if a[4] and size:
            cx.write(a[4], bytes(r.randrange(256) for _ in range(size)))
        cx.pack(size_at, 'I', size)
        return result

    def malloc(cx, ecx, a):
        p = 0 if r.random() < 0.1 else cx.alloc(max(a[0], 1), fill=0x55)
        cx.note('DebugMalloc', a[0], a[2], p)
        return p

    def free(cx, ecx, a):
        cx.note('DebugFree', a[0], a[2])
        return 0

    def handle(cx, ecx, a):         # (data, size, from, to, time)
        cx.note('HandleReceivedMessage', *a)
        cx.note('message', cx.read(a[0], min(a[1], 64)).hex() if a[0] else None)
        return 0

    if net:
        directplay = 0
        if r.random() > 0.05:
            directplay = ctx.alloc(0x10, fill=0x44)
            vtable = ctx.alloc(0x100, fill=0)
            ctx.pack(directplay, 'I', vtable)
            ctx.pack(vtable + 0x64, 'I', ctx.stub('Receive', pop=24, ret=receive, log=False))
        ctx.pack(net + 4, 'I', directplay)
    ctx.pack(ENTER_CS_IAT, 'I', ctx.stub('EnterCriticalSection', pop=4, ret=lambda cx, ecx, a: cx.note('Enter', *a), log=False))
    ctx.pack(LEAVE_CS_IAT, 'I', ctx.stub('LeaveCriticalSection', pop=4, ret=lambda cx, ecx, a: cx.note('Leave', *a), log=False))
    ctx.stub_at(0x00534aaf, 'sprintf', nargs=3, ret=lambda cx, ecx, a: cx.note('sprintf', *a[1:]), log=False)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(0x004a2e60, 'DebugFree', nargs=3, ret=free, log=False)
    ctx.stub_at(0x004afa90, 'HandleReceivedMessage', nargs=5, ret=handle, log=False)
    return dict(args=[r.randrange(1 << 32)])
