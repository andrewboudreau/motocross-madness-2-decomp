"""CarProcedural::PositionAtTime 0x00430b10, the path's Hermite position (tools/emu_diff.py case).

Paths of 2..9 keys (increasing times from 0 or later, positions at track scale) and times
inside, before, past and many periods outside the path, negative ones included (fmod and
the wrap). The cached key (+0x64) starts anywhere in the path, so both search loops run;
the first and last segments take the chord tangents. A count of 0 reads the key before the
array (padding) and returns the zero vector. A single key (period 0: fmod takes the CRT
error path, which faults in the emulator) is left out. HermiteWeights and fmod run as
retail code. The result goes through the hidden return pointer; the cached key's update
is compared.
"""
import struct

RETAIL_VA = 0x00430b10
SYMBOL = '?PositionAtTime@CarProcedural@@QAE?AUVector3@@M@Z'
SOURCE = 'src/bringup/Group2_CarProcedural.cpp'


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def cases(rng, n):
    for _ in range(n):
        count = rng.choice([0, 2, 3, 4, 6, 9])
        t = rng.choice([0.0, 0.0, rng.uniform(0, 3)])
        keys = []
        for _ in range(count):
            keys.append((t, rng.uniform(-500, 500), rng.uniform(-20, 40), rng.uniform(-500, 500)))
            t += rng.choice([rng.uniform(0.05, 4.0), 1.0, 0.5])
        speed = rng.choice([1.0, 2.0, 0.37, rng.uniform(0.1, 5.0)])
        inverse = f32(1.0 / speed)
        current = rng.randrange(max(count - 1, 1))
        period = (keys[-1][0] if keys else 1.0) * inverse
        time = rng.choice([rng.uniform(0, period), rng.uniform(-3 * period, 5 * period), 0.0,
                           period, -period, keys[rng.randrange(count)][0] * inverse if count else 0.0])
        yield (keys, speed, inverse, current, time)


def setup(ctx, case):
    keys, speed, inverse, current, time = case
    blob = struct.pack('<4f', 0.75, 1.0, 2.0, 3.0)   # the key before the array
    blob += b''.join(struct.pack('<4f', *k) for k in keys)
    array = ctx.alloc(len(blob) + 16, blob) + 16
    car = ctx.alloc(0x200)
    ctx.pack(car + 0x64, 'I', current)
    ctx.pack(car + 0x6c, 'I', len(keys))
    ctx.pack(car + 0x170, 'ff', speed, inverse)
    ctx.pack(car + 0x1b4, 'I', array)
    out = ctx.alloc(12, fill=0x77)
    return dict(ecx=car, args=[out, float(time)])
