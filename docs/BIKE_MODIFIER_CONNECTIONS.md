# Bike modifier connections

KrustyBike's ghost and rider-modifier paths now call six existing source
implementations. These are declaration connections; no provider body changed.

| Former call view | Existing provider | Retail entry / complete bytes |
| --- | --- | --- |
| `KbGhostMod` constructor | `GhostMod1` constructor | `0x47bb20` / 33 |
| `KbMorphMod` constructor | `MorphBastardModifier` constructor | `0x4a3150` / 48 |
| `KbObj128::StartFade` | `GhostMod1::StartFade` | `0x47bbf0` / 46 |
| `KbXform::AddModifier` | `D3DIMSoultreeObject::AddModifier` | `0x444d80` / 84 |
| `KbXform::RemoveModifier` | `D3DIMSoultreeObject::RemoveModifier` | `0x444de0` / 145 |
| `KbXform::AddSecondModifier` | `D3DIMSoultreeObject::AddSecondModifier` | `0x444eb0` / 84 |

## Receiver and lifetime evidence

The complete retail KrustyBike loader at `0x48fc80` allocates `0x48` bytes before
calling the GhostMod1 constructor and `0x58` before calling the morph constructor.
Those constructors install the corresponding retail vtables. RTTI from the
GhostMod1 table `0x553d70` and MorphBastardModifier table `0x555230` places their
GraphicsTest, D3DIMSoultreeModifier and GameObject bases at offset zero. The
existing fields and child-list calls therefore retain the original primary
pointer when the allocation uses the canonical class.

KrustyBike slot 50 attaches the same ghost modifier to the bike model and rider
model, then starts its fade. Tick removes it from both models. The loader attaches
the morph through the second list after loading its parameters and appending it
as a GameObject child. These eleven direct call sites retain every original
instruction and stack argument under the six symbol substitutions.

The model receiver is the primary D3DIMSoultreeObject pointer. Its RTTI table
`0x5513ec` identifies a distinct GameObject subobject at `+0xc`. The explicit model
cast preserves the primary address; the modifier argument is a GameObject pointer
at offset zero. The existing physics model declaration receives only the three
nonvirtual method declarations. This does not assert equivalence of every
provisional `KbXform` operation or consolidate the physics and rendering class
views.

Canonical GhostMod1, MorphBastardModifier and D3DIMSoultreeModifier declarations
live in narrow `*Core.h` headers. Their previous headers retain their includes and
expose the same declarations. This keeps the physics caller from importing the
separate complete rendering SoultreeObject declaration. Class fields, bases,
virtual order and method signatures are unchanged.

`KbMorphMod::LoadParameterFile` still has no emitted complete source provider;
its call and provisional return view remain unresolved. The morph's virtual
deformation and controller-angle dependencies also remain missing. This slice
does not establish complete rider deformation, ghost rendering or racing.

## Validation

`tests/test_bike_modifier_connections.py` verifies the retail RTTI and constructor
call/allocation evidence, both caller-view sizes, the offset-zero modifier
conversions and the model's `+0xc` GameObject conversion. Authentic VC6 SP3 retains
**157 strict comparisons** and **six complete sample partial fingerprints**.

The native fixture executes seven unchanged production methods extracted into
temporary units. Each extracted body and its actual production-TU body must
independently match the entire retail extent with relocations resolved (**14
checks**). The fixture covers **3,712 attachment, removal and fade cases**:
first/second lists, multiple models and modifiers, removal from either end,
reverse model order, bidirectional owner pointers/counts, list cleanup and
unchanged surrounding object storage. Fade cases include signed zero, positive
and negative values, infinities and a subnormal. Only the debug allocator boundary
is supplied by the fixture. Allocation failures, malformed or duplicate lists,
rendering and parameter loading are outside this native check.

Private before/after objects for KrustyBike, its near-miss aggregate, GhostMod,
MorphBastardModifier, D3DIMSoultreeModifier and D3DIMSoulTree preserve all **717
nondebug sections** and their raw bytes, flags and relocation destinations under
the six external renames. This includes all caller bodies, exception metadata,
initializer storage and provider bodies; no function extent is trimmed.

The isolated real source build compiled **210/210** translation units and reduced
unresolved names from **453 to 447**, exactly the six names above, with no additions.
The source-built game still does not link.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_bike_modifier_connections.py
```
