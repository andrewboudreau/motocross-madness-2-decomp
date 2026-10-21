# Garage shadow provider connections

The garage builder, `KrustyUI::BuildGarageScene` at `0x00498cf0`, now calls
the existing `ProjectedShadow` and `D3DIMSoultreeShadow` implementations.
The source uses their canonical declarations and explicit pointer conversions
at the remaining provisional-view boundaries. There are no new method bodies,
forwarding wrappers, or linker aliases.

| Existing provider | Retail VA | Complete VC6 extent |
| --- | --- | --- |
| `ProjectedShadow::ProjectedShadow(int)` | `0x004da570` | 331 bytes |
| `D3DIMSoultreeShadow::D3DIMSoultreeShadow(int)` | `0x00446840` | 67 bytes |
| `D3DIMSoultreeShadow::Attach` | `0x004468b0` | 58 bytes |
| `ProjectedShadow::AddCaster` | `0x004dab00` | 135 bytes |
| `ProjectedShadow::SetShadowStrength` | `0x004dace0` | 336 bytes |
| `ProjectedShadow::SetShadowGrey` | `0x004dae30` | 31 bytes |
| `ProjectedShadow::SetLight` | `0x004dae50` | 365 bytes |

All seven production providers match their entire retail extents strictly,
including resolved relocations. Their existing shadow source attribution is
documented in the provider files. The semantic method names remain inferred.

## Receiver and argument evidence

The complete 3,114-byte retail garage builder allocates `0x134` bytes for the
projected shadow and `0x48` for its receiver, calls the two constructors above,
and calls `Attach` before appending the returned object to the scene. The
original direct-call sequence contains two `AddCaster` calls and one call each
to `SetLight`, `SetShadowStrength`, and `SetShadowGrey`. The complete compiled
caller remains strict after the connections.

RTTI reached through vtables `0x005575c8` and `0x0055156c` identifies
`ProjectedShadow : GameObject` and
`D3DIMSoultreeShadow : ShadowReceiver : GameObject`. The relevant base PMDs
are `(mdisp=0, pdisp=-1, vdisp=0)`. Native compilation checks both complete
sizes and emits unchanged-pointer conversions to the primary `GameObject`.
This supports the allocation and returned-pointer conversions; it does not
establish equivalence of every provisional scene declaration.

`Attach` receives the same 32-bit render-context address, unchanged caster
pointer, and projected-shadow primary pointer. It sets the render context,
stores the shadow, registers the receiver, then stores the caster and returns
the receiver. Its null-shadow path returns null without changing the receiver.
The canonical host parameter is currently `int`; the explicit cast preserves
all pointer bits on the supported x86 target. `SetLight` takes two pointers
to three consecutive floats and an integer mode. Native layout checks cover
all three `Vector3`/`ShadowVec3` component offsets and their 12-byte sizes.
Its actual integer return is ignored by this caller, as in retail.

The existing `UnknownKrustyUIShadow` view is retained only for its still-missing
`Init` boundary and primary `GameObject` relationship. Its retired method
declarations and the unused local shadow-link class have been removed.
`ProjectedShadow.h` uses relative transitive includes so the garage caller
also compiles without an extra physics include directory.

## Regression evidence

`tests/test_garage_shadow_connections.py` checks the retail image hash, RTTI,
allocation constants, direct calls, native layouts, actual imported provider
names, 130 strict caller/provider cases, and the full unchanged 1,102-byte
`KrustyUI::Create` candidate fingerprint. That candidate remains nonexact
against its 1,104-byte retail extent.

The native fixture executes eight unchanged production function texts:
the five connected nonconstructor methods plus `AddReceiver`,
`ResizeVertexBuffer`, and `GameObject::SetRenderContext`. Each
production object and extracted test function must strictly match its
complete retail body before execution. Only the extracted file-name literal
binding changes; no function statements are replaced.

The 3,776 cases cover packed color writes, null-shadow attachment, receiver
list allocation/growth/reuse, raw pointer identity, caster growth and vertex
buffer capacity, light-mode return values, and camera callback arguments.
Whole shadow/receiver storage is checked around these state changes.
Allocator, vertex-count, and camera dependencies are scripted. Light inputs
are signed unit axes; the unexercised reciprocal-square-root boundary fails
fast. Constructors are proved by complete strict bodies, allocation/RTTI
evidence, and native layout checks; the fixture uses raw storage and does not
run their unrelated virtual/rendering dependencies.

Private before/after production-build comparison preserved all 207 objects:
10,409 complete nondebug sections and 44,995 relocations, allowing only the
seven named external substitutions. Raw section bytes, relocation addends,
types, and destinations were compared; compiler-local targets were identified
by destination-section symbol and offset. Four affected sample objects also
preserved all 132 nondebug sections and 610 relocations, including their
complete partial bodies. Reports remain under ignored
`work/game-build/shadow-round21/`.

The isolated real build compiled 207/207 translation units and reduced
unresolved symbols from 471 to 464, removing exactly the seven garage names
and introducing none. `ProjectedShadow::Init` is still a partial sample,
other shadow rendering providers are missing, and this does not establish
working garage rendering or complete game startup.

Reproduce the focused test with private `VC6_ROOT` and `MCM2_EXE` configured:

```powershell
python -m unittest discover -s tests -p test_garage_shadow_connections.py -v
```
