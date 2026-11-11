"""GR_BitString::ReadBits 0x00423ef0, the shared bit reader (tools/emu_diff.py case).

The reader state is three retail globals (current word 0x00578e90, bits used 0x00578e94,
word cursor 0x00578e98); the mask table is retail's (pointer at 0x00568224). Counts 0..32
against every bits-used value, so reads stay inside the word, end exactly on it (no
refill shift) and cross into the next byte-swapped word. Retail is inline assembly.
"""
import struct

RETAIL_VA = 0x00423ef0
SYMBOL = '?ReadBits@GR_BitString@@QAEIH@Z'
SOURCE = 'samples/render/BitStringRead.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        used = rng.randrange(32)
        count = rng.choice([0, 1, 7, 8, 31, 32, 32 - used, rng.randrange(33)])
        count = min(max(count, 0), 32)
        yield (used, count, rng.randrange(1 << 32), rng.randrange(1 << 32))


def setup(ctx, case):
    used, count, word, nxt = case
    words = ctx.alloc(16)
    ctx.pack(words, 'II', 0xdeadbeef, nxt)
    ctx.pack(0x00578e90, 'IiI', word, used, words)
    this = ctx.alloc(0x20)
    return dict(ecx=this, args=[count])
