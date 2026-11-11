"""NetworkInterface::CreateAddress 0x004abf10, the DirectPlay address builder (tools/emu_diff.py
case, call-record/replay mode).

The linked body is src/reconstructed/Net.cpp. The provider GUID is passed by value and is one
of IPX 0x005567d0, TCP/IP 0x005567e0, modem 0x00556800, serial 0x005567f0 or an unknown GUID;
the address and port strings are empty or not, the COM-port block null or not.

Replayed calls: lstrlenA (IAT 0x005501dc, called through the slot or, in the modem branch,
through a register holding the slot's contents; noted with its string, returns the length),
atoi 0x00536057, IDirectPlayLobby3A::CreateCompoundAddress (the lobby at this+8, vtable +0x38, 5 stdcall words;
the element array is logged as 0x48 bytes, so every element's type GUID, size and data pointer
is compared). The first lobby call answers DPERR_BUFFERTOOSMALL with a size, or another result;
the second succeeds or fails. DebugMalloc 0x004a2e20 and DebugFree 0x004a2e60 are left out of
the log (their __FILE__ pointers differ by build) and noted by size or pointer and line.
"""
import random

RETAIL_VA = 0x004abf10
SYMBOL = '?CreateAddress@NetworkInterface@@QAEHU_GUID@@PAD1PAXPAPAXPAK@Z'
SOURCE = 'src/reconstructed/Net.cpp'
LSTRLEN_IAT = ('iat', 0x005501dc)
LSTRLEN_ENTRY = 0x00164ed8      # the slot's unbound contents, called as `call ebp` in the modem branch
DEBUG_MALLOC, DEBUG_FREE = 0x004a2e20, 0x004a2e60
PROVIDERS = (0x005567d0, 0x005567e0, 0x00556800, 0x005567f0, None)
BUFFER_TOO_SMALL = 0x8877001e
PROBES = {0x004abf6e: 'ipx', 0x004ac005: 'tcp/ip', 0x004ac0db: 'modem', 0x004ac1ac: 'serial',
          0x004ac24a: 'allocate', 0x004ac29f: 'success'}


def _strlen(ctx, rec):
    s = ctx.read(rec.args[0], 256)
    s = s[:s.index(0)] if 0 in s else s
    ctx.note('lstrlenA', rec.args[0], s)
    return len(s)


def _malloc(ctx, rec):
    size, line = rec.args[0], rec.args[2]
    ctx.note('DebugMalloc', size, line)
    if ctx.data['malloc_null']:
        return 0
    return ctx.alloc(max(size, 1), fill=0x6d)


def _free(ctx, rec):
    ctx.note('DebugFree', rec.args[0], rec.args[2])
    return 0


REPLAY = dict(
    returns={LSTRLEN_IAT: _strlen, LSTRLEN_ENTRY: _strlen, DEBUG_MALLOC: _malloc, DEBUG_FREE: _free},
    pops={LSTRLEN_IAT: 4, LSTRLEN_ENTRY: 4},
    args={LSTRLEN_IAT: 1, LSTRLEN_ENTRY: 1, DEBUG_MALLOC: 3, DEBUG_FREE: 3},
    ecx={LSTRLEN_IAT: False, LSTRLEN_ENTRY: False},
    unlogged=(LSTRLEN_IAT, LSTRLEN_ENTRY, DEBUG_MALLOC, DEBUG_FREE),
)


def _text(rng):
    return rng.choice([b'', b'', b'192.168.0.7', b'5551234', b'Standard Modem', b'2300', b'x'])


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.getrandbits(32), provider=rng.choice(PROVIDERS),
            address=_text(rng), port=_text(rng), com=rng.random() < 0.7,
            first=rng.choice([BUFFER_TOO_SMALL] * 4 + [0, 0x80004005]),
            size=rng.choice([0, 1, 0x40, 0x123]),
            second=rng.choice([0, 0, 1, 0x80004005]),
            malloc_null=rng.random() < 0.15,
        )


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data['malloc_null'] = c['malloc_null']
    calls = []

    def compound(cx, rec):
        # CreateCompoundAddress(this, elements, count, buffer, &size)
        calls.append(rec.args[3])
        cx.note('elements', rec.args[2], cx.read(rec.args[1], 0x18 * min(rec.args[2], 3)))
        if len(calls) == 1:
            if c['first'] == BUFFER_TOO_SMALL:
                cx.pack(rec.args[4], 'I', c['size'])
            return c['first']
        return c['second']

    lobby = ctx.alloc(0x20)
    ctx.pack(lobby, 'I', ctx.fake_vtable('lobby', 24, pops={14: 20}, returns={14: compound}))
    net = ctx.alloc(0x100, fill=0x4e)
    ctx.pack(net + 8, 'I', lobby)
    guid = (ctx.read(c['provider'], 16) if c['provider'] is not None
            else bytes(r.getrandbits(8) for _ in range(16)))
    address = ctx.alloc(64, c['address'] + b'\0')
    port = ctx.alloc(64, c['port'] + b'\0')
    com = ctx.alloc(0x20, bytes(r.getrandbits(8) for _ in range(0x20))) if c['com'] else 0
    result = ctx.alloc(8, fill=0x77)
    size = ctx.alloc(8, fill=0x77)
    words = list(memoryview(guid).cast('I'))
    return dict(ecx=net, args=words + [address, port, com, result, size])
