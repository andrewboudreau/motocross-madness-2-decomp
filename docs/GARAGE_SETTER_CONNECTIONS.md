# Garage scene setter connections

`KrustyUI::BuildGarageScene` uses the existing `SoultreeObject` setters for the
three garage, rider and manufacturer-bike model nodes. Six direct calls now
resolve to two complete source providers. No setter body, wrapper or alternate
node class is added. The remaining provisional character view is unchanged.

| Existing provider in `src/krusty2/soultree/soultree.cpp` | Retail VA | Complete extent |
| --- | --- | --- |
| `SoultreeObject::SetPosition(const Vec3&)` | `0x004fc660` | 38 bytes |
| `SoultreeObject::SetAxes(float,float,float,float,float,float,int,int)` | `0x004fbd10` | 82 bytes |

Both providers and the complete 3,114-byte garage caller match retail strictly,
with relocations resolved. `D3DIMSoulTree.h` already exposes the canonical
rendering declaration of `SoultreeObject` to this caller. It now also declares
its nonvirtual `SetAxes` method with the same signature as the physics view;
this does not change the class layout or virtual slots. `Vec3.h` supplies the
value layout without introducing Math3D.h's separate per-unit vector constants.

## Receiver and arguments

The garage's primary character slot 11 loads each model. The model creation
path in `D3DIMSoultreeCharacter::CharacterVirtualSlot0` calls the actual
`D3DIMSoultreeObject` constructor at `0x00445b29`, then stores its returned
primary address at character `+0x1a0` (`0x00445b45`). Only the subsequent,
separate `GameObject` attachment adds `0x0c` (`0x00445b74`). Clone construction
also stores the primary model pointer. Retail RTTI confirms
`D3DIMSoultreeObject -> SoultreeObject` at offset zero and its `GameObject`
base at offset `0x0c`. The setter calls retain the primary address loaded from
`+0x1a0`; no secondary-base adjustment belongs at these boundaries.

The three position calls are at `0x00499414`, `0x004995dc`, and `0x00499774`.
`Vector3` and `Vec3` are twelve-byte xyz records with components at `0/4/8`.
The old pointer and canonical reference pass the same address; the explicit
cast does not copy the value or broaden the provisional class identity claim.

The axes calls are at `0x0049943b`, `0x00499603`, and `0x00499795`. The old
view declared seven floats followed by an integer. The actual provider takes
six floats followed by **two integers**, `orthogonalize` and `keepZ`. At all
three callers the seventh old argument was literal `+0.0f`, represented by the
same four zero bytes as canonical integer `0`. The calls now pass `0, 1` for
those two flags. The full eight-word argument sequence and `ret 0x20` remain
unchanged. This evidence does not imply arbitrary float and integer arguments
are interchangeable.

## Validation

Run with the private retail image and authentic VC6 SP3 available:

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_garage_setter_connections -v
```

The regression checks 199 complete strict caller/provider cases, including
all 26 registered character-model producer methods. It preserves the complete
admitted nonexact `KrustyUI::Create` fingerprint (1,102-byte candidate;
872 matching positions against 1,104 retail bytes). Reconstructed callers
compile without an extra project include directory, as in their normal gate.

The native fixture executes unchanged extracted production definitions of
`SetPosition`, `SetAxes`, `SetAxesPtr`, `InvalidateWorldMatrix`,
`InvalidateSiblingChain`, and `MarkSubtreeDirty`. Each extracted body and its
actual full-production-TU body independently matches its complete retail
extent, making twelve strict checks. Their math dependencies are the actual
production `VectorCalls.cpp`, `CrossProduct.cpp`, and `FastMath.cpp`, including
initialization of the reciprocal-square-root table. No dependency is scripted.

The 8,192 native cases cover external and overlapping position input,
translation preservation, unit and nonunit axes, both flags, independent
numerical rotation checks, and cache invalidation across ancestors, children,
a grandchild and siblings. Full raw node storage is compared, including the
receiver's sibling that must remain untouched. Native declarations check
both scene views' field offsets, the actual provisional character's model
pointer offset, vector layout, and primary versus secondary conversions.
These are bounded setter tests, not an end-to-end garage loading test.

Private baseline/current comparisons also preserve every nondebug section and
relocation destination of `KrustyUI.cpp` and standalone `KrustyUINearMisses.cpp`:
326 sections and 1,853 relocations, under only the two external name changes.
Generated objects and comparison reports remain private under
`work/game-build/garage-setters32`.

The isolated real link in `work/game-build/build-8rs279x_` removes exactly
these two missing names (344 to 342). The final composition preserves all
224 prior production objects: 10,504 nondebug sections and 45,410 relocations,
under only these two substitutions. This includes every shared-header consumer.

The extra declaration also renumbers thirteen compiler-local labels in the
unadmitted BikeRace aggregate's Tick. An old-header/current-header comparison
preserves its complete 5,980-byte body, all 253 relocation destinations, and
all 206 nondebug aggregate sections with 1,929 relocations. The race-event
regression identifies defined local labels by their COFF section and offset;
label spelling is not destination identity. Bytes, addends and destinations
remain part of the fingerprint. Private evidence is
`work/game-build/garage-setters32/race-header-preservation.json`.

## Remaining boundary

`UnknownKrustyUIShadow::Init` stays unresolved. The corresponding
`ProjectedShadow::Init` body remains an unadmitted nonexact sample; changing
its call spelling would not supply a provider. Its texture/camera allocation,
renderer-state changes and cleanup paths require their own complete review.
Neither these setter connections nor strict neighboring shadow methods prove
that initialization path or complete startup, menus and racing.
