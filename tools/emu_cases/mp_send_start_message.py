"""MultiPlayerDlg::SendStartMessage 0x004f2340, the lobby start message (tools/emu_diff.py case).

The linked body is src/bringup/Group5_MultiPlayerDlg.cpp. Offsets from the retail body:
g_TrackGame (0x0056e26c) +8 the network (local player +0xc), +0x18 playerCount, +0x56c the
KrustyUI (manufacturers +0x48, 200-byte records, model name +0x40; bikes +0x50, 0x94-byte
records: model +0, texture +0x48, engine size +0x8c, four-stroke +0x90; riders +0x58, 200-byte
records, model name +0x40), +0x570 the event manager (racesRun byte +0x48, results +0x50 stride
0x50 with the points byte at +0x28), +0x578 the mode settings, +0x2144 the plate number,
+0x2150 maxOpponents, +0x2158 racerCount, +0x215c the racers (0xf8 bytes), +0x2d70 the race
settings (raceMode +0, network player count byte +0x35, AI racer count +0x28, grid +0x14c),
+0x3424 networkAiRacerCount, +0x60c the difficulty. The dialog's lobby slots are at +0x7f68
(12 bytes: player id +0, granted racers +8); g_LobbyRemotePlayerCount is 0x00689df8 and the
lobby records 0x00689d08 (0x18 bytes, id +4).

Every case keeps the racers inside the eight message slots: 0..7 remote players, AI racers to
fill up to eight, slot grants that use all or fewer of them (GrantRacers 0x004f2fe0 answers a
count below, at or above the requested AI count); the racers' earlier strings are terminated, as
the game's COPY_TEXT copies leave them. Names run past the 15-character copies. The
grid is drawn by championship points (race mode 2 after a race) or at random (rand 0x00534c1b
is a stub; __ftol, strncpy and qsort with retail's comparators run natively in both bodies).
DebugCalloc/DebugMalloc/DebugFree, PickRandomRacers, MakeRandomNames, ResetSettingsEntries,
GetPlayerName and FillRacerSlots are stubs; Network::Send 0x004ac830 notes the message's defined
parts (start, race settings, difficulty, and with `start` the plates and the strings up to their
terminators), not the uninitialized header and tails.
"""
import random
import struct

RETAIL_VA = 0x004f2340
SYMBOL = '?SendStartMessage@MultiPlayerDlg@@QAEXF@Z'
SOURCE = 'src/bringup/Group5_MultiPlayerDlg.cpp'
PROBES = {0x004f23e2: 'clamp AI', 0x004f2506: 'slot racers', 0x004f27aa: 'grid by points',
          0x004f2819: 'grid at random', 0x004f2958: 'start fields', 0x004f2b76: 'fill slots'}

TRACK_GAME, LOBBY, LOBBY_COUNT = 0x0056e26c, 0x00689d08, 0x00689df8


def _cstr(ctx, at, limit=0x100):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _name(rng, longest=24):
    return ''.join(rng.choice('abcXYZ 09-') for _ in range(rng.choice([0, 1, 5, 15, 16, longest]))).encode()


