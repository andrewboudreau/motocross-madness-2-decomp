# InlineShock axis setup

`src/reconstructed/InlineShockAxis.cpp` supplies the complete 250-byte
`InlineShock::UpdateAxis` body at `0x004f9f90`. Authentic VC6 SP3 with
`vc6_o2_mt` reproduces every byte with resolved relocations, including both
`ret 8` exits and all conditional branches. The following six bytes are padding;
`ClampAndStep` starts at `0x004fa090`.

Bike loading now calls this actual declaration through the primary receiver.
The isolated source build compiled 246 translation units and reduced unresolved
names from 271 to 270, removing only
`?UpdateAxis@VehicleInlineShock@@QAEXPAVSoultreeObject@@0@Z`.
This supplies one loading dependency; it does not establish a runnable game or
promote the rest of the suspension implementation.

## Receiver and argument evidence

The complete 6,745-byte `Bike::LoadBike` body allocates `0xd8` bytes at `0x408258`,
passes that allocation unchanged in ECX to the constructor at `0x4082b3`, and
retains the returned primary pointer. It registers the object as a late ticker
at `0x4082ce`, passes it to `AddWheel` at `0x408652`, and uses that same pointer
as the receiver at `0x4086a8`. The two arguments come from the wheel's scene-node
field at `+0x1bc` and the handlebar's node field at `+0x40`; neither argument is
adjusted by the connection.

The complete 153-byte constructor at `0x004f9ee0` writes primary vtable
`0x00557b90`, initializes `minLength` at `+0xcc`, the anchor pointer at `+0xd0`,
and `axisLength` at `+0xd4`, and addresses the three axis floats at `+0xc0`.
That vtable's raw COL is `0x0055ef30`, has object offset zero, and names
`.?AVInlineShock@@`. The parsed hierarchy independently places InlineShock and
Shock at zero and the non-polymorphic MovingPart base at `+4`. The Bike cast
retains the primary pointer; it is not a cast to MovingPart.

The declarations in `Suspension.h` moved unchanged from the sample directory to
`src/reconstructed/`; the old path is an include shim. The reconstructed header
keeps the existing class, base, field and method declarations. Semantic field
names and original header/TU ownership remain provisional. The new source
filename is ours. No equivalence claim is made for every member of the older
`VehicleInlineShock` view, and its constructor declaration remains unchanged.

The two helper calls at `0x004f9fa1` and `0x004f9fb1` are actual
`SoultreeObject::GetPositionIn` at `0x004fc9a0`. Only these calls cast the old
ShockNode/three-float output views to the canonical scene/Vec3 boundary. Native
layout assertions retain 12-byte `x/y/z` records and the verified `0xd8` shock
layout. Raw scene RTTI confirms the primary SoultreeObject offset is zero.

## Source and preservation

The operation reads both world positions, stores their difference, calculates
its magnitude, stores that magnitude while forming the reciprocal, normalizes
the axis, and lowers `minLength` only when the measured magnitude is smaller.
The existing expression `axis * (1.0f / (axisLength = len))` retains the observed
store and rounding order. It uses the inline `sqrt` instruction; there is no
new math wrapper, forced link alias, or copied implementation bytes.

The sample's existing method body now includes the canonical source. All 27
registered suspension targets remain strict, including its constructors,
other methods and private vector initializers. Same-path before/after proofs
preserve every nondebug section byte, flag, relocation kind, addend and resolved
local destination:

| Existing object | Sections | Relocations | Intended changed external references |
| --- | ---: | ---: | --- |
| Bike | 242 | 1,089 | One UpdateAxis call |
| Shock sample aggregate | 56 | 219 | Two GetPositionIn calls inside UpdateAxis |

There are no function-byte, extent, partial-score, storage or initializer
exceptions. The retained whole-object fingerprints normalize only the known
checkout prefix in source filenames and the two reviewed symbol spellings.
Other sample methods keep their old GetPositionIn call view and binding.

The new provider has exactly three nondebug sections: compiler directives,
256 bytes of text storage containing the complete 250-byte method, and a
four-byte read-only `1.0f` constant. The test compares that constant's bytes to
retail and checks the section flags. There is no BSS, CRT initializer table, or
additional function in this provider. Moving the declaration does not move the
sample's four vectors or their startup functions into the game.

## Native validation

`tests/test_inline_shock_axis.py` checks **87 complete strict comparisons**:
52 Bike targets, 27 suspension targets, both copies of the promoted method, and
canonical/extracted copies of three scene helpers. The helper extraction keeps
actual source text and is separately byte/relocation-identical to the full
canonical object: GetPositionIn (134 bytes), UpdateWorldMatrix (858 bytes), and
WorldToLocalPoint (285 bytes).

The fixture links the actual standalone provider and those actual helpers.
The hash-pinned retail comparison executes the original complete method and
its own helpers without patches or scripted math. **149,508 cases** compare
all prepared shock/node/guard bytes, FP status/control, and access-fault kind,
address and state at the fault:

- 147,456 combinations of 16 seeds, eight input families, four acyclic parent
  arrangements, four cache-validity patterns, six prior minimum lengths, four
  rounding modes and three precision modes;
- 1,728 aliases placing the axis over local/world matrix storage, 128 null or
  identical-input cases, and 188 page-boundary cases covering both node reads
  and progressive shock writes;
- eight independent outcome checks for minimum-length reduction, the exact
  unit-length branch, unchanged object guards, and a partial axis-write fault.

Inputs include zero, subnormal, near-unit, very large and deterministic finite
raw-bit values; arithmetic may produce infinities and NaNs. Floating-point
exceptions are masked. This finite comparison does not establish arbitrary
unmasked exception behavior, the complete saved x87 register stack, cyclic
scene graphs, constructors/destructors, or full Bike loading. The method passes
a null reference frame, so the native run does not claim coverage of the
non-null-frame branch inside GetPositionIn merely because WorldToLocalPoint is
linked and independently strict.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_inline_shock_axis -v
```
