"""SPRaceInfoDlg::HandleEvent 0x004507a0, the single-player race results dialog
(tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group2_SPRaceInfoDlg.cpp. g_TrackGame 0x0056e26c is a block of
seeded bytes with the fields the body branches on set: +0x56c the UI (+0x40 replay available,
+0x3c current menu), +0x570 the event manager (+0x48 races run, a byte; +0x4c result count;
0x50-byte results from +0x50), +0x2d70 race mode (2 is a championship), +0x2d74 event type
(0..5 and out of range), +0x2d7c races. The init fills the stats lists for every event type;
the ButViewReplay command reads the replay tape header (fopen succeeds or fails, fread fills
0xeb0 seeded bytes), copies its race settings, garage engine and tape name, leaves the menus
and marks the parent dialog (+0x2c, a fake vtable) for release. _chkstk runs inline.

Every call is replayed. Text producers write their text so the strings the body formats
and passes on are compared: LoadResourceString 0x00521970 ("res <id>"), FormatLapTime
0x00518690 ("lap <time>") and sprintf 0x00534aaf (a %s/%d formatter, noted with its output).
_stricmp compares for real; strncpy 0x00534b10 runs natively.
"""
import random

RETAIL_VA = 0x004507a0
SYMBOL = '?HandleEvent@SPRaceInfoDlg@@UAEXPAUUnknownDialogEvent@@@Z'
SOURCE = 'src/bringup/Group2_SPRaceInfoDlg.cpp'
TRACK_GAME = 0x0056e26c
SPRINTF, STRICMP, LOAD_STRING, FORMAT_LAP = 0x00534aaf, 0x00536070, 0x00521970, 0x00518690
FOPEN, FREAD = 0x00534ceb, 0x00534f65
PROBES = {0x004507ca: 'init', 0x004508ff: 'race events', 0x00450b6e: 'other events',
          0x00450cb1: 'command', 0x00450d21: 'tape read'}


def _cstr(ctx, at, limit=256):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _sprintf(ctx, rec):
    out, fmt, rest = rec.args[0], _cstr(ctx, rec.args[1]), list(rec.args[2:])
    text, k, i = b'', 0, 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%' and i + 1 < len(fmt):
            spec = fmt[i + 1:i + 2]
            value = rest[k]
            k += 1
            if spec == b's':
                text += _cstr(ctx, value, 128)
            else:
                text += str(value - (1 << 32) if value & 0x80000000 else value).encode()
            i += 2
        else:
            text += fmt[i:i + 1]
            i += 1
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', text)
    return len(text)


def _stricmp(ctx, rec):
    a = _cstr(ctx, rec.args[0]).lower()
    b = _cstr(ctx, rec.args[1]).lower()
    return 0 if a == b else (0xffffffff if a < b else 1)


def _load_string(ctx, rec):
    # TrackGame::LoadResourceString(id, buffer, size)
    ctx.write(rec.args[1], b'res %#x\0' % rec.args[0])
    return 1


def _format_lap(ctx, rec):
    # FormatLapTime(buffer, time)
    ctx.write(rec.args[0], b'lap %08x\0' % rec.args[1])
    return 0


def _fopen(ctx, rec):
    return 0 if ctx.data['no_tape'] else ctx.alloc(0x40, fill=0x46)


def _fread(ctx, rec):
    # fread(buffer, size, count, file)
    n = rec.args[1] * rec.args[2]
    r = random.Random(ctx.data['seed'] ^ 0x7a9e)
    ctx.write(rec.args[0], bytes(r.getrandbits(8) for _ in range(min(n, 0x1000))))
    return rec.args[2]


REPLAY = dict(
    returns={SPRINTF: _sprintf, STRICMP: _stricmp, LOAD_STRING: _load_string, FORMAT_LAP: _format_lap,
             FOPEN: _fopen, FREAD: _fread},
    args={SPRINTF: 7, STRICMP: 2, LOAD_STRING: 3, FORMAT_LAP: 2, FOPEN: 2, FREAD: 4},
    ecx={SPRINTF: False, STRICMP: False},
    pops={SPRINTF: 0, STRICMP: 0, FORMAT_LAP: 0, FOPEN: 0, FREAD: 0},
    unlogged=(SPRINTF, STRICMP),
    passthrough=(0x00534b10,),                  # strncpy
    inline=(0x00535ed0,),                       # _chkstk (it returns with ESP lowered)
)


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.getrandbits(32), kind=rng.choice([5, 5, 5, 1, 1, 0, 3]),
            name=rng.choice([b'ButViewReplay', b'butviewreplay', b'ButOK', b'']),
            no_tape=rng.random() < 0.3, replay=rng.choice([0, 1, 7]),
            race_mode=rng.choice([2, 2, 0, 1, 3]), event_type=rng.choice([0, 1, 2, 3, 4, 5, 6, 0xffffffff]),
            races=rng.randrange(0, 6), run=rng.randrange(0, 8), results=rng.randrange(0, 9),
        )


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data['seed'] = c['seed']
    ctx.data['no_tape'] = c['no_tape']

    def block(size):
        return ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))

    game = block(0x3800)
    ctx.pack(TRACK_GAME, 'I', game)
    ui = block(0x100)
    ctx.pack(game + 0x56c, 'I', ui)
    ctx.pack(ui + 0x40, 'i', c['replay'])
    manager = block(0x50 + 0x50 * 10)
    for k in range(10):
        ctx.write(manager + 0x90 + 0x50 * k, b'Rider %d\0' % k)
        ctx.pack(manager + 0x70 + 0x50 * k, 'i', r.choice([0, 1]))
    ctx.pack(game + 0x570, 'I', manager)
    ctx.pack(manager + 0x48, 'B', c['run'])
    ctx.pack(manager + 0x4c, 'i', c['results'])
    ctx.pack(game + 0x2d70, 'iIi', c['race_mode'], c['event_type'], 0)
    ctx.pack(game + 0x2d7c, 'i', c['races'])
    dlg = block(0x200)
    parent = ctx.alloc(0x40)                                # +0x2c the parent dialog
    ctx.pack(parent, 'I', ctx.fake_vtable('parent', 32))
    ctx.pack(dlg + 0x2c, 'I', parent)
    name = ctx.alloc(32, c['name'] + b'\0')
    event = block(0x20)
    ctx.pack(event + 4, 'Ii', name, c['kind'])
    return dict(ecx=dlg, args=[event], ret_eax=False)
