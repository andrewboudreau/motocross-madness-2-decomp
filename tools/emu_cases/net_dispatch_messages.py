"""NetworkInterface::DispatchMessages 0x004aced0, the received-message ring walk (emu_diff case).

Linked from samples/net/NetNearMisses.cpp. The interface (ECX) holds the ring +0x34 (256
records of 0x8f4 bytes: type +0, from +4, to +8, flags +0x0c, data +0x14; a system message's
player flag and id at +0x18/+0x1c), the cleared ring-full word +0x38, the wrap flag +0x3c and
nextMessage +0x40. Runs cover an empty ring, an unwrapped ring and a wrapped ring with the
next slot at 0 (last = 0xff) or inside, and records of every kind: 0x40 acknowledgements,
system messages (from 0) of type 3 (create player, flag 1 or not, id or 0), 5 (destroy
player) and others, and player messages from known or unknown senders.

Stubbed: ResendPending 0x004acc50, FindPlayer 0x004ac800 (known or not), GetPlayerName
0x004ac720 (writes a name, succeeds or not), AddPlayer 0x004ac980 (the name it is handed
is noted), AcknowledgeMessage 0x004acca0 (only the short's 16 bits are noted: retail pushes
edx after loading dx), RemovePlayer 0x004aca80 and g_TrackGame's (0x0056e26c) vtable slot
+0x44 OnNetworkMessage(type, data, from, to, flags), all thiscall with ECX logged.
"""
import random
import struct

RETAIL_VA = 0x004aced0
SYMBOL = '?DispatchMessages@NetworkInterface@@QAEXH@Z'
SOURCE = 'samples/net/NetNearMisses.cpp'
TRACK_GAME = 0x0056e26c
MESSAGE_SIZE = 0x8f4
PROBES = {0x004ad038: 'empty ring', 0x004acf10: 'wrap to 0xff', 0x004acf74: 'add player',
          0x004acf8e: 'acknowledge', 0x004acfd7: 'remove player', 0x004ad003: 'system add player',
          0x004acfb4: 'OnNetworkMessage'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    net = ctx.alloc(0x200, fill=0x4e)
    ring = ctx.alloc(256 * MESSAGE_SIZE, fill=0)
    kind = r.choice(['empty', 'flat', 'flat', 'wrap', 'wrap0'])
    if kind == 'empty':
        nxt, wrapped = 0, 0
    elif kind == 'flat':
        nxt, wrapped = r.randrange(1, 12), 0
    elif kind == 'wrap':
        nxt, wrapped = r.choice([1, 3, 250]), 1
    else:
        nxt, wrapped = 0, 1
    ctx.pack(net + 0x34, 'Iiii', ring, r.randrange(2), wrapped, nxt)
    known = [r.randrange(1, 1 << 31) for _ in range(3)]
    for k in range(256):
        if not wrapped and k >= nxt:
            break
        if wrapped and kind == 'wrap' and 12 <= k < nxt - 8:
            continue                            # left zero: system type 0 records
        at = ring + k * MESSAGE_SIZE
        t = r.choice(['ack', 'sys3', 'sys5', 'sys', 'player', 'player', 'stranger'])
        if t == 'ack':
            fields = (0x40, r.choice([0] + known), r.randrange(1 << 31), r.randrange(4))
        elif t.startswith('sys'):
            fields = ({'sys3': 3, 'sys5': 5}.get(t, r.choice([0x21, 0x31, 7])), 0, 0, 0)
        else:
            sender = r.choice(known) if t == 'player' else r.randrange(1, 1 << 31)
            fields = (r.randrange(0x41, 0x100), sender, r.randrange(1 << 31), r.randrange(4))
        ctx.pack(at, 'iiii', *fields)
        ctx.write(at + 0x14, bytes(r.randrange(256) for _ in range(4)))
        if t == 'sys3' or t == 'sys5':
            ctx.pack(at + 0x18, 'ii', r.choice([1, 1, 0]), r.choice([0] + known + [r.randrange(1, 1 << 31)]))

    def find_player(cx, ecx, a):
        return 0x0f1a7e00 if a[0] in known[:2] else 0

    def get_name(cx, ecx, a):           # (id, char name[16])
        ok = r.random() < 0.7
        if ok:
            cx.write(a[1], b'rider%d\0' % r.randrange(100))
        return int(ok)

    def add_player(cx, ecx, a):
        cx.note('AddPlayer', ecx, a[0], cx.read(a[1], 8).split(b'\0')[0])
        return 0

    def ack(cx, ecx, a):
        cx.note('AcknowledgeMessage', ecx, a[0] & 0xffff)
        return 0

    game = ctx.alloc(0x20, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    vtable = ctx.alloc(0x80, fill=0)
    ctx.pack(game, 'I', vtable)
    ctx.pack(vtable + 0x44, 'I', ctx.stub('OnNetworkMessage', pop=20))
    ctx.stub_at(0x004acc50, 'ResendPending', pop=4)
    ctx.stub_at(0x004ac800, 'FindPlayer', pop=4, ret=find_player)
    ctx.stub_at(0x004ac720, 'GetPlayerName', pop=8, ret=get_name)
    ctx.stub_at(0x004ac980, 'AddPlayer', pop=8, ret=add_player, log=False)
    ctx.stub_at(0x004acca0, 'AcknowledgeMessage', pop=4, ret=ack, log=False)
    ctx.stub_at(0x004aca80, 'RemovePlayer', pop=4)
    return dict(ecx=net, args=[r.randrange(1 << 32)], ret_eax=False)
