"""LoadingDlg::HandleEvent 0x00451020, the loading dialog that ends itself once the race has
loaded (tools/emu_diff.py case, call-record/replay mode).

The linked body is src/reconstructed/DlgProcs.cpp. The event is an init (kind 5: finds the
progress area, builds its progress bar and starts the load), a kind 6 (ends the load
progress), the dialog timer (kind 7) or another kind. On the timer, g_LoadingTimerTicks
0x0059adfc counts up; from 10 on, EndDialog 0x0046ff30 runs on the event's dialog (+0xc),
the event is marked handled (+0x20) and the race starts, with the network branch chosen by
g_TrackGame 0x0056e26c +8. The tick count sits below, at or past 10.

Every call is replayed. operator new 0x004a3010 is left out of the log (its __FILE__ pointer
differs by build) and noted by size and line. It always succeeds: on a null result retail still
writes the bar's fields (+0x1f0, +0x1fc) through the null pointer, so both bodies fault. The
UIProgressBar constructor 0x0047b020 returns its ``this``.
"""
import random

from tools.emu_cases.kb_simulate_frame import install_seh_segment

RETAIL_VA = 0x00451020
SYMBOL = '?HandleEvent@LoadingDlg@@UAEXPAUDialogEvent@@@Z'
SOURCE = 'src/reconstructed/DlgProcs.cpp'
FPCWS = (0x027f,)
TRACK_GAME, TICKS = 0x0056e26c, 0x0059adfc
NEW, PROGRESS_BAR = 0x004a3010, 0x0047b020
PROBES = {0x00451078: 'EndDialog (network)', 0x004510ac: 'EndDialog', 0x00451120: 'kind 6',
          0x0045114a: 'init'}


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(rec.args[0], fill=0x6e)


REPLAY = dict(
    returns={NEW: _new, PROGRESS_BAR: lambda ctx, rec: rec.ecx},
    args={NEW: 3},
    ecx={NEW: False},
    pops={NEW: 0},
    unlogged=(NEW,),
)


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), kind=rng.choice([7, 7, 7, 5, 5, 6, 0, 1, 8]),
                   ticks=rng.choice([0, 9, 10, 11, rng.randrange(-5, 30)]), network=rng.choice([0, 0, 1]))


def setup(ctx, c):
    install_seh_segment(ctx)             # the body sets up an SEH frame (fs:[0])
    r = random.Random(c['seed'])

    def block(size):
        return ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))

    game = block(0xc40)
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(game + 8, 'I', c['network'] and r.getrandbits(32) | 1)
    for off in (0x55c, 0x568):
        ctx.pack(game + off, 'I', block(0x40) if r.random() < 0.7 else 0)
    ctx.pack(game + 0x56c, 'I', block(0x500))
    ctx.pack(game + 0x570, 'I', block(0x80))
    ctx.pack(TICKS, 'i', c['ticks'])
    dlg = block(0x7f60)
    ctx.pack(dlg + 0x30, 'I', block(0x40))
    event = block(0x30)
    ctx.pack(event + 8, 'I', c['kind'])
    ctx.pack(event + 0xc, 'I', block(0x7f60))
    return dict(ecx=dlg, args=[event], ret_eax=False)      # void
