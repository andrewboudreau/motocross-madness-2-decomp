# Wheel attachment update

`src/reconstructed/VehicleWheelAttachment.cpp` supplies the existing
`VehicleWheel::UpdateAttachment()` call view used by Bike's per-wheel update.
Authentic VC6 SP3 `vc6_o2_mt` emits all **195 strict bytes** at `0x00515660`, with
all four call relocations resolved. No caller changes, forwarding wrapper,
linker aliases or special compiler flags are involved.

The existing `Tire::UpdateAttachment()` sample is another source view of the
same retail body. Its body stays unchanged in `samples/physics/tire/Tire.cpp`.
Only the two existing attachment-field class declarations move verbatim to
`src/reconstructed/TireAttachmentViews.h`; the sample header includes that
shared declaration. This does not promote the rest of Tire, its constructors,
its provisional MovingPart declaration or the suspension hierarchy. Semantic
field/method names and original TU/header ownership remain provisional.

## Identity, receiver and complete behavior

The full retail extent is `0x00515660..0x00515722`, including both returns at
`0x5156bd` and `0x515722`. Thirteen padding NOPs follow. Every instruction and
internal branch is checked. There is no virtual call, jump table or EH handler.

The primary Tire table is `0x558578`, with raw COL `0x55f5e0`, object offset zero
and type `.?AVTire@@`. RTTI distinguishes the primary CollisionObject/QuadTree
base from GraphicsTest/GameObject/BaseObject at `+12`, MovingPart at `+380` and
CollisionPoint at `+184`. The full 1,168-byte constructor decode writes the
primary table at `0x513120`, GameObject table at `0x51312a` and CollisionPoint
table at `0x513135`.

Bike::LoadBike calls that constructor at `0x4084d7`, saves its primary EAX result
in ESI, and passes ESI to Vehicle::AddWheel at `0x408548`. AddWheel stores the
primary pointer into `wheelList` at `0x525d78` **before** adding `0xb8` solely for
the separate collision-contact registration. It stores attachments at `+0x2b0`
and `+0x2ac` and increments the wheel count. Bike slot41 reloads the primary
array entry directly into ECX at `0x40cd9e` and calls this body at `0x40cda1`.
The complete 160-byte AddWheel and 486-byte slot41 are independently strict;
this operation never consumes the secondary contact pointer.

The body prioritizes `primaryAux` (`+0x2b0`). With this attachment it computes
`-position * direction`, then passes the resulting vector by value to its scene
node. Only when that pointer is null does it inspect `secondaryAux` (`+0x2ac`),
compute `position / scale`, and test whether the quotient is finite. A finite
quotient causes the node's rest matrix to be restored and then rotated about
the attachment's axis. Both-null does nothing. The decoded reads and writes,
including fault order, are retained without added validation or cleanup.

Shared provisional attachment views preserve the observed fields:

| View | Relevant offsets |
| --- | --- |
| Inline attachment | node `0x44`, position `0x98`, direction `0xc0` |
| Rotating attachment | rest matrix `0x04`, node `0x44`, position `0x98`, scale `0xc8`, axis `0xd4` |

The new body casts only these existing integer/pointer fields and compatible
three-float/64-byte matrix views at the real scene-method boundary. Compiler
checks protect their field offsets, sizes, the wheel's `0x2c0` extent and both
attachment-pointer offsets. These checks do not equate all provisional fields
with a complete original C++ class.

## Real source dependencies

The four direct retail calls are:

| Call site | Target | Complete provider |
| --- | --- | --- |
| `0x5156b4` | `0x4fc630` | Actual by-value Soultree position setter, 38 strict bytes |
| `0x5156de` | `0x534d07` | Authentic VC6 `_finite`, 20 bytes identical to retail |
| `0x5156f1` | `0x4fca30` | Actual SetLocalMatrix, 33 strict bytes |
| `0x515719` | `0x4fceb0` | Actual Rotate, 465 strict bytes |

