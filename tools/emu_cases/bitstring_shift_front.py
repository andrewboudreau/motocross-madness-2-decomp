"""GR_BitString::ShiftBitsTowardFront 0x004239c0, sub-word shift of a bit string (tools/emu_diff.py case).

Strings of 1..200 bits (bitCount +0x04, words at +0x08, fill word +0x18 zero or -1)
shifted by 1..31 bits; the words mix zero, all-ones and random values so all three arms
of the word loop run. Retail's high-bit mask table (0x00568098) is used by both bodies.
``bitstring_shift_back`` reuses the inputs for ShiftBitsTowardBack.
"""
import struct

RETAIL_VA = 0x004239c0
SYMBOL = '?ShiftBitsTowardFront@GR_BitString@@QAEXH@Z'
SOURCE = 'samples/render/BitStringShifts.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        bits = rng.choice([1, 31, 32, 33, 64, 65, 100, 200, rng.randrange(1, 257)])
        words = (bits + 31) // 32
        values = [rng.choice([0, 0xffffffff, rng.randrange(1 << 32), 1 << rng.randrange(32)])
                  for _ in range(words)]
        yield (bits, values, rng.randrange(1, 32), rng.choice([0, -1]))


def setup(ctx, case):
    bits, values, count, fill = case
    words = ctx.alloc(4 * len(values) + 8, struct.pack('<%dI' % len(values), *values) + b'\x77' * 8)
    this = ctx.alloc(0x20)
    ctx.pack(this + 4, 'iI', bits, words)
    ctx.pack(this + 0x18, 'i', fill)
    return dict(ecx=this, args=[count], ret_eax=False)
