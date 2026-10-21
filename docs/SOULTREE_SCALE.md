# Rendering scene scale

## Functional build review

`samples/render/D3DIMScale.cpp` supplies the existing rendering call view of
the complete scale operation at `0x004fd340`. VC6 SP3 `vc6_o2_mt` emits 630
bytes, with 618 resolved matching positions. It remains reviewed nonexact:
floating-point operand ordering differs in twelve byte positions. No flags,
link aliases, wrappers, or reference patches compensate for that difference.

The body uses the existing thirteen-store matrix product, extracted verbatim
from `samples/physics/helpers/SoultreeBounds.cpp` into
`SoultreeScaleProduct.h`. The original aggregate preserves all 14 nondebug
sections and 32 relocations, including every byte, flag and destination.
The new rendering provider has only directives and text sections, with no
data, BSS or startup initializer. Its complete emitted body and relocations
equal the existing base-class implementation. Original TU ownership and the
semantic naming of shared layout fields remain inferred.

## Receiver and operation

The two unchanged real callers construct a D3DIM model through `0x0043f160`
and retain its primary pointer: Arcade stores it at `+0x2c`, LightSource at
`+0x98`. Their calls at `0x0040146c` and `0x0049e2eb` pass that pointer directly
to scale. The complete 275-byte constructor and both complete caller target
sets remain strict. Raw model RTTI confirms primary object offset zero and a
distinct GameObject view at offset twelve; no secondary-pointer adjustment is
introduced. The retained regression checks the constructor, stores, reloads,
argument preparation and direct call instructions.

The method snapshots all 64 matrix bytes, forms a diagonal scale matrix, then
writes offsets 0 through 44 and offset 60 in the observed order. Translation
bytes at offsets 48 through 59 remain untouched. Zero products remain explicit
because removing them would change exceptional floating-point behavior.
The method invokes the actual cache invalidation operation at `0x004fb850`.

## Validation and limits

`tests/test_soultree_scale.py` passes 43 strict caller/helper comparisons and
99,827 native comparisons against the hash-pinned retail operation. Both sides
execute their own actual invalidation, sibling and subtree helpers. Extracted
helpers are independently strict and equal their complete canonical emissions.
The test also independently recompiles the candidate with the game command
and compares every nondebug section and relocation.

The native cases include ordinary and finite edge inputs, selected infinities
and NaNs, deterministic raw finite values, all four rounding modes and three
precision modes, cache masks, matrix-derived arguments, protected node/storage
pages, and individually unmasked exception classes. It compares prepared
node/guard bytes, cache state, floating-point control/status, and fault kind,
address and observable memory. Independent oracles check diagonal scaling,
cache propagation, a progressive write fault, and the null-receiver read fault.

These finite cases do not prove all floating-point inputs equivalent. Fault
instruction addresses, the saved x87 register stack/TOP, and exception resumption
are outside the comparison. Prepared raw storage does not exercise construction,
destruction, scene loading or gameplay. The game still must link before startup,
menus and racing can be tested.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_soultree_scale -v
```
