# Steering axis from scene points

`SteeringControl::SetAxisFromPoints` is a missing `Bike::LoadBike` dependency. The
standalone candidate in `samples/physics/motion/SteeringAxisFromPoints.cpp` retains
the existing readable body unchanged. It obtains both nodes' world positions,
normalizes their difference and, for a nonnull frame, transforms and stores the
axis. A null frame returns zero **after** both position/cache operations and the
normalization arithmetic. This is a finite native behavior review, not an exact
match or an original translation-unit ownership claim.

## Retail identity and complete extent

The complete body is `0x00504d30..0x00504e19`, 234 bytes. Both returns pop twelve
bytes (`0x504e0e` and `0x504e17`); six padding NOPs follow before the next body at
`0x504e20`. All branches stay within the decoded extent. Its only calls are
`GetPositionIn` at `0x504d41` and `0x504d51`, targeting `0x4fc9a0`, and
`WorldToLocalDirection` at `0x504ded`, targeting `0x4fd710`. It uses inline x87
`FSQRT`, with no `_finite` guard, CRT square-root call, virtual call or EH handler.

The full 6,745-byte `Bike::LoadBike` caller contains the call at `0x408b6b`.
The receiver comes from `Bike+0x47c`; its three arguments come from the front-wheel
scene node (`Bike+0x5f0`, then `+0x1bc`), handlebars node (`Bike+0x60c`, then
`+0x40`) and frame (`Bike+0x42c`). The following angle operation consumes the axis.
The regression preserves the caller's complete existing partial body and all
219 relocation destinations; it does not relabel that caller exact.

`SteeringControl` has no RTTI/vptr. Its existing layout is 24 bytes: node at zero,
angle at four, maximum angle at eight and the three-float axis at twelve. The
strict constructor's literal `D:\aardvark\VC\krusty2\SteeringControl.cpp` at
`0x5745ac` supports source context. Names and the new standalone grouping retain
their reconstruction status. Raw primary `SoultreeObject` COL/type evidence and
layout are checked separately; no adjustment to its secondary GameObject view
is introduced.

## Source, helper and aggregate evidence

Authentic VC6 SP3 `vc6_o2_mt`, without additional flags, emits the complete
230-byte candidate: 145 matching positions against all 234 retail bytes.
The historical x87 scheduling difference remains. No tolerance comparison,
matching-only rewrite or behavioral correction was needed for the tested cases.

The source and reference each execute their own real scene/cache math:

| Helper | Retail extent | Evidence |
| --- | ---: | --- |
| `GetPositionIn` at `0x4fc9a0` | 134 | Complete strict canonical and extracted validation bodies |
| `UpdateWorldMatrix` at `0x4fb4f0` | 858 | Complete strict canonical and extracted validation bodies |
| `WorldToLocalPoint` at `0x4fd7f0` | 285 | Complete strict canonical and extracted validation bodies |
| `WorldToLocalDirection` at `0x4fd710` | 209 | Existing reviewed nonexact standalone, 207 matching positions |

The first three definitions and their original inline math helpers are copied
verbatim into a temporary validation-only TU and independently compared against
both the complete canonical object and retail before linking. The actual whole
`SoultreeTransform.cpp` supplies the direction method. Its existing two-byte
operand-order difference is retained; this review does not make it universally
exact. There are **no reference call patches, mocked math, CRT substitutions or
scripted callbacks** in this fixture. The reference runs its complete retail
helpers. Production never maps or dispatches to retail code.

Before extraction, the aggregate was compiled at its actual existing pathname.
All 32 nondebug sections and 90 relocations, including every other function,
initializer, storage payload/flag and destination/addend, remain identical after
replacing only this body with the standalone include. Private same-path proof:
`work/connections43/aggregate-proof.json`, reproduced by `prove.py` from
`before.obj` and `after.obj`, with no exceptions. The retained regression pins the
same complete object, normalizing only the known canonical `__FILE__` checkout
prefix for portability. The full candidate body/relocations also equal those in
the aggregate and the actual `tools.build_game.compile_source` command.

The ordinary Math3D header emits eight private vector initializer functions in
the new standalone. Their full bodies/relocations equal the corresponding
aggregate initializers. They do not establish original TU ownership; the
candidate's only external dependencies are the two existing scene methods and
VC6's `__fltused`. The retained checks preserve all 21 existing strict steering
functions/initializers in both canonical and aggregate objects (42 checks), plus
the three dual-checked scene helpers (six checks).

## Native validation

`tests/test_steering_axis_from_points.py` and
`tests/fixtures/steering_axis_from_points_compare.cpp` pass **48 strict checks and
100,852 native scenarios**. The private reference is pinned to SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
The actual separately compiled candidate executes at ordinary source addresses;
only the validation executable reserves space for the reference image.

The matrix contains:

- 98,304 combinations: 16 deterministic seeds, eight translation/matrix
  families, four acyclic parent arrangements, four frame selections, four cache
  masks, four rounding modes and three precision modes.
- 1,152 prepared receiver/output aliases into local matrix, cached world matrix
  or frame cache fields; 640 null/invalid/equal-node combinations.
- 172 progressive guard-page cases for source node, frame and axis output.
- 576 cases unmasking each of six FP exception classes individually, across eight
  families and all twelve rounding/precision combinations.
- Eight independent result/cache/fault oracles: positive X, negative Z, rotated
  positive Y, null-frame unchanged axis, null receiver with unused output, and
  output faults after zero, one or two successfully stored floats.

Finite inputs include ordinary signed values, exact and near-unit differences,
equal positions, subnormals, large values causing intermediate overflow and
varied raw finite float bit patterns. Zero normalization and finite overflow may
produce nonfinite intermediate/results; their actual bits are compared. Frames
include identity, rotation and nonorthogonal matrices. Parents are shallow and
acyclic; both stale and already-valid world caches execute real helper behavior.

Each run compares every byte of all four prepared nodes, steering storage and
surrounding guards, the full readable guard page, return result, exception code,
access kind/address, x87 control/status and the low six fault status flags plus
fault control word. Success oracles independently require no exception. Partial
output oracles require the expected write AV/address and exact written prefix.
No floating tolerance is used.

This is bounded prepared-storage evidence. It does not establish arbitrary C++
object lifetime/alias validity, cyclic hierarchy behavior, all NaN payloads or
all FP exception-mask combinations. Exception context comparison excludes x87
TOP/condition bits, register-stack payloads, fault instruction address and
resumption equivalence. Each exception is caught and the FP environment reset;
no in-place continuation is attempted. There is no real game startup, racing or
input-device integration claim.

## Functional build review

The unchanged standalone is explicitly admitted as a reviewed nonexact source
build candidate. It supplies the missing steering operation using existing
source providers; it remains in `samples/`. The final real link in
`work/game-build/build-55x25rcs/report.json` compiles 245 translation units and removes exactly this dependency, **272 -> 271**,
with no new unresolved names. A successful native slice does not establish a
linked or playable game.

Reproduce on Windows with authentic VC6 SP3 and the private retail executable:

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_steering_axis_from_points.py -v
```
