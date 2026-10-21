# Model LOD, lighting and screen-cache connections

Seven caller translation units now use five existing D3DIMSoultreeObject
implementations through canonical declarations. Ten unresolved call-view names
are retired. No provider implementation, forwarding wrapper, virtual slot or
object layout is added or changed.

| Existing provider | Retail entry | Complete bytes | Retired receiver views |
| --- | --- | ---: | --- |
| SetUseLighting(int) | `0x4444c0` | 23 | BikeA1A0, UnknownKrustyUISoultree, UnknownSceneLodObject, SoultreeD3DNode |
| ClearScreenRects() | `0x4444e0` | 55 | BikeA1A0, VehSceneNodeView::Method_004444E0 |
| SetLod(int) | `0x444d00` | 56 | KbXform |
| SetLowestLod(int) | `0x444d40` | 32 | KbXform |
| ApplyLodSettings(int) | `0x4451e0` | 81 | UnknownBikeRaceModel, UnknownSceneLodObject |

All five complete providers match retail strictly with resolved relocations.
Their existing dependencies are also supplied: SoultreeObject::SetFlag14c
(`0x4fdb40`, 11 bytes), D3DIMSoultreeObject::SetLodThreshold (`0x444d60`,
19 bytes), and the shared LOD settings pointer/tables.

## Caller and receiver evidence

The canonical calls cover Bike's rider lighting and screen-cache cleanup,
Vehicle's model cleanup, KrustyBike's bike/rider LOD selection, BikeRace and
Scene detail settings, the KrustyUI garage models, and both physics model
loaders. There are 25 emitted direct sites in the seven production callers.
KrustyBike's two source branches share one emitted SetLod call. Two remaining
lighting calls in the nonadmitted SceneManager sample use the same boundary.

These are primary model pointers. D3DIMSoultreeObject RTTI places SoultreeObject
at zero and GameObject at `+0xc`; none of these calls requires the secondary
GameObject adjustment. Character stores its model at `+0x1a0`, reached at
`+0x3bc` within Vehicle, and Bike's rider pointer is at `+0x5c4`. The race
views retain those offsets. Scene's character and procedural-object paths
read their model pointers at `+0x1a0` and `+0x34`, respectively.

The physics object constructor stores its actual D3DIMSoultreeObject base,
at object offset `+0x21c`, into sceneNode. The character loader instead copies
its loaded modelNode there. The added declarations in the existing physics
model view are nonvirtual methods; they neither change that base relationship
nor introduce a second implementation. Rendering consumers use the existing
full model declaration. Explicit casts preserve the observed pointer values;
other provisional methods and class views remain separate.

SetUseLighting preserves arbitrary integer values at `+0x1a4` and sets the
dirty flag at `+0x18c`. ClearScreenRects zeroes both rectangles at
`+0x298/+0x2a8`, then sets the node flag at `+0x14c`. SetLod and SetLowestLod
retain their original clamp order, including zero/negative LOD counts, and
SetLod marks dirty only when the selected level changes. ApplyLodSettings
still reads the active 16-byte settings row, stores the reciprocal threshold
at `+0x284`, copies perspective/shading/lighting values, and marks dirty.

## Preservation and native checks

The isolated real build `build-_oyof5cf` compiles 212/212 translation units
and reduces unresolved names from 415 to 405: exactly the ten names above
removed, none added. Compared with the preceding collision slice's
`build-3fd68y1r`, every pre-existing production object is unchanged under
those ten symbol substitutions: 10,456 nondebug sections and 45,266
relocations retain raw bytes, attributes, addends and destination identities.

A same-path private trial also compares all ten affected caller/sample
objects: 2,040 nondebug sections and 11,315 relocations are unchanged under
the same substitutions. This includes the complete BikeRace, KrustyBike and
SceneManager near-miss aggregates, rather than only their matched prefixes.

The composed GraphicsTest inherited-method cleanup independently changes
one nonadmitted sample: CreateRacers still emits 13,428 bytes, but at offsets
538 through 544 VC6 exchanges `MOV EBX,1; XOR EDI,EDI` for
`XOR EDI,EDI; MOV EBX,1`. Both orders produce identical registers and flags,
with no memory access or exception. There is no branch, symbol or relocation
target into the window interior. Every other body byte and all 206 aggregate
sections/1,929 relocation records are preserved under the combined symbol
renames. Matching positions improve from 13,390 to 13,396; the function
remains nonexact. Only this complete candidate fingerprint is refreshed in
the existing character-motion and garage-preview tests.

`tests/test_model_render_state_connections.py` checks 511 complete strict
cases, 28 complete candidate fingerprints, primary/secondary retail RTTI,
the actual pointer-field layouts, and compiled base conversions. Seven
unchanged helper bodies are extracted for native execution; both their
production objects and extracted objects must match their complete retail
extents before linking the fixture. Its 29,040 cases compare all model bytes
and surrounding guards across LOD clamp/change paths, integer lighting values,
screen clearing, and both selected settings-table pointers. No model is
constructed and no rendering dependency is mocked inside a tested helper.
This is a provider and call-boundary test, not integrated rendering or racing.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_model_render_state_connections.py
python tools/build_game.py -j6
```
