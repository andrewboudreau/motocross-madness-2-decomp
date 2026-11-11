"""Vehicle::Vehicle(int) 0x005257a0 (tools/emu_diff.py case).

A constructor with a virtual base: the hidden second argument selects the most-derived
path (vbtable pointers at +4 and +0x220, GameObject(1) at the virtual base +0x5c0). The
three calls are stubbed and logged: GameObject's constructor 0x00468ca0 (thiscall, one
argument), SoultreePhysicsCharacter's 0x00503c70 (thiscall, flags and 0) and
ResetDynamicState 0x00525550 (thiscall, 165.0f). Every field store, the vtable and vbase
displacement writes and the SEH registration at fs:[0] (page 0 mapped for this case) are
compared. The object starts filled with 0x5a; a non-most-derived call gets the retail
vbtable pointer pre-set, as a derived constructor would leave it.
"""

RETAIL_VA = 0x005257a0
SYMBOL = '??0Vehicle@@QAE@H@Z'
SOURCE = 'src/krusty2/vehicle/Vehicle.cpp'
PROBES = {0x005257ce: 'most derived', 0x005257f8: 'base constructor'}


def cases(rng, n):
    for _ in range(n):
        yield dict(flags=rng.choice([0, 1, 2, 0x10, rng.getrandbits(32)]), most_derived=rng.choice([0, 1]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    ctx.pack(0, 'I', 0x0012ff00)                 # previous SEH registration
    ctx.stub_at(0x00468ca0, 'GameObject::GameObject', pop=4, ret=lambda ctx, ecx, a: ecx)
    ctx.stub_at(0x00503c70, 'SoultreePhysicsCharacter::SoultreePhysicsCharacter', pop=8,
                ret=lambda ctx, ecx, a: ecx)
    ctx.stub_at(0x00525550, 'Vehicle::ResetDynamicState', pop=4)
    obj = ctx.alloc(0x700, fill=0x5a)
    if not c['most_derived']:
        ctx.pack(obj + 4, 'I', 0x00558c10)
    return dict(ecx=obj, args=[c['flags'], c['most_derived']])
