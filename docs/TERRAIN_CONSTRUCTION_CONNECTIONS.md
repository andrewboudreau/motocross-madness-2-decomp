# Terrain construction and rendering connections

Terrain now calls eight existing implementations through their canonical
declarations. No provider body changed.

| Former declaration | Existing implementation | Retail entry / complete bytes |
| --- | --- | --- |
| `GetIdentityMatrix(TerrainMatrix*)` | `IdentityMatrix()` | `0x4a1410` / 71 |
| `TerrainColorMapper` constructor | `ColorMapper` constructor | `0x4ddf40` / 689 |
| `TerrainColorMapper::GetColors` | `ColorMapper::GetColors` | `0x4de270` / 4 |
| `TerrainGridBlock` constructor | `GridBaseBlock` constructor | `0x47dc20` / 382 |
| `TerrainShutdownObject` constructor | `DrawableGridNodeSharedTextures` constructor | `0x47c880` / 176 |
| `TerrainShutdownObject::DrawVisible` | `DrawableGridNode::DrawVisible` | `0x480900` / 23 |
| `TerrainShutdownObject::DrawVisibleWithDetail` | `DrawableGridNode::DrawVisibleWithDetail` | `0x480920` / 26 |
| `TerrainShutdownObject::Shutdown` | `DrawableGridNode::ReleaseVertexBuffers` | `0x47edb0` / 441 |

## Identity and call boundaries

The loader's allocations retain sizes `0x18b10`, `0xd50` and `0x5c`, their
original debug source lines, and all stack arguments. ColorMapper's constructor
installs vtable `0x557658`; its primary BaseObject base and RGB entries at `+0x10`
agree with the loader's stored palette pointer and 256-entry palette copy.
GridBaseBlock's three streamed arrays and base-height member account for the
complete `0xd50` allocation. Its constructor takes the same stream already used
by Terrain's canonical Read/Seek/Tell calls.

The shared-texture constructor installs vtable `0x553e54`. Retail RTTI puts
DrawableGridNode and GridNode at offset zero. The stored height-field pointer
therefore supplies the primary receiver for drawing and recursive buffer release.
The eleven constructor arguments retain the stream, host, null parent, Terrain
pointer, dimensions, indices and bounds pointers in their original order.
The detail texture remains the same 32-bit argument in DrawVisibleWithDetail's
currently provisional integer parameter. These selected connections do not
establish every field or method in the remaining TerrainShutdownObject view.

The identity helper returns a 64-byte Matrix4 by value through VC6's hidden
output argument. The Terrain constructor now expresses that return convention
using the canonical helper and a Matrix4 view of its 64-byte matrix storage.
The complete caller still matches all 370 bytes, including its copy from the
returned pointer. Matrix names and complete original source ownership remain
inferences as documented in MatrixUtil.h.

## Validation

The extended `tests/test_terrain_connections.py` checks all eight complete
providers strictly, ten direct caller sites against retail, and the existing
190 strict terrain/dependency cases. The layout fixture checks allocation sizes,
palette offset, stream fields, grid level/shift offsets and matrix extent.
Retail RTTI checks cover ColorMapper's BaseObject base and the shared node's
DrawableGridNode/GridNode bases, all at offset zero.

Terrain's complete constructor (370 bytes), render slot (3,173 bytes) and
destructor (441 bytes) retain strict matches. Load remains nonexact: its complete
2,117-byte candidate, all exception sections and relocation destinations are
unchanged under four external-name substitutions. Its named-relocation
fingerprint changes for those names; this is not promotion of the loader to a
retail-byte match.

The private object audit preserves all 85 Terrain nondebug sections and 517
relocations. Across the combined terrain and bike-modifier changes, all 210
previous production objects retain their 10,447 sections and 45,217 relocations
under the fourteen verified external renames. This is before/after preservation,
not proof that every production object matches retail.

The isolated real link compiled 210/210 translation units and reduced unresolved
names from 447 to 439, removing exactly these eight with no additions. Live
terrain loading, rendering and racing await a linked source-built game.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_terrain_connections.py
python tests/test_terrain_sampling_connections.py
python tools/build_game.py -j6
```
