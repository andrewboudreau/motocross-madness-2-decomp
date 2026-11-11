"""PCBailoutDlg::HandleEvent 0x004d8fc0, the career bail-out offer (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group5_PCBailoutDlg.cpp. g_TrackGame 0x0056e26c +0x3444 is the
pro circuit (+0x40 series, +0x1229 the unaligned entry-fee table, +0x495 the player's money,
+0x499 debts, +0x464 career flags), +0x3448 the career file, +0x420 the resource instance.
The dialog keeps the entry fee, the player's cash and the shortfall at +0x7f58/+0x7f5c/+0x7f60.

The event is an init (kind 5, which fills the labels), a command (kind 1) naming ButAccept,
ButDecline (either case) or another control, or another kind. The money can be negative
(its label turns red). Replayed calls: FindControl 0x0046ebf0 (a fresh control each time),
SetTextFromResource, SetText (the text is logged), SetFontColor, Show, ProCircuit::Save,
the dialog constructors, ShowDialog and EndDialog. sprintf 0x00534aaf formats "$%d" into the
caller's buffer; _stricmp 0x00536070 compares for real; operator new 0x004a3010 is left out
of the log (its __FILE__ pointer differs by build) and noted by size and line.
"""
import random

from tools.emu_cases.kb_simulate_frame import install_seh_segment

RETAIL_VA = 0x004d8fc0
SYMBOL = '?HandleEvent@PCBailoutDlg@@UAEXPAUUnknownDialogEvent@@@Z'
SOURCE = 'src/bringup/Group5_PCBailoutDlg.cpp'
TRACK_GAME = 0x0056e26c
FIND_CONTROL, SPRINTF, STRICMP, NEW = 0x0046ebf0, 0x00534aaf, 0x00536070, 0x004a3010
PROBES = {0x004d9005: 'init', 0x004d9205: 'accept', 0x004d929b: 'decline'}


def _cstr(ctx, at):
    s = ctx.read(at, 128)
    return s[:s.index(0)] if 0 in s else s


def _find(ctx, rec):
    return ctx.alloc(0x200, fill=0x43)


def _sprintf(ctx, rec):
    fmt = _cstr(ctx, rec.args[1])
    value = rec.args[2] - (1 << 32) if rec.args[2] & 0x80000000 else rec.args[2]
    out = fmt.replace(b'%d', str(value).encode()) + b'\0'
    ctx.write(rec.args[0], out)
    return len(out) - 1


def _stricmp(ctx, rec):
    a = _cstr(ctx, rec.args[0]).lower()
    b = _cstr(ctx, rec.args[1]).lower()
    return 0 if a == b else (0xffffffff if a < b else 1)


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    if ctx.data['new_null']:
        return 0
    return ctx.alloc(rec.args[0], fill=0x6e)


REPLAY = dict(
    returns={FIND_CONTROL: _find, SPRINTF: _sprintf, STRICMP: _stricmp, NEW: _new},
    args={SPRINTF: 3, STRICMP: 2, NEW: 3},
    ecx={SPRINTF: False, STRICMP: False, NEW: False},
    pops={SPRINTF: 0, STRICMP: 0, NEW: 0},
    unlogged=(NEW, STRICMP),
)

NAMES = [b'ButAccept', b'BUTACCEPT', b'ButDecline', b'butdecline', b'Back', b'']


def cases(rng, n):
    for _ in range(n):
        yield dict(
            seed=rng.getrandbits(32), kind=rng.choice([5, 5, 1, 1, 1, 0, 2, 6]),
            name=rng.choice(NAMES), series=rng.randrange(4),
            money=rng.choice([0, -1, rng.randrange(-50000, 50000), rng.randrange(1 << 31)]),
            new_null=rng.random() < 0.1,
        )


def setup(ctx, c):
    install_seh_segment(ctx)             # the body sets up an SEH frame (fs:[0])
    r = random.Random(c['seed'])
    ctx.data['new_null'] = c['new_null']
    game = ctx.alloc(0x3500, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    circuit = ctx.alloc(0x1300, bytes(r.getrandbits(8) for _ in range(0x1300)))
    ctx.pack(game + 0x3444, 'I', circuit)
    ctx.pack(game + 0x420, 'I', 0x00400000)
    ctx.pack(circuit + 0x40, 'i', c['series'])
    for k in range(8):
        ctx.pack(circuit + 0x1229 + 4 * k, 'i', r.randrange(0, 100000))
    ctx.pack(circuit + 0x495, 'i', c['money'])
    ctx.pack(circuit + 0x499, 'i', r.randrange(-1000, 100000))
    dlg = ctx.alloc(0x8000, fill=0x44)
    for off in (0x7f58, 0x7f5c, 0x7f60):
        ctx.pack(dlg + off, 'i', r.randrange(-100000, 100000))
    ctx.pack(dlg + 0x30, 'I', ctx.alloc(0x40, fill=0x4d))
    name = ctx.alloc(32, c['name'] + b'\0')
    event = ctx.alloc(0x20, fill=0x45)
    ctx.pack(event + 4, 'Ii', name, c['kind'])
    return dict(ecx=dlg, args=[event], ret_eax=False)      # void
