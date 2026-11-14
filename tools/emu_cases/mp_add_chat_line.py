"""MultiPlayerDlg::AddChatLine 0x004f3720, the lobby chat line (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/reconstructed/SelectGamePicProcs.cpp. FindControl 0x0046ebf0 answers the
"ListChat" list box (its row count at +0x1ec straddles the 100-row limit, also in the high half
that the short compare ignores) or null; Network::GetPlayerName 0x004ac720 writes a name or fails;
LoadResourceString 0x00521970 writes the system label. g_TrackGame (0x0056e26c) carries the
network (+8, local player +0xc) and the IME flag (+0x538); the seven lobby records
(g_LobbyRemotePlayers 0x00689d08, 0x18 bytes, id +4) hold ids that may match the player once,
twice or not at all. sprintf is modelled and compared through its text; RemoveRow, AddRow (its
text) and ScrollToRow are logged.

The system player (-1) never matches the local player or a lobby id here: retail then formats the
unset name buffer (stack garbage in both builds).
"""
import random

RETAIL_VA = 0x004f3720
SYMBOL = '?AddChatLine@MultiPlayerDlg@@QAEXHPBD@Z'
SOURCE = 'src/reconstructed/SelectGamePicProcs.cpp'
PROBES = {0x004f3767: 'named', 0x004f37f2: 'system plain', 0x004f3809: 'system IME',
          0x004f37a1: 'local colour', 0x004f3835: 'lobby colour', 0x004f386a: 'remove row'}

TRACK_GAME, LOBBY = 0x0056e26c, 0x00689d08
FIND_CONTROL, GET_PLAYER_NAME, LOAD_STRING = 0x0046ebf0, 0x004ac720, 0x00521970
SPRINTF, ADD_ROW = 0x00534aaf, 0x00476d80


def _cstr(ctx, at, limit=0x100):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _sprintf(ctx, rec):
    out, fmt = rec.args[0], _cstr(ctx, rec.args[1])
    rest = list(ctx.unpack(rec.esp + 8, '4I'))
    text, i = b'', 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%':
            j = i + 1
            while fmt[j:j + 1].isdigit():
                j += 1
            width, spec, value = fmt[i + 1:j], fmt[j:j + 1], rest.pop(0)
            if spec == b's':
                text += _cstr(ctx, value)
            else:
                text += (('%' + width.decode() + 'd') % (value - (1 << 32) if value >> 31 else value)).encode()
            i = j + 1
        else:
            text += fmt[i:i + 1]
            i += 1
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', fmt, text)
    return len(text)


def _find(ctx, rec):
    ctx.note('FindControl', _cstr(ctx, rec.args[0]), rec.args[1])
    return ctx.data['list']


def _get_name(ctx, rec):
    name = ctx.data['case']['name']
    if name is None:
        return 0
    ctx.write(rec.args[1], name + b'\0')
    return 1


def _load_string(ctx, rec):
    ctx.write(rec.args[1], b'System\0')
    return 6


def _add_row(ctx, rec):
    ctx.note('AddRow', rec.ecx, _cstr(ctx, rec.args[0]), rec.args[1], rec.args[2])
    return 1


REPLAY = dict(returns={FIND_CONTROL: _find, GET_PLAYER_NAME: _get_name, LOAD_STRING: _load_string,
                       SPRINTF: _sprintf, ADD_ROW: _add_row},
              pops={SPRINTF: 0}, args={SPRINTF: 2}, ecx={SPRINTF: False}, unlogged=(SPRINTF,))


def cases(rng, n):
    for _ in range(n):
        player = rng.choice([-1, -1, 0, 1, 2, 3, 7, 0x1234])
        local = rng.choice([0, 1, 2, 0x1234, player if player != -1 else 0])
        ids = [rng.choice([0, 1, 2, 3, 5, 7, 0x1234, -2]) for _ in range(7)]
        if player != -1 and rng.random() < 0.5:
            ids[rng.randrange(7)] = player
        yield dict(seed=rng.getrandbits(32), player=player, local=local, ids=ids,
                   text=rng.choice([b'', b'hi', b'gg wp ' * 6]),
                   name=rng.choice([b'Ann', b'Joe the rider', b'', None]),
                   ime=rng.choice([0, 0, 1]), found=rng.random() < 0.9,
                   rows=rng.choice([0, 1, 98, 99, 100, 101, 0x7fff, 0x8000, 0xffff, 0x10063, 0x10064]))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def noise(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    ctx.data['case'] = c
    net = ctx.alloc(0x40, noise(0x40))
    ctx.pack(net + 0xc, 'i', c['local'])
    game = ctx.alloc(0x600, noise(0x600))
    ctx.pack(game + 8, 'I', net)
    ctx.pack(game + 0x538, 'i', c['ime'])
    ctx.pack(TRACK_GAME, 'I', game)
    for k, ident in enumerate(c['ids']):
        ctx.write(LOBBY + 0x18 * k, noise(0x18))
        ctx.pack(LOBBY + 0x18 * k + 4, 'i', ident)
    if c['found']:
        listbox = ctx.alloc(0x200, noise(0x200))
        ctx.pack(listbox + 0x1ec, 'I', c['rows'])
        ctx.data['list'] = listbox
    else:
        ctx.data['list'] = 0
    text = ctx.alloc(0x40, c['text'] + b'\0')
    dialog = ctx.alloc(0x400, noise(0x400))
    return dict(ecx=dialog, args=[c['player'] & 0xffffffff, text])
