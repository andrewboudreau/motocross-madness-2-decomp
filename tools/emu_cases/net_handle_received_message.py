"""HandleReceivedMessage 0x004afa90, the network message dispatcher (tools/emu_diff.py case).

Linked from src/reconstructed/NetThread.cpp. The network interface (g_TrackGame
0x0056e26c +8) is a fresh block with the offsets retail reads: localPlayer +0x0c, isHost
+0x10, connectionMode +0x18 (3 = routed through the host in some runs), the message ring
+0x34 (0x8f4-byte records: type +0, from +4, size +0x10, data +0x14), ringFull +0x38,
nextMessage +0x40, the keep-alive lock +0x60, heldMessages +0x100 {count, data, grow,
capacity} (its inline Add regrows through operator new/delete) and files +0x114 {count,
data}, whose NetFile entries carry sourceId at +0x110. Retail's statics s_DuplicateToggle
0x006886e0 and the duplicate filter switch 0x006886e4 are bound and seeded.

Messages: system messages (from 0) with DPSYS_HOST (0x101) or another dword, and player
messages whose first byte is a random type or 0x4b keep-alive, 0xcc stop keep-alive,
0xcd/0xce/0xcf file transfer (each with the 0x80 acknowledgement bit) or a 0x20 routed
type; froms that match some files' sourceId; targets 0, local or other.

Stubbed (thiscall stubs log ECX and arguments; cdecl and import stubs note their arguments
only, ECX being scratch there): IsSameMessage 0x004afa40 (random answer),
StripMessageTrailer 0x004abd40 (writes guaranteed/sequence/target, may advance the data
pointer and shrink the size), operator new 0x004a3010 (size and line noted; the
__FILE__ pointers differ by build; null in some runs), NetIncomingMessage 0x004aad50 and
NetFile 0x004ab0f0 constructors, SetReceived 0x004aad60, operator delete 0x004a30c0,
~NetFile 0x004ab160, SendRaw 0x004ad280 (the acknowledgement's sequence half is noted;
the other half is never written by either body), Enter/LeaveCriticalSection IAT slots
0x0055025c/0x00550284, FindPlayer 0x004ac800, ReadClock 0x004bfa80, StopKeepAlive
0x004ac950, BeginReceive 0x004ab280, ReceiveChunk 0x004ab380 and QueueMessage 0x004acb00.
Page 0 is mapped for the SEH registration at fs:[0].
"""
import random
import struct

RETAIL_VA = 0x004afa90
SYMBOL = '?HandleReceivedMessage@@YAXPAEIHHI@Z'
SOURCE = 'src/reconstructed/NetThread.cpp'
TRACK_GAME = 0x0056e26c
DUPLICATE_TOGGLE, DUPLICATE_FILTER = 0x006886e0, 0x006886e4
ENTER_CS_IAT, LEAVE_CS_IAT = 0x0055025c, 0x00550284
BINDINGS = {'_s_DuplicateToggle': DUPLICATE_TOGGLE, '_g_UnknownGlobal6886e4': DUPLICATE_FILTER}
PROBES = {0x004afb40: 'duplicate', 0x004afbb1: 'strip trailer', 0x004afc10: 'hold for routing',
          0x004afc83: 'held list grows', 0x004afd0a: 'DPSYS_HOST', 0x004afd46: 'acknowledge',
          0x004afd74: 'keep-alive', 0x004afdce: 'stop keep-alive', 0x004afe22: 'file begin',
          0x004afe78: 'file chunk', 0x004aff16: 'file removed', 0x004aff5c: 'queued'}