def cases(rng, n):
    for _ in range(n):
        lobby = rng.choice([0, 0, 1, 2, 3, 7])
        humans = lobby + 1
        ai = rng.randrange(0, 8 - humans + 1)
        grants = [0] * 8
        for _ in range(rng.choice([ai, ai, max(ai - 1, 0)])):
            grants[rng.randrange(8)] += 1
        yield dict(seed=rng.getrandbits(32), start=rng.choice([0, 1, 1, 1, -2]), lobby=lobby, ai=ai,
                   grants=grants, granted=rng.choice([ai, ai, max(ai - 1, 0), ai + 1]),
                   race_mode=rng.choice([0, 1, 2, 2]), races_run=rng.choice([0, 1, 3, 0x80]),
                   local_name=_name(rng, 40))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def noise(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    def table(count, size, fields):
        at = ctx.alloc(count * size, noise(count * size))
        for k in range(count):
            for off, value in fields():
                ctx.write(at + size * k + off, value)
        return at

    manufacturers = table(6, 200, lambda: [(0x40, _name(r, 70) + b'\0')])
    bikes = table(5, 0x94, lambda: [(0, struct.pack('<i', r.randrange(6))), (0x48, _name(r, 70) + b'\0'),
                                    (0x8c, struct.pack('<ii', r.choice([125, 250]), r.randrange(2)))])
    riders = table(5, 200, lambda: [(0x40, _name(r, 70) + b'\0')])
    ui = ctx.alloc(0x100, noise(0x100))
    ctx.pack(ui + 0x48, 'I', manufacturers)
    ctx.pack(ui + 0x50, 'I', bikes)
    ctx.pack(ui + 0x58, 'I', riders)
    events = ctx.alloc(0x50 * 12, noise(0x50 * 12))
    ctx.pack(events + 0x48, 'B', c['races_run'])
    net = ctx.alloc(0x40, noise(0x40))
    game = ctx.alloc(0x3500, noise(0x3500))
    ctx.pack(game + 8, 'I', net)
    ctx.pack(game + 0x56c, 'I', ui)
    ctx.pack(game + 0x570, 'I', events)
    ctx.pack(game + 0x2d70, 'i', c['race_mode'])
    ctx.pack(game + 0x2d98, 'i', c['ai'])
    ctx.pack(TRACK_GAME, 'I', game)
    for k in range(8):                  # earlier racers' strings (terminated, as COPY_TEXT leaves them)
        for off, size in ((0, 0x40), (0x40, 0x40), (0x80, 0x40), (0xdc, 0x10)):
            ctx.write(game + 0x215c + 0xf8 * k + off, _name(r, size - 1)[:size - 1] + bytes(1))
    ctx.pack(LOBBY_COUNT, 'i', c['lobby'])
    for k in range(7):
        ctx.write(LOBBY + 0x18 * k, noise(0x18))
    dialog = ctx.alloc(0x8000, noise(0x8000))
    for j, granted in enumerate(c['grants']):
        ctx.pack(dialog + 0x7f68 + 12 * j + 8, 'i', granted)

    def calloc(cx, e, a):
        cx.note('DebugCalloc', a[0], a[1], a[3])
        return cx.alloc(a[0] * a[1] + 4, fill=0)

    def malloc(cx, e, a):
        cx.note('DebugMalloc', a[0], a[2])
        block = cx.alloc(a[0] + 4, fill=0x99)
        cx.data['names'] = block
        return block

    def free(cx, e, a):
        cx.note('DebugFree', a[0] == cx.data.get('names'), a[2])

    def pick(cx, e, a):
        cx.note('PickRandomRacers', a[1])
        cx.pack(a[0], '%di' % a[1], *[cx.rng.randrange(5) for _ in range(a[1])])
        cx.pack(a[2], '%di' % a[1], *[cx.rng.randrange(5) for _ in range(a[1])])

    def names(cx, e, a):
        cx.note('MakeRandomNames', a[1])
        cx.pack(a[0], '%dI' % a[1], *[cx.alloc(0x20, _name(cx.rng) + b'\0') for _ in range(a[1])])

    def player_name(cx, e, a):
        cx.note('GetPlayerName', a[0])
        cx.write(a[1], c['local_name'] + b'\0')
        return 1

    def send(cx, e, a):
        data = a[1]
        start, = cx.unpack(data + 4, 'h')
        fields = [a[0], a[2], a[3], a[4], start, cx.read(data + 8, 0x1f0)]
        if start:
            total = cx.unpack(game + 0x2158, 'i')[0]
            humans = c['lobby'] + 1
            for i in range(total):
                fields += [cx.unpack(data + 0x1f8 + 4 * i, 'i')[0], _cstr(cx, data + 0x218 + 0x10 * i)]
                if i >= humans:
                    fields += [_cstr(cx, data + base + 0x40 * i) for base in (0x298, 0x498, 0x698)]
                    fields += list(cx.unpack(data + 0x898 + 4 * i, 'i')) + list(cx.unpack(data + 0x8b8 + 4 * i, 'i'))
        cx.note('Send', *fields)
        return 1

    ctx.stub_at(0x004f2fe0, 'LobbySlots::GrantRacers', pop=8, ret=c['granted'])
    ctx.stub_at(0x004a2fc0, 'DebugCalloc', nargs=4, ret=calloc, log=False)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(0x004a2e60, 'DebugFree', nargs=3, ret=free, log=False)
    ctx.stub_at(0x0049b0d0, 'KrustyUI::PickRandomRacers', pop=12, ret=pick, log=False)
    ctx.stub_at(0x0049b020, 'KrustyUI::MakeRandomNames', pop=8, ret=names, log=False)
    def rand(cx, e, a):
        value = cx.rng.randrange(32768)
        cx.note('rand', value)          # cdecl: ECX is scratch
        return value

    ctx.stub_at(0x00534c1b, 'rand', ret=rand, log=False)
    ctx.stub_at(0x00522cd0, 'TrackGameMode::ResetSettingsEntries', pop=0)
    ctx.stub_at(0x004ac720, 'Network::GetPlayerName', pop=8, ret=player_name, log=False)
    ctx.stub_at(0x004ac830, 'Network::Send', pop=20, ret=send, log=False)
    ctx.stub_at(0x004f20d0, 'MultiPlayerDlg::FillRacerSlots', pop=0)
    return dict(ecx=dialog, args=[c['start'] & 0xffffffff], ret_eax=False)