Rotation further executes the actual 684-byte MatrixMultiply and the genuine
cache methods InvalidateWorldMatrix (45), InvalidateSiblingChain (50) and
MarkSubtreeDirty (38). All are strict. The native fixture extracts the original
scene definitions and inline AxisAngleMatrix, plus MatrixMultiply, into private
validation TUs. Every complete extracted provider independently matches both
the canonical full-source object and retail bytes/relocations before execution.
The actual complete SoultreePositionValue.cpp object is linked directly.

The reference executes its own unmodified retail helpers. The source executes
actual source math and the linked authentic CRT. The test verifies `_finite`
from the linked executable against all 20 retail bytes. There are no reference
call patches, scripted helpers, replaced constants or mocked math. Production
contains no retail mapping or dispatch.

## Whole objects and header initialization

The original Tire aggregate is completely unchanged after the declaration
extraction: **82 nondebug sections and 441 relocations**, with no symbol joins.
The already-composed Bike object is also unchanged: **242 sections and 1,089
relocations**. Every function, partial body, static initializer, storage payload,
section flag, relocation location/addend and destination remains protected.
The private same-path comparison is in `work/tire44/`; the retained pins are in
`tests/fixtures/wheel_attachment_objects.json`. Only Bike's known complete
`__FILE__` checkout prefix is normalized for portable retained hashes. Parent
integration compares actual old production objects without filename exclusions.

The new provider's ordinary Vehicle/Math3D includes emit four additional private
vector objects in 60 bytes of writable zero-filled BSS, including alignment:
zero at offset0, Z at16, X at32 and Y at48. Its 16-byte `.CRT$XCU` table points to
four local initialization thunks. All eight complete thunk/body functions equal
the existing canonical header emissions; the latter are independently strict
against their retail targets. Tests protect the local symbols/offsets, writable
flags, complete CRT bytes, relocation entries and initializer destinations.
This is reconstructed-TU overhead, not evidence of original TU ownership or
aliases for existing game-global vectors. The new object has twelve nondebug
sections. Its external dependencies are the three genuine scene methods,
`_finite` and `__fltused`.

## Native validation

`tests/test_wheel_attachment.py` and
`tests/fixtures/wheel_attachment_compare.cpp` pass **48 strict comparisons**,
two complete-object pins, and **47,006 native scenarios**. The separately
compiled actual provider object also equals the complete output of the real
game-build compiler command. The retail image is pinned to SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

The scenario count comprises:

- 36,864 combinations of 24 seeds, four attachment modes, four value/matrix
  families, eight cache masks, four rounding modes and three precision modes.
- 9,216 prepared attachment/node aliases, including direction overlapping local
  matrix data and a rest matrix overlapping the destination node's local matrix.
- 320 null/invalid wheel, attachment and node combinations.
- 600 progressive guard-page cases across 25 cut points in attachment or node
  storage, both attachment paths and all four rounding modes.
- Six independent outcome checks: slider result/cache changes and primary
  precedence, both-null unchanged state, nonfinite quotient with an unused null
  node, zero-angle rest-matrix restoration, a later axis read fault after real
  matrix/cache changes, and a second-component write fault retaining the first
  translation component.

Values include signed zero, subnormal, finite extreme, infinity and NaN bit
patterns, deterministic raw finite values, ordinary rational values and
nonorthogonal rest matrices. Every scenario compares complete wheel,
attachments, all three prepared nodes, guards and readable fault-page bytes,
exception code/kind/address, x87 status/control and the low six fault-status
flags. No numerical tolerance is used. Success oracles independently require
no exception; partial-fault oracles check the expected access kind/address and
surviving writes/cache effects.

This is bounded prepared-storage evidence. No constructors, object lifetime,
wheel solver, running game scene or arbitrary malformed hierarchy are exercised.
All floating-point exceptions are masked. Transient FP TOP/condition bits,
register-stack payloads, unmasked traps, resumption and arbitrary alias/NaN
behavior remain outside the finite claim. The fixture invokes the complete
attachment method and real helpers; the complete Bike caller is protected by
strict/whole-object evidence rather than executed as part of a running bike.

## Functional source build

The canonical provider supplies the existing wheel call without changing its
caller or admitting the rest of the Tire sample. The source build's actual link
result is recorded by parent integration. Complete game linkage, startup and
racing remain separate unmet gates.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_wheel_attachment.py -v
```
