# Vector value arithmetic providers

`src/reconstructed/VectorValueArithmetic.cpp` supplies three existing call views
with ordinary C++ bodies. VC6 SP3 `vc6_o2_mt` emits complete strict 69-byte
functions, without relocations, for each:

| Existing call view | Retail address | Operation |
|---|---|---|
| `SegmentAdd` | `0x00421cb0` | Component-wise addition |
| `SegmentSubtract` | `0x00421d00` | Component-wise subtraction |
| `WreckerSubtractValue` | `0x00421d00` | Component-wise subtraction |

These names and the new source grouping are provisional. Two declarations
refer to the same retail subtraction body; this does not establish two retail
functions or original TU ownership. The existing `Vec3` and `Vector3` views
both have three floats at offsets 0, 4 and 8 and size 12. The code supplies
actual definitions for those declared views, without linker aliases or retail
dispatch. No caller or shared header changes are needed.

Both retail functions use cdecl with a hidden result pointer followed by two
input references, return the result pointer in EAX, and end at their complete
69-byte COFF extents. Eleven alignment NOPs follow each retail return. All
input components are read before the first result write; x is written before
the final x87 temporary store, followed by y and z. This ordering matters for
overlap, exceptions and progressive output faults.

`SegmentTouchesObject` at `0x00439820` calls subtraction three times and
addition twice. Its complete 1,493-byte candidate remains nonexact at
1,309 matching positions after resolving relocations; its existing stack-slot
and scheduling differences are unchanged. `Wrecker::Tick` at `0x005306e0`
retains all 4,178 strict bytes, including its subtraction call at `0x005316d7`.
The three pushed arguments and caller cleanup establish the hidden-result ABI
independently of the symbol names. Existing whole objects are also compared
against the published baseline in the integration proof.

`tests/test_vector_values.py` recompiles all three providers and both callers,
checks complete extents and all six call relocations, and runs the actual
providers against the complete hash-pinned retail bodies. The validation-only
fixture checks 152,535 cases: ordinary C++ return expressions with independent
numeric expectations; overlapping results and inputs; signed zero, subnormal,
infinity, NaN and deterministic raw-bit inputs; all four rounding modes and
three precision modes; null references/results; and progressive input/output
access faults. It compares complete storage, returned pointer, floating-point
status/control, exception kind/address and partial writes. Floating-point
exceptions are masked in this fixture. This is finite helper validation, not
integrated collision or racing execution.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_vector_values -v
```

The isolated real link compiles 243 translation units and removes exactly
these three unresolved names, **275 -> 272**, with no new missing dependencies
(`work/game-build/build-2u175dm8/report.json`). The game remains unlinked.
