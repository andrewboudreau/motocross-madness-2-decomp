"""GameObject::ReleaseDetached 0x00469c80, which removes released dialogs from the tree
(tools/emu_diff.py case).

The linked body is src/reconstructed/GameObject.cpp. Game::Tick calls it on the root once per
frame (retail 0x0046883c). It walks the children (firstChild +0x10, nextSibling +0xc): a child
with pendingRelease (bit 3 of +0x25) is unlinked by the real retail Unlink 0x004691f0 and
released through its slot 2 (a stub that logs it), inside g_MemTagStack's (0x0056df04)
Push(0x0056b6f8)/Pop pair (stubs); any other child is walked recursively. The candidate's
self-call stays in the candidate (PREFER_OBJECT). The tree is up to three levels deep with
random pendingRelease bits, and g_ActiveGameObjectIterators (0x0065b548) is clear or set, in
which case Unlink leaves the links alone but the child is still released. The final links
and flags of every node are compared.
"""
import random

RETAIL_VA = 0x00469c80
SYMBOL = '?ReleaseDetached@GameObject@@QAEXXZ'
SOURCE = 'src/reconstructed/GameObject.cpp'
PREFER_OBJECT = {'?ReleaseDetached@GameObject@@QAEXXZ'}
FPCWS = (0x027f,)
ITERATORS, MEM_TAGS = 0x0065b548, 0x0056df04
PUSH, POP = 0x004a2d00, 0x004a2d90
PROBES = {0x00469c8f: 'release', 0x00469cc0: 'recurse', 0x004691f0: 'Unlink'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), depth=rng.choice([0, 1, 2, 2, 3]),
                   iterators=rng.choice([0, 0, 0, 1]), pending=rng.choice([0.0, 0.3, 0.5, 1.0]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.stub_at(PUSH, 'MemTagStack::Push', pop=4, ret=r.getrandbits(32))
    ctx.stub_at(POP, 'MemTagStack::Pop', pop=4)
    release = ctx.stub('GameObject slot 2', ret=r.getrandbits(32))
    ctx.pack(MEM_TAGS, 'I', ctx.alloc(0x20, fill=0x4d))
    ctx.pack(ITERATORS, 'I', c['iterators'])
    vt = ctx.vtable([0, 0, release] + [0] * 30)

    def node(depth, parent):
        at = ctx.alloc(0x30, bytes(r.getrandbits(8) for _ in range(0x30)))
        flags = r.getrandbits(8) & ~8 | (8 if r.random() < c['pending'] else 0)
        ctx.pack(at, 'I', vt)
        ctx.pack(at + 8, 'IIII', 0, 0, 0, parent)
        ctx.pack(at + 0x25, 'B', flags)
        if depth > 0:
            kids = [node(depth - 1, at) for _ in range(r.choice([0, 1, 2, 3, 4]))]
            for k, kid in enumerate(kids):
                ctx.pack(kid + 8, 'II', kids[k - 1] if k else 0, kids[k + 1] if k + 1 < len(kids) else 0)
            ctx.pack(at + 0x10, 'I', kids[0] if kids else 0)
        return at
    root = node(c['depth'], 0)
    ctx.pack(root + 0x25, 'B', ctx.read(root + 0x25, 1)[0] & ~8)
    return dict(ecx=root, args=[], ret_eax=False)    # void
