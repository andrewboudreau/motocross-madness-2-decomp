"""SoultreeObject::LocalToWorldDirection 0x004fd5c0 through DrawSoultreeObjectView (tools/emu_diff.py case).

src/bringup/Group5_SoultreeNodeView.cpp forwards it to SoultreeObject::LocalToWorldDirection,
bound to the same retail body; the case checks the forwarding (the hidden return slot, `this`,
the vector and the returned pointer). The node's world rotation rows are at +0xf8..+0x120.
Stubbed: 0x004fb4f0 (refreshes the world matrix; `this` noted). Random matrices and vectors,
the output aliased to the input.
"""
import random

RETAIL_VA = 0x004fd5c0
SYMBOL = '?LocalToWorldDirection@DrawSoultreeObjectView@@QAE?AUVector3@@PBU2@@Z'
SOURCE = 'src/bringup/Group5_SoultreeNodeView.cpp'


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    node = ctx.alloc(0x140, fill=0x4e)
    ctx.pack(node + 0xf8, '12f', *[r.uniform(-2, 2) for _ in range(12)])
    v = ctx.alloc(12)
    ctx.pack(v, '3f', *[r.uniform(-100, 100) for _ in range(3)])
    out = v if r.random() < 0.25 else ctx.alloc(12, fill=0x77)
    ctx.stub_at(0x004fb4f0, 'UpdateWorldMatrix')
    return dict(ecx=node, args=[out, v])
