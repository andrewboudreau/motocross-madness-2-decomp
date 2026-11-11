"""UIDialog::EndDialog 0x0046ff30, the first step of a dialog's close (tools/emu_diff.py case).

The linked body is src/reconstructed/GameUi.cpp. An open dialog (+0x7f50 isClosing clear)
stores the result at +0x17c, sets isClosing and copies its render context's (+0x18)
frameCount (+0x1c) into closeFrame (+0x7f54); RenderPost3D 0x0046f320 marks it for release
once the frame count has moved on. A dialog already closing keeps its first result and frame.
The dialog is seeded bytes with isClosing either clear or a random nonzero word.
"""
import random

RETAIL_VA = 0x0046ff30
SYMBOL = '?EndDialog@UIDialog@@QAEXH@Z'
SOURCE = 'src/reconstructed/GameUi.cpp'
FPCWS = (0x027f,)
PROBES = {0x0046ff3a: 'open', 0x0046ff5a: 'return'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), closing=rng.choice([0, 0, 1, rng.getrandbits(32) | 1]),
                   result=rng.choice([0, 1, -1, rng.getrandbits(32)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    target = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    dlg = ctx.alloc(0x7f58, bytes(r.getrandbits(8) for _ in range(0x7f58)))
    ctx.pack(dlg + 0x18, 'I', target)
    ctx.pack(dlg + 0x7f50, 'I', c['closing'])
    return dict(ecx=dlg, args=[c['result'] & 0xffffffff], ret_eax=False)    # void
