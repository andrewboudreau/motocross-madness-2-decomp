"""GameObject::MarkPendingRelease 0x004692c0, slot 26 (tools/emu_diff.py case).

The linked body is src/reconstructed/GameObject.cpp. It calls slot 26 on every child
(firstChild +0x10, nextSibling +0xc), then sets pendingRelease (bit 3 of +0x25), which the
next Game::Tick's ReleaseDetached 0x00469c80 acts on. The tree is up to three levels deep
with seeded flag bytes; every node's vtable has ``ctx.entry`` at slot 26, so the recursion
runs in the body under test and the final flag bytes of the whole subtree are compared.
"""
import random

RETAIL_VA = 0x004692c0
SYMBOL = '?MarkPendingRelease@GameObject@@UAEXXZ'
SOURCE = 'src/reconstructed/GameObject.cpp'
FPCWS = (0x027f,)
PROBES = {0x004692cb: 'child', 0x004692d9: 'mark'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), depth=rng.choice([0, 1, 2, 3]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    vt = ctx.vtable([0] * 26 + [ctx.entry] + [0] * 6)

    def node(depth):
        at = ctx.alloc(0x30, bytes(r.getrandbits(8) for _ in range(0x30)))
        ctx.pack(at, 'I', vt)
        first = 0
        if depth > 0:
            kids = [node(depth - 1) for _ in range(r.choice([0, 1, 2, 3]))]
            for k, kid in enumerate(kids):
                ctx.pack(kid + 0xc, 'I', kids[k + 1] if k + 1 < len(kids) else 0)
                ctx.pack(kid + 0x14, 'I', at)
            first = kids[0] if kids else 0
        ctx.pack(at + 0x10, 'I', first)
        return at
    return dict(ecx=node(c['depth']), args=[], ret_eax=False)    # void