MESSAGE_SIZE = 0x8f4


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    ctx.uc.mem_map(0, 0x1000)
    local = r.randrange(1, 1 << 31)
    game = ctx.alloc(0x20, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    net = ctx.alloc(0x200, fill=0)
    ctx.pack(game + 8, 'I', net)
    ctx.pack(net + 0x0c, 'iii', local, r.choice([0, 1, 1]), r.randrange(3))
    ctx.pack(net + 0x18, 'i', r.choice([3, 3, 1, 0]))
    ring = ctx.alloc(256 * MESSAGE_SIZE)
    ctx.pack(net + 0x34, 'Ii', ring, r.choice([0, 0, 1]))
    next_message = r.choice([0, 0, 1, 5, 255])
    ctx.pack(net + 0x40, 'i', next_message)
    for k in {next_message - 1, 0xff}:
        if k >= 0:
            ctx.pack(ring + k * MESSAGE_SIZE, 'iiiii', r.randrange(256), r.randrange(1 << 31),
                     r.randrange(1 << 31), r.randrange(1 << 31), r.randrange(64))
    held_count = r.randrange(0, 5)
    held_capacity = held_count + r.choice([0, 0, 0, 1, 3])
    held = ctx.alloc(4 * max(held_capacity, 1), struct.pack('<%dI' % held_capacity,
                                                            *[r.randrange(1 << 32) for _ in range(held_capacity)]))
    ctx.pack(net + 0x100, 'iIii', held_count, held, r.choice([1, 4, 8]), held_capacity)

    from_ = 0 if r.random() < 0.15 else r.randrange(1, 1 << 31)
    files = []
    for _ in range(r.choice([0, 1, 2, 4])):
        f = ctx.alloc(0x120, fill=0x46)
        ctx.pack(f + 0x110, 'i', from_ if r.random() < 0.5 else r.randrange(1 << 31))
        files.append(f)
    ctx.pack(net + 0x114, 'iI', len(files), ctx.alloc(4 * max(len(files), 1),
                                                      struct.pack('<%dI' % len(files), *files)))

    size = r.choice([8, 12, 32, 64])
    payload = bytearray(r.randrange(256) for _ in range(size + 8))
    if from_ == 0:
        struct.pack_into('<I', payload, 0, r.choice([0x101, 0x101, 5, r.randrange(1 << 32)]))
    else:
        payload[0] = r.choice([0x4b, 0xcc, 0xcc, 0xcc, 0xcd, 0xce, 0xcf, 0x20, 0x60, 0xa0, 0x21, 0x3f, r.randrange(256)])
        if r.random() < (0.8 if payload[0] == 0xcc else 0.3):
            struct.pack_into('<i', payload, 4, local)
    data = ctx.alloc(len(payload), bytes(payload))
    to = r.choice([0, local, r.randrange(1, 1 << 31)])
    ctx.pack(DUPLICATE_TOGGLE, 'ii', r.choice([0, 1]), r.choice([0, 1, 1]))

    def note(name):
        return lambda cx, ecx, a: cx.note(name, *a)

    def same(cx, ecx, a):
        cx.note('IsSameMessage', *a)
        return r.choice([0, 1])

    def strip(cx, ecx, a):              # (&data, &size, &guaranteed, &sequence, &target)
        d, s = cx.unpack(a[0], 'I')[0], cx.unpack(a[1], 'I')[0]
        cx.note('Strip', d, s)
        if r.random() < 0.5:
            cx.pack(a[0], 'I', d + 4)
            cx.pack(a[1], 'I', s - 4)
        cx.pack(a[2], 'i', r.choice([0, 1]))
        cx.pack(a[3], 'i', r.choice([0, r.randrange(1, 1 << 31)]))
        cx.pack(a[4], 'i', r.choice([0, local, r.randrange(1, 1 << 31)]))
        return 0

    def new(cx, ecx, a):                # (size, __FILE__, line)
        p = 0 if r.random() < 0.1 else cx.alloc(max(a[0], 1), fill=0x6e)
        cx.note('new', a[0], a[2], p)
        return p

    def send_raw(cx, ecx, a):           # (0x40, short ack[2], 4, sequence, local, from, 0, 0)
        cx.note('ack', cx.read(a[1] + 2, 2).hex())
        return 0

    def find_player(cx, ecx, a):
        return cx.alloc(0x40, fill=0x50) if r.random() < 0.7 else 0

    ctx.pack(ENTER_CS_IAT, 'I', ctx.stub('EnterCriticalSection', pop=4, ret=note('Enter'), log=False))
    ctx.pack(LEAVE_CS_IAT, 'I', ctx.stub('LeaveCriticalSection', pop=4, ret=note('Leave'), log=False))
    ctx.stub_at(0x004afa40, 'IsSameMessage', nargs=8, ret=same, log=False)
    ctx.stub_at(0x004abd40, 'StripMessageTrailer', pop=20, ret=strip)
    ctx.stub_at(0x004a3010, 'new', nargs=3, ret=new, log=False)
    ctx.stub_at(0x004a30c0, 'delete', nargs=1, ret=note('delete'), log=False)
    ctx.stub_at(0x004aad50, 'NetIncomingMessage', ret=lambda cx, ecx, a: ecx)
    ctx.stub_at(0x004aad60, 'SetReceived', pop=16)
    ctx.stub_at(0x004ab0f0, 'NetFile', ret=lambda cx, ecx, a: ecx)
    ctx.stub_at(0x004ab160, '~NetFile')
    ctx.stub_at(0x004ad280, 'SendRaw', pop=32, ret=send_raw)
    ctx.stub_at(0x004ac800, 'FindPlayer', pop=4, ret=find_player)
    ctx.stub_at(0x004bfa80, 'ReadClock', ret=lambda cx, ecx, a: r.randrange(1 << 32), log=False)
    ctx.stub_at(0x004ac950, 'StopKeepAlive')
    ctx.stub_at(0x004ab280, 'BeginReceive', pop=8)
    ctx.stub_at(0x004ab380, 'ReceiveChunk', pop=8)
    ctx.stub_at(0x004acb00, 'QueueMessage', pop=20)
    return dict(args=[data, size, from_, to, r.randrange(1 << 32)], ret_eax=False)
