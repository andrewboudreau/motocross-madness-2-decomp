# Vehicle camera and audio model connections

The former `UnknownVehiclePart` view in `VehicleCamera.h` duplicated six
existing scene-node providers. VehicleCamera, BikeCamera, KrustyBikeCamera,
AuralScape, RaceSound and TrackOverlay now use `SoultreeObject` pointers.
The pointer-only headers forward-declare the class; the caller implementations
include the render-side declaration in `D3DIMSoulTree.h`.
RaceView retains `UnknownVehiclePart` as a typedef to `SoultreeObject`, not a
second class. Keeping this local compatibility name preserves VC6's code
generation for ProCircuitProcs through TrackGame.h: replacing the race-view
name directly changed nine positions in the 756-byte `AddCircuitRow` body
at `0x004d59a0`. The alias restores its complete strict match and emits the
same canonical scene-node method symbols.

| Existing provider | Retail VA | Complete bytes | Source |
| --- | --- | ---: | --- |
| `SoultreeObject::GetAxes` | `0x004fc4f0` | 71 | `src/krusty2/soultree/soultree.cpp` |
| `SoultreeObject::GetPosition` | `0x004fc970` | 33 | same |
| `SoultreeObject::GetPositionIn` | `0x004fc9a0` | 134 | same |
| `SoultreeObject::FindByName` | `0x004fdae0` | 84 | same |
| `SoultreeObject::ClearFlag14c` | `0x004fdb50` | 11 | same |
| `D3DIMSoultreeObject::ClearScreenRects` | `0x004444e0` | 55 | `src/reconstructed/D3DIMSoulTree.cpp` |

The identity evidence is the full decoded caller/provider behavior and resolved
direct calls, supported by retail RTTI and layouts. The node's first-child link
is at `+0x140`. Its local/world matrices are at `+0xb8/+0xf8`.
`GetPositionIn` takes a node-relative frame pointer, not an integer index; these
callers pass null for world space. `GetPosition` writes the local translation
to its explicit output buffer and returns that same pointer in EAX. The callers
ignore the return. `GetAxes` writes local matrix rows three and two, accepting
null output pointers independently. Existing `Vector3` locals remain in place
and cross the `Vec3*` interface through explicit casts; both representations
contain the same three consecutive floats.

`ClearScreenRects` is a derived render helper. Only BikeCamera's two model-root
calls cast to `D3DIMSoultreeObject*`; the helper clears the two rectangles at
`+0x298/+0x2a8` and calls `SetFlag14c`. Retail RTTI gives the derived class's
SoultreeObject/QuadTreeObject bases at zero and its GameObject base at `+0x0c`:
COLs `0x0055b2b8/0x0055b248`, vtables `0x005513ec/0x0055137c`. SoultreeObject's
own COLs are `0x0055f030/0x0055eff0`, vtables `0x00557c18/0x00557c40`, with the
same object offsets. The existing render declaration preserves this multiple
inheritance. The separate flat physics declaration in
`src/krusty2/core/SoultreeObject.h` is not included in these render callers;
unifying the two complete class views remains separate work.

The local containing vehicle, rider and racer views remain provisional. This
connection does not establish their full types or original translation-unit
ownership. Semantic helper names are reconstructed names. The complete node
pointer parameter also rekeys `SoultreeSoundEmitter::CreateFollowing` at
`0x004029f0`; canonical and partial SceneManager sample bindings follow that
same provider. The SceneManager near misses remain samples.

`test_vehicle_model_connections.py` compiles the six callers, D3DIMSoulTree,
SceneManager, soultree.cpp and layout assertions with authentic VC6 SP3. It
also compiles ProCircuitProcs to cover the indirect header consumer. It
checks 405 complete calibration cases with relocation resolution, all six
provider extents above, real COFF definitions/imports, and independently parsed
retail RTTI. It additionally compiles the partial SceneManager sample to check
its `CreateFollowing` relocation without claiming its bodies exact. No provider
body, object lifetime or runtime fallback is added. This removes six missing
link names. The [current combined source build](GAME_BUILD.md) also includes
the scene-profiler connections and reviewed transform candidates; the game
does not yet link or run from source.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_vehicle_model_connections.py -v
python tools/build_game.py -j6
```
