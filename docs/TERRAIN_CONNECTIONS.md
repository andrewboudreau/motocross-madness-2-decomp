# Terrain caller connections

Three provisional method names now call the existing canonical Terrain
definitions. Each caller includes `src/krusty2/broadphase/Terrain.h` and casts
only at the evidenced call boundary; partial views and all other operations
remain unchanged. No wrapper, replacement provider or alternate Terrain class
was added.

| Caller | Retired method view | Canonical provider | Full extent |
| --- | --- | --- | --- |
| Grid1 texture eviction callbacks | `Grid1Terrain::RetireOwnedObject` | `Terrain::RetireOwnedObject`, 0x005057d0 | 83 bytes |
| QuarryStuntEvent detail controls | `UnknownTerrain::SelectQuality` | `Terrain::SelectQuality`, 0x00507960 | 129 bytes |
| NameOverlay visibility test in TrackOverlay | `UnknownTerrain::CastSegment` | `Terrain::CastSegment`, 0x00506e90 | 1654 bytes |

## Boundary evidence

Confirmed retail RTTI identifies Terrain's primary GameObject base at zero and
its GroundFogableObject base at +0x2c (COL 0x0055f338). That establishes the
canonical inheritance, not the identity of every provisional view by itself.
The connections also use the original receiver-producing instructions, direct
calls and complete caller byte matches:

- Grid1's callbacks at 0x0047ca00 and 0x0047ca40 obtain the node's terrain,
  retire its built texture and clear the texture/age fields. The node constructor
  retains its supplied terrain pointer. The view's focal length at +0x70 and
  texture table at +0x544 match Terrain. The actual provider searches the live
  pointer array at +0xcb8, shrinks its count at +0xcbc, swaps in the last live
  entry, and keeps the retired pointer immediately after the live prefix.
- QuarryStuntEvent's control handler at 0x004e0ce0 changes the detail setting
  and supplies its `eventTerrain` directly. The event loader creates Terrain
  with 0x00505830 and loads it with 0x005059d0. The provider preserves the
  existing quality table, detail override and dirty-distance behavior.
- NameOverlay's 0x00519000 visibility test supplies the event's terrain, the
  camera eye position, target point and a local hit vector. Its integer return
  remains the hit/occlusion result. `Vector3` and `TerrainVec3` are each three
  floats at offsets 0/4/8; the two input pointers remain const and the third
  remains writable. The thiscall receiver and all six stack arguments are
  unchanged. Terrain's segment cast scales and clips the segment, calls its
  height-field interface and writes the world-space hit on success.

These are specific method-boundary connections. They do not merge the whole
`Grid1Terrain` or `UnknownTerrain` declarations with Terrain or infer equivalence
of unrelated virtual slots.

## Verification and remaining dependencies

With native `VC6_ROOT` and the hash-pinned private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python tests/test_terrain_connections.py -v
```

The regression compiles the three actual caller TUs, Terrain.cpp,
TerrainSupport.cpp, FileStream.cpp and native layout assertions. It preserves **190** strict
cases, proves the three full COFF function extents and retail bytes with
resolved relocations, and checks that actual caller objects import the
canonical providers rather than the retired names. It also retains complete
COFF-body/named-relocation hashes for the two existing partial Terrain functions
(`Load` and `QueryGround`); these hashes are preservation checks, not retail
matches. Retail RTTI and native layout checks protect the selected base,
receiver-field and vector boundaries.

Reconstructed consumers and the layout fixture compile without an added physics
include search path, matching the calibration driver. Terrain.h uses relative
paths for its core/math dependencies, with their original include order intact.

The isolated source build compiled **198/198** translation units and reduced
unresolved names **527 to 524**, exactly these three names, with none added.
This is not full terrain dependency closure: CastSegment still imports the
existing unresolved `TerrainVec3::operator*=`, and its runtime height-field
object must be supplied by the remaining game reconstruction. No terrain
runtime integration or completed game is claimed.

## Terrain stream calls

Terrain::Load now calls the existing UnknownTextureStream `Read`, `Seek` and
`Tell` methods at its stream boundary. The provisional TerrainStream layout
and Load signature remain unchanged. Both stream views are 0x134 bytes, with
their nested stream at +0x1c and slice offset at +0x130. Load's existing
receiver-producing instructions, all 14 reads, three seeks and one tell match
the corresponding retail call sites. The complete retail loader is 2116 bytes;
the current 2117-byte source loader remains nonexact.

Before/after VC6 objects preserve all 84 non-debug sections' bytes, sizes,
attributes and relocation destinations, allowing only the three externally
renamed providers. Compiler-local labels retain their section and offset.
Adding the stream header renumbers an EH label in Load, so its full named
relocation fingerprint was refreshed after that comparison; its raw instruction
hash and the complete QueryGround fingerprint are unchanged. The committed
test checks those fingerprints, all 18 actual imports, decoded retail call
counts, stream layouts and the existing strict caller/provider cases.

Read and Seek remain the already reviewed nonexact bodies in
`samples/io/FileStream.cpp`; Tell's complete 63-byte body is strict exact.
The [stream differential test](FILESTREAM.md#functional-build-review) still
passes 12,390 cases. This boundary change does not newly validate the complete
terrain loader or claim a fully reconstructed stream hierarchy. The isolated
real build compiles 200/200 units and removes exactly these three obsolete
names: **522 to 519**, with no new unresolved names.

## QueryGround behavior remains incomplete

The five production QueryGround view names now use the canonical declaration;
see [the caller connection proof](TERRAIN_QUERY_CONNECTIONS.md). That change
preserves caller bytes and does not accept the provider's behavior.
The existing provider at 0x00507c10 is registered as partial. Its current native
COFF extent is **3255 bytes**, against the complete **2963-byte** retail body,
with a separate 370-byte local conversion helper. Unresolved helper bindings
still prevent a complete strict comparison.

The [cell conversion repair](TERRAIN_CELL_CONVERSION.md) corrects the former
truncation defect: a scaled coordinate of 1.25 now selects cell one under
nearest rounding. It passes 278,411 native comparisons through the first
GetCellCorners request, with masked exceptions. The complete QueryGround
fingerprint was intentionally refreshed for that repair; it is not preserved
from the earlier connection-only slice. Full interpolation, normals, material
selection and unmasked exception behavior still need independent validation.
