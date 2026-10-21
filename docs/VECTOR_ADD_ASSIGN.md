# In-place vector addition

This slice supplies the existing BVH `Vec3AddAssignCall::operator+=` call view
with ordinary C++, and connects Vehicle's two addition sites to the canonical
`Vec3::operator+=`. Each emitted provider matches the complete **33 bytes** at
`0x00428060`. The call-view names and reconstruction grouping are provisional;
shared code bytes alone do not establish a common original class or file.

## Source and call boundaries

`src/reconstructed/VectorAddAssign.cpp` implements three sequential component
additions followed by `return *this`. Its narrow header holds the existing
BVH view declaration, moved verbatim from the query header at the same declaration
position. It adds no fields, virtual functions or inheritance changes. The
provider uses ordinary `vc6_o2_mt`, has no relocations or initializers, exports
one method, and imports only `__fltused`.

The canonical inline Vec3 implementation already emits an exact out-of-line
COMDAT in CollisionObject. The natural Vehicle expressions `*up += scratchVector`
and `*zero += scratchVector` retain both calls and naturally emit the same exact
canonical COMDAT in Vehicle. The superseded Vehicle view declaration is removed.
There is no wrapper or linker alias.

| Caller | Complete retail body | Calls to 0x00428060 |
| --- | --- | --- |
| `LeafPairQuery` | 0x004275f0 / 736 bytes, strict | 0x0042786a |
| `Vehicle::Method_00529450` | 0x00529450 / 1,415 bytes, strict | 0x0052993b, 0x005299ae |

Retail copies ECX to EAX, reads one source pointer from `[esp+4]`, updates
x/y/z at offsets 0/4/8 and returns with `ret 4`. EAX remains the receiver.
The BVH caller initializes its 12-byte local hit vector from a scaled direction,
adds the transformed start point, then passes that same local to PointInTriangle.
Vehicle's receiver addresses are its `up` and `zero` vector parameters; the
source is its scratch vector after force projection or moment transformation.
These are vector-storage boundaries, not pointers to whole Vehicle objects.
The complete strict callers preserve all pointer formation, argument pushes,
post-call use and surrounding control flow.

The order is observable: x is read and written before y is read, and y is written
before z is read. Shifted overlap may therefore propagate an earlier write into
a later source read. A later inaccessible input can fault after an earlier
component was already stored. An out-of-place add followed by a copy would not
preserve this behavior.

## Complete preservation evidence

Private trials first used ordinary canonical expressions at all three sites.
Vehicle retained its complete strict method; the BVH expression inlined and
changed its strict 736-byte body to 755 bytes. Only Vehicle's natural connection
was retained. The BVH view receives its own readable implementation while its
original call site remains unchanged.

The four affected translation units retain all **257 old nondebug sections and
1,709 relocations**: BoundingBoxTreeQuery, Vehicle, CollisionTransforms and the
separate BoundingBoxTreeQueryNearMisses aggregate. Every old section payload,
flag and relocation location/type/addend/destination is preserved. The two
Vehicle relocation destinations change from the obsolete external view to the
verified canonical operator. Vehicle adds one natural 33-byte COMDAT; that
complete body is checked separately against the existing canonical provider
and retail. Removing the unused declaration also preserves every old section.
No prior strict body or partial function changes.
The isolated real source build removes exactly two unresolved names, **299 to
297**, with no newly unresolved symbols. This is link progress, not a claim that
the reconstructed game executable is complete.

`tests/fixtures/vector_add_assign_objects.json` pins the complete old objects.
Defined targets retain section identity and offset; the sole Vehicle function
join is explicit. Complete recognized `__FILE__` literals use their source
suffix for checkout portability. Actual same-path before/after comparisons
require all raw bytes and do not need a filename exception.

The existing gearbox regression previously used numeric COFF section indices
for compiler-local labels. Vehicle's new public COMDAT shifted those indices
without changing the destinations. Its fingerprint now uses public section
symbol identity; anonymous destinations retain their anonymous-section ordinal,
flags, complete payload and recursive relocation destinations. Existing source
filename normalization also applies through EH-to-filename references.
All **28** published/current raw bodies and newly normalized fingerprints were
independently equal before nine representation digests were updated. Extents
and retail matching scores remain unchanged. The existing unit regression
protects public renumbering, distinct destinations/offsets, anonymous payload and
relocation changes, external names, and insertion of an unrelated public COMDAT.

## Native validation

`tests/test_vector_add_assign.py` passes **127 strict checks**, verifies the
three actual caller relocations and protects the four complete old objects.
The native fixture links the actual standalone BVH provider. It separately takes
the actual canonical header member's address to emit a normal canonical COMDAT;
that complete 33-byte emission must equal both production emissions in Vehicle
and CollisionObject and retail. This validation-only address use is not a
production mechanism to force call shape.

The fixture uses the decoded thiscall/member-pointer boundary and requires the
returned reference to identify the receiver. Layout checks require 12-byte xyz
storage and a zero-offset Vec3 base. Only the private executable maps the
hash-pinned retail body. Neither production code nor the tested objects dispatch
to retail; no arithmetic is scripted or candidate bytes patched.

The **34,014** native comparisons comprise:

- 33,792 arithmetic/overlap cases: both provider forms, 128 edge/raw-bit seeds,
  11 output positions, four rounding modes and three x87 precision modes.
- 216 access-fault cases: both providers, nine null/dual-invalid/progressive
  guard-page modes and all twelve rounding/precision combinations.
- Six independent outcomes: for each provider, shifted overlap must propagate
  `1,2,3,4` into `3,6,10`; a dual-null call must read-fault at zero; and a
  second-component input fault must retain the completed first-component store.

Each case compares all 32 backing words, the complete committed guard page,
returned address, FP status/control, access-fault kind/address and the six FP
exception-status bits saved at the fault. Inputs include signed zeros,
denormals, finite extremes, infinities and NaNs. Floating-point exceptions are
masked. Complete saved register context, unmasked FP traps, resuming faults,
real object lifetimes, full BVH traversal and whole Vehicle simulation are not
claimed by this isolated native fixture.

The existing gearbox suite also passes **292 strict bodies, 28 complete partial
fingerprints and 48 native cases** with the strengthened destination identities.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_vector_add_assign.py -v
python -m unittest discover -s tests -p test_vehicle_gearbox_connections.py -v
```

Private reconstruction and whole-object evidence is under
`work/vector-connections40/`, including `implementation-proof.json`,
`vehicle-cleanup.obj` and `gearbox/normalized-proof.json`. These artifacts are
not prerequisites for the retained regressions. Source linking and actual
startup, menus and racing remain separate completion gates.
