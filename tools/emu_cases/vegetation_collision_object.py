"""Vegetation::GetCollisionObject 0x004570a0, the placed vegetation collision shape (tools/emu_diff.py case).

The game links this body (samples/ecosystem/VegetationCollisionObject.cpp) behind the BikeAI
view GetObject@BikeAIVegetation. Random quantized positions, definitions and radius/height
parameters on both sides of 0x80 (both arms of the retail helpers 0x00455ff0/0x00455f90, which
run as retail code). Shape types 4 (sphere), 3 (capsule), 0 (hull; SetTransform 0x00435830 is
a stub that logs the 64-byte matrix it receives) and others.
"""
RETAIL_VA = 0x004570a0
SYMBOL = '?GetCollisionObject@Vegetation@@QAEPAVCollisionObject@@H@Z'
SOURCE = 'samples/ecosystem/VegetationCollisionObject.cpp'
ECO = 0x0059aebc


def cases(rng, n):
    for _ in range(n):
        yield dict(
            shape=rng.choice([4, 3, 0, 0, 1, 2]),
            index=rng.randrange(4),
            definition=rng.randrange(6),
            q=[rng.randrange(0x10000) for _ in range(3)],
            radius_param=rng.randrange(256), height_param=rng.randrange(256),
            unit=rng.choice([0.01, 0.125, rng.uniform(0.001, 0.5)]),
            radii=[rng.uniform(0.2, 5) for _ in range(3)],
            heights=[rng.uniform(0.5, 20) for _ in range(3)],
            matrix=[rng.uniform(-3, 3) for _ in range(16)],
        )


def setup(ctx, c):
    def sink(cx, ecx, a):
        cx.note(cx.read(a[0], 64).hex())
    ctx.stub_at(0x00435830, 'CollisionObject::SetTransform', pop=4, ret=sink)

    shape = ctx.alloc(0x200, fill=0x66)
    ctx.pack(shape + 0xc8, '16f', *c['matrix'])
    objects = ctx.alloc(16)
    obj = ctx.alloc(0x100, fill=0x44)
    ctx.pack(obj + 0x50, 'iI', c['shape'], shape)
    ctx.pack(objects + 4 * c['index'], 'I', obj)
    definition = ctx.alloc(0x220)
    # meanHeight, height low/high (0x180..0x188); meanRadius, radius low/high (0x18c..0x194).
    ctx.pack(definition + 0x180, '6f', c['heights'][0], c['heights'][1], c['heights'][2],
             c['radii'][0], c['radii'][1], c['radii'][2])
    ctx.pack(definition + 0x20c, 'I', objects)
    eco = ctx.alloc(0x600)
    ctx.pack(eco + 0x58 + 4 * c['definition'], 'I', definition)
    ctx.pack(eco + 0x5a8, 'f', c['unit'])
    ctx.pack(ECO, 'I', eco)
    veg = ctx.alloc(0x40, fill=0x22)
    ctx.pack(veg + 0x0c, '3HBBBB', *c['q'], c['definition'], 0, c['height_param'], c['radius_param'])
    return dict(ecx=veg, args=[c['index']])
